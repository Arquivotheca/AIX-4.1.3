# @(#)20	1.3  src/bos/diag/util/umkdskt/Makefile, dsaumkdskt, bos411, 9428A410j 12/5/93 15:49:11
#
#   COMPONENT_NAME: DSAUMKDSKT
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


PROGRAMS	= umkdskt

MSGHDRS		= umkdskt_msg.h
CATFILES 	= umkdskt.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc

ILIST		= ${PROGRAMS} umkdskt.cat
IDIR		= /usr/lpp/diagnostics/bin/
umkdskt.cat_IDIR = /usr/lpp/diagnostics/catalog/default/
umkdskt_OFILES	= umkdskt.o

.include <${RULES_MK}>
