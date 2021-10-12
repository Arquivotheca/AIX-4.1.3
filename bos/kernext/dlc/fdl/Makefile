# @(#)80	1.4  src/bos/kernext/dlc/fdl/Makefile, sysxdlcf, bos411, 9428A410j 4/1/93 14:57:36
#
# COMPONENT_NAME: (SYSXDLCF) FDDI Data Link Controls
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
#  Makefile for dlcfddi
#-----------------------------------------------------------------------

.include <${MAKETOP}bos/kernext/Kernext.mk>

KERNEL_EXT = FDL.ext
IDIR = /usr/lib/drivers/
ILIST = FDL.ext fdlextcb.h

fdlextcb.h_IDIR = /usr/include/sys/

LIBS=  ${KERNEXT_LIBS} libdlch.a

VPATH := ${VPATH}:../lan

INCLUDES= fdlextcb.h

EXPDIR = /kernel/include/sys/

CFLAGS = -DFDL -D_KERNEL

FDL.ext_ENTRYPOINT = dlcconfig

FDL.ext_OFILES = lan.o lankproc.o landiscv.o lanhash.o lanmisc.o lansta1.o \
	lansta2.o lansta3.o fdlmod.o lanerr.o dump.o

INCFLAGS= -I${MAKETOP}/bos/kernext/dlc/include -I${MAKETOP}/bos/kernext/dlc/head -I${MAKETOP}/bos/kernext/dlc/lan

.include <${RULES_MK}>

