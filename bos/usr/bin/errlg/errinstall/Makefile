# @(#)60        1.3  src/bos/usr/bin/errlg/errinstall/Makefile, cmderrlg, bos411, 9428A410j 4/16/93 16:23:10

# COMPONENT_NAME: Makefile
#
# FUNCTIONS: Makefile for errinstall
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

PROGRAMS	= errinstall
MSGHDRS		= 
CATFILES	= 
HFILES		= ${MSGHDRS}

ILIST		= errinstall
IDIR		= /usr/bin/

OFILES		= main.o pass1.o pass2.o

INCFLAGS	= -I..

LIBS		= -lras -lerrlg

.include <${RULES_MK}>
