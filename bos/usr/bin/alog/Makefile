#  @(#)46        1.4  src/bos/usr/bin/alog/Makefile, cmdalog, bos411, 9428A410j 10/18/93 17:39:31 
#
#
#   COMPONENT_NAME: CMDALOG
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#

PROGRAMS        = alog
MSGHDRS         = alog_msg.h
CATFILES        = alog.cat
HFILES          = ${MSGHDRS}
alog_OFILES     = alog.o alog_db.o alog_util.o

ILIST           = alog
IDIR            = /usr/bin/

LIBS            = -lodm -lcfg -lras

.include <${RULES_MK}>
