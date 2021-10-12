# @(#)20        1.2  src/bos/usr/bin/errlg/errdead/Makefile, cmderrlg, bos411, 9428A410j 12/3/93 10:50:21
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= errdead
MSGHDRS		= 
CATFILES	= 
HFILES		= ${MSGHDRS}

ILIST		= errdead
IDIR		= /usr/lib/

OFILES		= main.o

INCFLAGS	= -I..

LIBFLAGS	= -L../libras -L../liberrlg

LIBS		= -lras -lerrlg -lodm -lcfg -lrts

.include <${RULES_MK}>
