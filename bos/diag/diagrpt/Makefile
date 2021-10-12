# @(#)25	1.1  src/bos/diag/diagrpt/Makefile, cmddiag, bos411, 9428A410j 12/10/92 09:47:56
#
#   COMPONENT_NAME: CMDDIAG
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


PROGRAMS	= diagrpt

MSGHDRS		= diagrpt_msg.h
HFILES		= ${MSGHDRS}
CATFILES 	= diagrpt.cat

LIBS		= -ldiag -lc

ILIST		= ${PROGRAMS} diagrpt.cat
IDIR		= /usr/lpp/diagnostics/bin/
diagrpt.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

diagrpt_OFILES	= diagrpt.o

.include <${RULES_MK}>
