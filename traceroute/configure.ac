# automatic configuration script (a.k.a. autoconf) for KCN programs.
#
# please process configure.ac to produce configure with following commands:
# % aclocal
# % autoheader
# % automake --add-missing --copy
# % autoconf

#
AC_PREREQ([2.59])
AC_INIT([kcn-traceroute], [0.0.0], [ohmori@tottori-u.ac.jp])
AC_CANONICAL_TARGET([])
AC_CONFIG_SRCDIR([README])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE

#
AC_PROG_MAKE_SET
AC_PROG_CPP
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_GCC_TRADITIONAL
dnl
dnl expand AC_PROG_OBJC for old autoconf tools
dnl
AC_CHECK_TOOL([OBJC], [gcc])
AC_SUBST(OBJC)
AC_SUBST(OBJCFLAGS)
dnl AC_PROG_OBJC
AC_PROG_RANLIB
AC_PROG_INSTALL
AC_CHECK_TOOL(AR, ar)
AC_CHECK_TOOL(RANLIB, ranlib, :)

#
AC_CHECK_HEADERS([stdbool.h netinet/in.h rump/rump.h])
AC_CHECK_HEADERS([netinet/ip_var.h netinet/udp_var.h], [], [], [
	#ifdef HAVE_NETINET_IN_H
	#include <netinet/in.h>
	#endif /* HAVE_NETINET_IN_H */
	])

#
AC_CHECK_TYPES([struct ipovly, struct udpiphdr])

#
AC_CHECK_MEMBERS([struct sockaddr_in.sin_len],[],[],[#include <netinet/in.h>])
AC_CHECK_MEMBERS([struct icmp.icmp_nextmtu],[],[],
    [#include <netinet/ip_icmp.h>])

#
AC_CHECK_LIB(rump, rumpclient_init)

#
AC_CHECK_FUNCS([arc4random getprogname setprogname])

#
AC_CHECK_SYSCTL

#
PKG_CHECK_MODULES(CURL, libcurl)
AC_SUBST(CURL_CFLAGS)
AC_SUBST(CURL_LIBS)
#
PKG_CHECK_MODULES(JANSSON, jansson)
AC_SUBST(JANSSON_CFLAGS)
AC_SUBST(JANSSON_LIBS)
#
AC_CHECK_KCN
AC_SUBST(KCN_CFLAGS)
AC_SUBST(KCN_LIBS)

#
CFLAGS="$CFLAGS -W -Wall -Werror -Wmissing-declarations -fstrict-aliasing -Wstrict-aliasing=0"

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
