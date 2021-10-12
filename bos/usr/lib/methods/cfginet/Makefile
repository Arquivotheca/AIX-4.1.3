# @(#)14        1.1  src/bos/usr/lib/methods/cfginet/Makefile, cfgmethods, bos411, 9428A410j 8/5/92 16:57:09
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

PROGRAMS	= cfginet
MSGHDRS		= cfginet_msg.h
CATFILES	= cfginet.cat
HFILES		= ${MSGHDRS}

ILIST		= cfginet
IDIR		= /usr/lib/methods/

INCFLAGS	= -I../common

LIBS		= -lcfgtcp -lodm -lsrc -lcfg -lbsd

.include <${RULES_MK}>
