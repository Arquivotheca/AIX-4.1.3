# @(#)97       1.2  src/bldenv/gos/mkfontdir/Makefile, x11, bos412, GOLDA411a 4/21/93 18:34:40
#
# COMPONENT_NAME: xinstall
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1987, 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		=${MAKETOP}/gos/2d/XTOP/fonts/clients/mkfontdir:${MAKETOP}/gos/2d/XTOP/include

PROGRAMS	= mkfontdir

CFLAGS          = ${FONTSERVERDEFINES}

INCLUDES        = Xfuncs.h Xosdefs.h Xos.h Xproto.h Xmd.h Xprotostr.h X.h
EXPDIR          = /usr/include/X11/

MSGHDRS		= mkfontdir_msg.h
HFILES		= ${MSGHDRS}
CATFILES	= mkfontdir.cat

ILIST		= ${PROGRAMS} mkfontdir.cat
IDIR		= /usr/lpp/X11/bin/

INCFLAGS	= -I${MAKETOP}/gos/2d/XTOP/fonts/clients/mkfontdir -I${MAKETOP}/gos/2d/XTOP/include -I${MAKETOP}/gos/2d/XTOP/fonts/lib/font/include -I${MAKETOP}/gos/2d/XTOP/fonts/include

LIBS		= -lfont

.include <${MAKETOP}/gos/gos.mk>

.include <${RULES_MK}>
