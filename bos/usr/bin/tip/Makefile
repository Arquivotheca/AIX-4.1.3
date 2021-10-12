# @(#)41        1.2  src/bos/usr/bin/tip/Makefile, cmdtip, bos411, 9428A410j 1/22/93 12:06:45
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

SUBDIRS		= aculib

PROGRAMS	= tip
MSGHDRS		= tip_msg.h
CATFILES	= tip.cat
HFILES		= ${MSGHDRS}

INCLUDES	= tip_msg.h
tip_msg.h_EXPDIR = /usr/include/

ILIST		= tip phones-file remote-file

IDIR		= /usr/bin/
phones-file_IDIR = /usr/lib/
remote-file_IDIR = /usr/lib/

CFLAGS		= -DDEFBR=1200 -DDEFFS=BUFSIZ -DACULOG -DPRISTINE -D_BSD

OFILES		= acu.o cmds.o cmdtab.o cu.o hunt.o log.o partab.o \
		  remote.o tip.o tipout.o value.o vars.o acutab.o \
		  remcap.o

remcap.o_CFLAGS	= -DBUFSIZ=1024
acutab.o_CFLAGS	= -DV831 -DVENTEL -DHAYES -DCOURIER

LIBFLAGS	= -Laculib

LIBS		= -laculib

.include <${RULES_MK}>
