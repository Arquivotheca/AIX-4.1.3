# @(#)73        1.1  src/bos/usr/bin/iostat/Makefile, cmdstat, bos411, 9428A410j 8/5/92 16:30:12
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

PROGRAMS	= iostat
MSGHDRS		= iostat_msg.h
CATFILES	= iostat.cat
HFILES		= ${MSGHDRS}

ILIST		= iostat
IDIR		= /usr/bin/

CFLAGS		= -D_BSD -DAIXV3AR

LIBS		= -ls -lld

.include <${RULES_MK}>
