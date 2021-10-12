# @(#)62        1.2  src/bldenv/tar/Makefile, ade_build, bos412, GOLDA411a 1/10/94 15:53:57
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

VPATH		= ${MAKETOP}bos/usr/bin/tar:${MAKETOP}bos/usr/ccs/lib/libIN

PROGRAMS	= tar
OFILES		= tar.o eprintf.o
MSGHDRS		= tar_msg.h
CATFILES	= tar.cat
HFILES		= ${MSGHDRS}

CFLAGS		=-UMSG -D_BLD -D_${TARGET_MACHINE}

ILIST		= tar
IDIR		= /usr/bin/

.include <${RULES_MK}>
