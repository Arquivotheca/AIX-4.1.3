# @(#)38        1.6  src/bos/usr/bin/tset/Makefile, cmdtty, bos411, 9428A410j 2/21/94 08:41:25
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# LEVEL 1, 5 Years Bull Confidential Information
# 

PROGRAMS	= tset
MSGHDRS		= tset_msg.h
CATFILES	= tset.cat
HFILES		= ${MSGHDRS}

ILIST		= tset
IDIR		= /usr/bin/

CFLAGS		+= -D_NO_PROTO -UNONSTD_TYPES ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURSESLIBFLAG}

tset_LIBS	= -lcurses -lodm -lcfg

.include <${RULES_MK}>
