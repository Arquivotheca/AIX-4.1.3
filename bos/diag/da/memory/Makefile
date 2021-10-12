# @(#)41	1.3  src/bos/diag/da/memory/Makefile, damemory, bos411, 9428A410j 12/5/93 20:08:21
#
#   COMPONENT_NAME: DAMEMORY
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992, 1993.
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#



PROGRAMS	= dmemory

MSGHDRS		= memory_msg.h
CATFILES 	= memory.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lm -lc

ILIST		= ${PROGRAMS} memory.cat
IDIR		= /usr/lpp/diagnostics/da/
memory.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/


dmemory_OFILES	= dmemory.o

.include <${RULES_MK}>
