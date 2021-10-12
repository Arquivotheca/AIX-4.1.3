# @(#)05        1.3  src/bldenv/errlg/odm/Makefile, cmderrlg, bos412, GOLDA411a 1/10/94 13:40:39
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

VPATH		= ${MAKETOP}bos/usr/bin/errlg/odm

OTHERS		= errnotify.h

INCLUDES	= ${OTHERS}
errnotify.h_IDIR = /usr/include/

.include <${RULES_MK}>

errnotify.h : errnotify.crex
	${RM} ${_RMFLAGS_} ${.TARGET}
	ODMDIR=. ${ODMCREATE} -h errnotify.crex

errnotify.crex : errnotify.cre
	${RM} ${_RMFLAGS_} ${.TARGET}
	${CP} ${errnotify.cre:P} ${.TARGET}
