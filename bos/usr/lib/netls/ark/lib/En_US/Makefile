# @(#)08	1.2  src/bos/usr/lib/netls/ark/lib/En_US/Makefile, libnetls, bos411, 9428A410j 4/22/94 16:59:21
#
#   COMPONENT_NAME: libnetls
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
OTHERS += netls.cat
netls.cat: netls.cat.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
