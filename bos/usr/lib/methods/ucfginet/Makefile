# @(#)22        1.1  src/bos/usr/lib/methods/ucfginet/Makefile, cfgmethods, bos411, 9428A410j 8/5/92 16:58:10
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

PROGRAMS	= ucfginet
MSGHDRS		= ucfginet_msg.h
CATFILES	= ucfginet.cat
HFILES		= ${MSGHDRS}

ILIST		= ucfginet
IDIR		= /usr/lib/methods/

INCFLAGS	= -I../common

LIBS		= -lcfgtcp -lodm -lsrc -lcfg -lbsd

.include <${RULES_MK}>
