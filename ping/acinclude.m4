dnl autoconf KCN macros
# ----------------------------------------------------------------
# AC_CHECK_KCN
# ----------------------------------------------------------------
AC_DEFUN([AC_CHECK_KCN],
	[AC_MSG_CHECKING(for KCN library)
	 AC_ARG_WITH(kcn,
	     [  --with-kcn[=PREFIX]       support KCN],,
	     [with_kcn="yes"])

	AC_MSG_RESULT($with_kcn)

	if test x"$with_kcn" != xno; then
		AC_MSG_CHECKING(if KCN API is available)
		if test x"$with_kcn" = xyes; then
			with_kcn='/usr/local /usr/pkg /usr'
		fi
		kcndir=''
		for dir in $with_kcn; do
			if test -f $dir/include/kcn_info.h; then
				kcndir=$dir
				break
			fi
		done
		if test x"$kcndir" = x; then
			AC_MSG_ERROR([KCN library not found.])
		fi
		KCN_LIBS=""
		if test $kcndir != '/usr'; then
			KCN_CFLAGS="-I$kcndir/include"
			KCN_LIBS="-Wl,-rpath=$kcndir/lib -L$kcndir/lib"
		fi
		KCN_LIBS="$KCN_LIBS -lkcn"
		AC_MSG_RESULT(yes, $kcndir)
	fi])
