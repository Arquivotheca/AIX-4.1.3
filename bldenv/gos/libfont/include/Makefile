# @(#)96        1.2  src/bldenv/gos/libfont/include/Makefile, x11, bos412, GOLDA411a 12/10/93 16:39:24
#
# COMPONENT_NAME: x11
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

VPATH		= ${MAKETOP}/gos/2d/XTOP/fonts/lib/font/include:${MAKETOP}/gos/2d/XTOP/fonts/include

EXPDIR		= /usr/include/X11/

MSGHDRS		= fontlib_msg.h
CATFILES	= fontlib.cat

ILIST		= fontlib.cat	
IDIR		= /usr/lib/nls/msg/En_US/
IMODE		= 644

INCLUDES	= 	     \
		${MSGHDRS}   \
		FSproto.h    \
		bitmap.h     \
		bufio.h	     \
		fontfile.h   \
		fontfileio.h \
		fontfilest.h \
		fontmisc.h   \
		fontshow.h   \
		fontxlfd.h

.include <${MAKETOP}/gos/gos.mk>

.include <${RULES_MK}>

