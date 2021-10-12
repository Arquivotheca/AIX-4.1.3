# @(#)62        1.3  src/bos/usr/bin/errlg/errpt/Makefile, cmderrlg, bos411, 9428A410j 12/2/93 18:06:32
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

VPATH		= ${MAKETOP}/bos/diag/lscfg

PROGRAMS	= errpt
MSGHDRS		= lscfg_msg.h
CATFILES	= 
HFILES		= ${MSGHDRS} parse.h

ILIST		= errpt
IDIR		= /usr/bin/

OFILES		= pr.o rpt.o vpdcfg.o main.o

INCFLAGS	= -I.. -Idiag

LIBFLAGS	= -L../libras -L../libras

LIBS		= -lras -lerrlg -lodm -lcfg -lrts

.include <${RULES_MK}>
