# @(#)56        1.3  src/bldenv/odmadd/Makefile, ade_build, bos412, GOLDA411a 2/8/94 17:15:45
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

VPATH		= ${MAKETOP}bos/usr/bin/odmadd

PROGRAMS	= odmadd
MSGHDRS		=
CATFILES	=
HFILES		= ${MSGHDRS}

CFLAGS		= -DMACH -DCMU -D_NO_PROTO
INCFLAGS	= -I${MAKETOP}bos/usr/bin/odmadd

ILIST		= ${PROGRAMS}
IDIR		= /usr/bin/

LIBS		= -lodm

.include <${RULES_MK}>
