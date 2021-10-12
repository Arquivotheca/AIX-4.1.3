# @(#)85        1.4  src/bldenv/as/Makefile, ade_build, bos412, GOLDA411a 12/4/93 16:23:16
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

VPATH		= ${MAKETOP}/bos/usr/ccs/bin/as:${MAKETOP}/bos/usr/ccs/bin/as/${TARGET_MACHINE}

PROGRAMS	= as
MSGHDRS		= as_msg.h
CATFILES	= as.cat

ILIST		= ${PROGRAMS}
IDIR		= /usr/bin/

INCFLAGS	= -I${MAKETOP}/bos/usr/ccs/bin/as -I${MAKETOP}/bos/usr/ccs/bin/as/${TARGET_MACHINE}
CFLAGS		=
CC_OPT_LEVEL	=-g
HFILES		= ${MSGHDRS} as0.h

OFILES		= as0.o   as1.o as2.o   as3.o   asst.o \
		  aspso.o asl.o aslst.o asomf.o asmsgs.o

LIBS		= -lrts 

.include <${RULES_MK}>
