# @(#)02        1.2  src/bldenv/errlg/errinstall/Makefile, cmderrlg, bos412, GOLDA411a 3/25/93 08:10:28
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

VPATH		= ${MAKETOP}/bos/usr/bin/errlg/errinstall

PROGRAMS	= errinstall
MSGHDRS		= 
CATFILES	= 
HFILES		= ${MSGHDRS}

ILIST		= errinstall
IDIR		= /usr/bin/

INCLUDES	= errinstall.h
EXPDIR		= /usr/include/

OFILES		= main.o pass1.o pass2.o

CFLAGS		= -D_JNLS

INCFLAGS	= -I.. -I${MAKETOP}/bos/usr/bin/errlg/errinstall

LIBFLAGS	= -L../libras -L../liberrlg

LIBS		= -lras -lerrlg

.include <${RULES_MK}>

