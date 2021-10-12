# @(#)85	1.2  src/bos/usr/lib/ncs/lib/En_US/Makefile, libncs, bos411, 9428A410j 4/22/94 16:59:26
#
#   COMPONENT_NAME: libncs
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


OTHERS =
OTHERS += ncs.cat
ncs.cat: ncs.cat.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
