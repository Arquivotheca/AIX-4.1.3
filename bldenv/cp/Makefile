# @(#)93        1.2  src/bldenv/cp/Makefile, ade_build, bos412, GOLDA411a 1/10/94 15:53:01
#
#   COMPONENT_NAME: BOSBUILD
#
#   FUNCTIONS:
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

VPATH		= ${MAKETOP}bos/usr/bin/cp

PROGRAMS	= cp
MSGHDRS		= cp_msg.h
CATFILES	= cp.cat
HFILES		= ${MSGHDRS}

CFLAGS		=-UMSG -D_BLD -D_${TARGET_MACHINE}

ILIST		= cp
IDIR		= /usr/bin/

LIBS		= -ls

.include <${RULES_MK}>
