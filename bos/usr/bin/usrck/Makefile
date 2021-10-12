# @(#)60	1.6  src/bos/usr/bin/usrck/Makefile, Makefiles, bos411, 9428A410j 3/25/94 14:48:58
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

PROGRAMS	= usrck
MSGHDRS		= usrck_msg.h
CATFILES	= usrck.cat
HFILES		= ${MSGHDRS}

ILIST		= usrck
IDIR		= /usr/bin/

OFILES		= comfix.o usrck.o usraudit.o usrauth.o usrdisable.o \
		  usrgroups.o usrhome.o usrinit.o usrlimits.o \
		  usrshell.o usrtpath.o usrttys.o usruser.o usrlogtimes.o \
		  usrlogretries.o usrexpires.o usrpwdwarn.o usrpwdrest.o

LIBS		= -ls

.include <${RULES_MK}>
