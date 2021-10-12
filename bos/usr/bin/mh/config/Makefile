# @(#)72	1.3  src/bos/usr/bin/mh/config/Makefile, cmdmh, bos411, 9428A410j 12/2/93 14:38:41
# COMPONENT_NAME: CMDMH
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

VPATH		= ../uip

LIBRARIES	= libconf.a libvers.a
MSGHDRS		= 
CATFILES	= 
HFILES		= ${MSGHDRS}

CFLAGS		= -D_BSD -DBSD42 -DMHE -DMHRC -DSENDMTS -DBERK

INCFLAGS	= -I../h

libconf.a_OFILES	= config.o
libvers.a_OFILES	= version.o

.include <${RULES_MK}>

version.c:	version.sh
	${CP} ${version.sh:P} .
	${CHMOD} +x version.sh
	./version.sh
	${RM} -f version.sh
