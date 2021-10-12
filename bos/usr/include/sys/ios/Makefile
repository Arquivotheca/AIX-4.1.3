# @(#)83	1.3  src/bos/usr/include/sys/ios/Makefile, Makefiles, bos411, 9428A410j 5/24/93 10:20:28
#
#   COMPONENT_NAME: MAKEFILES
#
#   FUNCTIONS: none
#
#   ORIGINS: 10,27
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1988,1989
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/kernel/ios:${MAKETOP}/bos/kernel/ios/${TARGET_MACHINE}

IDIR		= /usr/include/sys/ios/

DATAFILES	= \
	dma_hw.h

INCLUDES	= ${DATAFILES} interrupt.h

ILIST		=

.if exists (${TARGET_MACHINE}/machdep.mk)
.include <${TARGET_MACHINE}/machdep.mk>
.endif

.include <${RULES_MK}>

cfgodm.h: cfgodm.cre
	${CP} ${cfgodm.cre:P} .
	${ODMCREATE} -h -p cfgodm.cre
	${CP} -f cfgodm.cre cfgodm.c 

srcobj.h: srcobj.cre
	${CP} ${srcobj.cre:P} .
	${ODMCREATE} -h -p  srcobj.cre
	${RM} -f srcobj.cre srcobj.c

errids.h: errids.desc
	${CP}${errids.desc:P} .
	${ERRTMPLTBLD} -d errids.desc -o errids.h
	${RM} -f errids.desc
