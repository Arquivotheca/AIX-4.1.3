# @(#)63        1.2  src/bos/usr/bin/errlg/errupdate/Makefile, cmderrlg, bos411, 9428A410j 3/3/93 08:45:34
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

PROGRAMS	= errupdate
MSGHDRS		= 
CATFILES	= 
HFILES		= ${MSGHDRS} parse.h

ILIST		= errupdate
IDIR		= /usr/bin/

OFILES		= pass1.o crc.o parse.o lex.o main.o \
			  sym.o udbutil.o alertable.o pass2.o etcpy.o parseutil.o


CFLAGS		= -D_JNLS

INCFLAGS	= -I..

LIBFLAGS	= -L../liberrlg -L../libras

LIBS		= -lerrlg -lras

.include <${RULES_MK}>
