# @(#)84	1.4  src/bos/usr/bin/que/rem/Makefile, cmdque, bos411, 9428A410j 1/30/93 09:40:17
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

PROGRAMS	= rembak lpd
MSGHDRS		= rem_msg.h lpd_msg.h
CATFILES	= rem.cat lpd.cat
HFILES		= ${MSGHDRS}
INCFLAGS	= -I../

ILIST		= rembak lpd
IDIR		= /usr/lib/lpd/
lpd_IDIR	= /usr/sbin/

rembak_OFILES	= rembak.o commonr.o cf.o debugr.o paramr.o

lpd_OFILES	= lpd.o commonr.o debugr.o dispatch.o

LIBFLAGS	= -L../libque

LIBS		= -lque -ls -lqb -lsrc

.include <${RULES_MK}>
