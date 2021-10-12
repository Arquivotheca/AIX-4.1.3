# @(#)11	1.1  src/bos/diag/util/ubackup/Makefile, dsaubackup, bos411, 9432B411a 8/9/94 13:21:22
#
#   COMPONENT_NAME: DSAUBACKUP
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


PROGRAMS	= ubackrest

MSGHDRS		= ubackrest_msg.h
CATFILES 	= ubackrest.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc

ILIST		= ${PROGRAMS} ubackrest.cat
IDIR		= /usr/lpp/diagnostics/bin/
ubackrest.cat_IDIR = /usr/lpp/diagnostics/catalog/default/
ubackrest_OFILES	= ubackrest.o

.include <${RULES_MK}>
