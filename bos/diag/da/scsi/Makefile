# @(#)49	1.4  src/bos/diag/da/scsi/Makefile, dascsi, bos411, 9428A410j 12/5/93 20:46:14
#
#   COMPONENT_NAME: DASCSI
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992, 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


PROGRAMS	= dscsia

MSGHDRS		= dscsia_msg.h
CATFILES 	= dscsia.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc

ILIST		= ${PROGRAMS} dscsia.cat
IDIR		= /usr/lpp/diagnostics/da/
dscsia.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

dscsia_OFILES	= dscsia.o  dscsia_mn.o

.include <${RULES_MK}>
