dnl Automatically generated from config/m4/ files by autogen.sh!
dnl Do not modify!
#serial AM1

dnl From Bruno Haible.

AC_DEFUN([AM_LANGINFO_CODESET],
[
  AC_CACHE_CHECK([for nl_langinfo and CODESET], am_cv_langinfo_codeset,
    [AC_TRY_LINK([#include <langinfo.h>],
      [char* cs = nl_langinfo(CODESET);],
      am_cv_langinfo_codeset=yes,
      am_cv_langinfo_codeset=no)
    ])
  if test $am_cv_langinfo_codeset = yes; then
    AC_DEFINE(HAVE_LANGINFO_CODESET, 1,
      [Define if you have <langinfo.h> and nl_langinfo(CODESET).])
  fi
])
dnl ===================================================================
dnl Macros for various checks
dnl ===================================================================

dnl TODO: Make EL_CONFIG* macros assume CONFIG_* defines so it is possible
dnl to write EL_CONFIG_DEPENDS(SCRIPTING, [GUILE LUA PERL], [...])

dnl EL_CONFIG(define, what)
AC_DEFUN([EL_CONFIG], [
	  $1=yes
	  ABOUT_$1="$2"
	  AC_DEFINE($1, 1, [Define if you want: $2 support])])

dnl EL_LOG_CONFIG(define, description, value)
AC_DEFUN([EL_LOG_CONFIG],
[
	about="$2"
	value="$3"
	[msgdots2="`echo $about | sed 's/[0-9]/./g'`"]
	[msgdots1="`echo $msgdots2 | sed 's/[a-z]/./g'`"]
	[msgdots0="`echo $msgdots1 | sed 's/[A-Z]/./g'`"]
	[msgdots="`echo $msgdots0 | sed 's/[_ ()]/./g'`"]
	DOTS="................................"
	dots=`echo $DOTS | sed "s/$msgdots//"`

	# $msgdots too big?
	if test "$dots" = "$DOTS"; then
		dots=""
	fi

	if test -z "$value"; then
		value="[$]$1"
	fi

	echo "$about $dots $value" >> features.log
	AC_SUBST([$1])
])

dnl EL_CONFIG_DEPENDS(define, CONFIG_* dependencies, what)
AC_DEFUN([EL_CONFIG_DEPENDS],
[
	$1=no
	el_value=

	for dependency in $2; do
		# Hope this is portable?!? --jonas
		eval el_config_value=$`echo $dependency`

		if test "$el_config_value" = yes; then
			el_about_dep=$`echo ABOUT_$dependency`
			eval depvalue=$el_about_dep

			if test -z "$el_value"; then
				el_value="$depvalue"
			else
				el_value="$el_value, $depvalue"
			fi
			$1=yes
		fi
	done

	if test "[$]$1" = yes; then
		EL_CONFIG($1, [$3])
	fi
	EL_LOG_CONFIG([$1], [$3], [$el_value])
])

dnl EL_ARG_ENABLE(define, name, conf-help, arg-help)
AC_DEFUN([EL_ARG_ENABLE],
[
	AC_ARG_ENABLE($2, [$4],
	[
		if test "$enableval" != no; then enableval="yes"; fi
		$1="$enableval";
	])

	if test "x[$]$1" = xyes; then
		EL_CONFIG($1, [$3])
	else
		$1=no
	fi
	EL_LOG_CONFIG([$1], [$3], [])
])

dnl EL_ARG_DEPEND(define, name, depend, conf-help, arg-help)
AC_DEFUN([EL_ARG_DEPEND],
[
	AC_ARG_ENABLE($2, [$5],
	[
		if test "$enableval" != no; then enableval="yes"; fi
		$1="$enableval"
	])

	ENABLE_$1="[$]$1";
	if test "x[$]$1" = xyes; then
		# require all dependencies to be met
		for dependency in $3; do
			el_name=`echo "$dependency" | sed 's/:.*//'`;
			el_arg=`echo "$dependency" | sed 's/.*://'`;
			# Hope this is portable?!? --jonas
			eval el_value=$`echo $el_name`;

			if test "x$el_value" != "x$el_arg"; then
				ENABLE_$1=no;
				break;
			fi
		done

		if test "[$]ENABLE_$1" = yes; then
			EL_CONFIG($1, [$4])
		else
			$1=no;
		fi
	else
		$1=no;
	fi
	EL_LOG_CONFIG([$1], [$4], [])
])

dnl EL_DEFINE(define, what)
AC_DEFUN([EL_DEFINE], [AC_DEFINE($1, 1, [Define if you have $2])])

dnl EL_CHECK_CODE(type, define, includes, code)
AC_DEFUN([EL_CHECK_CODE],
[
	$2=yes;
	AC_MSG_CHECKING([for $1])
	AC_TRY_COMPILE([$3], [$4], [EL_DEFINE($2, [$1])], $2=no)
	AC_MSG_RESULT([$]$2)
])

dnl EL_CHECK_TYPE(type, default)
AC_DEFUN([EL_CHECK_TYPE],
[
        EL_CHECK_TYPE_LOCAL=yes;
        AC_MSG_CHECKING([for $1])
        AC_TRY_COMPILE([
#include <sys/types.h>
        ], [int a = sizeof($1);],
        [EL_CHECK_TYPE_LOCAL=yes], [EL_CHECK_TYPE_LOCAL=no])
        AC_MSG_RESULT([$]EL_CHECK_TYPE_LOCAL)
        if test "x[$]EL_CHECK_TYPE_LOCAL" != "xyes"; then
                AC_DEFINE($1, $2, [Define to $2 if <sys/types.h> doesn't define.])
        fi
])

dnl EL_CHECK_SYS_TYPE(type, define, includes)
AC_DEFUN([EL_CHECK_SYS_TYPE],
[
	EL_CHECK_CODE([$1], [$2], [
#include <sys/types.h>
$3
	], [int a = sizeof($1);])
])

dnl EL_CHECK_NET_TYPE(type, define, include)
AC_DEFUN([EL_CHECK_NET_TYPE],
[
	EL_CHECK_SYS_TYPE([$1], [$2], [
#include<sys/socket.h>
$3
	])
])

dnl EL_CHECK_INT_TYPE(type, define)
AC_DEFUN([EL_CHECK_INT_TYPE],
[
	EL_CHECK_SYS_TYPE([$1], [$2], [
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
	])
])


dnl Save and restore the current build flags

AC_DEFUN([EL_SAVE_FLAGS],
[
	CFLAGS_X="$CFLAGS";
	CPPFLAGS_X="$CPPFLAGS";
	LDFLAGS_X="$LDFLAGS";
	LIBS_X="$LIBS";
])

AC_DEFUN([EL_RESTORE_FLAGS],
[
	CFLAGS="$CFLAGS_X";
	CPPFLAGS="$CPPFLAGS_X";
	LDFLAGS="$LDFLAGS_X";
	LIBS="$LIBS_X";
])

# Macro to add for using GNU gettext.
# Ulrich Drepper <drepper@cygnus.com>, 1995.
#
# This file can be copied and used freely without restrictions.  It can
# be used in projects which are not available under the GNU General Public
# License but which still want to provide support for the GNU gettext
# functionality.
# Please note that the actual code of GNU gettext is covered by the GNU
# General Public License and is *not* in the public domain.

# serial 10

dnl Note that we always use own gettext implementation, even if we found
dnl working system one. We have some own modifications in our implementation
dnl and we rely on them.

dnl Usage: AM_WITH_NLS([TOOLSYMBOL], [NEEDSYMBOL], [LIBDIR]).
dnl If TOOLSYMBOL is specified and is 'use-libtool', then a libtool library
dnl    $(top_builddir)/intl/libintl.la will be created (shared and/or static,
dnl    depending on --{enable,disable}-{shared,static} and on the presence of
dnl    AM-DISABLE-SHARED). Otherwise, a static library
dnl    $(top_builddir)/intl/libintl.a will be created.
dnl If NEEDSYMBOL is specified and is 'need-ngettext', then GNU gettext
dnl    implementations (in libc or libintl) without the ngettext() function
dnl    will be ignored.
dnl LIBDIR is used to find the intl libraries.  If empty,
dnl    the value `$(top_builddir)/intl/' is used.
dnl
dnl The result of the configuration is one of three cases:
dnl 1) GNU gettext, as included in the intl subdirectory, will be compiled
dnl    and used.
dnl    Catalog format: GNU --> install in $(datadir)
dnl    Catalog extension: .mo after installation, .gmo in source tree
dnl 2) GNU gettext has been found in the system's C library.
dnl    Catalog format: GNU --> install in $(datadir)
dnl    Catalog extension: .mo after installation, .gmo in source tree
dnl 3) No internationalization, always use English msgid.
dnl    Catalog format: none
dnl    Catalog extension: none
dnl The use of .gmo is historical (it was needed to avoid overwriting the
dnl GNU format catalogs when building on a platform with an X/Open gettext),
dnl but we keep it in order not to force irrelevant filename changes on the
dnl maintainers.
dnl
AC_DEFUN([AM_WITH_NLS],
  [AC_MSG_CHECKING([whether NLS is requested])
    dnl Default is enabled NLS
    CONFIG_NLS=yes
    EL_ARG_ENABLE(CONFIG_NLS, nls, [Native Language Support],
      [  --disable-nls           do not use Native Language Support])

    AC_MSG_RESULT($CONFIG_NLS)
    AC_SUBST(CONFIG_NLS)

    AM_CONDITIONAL(CONFIG_NLS, test "$CONFIG_NLS" = "yes")

    dnl If we use NLS figure out what method
    if test "$CONFIG_NLS" = "yes"; then
      AC_DEFINE(CONFIG_NLS, 1,
        [Define to 1 if translation of program messages to the user's native language
   is requested.])
dnl      AC_MSG_CHECKING([whether included gettext is requested])
dnl      AC_ARG_WITH(included-gettext,
dnl        [  --with-included-gettext use the GNU gettext library included here],
dnl        nls_cv_force_use_gnu_gettext=$withval,
dnl        nls_cv_force_use_gnu_gettext=no)
dnl      AC_MSG_RESULT($nls_cv_force_use_gnu_gettext)

      nls_cv_force_use_gnu_gettext=yes
      nls_cv_use_gnu_gettext=yes

      dnl Mark actions used to generate GNU NLS library.
      AM_PATH_PROG_WITH_TEST(MSGFMT, msgfmt,
	[$ac_dir/$ac_word --statistics /dev/null >/dev/null 2>&1], :)
      AC_PATH_PROG(GMSGFMT, gmsgfmt, $MSGFMT)
      AM_PATH_PROG_WITH_TEST(XGETTEXT, xgettext,
	[$ac_dir/$ac_word --omit-header /dev/null >/dev/null 2>&1], :)
      AC_SUBST(MSGFMT)
      LIBS=`echo " $LIBS " | sed -e 's/ -lintl / /' -e 's/^ //' -e 's/ $//'`
      LIBS="$LIBS $LIBICONV"

      dnl This could go away some day; the PATH_PROG_WITH_TEST already does it.
      dnl Test whether we really found GNU msgfmt.
      if test "$GMSGFMT" != ":"; then
	dnl If it is no GNU msgfmt we define it as : so that the
	dnl Makefiles still can work.
	if $GMSGFMT --statistics /dev/null >/dev/null 2>&1; then
	  : ;
	else
	  AC_MSG_RESULT(
	    [found msgfmt program is not GNU msgfmt; ignore it])
	  GMSGFMT=":"
	fi
      fi

      dnl This could go away some day; the PATH_PROG_WITH_TEST already does it.
      dnl Test whether we really found GNU xgettext.
      if test "$XGETTEXT" != ":"; then
	dnl If it is no GNU xgettext we define it as : so that the
	dnl Makefiles still can work.
	if $XGETTEXT --omit-header /dev/null >/dev/null 2>&1; then
	  : ;
	else
	  AC_MSG_RESULT(
	    [found xgettext program is not GNU xgettext; ignore it])
	  XGETTEXT=":"
	fi
      fi
    fi


    dnl intl/plural.c is generated from intl/plural.y. It requires bison,
    dnl because plural.y uses bison specific features. It requires at least
    dnl bison-1.26 because earlier versions generate a plural.c that doesn't
    dnl compile.
    dnl bison is only needed for the maintainer (who touches plural.y). But in
    dnl order to avoid separate Makefiles or --enable-maintainer-mode, we put
    dnl the rule in general Makefile. Now, some people carelessly touch the
    dnl files or have a broken "make" program, hence the plural.c rule will
    dnl sometimes fire. To avoid an error, defines BISON to ":" if it is not
    dnl present or too old.
    AC_CHECK_PROGS([INTLBISON], [bison])
    if test -z "$INTLBISON"; then
      ac_verc_fail=yes
    else
      dnl Found it, now check the version.
      AC_MSG_CHECKING([version of bison])
changequote(<<,>>)dnl
      ac_prog_version=`$INTLBISON --version 2>&1 | sed -n 's/^.*GNU Bison.* \([0-9]*\.[0-9.]*\).*$/\1/p'`
      case $ac_prog_version in
        '') ac_prog_version="v. ?.??, bad"; ac_verc_fail=yes;;
        1.2[6-9]* | 1.[3-9][0-9]* | [2-9].*)
changequote([,])dnl
           ac_prog_version="$ac_prog_version, ok"; ac_verc_fail=no;;
        *) ac_prog_version="$ac_prog_version, bad"; ac_verc_fail=yes;;
      esac
      AC_MSG_RESULT([$ac_prog_version])
    fi
    if test $ac_verc_fail = yes; then
      INTLBISON=:
    fi

    dnl These rules are solely for the distribution goal.  While doing this
    dnl we only have to keep exactly one list of the available catalogs
    dnl in configure.in.
    for lang in $ALL_LINGUAS; do
      GMOFILES="$GMOFILES $lang.gmo"
    done

    dnl Make all variables we use known to autoconf.
    AC_SUBST(CATALOGS)
    AC_SUBST(GMOFILES)

    dnl For backward compatibility. Some configure.ins may be using this.
    nls_cv_header_intl=
    nls_cv_header_libgt=
  ])

dnl Usage: Just like AM_WITH_NLS, which see.
AC_DEFUN([AM_GNU_GETTEXT],
  [AC_REQUIRE([AC_PROG_MAKE_SET])dnl
   AC_REQUIRE([AC_PROG_CC])dnl
   AC_REQUIRE([AC_CANONICAL_HOST])dnl
   AC_REQUIRE([AC_PROG_RANLIB])dnl
   AC_REQUIRE([AC_ISC_POSIX])dnl
   AC_REQUIRE([AC_HEADER_STDC])dnl
   AC_REQUIRE([AC_C_CONST])dnl
   AC_REQUIRE([AC_C_INLINE])dnl
   AC_REQUIRE([AC_TYPE_OFF_T])dnl
   AC_REQUIRE([AC_TYPE_SIZE_T])dnl
   AC_REQUIRE([AC_FUNC_ALLOCA])dnl
   AC_REQUIRE([AC_FUNC_MMAP])dnl
   AC_REQUIRE([jm_GLIBC21])dnl

   AC_CHECK_HEADERS([argz.h limits.h locale.h nl_types.h malloc.h stddef.h \
stdlib.h string.h unistd.h sys/param.h])
   AC_CHECK_FUNCS([feof_unlocked fgets_unlocked getcwd getegid geteuid \
getgid getuid mempcpy munmap putenv setenv setlocale stpcpy strchr strcasecmp \
strdup strtoul tsearch __argz_count __argz_stringify __argz_next])

   AM_ICONV
   AM_LANGINFO_CODESET
   AM_LC_MESSAGES
   AM_WITH_NLS([$1],[$2],[$3])

   if test "x$ALL_LINGUAS" = "x"; then
     LINGUAS=
   else
     AC_MSG_CHECKING(for catalogs to be installed)
     NEW_LINGUAS=
     for presentlang in $ALL_LINGUAS; do
       useit=no
       for desiredlang in ${LINGUAS-$ALL_LINGUAS}; do
         # Use the presentlang catalog if desiredlang is
         #   a. equal to presentlang, or
         #   b. a variant of presentlang (because in this case,
         #      presentlang can be used as a fallback for messages
         #      which are not translated in the desiredlang catalog).
         case "$desiredlang" in
           "$presentlang"*) useit=yes;;
         esac
       done
       if test $useit = yes; then
         NEW_LINGUAS="$NEW_LINGUAS $presentlang"
       fi
     done
     LINGUAS=$NEW_LINGUAS
     AC_MSG_RESULT($LINGUAS)
   fi

   dnl Construct list of names of catalog files to be constructed.
   if test -n "$LINGUAS"; then
     for lang in $LINGUAS; do CATALOGS="$CATALOGS $lang.gmo"; done
   fi

   dnl If the AC_CONFIG_AUX_DIR macro for autoconf is used we possibly
   dnl find the mkinstalldirs script in another subdir but $(top_srcdir).
   dnl Try to locate is.
   MKINSTALLDIRS=
   if test -n "$ac_aux_dir"; then
     MKINSTALLDIRS="$ac_aux_dir/mkinstalldirs"
   fi
   if test -z "$MKINSTALLDIRS"; then
     MKINSTALLDIRS="\$(top_srcdir)/mkinstalldirs"
   fi
   AC_SUBST(MKINSTALLDIRS)

   dnl Enable libtool support if the surrounding package wishes it.
   INTL_LIBTOOL_SUFFIX_PREFIX=ifelse([$1], use-libtool, [l], [])
   AC_SUBST(INTL_LIBTOOL_SUFFIX_PREFIX)
  ])
#serial 2

# Test for the GNU C Library, version 2.1 or newer.
# From Bruno Haible.

AC_DEFUN([jm_GLIBC21],
  [
    AC_CACHE_CHECK(whether we are using the GNU C Library 2.1 or newer,
      ac_cv_gnu_library_2_1,
      [AC_EGREP_CPP([Lucky GNU user],
	[
#include <features.h>
#ifdef __GNU_LIBRARY__
 #if (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 1) || (__GLIBC__ > 2)
  Lucky GNU user
 #endif
#endif
	],
	ac_cv_gnu_library_2_1=yes,
	ac_cv_gnu_library_2_1=no)
      ]
    )
    AC_SUBST(GLIBC21)
    GLIBC21="$ac_cv_gnu_library_2_1"
  ]
)
#serial AM2

dnl From Bruno Haible.

AC_DEFUN([AM_ICONV],
[
  dnl Some systems have iconv in libc, some have it in libiconv (OSF/1 and
  dnl those with the standalone portable GNU libiconv installed).

  AC_ARG_WITH([libiconv],
[  --with-libiconv=DIR     search for libiconv in DIR/include and DIR/lib], [
    for dir in `echo "$withval" | tr : ' '`; do
      if test -d $dir/include; then CPPFLAGS="$CPPFLAGS -I$dir/include"; fi
      if test -d $dir/lib; then LDFLAGS="$LDFLAGS -L$dir/lib"; fi
    done
   ])

  AC_CACHE_CHECK(for iconv, am_cv_func_iconv, [
    am_cv_func_iconv="no, consider installing GNU libiconv"
    am_cv_lib_iconv=no
    AC_TRY_LINK([#include <stdlib.h>
#include <iconv.h>],
      [iconv_t cd = iconv_open("","");
       iconv(cd,NULL,NULL,NULL,NULL);
       iconv_close(cd);],
      am_cv_func_iconv=yes)
    if test "$am_cv_func_iconv" != yes; then
      am_save_LIBS="$LIBS"
      LIBS="$LIBS -liconv"
      AC_TRY_LINK([#include <stdlib.h>
#include <iconv.h>],
        [iconv_t cd = iconv_open("","");
         iconv(cd,NULL,NULL,NULL,NULL);
         iconv_close(cd);],
        am_cv_lib_iconv=yes
        am_cv_func_iconv=yes)
      LIBS="$am_save_LIBS"
    fi
  ])
  if test "$am_cv_func_iconv" = yes; then
    AC_DEFINE(HAVE_ICONV, 1, [Define if you have the iconv() function.])
    AC_MSG_CHECKING([for iconv declaration])
    AC_CACHE_VAL(am_cv_proto_iconv, [
      AC_TRY_COMPILE([
#include <stdlib.h>
#include <iconv.h>
extern
#ifdef __cplusplus
"C"
#endif
#if defined(__STDC__) || defined(__cplusplus)
size_t iconv (iconv_t cd, char * *inbuf, size_t *inbytesleft, char * *outbuf, size_t *outbytesleft);
#else
size_t iconv();
#endif
], [], am_cv_proto_iconv_arg1="", am_cv_proto_iconv_arg1="const")
      am_cv_proto_iconv="extern size_t iconv (iconv_t cd, $am_cv_proto_iconv_arg1 char * *inbuf, size_t *inbytesleft, char * *outbuf, size_t *outbytesleft);"])
    am_cv_proto_iconv=`echo "[$]am_cv_proto_iconv" | tr -s ' ' | sed -e 's/( /(/'`
    AC_MSG_RESULT([$]{ac_t:-
         }[$]am_cv_proto_iconv)
    AC_DEFINE_UNQUOTED(ICONV_CONST, $am_cv_proto_iconv_arg1,
      [Define as const if the declaration of iconv() needs const.])
  fi
  LIBICONV=
  if test "$am_cv_lib_iconv" = yes; then
    LIBICONV="-liconv"
  fi
])
#serial 1
# This test replaces the one in autoconf.
# Currently this macro should have the same name as the autoconf macro
# because gettext's gettext.m4 (distributed in the automake package)
# still uses it.  Otherwise, the use in gettext.m4 makes autoheader
# give these diagnostics:
#   configure.in:556: AC_TRY_COMPILE was called before AC_ISC_POSIX
#   configure.in:556: AC_TRY_RUN was called before AC_ISC_POSIX

undefine([AC_ISC_POSIX])

AC_DEFUN([AC_ISC_POSIX],
  [
    dnl This test replaces the obsolescent AC_ISC_POSIX kludge.
    AC_CHECK_LIB(cposix, strerror, [LIBS="$LIBS -lcposix"])
  ]
)
# Check whether LC_MESSAGES is available in <locale.h>.
# Ulrich Drepper <drepper@cygnus.com>, 1995.
#
# This file can be copied and used freely without restrictions.  It can
# be used in projects which are not available under the GNU General Public
# License but which still want to provide support for the GNU gettext
# functionality.
# Please note that the actual code of GNU gettext is covered by the GNU
# General Public License and is *not* in the public domain.

# serial 2

AC_DEFUN([AM_LC_MESSAGES],
  [if test $ac_cv_header_locale_h = yes; then
    AC_CACHE_CHECK([for LC_MESSAGES], am_cv_val_LC_MESSAGES,
      [AC_TRY_LINK([#include <locale.h>], [return LC_MESSAGES],
       am_cv_val_LC_MESSAGES=yes, am_cv_val_LC_MESSAGES=no)])
    if test $am_cv_val_LC_MESSAGES = yes; then
      AC_DEFINE(HAVE_LC_MESSAGES, 1,
        [Define if your <locale.h> file defines LC_MESSAGES.])
    fi
  fi])

AC_DEFUN([EL_CONFIG_OS2],
[
	AC_MSG_CHECKING([for OS/2 threads])

	EL_SAVE_FLAGS
	CFLAGS="$CFLAGS -Zmt"

	AC_TRY_LINK([#include <stdlib.h>],
	            [_beginthread(NULL, NULL, 0, NULL)], cf_result=yes, cf_result=no)
	AC_MSG_RESULT($cf_result)

	if test "$cf_result" = yes; then
		EL_DEFINE(HAVE_BEGINTHREAD, [_beginthread()])
	else
		EL_RESTORE_FLAGS
	fi

	AC_CHECK_FUNC(MouOpen, EL_DEFINE(HAVE_MOUOPEN, [MouOpen()]))
	AC_CHECK_FUNC(_read_kbd, EL_DEFINE(HAVE_READ_KBD, [_read_kbd()]))

	AC_MSG_CHECKING([for XFree for OS/2])

	EL_SAVE_FLAGS

	cf_result=no

	if test -n "$X11ROOT"; then
		CFLAGS="$CFLAGS_X -I$X11ROOT/XFree86/include"
		LIBS="$LIBS_X -L$X11ROOT/XFree86/lib -lxf86_gcc"
		AC_TRY_LINK([#include <pty.h>],
			    [struct winsize win;ptioctl(1, TIOCGWINSZ, &win)],
			    cf_result=yes, cf_result=no)
		if test "$cf_result" = no; then
			LIBS="$LIBS_X -L$X11ROOT/XFree86/lib -lxf86"
			AC_TRY_LINK([#include <pty.h>],
				    [struct winsize win;ptioctl(1, TIOCGWINSZ, &win)],
				    cf_result=yes, cf_result=no)
		fi
	fi

	if test "$cf_result" != yes; then
		EL_RESTORE_FLAGS
	else
		EL_DEFINE(X2, [XFree under OS/2])
	fi

	AC_MSG_RESULT($cf_result)
])
# Search path for a program which passes the given test.
# Ulrich Drepper <drepper@cygnus.com>, 1996.
#
# This file can be copied and used freely without restrictions.  It can
# be used in projects which are not available under the GNU General Public
# License but which still want to provide support for the GNU gettext
# functionality.
# Please note that the actual code of GNU gettext is covered by the GNU
# General Public License and is *not* in the public domain.

# serial 2

dnl AM_PATH_PROG_WITH_TEST(VARIABLE, PROG-TO-CHECK-FOR,
dnl   TEST-PERFORMED-ON-FOUND_PROGRAM [, VALUE-IF-NOT-FOUND [, PATH]])
AC_DEFUN([AM_PATH_PROG_WITH_TEST],
[# Extract the first word of "$2", so it can be a program name with args.
set dummy $2; ac_word=[$]2
AC_MSG_CHECKING([for $ac_word])
AC_CACHE_VAL(ac_cv_path_$1,
[case "[$]$1" in
  /*)
  ac_cv_path_$1="[$]$1" # Let the user override the test with a path.
  ;;
  *)
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:"
  for ac_dir in ifelse([$5], , $PATH, [$5]); do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/$ac_word; then
      if [$3]; then
	ac_cv_path_$1="$ac_dir/$ac_word"
	break
      fi
    fi
  done
  IFS="$ac_save_ifs"
dnl If no 4th arg is given, leave the cache variable unset,
dnl so AC_PATH_PROGS will keep looking.
ifelse([$4], , , [  test -z "[$]ac_cv_path_$1" && ac_cv_path_$1="$4"
])dnl
  ;;
esac])dnl
$1="$ac_cv_path_$1"
if test ifelse([$4], , [-n "[$]$1"], ["[$]$1" != "$4"]); then
  AC_MSG_RESULT([$]$1)
else
  AC_MSG_RESULT(no)
fi
AC_SUBST($1)dnl
])
dnl Thank you very much Vim for this lovely ruby configuration
dnl The hitchhiked code is from Vim configure.in version 1.98


AC_DEFUN([EL_CONFIG_RUBY],
[
AC_MSG_CHECKING([for Ruby])

CONFIG_RUBY_WITHVAL="no"
CONFIG_RUBY="no"

EL_SAVE_FLAGS

AC_ARG_WITH(ruby,
	[  --with-ruby             enable Ruby support],
	[CONFIG_RUBY_WITHVAL="$withval"])

if test "$CONFIG_RUBY_WITHVAL" != no; then
	CONFIG_RUBY="yes"
fi

AC_MSG_RESULT($CONFIG_RUBY)

if test "$CONFIG_RUBY" = "yes"; then
	if test -d "$CONFIG_RUBY_WITHVAL"; then
		RUBY_PATH="$CONFIG_RUBY_WITHVAL:$PATH"
	else
		RUBY_PATH="$PATH"
	fi

	AC_PATH_PROG(CONFIG_RUBY, ruby, no, $RUBY_PATH)
	if test "$CONFIG_RUBY" != "no"; then

		AC_MSG_CHECKING(Ruby version)
		if $CONFIG_RUBY -e '(VERSION rescue RUBY_VERSION) >= "1.6.0" or exit 1' >/dev/null 2>/dev/null; then
			ruby_version=`$CONFIG_RUBY -e 'puts "#{VERSION rescue RUBY_VERSION}"'`
			AC_MSG_RESULT($ruby_version)

			AC_MSG_CHECKING(for Ruby header files)
			rubyhdrdir=`$CONFIG_RUBY -r mkmf -e 'print Config::CONFIG[["archdir"]] || $hdrdir' 2>/dev/null`

			if test "X$rubyhdrdir" != "X"; then
				AC_MSG_RESULT($rubyhdrdir)
				RUBY_CFLAGS="-I$rubyhdrdir"
				rubylibs=`$CONFIG_RUBY -r rbconfig -e 'print Config::CONFIG[["LIBS"]]'`

				if test "X$rubylibs" != "X"; then
					RUBY_LIBS="$rubylibs"
				fi

				librubyarg=`$CONFIG_RUBY -r rbconfig -e 'print Config.expand(Config::CONFIG[["LIBRUBYARG"]])'`

				if test -f "$rubyhdrdir/$librubyarg"; then
					librubyarg="$rubyhdrdir/$librubyarg"

				else
					rubylibdir=`$CONFIG_RUBY -r rbconfig -e 'print Config.expand(Config::CONFIG[["libdir"]])'`
					if test -f "$rubylibdir/$librubyarg"; then
						librubyarg="$rubylibdir/$librubyarg"
					elif test "$librubyarg" = "libruby.a"; then
						dnl required on Mac OS 10.3 where libruby.a doesn't exist
						librubyarg="-lruby"
					else
						librubyarg=`$CONFIG_RUBY -r rbconfig -e "print '$librubyarg'.gsub(/-L\./, %'-L#{Config.expand(Config::CONFIG[\"libdir\"])}')"`
					fi
				fi

				if test "X$librubyarg" != "X"; then
					RUBY_LIBS="$librubyarg $RUBY_LIBS"
				fi

				rubyldflags=`$CONFIG_RUBY -r rbconfig -e 'print Config::CONFIG[["LDFLAGS"]]'`
				if test "X$rubyldflags" != "X"; then
					LDFLAGS="$rubyldflags $LDFLAGS"
				fi

				LIBS="$RUBY_LIBS $LIBS"
				CFLAGS="$RUBY_CFLAGS $CFLAGS"
				CPPFLAGS="$CPPFLAGS $RUBY_CFLAGS"

				AC_TRY_LINK([#include <ruby.h>],
					    [ruby_init();],
					    CONFIG_RUBY=yes, CONFIG_RUBY=no)
			else
				AC_MSG_RESULT([Ruby header files not found])
			fi
		else
			AC_MSG_RESULT(too old; need Ruby version 1.6.0 or later)
		fi
	fi
fi

EL_RESTORE_FLAGS

if test "$CONFIG_RUBY" != "yes"; then
	if test -n "$CONFIG_RUBY_WITHVAL" &&
	   test "$CONFIG_RUBY_WITHVAL" != no; then
		AC_MSG_ERROR([Ruby not found])
	fi
else
	EL_CONFIG(CONFIG_RUBY, [Ruby])

	LIBS="$LIBS $RUBY_LIBS"
	AC_SUBST(RUBY_CFLAGS)
	AC_SUBST(RUBY_LIBS)
fi
])

AC_DEFUN([EL_CONFIG_WIN32],
[
	AC_MSG_CHECKING([for win32 threads])

	EL_SAVE_FLAGS

	AC_TRY_LINK([#include <stdlib.h>],
	            [_beginthread(NULL, NULL, 0, NULL)], cf_result=yes, cf_result=no)
	AC_MSG_RESULT($cf_result)

	if test "$cf_result" = yes; then
		EL_DEFINE(HAVE_BEGINTHREAD, [_beginthread()])
	else
		EL_RESTORE_FLAGS
	fi

	AC_CHECK_HEADERS(windows.h)

	# TODO: Check this?
	# TODO: Check -lws2_32 for IPv6 support
	LIBS="$LIBS -lwsock32"
])
