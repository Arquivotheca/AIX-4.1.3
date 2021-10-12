# @(#)34	1.3  src/bos/usr/lib/nls/loc/methods/shared.bidi/Makefile, libbidi, bos411, 9428A410j 11/8/93 19:52:04
#
#   COMPONENT_NAME: LIBBIDI
#
#   FUNCTIONS: none
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

PROGRAMS		= shared.bidi.o
CFLAGS			= -DAIX321
${PROGRAMS}_ENTRYPOINT  = Initiate

INCLUDES		= bdescapes.h
ILIST                   = ${PROGRAMS} ${INCLUDES}
bdescapes.h_IDIR	= /usr/include/bidi/

IDIR                    = /usr/lib/nls/loc/methods/


HFILES			= ${INCLUDES}


OFILES			= BDInit.o blkapifn.o csd.o edit.o getvalues.o \
			  groups.o ics.o inverse.o setvalues.o shape.o \
			  shapebox.o transform.o wcsedit.o wcstransform.o \
			  compose.o

LIBS			= -lc -liconv


.include <${RULES_MK}>
