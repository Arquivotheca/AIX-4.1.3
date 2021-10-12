# @(#)03        1.2  src/bldenv/errlg/errupdate/Makefile, cmderrlg, bos412, GOLDA411a 3/25/93 08:21:46
#
#   COMPONENT_NAME: BOSBUILD
#
#   FUNCTIONS:
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/usr/bin/errlg/errupdate

PROGRAMS	= errupdate
MSGHDRS		= 
CATFILES	= 
HFILES		= ${MSGHDRS} parse.h

ILIST		= errupdate
IDIR		= /usr/bin/

OFILES		= pass1.o crc.o parse.o lex.o main.o \
			  sym.o udbutil.o alertable.o pass2.o etcpy.o parseutil.o


CFLAGS		= -D_JNLS

INCFLAGS	= -I.. -I${MAKETOP}/bos/usr/bin/errlg/errupdate

LIBFLAGS	= -L../liberrlg -L../libras

LIBS		= -lerrlg -lras

.include <${RULES_MK}>
