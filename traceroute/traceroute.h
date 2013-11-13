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

#define __FAVOR_BSD

#ifndef IPCTL_DEFTTL
#define IPCTL_DEFTTL	255
#endif /* ! IPCTL_DEFTTL */

