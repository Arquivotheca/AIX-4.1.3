# @(#)55       1.1  src/bldenv/gos/xcpp/Makefile, ade_build, bos412, GOLDA411a 3/17/94 19:57:58
#
# COMPONENT_NAME: X11
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1987, 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#

VPATH		= ${MAKETOP}/gos/2d/XTOP/util/xcpp

PROGRAMS	= cpp

OFILES          = CSlocc.o CSlen.o CScmp.o CScpy.o CSgetl.o \
		  CAcpy.o CSskps.o CSsname.o CScpyum.o CAfill.o \
	   	  cpy.o cpp.o

CFLAGS          = -D_BSD -DCXREF

INCFLAGS	= -I${MAKETOP}/gos/2d/XTOP/util/xcpp


ILIST		= cpp
IDIR		= /usr/lpp/X11/Xamples/util/cpp/

.include <${RULES_MK}>
