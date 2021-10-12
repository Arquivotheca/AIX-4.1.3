# @(#)95	1.3  src/bos/usr/lib/piosCN/lib/Makefile, ils-zh_CN, bos41J, 9516A_all 4/17/95 15:24:37
#
#   COMPONENT_NAME: ils-zh_CN
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

LIBRARIES = libfontbosCN.a

#INCFLAGS = -I${MAKETOP}/gos/2d/XTOP/fonts/lib/font/include -I${MAKETOP}/gos/2d/XTOP/fonts/lib/font/bitmap -I${MAKETOP}/gos/2d/XTOP/fonts/include

CFLAGS = -DX11R5BOS -DCOMPRESSED_FONTS

OFILES = \
        bitmap.o \
        bitmapfuncs.o \
        dirfile.o \
	fontdir.o \
	lc.o \
	lct.o \
	miscutil.o \
	pcfread.o \
	snfread.o \
	sf.o \
	shrink.o

EXPLIB_TARGETS = \
	export_libfontbosCN.a

EXPDIR = /usr/ccs/lib/

.include <${RULES_MK}>
