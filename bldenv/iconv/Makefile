# @(#)12        1.1  src/bldenv/iconv/Makefile, ade_build, bos412, GOLDA411a 2/17/93 08:10:34
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

VPATH		= ${MAKETOP}/bos/usr/bin/iconv

PROGRAMS	= iconv
MSGHDRS		= iconv_msg.h
CATFILES	= iconv.cat
HFILES		= ${MSGHDRS}

INCFLAGS	= -I${MAKETOP}/bos/usr/include -I${MAKETOP}/bos/kernel

ILIST		= iconv
IDIR		= /usr/bin/

CFLAGS		= -DAIXV3AR

LIBS		= -liconv

.include <${RULES_MK}>
