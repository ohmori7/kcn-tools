/* Links viewing/manipulation handling */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include "elinks.h"

#include "bfu/listmenu.h"
#include "bfu/menu.h"
#include "dialogs/menu.h"
#include "dialogs/status.h"
#include "document/document.h"
#include "document/forms.h"
#include "document/html/parser.h"
#include "document/html/renderer.h"
#include "document/options.h"
#include "document/view.h"
#include "ecmascript/ecmascript.h"
#include "intl/gettext/libintl.h"
#include "main/object.h"
#include "protocol/uri.h"
#include "session/session.h"
#include "session/task.h"
#include "terminal/color.h"
#include "terminal/draw.h"
#include "terminal/kbd.h"
#include "terminal/screen.h"
#include "terminal/tab.h"
#include "terminal/terminal.h"
#include "util/conv.h"
#include "util/error.h"
#include "util/memory.h"
#include "util/string.h"
#include "viewer/action.h"
#include "viewer/text/form.h"
#include "viewer/text/link.h"
#include "viewer/text/search.h"
#include "viewer/text/textarea.h"
#include "viewer/text/view.h"
#include "viewer/text/vs.h"


/* Perhaps some of these would be more fun to have in viewer/common/, dunno.
 * --pasky */


static int
current_link_evhook(struct document_view *doc_view, enum script_event_hook_type type)
{
#ifdef CONFIG_ECMASCRIPT
	struct link *link;
	struct script_event_hook *evhook;

	assert(doc_view && doc_view->vs);
	link = get_current_link(doc_view);
	if (!link) return -1;
	if (!link->event_hooks) return -1;

	if (!doc_view->vs->ecmascript) return -1;

	foreach (evhook, *link->event_hooks) {
		struct string src = INIT_STRING(evhook->src, strlen(evhook->src));

		if (evhook->type != type) continue;
		/* TODO: Some even handlers return a bool. */
		if (!ecmascript_eval_boolback(doc_view->vs->ecmascript, &src))
			return 0;
	}

	return 1;
#else
	return -1;
#endif
}

#define current_link_hover(dv) \
do { \
	current_link_evhook(dv, SEVHOOK_ONMOUSEOVER); \
	current_link_evhook(dv, SEVHOOK_ONHOVER); \
	current_link_evhook(dv, SEVHOOK_ONFOCUS); \
} while (0)
#define current_link_blur(dv) \
do { \
	current_link_evhook(dv, SEVHOOK_ONMOUSEOUT); \
	current_link_evhook(dv, SEVHOOK_ONBLUR); \
} while (0)


void
set_link(struct document_view *doc_view)
{
	assert(doc_view);
	if_assert_failed return;

	if (current_link_is_visible(doc_view)) return;

	find_link_page_down(doc_view);
}

static inline int
get_link_cursor_offset(struct document_view *doc_view, struct link *link)
{
	struct form_control *fc;
	struct form_state *fs;

	switch (link->type) {
		case LINK_CHECKBOX:
			return 1;

		case LINK_BUTTON:
			return 2;

		case LINK_FIELD:
			fc = get_link_form_control(link);
			fs = find_form_state(doc_view, fc);
			return fs ? fs->state - fs->vpos : 0;

		case LINK_AREA:
			fc = get_link_form_control(link);
			fs = find_form_state(doc_view, fc);
			return fs ? area_cursor(fc, fs) : 0;

		case LINK_HYPERTEXT:
		case LINK_MAP:
		case LINK_SELECT:
			return 0;
	}

	return 0;
}

/* Allocate doc_view->link_bg with enough space to save the colour
 * and attributes of each point of the given link plus one byte
 * for the template character. Initialise that template character
 * with the colour and attributes appropriate for an active link. */
static inline struct screen_char *
init_link_drawing(struct document_view *doc_view, struct link *link, int invert)
{
	struct document_options *doc_opts;
	struct screen_char *template;
	enum color_flags color_flags;
	enum color_mode color_mode;
	struct color_pair colors;

	/* Allocate an extra background char to work on here. */
	doc_view->link_bg = mem_alloc((1 + link->npoints) * sizeof(*doc_view->link_bg));
	if (!doc_view->link_bg) return NULL;

	doc_view->link_bg_n = link->npoints;

	/* Setup the template char. */
	template = &doc_view->link_bg[link->npoints].c;

	template->attr = SCREEN_ATTR_STANDOUT;

	doc_opts = &doc_view->document->options;

	color_flags = (doc_opts->color_flags | COLOR_DECREASE_LIGHTNESS);
	color_mode = doc_opts->color_mode;

	if (doc_opts->active_link.underline)
		template->attr |= SCREEN_ATTR_UNDERLINE;

	if (doc_opts->active_link.bold)
		template->attr |= SCREEN_ATTR_BOLD;

	if (doc_opts->active_link.color) {
		colors.foreground = doc_opts->active_link.fg;
		colors.background = doc_opts->active_link.bg;
	} else {
		colors.foreground = link->color.foreground;
		colors.background = link->color.background;
	}

	if (invert && doc_opts->active_link.invert) {
		swap_values(color_T, colors.foreground, colors.background);

		/* Highlight text-input form-fields correctly if contrast
		 * correction is needed. */
		if (link_is_textinput(link)) {
			/* Make sure to use the options belonging to the
			 * current document when checking for fg and bg color
			 * usage. */
			doc_opts = &doc_view->document->options;

			/* Are we fallen angels who didn't want to believe that
			 * nothing /is/ nothing and so were born to lose our
			 * loved ones and dear friends one by one and finally
			 * our own life, to see it proved? --Kerouac */

			/* Wipe out all default correction for 16 color mode */
			color_flags = (color_flags & ~COLOR_INCREASE_CONTRAST);
			/* Make contrast correction invert things properly */
			color_flags |= COLOR_ENSURE_INVERTED_CONTRAST;
		}
	}

	set_term_color(template, &colors, color_flags, color_mode);

	return template;
}

/* Save the current link's colours and attributes to doc_view->link_bg
 * and give it the appropriate colour and attributes for an active link. */
void
draw_current_link(struct session *ses, struct document_view *doc_view)
{
	struct terminal *term = ses->tab->term;
	struct screen_char *template;
	struct link *link;
	int cursor_offset;
	int xpos, ypos;
	int i;

	assert(term && doc_view && doc_view->vs);
	if_assert_failed return;

	assert(ses->tab == get_current_tab(term));
	if_assert_failed return;

	assertm(!doc_view->link_bg, "link background not empty");
	if_assert_failed mem_free(doc_view->link_bg);

	link = get_current_link(doc_view);
	if (!link) return;

	i = !link_is_textinput(link) || ses->insert_mode == INSERT_MODE_OFF;
	template = init_link_drawing(doc_view, link, i);
	if (!template) return;

	xpos = doc_view->box.x - doc_view->vs->x;
	ypos = doc_view->box.y - doc_view->vs->y;

	if (ses->insert_mode == INSERT_MODE_OFF
	    && ses->navigate_mode == NAVIGATE_CURSOR_ROUTING) {
		/* If we are navigating using cursor routing and not editing a
		 * text-input form-field never set the cursor. */
		cursor_offset = -1;
	} else {
		cursor_offset = get_link_cursor_offset(doc_view, link);
	}

	for (i = 0; i < link->npoints; i++) {
		int x = link->points[i].x + xpos;
		int y = link->points[i].y + ypos;
		struct screen_char *co;

		if (!is_in_box(&doc_view->box, x, y)) {
			doc_view->link_bg[i].x = -1;
			doc_view->link_bg[i].y = -1;
			continue;
		}

		doc_view->link_bg[i].x = x;
		doc_view->link_bg[i].y = y;

		co = get_char(term, x, y);
		copy_screen_chars(&doc_view->link_bg[i].c, co, 1);

		if (i == cursor_offset) {
			int blockable = (!link_is_textinput(link)
					 && co->color != template->color);

			set_cursor(term, x, y, blockable);
			set_window_ptr(ses->tab, x, y);
		}

 		template->data = co->data;
 		copy_screen_chars(co, template, 1);
		set_screen_dirty(term->screen, y, y);
	}
}

void
free_link(struct document_view *doc_view)
{
	assert(doc_view);
	if_assert_failed return;

	mem_free_set(&doc_view->link_bg, NULL);
	doc_view->link_bg_n = 0;
}

/* Restore the colours and attributes that the active link had
 * before it was selected. */
void
clear_link(struct terminal *term, struct document_view *doc_view)
{
	assert(term && doc_view);
	if_assert_failed return;

	if (doc_view->link_bg) {
		struct link_bg *link_bg = doc_view->link_bg;
		int i;

		for (i = doc_view->link_bg_n - 1; i >= 0; i--) {
			struct link_bg *bgchar = &link_bg[i];

			if (bgchar->x != -1 && bgchar->y != -1) {
				struct terminal_screen *screen = term->screen;
				struct screen_char *co;

				co = get_char(term, bgchar->x, bgchar->y);
				copy_screen_chars(co, &bgchar->c, 1);
				set_screen_dirty(screen, bgchar->y, bgchar->y);
			}
		}

		free_link(doc_view);
	}
}

struct link *
get_first_link(struct document_view *doc_view)
{
	struct link *link, *undef;
	struct document *document;
	int height;
	int i;

	assert(doc_view && doc_view->document);
	if_assert_failed return NULL;

	document = doc_view->document;

	if (!document->lines1) return NULL;

	height = doc_view->vs->y + doc_view->box.height;
	link = undef = document->links + document->nlinks;

	for (i = int_max(0, doc_view->vs->y);
	     i < int_min(height, document->height);
	     i++) {
		if (document->lines1[i]
		    && document->lines1[i] < link)
			link = document->lines1[i];
	}

	return (link == undef) ? NULL : link;
}

struct link *
get_last_link(struct document_view *doc_view)
{
	struct link *link = NULL;
	struct document *document;
	int height;
	int i;

	assert(doc_view && doc_view->document);
	if_assert_failed return NULL;

	document = doc_view->document;

	if (!document->lines2) return NULL;

	height = doc_view->vs->y + doc_view->box.height;

	for (i = int_max(0, doc_view->vs->y);
	     i < int_min(height, document->height);
	     i++)
		if (document->lines2[i] > link)
			link = document->lines2[i];

	return link;
}

static int
link_in_view_x(struct document_view *doc_view, struct link *link)
{
	int i, dx, width;

	assert(doc_view && link);
	if_assert_failed return 0;

	dx = doc_view->vs->x;
	width = doc_view->box.width;

	for (i = 0; i < link->npoints; i++) {
		int x = link->points[i].x - dx;

		if (x >= 0 && x < width)
			return 1;
	}

	return 0;
}

static int
link_in_view_y(struct document_view *doc_view, struct link *link)
{
	int i, dy, height;

	assert(doc_view && link);
	if_assert_failed return 0;

	dy = doc_view->vs->y;
	height = doc_view->box.height;

	for (i = 0; i < link->npoints; i++) {
		int y = link->points[i].y - dy;

		if (y >= 0 && y < height)
			return 1;
	}

	return 0;
}

static int
link_in_view(struct document_view *doc_view, struct link *link)
{
	assert(doc_view && link);
	if_assert_failed return 0;
	return link_in_view_y(doc_view, link) && link_in_view_x(doc_view, link);
}

int
current_link_is_visible(struct document_view *doc_view)
{
	struct link *link;

	assert(doc_view && doc_view->vs);
	if_assert_failed return 0;

	link = get_current_link(doc_view);
	return (link && link_in_view(doc_view, link));
}

/* Look for the first and the last link currently visible in our
 * viewport. */
static void
get_visible_links_range(struct document_view *doc_view, int *first, int *last)
{
	struct document *document = doc_view->document;
	int height = int_min(doc_view->vs->y + doc_view->box.height,
	                     document->height);
	int y;

	*first = document->nlinks - 1;
	*last = 0;

	for (y = int_max(0, doc_view->vs->y); y < height; y++) {
		if (document->lines1[y])
			int_upper_bound(first, document->lines1[y]
					       - document->links);

		if (document->lines2[y])
			int_lower_bound(last, document->lines2[y]
					      - document->links);
	}
}

static int
next_link_in_view_(struct document_view *doc_view, int current, int direction,
	           int (*fn)(struct document_view *, struct link *),
	           void (*cntr)(struct document_view *, struct link *))
{
	struct document *document;
	struct view_state *vs;
	int start, end;

	assert(doc_view && doc_view->document && doc_view->vs && fn);
	if_assert_failed return 0;

	document = doc_view->document;
	vs = doc_view->vs;

	get_visible_links_range(doc_view, &start, &end);

	current_link_blur(doc_view);

	/* Go from the @current link in @direction until either
	 * fn() is happy or we would leave the current viewport. */
	while (current >= start && current <= end) {
		if (fn(doc_view, &document->links[current])) {
			vs->current_link = current;
			if (cntr) cntr(doc_view, &document->links[current]);
			current_link_hover(doc_view);
			return 1;
		}
		current += direction;
	}

	vs->current_link = -1;
	return 0;
}

int
next_link_in_view(struct document_view *doc_view, int current, int direction)
{
	return next_link_in_view_(doc_view, current, direction, link_in_view, NULL);
}

int
next_link_in_view_y(struct document_view *doc_view, int current, int direction)
{
	return next_link_in_view_(doc_view, current, direction, link_in_view_y, set_pos_x);
}

/* Get the bounding columns of @link at line @y (or all lines if @y == -1). */
static void
get_link_x_bounds(struct link *link, int y, int *min_x, int *max_x)
{
	int point;

	if (min_x) *min_x = INT_MAX;
	if (max_x) *max_x = 0;

	for (point = 0; point < link->npoints; point++) {
		if (y >= 0 && link->points[point].y != y)
			continue;
		if (min_x) int_upper_bound(min_x, link->points[point].x);
		if (max_x) int_lower_bound(max_x, link->points[point].x);
	}
}

/* Check whether there is any point between @min_x and @max_x at the line @y
 * in link @link. */
static int
get_link_x_intersect(struct link *link, int y, int min_x, int max_x)
{
	int point;

	for (point = 0; point < link->npoints; point++) {
		if (link->points[point].y != y)
			continue;
		if (link->points[point].x >= min_x
		    && link->points[point].x <= max_x)
			return link->points[point].x + 1;
	}

	return 0;
}

/* Check whether there is any point between @min_y and @max_y in the column @x
 * in link @link. */
static int
get_link_y_intersect(struct link *link, int x, int min_y, int max_y)
{
	int point;

	for (point = 0; point < link->npoints; point++) {
		if (link->points[point].x != x)
			continue;
		if (link->points[point].y >= min_y
		    && link->points[point].y <= max_y)
			return link->points[point].y + 1;
	}

	return 0;
}

int
next_link_in_dir(struct document_view *doc_view, int dir_x, int dir_y)
{
	struct document *document;
	struct view_state *vs;
	struct link *link;
	int min_x = INT_MAX, max_x = 0;
	int min_y, max_y;

	assert(doc_view && doc_view->document && doc_view->vs);
	if_assert_failed return 0;
	assert(dir_x || dir_y);
	if_assert_failed return 0;

	document = doc_view->document;
	vs = doc_view->vs;

	link = get_current_link(doc_view);
	if (!link) return 0;

	/* Find the link's "bounding box" coordinates. */

	get_link_x_bounds(link, -1, &min_x, &max_x);

	min_y = link->points[0].y;
	max_y = link->points[link->npoints - 1].y;

	/* Now go from the bounding box edge in the appropriate
	 * direction and find the nearest link. */

	if (dir_y) {
		/* Vertical movement */

		/* The current line number */
		int y = (dir_y > 0 ? max_y : min_y) + dir_y;
		/* The bounding line numbers */
		int top = int_max(0, doc_view->vs->y);
		int bottom = int_min(doc_view->vs->y + doc_view->box.height,
				     document->height);

		for (; dir_y > 0 ? y < bottom : y >= top; y += dir_y) {
			/* @backup points to the nearest link from the left
			 * to the desired position. */
			struct link *backup = NULL;

			link = document->lines1[y];
			if (!link) continue;

			/* Go through all the links on line. */
			for (; link <= document->lines2[y]; link++) {
				int l_min_x, l_max_x;

				/* Some links can be totally out of order here,
				 * ie. in tables or when using tabindex. */
				if (y < link->points[0].y
				    || y > link->points[link->npoints - 1].y)
					continue;

				get_link_x_bounds(link, y, &l_min_x, &l_max_x);
				if (l_min_x > max_x) {
					/* This link is too at the right. */
					if (!backup)
						backup = link;
					continue;
				}
				if (l_max_x < min_x) {
					/* This link is too at the left. */
					backup = link;
					continue;
				}
				/* This link is aligned with the current one. */
				goto chose_link;
			}

			if (backup) {
				link = backup;
				goto chose_link;
			}
		}

		if (!y || y == document->height) {
			/* We just stay at the same place, do not invalidate
			 * the link number. */
			return 0;
		}

	} else {
		/* Horizontal movement */

		/* The current column number */
		int x = (dir_x > 0 ? max_x : min_x) + dir_x;
		/* How many lines are already past their last link */
		int last = 0;

		while ((last < max_y - min_y + 1) && (x += dir_x) >= 0) {
			int y;

			last = 0;

			/* Go through all the lines */
			for (y = min_y; y <= max_y; y++) {
				link = document->lines1[y];
				if (!link) continue;

				/* Go through all the links on line. */
				while (link <= document->lines2[y]) {
					if (get_link_y_intersect(link, x,
					                         min_y,
					                         max_y))
						goto chose_link;
					link++;
				}

				/* Check if we already aren't past the last
				 * link on this line. */
				if (!get_link_x_intersect(document->lines2[y],
				                          y, x, INT_MAX))
					last++;
			}
		}

		/* We just stay  */
		return 0;
	}

	current_link_blur(doc_view);
	vs->current_link = -1;
	return 0;

chose_link:
	/* The link is in bounds, take it. */
	current_link_blur(doc_view);
	vs->current_link = get_link_index(document, link);
	set_pos_x(doc_view, link);
	current_link_hover(doc_view);
	return 1;
}

void
set_pos_x(struct document_view *doc_view, struct link *link)
{
	int xm = 0;
	int xl = INT_MAX;
	int i;

	assert(doc_view && link);
	if_assert_failed return;

	for (i = 0; i < link->npoints; i++) {
		int y = link->points[i].y - doc_view->vs->y;

		if (y >= 0 && y < doc_view->box.height) {
			int_lower_bound(&xm, link->points[i].x + 1);
			int_upper_bound(&xl, link->points[i].x);
		}
	}

	if (xl != INT_MAX)
		int_bounds(&doc_view->vs->x, xm - doc_view->box.width, xl);
}

void
set_pos_y(struct document_view *doc_view, struct link *link)
{
	int ym = 0;
	int height;
	int i;

	assert(doc_view && doc_view->document && doc_view->vs && link);
	if_assert_failed return;

	height = doc_view->document->height;
	for (i = 0; i < link->npoints; i++) {
		int_lower_bound(&ym, link->points[i].y + 1);
		int_upper_bound(&height, link->points[i].y);
	}
	doc_view->vs->y = (ym + height - doc_view->box.height) / 2;
	int_bounds(&doc_view->vs->y, 0,
		   doc_view->document->height - doc_view->box.height);
}

/* direction == 1 -> DOWN
 * direction == -1 -> UP */
static void
find_link(struct document_view *doc_view, int direction, int page_mode)
{
	struct link **line;
	struct link *link = NULL;
	int link_pos;
	int y, ymin, ymax;

	assert(doc_view && doc_view->document && doc_view->vs);
	if_assert_failed return;

	if (direction == -1) {
		/* UP */
		line = doc_view->document->lines2;
		if (!line) goto nolink;
		y = doc_view->vs->y + doc_view->box.height - 1;
		int_upper_bound(&y, doc_view->document->height - 1);
		if (y < 0) goto nolink;
	} else {
		/* DOWN */
		line = doc_view->document->lines1;
		if (!line) goto nolink;
		y = doc_view->vs->y;
		int_lower_bound(&y, 0);
		if (y >= doc_view->document->height) goto nolink;
	}

	ymin = int_max(0, doc_view->vs->y);
	ymax = int_min(doc_view->document->height,
		       doc_view->vs->y + doc_view->box.height);

	if (direction == -1) {
		/* UP */
		do {
			struct link *cur = line[y--];

			if (cur && (!link || cur > link))
				link = cur;
		} while (y >= ymin && y < ymax);

	} else {
		/* DOWN */
		do {
			struct link *cur = line[y++];

			if (cur && (!link || cur < link))
				link = cur;
		} while (y >= ymin && y < ymax);
	}

	if (!link) goto nolink;

	link_pos = link - doc_view->document->links;
	if (page_mode) {
		/* PAGE */
		next_link_in_view(doc_view, link_pos, direction);
		return;
	}
	current_link_blur(doc_view);
	doc_view->vs->current_link = link_pos;
	set_pos_x(doc_view, link);
	current_link_hover(doc_view);
	return;

nolink:
	current_link_blur(doc_view);
	doc_view->vs->current_link = -1;
}

void
find_link_up(struct document_view *doc_view)
{
	find_link(doc_view, -1, 0);
}

void
find_link_page_up(struct document_view *doc_view)
{
	find_link(doc_view, -1, 1);
}

void
find_link_down(struct document_view *doc_view)
{
	find_link(doc_view, 1, 0);
}

void
find_link_page_down(struct document_view *doc_view)
{
	find_link(doc_view, 1, 1);
}

struct uri *
get_link_uri(struct session *ses, struct document_view *doc_view,
	     struct link *link)
{
	assert(ses && doc_view && link);
	if_assert_failed return NULL;

	switch (link->type) {
		case LINK_HYPERTEXT:
		case LINK_MAP:
			if (link->where) return get_uri(link->where, 0);
			return get_uri(link->where_img, 0);

		case LINK_BUTTON:
		case LINK_FIELD:
			return get_form_uri(ses, doc_view,
					    get_link_form_control(link));

		default:
			return NULL;
	}
}

struct link *
goto_current_link(struct session *ses, struct document_view *doc_view, int do_reload)
{
	struct link *link;
	struct uri *uri;

	assert(doc_view && ses);
	if_assert_failed return NULL;

	link = get_current_link(doc_view);
	if (!link) return NULL;

	if (link_is_form(link))
		uri = get_form_uri(ses, doc_view, get_link_form_control(link));
	else
		uri = get_link_uri(ses, doc_view, link);

	if (!uri) return NULL;

	if (link->type == LINK_MAP) {
		/* TODO: Test reload? */
		goto_imgmap(ses, uri, null_or_stracpy(link->target));

	} else {
		enum cache_mode mode = do_reload ? CACHE_MODE_FORCE_RELOAD
						 : CACHE_MODE_NORMAL;

		goto_uri_frame(ses, uri, link->target, mode);
	}

	done_uri(uri);
	return link;
}

static enum frame_event_status
activate_link(struct session *ses, struct document_view *doc_view,
              struct link *link, int do_reload)
{
	struct form_control *link_fc;
	struct form_state *fs;
	struct form *form;

	switch (link->type) {
	case LINK_HYPERTEXT:
	case LINK_MAP:
	case LINK_FIELD:
	case LINK_AREA:
	case LINK_BUTTON:
		if (goto_current_link(ses, doc_view, do_reload))
			return FRAME_EVENT_OK;

		break;
	case LINK_CHECKBOX:
		link_fc = get_link_form_control(link);

		if (form_field_is_readonly(link_fc))
			return FRAME_EVENT_OK;

		fs = find_form_state(doc_view, link_fc);
		if (!fs) return FRAME_EVENT_OK;

		if (link_fc->type == FC_CHECKBOX) {
			fs->state = !fs->state;

			return FRAME_EVENT_REFRESH;
		}

		/* @link_fc->type must be FC_RADIO, then.  First turn
		 * this one on, and then turn off all the other radio
		 * buttons in the group.  Do it in this order because
		 * further @find_form_state calls may reallocate
		 * @doc_view->vs->form_info[] and thereby make the @fs
		 * pointer invalid.  This also allows us to re-use
		 * @fs in the loop. */
		fs->state = 1;
		foreach (form, doc_view->document->forms) {
			struct form_control *fc;

			if (form != link_fc->form)
				continue;

			foreach (fc, form->items) {
				if (fc->type == FC_RADIO
				    && !xstrcmp(fc->name, link_fc->name)
				    && fc != link_fc) {
					fs = find_form_state(doc_view, fc);
					if (fs) fs->state = 0;
				}
			}
		}

		break;

	case LINK_SELECT:
		link_fc = get_link_form_control(link);

		if (form_field_is_readonly(link_fc))
			return FRAME_EVENT_OK;

		object_lock(doc_view->document);
		add_empty_window(ses->tab->term,
				 (void (*)(void *)) release_document,
				 doc_view->document);
		do_select_submenu(ses->tab->term, link_fc->menu, ses);

		break;

	default:
		INTERNAL("bad link type %d", link->type);
	}

	return FRAME_EVENT_REFRESH;
}

enum frame_event_status
enter(struct session *ses, struct document_view *doc_view, int do_reload)
{
	struct link *link;

	assert(ses && doc_view && doc_view->vs && doc_view->document);
	if_assert_failed return FRAME_EVENT_REFRESH;

	link = get_current_link(doc_view);
	if (!link) return FRAME_EVENT_REFRESH;


	if (!current_link_evhook(doc_view, SEVHOOK_ONCLICK))
		return FRAME_EVENT_REFRESH;
	return activate_link(ses, doc_view, link, do_reload);
}

struct link *
get_link_at_coordinates(struct document_view *doc_view, int x, int y)
{
	struct link *l1, *l2, *link;
	int i, height;

	assert(doc_view && doc_view->vs && doc_view->document);
	if_assert_failed return NULL;

	/* If there are no links in in document, there is nothing to do. */
	if (!doc_view->document->nlinks) return NULL;

	/* If the coordinates are outside document view, no need to go further. */
	if (x < 0 || x >= doc_view->box.width) return NULL;
	if (y < 0 || y >= doc_view->box.height) return NULL;

	/* FIXME: This doesn't work. --Zas
	if (!check_mouse_position(ev, &doc_view->box))
		return NULL;
	*/

	/* Find link candidates. */
	l1 = doc_view->document->links + doc_view->document->nlinks;
	l2 = doc_view->document->links;
	height = int_min(doc_view->document->height,
			 doc_view->vs->y + doc_view->box.height);

	for (i = doc_view->vs->y; i < height; i++) {
		if (doc_view->document->lines1[i]
		    && doc_view->document->lines1[i] < l1)
			l1 = doc_view->document->lines1[i];

		if (doc_view->document->lines2[i]
		    && doc_view->document->lines2[i] > l2)
			l2 = doc_view->document->lines2[i];
	}

	/* Is there a link at the given coordinates? */
	x += doc_view->vs->x;
	y += doc_view->vs->y;

	for (link = l1; link <= l2; link++) {
		for (i = 0; i < link->npoints; i++)
			if (link->points[i].x == x
			    && link->points[i].y == y)
				return link;
	}

	return NULL;
}

/* This is backend of the backend goto_link_number_do() below ;)). */
void
jump_to_link_number(struct session *ses, struct document_view *doc_view, int n)
{
	assert(ses && doc_view && doc_view->vs && doc_view->document);
	if_assert_failed return;

	if (n < 0 || n >= doc_view->document->nlinks) return;
	current_link_blur(doc_view);
	doc_view->vs->current_link = n;
	if (ses->navigate_mode == NAVIGATE_CURSOR_ROUTING) {
		struct link *link = get_current_link(doc_view);
		int offset = get_link_cursor_offset(doc_view, link);

		if (link->npoints > offset) {
			int x = link->points[offset].x
				+ doc_view->box.x - doc_view->vs->x;
			int y = link->points[offset].y
				+ doc_view->box.y - doc_view->vs->y;

			move_cursor(ses, doc_view, x, y);
		}
	}
	check_vs(doc_view);
	current_link_hover(doc_view);
}

/* This is common backend for goto_link_number() and try_document_key(). */
static void
goto_link_number_do(struct session *ses, struct document_view *doc_view, int n)
{
	struct link *link;

	assert(ses && doc_view && doc_view->document);
	if_assert_failed return;
	if (n < 0 || n >= doc_view->document->nlinks) return;
	jump_to_link_number(ses, doc_view, n);

	link = &doc_view->document->links[n];
	if (!link_is_textinput(link)
	    && get_opt_bool("document.browse.accesskey.auto_follow"))
		enter(ses, doc_view, 0);
}

void
goto_link_number(struct session *ses, unsigned char *num)
{
	struct document_view *doc_view;

	assert(ses && num);
	if_assert_failed return;
	doc_view = current_frame(ses);
	assert(doc_view);
	if_assert_failed return;
	goto_link_number_do(ses, doc_view, atoi(num) - 1);
}

/* See if this document is interested in the key user pressed. */
enum frame_event_status
try_document_key(struct session *ses, struct document_view *doc_view,
		 struct term_event *ev)
{
	long key;
	int i; /* GOD I HATE C! --FF */ /* YEAH, BRAINFUCK RULEZ! --pasky */

	assert(ses && doc_view && doc_view->document && doc_view->vs && ev);
	if_assert_failed return FRAME_EVENT_IGNORED;

	if (!check_kbd_modifier(ev, KBD_MOD_ALT)) {
		/* We accept those only in alt-combo. */
		return FRAME_EVENT_IGNORED;
	}

	/* Run through all the links and see if one of them is bound to the
	 * key we test.. */
	key = get_kbd_key(ev);

	i = doc_view->vs->current_link + 1;
	for (; i < doc_view->document->nlinks; i++) {
		struct link *link = &doc_view->document->links[i];

		if (key == link->accesskey) {	/* FIXME: key vs unicode ... */
			ses->kbdprefix.repeat_count = 0;
			goto_link_number_do(ses, doc_view, i);
			return FRAME_EVENT_REFRESH;
		}
	}
	for (i = 0; i <= doc_view->vs->current_link; i++) {
		struct link *link = &doc_view->document->links[i];

		if (key == link->accesskey) {	/* FIXME: key vs unicode ... */
			ses->kbdprefix.repeat_count = 0;
			goto_link_number_do(ses, doc_view, i);
			return FRAME_EVENT_REFRESH;
		}
	}

	return FRAME_EVENT_IGNORED;
}

/* Open a contextual menu on a link, form or image element. */
/* TODO: This should be completely configurable. */
void
link_menu(struct terminal *term, void *xxx, void *ses_)
{
	struct session *ses = ses_;
	struct document_view *doc_view;
	struct link *link;
	struct menu_item *mi;
	struct form_control *fc;

	assert(term && ses);
	if_assert_failed return;

	doc_view = current_frame(ses);
	mi = new_menu(FREE_LIST);
	if (!mi) return;
	if (!doc_view) goto end;

	assert(doc_view->vs && doc_view->document);
	if_assert_failed return;

	link = get_current_link(doc_view);
	if (!link) goto end;

	if (link->where && !link_is_form(link)) {
		if (link->type == LINK_MAP) {
			add_to_menu(&mi, N_("Display ~usemap"), NULL, ACT_MAIN_LINK_FOLLOW,
				    NULL, NULL, SUBMENU);
		} else {
			add_menu_action(&mi, N_("~Follow link"), ACT_MAIN_LINK_FOLLOW);

			add_menu_action(&mi, N_("Follow link and r~eload"), ACT_MAIN_LINK_FOLLOW_RELOAD);

			add_menu_separator(&mi);

			add_new_win_to_menu(&mi, N_("Open in new ~window"), term);

			add_menu_action(&mi, N_("Open in new ~tab"), ACT_MAIN_OPEN_LINK_IN_NEW_TAB);

			add_menu_action(&mi, N_("Open in new tab in ~background"),
					ACT_MAIN_OPEN_LINK_IN_NEW_TAB_IN_BACKGROUND);

			if (!get_cmd_opt_bool("anonymous")) {
				add_menu_separator(&mi);
				add_menu_action(&mi, N_("~Download link"), ACT_MAIN_LINK_DOWNLOAD);

#ifdef CONFIG_BOOKMARKS
				add_menu_action(&mi, N_("~Add link to bookmarks"),
						ACT_MAIN_ADD_BOOKMARK_LINK);
#endif
				add_uri_command_to_menu(&mi, PASS_URI_LINK);
			}
		}
	}

	fc = get_link_form_control(link);
	if (fc) {
		switch (fc->type) {
		case FC_RESET:
			add_menu_action(&mi, N_("~Reset form"), ACT_MAIN_RESET_FORM);
			break;

		case FC_TEXTAREA:
			if (!form_field_is_readonly(fc)) {
				struct string keystroke;

				if (init_string(&keystroke))
					add_keystroke_action_to_string(
					                 &keystroke,
					                 ACT_EDIT_OPEN_EXTERNAL,
							 KEYMAP_EDIT);

				add_to_menu(&mi, N_("Open in ~external editor"),
					    keystroke.source, ACT_MAIN_NONE,
					    menu_textarea_edit, NULL, FREE_RTEXT);
			}
			/* Fall through */
		default:
			add_menu_action(&mi, N_("~Submit form"), ACT_MAIN_SUBMIT_FORM);
			add_menu_action(&mi, N_("Submit form and rel~oad"), ACT_MAIN_SUBMIT_FORM_RELOAD);

			assert(fc->form);
			if (fc->form->method == FORM_METHOD_GET) {
				add_new_win_to_menu(&mi, N_("Submit form and open in new ~window"), term);

				add_menu_action(&mi, N_("Submit form and open in new ~tab"),
						ACT_MAIN_OPEN_LINK_IN_NEW_TAB);

				add_menu_action(&mi, N_("Submit form and open in new tab in ~background"),
						ACT_MAIN_OPEN_LINK_IN_NEW_TAB_IN_BACKGROUND);
			}

			if (!get_cmd_opt_bool("anonymous"))
				add_menu_action(&mi, N_("Submit form and ~download"), ACT_MAIN_LINK_DOWNLOAD);

			add_menu_action(&mi, N_("~Reset form"), ACT_MAIN_RESET_FORM);
		}

		add_to_menu(&mi, N_("Form f~ields"), NULL, ACT_MAIN_LINK_FORM_MENU,
			    NULL, NULL, SUBMENU);
	}

	if (link->where_img) {
		add_menu_action(&mi, N_("V~iew image"), ACT_MAIN_VIEW_IMAGE);
		if (!get_cmd_opt_bool("anonymous"))
			add_menu_action(&mi, N_("Download ima~ge"), ACT_MAIN_LINK_DOWNLOAD_IMAGE);
	}

	/* TODO: Make it possible to trigger any script event hooks associated
	 * to the link. --pasky */

end:
	if (!mi->text) {
		add_to_menu(&mi, N_("No link selected"), NULL, ACT_MAIN_NONE,
			    NULL, NULL, NO_SELECT);
	}

	do_menu(term, mi, ses, 1);
}

/* Return current link's title. Pretty trivial. */
unsigned char *
get_current_link_title(struct document_view *doc_view)
{
	struct link *link;

	assert(doc_view && doc_view->document && doc_view->vs);
	if_assert_failed return NULL;

	if (doc_view->document->frame_desc)
		return NULL;

	link = get_current_link(doc_view);

	return (link && link->title && *link->title) ? stracpy(link->title) : NULL;
}

unsigned char *
get_current_link_info(struct session *ses, struct document_view *doc_view)
{
	struct link *link;

	assert(ses && doc_view && doc_view->document && doc_view->vs);
	if_assert_failed return NULL;

	if (doc_view->document->frame_desc)
		return NULL;

	link = get_current_link(doc_view);
	if (!link) return NULL;

	/* TODO: Provide info about script event hooks too. --pasky */

	if (!link_is_form(link)) {
		struct terminal *term = ses->tab->term;
		struct string str;
		unsigned char *uristring = link->where;

		if (!init_string(&str)) return NULL;

		if (!link->where && link->where_img) {
			add_to_string(&str, _("Image", term));
			add_char_to_string(&str, ' ');
			uristring = link->where_img;

		} else if (link->type == LINK_MAP) {
			add_to_string(&str, _("Usemap", term));
			add_char_to_string(&str, ' ');
		}

		/* Add the uri with password and post info stripped */
		add_string_uri_to_string(&str, uristring, URI_PUBLIC);
		if (link->accesskey > 0
		    && get_opt_bool("document.browse.accesskey.display")) {
			add_to_string(&str, " (");
			add_accesskey_to_string(&str, link->accesskey);
			add_char_to_string(&str, ')');
		}

		decode_uri_string_for_display(&str);
		return str.source;
	}

	if (!get_link_form_control(link)) return NULL;

	return get_form_info(ses, doc_view);
}
