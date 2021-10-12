# @(#)41        1.6  src/bos/usr/lib/objrepos/swvpd/Makefile, libswvpd, bos411, 9440A411d 10/5/94 13:12:37
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

SCRIPTS		= swvpd.add

OTHERS		= lpp lpp.vc product product.vc history history.vc \
		  inventory inventory.vc fix fix.vc

ILIST		= ${OTHERS}
IDIR		= /usr/lib/objrepos/

NOUPDATE_LIST   = /usr/lib/objrepos/lpp \
		  /usr/lib/objrepos/lpp.vc \
		  /usr/lib/objrepos/product \
		  /usr/lib/objrepos/product.vc \
		  /usr/lib/objrepos/history \
		  /usr/lib/objrepos/history.vc \
		  /usr/lib/objrepos/fix \
		  /usr/lib/objrepos/fix.vc \
		  /usr/lib/objrepos/inventory \
		  /usr/lib/objrepos/inventory.vc

.include <${RULES_MK}>

${OTHERS} :	swvpd.make

swvpd.make :	swvpd.cre swvpd.add
	${RM} -f ${OTHERS} 
	${ECHO} creating swvpd
	${ODMCREATE} -c ${swvpd.cre:P}
	${ODMADD} ${swvpd.add:P}
	${TOUCH} swvpd.make
