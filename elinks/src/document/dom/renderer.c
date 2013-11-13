/* DOM document renderer */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h> /* FreeBSD needs this before regex.h */
#ifdef HAVE_REGEX_H
#include <regex.h>
#endif
#include <string.h>

#include "elinks.h"

#include "bookmarks/bookmarks.h"	/* get_bookmark() */
#include "cache/cache.h"
#include "document/css/css.h"
#include "document/css/parser.h"
#include "document/css/property.h"
#include "document/css/stylesheet.h"
#include "document/docdata.h"
#include "document/document.h"
#include "document/dom/renderer.h"
#include "document/renderer.h"
#include "dom/scanner.h"
#include "dom/sgml/parser.h"
#include "dom/sgml/html/html.h"
#include "dom/node.h"
#include "dom/stack.h"
#include "intl/charsets.h"
#include "globhist/globhist.h"		/* get_global_history_item() */
#include "protocol/uri.h"
#include "terminal/draw.h"
#include "util/box.h"
#include "util/error.h"
#include "util/memory.h"
#include "util/snprintf.h"
#include "util/string.h"


struct dom_renderer {
	enum sgml_document_type doctype;
	struct document *document;

	struct conv_table *convert_table;
	enum convert_string_mode convert_mode;

	struct uri *base_uri;

	unsigned char *source;
	unsigned char *end;

	unsigned char *position;
	int canvas_x, canvas_y;

#ifdef HAVE_REGEX_H
	regex_t url_regex;
	unsigned int find_url:1;
#endif
	struct screen_char styles[DOM_NODES];
};

#define URL_REGEX "(file://|((f|ht|nt)tp(s)?|smb)://[[:alnum:]]+([-@:.]?[[:alnum:]])*\\.[[:alpha:]]{2,4}(:[[:digit:]]+)?)(/(%[[:xdigit:]]{2}|[-_~&=;?.a-z0-9])*)*"
#define URL_REGFLAGS (REG_ICASE | REG_EXTENDED)

static void
init_template(struct screen_char *template, struct document_options *options,
	      color_T background, color_T foreground)
{
	struct color_pair colors = INIT_COLOR_PAIR(background, foreground);

	template->attr = 0;
	template->data = ' ';
	set_term_color(template, &colors,
		       options->color_flags, options->color_mode);
}

static inline struct css_property *
get_css_property(struct list_head *list, enum css_property_type type)
{
	struct css_property *property;

	foreach (property, *list)
		if (property->type == type)
			return property;

	return NULL;
}


/* Checks the user CSS for properties for each DOM node type name */
static inline void
init_dom_renderer(struct dom_renderer *renderer, struct document *document,
		  struct string *buffer, struct conv_table *convert_table)
{
	enum dom_node_type type;
	struct css_stylesheet *css = &default_stylesheet;

	memset(renderer, 0, sizeof(*renderer));

	renderer->document	= document;
	renderer->convert_table = convert_table;
	renderer->convert_mode	= document->options.plain ? CSM_NONE : CSM_DEFAULT;
	renderer->source	= buffer->source;
	renderer->end		= buffer->source + buffer->length;
	renderer->position	= renderer->source;
	renderer->base_uri	= get_uri_reference(document->uri);

#ifdef HAVE_REGEX_H
	if (renderer->document->options.plain_display_links) {
		if (regcomp(&renderer->url_regex, URL_REGEX, URL_REGFLAGS)) {
			regfree(&renderer->url_regex);
		} else {
			renderer->find_url = 1;
		}
	}
#endif

	for (type = 0; type < DOM_NODES; type++) {
		struct screen_char *template = &renderer->styles[type];
		color_T background = document->options.default_bg;
		color_T foreground = document->options.default_fg;
		static int i_want_struct_module_for_dom;

		struct dom_string *name = get_dom_node_type_name(type);
		struct css_selector *selector = NULL;

		if (!i_want_struct_module_for_dom) {
			static const unsigned char default_colors[] =
				"document	{ color: yellow } "
				"element	{ color: lightgreen } "
				"entity-reference { color: red } "
				"proc-instruction { color: red } "
				"attribute	{ color: magenta } "
				"comment	{ color: aqua } "
				"cdata-section	{ color: orange2 } ";
			unsigned char *styles = (unsigned char *) default_colors;

			i_want_struct_module_for_dom = 1;
			/* When someone will get here earlier than at 4am,
			 * this will be done in some init function, perhaps
			 * not overriding the user's default stylesheet. */
			css_parse_stylesheet(css, NULL, styles, styles + sizeof(default_colors) + 1);
		}

		if (name)
		if (is_dom_string_set(name))
			selector = find_css_selector(&css->selectors,
						     CST_ELEMENT, CSR_ROOT,
						     name->string, name->length);

		if (selector) {
			struct list_head *properties = &selector->properties;
			struct css_property *property;

			property = get_css_property(properties, CSS_PT_BACKGROUND_COLOR);
			if (!property)
				property = get_css_property(properties, CSS_PT_BACKGROUND);

			if (property && property->value_type == CSS_VT_COLOR)
				background = property->value.color;

			property = get_css_property(properties, CSS_PT_COLOR);
			if (property) foreground = property->value.color;
		}

		init_template(template, &document->options, background, foreground);
	}
}


/* Document maintainance */

static struct screen_char *
realloc_line(struct document *document, int x, int y)
{
	struct line *line = realloc_lines(document, y);

	if (!line) return NULL;

	if (x > line->length) {
		if (!ALIGN_LINE(&line->chars, line->length, x))
			return NULL;

		for (; line->length < x; line->length++) {
			line->chars[line->length].data = ' ';
		}

		if (x > document->width) document->width = x;
	}

	return line->chars;
}

static struct node *
add_search_node(struct dom_renderer *renderer, int width)
{
	struct node *node = mem_alloc(sizeof(*node));

	if (node) {
		set_box(&node->box, renderer->canvas_x, renderer->canvas_y,
			width, 1);
		add_to_list(renderer->document->nodes, node);
	}

	return node;
}

#define X(renderer)		((renderer)->canvas_x)
#define Y(renderer)		((renderer)->canvas_y)
#define POS(renderer)		(&(renderer)->document->data[Y(renderer)].chars[X(renderer)])
#define WIDTH(renderer, add)	((renderer)->canvas_x + (add))

static void
render_dom_line(struct dom_renderer *renderer, struct screen_char *template,
		unsigned char *string, int length)
{
	struct document *document = renderer->document;
	struct conv_table *convert = renderer->convert_table;
	enum convert_string_mode mode = renderer->convert_mode;
	int x;

	assert(renderer && template && string && length);

	string = convert_string(convert, string, length, document->options.cp,
	                        mode, &length, NULL, NULL);
	if (!string) return;

	if (!realloc_line(document, WIDTH(renderer, length), Y(renderer))) {
		mem_free(string);
		return;
	}

	add_search_node(renderer, length);

	for (x = 0; x < length; x++, renderer->canvas_x++) {
		unsigned char data = string[x];

		/* This is mostly to be able to break out so the indentation
		 * level won't get to high. */
		switch (data) {
		case ASCII_TAB:
		{
			int tab_width = 7 - (X(renderer) & 7);
			int width = WIDTH(renderer, length - x + tab_width);

			template->data = ' ';

			if (!realloc_line(document, width, Y(renderer)))
				break;

			/* Only loop over the expanded tab chars and let the
			 * ``main loop'' add the actual tab char. */
			for (; tab_width-- > 0; renderer->canvas_x++)
				copy_screen_chars(POS(renderer), template, 1);
			break;
		}
		default:
			template->data = isscreensafe(data) ? data : '.';
		}

		copy_screen_chars(POS(renderer), template, 1);
	}

	mem_free(string);
}

static inline unsigned char *
split_dom_line(unsigned char *line, int length, int *linelen)
{
	unsigned char *end = line + length;
	unsigned char *pos;

	/* End of line detection.
	 * We handle \r, \r\n and \n types here. */
	for (pos = line; pos < end; pos++) {
		int step = 0;

		if (pos[step] == ASCII_CR)
			step++;

		if (pos[step] == ASCII_LF)
			step++;

		if (step) {
			*linelen = pos - line;
			return pos + step;
		}
	}

	*linelen = length;
	return NULL;
}

static void
render_dom_text(struct dom_renderer *renderer, struct screen_char *template,
		unsigned char *string, int length)
{
	int linelen;

	for (; length > 0; string += linelen, length -= linelen) {
		unsigned char *newline = split_dom_line(string, length, &linelen);

		if (linelen)
			render_dom_line(renderer, template, string, linelen);

		if (newline) {
			renderer->canvas_y++;
			renderer->canvas_x = 0;
			linelen = newline - string;
		}
	}
}

#define realloc_document_links(doc, size) \
	ALIGN_LINK(&(doc)->links, (doc)->nlinks, size)

static inline struct link *
add_dom_link(struct dom_renderer *renderer, unsigned char *string, int length)
{
	struct document *document = renderer->document;
	int x = renderer->canvas_x;
	int y = renderer->canvas_y;
	unsigned char *where;
	struct link *link;
	struct point *point;
	struct screen_char template;
	unsigned char *uristring;
	color_T fgcolor;

	if (!realloc_document_links(document, document->nlinks + 1))
		return NULL;

	link = &document->links[document->nlinks];

	if (!realloc_points(link, length))
		return NULL;

	uristring = convert_string(renderer->convert_table,
				   string, length, document->options.cp,
	                           CSM_DEFAULT, NULL, NULL, NULL);
	if (!uristring) return NULL;

	where = join_urls(renderer->base_uri, uristring);

	mem_free(uristring);

	if (!where)
		return NULL;
#ifdef CONFIG_GLOBHIST
	else if (get_global_history_item(where))
		fgcolor = document->options.default_vlink;
#endif
#ifdef CONFIG_BOOKMARKS
	else if (get_bookmark(where))
		fgcolor = document->options.default_bookmark_link;
#endif
	else
		fgcolor = document->options.default_link;

	link->npoints = length;
	link->type = LINK_HYPERTEXT;
	link->where = where;
	link->color.background = document->options.default_bg;
	link->color.foreground = fgcolor;
	link->number = document->nlinks;

	init_template(&template, &document->options,
		      link->color.background, link->color.foreground);

	render_dom_text(renderer, &template, string, length);

	for (point = link->points; length > 0; length--, point++, x++) {
		point->x = x;
		point->y = y;
	}

	document->nlinks++;

	return link;
}


/* DOM Source Renderer */

#define check_dom_node_source(renderer, str, len)	\
	((renderer)->source <= (str) && (str) + (len) <= (renderer)->end)

#define assert_source(renderer, str, len) \
	assertm(check_dom_node_source(renderer, str, len), "renderer[%p : %p] str[%p : %p]", \
		(renderer)->source, (renderer)->end, (str), (str) + (len))

static inline void
render_dom_flush(struct dom_renderer *renderer, unsigned char *string)
{
	struct screen_char *template = &renderer->styles[DOM_NODE_TEXT];
	int length = string - renderer->position;

	assert_source(renderer, renderer->position, 0);
	assert_source(renderer, string, 0);

	if (length <= 0) return;
	render_dom_text(renderer, template, renderer->position, length);
	renderer->position = string;

	assert_source(renderer, renderer->position, 0);
}

static inline void
render_dom_node_text(struct dom_renderer *renderer, struct screen_char *template,
		     struct dom_node *node)
{
	unsigned char *string = node->string.string;
	int length = node->string.length;

	if (node->type == DOM_NODE_ENTITY_REFERENCE) {
		string -= 1;
		length += 2;
	}

	if (check_dom_node_source(renderer, string, length)) {
		render_dom_flush(renderer, string);
		renderer->position = string + length;
		assert_source(renderer, renderer->position, 0);
	}

	render_dom_text(renderer, template, string, length);
}

#ifdef HAVE_REGEX_H
static inline void
render_dom_node_enhanced_text(struct dom_renderer *renderer, struct dom_node *node)
{
	regex_t *regex = &renderer->url_regex;
	regmatch_t regmatch;
	unsigned char *string = node->string.string;
	int length = node->string.length;
	struct screen_char *template = &renderer->styles[node->type];
	unsigned char *alloc_string;

	if (check_dom_node_source(renderer, string, length)) {
		render_dom_flush(renderer, string);
		renderer->position = string + length;
		assert_source(renderer, renderer->position, 0);
	}

	alloc_string = memacpy(string, length);
	if (alloc_string)
		string = alloc_string;

	while (length > 0 && !regexec(regex, string, 1, &regmatch, 0)) {
		int matchlen = regmatch.rm_eo - regmatch.rm_so;
		int offset = regmatch.rm_so;

		if (!matchlen || offset < 0 || regmatch.rm_eo > length)
			break;

		if (offset > 0)
			render_dom_text(renderer, template, string, offset);

		string += offset;
		length -= offset;

		add_dom_link(renderer, string, matchlen);

		length -= matchlen;
		string += matchlen;
	}

	if (length > 0)
		render_dom_text(renderer, template, string, length);

	mem_free_if(alloc_string);
}
#endif

static void
render_dom_node_source(struct dom_stack *stack, struct dom_node *node, void *data)
{
	struct dom_renderer *renderer = stack->current->data;

	assert(node && renderer && renderer->document);

#ifdef HAVE_REGEX_H
	if (renderer->find_url
	    && (node->type == DOM_NODE_TEXT
		|| node->type == DOM_NODE_CDATA_SECTION
		|| node->type == DOM_NODE_COMMENT)) {
		render_dom_node_enhanced_text(renderer, node);
		return;
	}
#endif

	render_dom_node_text(renderer, &renderer->styles[node->type], node);
}

/* This callback is also used for rendering processing instruction nodes.  */
static void
render_dom_element_source(struct dom_stack *stack, struct dom_node *node, void *data)
{
	struct dom_renderer *renderer = stack->current->data;

	assert(node && renderer && renderer->document);

	render_dom_node_text(renderer, &renderer->styles[node->type], node);
}

static void
render_dom_element_end_source(struct dom_stack *stack, struct dom_node *node, void *data)
{
	struct dom_renderer *renderer = stack->current->data;
	struct dom_stack_state *state = get_dom_stack_top(stack);
	struct sgml_parser_state *pstate = get_dom_stack_state_data(stack->contexts[0], state);
	struct dom_scanner_token *token = &pstate->end_token;
	unsigned char *string = token->string.string;
	int length = token->string.length;

	assert(node && renderer && renderer->document);

	if (!string || !length)
		return;

	if (check_dom_node_source(renderer, string, length)) {
		render_dom_flush(renderer, string);
		renderer->position = string + length;
		assert_source(renderer, renderer->position, 0);
	}

	render_dom_text(renderer, &renderer->styles[node->type], string, length);
}

static void
set_base_uri(struct dom_renderer *renderer, unsigned char *value, size_t valuelen)
{
	unsigned char *href = memacpy(value, valuelen);
	unsigned char *uristring;
	struct uri *uri;

	if (!href) return;
	uristring = join_urls(renderer->base_uri, href);
	mem_free(href);

	if (!uristring) return;
	uri = get_uri(uristring, 0);
	mem_free(uristring);

	if (!uri) return;

	done_uri(renderer->base_uri);
	renderer->base_uri = uri;
}

static void
render_dom_attribute_source(struct dom_stack *stack, struct dom_node *node, void *data)
{
	struct dom_renderer *renderer = stack->current->data;
	struct screen_char *template = &renderer->styles[node->type];

	assert(node && renderer->document);

	render_dom_node_text(renderer, template, node);

	if (is_dom_string_set(&node->data.attribute.value)) {
		int quoted = node->data.attribute.quoted == 1;
		unsigned char *value = node->data.attribute.value.string - quoted;
		int valuelen = node->data.attribute.value.length + quoted * 2;

		if (check_dom_node_source(renderer, value, 0)) {
			render_dom_flush(renderer, value);
			renderer->position = value + valuelen;
			assert_source(renderer, renderer->position, 0);
		}

		if (node->data.attribute.reference
		    && valuelen - quoted * 2 > 0) {
			int skips;

			/* Need to flush the first quoting delimiter and any
			 * leading whitespace so that the renderers x position
			 * is at the start of the value string. */
			for (skips = 0; skips < valuelen; skips++) {
				if ((quoted && skips == 0)
				    || isspace(value[skips])
				    || value[skips] < ' ')
					continue;

				break;
			}

			if (skips > 0) {
				render_dom_text(renderer, template, value, skips);
				value    += skips;
				valuelen -= skips;
			}

			/* Figure out what should be skipped after the actual
			 * link text. */
			for (skips = 0; skips < valuelen; skips++) {
				if ((quoted && skips == 0)
				    || isspace(value[valuelen - skips - 1])
				    || value[valuelen - skips - 1] < ' ')
					continue;

				break;
			}

			if (renderer->doctype == SGML_DOCTYPE_HTML
			    && node->data.attribute.type == HTML_ATTRIBUTE_HREF
			    && node->parent->data.element.type == HTML_ELEMENT_BASE) {
				set_base_uri(renderer, value, valuelen - skips);
			}

			add_dom_link(renderer, value, valuelen - skips);

			if (skips > 0) {
				value += valuelen - skips;
				render_dom_text(renderer, template, value, skips);
			}
		} else {
			render_dom_text(renderer, template, value, valuelen);
		}
	}
}

static void
render_dom_cdata_source(struct dom_stack *stack, struct dom_node *node, void *data)
{
	struct dom_renderer *renderer = stack->current->data;
	unsigned char *string = node->string.string;

	assert(node && renderer && renderer->document);

	/* Highlight the 'CDATA' part of <![CDATA[ if it is there. */
	if (check_dom_node_source(renderer, string - 6, 6)) {
		render_dom_flush(renderer, string - 6);
		render_dom_text(renderer, &renderer->styles[DOM_NODE_ATTRIBUTE], string - 6, 5);
		renderer->position = string - 1;
		assert_source(renderer, renderer->position, 0);
	}

	render_dom_node_text(renderer, &renderer->styles[node->type], node);
}

static void
render_dom_document_end(struct dom_stack *stack, struct dom_node *node, void *data)
{
	struct dom_renderer *renderer = stack->current->data;

	/* If there are no non-element nodes after the last element node make
	 * sure that we flush to the end of the cache entry source including
	 * the '>' of the last element tag if it has one. (bug 519) */
	if (check_dom_node_source(renderer, renderer->position, 0)) {
		render_dom_flush(renderer, renderer->end);
	}
}

static struct dom_stack_context_info dom_source_renderer_context_info = {
	/* Object size: */			0,
	/* Push: */
	{
		/*				*/ NULL,
		/* DOM_NODE_ELEMENT		*/ render_dom_element_source,
		/* DOM_NODE_ATTRIBUTE		*/ render_dom_attribute_source,
		/* DOM_NODE_TEXT		*/ render_dom_node_source,
		/* DOM_NODE_CDATA_SECTION	*/ render_dom_cdata_source,
		/* DOM_NODE_ENTITY_REFERENCE	*/ render_dom_node_source,
		/* DOM_NODE_ENTITY		*/ render_dom_node_source,
		/* DOM_NODE_PROC_INSTRUCTION	*/ render_dom_element_source,
		/* DOM_NODE_COMMENT		*/ render_dom_node_source,
		/* DOM_NODE_DOCUMENT		*/ NULL,
		/* DOM_NODE_DOCUMENT_TYPE	*/ render_dom_node_source,
		/* DOM_NODE_DOCUMENT_FRAGMENT	*/ render_dom_node_source,
		/* DOM_NODE_NOTATION		*/ render_dom_node_source,
	},
	/* Pop: */
	{
		/*				*/ NULL,
		/* DOM_NODE_ELEMENT		*/ render_dom_element_end_source,
		/* DOM_NODE_ATTRIBUTE		*/ NULL,
		/* DOM_NODE_TEXT		*/ NULL,
		/* DOM_NODE_CDATA_SECTION	*/ NULL,
		/* DOM_NODE_ENTITY_REFERENCE	*/ NULL,
		/* DOM_NODE_ENTITY		*/ NULL,
		/* DOM_NODE_PROC_INSTRUCTION	*/ NULL,
		/* DOM_NODE_COMMENT		*/ NULL,
		/* DOM_NODE_DOCUMENT		*/ render_dom_document_end,
		/* DOM_NODE_DOCUMENT_TYPE	*/ NULL,
		/* DOM_NODE_DOCUMENT_FRAGMENT	*/ NULL,
		/* DOM_NODE_NOTATION		*/ NULL,
	}
};


/* Shared multiplexor between renderers */
void
render_dom_document(struct cache_entry *cached, struct document *document,
		    struct string *buffer)
{
	unsigned char *head = empty_string_or_(cached->head);
	struct dom_node *root;
	struct dom_renderer renderer;
	struct conv_table *convert_table;
	struct sgml_parser *parser;
	unsigned char *string = struri(cached->uri);
	size_t length = strlen(string);
	struct dom_string uri = INIT_DOM_STRING(string, length);
	struct dom_string source = INIT_DOM_STRING(buffer->source, buffer->length);

	assert(document->options.plain);

	convert_table = get_convert_table(head, document->options.cp,
					  document->options.assume_cp,
					  &document->cp,
					  &document->cp_status,
					  document->options.hard_assume);

	init_dom_renderer(&renderer, document, buffer, convert_table);

	document->bgcolor = document->options.default_bg;

	if (!c_strcasecmp("application/rss+xml", cached->content_type)) {
		renderer.doctype = SGML_DOCTYPE_RSS;

	} else if (!c_strcasecmp("application/xbel+xml", cached->content_type)
		   || !c_strcasecmp("application/x-xbel", cached->content_type)
		   || !c_strcasecmp("application/xbel", cached->content_type)) {
		renderer.doctype = SGML_DOCTYPE_XBEL;

	} else {
		assertm(!c_strcasecmp("text/html", cached->content_type)
			|| !c_strcasecmp("application/xhtml+xml", cached->content_type),
			"Couldn't resolve doctype '%s'", cached->content_type);

		renderer.doctype = SGML_DOCTYPE_HTML;
	}

	parser = init_sgml_parser(SGML_PARSER_STREAM, renderer.doctype, &uri);
	if (!parser) return;

	add_dom_stack_context(&parser->stack, &renderer,
			      &dom_source_renderer_context_info);

	root = parse_sgml(parser, &source);
	if (root) {
		assert(parser->stack.depth == 1);

		get_dom_stack_top(&parser->stack)->immutable = 0;
		/* For SGML_PARSER_STREAM this will free the DOM
		 * root node. */
		pop_dom_node(&parser->stack);
	}

#ifdef HAVE_REGEX_H
	if (renderer.find_url)
		regfree(&renderer.url_regex);
#endif
	done_uri(renderer.base_uri);
	done_sgml_parser(parser);
}
