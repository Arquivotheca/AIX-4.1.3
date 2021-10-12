# @(#)60	1.3  src/bos/diag/da/niopp/Makefile, daniopp, bos411, 9428A410j 12/5/93 20:16:22
#
#   COMPONENT_NAME: DANIOPP
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


VPATH		= ${MAKETOP}/bos/diag/tu/niopp
PROGRAMS	= dpp

MSGHDRS		= dpp_msg.h
CATFILES 	= dpp.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc

ILIST		= ${PROGRAMS} dpp.cat
IDIR		= /usr/lpp/diagnostics/da/
dpp.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

dpp_OFILES	= dpp.o niopptu.o

.include <${RULES_MK}>
