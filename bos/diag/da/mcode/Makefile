# @(#)40	1.2  src/bos/diag/da/mcode/Makefile, damcode, bos411, 9428A410j 12/5/93 20:45:10
#
#   COMPONENT_NAME: DAMCODE
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



PROGRAMS	= umcode

MSGHDRS		= umcode_msg.h
CATFILES 	= umcode.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc

ILIST		= ${PROGRAMS} umcode.cat
IDIR		= /usr/lpp/diagnostics/bin/
umcode.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/


umcode_OFILES	= umcode.o

.include <${RULES_MK}>
