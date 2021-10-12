# @(#)27	1.2  src/bldenv/pkgtools/updatefixdata/Makefile, pkgtools, bos412, GOLDA411a 7/8/94 16:39:41
#
#   COMPONENT_NAME: PKGTOOLS
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992,1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

SCRIPTS			= updatefixdata
PROGRAMS		= buildfixdata addfixdata
INCFLAGS		+= -I${MAKETOP}bldenv/pkgtools/common
COMMON_OFILES		= ../common/util.o
buildfixdata_OFILES	= buildfixdata.o ${COMMON_OFILES}
addfixdata_OFILES	= addfixdata.o ${COMMON_OFILES}

ILIST           	= ${PROGRAMS} ${SCRIPTS}
IDIR            	= /usr/bin/

.include <${RULES_MK}>
