# @(#)12        1.1  src/bos/usr/lib/lpd/plotgbe/Makefile, sysxpsla, bos411, 9428A410j 9/14/92 08:13:20
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

PROGRAMS	= plotgbe

MSGHDRS		= plotgbe_msg.h
CATFILES	= plotgbe.cat
HFILES		= ${MSGHDRS}

OFILES		= plotbkr.o plot_rd.o plot_wr.o plot_or.o plot_msg.o \
		  plot_int.o

LIBS		= -lqb

ILIST		= plotgbe
IDIR		= /usr/lib/lpd/

.include <${RULES_MK}>
