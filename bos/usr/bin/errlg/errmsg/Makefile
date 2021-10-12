# @(#)18        1.1  src/bos/usr/bin/errlg/errmsg/Makefile, cmderrlg, bos411, 9428A410j 4/16/93 16:03:52

# COMPONENT_NAME: cmderrlg
#
# FUNCTIONS: Makefile for errmsg 
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

PROGRAMS	= errmsg
MSGHDRS		= 
CATFILES	= 
HFILES		= ${MSGHDRS}

ILIST		= errmsg
IDIR		= /usr/bin/

OFILES		= main.o pass1.o pass2.o

INCFLAGS	= -I..

LIBS		= -lras -lerrlg

.include <${RULES_MK}>
