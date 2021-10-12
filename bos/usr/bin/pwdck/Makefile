# @(#)58        1.4  src/bos/usr/bin/pwdck/Makefile, Makefiles, bos411, 9428A410j 3/25/94 14:48:55
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/usr/sbin/grpck

PROGRAMS	= pwdck
MSGHDRS		= pwdck_msg.h
CATFILES	= pwdck.cat
HFILES		= ${MSGHDRS}

ILIST		= pwdck
IDIR		= /usr/bin/

OFILES		= pwdck.o pwdbuild.o pwdckauth.o pwdckuser.o pwdutil.o \
		  pwdwrite.o comfix.o

LIBS		= -ls

.include <${RULES_MK}>
