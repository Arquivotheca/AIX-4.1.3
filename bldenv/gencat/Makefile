# @(#)74	1.3  src/bldenv/gencat/Makefile, ade_build, bos412, GOLDA411a 1/10/94 13:27:58
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

VPATH		= ${MAKETOP}bos/usr/bin/gencat

PROGRAMS	= gencat
MSGHDRS		= msgfac_msg.h
CATFILES	= msgfac.cat
HFILES		= ${MSGHDRS}
INCFLAGS	= -I${MAKETOP}bos/usr/bin/gencat

ILIST		= gencat
IDIR		= /usr/bin/

OFILES		= gencat.o catio.o

.include <${RULES_MK}>
