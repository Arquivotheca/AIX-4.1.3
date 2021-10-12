# @(#)69        1.1  src/bos/usr/bin/mh/mts/Makefile, cmdmh, bos411, 9428A410j 8/5/92 16:52:00
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

LIBRARIES	= libmts.a
MSGHDRS		= 
CATFILES	= 
HFILES		= ${MSGHDRS}

CFLAGS		= -D_BSD -DBSD42 -DMHE -DMHRC -DSENDMTS -DBERK

INCFLAGS	= -I../h -I../zotnet -I../config

OFILES		= hosts.o smail.o

.include <${RULES_MK}>
