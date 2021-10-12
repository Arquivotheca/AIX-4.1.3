# @(#)95        1.3  src/bldenv/gos/libfont/Makefile, x11, bos412, GOLDA411a 1/10/94 15:20:00
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

VPATH		= ${MAKETOP}gos/2d/XTOP/fonts/lib/font/bitmap:${MAKETOP}gos/2d/XTOP/fonts/lib/font/fc:${MAKETOP}gos/2d/XTOP/fonts/lib/font/fontfile:${MAKETOP}gos/2d/XTOP/fonts/lib/font/Speedo:${MAKETOP}gos/2d/XTOP/fonts/lib/font/Type1:${MAKETOP}gos/2d/XTOP/fonts/lib/font/util:${MAKETOP}gos/2d/XTOP/fonts/lib/font/include

EXPINC_SUBDIRS	= include
SUBDIRS		= include

LIBRARIES	= libfont.a

EXPLIB_TARGETS	= export_libfont.a
EXPDIR		= /usr/lib/

INCFLAGS	= -Iinclude -I${MAKETOP}bldenv/gos/libfont/include -I${MAKETOP}gos/2d/XTOP/fonts/lib/font/include -I${MAKETOP}gos/2d/XTOP/fonts/include -I${MAKETOP}gos/2d/XTOP/fonts/server/include -I${MAKETOP}gos/2d/XTOP/fonts/lib/font/Type1 -I${MAKETOP}gos/2d/XTOP/fonts/lib/font/Speedo -I${MAKETOP}gos/2d/XTOP/fonts/lib/font/fc -I${MAKETOP}gos/2d/XTOP/fonts/lib/font/bitmap

CFLAGS		= ${FONTSERVERDEFINES}

BITMAP_OFILES	=	\
	bdfread.o	\
	bdfutils.o	\
	bitmap.o	\
	bitmapfuncs.o	\
	bitmaputils.o	\
	bitscale.o	\
	fontink.o	\
	fsfuncs.o	\
	pcfread.o	\
	pcfwrite.o	\
	snfread.o

FONTFILE_OFILES	=	\
	bitsource.o	\
	bufio.o		\
	decompress.o	\
	defaults.o	\
	dirfile.o	\
	fontdir.o	\
	fontfile.o	\
	fontfileio.o	\
	fontfilewr.o	\
	fontscale.o	\
	fontutil.o	\
	renderers.o

FC_OFILES	=	\
	fsconvert.o	\
	fserve.o	\
	fsio.o

SPEEDO_OFILES	=	\
	do_char.o	\
	do_trns.o	\
	out_bl2d.o	\
	out_blk.o	\
	out_scrn.o	\
	out_util.o	\
	reset.o		\
	set_spcs.o	\
	set_trns.o	\
	spencode.o	\
	sperr.o		\
	spfile.o	\
	spfont.o	\
	spfuncs.o	\
	spglyph.o	\
	spinfo.o

TYPE1_OFILES	=	\
	arith.o		\
	hints.o		\
	objects.o	\
	scanfont.o	\
	t1info.o	\
	t1snap.o	\
	token.o		\
	curves.o	\
	lines.o		\
	paths.o		\
	spaces.o	\
	t1io.o		\
	t1stub.o	\
	type1.o		\
	fontfcn.o	\
	regions.o	\
	t1funcs.o	\
	t1malloc.o	\
	util.o		\
	encodings.o

UTIL_OFILES	=	\
	fsatom.o	\
	bitmaputil.o	\
	fontaccel.o	\
	fontnames.o	\
	fntutil.o	\
	fontxlfd.o	\
	format.o	\
	miscutil.o	\
	patcache.o	\
	private.o

OFILES	=			\
	${BITMAP_OFILES}	\
	${FONTFILE_OFILES}	\
	${FC_OFILES}		\
	${SPEEDO_OFILES}	\
	${TYPE1_OFILES}		\
	${UTIL_OFILES}

.include <${MAKETOP}gos/gos.mk>

.include <${RULES_MK}>
