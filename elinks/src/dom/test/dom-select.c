/* Tool for testing the SGML parser */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "elinks.h"

#include "dom/node.h"
#include "dom/select.h"
#include "dom/sgml/parser.h"
#include "dom/stack.h"


void die(const char *msg, ...)
{
	va_list args;

	if (msg) {
		va_start(args, msg);
		vfprintf(stderr, msg, args);
		fputs("\n", stderr);
		va_end(args);
	}

	exit(!!NULL);
}

int
main(int argc, char *argv[])
{
	struct dom_node *root;
	struct sgml_parser *parser;
	struct dom_select *select;
	enum sgml_document_type doctype = SGML_DOCTYPE_HTML;
	struct dom_string uri = INIT_DOM_STRING("dom://test", -1);
	struct dom_string source = INIT_DOM_STRING("(no source)", -1);
	struct dom_string selector = INIT_DOM_STRING("(no select)", -1);
	int i;

	for (i = 1; i < argc; i++) {
		char *arg = argv[i];

		if (strncmp(arg, "--", 2))
			break;

		arg += 2;

		if (!strncmp(arg, "uri", 3)) {
			arg += 3;
			if (*arg == '=') {
				arg++;
				set_dom_string(&uri, arg, strlen(arg));
			} else {
				i++;
				if (i >= argc)
					die("--uri expects a URI");
				set_dom_string(&uri, argv[i], strlen(argv[i]));
			}

		} else if (!strncmp(arg, "src", 3)) {
			arg += 3;
			if (*arg == '=') {
				arg++;
				set_dom_string(&source, arg, strlen(arg));
			} else {
				i++;
				if (i >= argc)
					die("--src expects a string");
				set_dom_string(&source, argv[i], strlen(argv[i]));
			}

		} else if (!strncmp(arg, "selector", 3)) {
			arg += 8;
			if (*arg == '=') {
				arg++;
				set_dom_string(&selector, arg, strlen(arg));
			} else {
				i++;
				if (i >= argc)
					die("--selector expects a string");
				set_dom_string(&selector, argv[i], strlen(argv[i]));
			}

		} else if (!strcmp(arg, "help")) {
			die(NULL);

		} else {
			die("Unknown argument '%s'", arg - 2);
		}
	}

	parser = init_sgml_parser(SGML_PARSER_TREE, doctype, &uri);
	if (!parser) return 1;

	add_dom_stack_tracer(&parser->stack, "sgml-parse: ");

	root = parse_sgml(parser, &source);
	done_sgml_parser(parser);

	if (!root) die("No root node");

	select = init_dom_select(DOM_SELECT_SYNTAX_CSS, &selector);
	if (select) {
		struct dom_node_list *list;

		list = select_dom_nodes(select, root);
		if (list) {
			struct dom_node *node;
			int index;

			foreach_dom_node (list, node, index) {
				DBG("Match");
			}

			mem_free(list);
		}

		done_dom_select(select);
	}

	done_dom_node(root);

	return 0;
}
