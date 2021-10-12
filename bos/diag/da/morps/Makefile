# @(#)42	1.2  src/bos/diag/da/morps/Makefile, damorps, bos411, 9428A410j 12/5/93 20:46:49
#
#   COMPONENT_NAME: DAMORPS
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


PROGRAMS	= dmorps

MSGHDRS		= morps_msg.h
CATFILES 	= morps.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc

ILIST		= ${PROGRAMS} morps.cat
IDIR		= /usr/lpp/diagnostics/da/
morps.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

dmorps_OFILES	= dmorps.o

.include <${RULES_MK}>
