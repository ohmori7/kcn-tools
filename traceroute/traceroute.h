#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifndef __RCSID
#define __RCSID(a)
#endif /* ! __RCSID */

#ifndef __COPYRIGHT
#define __COPYRIGHT(a)
#endif /* ! __COPYRIGHT */

#ifdef __GNUC__
#define __dead		__attribute__((__noreturn__))
#else /* __GNUC__ */
#define __dead
#endif /* ! __GNUC__ */

#ifdef __GNUC__
#define __unused	__attribute__((__unused__))
#else /* __GNUC__ */
#define __unused
#endif /* ! __GNUC__ */

#define __FAVOR_BSD
