# @(#)41	1.5  src/bos/diag/da/cdrom/Makefile, dacdrom, bos411, 9435B411a 8/31/94 16:46:14
#
#   COMPONENT_NAME: DACDROM
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

VPATH		= ${MAKETOP}/bos/diag/util/uformat
PROGRAMS	= dcdrom

MSGHDRS		= dcdrom_msg.h
CATFILES 	= dcdrom.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc -lscsitu

ILIST		= ${PROGRAMS} dcdrom.cat
IDIR		= /usr/lpp/diagnostics/da/
dcdrom.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

dcdrom_OFILES	= disp_menu.o dcdrom.o disktools.o

.include <${RULES_MK}>
