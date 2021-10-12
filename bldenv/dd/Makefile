# @(#)96        1.3  src/bldenv/dd/Makefile, ade_build, bos412, GOLDA411a 1/26/94 08:32:52
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

VPATH		= ${MAKETOP}/bos/usr/bin/dd

PROGRAMS	= dd
MSGHDRS		= dd_msg.h
CATFILES	= dd.cat
HFILES		= ${MSGHDRS}

ILIST		= dd
IDIR		= /usr/bin/

.include <${RULES_MK}>

LLSEEK_EXIST	!=/usr/bin/nm /usr/lib/libc.a | ${GREP} llseek >/dev/null; \
		  [ $$? -eq 0 ] || ${ECHO} "-D_BLD"

CFLAGS		=${LLSEEK_EXIST}
