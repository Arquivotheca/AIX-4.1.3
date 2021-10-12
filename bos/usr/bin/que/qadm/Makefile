# @(#)85	1.2  src/bos/usr/bin/que/qadm/Makefile, cmdque, bos411, 9428A410j 1/29/93 12:57:14
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= lsque lsquedev mkque mkquedev rmque rmquedev chque \
		  chquedev lsallq lsallqdev
MSGHDRS		= qcadm_msg.h
CATFILES	= qcadm.cat
HFILES		= ${MSGHDRS}
INCFLAGS	= -I../

ILIST		= ${PROGRAMS}
IDIR		= /usr/bin/

lsque_OFILES	= lsque.o qccom.o
lsquedev_OFILES	= lsquedev.o qccom.o
mkque_OFILES	= mkque.o qccom.o
mkquedev_OFILES	= mkquedev.o qccom.o
rmque_OFILES	= rmque.o qccom.o
rmquedev_OFILES	= rmquedev.o qccom.o
chque_OFILES	= chque.o qccom.o
chquedev_OFILES	= chquedev.o qccom.o
lsallq_OFILES	= lsallq.o qccom.o
lsallqdev_OFILES = lsallqdev.o qccom.o

LIBFLAGS	= -L. -L../libque

LIBS		= -lque -ls -lqb

.include <${RULES_MK}>
