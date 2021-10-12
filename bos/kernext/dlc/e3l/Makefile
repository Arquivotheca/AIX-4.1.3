# @(#)08	1.4  src/bos/kernext/dlc/e3l/Makefile, sysxdlci, bos411, 9428A410j 4/1/93 14:54:49
#
# COMPONENT_NAME: (SYSXDLCI) IEEE 802.3 Data Link Controls
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
#  Makefile for dlc802.3
#-----------------------------------------------------------------------

.include <${MAKETOP}bos/kernext/Kernext.mk>

KERNEL_EXT = E3L.ext
IDIR = /usr/lib/drivers/
ILIST = E3L.ext

LIBS=  ${KERNEXT_LIBS} libdlch.a

VPATH := ${VPATH}:../lan

EXPDIR = /usr/include/sys/

CFLAGS = -DE3L -D_KERNEL

E3L.ext_ENTRYPOINT = dlcconfig

E3L.ext_OFILES = lan.o lankproc.o landiscv.o lanhash.o lanmisc.o lansta1.o \
	lansta2.o lansta3.o e3lmod.o lanerr.o dump.o


INCFLAGS =  -I${MAKETOP}/bos/kernext/dlc/include -I${MAKETOP}/bos/kernext/dlc/head -I${MAKETOP}/bos/kernext/dlc/lan

.include <${RULES_MK}>

