# @(#)59        1.7  src/bldenv/v3fshelpers/libfs/Makefile, ade_build, bos412, GOLDA411a 4/4/94 15:51:41
#
#   COMPONENT_NAME: BOSBUILD
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

VPATH		= ${MAKETOP}/bos/sbin/helpers/v3fshelpers/libfs

HFILES		= libfs.h fslimits.h fsmacros.h
OFILES          = device.o super.o frag.o inode.o fslimits.o decompress.o
LIBRARIES	= libfs.a
EXPDIR		= /usr/include/libfs/
libfs.a_EXPDIR  = /usr/ccs/lib/
EXPLIB_TARGETS  = export_libfs.a
INCLUDES	= libfs.h fsmacros.h fslimits.h
EXPINC_TARGETS  = export_libfs.h export_fsmacros.h export_fslimits.h

INCFLAGS	= -I${MAKETOP}/bos/sbin/helpers/v3fshelpers \
			-I${MAKETOP}/bos/kernel/sys/${TARGET_MACHINE}

${LIBRARIES}:	$${.TARGET}(${OFILES} decompress.imp)
		${AR} ${DEF_ARFLAGS} ${.TARGET} ${.OODATE}

.include <${RULES_MK}>

LLSEEK_EXIST	!=/usr/bin/nm /usr/lib/libc.a | ${GREP} llseek >/dev/null; \
		  [ $$? -eq 0 ] || ${ECHO} "-D_BLD"

CFLAGS		=${LLSEEK_EXIST}
