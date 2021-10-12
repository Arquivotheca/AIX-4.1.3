# @(#)89        1.2  src/bos/usr/ccs/lib/libPW/Makefile, libPW, bos411, 9428A410j 9/1/92 19:21:55
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 10,27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

LIBRARIES       = libPW.a
MSGHDRS         = pw_msg.h
CATFILES        = pw.cat
HFILES          = ${MSGHDRS}

EXPLIB_TARGETS  = export_libPW.a

ILIST           = libPW.a
IDIR            = /usr/lib/

OFILES = \
	abspath.o any.o anystr.o bal.o cat.o clean.o \
	curdir.o data.o dname.o fatal.o fdfopen.o \
	giveup.o imatch.o lockit.o logname.o move.o \
	patoi.o patol.o regcmp.o regex.o repeat.o \
	repl.o satoi.o setsig.o sname.o strend.o \
	substr.o trnslat.o userdir.o userexit.o \
	username.o verify.o xalloc.o xcreat.o \
	xlink.o xmsg.o xopen.o xpipe.o xunlink.o \
	xwrite.o zero.o zeropad.o

.include <${RULES_MK}>
