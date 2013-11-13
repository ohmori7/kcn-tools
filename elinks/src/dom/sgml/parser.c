/* SGML node handling */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include "elinks.h"

#include "dom/node.h"
#include "dom/sgml/parser.h"
#include "dom/sgml/scanner.h"
#include "dom/sgml/sgml.h"
#include "dom/stack.h"
#include "dom/string.h"
#include "util/error.h"
#include "util/memory.h"


/* This holds info about a chunk of text being parsed. The SGML parser uses
 * these to keep track of possible nested calls to parse_sgml(). This can be
 * used to feed output of stuff like ECMAScripts document.write() from
 * <script>-elements back to the SGML parser. */
struct sgml_parsing_state {
	struct dom_scanner scanner;
	struct dom_node *node;
	size_t depth;
};

static struct sgml_parsing_state *
init_sgml_parsing_state(struct sgml_parser *parser, struct dom_string *buffer);


/* When getting the sgml_parser struct it is _always_ assumed that the parser
 * is the first to add it's context, which it is since it initializes the
 * stack. */

#define get_sgml_parser(stack) ((stack)->contexts[0]->data)

#define get_sgml_parser_state(stack, state) \
	get_dom_stack_state_data(stack->contexts[0], state)


/* Functions for adding new nodes to the DOM tree: */

/* They wrap init_dom_node() and add_dom_*() and set up of additional
 * information like node subtypes and SGML parser state information. */

static inline struct dom_node *
add_sgml_document(struct dom_stack *stack, struct dom_string *string)
{
	struct dom_node *node = init_dom_node(DOM_NODE_DOCUMENT, string);

	return node ? push_dom_node(stack, node) : NULL;
}

static inline struct dom_node *
add_sgml_element(struct dom_stack *stack, struct dom_scanner_token *token)
{
	struct sgml_parser *parser = get_sgml_parser(stack);
	struct dom_node *parent = get_dom_stack_top(stack)->node;
	struct dom_stack_state *state;
	struct sgml_parser_state *pstate;
	struct dom_node *node;
	struct sgml_node_info *node_info;

	node = add_dom_element(parent, &token->string);
	if (!node) return NULL;

	node_info = get_sgml_node_info(parser->info->elements, node);
	node->data.element.type = node_info->type;

	if (!push_dom_node(stack, node))
		return NULL;

	state = get_dom_stack_top(stack);
	assert(node == state->node);

	pstate = get_sgml_parser_state(stack, state);
	pstate->info = node_info;

	return node;
}


static inline void
add_sgml_attribute(struct dom_stack *stack,
		   struct dom_scanner_token *token, struct dom_scanner_token *valtoken)
{
	struct sgml_parser *parser = get_sgml_parser(stack);
	struct dom_node *parent = get_dom_stack_top(stack)->node;
	struct dom_string *value = valtoken ? &valtoken->string : NULL;
	struct sgml_node_info *info;
	struct dom_node *node;

	node = add_dom_attribute(parent, &token->string, value);

	info = get_sgml_node_info(parser->info->attributes, node);

	node->data.attribute.type      = info->type;
	node->data.attribute.id	       = !!(info->flags & SGML_ATTRIBUTE_IDENTIFIER);
	node->data.attribute.reference = !!(info->flags & SGML_ATTRIBUTE_REFERENCE);

	if (valtoken && valtoken->type == SGML_TOKEN_STRING)
		node->data.attribute.quoted = 1;

	if (!node || !push_dom_node(stack, node))
		return;

	pop_dom_node(stack);
}

static inline struct dom_node *
add_sgml_proc_instruction(struct dom_stack *stack, struct dom_scanner_token *target,
			  struct dom_scanner_token *data)
{
	struct dom_node *parent = get_dom_stack_top(stack)->node;
	struct dom_string *data_str = data ? &data->string : NULL;
	struct dom_node *node;

	node = add_dom_proc_instruction(parent, &target->string, data_str);
	if (!node) return NULL;

	switch (target->type) {
	case SGML_TOKEN_PROCESS_XML:
		node->data.proc_instruction.type = DOM_PROC_INSTRUCTION_XML;
		break;

	case SGML_TOKEN_PROCESS:
	default:
		node->data.proc_instruction.type = DOM_PROC_INSTRUCTION;
	}

	return push_dom_node(stack, node);
}

static inline void
add_sgml_node(struct dom_stack *stack, enum dom_node_type type, struct dom_scanner_token *token)
{
	struct dom_node *parent = get_dom_stack_top(stack)->node;
	struct dom_node *node = add_dom_node(parent, type, &token->string);

	if (!node) return;

	if (token->type == SGML_TOKEN_SPACE)
		node->data.text.only_space = 1;

	if (push_dom_node(stack, node))
		pop_dom_node(stack);
}


/* SGML parser main handling: */

static inline void
parse_sgml_attributes(struct dom_stack *stack, struct dom_scanner *scanner)
{
	struct dom_scanner_token name;

	assert(dom_scanner_has_tokens(scanner)
	       && (get_dom_scanner_token(scanner)->type == SGML_TOKEN_ELEMENT_BEGIN
	           || (get_dom_stack_top(stack)->node->type == DOM_NODE_PROCESSING_INSTRUCTION)));

	if (get_dom_scanner_token(scanner)->type == SGML_TOKEN_ELEMENT_BEGIN)
		skip_dom_scanner_token(scanner);

	while (dom_scanner_has_tokens(scanner)) {
		struct dom_scanner_token *token = get_dom_scanner_token(scanner);

		assert(token);

		switch (token->type) {
		case SGML_TOKEN_TAG_END:
			skip_dom_scanner_token(scanner);
			/* and return */
		case SGML_TOKEN_ELEMENT:
		case SGML_TOKEN_ELEMENT_BEGIN:
		case SGML_TOKEN_ELEMENT_END:
		case SGML_TOKEN_ELEMENT_EMPTY_END:
			return;

		case SGML_TOKEN_IDENT:
			copy_struct(&name, token);

			/* Skip the attribute name token */
			token = get_next_dom_scanner_token(scanner);
			if (token && token->type == '=') {
				/* If the token is not a valid value token
				 * ignore it. */
				token = get_next_dom_scanner_token(scanner);
				if (token
				    && token->type != SGML_TOKEN_IDENT
				    && token->type != SGML_TOKEN_ATTRIBUTE
				    && token->type != SGML_TOKEN_STRING)
					token = NULL;
			} else {
				token = NULL;
			}

			add_sgml_attribute(stack, &name, token);

			/* Skip the value token */
			if (token)
				skip_dom_scanner_token(scanner);
			break;

		default:
			skip_dom_scanner_token(scanner);

		}
	}
}

static void
parse_sgml_plain(struct dom_stack *stack, struct dom_scanner *scanner)
{
	struct dom_scanner_token target;

	while (dom_scanner_has_tokens(scanner)) {
		struct dom_scanner_token *token = get_dom_scanner_token(scanner);

		switch (token->type) {
		case SGML_TOKEN_ELEMENT:
		case SGML_TOKEN_ELEMENT_BEGIN:
			if (!add_sgml_element(stack, token)) {
				if (token->type == SGML_TOKEN_ELEMENT) {
					skip_dom_scanner_token(scanner);
					break;
				}

				skip_sgml_tokens(scanner, SGML_TOKEN_TAG_END);
				break;
			}

			if (token->type == SGML_TOKEN_ELEMENT_BEGIN) {
				parse_sgml_attributes(stack, scanner);
			} else {
				skip_dom_scanner_token(scanner);
			}

			break;

		case SGML_TOKEN_ELEMENT_EMPTY_END:
			pop_dom_node(stack);
			skip_dom_scanner_token(scanner);
			break;

		case SGML_TOKEN_ELEMENT_END:
			if (!token->string.length) {
				pop_dom_node(stack);
			} else {
				struct dom_string string;
				struct dom_stack_state *state;

				set_dom_string(&string, token->string.string, token->string.length);
				state = search_dom_stack(stack, DOM_NODE_ELEMENT,
							 &string);
				if (state) {
					struct sgml_parser_state *pstate;

					pstate = get_sgml_parser_state(stack, state);
					copy_struct(&pstate->end_token, token);

					pop_dom_state(stack, state);
				}
			}
			skip_dom_scanner_token(scanner);
			break;

		case SGML_TOKEN_NOTATION_COMMENT:
			add_sgml_node(stack, DOM_NODE_COMMENT, token);
			skip_dom_scanner_token(scanner);
			break;

		case SGML_TOKEN_NOTATION_ATTLIST:
		case SGML_TOKEN_NOTATION_DOCTYPE:
		case SGML_TOKEN_NOTATION_ELEMENT:
		case SGML_TOKEN_NOTATION_ENTITY:
		case SGML_TOKEN_NOTATION:
			skip_dom_scanner_token(scanner);
			break;

		case SGML_TOKEN_CDATA_SECTION:
			add_sgml_node(stack, DOM_NODE_CDATA_SECTION, token);
			skip_dom_scanner_token(scanner);
			break;

		case SGML_TOKEN_PROCESS_XML_STYLESHEET:
		case SGML_TOKEN_PROCESS_XML:
		case SGML_TOKEN_PROCESS:
			copy_struct(&target, token);

			/* Skip the target token */
			token = get_next_dom_scanner_token(scanner);
			if (!token) break;

			assert(token->type == SGML_TOKEN_PROCESS_DATA);

			if (add_sgml_proc_instruction(stack, &target, token)
			    && (target.type == SGML_TOKEN_PROCESS_XML
			        || target.type == SGML_TOKEN_PROCESS_XML_STYLESHEET)
			    && token->string.length > 0) {
				/* Parse the <?xml data="attributes"?>. */
				struct dom_scanner attr_scanner;

				init_dom_scanner(&attr_scanner, &sgml_scanner_info,
						 &token->string, SGML_STATE_ELEMENT,
						 scanner->count_lines);

				if (dom_scanner_has_tokens(&attr_scanner))
					parse_sgml_attributes(stack, &attr_scanner);
			}

			pop_dom_node(stack);
			skip_dom_scanner_token(scanner);
			break;

		case SGML_TOKEN_ENTITY:
			add_sgml_node(stack, DOM_NODE_ENTITY_REFERENCE, token);
			skip_dom_scanner_token(scanner);
			break;

		case SGML_TOKEN_SPACE:
		case SGML_TOKEN_TEXT:
		default:
			add_sgml_node(stack, DOM_NODE_TEXT, token);
			skip_dom_scanner_token(scanner);
		}
	}
}

struct dom_node *
parse_sgml(struct sgml_parser *parser, struct dom_string *buffer)
{
	struct sgml_parsing_state *parsing;

	if (!parser->root) {
		parser->root = add_sgml_document(&parser->stack, &parser->uri);
		if (!parser->root)
			return NULL;
		get_dom_stack_top(&parser->stack)->immutable = 1;
	}

	parsing = init_sgml_parsing_state(parser, buffer);
	if (!parsing) return NULL;

	/* FIXME: Make parse_sgml_plain() return something (error code or if
	 * can be guarenteed a root node). */
	parse_sgml_plain(&parser->stack, &parsing->scanner);

	pop_dom_node(&parser->parsing);

	return parser->root;
}


/* Parsing state management: */

/* The SGML parser can handle nested calls to parse_sgml(). This can be used to
 * handle output of external processing of data in the document tree. For
 * example this can allows output of the document.write() from DOM scripting
 * interface to be parsed. */

static void
sgml_parsing_push(struct dom_stack *stack, struct dom_node *node, void *data)
{
	struct sgml_parser *parser = get_sgml_parser(stack);
	struct sgml_parsing_state *parsing = data;

	parsing->depth = parser->stack.depth;
	get_dom_stack_top(&parser->stack)->immutable = 1;
	init_dom_scanner(&parsing->scanner, &sgml_scanner_info, &node->string,
			 SGML_STATE_TEXT, 0);
}

static void
sgml_parsing_pop(struct dom_stack *stack, struct dom_node *node, void *data)
{
	struct sgml_parser *parser = get_sgml_parser(stack);
	struct sgml_parsing_state *parsing = data;

	/* Pop the stack back to the state it was in. This includes cleaning
	 * away even immutable states left on the stack. */
	while (parsing->depth < parser->stack.depth) {
		get_dom_stack_top(&parser->stack)->immutable = 0;
		pop_dom_node(&parser->stack);
	}

	assert(parsing->depth == parser->stack.depth);
}

static struct dom_stack_context_info sgml_parsing_context_info = {
	/* Object size: */			sizeof(struct sgml_parsing_state),
	/* Push: */
	{
		/*				*/ NULL,
		/* DOM_NODE_ELEMENT		*/ NULL,
		/* DOM_NODE_ATTRIBUTE		*/ NULL,
		/* DOM_NODE_TEXT		*/ sgml_parsing_push,
		/* DOM_NODE_CDATA_SECTION	*/ NULL,
		/* DOM_NODE_ENTITY_REFERENCE	*/ NULL,
		/* DOM_NODE_ENTITY		*/ NULL,
		/* DOM_NODE_PROC_INSTRUCTION	*/ NULL,
		/* DOM_NODE_COMMENT		*/ NULL,
		/* DOM_NODE_DOCUMENT		*/ NULL,
		/* DOM_NODE_DOCUMENT_TYPE	*/ NULL,
		/* DOM_NODE_DOCUMENT_FRAGMENT	*/ NULL,
		/* DOM_NODE_NOTATION		*/ NULL,
	},
	/* Pop: */
	{
		/*				*/ NULL,
		/* DOM_NODE_ELEMENT		*/ NULL,
		/* DOM_NODE_ATTRIBUTE		*/ NULL,
		/* DOM_NODE_TEXT		*/ sgml_parsing_pop,
		/* DOM_NODE_CDATA_SECTION	*/ NULL,
		/* DOM_NODE_ENTITY_REFERENCE	*/ NULL,
		/* DOM_NODE_ENTITY		*/ NULL,
		/* DOM_NODE_PROC_INSTRUCTION	*/ NULL,
		/* DOM_NODE_COMMENT		*/ NULL,
		/* DOM_NODE_DOCUMENT		*/ NULL,
		/* DOM_NODE_DOCUMENT_TYPE	*/ NULL,
		/* DOM_NODE_DOCUMENT_FRAGMENT	*/ NULL,
		/* DOM_NODE_NOTATION		*/ NULL,
	}
};

/* Create a new parsing state by pushing a new text node containing the*/
static struct sgml_parsing_state *
init_sgml_parsing_state(struct sgml_parser *parser, struct dom_string *buffer)
{
	struct dom_stack_state *state;
	struct dom_node *node;

	node = init_dom_node(DOM_NODE_TEXT, buffer);
	if (!node || !push_dom_node(&parser->parsing, node))
		return NULL;

	state = get_dom_stack_top(&parser->parsing);

	return get_dom_stack_state_data(parser->parsing.contexts[0], state);
}


/* Parser creation and destruction: */

/* FIXME: For now the main SGML parser context doesn't do much other than
 * declaring the sgml_parser_state object. */
static struct dom_stack_context_info sgml_parser_context_info = {
	/* Object size: */			sizeof(struct sgml_parser_state),
	/* Push: */
	{
		/*				*/ NULL,
		/* DOM_NODE_ELEMENT		*/ NULL,
		/* DOM_NODE_ATTRIBUTE		*/ NULL,
		/* DOM_NODE_TEXT		*/ NULL,
		/* DOM_NODE_CDATA_SECTION	*/ NULL,
		/* DOM_NODE_ENTITY_REFERENCE	*/ NULL,
		/* DOM_NODE_ENTITY		*/ NULL,
		/* DOM_NODE_PROC_INSTRUCTION	*/ NULL,
		/* DOM_NODE_COMMENT		*/ NULL,
		/* DOM_NODE_DOCUMENT		*/ NULL,
		/* DOM_NODE_DOCUMENT_TYPE	*/ NULL,
		/* DOM_NODE_DOCUMENT_FRAGMENT	*/ NULL,
		/* DOM_NODE_NOTATION		*/ NULL,
	},
	/* Pop: */
	{
		/*				*/ NULL,
		/* DOM_NODE_ELEMENT		*/ NULL,
		/* DOM_NODE_ATTRIBUTE		*/ NULL,
		/* DOM_NODE_TEXT		*/ NULL,
		/* DOM_NODE_CDATA_SECTION	*/ NULL,
		/* DOM_NODE_ENTITY_REFERENCE	*/ NULL,
		/* DOM_NODE_ENTITY		*/ NULL,
		/* DOM_NODE_PROC_INSTRUCTION	*/ NULL,
		/* DOM_NODE_COMMENT		*/ NULL,
		/* DOM_NODE_DOCUMENT		*/ NULL,
		/* DOM_NODE_DOCUMENT_TYPE	*/ NULL,
		/* DOM_NODE_DOCUMENT_FRAGMENT	*/ NULL,
		/* DOM_NODE_NOTATION		*/ NULL,
	}
};

struct sgml_parser *
init_sgml_parser(enum sgml_parser_type type, enum sgml_document_type doctype,
		 struct dom_string *uri)
{
	struct sgml_parser *parser;
	enum dom_stack_flag flags = 0;

	parser = mem_calloc(1, sizeof(*parser));
	if (!parser) return NULL;

	if (!init_dom_string(&parser->uri, uri->string, uri->length)) {
		mem_free(parser);
		return NULL;
	}

	parser->type = type;
	parser->info = get_sgml_info(doctype);

	if (type == SGML_PARSER_TREE)
		flags |= DOM_STACK_KEEP_NODES;

	init_dom_stack(&parser->stack, flags);
	/* FIXME: Some sgml backend specific callbacks? Handle HTML script tags,
	 * and feed document.write() data back to the parser. */
	add_dom_stack_context(&parser->stack, parser, &sgml_parser_context_info);

	/* Don't keep the 'fake' text nodes that holds the parsing data. */
	init_dom_stack(&parser->parsing, 0);
	add_dom_stack_context(&parser->parsing, parser, &sgml_parsing_context_info);

	return parser;
}

void
done_sgml_parser(struct sgml_parser *parser)
{
	done_dom_stack(&parser->stack);
	done_dom_stack(&parser->parsing);
	done_dom_string(&parser->uri);
	mem_free(parser);
}
