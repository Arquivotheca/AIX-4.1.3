# @(#)12	1.4  src/bos/diag/util/udtl/Makefile, dsaudtl, bos411, 9428A410j 12/5/93 20:11:40
#
#   COMPONENT_NAME: DSAUDTL
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


PROGRAMS	= udtl

MSGHDRS		= udtl_msg.h
CATFILES 	= udtl.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc

ILIST		= ${PROGRAMS} udtl.cat
IDIR		= /usr/lpp/diagnostics/bin/
udtl.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/
udtl_OFILES	= udtl.o

.include <${RULES_MK}>
