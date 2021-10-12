# @(#)75	1.4  src/bos/diag/util/ucfgvpd/Makefile, dsauchgvpd, bos411, 9428A410j 12/5/93 20:13:47
#
#   COMPONENT_NAME: DSAUCHGVPD
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


PROGRAMS	= ucfgvpd

MSGHDRS		= ucfgvpd_msg.h
CATFILES 	= ucfgvpd.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc

ILIST		= ${PROGRAMS} ucfgvpd.cat
IDIR		= /usr/lpp/diagnostics/bin/
ucfgvpd.cat_IDIR = /usr/lpp/diagnostics/catalog/default/

ucfgvpd_OFILES  = ucfgvpd.o uchgvpd.o ucfgchg.o

.include <${RULES_MK}>
