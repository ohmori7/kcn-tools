/* Event system support routines. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "elinks.h"

#include "intl/gettext/libintl.h"
#include "main/main.h"			/* terminate */
#include "main/object.h"
#include "session/session.h"
#include "terminal/draw.h"
#include "terminal/event.h"
#include "terminal/kbd.h"
#include "terminal/mouse.h"
#include "terminal/tab.h"
#include "terminal/terminal.h"
#include "terminal/screen.h"
#include "terminal/window.h"
#include "util/conv.h"
#include "util/error.h"
#include "util/memory.h"
#include "util/snprintf.h"
#include "util/string.h"
#include "viewer/timer.h"


/* Information used for communication between ELinks instances */
struct terminal_interlink {
	/* How big the input queue is and how much is free */
	int qlen;
	int qfreespace;

	/* UTF8 input key value decoding data. */
	struct {
		unicode_val_T ucs;
		int len;
		int min;
	} utf_8;

	/* This is the queue of events as coming from the other ELinks instance
	 * owning the hosting terminal. */
	unsigned char input_queue[1];
};


void
term_send_event(struct terminal *term, struct term_event *ev)
{
	struct window *win;

	assert(ev && term);
	if_assert_failed return;

	switch (ev->ev) {
	case EVENT_INIT:
	case EVENT_RESIZE:
	{
		int width = ev->info.size.width;
		int height = ev->info.size.height;

		if (width < 0 || height < 0) {
			ERROR(gettext("Bad terminal size: %d, %d"),
			      width, height);
			break;
		}

		resize_screen(term, width, height);
		erase_screen(term);
		/* Fall through */
	}
	case EVENT_REDRAW:
		/* Nasty hack to avoid assertion failures when doing -remote
		 * stuff and the client exits right away */
		if (!term->screen->image) break;

		clear_terminal(term);
		term->redrawing = TREDRAW_DELAYED;
		/* Note that you do NOT want to ever go and create new
		 * window inside EVENT_INIT handler (it'll get second
		 * EVENT_INIT here). Perhaps the best thing you could do
		 * is registering a bottom-half handler which will open
		 * additional windows.
		 * --pasky */
		if (ev->ev == EVENT_RESIZE) {
			/* We want to propagate EVENT_RESIZE even to inactive
			 * tabs! Nothing wrong will get drawn (in the final
			 * result) as the active tab is always the first one,
			 * thus will be drawn last here. Thanks, Witek!
			 * --pasky */
			foreachback (win, term->windows)
				win->handler(win, ev);

		} else {
			foreachback (win, term->windows)
				if (!inactive_tab(win))
					win->handler(win, ev);
		}
		term->redrawing = TREDRAW_READY;
		break;

	case EVENT_MOUSE:
	case EVENT_KBD:
	case EVENT_ABORT:
		assert(!list_empty(term->windows));
		if_assert_failed break;

		/* We need to send event to correct tab, not to the first one. --karpov */
		/* ...if we want to send it to a tab at all. --pasky */
		win = term->windows.next;
		if (win->type == WINDOW_TAB) {
			win = get_current_tab(term);
			assertm(win, "No tab to send the event to!");
			if_assert_failed return;
		}

		win->handler(win, ev);
	}
}

static void
term_send_ucs(struct terminal *term, struct term_event *ev, unicode_val_T u)
{
	unsigned char *recoded;

	recoded = u2cp_no_nbsp(u, get_opt_codepage_tree(term->spec, "charset"));
	if (!recoded) recoded = "*";
	while (*recoded) {
		ev->info.keyboard.key = *recoded;
		term_send_event(term, ev);
		recoded++;
	}
}

static void
check_terminal_name(struct terminal *term, struct terminal_info *info)
{
	unsigned char name[MAX_TERM_LEN + 10];
	int i;

	/* We check TERM env. var for sanity, and fallback to _template_ if
	 * needed. This way we prevent elinks.conf potential corruption. */
	for (i = 0; info->name[i]; i++) {
		if (isident(info->name[i])) continue;

		usrerror(_("Warning: terminal name contains illicit chars.", term));
		return;
	}

	snprintf(name, sizeof(name), "terminal.%s", info->name);

	/* Unlock the default _template_ option tree that was asigned by
	 * init_term() and get the correct one. */
	object_unlock(term->spec);
	term->spec = get_opt_rec(config_options, name);
	object_lock(term->spec);
}

#ifdef CONFIG_MOUSE
static int
ignore_mouse_event(struct terminal *term, struct term_event *ev)
{
	struct term_event_mouse *prev = &term->prev_mouse_event;
	struct term_event_mouse *current = &ev->info.mouse;

	if (check_mouse_action(ev, B_UP)
	    && current->y == prev->y
	    && (current->button & ~BM_ACT) == (prev->button & ~BM_ACT)) {
		do_not_ignore_next_mouse_event(term);

		return 1;
	}

	copy_struct(prev, current);

	return 0;
}
#endif

static int
handle_interlink_event(struct terminal *term, struct term_event *ev)
{
	struct terminal_info *info = NULL;
	struct terminal_interlink *interlink = term->interlink;

	switch (ev->ev) {
	case EVENT_INIT:
		if (interlink->qlen < TERMINAL_INFO_SIZE)
			return 0;

		info = (struct terminal_info *) ev;

		if (interlink->qlen < TERMINAL_INFO_SIZE + info->length)
			return 0;

		info->name[MAX_TERM_LEN - 1] = 0;
		check_terminal_name(term, info);

		memcpy(term->cwd, info->cwd, MAX_CWD_LEN);
		term->cwd[MAX_CWD_LEN - 1] = 0;

		term->environment = info->system_env;

		/* We need to make sure that it is possible to draw on the
		 * terminal screen before decoding the session info so that
		 * handling of bad URL syntax by openning msg_box() will be
		 * possible. */
		term_send_event(term, ev);

		/* Either the initialization of the first session failed or we
		 * are doing a remote session so quit.*/
		if (!decode_session_info(term, info)) {
			destroy_terminal(term);
			/* Make sure the user is notified if the initialization
			 * of the first session fails. */
			if (program.terminate) {
				usrerror(_("Failed to create session.", term));
				program.retval = RET_FATAL;
			}
			return 0;
		}

		ev->ev = EVENT_REDRAW;
		/* Fall through */
	case EVENT_REDRAW:
	case EVENT_RESIZE:
		term_send_event(term, ev);
		break;

	case EVENT_MOUSE:
#ifdef CONFIG_MOUSE
		reset_timer();
		if (!ignore_mouse_event(term, ev))
			term_send_event(term, ev);
#endif
		break;

	case EVENT_KBD:
	{
		int utf8_io = -1;
		int key = get_kbd_key(ev);

		reset_timer();

		if (check_kbd_modifier(ev, KBD_MOD_CTRL) && c_toupper(key) == 'L') {
			redraw_terminal_cls(term);
			break;

		} else if (key == KBD_CTRL_C) {
			destroy_terminal(term);
			return 0;
		}

		if (interlink->utf_8.len) {
			utf8_io = get_opt_bool_tree(term->spec, "utf_8_io");

			if ((key & 0xC0) == 0x80 && utf8_io) {
				interlink->utf_8.ucs <<= 6;
				interlink->utf_8.ucs |= key & 0x3F;
				if (! --interlink->utf_8.len) {
					unicode_val_T u = interlink->utf_8.ucs;

					if (u < interlink->utf_8.min)
						u = UCS_NO_CHAR;
					term_send_ucs(term, ev, u);
				}
				break;

			} else {
				interlink->utf_8.len = 0;
				term_send_ucs(term, ev, UCS_NO_CHAR);
			}
		}

		if (key < 0x80 || key > 0xFF
		    || (utf8_io == -1
			? !get_opt_bool_tree(term->spec, "utf_8_io")
			: !utf8_io)) {

			term_send_event(term, ev);
			break;

		} else if ((key & 0xC0) == 0xC0 && (key & 0xFE) != 0xFE) {
			unsigned int mask, cov = 0x80;
			int len = 0;

			for (mask = 0x80; key & mask; mask >>= 1) {
				len++;
				interlink->utf_8.min = cov;
				cov = 1 << (1 + 5 * len);
			}

			interlink->utf_8.len = len - 1;
			interlink->utf_8.ucs = key & (mask - 1);
			break;
		}

		term_send_ucs(term, ev, UCS_NO_CHAR);
		break;
	}

	case EVENT_ABORT:
		destroy_terminal(term);
		return 0;

	default:
		ERROR(gettext("Bad event %d"), ev->ev);
	}

	/* For EVENT_INIT we read a liitle more */
	if (info) return TERMINAL_INFO_SIZE + info->length;
	return sizeof(*ev);
}

void
in_term(struct terminal *term)
{
	struct terminal_interlink *interlink = term->interlink;
	ssize_t r;
	unsigned char *iq;

	if (!interlink
	    || !interlink->qfreespace
	    || interlink->qfreespace - interlink->qlen > ALLOC_GR) {
		int qlen = interlink ? interlink->qlen : 0;
		int queuesize = ((qlen + ALLOC_GR) & ~(ALLOC_GR - 1));
		int newsize = sizeof(*interlink) + queuesize;

		interlink = mem_realloc(interlink, newsize);
		if (!interlink) {
			destroy_terminal(term);
			return;
		}

		/* Blank the members for the first allocation */
		if (!term->interlink)
			memset(interlink, 0, sizeof(*interlink));

		term->interlink = interlink;
		interlink->qfreespace = queuesize - interlink->qlen;
	}

	iq = interlink->input_queue;
	r = safe_read(term->fdin, iq + interlink->qlen, interlink->qfreespace);
	if (r <= 0) {
		if (r == -1 && errno != ECONNRESET)
			ERROR(gettext("Could not read event: %d (%s)"),
			      errno, (unsigned char *) strerror(errno));

		destroy_terminal(term);
		return;
	}

	interlink->qlen += r;
	interlink->qfreespace -= r;

	while (interlink->qlen >= sizeof(struct term_event)) {
		struct term_event *ev = (struct term_event *) iq;
		int event_size = handle_interlink_event(term, ev);

		/* If the event was not handled save the bytes in the queue for
		 * later in case more stuff is read later. */
		if (!event_size) break;

		/* Acount for the handled bytes */
		interlink->qlen -= event_size;
		interlink->qfreespace += event_size;

		/* If there are no more bytes to handle stop else move next
		 * event bytes to the front of the queue. */
		if (!interlink->qlen) break;
		memmove(iq, iq + event_size, interlink->qlen);
	}
}