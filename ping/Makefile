#	$NetBSD: Makefile,v 1.16 2010/12/13 17:42:17 pooka Exp $
#	@(#)Makefile	8.1 (Berkeley) 6/5/93

USE_FORT?= yes	# setuid
RUMPPRG=ping
MAN=	ping.8
BINOWN=	root
BINMODE=4555
LDADD=	-lm
DPADD=	${LIBM}
CPPFLAGS+=	-DIPSEC
CPPFLAGS+=	-DHAVE_KCN -I/usr/local/include
LDADD+= -lipsec
LDADD+= -Wl,-rpath=/usr/local/lib -L/usr/local/lib -lkcn -ljansson
LDADD+= -Wl,-rpath=/usr/pkg/lib -L/usr/pkg/lib -lcurl
DPADD+= ${LIBIPSEC}

.include <bsd.prog.mk>
