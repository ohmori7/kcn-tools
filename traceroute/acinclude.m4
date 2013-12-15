dnl macro for sysctl()
# ----------------------------------------------------------------
# AC_CHECK_SYSCTL
# ----------------------------------------------------------------
AC_DEFUN([AC_CHECK_SYSCTL_WRAP],
[
	cflags=$CFLAGS
	CFLAGS="$CFLAGS -Wall -Werror"
	AC_TRY_COMPILE(
		[
		#include <sys/types.h>
		#include <sys/sysctl.h>
		],
		[
		int (*fn)($1);
		fn = sysctl
		],
		[$2],[$3])
	CFLAGS=$cflags
])
dnl
AC_DEFUN([AC_CHECK_SYSCTL],
[
	AC_MSG_CHECKING(for sysctl type)
	AC_CHECK_SYSCTL_WRAP(
		[const int *, u_int, void *, size_t *, const void *, size_t],
		[
		AC_DEFINE([HAVE_BSD_SYSCTL],[1],
		    [Define if BSD-style sysctl exists])
		AC_MSG_RESULT(bsd)
		],
		[AC_CHECK_SYSCTL_WRAP(
			[int *, int, void *, size_t *, void *, size_t],
			[
			AC_DEFINE([HAVE_LINUX_SYSCTL],[1],
				[Define if Linux-style sysctl exists])
			AC_MSG_RESULT(linux)
			],
			[AC_MSG_ERROR([unknown sysctl.])])]
		)
])
dnl autoconf KCN macros
# ----------------------------------------------------------------
# AC_CHECK_KCN
# ----------------------------------------------------------------
AC_DEFUN([AC_CHECK_KCN],
[
	AC_MSG_CHECKING(for KCN library)
	AC_ARG_WITH(kcn,
	     [  --with-kcn[=PREFIX]       support KCN],,
	     [with_kcn="yes"])

	AC_MSG_RESULT($with_kcn)

	if test x"$with_kcn" != xno; then
		if test x"$with_kcn" = xyes; then
			with_kcn='/usr/local /usr/pkg /usr'
		fi
		kcndir=''
		ldflags="$LDFLAGS"
		for dir in $with_kcn; do
			LDFLAGS="$LDFLAGS -L$dir/lib $CURL_LIBS $JANSSON_LIBS"
			unset ac_cv_lib_kcn_kcn_info_new
			AC_CHECK_LIB(kcn, kcn_info_new, [kcndir="$dir"; break])
		done
		if test x"$kcndir" = x; then
			AC_MSG_ERROR([KCN library not found.])
		fi
		LDFLAGS="$ldflags"
		KCN_LIBS=""
		if test $kcndir != '/usr'; then
			KCN_CFLAGS="-I$kcndir/include"
			KCN_LIBS="-Wl,-rpath=$kcndir/lib -L$kcndir/lib"
		fi
		KCN_LIBS="$KCN_LIBS -lkcn"
		AC_DEFINE([HAVE_KCN], [1],
		    [Define to 1 if you have a KCN support.])
	fi
])
