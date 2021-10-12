# @(#)49        1.1  src/bos/diag/util/u7135/Makefile, dsau7135, bos41J, 9510A_all 3/6/95 14:20:21
#
#   COMPONENT_NAME: DSAU7135
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

PROGRAMS	= u7135

MSGHDRS		= u7135_msg.h
CATFILES 	= u7135.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc -lscsitu
SCRIPTS		= u7135.data

ILIST		= ${PROGRAMS} u7135.cat u7135.data
IDIR		= /usr/lpp/diagnostics/bin/
u7135.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

u7135_OFILES	= u7135m.o u7135a.o u7135t.o

.include <${RULES_MK}>
