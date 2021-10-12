# @(#)24        1.1  src/bldenv/lint/pass2/Makefile, ade_build, bos412, GOLDA411a 2/17/93 08:26:20
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

VPATH		= ${MAKETOP}/bos/usr/ccs/bin/lint/pass2:${MAKETOP}/bos/usr/ccs/bin/common

PROGRAMS	= lint2
MSGHDRS		= lint_msg.h ctools_msg.h
CATFILES	= lint.cat ctools.cat
HFILES		= ${MSGHDRS}

ILIST		= lint2
IDIR		= /usr/lib/

INCFLAGS	= -I${MAKETOP}/bos/usr/ccs/bin/common -I${MAKETOP}/bos/usr/ccs/bin/lint/pass2
CFLAGS		= -DHOSTIEEE -DLINTP2

OFILES		= \
	hasher.o lint2.o main.o output.o reader.o storage.o tftn.o \
	xdefs.o

.include <${RULES_MK}>
