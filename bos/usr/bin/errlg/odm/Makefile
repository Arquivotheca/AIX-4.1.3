# @(#)65        1.6  src/bos/usr/bin/errlg/odm/Makefile, cmderrlg, bos41J, 9516B_all 4/17/95 13:23:02 
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

SCRIPTS		= notifymeth logsymptom
OTHERS		= errnotify.h

ILIST		= ${SCRIPTS} errnotify errnotify.add errnotify.cre \
		  logsymptom.add
IDIR		= /usr/lib/ras/
errnotify_IDIR	= /etc/objrepos/

INCLUDES	= ${OTHERS}
errnotify.h_IDIR = /usr/include/

NOUPDATE_LIST = \
    /etc/objrepos/errnotify \
    /usr/lib/ras/errnotify.add \
    /usr/lib/ras/logsymptom.add \
    /usr/lib/ras/errnotify.cre


.include <${RULES_MK}>

errnotify:	errnotify.crex
	${RM} -f errnotify
	ODMDIR=. ${ODMCREATE} -p errnotify.crex
	ODMDIR=. ${ODMADD} ${errnotify.add:P} > /dev/null || { ${RM} -f errnotify; false ; }

errnotify.h :	errnotify.crex
	${RM} -f ${.TARGET} ${.TARGET:.h=.c}
	ODMDIR=. ${ODMCREATE} -h errnotify.crex

errnotify.crex :	errnotify.cre
	${CP} ${errnotify.cre:P} ${.TARGET}
