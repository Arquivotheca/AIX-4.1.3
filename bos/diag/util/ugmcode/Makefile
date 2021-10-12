# @(#)35	1.1  src/bos/diag/util/ugmcode/Makefile, dsaugmcode, bos411, 9432B411a 8/8/94 15:51:57
#
#   COMPONENT_NAME: DSAUGMCODE
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


PROGRAMS	= ugenucodesa

MSGHDRS		= ugenucodesa_msg.h
CATFILES 	= ugenucodesa.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc

ILIST		= ${PROGRAMS} ugenucodesa.cat
IDIR		= /usr/lpp/diagnostics/bin/
ugenucodesa.cat_IDIR = /usr/lpp/diagnostics/catalog/default/
ugenucodesa_OFILES	= ugenucodesa.o

.include <${RULES_MK}>
