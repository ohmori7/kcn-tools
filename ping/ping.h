#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifndef __RCSID
#define __RCSID(a)
#endif /* ! __RCSID */

#if defined(__GNUC__)
#define __dead		__attribute__((__noreturn__))
#else /* __GNUC__ */
#define __dead
#endif /* ! __GNUC__ */
