# @(#)14	1.2  src/bldenv/loc/iconvTable/Makefile, cmdiconv, bos412, GOLDA411a 4/25/94 16:50:42
#
#   COMPONENT_NAME: CMDICONV
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

VPATH		= ${MAKETOP}/bos/usr/lib/nls/loc/iconvTable

OTHERS		= \
	IBM-850_ISO8859-1 \
	ISO8859-1_IBM-850

ILIST		= ${OTHERS}
IDIR		= ${LOCPATH}/iconvTable/

.include <${RULES_MK}>

${OTHERS} : $${.TARGET:=_src}
	${GENXLT} ${.TARGET} < ${.ALLSRC}
