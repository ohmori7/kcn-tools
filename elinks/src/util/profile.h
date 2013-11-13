#ifndef EL__UTIL_PROFILE_H
#define EL__UTIL_PROFILE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* Process' CPU time utilities */
#ifdef CONFIG_DEBUG
#include <time.h>

/* CLK_DECL(n) declares an array of @n clock_t to be used by CLK_* macros.
 * Must occur before any CLK_* macros call. */
#define CLK_DECL(n) clock_t clk_start[n], clk_end[n], clk_diff[n]; int clk_start_line[n], clk_stop_line[n]

/* CLK_START(n) starts clock @n.
 * Must occur after CLK_DECL() and before CLK_STOP(). */
#define CLK_START(n) do { clk_start_line[n] = __LINE__; clk_start[n] = clock(); } while (0)

/* CLK_STOP(n) stops clock @n.
 * Must occur after CLK_START() and before CLK_DUMP(). */
#define CLK_STOP(n) do { clk_end[n] = clock(); clk_diff[n] = clk_end[n] - clk_start[n]; clk_stop_line[n] = __LINE__; } while (0)

/* CLK_DUMP(n) echoes cpu time spent between CLK_START(@n) and CLK_STOP(@n).
 * Must occur after CLK_START() and  CLK_STOP(). */
#define CLK_DUMP(n) do { \
	fprintf(stderr, "%s:%d->%d CLK[%d]= %.16f sec.\n", \
		__FILE__, clk_start_line[n], clk_stop_line[n], \
		n, (double) clk_diff[n] / (double) CLOCKS_PER_SEC);\
} while (0)

/* Shortcuts for function profiling.
 * CLK_STA() must be at end of local declarations.
 * CLK_STO() must be just before end of function. */
#define CLK_STA() CLK_DECL(1); CLK_START(0)
#define CLK_STO() CLK_STOP(0); CLK_DUMP(0)

#else
/* Dummy macros. */
#define CLK_DECL(n)
#define CLK_START(n) do {} while (0)
#define CLK_STOP(n)  do {} while (0)
#define CLK_DUMP(n)  do {} while (0)
#define CLK_STA()
#define CLK_STO()
#endif

#endif
