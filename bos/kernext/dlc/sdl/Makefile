# @(#)82	1.5  src/bos/kernext/dlc/sdl/Makefile, sysxdlcs, bos411, 9428A410j 3/3/94 15:35:56
#
# COMPONENT_NAME: (SYSXDLCS) SDLC Data Link Controls
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#----------------------------------------------------------------------
#  Makefile for dlcsdlc
#-----------------------------------------------------------------------

.include <${MAKETOP}bos/kernext/Kernext.mk>

KERNEL_EXT = SDL.ext
IDIR = /usr/lib/drivers/
ILIST = SDL.ext sdlextcb.h

sdlextcb.h_IDIR = /usr/include/sys/

LIBS=  ${KERNEXT_LIBS} libdlch.a

VPATH := ${VPATH}:../head

INCLUDES= sdlextcb.h

EXPDIR = /kernel/include/sys/

CFLAGS = -D_KERNEL -D_AIXV3_POSTWAIT

SDL.ext_ENTRYPOINT = dlcconfig

SDL.ext_OFILES = sdlcommon.o sdlerr.o sdlioctl.o sdlkproc.o sdlopen.o \
	sdlpri.o sdlsec.o sdlwrite.o


INCFLAGS =  -I${MAKETOP}/bos/kernext/dlc/include -I${MAKETOP}/bos/kernext/dlc/head -I${MAKETOP}/bos/kernext/dlc/lan

.include <${RULES_MK}>

