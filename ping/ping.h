#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifndef __RCSID
#define __RCSID(a)
#endif /* ! __RCSID */

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

#ifndef timespecadd
#define timespecadd(t, u, v)						\
do {									\
	(v)->tv_sec = (t)->tv_sec + (u)->tv_sec;			\
	(v)->tv_nsec = (t)->tv_nsec + (u)->tv_nsec;			\
	if ((v)->tv_nsec >= 1000000000L) {				\
		(v)->tv_sec++;						\
		(v)->tv_nsec -= 1000000000L;				\
	}								\
} while (/*CONSTCOND*/0)
#endif /*! timespecadd */
