# @(#)17        1.2  src/bldenv/keycomp/Makefile, ade_build, bos412, GOLDA411a 7/8/93 20:36:45
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

VPATH		= ${MAKETOP}/bos/usr/bin/keycomp

PROGRAMS	= keycomp
MSGHDRS		= keycomp_msg.h
CATFILES	= keycomp.cat
HFILES		= ${MSGHDRS}

ILIST		= keycomp
IDIR		= /usr/bin/

YFLAGS		= -Nm200000
INCFLAGS	= -I${VPATH}

OFILES		= keycomp.o CompileCompose.o Layer.o ComposeTable.o \
			Item.o Rule.o

LIBS		= -ll -ly

.include <${RULES_MK}>

Item.o : Rule.h
