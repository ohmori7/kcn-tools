/* Tab-style (those containing real documents) windows infrastructure. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "elinks.h"

#include "bfu/dialog.h"
#include "config/options.h"
#include "dialogs/menu.h"
#include "document/document.h"
#include "document/view.h"
#include "intl/gettext/libintl.h"
#include "main/select.h"
#include "protocol/uri.h"
#include "session/session.h"
#include "terminal/screen.h"
#include "terminal/tab.h"
#include "terminal/terminal.h"
#include "terminal/window.h"
#include "util/error.h"
#include "util/memory.h"
#include "util/lists.h"
#include "viewer/text/link.h"
#include "viewer/text/view.h"


struct window *
init_tab(struct terminal *term, void *data, window_handler_T handler)
{
	struct window *win = mem_calloc(1, sizeof(*win));

	if (!win) return NULL;

	win->handler = handler;
	win->term = term;
	win->data = data;
	win->type = WINDOW_TAB;
	win->resize = 1;

	add_to_list(term->windows, win);

	return win;
}

/* Number of tabs at the terminal (in term->windows) */
inline int
number_of_tabs(struct terminal *term)
{
	int result = 0;
	struct window *win;

	foreach_tab (win, term->windows)
		result++;

	return result;
}

/* Number of tab */
int
get_tab_number(struct window *window)
{
	struct terminal *term = window->term;
	struct window *win;
	int current = 0;
	int num = 0;

	foreachback_tab (win, term->windows) {
		if (win == window) {
			num = current;
			break;
		}
		current++;
	}

	return num;
}

/* Get tab of an according index */
struct window *
get_tab_by_number(struct terminal *term, int num)
{
	struct window *win = NULL;

	foreachback_tab (win, term->windows) {
		if (!num) break;
		num--;
	}

	return win;
}

/* Returns number of the tab at @xpos, or -1 if none. */
int
get_tab_number_by_xpos(struct terminal *term, int xpos)
{
	int num = 0;
	struct window *win = NULL;

	foreachback_tab (win, term->windows) {
		if (xpos >= win->xpos
		    && xpos < win->xpos + win->width)
			return num;
		num++;
	}

	return -1;
}

/* if @tabs > 0, then it is taken as the result of a recent
 * call to number_of_tabs() so it just uses this value. */
void
switch_to_tab(struct terminal *term, int tab, int tabs)
{
	if (tabs < 0) tabs = number_of_tabs(term);

	if (tabs > 1) {
		if (tab >= tabs) {
			if (get_opt_bool("ui.tabs.wraparound"))
				tab = 0;
			else
				tab = tabs - 1;
		}

		if (tab < 0) {
			if (get_opt_bool("ui.tabs.wraparound"))
				tab = tabs - 1;
			else
				tab = 0;
		}
	} else tab = 0;

	if (tab != term->current_tab) {
		term->current_tab = tab;
		set_screen_dirty(term->screen, 0, term->height);
		redraw_terminal(term);
	}
}

void
switch_current_tab(struct session *ses, int direction)
{
	struct terminal *term = ses->tab->term;
	int num_tabs = number_of_tabs(term);
	int count;

	if (num_tabs < 2)
		return;

	count = eat_kbd_repeat_count(ses);
	if (count) direction *= count;

	switch_to_tab(term, term->current_tab + direction, num_tabs);
}

static void
really_close_tab(struct session *ses)
{
	struct terminal *term = ses->tab->term;
	struct window *current_tab = get_current_tab(term);

	if (ses->tab == current_tab) {
		int num_tabs = number_of_tabs(term);

		switch_to_tab(term, term->current_tab - 1, num_tabs - 1);
	}

	delete_window(ses->tab);
}

void
close_tab(struct terminal *term, struct session *ses)
{
	int num_tabs = number_of_tabs(term);

	if (num_tabs < 2) {
		query_exit(ses);
		return;
	}

	if (!get_opt_bool("ui.tabs.confirm_close")) {
		really_close_tab(ses);
		return;
	}

	msg_box(term, NULL, 0,
		N_("Close tab"), ALIGN_CENTER,
		N_("Do you really want to close the current tab?"),
		ses, 2,
		N_("~Yes"), (void (*)(void *)) really_close_tab, B_ENTER,
		N_("~No"), NULL, B_ESC);
}

static void
really_close_tabs(struct session *ses)
{
	struct terminal *term = ses->tab->term;
	struct window *current = get_current_tab(term);
	struct window *tab;

	foreach_tab (tab, term->windows) {
		if (tab == current) continue;
		tab = tab->prev;
		delete_window(tab->next);
	}

	term->current_tab = 0;
	redraw_terminal(term);
}

void
close_all_tabs_but_current(struct session *ses)
{
	assert(ses);
	if_assert_failed return;

	if (!get_opt_bool("ui.tabs.confirm_close")) {
		really_close_tabs(ses);
		return;
	}

	msg_box(ses->tab->term, NULL, 0,
		N_("Close tab"), ALIGN_CENTER,
		N_("Do you really want to close all except the current tab?"),
		ses, 2,
		N_("~Yes"), (void (*)(void *)) really_close_tabs, B_ENTER,
		N_("~No"), NULL, B_ESC);
}


void
open_uri_in_new_tab(struct session *ses, struct uri *uri, int in_background,
                    int based)
{
	assert(ses);
	/* @based means whether the current @ses location will be preloaded
	 * in the tab. */
	init_session(based ? ses : NULL, ses->tab->term, uri, in_background);
}

void
open_current_link_in_new_tab(struct session *ses, int in_background)
{
	struct document_view *doc_view = current_frame(ses);
	struct uri *uri = NULL;
	struct link *link;

	if (doc_view) assert(doc_view->vs && doc_view->document);
	if_assert_failed return;

	link = get_current_link(doc_view);
	if (link) uri = get_link_uri(ses, doc_view, link);

	open_uri_in_new_tab(ses, uri, in_background, 1);
	if (uri) done_uri(uri);
}

void
move_current_tab(struct session *ses, int direction)
{
	struct terminal *term = ses->tab->term;
	int tabs = number_of_tabs(term);
	struct window *current_tab = get_current_tab(term);
	struct window *tab;
	int new_pos;
	int count;

	assert(ses && direction);

	count = eat_kbd_repeat_count(ses);
	if (count) direction *= count;

	new_pos = term->current_tab + direction;

	while (new_pos < 1 || new_pos > tabs)
		new_pos += new_pos < 1 ? tabs : -tabs;

	assert(0 < new_pos && new_pos <= tabs);

	if (new_pos == term->current_tab) return;

	tab = get_tab_by_number(term, new_pos);

	del_from_list(current_tab);

	if (new_pos < term->current_tab) {
		add_at_pos(tab, current_tab);
	} else {
		add_to_list_end(*tab, current_tab);
	}

	switch_to_tab(term, new_pos, tabs);
}
