# @(#)16        1.3  src/bos/usr/bin/errlg/errclear/Makefile, cmderrlg, bos411, 9428A410j 12/2/93 18:07:02
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

PROGRAMS	= errclear
MSGHDRS		=
CATFILES	=
HFILES		= ${MSGHDRS}

ILIST		= errclear
IDIR		= /usr/bin/

OFILES		= main.o clear.o

INCFLAGS	= -I..

LIBFLAGS	= -L../libras -L../liberrlg

LIBS		= -lras -lerrlg -lodm -lcfg -lrts

.include <${RULES_MK}>
