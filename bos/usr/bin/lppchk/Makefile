# @(#)75        1.3  src/bos/usr/bin/lppchk/Makefile, cmdswvpd, bos411, 9428A410j 3/31/94 21:27:28
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
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

VPATH		=../lslpp

PROGRAMS	= lppchk
MSGHDRS		= swvpd_msg.h
CATFILES	= swvpd.cat
HFILES		= ${MSGHDRS}

ILIST		= lppchk
IDIR		= /usr/bin/

INCFLAGS	=-I../lslpp -I../../sbin/install/include -I../../sbin/install/ckprereq -I../../sbin/install/messages  -I../../sbin/install/sysck
OFILES		= \
		lppchk.o lppchkd.o lppchkf.o lppchkl.o lppchkv.o \
		lppchkva.o lppchkvf.o lppchkck.o lppchkcp.o lppchkbl.o \
		swvpd_str.o

LIBS		= -lswvpd -lodm -lckprereq -linulib

.include <${RULES_MK}>
