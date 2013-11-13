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
#define __packed	__attribute__((__packed__))
#else /* __GNUC__ */
#define __dead
#define __packed
#endif /* ! __GNUC__ */

#define __FAVOR_BSD

#ifndef IPCTL_DEFTTL
#define IPCTL_DEFTTL	255
#endif /* ! IPCTL_DEFTTL */

#ifdef HAVE_STRUCT_ICMP_ICMP_NEXTMTU
#define HAVE_ICMP_NEXTMTU
#endif /* HAVE_STRUCT_ICMP_ICMP_NEXTMTU */
