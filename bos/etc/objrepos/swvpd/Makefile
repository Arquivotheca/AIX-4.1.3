# @(#)58        1.5  src/bos/etc/objrepos/swvpd/Makefile, cmdswvpd, bos411, 9434A411a 8/18/94 16:26:55
# COMPONENT_NAME:  MAKEFILES
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

VPATH		= ${MAKETOP}/bos/usr/lib/objrepos/swvpd

SCRIPTS		= swvpd.add

BUILD_LIST	= history history.vc inventory inventory.vc \
		  lpp lpp.vc product product.vc

OTHERS		= swvpd.make

ILIST		= ${BUILD_LIST}

IDIR		= /etc/objrepos/

NOUPDATE_LIST	= /etc/objrepos/lpp \
		  /etc/objrepos/lpp.vc \
		  /etc/objrepos/product \
		  /etc/objrepos/product.vc \
		  /etc/objrepos/history \
		  /etc/objrepos/history.vc \
		  /etc/objrepos/inventory \
		  /etc/objrepos/inventory.vc

.include <${RULES_MK}>

swvpd.make :	swvpd.cre swvpd.add
	@${RM} -f ${BUILD_LIST}
	${CP} ${swvpd.cre:P} swvpd.cre.tmp
	(ODMDIR=. ; ${ODMCREATE} -c swvpd.cre.tmp )
	@${RM} -f swvpd.cre.tmp
	(ODMDIR=. ; ${ODMADD} ${swvpd.add:P} )
	${TOUCH} swvpd.make
