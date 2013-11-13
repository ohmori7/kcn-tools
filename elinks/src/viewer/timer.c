/* Internal inactivity timer. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "elinks.h"

#include "config/kbdbind.h"
#include "config/options.h"
#include "main/module.h"
#include "main/timer.h"
#include "terminal/event.h"
#include "terminal/kbd.h"
#include "terminal/terminal.h"
#include "util/time.h"
#include "viewer/timer.h"

#define COUNT_DOWN_DELAY	((milliseconds_T) 1000)

static timer_id_T countdown = TIMER_ID_UNDEF;

static int timer_duration = 0;

int
get_timer_duration(void)
{
	return timer_duration;
}

static void
count_down(void *xxx)
{
	struct keybinding *keybinding;

	timer_duration--;
	if (timer_duration) {
		install_timer(&countdown, COUNT_DOWN_DELAY, count_down, NULL);
		return;
	} else {
		countdown = TIMER_ID_UNDEF;
	}

	keybinding = kbd_nm_lookup(KEYMAP_MAIN, get_opt_str("ui.timer.action"));
	if (keybinding) {
		struct terminal *terminal;
		struct term_event ev;

		set_kbd_term_event(&ev, keybinding->kbd.key,
				   keybinding->kbd.modifier);

		foreach (terminal, terminals) {
			term_send_event(terminal, &ev);
		}
	}

	reset_timer();
}

void
reset_timer(void)
{
	kill_timer(&countdown);

	if (!get_opt_int("ui.timer.enable")) return;

	timer_duration = get_opt_int("ui.timer.duration");
	install_timer(&countdown, COUNT_DOWN_DELAY, count_down, NULL);
}

static void
init_timer(struct module *module)
{
	reset_timer();
}

static void
done_timer(struct module *module)
{
	kill_timer(&countdown);
}

struct module timer_module = struct_module(
	/* name: */		"Timer",
	/* options: */		NULL,
	/* hooks: */		NULL,
	/* submodules: */	NULL,
	/* data: */		NULL,
	/* init: */		init_timer,
	/* done: */		done_timer
);
