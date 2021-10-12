# @(#)18        1.1  src/bos/usr/lib/methods/definet/Makefile, cfgmethods, bos411, 9428A410j 8/5/92 16:57:40
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= definet
MSGHDRS		= definet_msg.h
CATFILES	= definet.cat
HFILES		= ${MSGHDRS}

ILIST		= definet
IDIR		= /usr/lib/methods/

INCFLAGS	= -I../common

LIBS		= -lcfgtcp -lodm -lsrc -lcfg -lbsd

.include <${RULES_MK}>
