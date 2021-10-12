# @(#)23        1.3  src/bldenv/m4/Makefile, ade_build, bos412, GOLDA411a 2/8/94 17:17:33
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

VPATH		= ${MAKETOP}bos/usr/ccs/bin/m4

PROGRAMS	= m4
MSGHDRS		= m4_msg.h
CATFILES	= m4.cat

INCFLAGS	= -I${MAKETOP}bos/usr/ccs/bin/m4

ILIST		= ${PROGRAMS}
IDIR		= /usr/bin/

OFILES		= m4.o m4y.o m4ext.o m4macs.o

.include <${RULES_MK}>

m4.o m4ext.o m4macs.o: m4.h m4_msg.h
