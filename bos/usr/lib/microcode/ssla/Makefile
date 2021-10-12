# @(#)54        1.3  src/bos/usr/lib/microcode/ssla/Makefile, ucodssla, bos411, 9428A410j 10/11/93 15:48:15
#
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


ILIST		= 8787S.00.01

IDIR		= /usr/lib/microcode/


OTHERS=

OTHERS+=8787S.00.01
8787S.00.01:8787S.00.01.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>

