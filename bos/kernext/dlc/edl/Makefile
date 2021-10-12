# @(#)53	1.4  src/bos/kernext/dlc/edl/Makefile, sysxdlce, bos411, 9428A410j 4/1/93 14:56:08
#
# COMPONENT_NAME: (SYSXDLCE) Ethernet Data Link Controls
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
#  Makefile for dlcether
#-----------------------------------------------------------------------

.include <${MAKETOP}bos/kernext/Kernext.mk>

KERNEL_EXT = EDL.ext
IDIR = /usr/lib/drivers/
ILIST = EDL.ext

LIBS=  ${KERNEXT_LIBS} libdlch.a

VPATH := ${VPATH}:../lan

EXPDIR = /usr/include/sys/

CFLAGS = -DEDL -D_KERNEL

EDL.ext_ENTRYPOINT = dlcconfig

EDL.ext_OFILES = lan.o lankproc.o landiscv.o lanhash.o lanmisc.o lansta1.o \
	lansta2.o lansta3.o edlmod.o lanerr.o dump.o


INCFLAGS =  -I${MAKETOP}/bos/kernext/dlc/include -I${MAKETOP}/bos/kernext/dlc/head -I${MAKETOP}/bos/kernext/dlc/lan

.include <${RULES_MK}>

