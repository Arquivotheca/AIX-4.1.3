# @(#)33        1.1  src/bos/diag/da/d7135/Makefile, da7135, bos41J, 9510A_all 3/6/95 13:50:35
#
#   COMPONENT_NAME: DA7135
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993,1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= d7135

MSGHDRS		= d7135_msg.h
CATFILES 	= d7135.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc -lscsitu
SCRIPTS		= d7135.data

ILIST		= ${PROGRAMS} d7135.cat d7135.data
IDIR		= /usr/lpp/diagnostics/da/
d7135.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

d7135_OFILES	= d7135m.o d7135a.o d7135t.o

.include <${RULES_MK}>
