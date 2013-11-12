/*	$NetBSD: ping_rumpops.c,v 1.2 2011/03/11 09:59:56 pooka Exp $	*/

/*
 * Copyright (c) 2010 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <sys/cdefs.h>
#ifndef lint
__RCSID("$NetBSD: ping_rumpops.c,v 1.2 2011/03/11 09:59:56 pooka Exp $");
#endif /* !lint */

#include <sys/types.h>
#include <sys/socket.h>

#include <poll.h>
#include <unistd.h>

#include <rump/rump.h>
#include <rump/rump_syscalls.h>
#include <rump/rumpclient.h>

#include "prog_ops.h"

const struct prog_ops prog_ops = {
	.op_init =	rumpclient_init,

	.op_socket =	rump_sys_socket,
	.op_setsockopt=	rump_sys_setsockopt,
	.op_shutdown =	rump_sys_shutdown,
	.op_poll =	rump_sys_poll,
	.op_sendto =	rump_sys_sendto,
	.op_recvfrom =	rump_sys_recvfrom,
	.op_close =	rump_sys_close,
	.op_getuid =	rump_sys_getuid,
	.op_setuid =	rump_sys_setuid,
};
