
#ifndef EL_DOM_SGML_PARSER_H
#define EL_DOM_SGML_PARSER_H

#include "dom/node.h"
#include "dom/stack.h"
#include "dom/sgml/sgml.h"
#include "dom/scanner.h"

struct string;
struct uri;

enum sgml_parser_type {
	/* The first one is a DOM tree builder. */
	SGML_PARSER_TREE,
	/* The second one will simply push nodes on the stack, not building a
	 * DOM tree. This interface is similar to that of SAX (Simple API for
	 * XML) where events are fired when nodes are entered and exited. It is
	 * useful when you are not actually interested in the DOM tree, but can
	 * do all processing in a stream-like manner, such as when highlighting
	 * HTML code. */
	SGML_PARSER_STREAM,
};

struct sgml_parser_state {
	/* Info about the properties of the node contained by state.
	 * This is only meaningful to element and attribute nodes. For
	 * unknown nodes it points to the common 'unknown node' info. */
	struct sgml_node_info *info;
	/* This is used by the DOM source renderer for highlighting the
	 * end-tag of an element. */
	struct dom_scanner_token end_token;
};

struct sgml_parser {
	enum sgml_parser_type type;	/* Stream or tree */

	struct sgml_info *info;		/* Backend dependent info */

	struct dom_string uri;		/* The URI of the DOM document */
	struct dom_node *root;		/* The document root node */

	struct dom_stack stack;		/* A stack for tracking parsed nodes */
	struct dom_stack parsing;	/* Used for tracking parsing states */
};

struct sgml_parser *
init_sgml_parser(enum sgml_parser_type type, enum sgml_document_type doctype,
		 struct dom_string *uri);

void done_sgml_parser(struct sgml_parser *parser);

struct dom_node *parse_sgml(struct sgml_parser *parser, struct dom_string *buffer);

#endif
