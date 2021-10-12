# @(#)80	1.7  src/bos/sbin/helpers/v3fshelpers/libfs/Makefile, cmdfs, bos411, 9428A410j 3/29/94 18:03:37
# COMPONENT_NAME:  CMDFS
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
HFILES		= libfs.h fsmacros.h fslimits.h
OFILES		= device.o super.o frag.o inode.o fslimits.o decompress.o
LIBRARIES	= libfs.a
EXPDIR		= /usr/include/libfs/

INCFLAGS	= -I${MAKETOP}/bos/kernel/sys/${TARGET_MACHINE}
INCLUDES	= libfs.h fsmacros.h fslimits.h

EXPINC_TARGETS	= export_libfs.h export_fsmacros.h export_fslimits.h
libfs.a_EXPDIR	= /usr/ccs/lib/
EXPLIB_TARGETS	= export_libfs.a

${LIBRARIES}:	$${.TARGET}(${OFILES} decompress.imp)
		${AR} ${DEF_ARFLAGS} ${.TARGET} ${.OODATE}

.include <${RULES_MK}>
