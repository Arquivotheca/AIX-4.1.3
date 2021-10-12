# @(#)05	1.1  src/bos/diag/util/udisacfg/Makefile, dsauisacfg, bos41J, 9512A_all 3/14/95 10:16:32
#
#   COMPONENT_NAME: DSAUISACFG 
#
#   FUNCTIONS: Makefile 
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


PROGRAMS	= udisacfg

MSGHDRS		= udisacfg_msg.h
CATFILES 	= udisacfg.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc

ILIST		= ${PROGRAMS} udisacfg.cat
IDIR		= /usr/lpp/diagnostics/bin/
udisacfg.cat_IDIR = /usr/lpp/diagnostics/catalog/default/

udisacfg_OFILES  = udisacfg.o

.include <${RULES_MK}>
