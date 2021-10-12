# @(#)19	1.3  src/bos/diag/util/umblist/Makefile, dsaumblist, bos411, 9428A410j 12/5/93 14:36:16
#
#   COMPONENT_NAME: DSAUMBLIST
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


PROGRAMS	= umblist

MSGHDRS		= umblist_msg.h
CATFILES 	= umblist.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc

ILIST		= ${PROGRAMS} umblist.cat
IDIR		= /usr/lpp/diagnostics/bin/
umblist.cat_IDIR = /usr/lpp/diagnostics/catalog/default/
umblist_OFILES	= umblist.o

.include <${RULES_MK}>
