# @(#)34        1.2  src/bos/usr/include/graphics/rcm/Makefile, rcm, bos411, 9428A410j 5/11/94 18:03:32
#
# COMPONENT_NAME: (rcm)  Kernel extension(s) for RCM
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		=${MAKETOP}/bos/kernext/rcm/inc:${MAKETOP}/bos/kernext/rcm/inc/sys

INSTALL_FILES	= rcm.h  rcm_mac.h  rcm_wg.h  rcm_win.h rcmioctl.h aixgsc.h

INCLUDES	= ${INSTALL_FILES} 	rcmdds.h 

rcmdds.h_EXPDIR	= /usr/include/
EXPDIR		= /usr/include/sys/

IDIR		= /usr/include/sys/
ILIST   	= ${INSTALL_FILES}


.if exists (${TARGET_MACHINE}/machdep.mk)
.include <${TARGET_MACHINE}/machdep.mk>
.endif

.include <${RULES_MK}>
