# @(#)68	1.1  src/bos/diag/da/resid/Makefile, daresid, bos41J, 9520A_all 5/15/95 17:24:58
#
#   COMPONENT_NAME: DARESID
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#



PROGRAMS	= dresid

MSGHDRS		= dresid_msg.h
CATFILES 	= dresid.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lm -lc

ILIST		= ${PROGRAMS} dresid.cat
IDIR		= /usr/lpp/diagnostics/da/
dresid.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/


dresid_OFILES	= dresid.o

.include <${RULES_MK}>
