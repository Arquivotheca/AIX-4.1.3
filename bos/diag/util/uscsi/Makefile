# @(#)21	1.4  src/bos/diag/util/uscsi/Makefile, dsauscsi, bos411, 9428A410j 12/5/93 20:41:30
#
#   COMPONENT_NAME: DSAUSCSI
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


PROGRAMS	= uscsi

MSGHDRS		= uscsi_msg.h
CATFILES 	= uscsi.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc

ILIST		= ${PROGRAMS} uscsi.cat
IDIR		= /usr/lpp/diagnostics/bin/
uscsi.cat_IDIR = /usr/lpp/diagnostics/catalog/default/
uscsi_OFILES	= uscsi.o

.include <${RULES_MK}>
