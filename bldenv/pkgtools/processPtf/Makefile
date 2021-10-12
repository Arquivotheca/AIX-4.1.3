# @(#)51	1.3  src/bldenv/pkgtools/processPtf/Makefile, pkgtools, bos412, GOLDA411a 4/15/94 13:34:40
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

PROGRAMS		= processPtf
INCFLAGS		+= -I${MAKETOP}bldenv/pkgtools/common
COMMON_OFILES		= ../common/util.o ../common/vrmf.o ../common/processPtfmsg.o
processPtf_OFILES	= processPtf.o ptfReqs.o ptfLists.o tree.o ${COMMON_OFILES}

ILIST           	= ${PROGRAMS}
IDIR            	= /usr/bin/

.include <${RULES_MK}>

${COMMON_OFILES} :
