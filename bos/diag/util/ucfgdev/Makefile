# @(#)09	1.3  src/bos/diag/util/ucfgdev/Makefile, dsaucfgdev, bos411, 9428A410j 4/5/93 09:47:25
#
#   COMPONENT_NAME: DSAUCFGDEV
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


PROGRAMS	= ucfgdev

MSGHDRS		= ucfgdev_msg.h
CATFILES 	= ucfgdev.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc

ILIST		= ${PROGRAMS} ucfgdev.cat
IDIR		= /usr/lpp/diagnostics/bin/
ucfgdev.cat_IDIR = /usr/lpp/diagnostics/catalog/default/
ucfgdev_OFILES	= ucfgdev.o

.include <${RULES_MK}>
