# @(#)01	1.6  src/bos/usr/lib/piosk/libR5/Makefile, cmdpiosk, bos411, 9428A410j 2/2/94 15:14:20
#
#   COMPONENT_NAME: CMDPIOSK
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

INCLUDES = ffaccess.h lc.h
ffaccess.h_EXPDIR = /usr/include/
lc.h_EXPDIR = /usr/include/

PIOS = ${MAKETOP}bos/usr/lib/pios
PIOS_KJI = ${MAKETOP}bos/usr/lib/piosk

INCFLAGS = -I${PIOS_KJI}/msg -I${PIOS_KJI}/incR5 -I${PIOS} \
           -I${PIOS}/msg -I${MAKETOP}/bos/usr/include

CFLAGS = -DX11R5BOS -DAIXV3 -DCOMPRESSED_FONTS

LIBRARIES = libfontbos.a libpiosk.a
EXPLIB_TARGETS = export_libfontbos.a export_libpiosk.a 
libfontbos.a_EXPDIR = /usr/lib/
libpiosk.a_EXPDIR = /usr/lib/

libfontbos.a_OFILES = \
	sf2.o bitmap.o bitmapfuncs.o defaults.o format.o \
	pcfread.o snfread.o bitmaputil.o fsatom.o \
	dirfile.o fontdir.o miscutil.o fontxlfd.o fontutil.o fontnames.o

libpiosk.a_OFILES = \
	lcmb.o lc.o sf.o

.include <${RULES_MK}>
