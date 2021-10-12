# @(#)59        1.2  src/bos/usr/bin/errlg/errdemon/Makefile, cmderrlg, bos411, 9428A410j 3/1/93 09:08:40
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= errdemon
MSGHDRS		= 
CATFILES	= 
HFILES		= ${MSGHDRS}

ILIST		= errdemon
IDIR		= /usr/lib/

OFILES		= main.o

INCFLAGS	= -I..

LIBFLAGS	= -L../libras -L../liberrlg

LIBS		= -lras -lerrlg -lodm -lrts -lcfg

.include <${RULES_MK}>
