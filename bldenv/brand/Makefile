# @(#)90        1.5  src/bldenv/brand/Makefile, bldprocess, bos412, GOLDA411a 1/10/94 13:30:18
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

PROGRAMS		= brand

SCRIPTS			= brandcfg.nls

ILIST           	= ${PROGRAMS} ${SCRIPTS}
IDIR            	= /usr/bin/
brandcfg.nls_IDIR 	= /usr/lib/

.include <${RULES_MK}>

brandcfg.nls : brandcfg.nls.S
	${RM} ${_RMFLAGS_} ${.TARGET}
	${CP} ${brandcfg.nls.S:P} ${.TARGET}
