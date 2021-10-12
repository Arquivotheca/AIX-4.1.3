# @(#)61        1.2  src/bldenv/cmdtext/makedev/Makefile, ade_build, bos412, GOLDA411a 3/9/93 16:58:25
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

VPATH		= ${MAKETOP}/cmdtext/usr/bin/makedev

MSGHDRS		= makedev_msg.h
HFILES		= ${MSGHDRS}

PROGRAMS	= local
OFILES		= makedev.o

CFLAGS		= -DLOCAL
INCFLAGS	= -I${MAKETOP}/cmdtext/usr/bin/roff/troff

ILIST		= local
IDIR		= /usr/bin/

.include <${RULES_MK}>
