# @(#)09	1.7  src/bos/usr/lib/nls/charmap/Makefile, cfgnls, bos41B, 9504A 12/16/94 16:57:44
#
#   COMPONENT_NAME: CFGNLS
#
#   FUNCTIONS:		Makefile for charmaps
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993,1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

CHARMAPS = \
	IBM-850 \
	IBM-856 \
	IBM-932 \
	IBM-1046 \
	IBM-eucJP \
	IBM-eucKR \
	IBM-eucCN \
	IBM-eucTW \
	ISO8859-1 \
	ISO8859-2 \
	ISO8859-5 \
	ISO8859-6 \
	ISO8859-7 \
	ISO8859-8 \
	ISO8859-9 \
	ZH_CN

OTHERS = ZH_CN

ILIST = ${CHARMAPS}
IDIR = /usr/lib/nls/charmap/

.include <${RULES_MK}>

${OTHERS} : 	UTF-8.cm CN.csid
	${CAT} ${.ALLSRC} > ${.TARGET}

