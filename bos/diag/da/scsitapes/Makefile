# @(#)94	1.4  src/bos/diag/da/scsitapes/Makefile, datape, bos411, 9428A410j 1/7/94 13:20:20
#
#   COMPONENT_NAME: DATAPE
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= dtape

MSGHDRS		= datape_msg.h
CATFILES 	= datape.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc -lscsitu

ILIST		= ${PROGRAMS} datape.cat
IDIR		= /usr/lpp/diagnostics/da/
datape.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

dtape_OFILES	= disp_mn.o  do_tests.o dtape.o

.include <${RULES_MK}>
