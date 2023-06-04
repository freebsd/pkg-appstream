# Copyright (c) 2023 Serenity Cyber Security, LLC
# Author: Gleb Popov <arrowd@FreeBSD.org>
# SPDX-License-Identifier: BSD-2-Clause

.include <bsd.own.mk>

PREFIX?=	/usr/local
.ifndef LOCALBASE
LIBPKG_CFLAGS?=	-I/usr/local/include
.endif

CFLAGS+=	${LIBPKG_CFLAGS}
LIBDIR=		${PREFIX}/lib/pkg
SHLIB_NAME=	${PLUGIN_NAME}.so

PLUGIN_NAME=	appstream
SRCS=		appstream.c

.include <bsd.lib.mk>
