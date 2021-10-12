# @(#)85	1.2  src/bos/kernext/dlc/head/Makefile, sysxdlcg, bos411, 9428A410j 2/25/93 15:49:45
#
# COMPONENT_NAME: (SYSXDLCG) Data Link Control Head Code
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

.include <${MAKETOP}bos/kernext/Kernext.mk>

LIBRARIES = libdlch.a

EXPLIB_TARGETS = export_libdlch.a

EXPDIR = /kernel/lib/

OFILES =   \
	dlcrq.o \
	dlcclose.o \
	dlcconfig.o \
	dlcinte.o \
	dlcintr.o \
	dlcioctl.o \
	dlcmpx.o \
	dlcopen.o \
	dlcread.o \
	dlcselect.o \
	dlcwrite.o

INCFLAGS = -I${MAKETOP}/bos/kernext/dlc/include

.include <${RULES_MK}>

