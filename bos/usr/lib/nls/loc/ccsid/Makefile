# @(#)85	1.2  src/bos/usr/lib/nls/loc/ccsid/Makefile, libiconv, bos411, 9428A410j 12/17/93 08:21:50
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
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

OTHERS	= ccsid.tbl

ILIST		= ${OTHERS} ccsid.src
IDIR		= /usr/lib/nls/loc/ccsid/

.include <${RULES_MK}>

${OTHERS} : ccsid.src
	${GENXLT} -ccsid < ${ccsid.src:P} > ${OTHERS}
