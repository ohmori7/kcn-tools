
#ifndef EL__VIEWER_TEXT_VS_H
#define EL__VIEWER_TEXT_VS_H

#include "util/lists.h"

/* Crossdeps are evil. */
struct document_view;
struct form_state;
struct session;
struct uri;

struct view_state {
	struct document_view *doc_view;
	struct uri *uri;

	struct list_head forms; /* -> struct form_view */
	struct form_state *form_info;
	int form_info_len;

	int x, y;
	int current_link;

	int plain;
	unsigned int wrap:1;
	unsigned int did_fragment:1;

#ifdef CONFIG_ECMASCRIPT
	/* If set, we reset the interpreter state the next time we are going to
	 * render document attached to this view state. This means a real
	 * document (not just struct document_view, which randomly appears and
	 * disappears during gradual rendering) is getting replaced. So set this
	 * always when you replace the view_state URI, but also when reloading
	 * the document. You also cannot reset the document right away because
	 * it might take some time before the first rerendering is done and
	 * until then the old document is still hanging there. */
	unsigned int ecmascript_fragile:1;
	struct ecmascript_interpreter *ecmascript;
#endif
};

void init_vs(struct view_state *, struct uri *uri, int);
void destroy_vs(struct view_state *, int blast_ecmascript);

void copy_vs(struct view_state *, struct view_state *);
void check_vs(struct document_view *);

void next_frame(struct session *, int);

#endif
