# @(#)06	1.4  src/bos/diag/dctrl/Makefile, dctrl, bos411, 9433A411a 8/10/94 13:32:29
#
#   COMPONENT_NAME: DCTRL
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


PROGRAMS	= dctrl

MSGHDRS		= dctrl_msg.h
HFILES		= ${MSGHDRS}
CATFILES 	= dctrl.cat

LIBS		= -lasl -ldiag -lodm -lcfg -lc


ILIST		= ${PROGRAMS} dctrl.cat
IDIR		= /usr/lpp/diagnostics/bin/
dctrl.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

dctrl_OFILES	= dctrltest.o dctrldsp.o dctrlsup.o dctrlela.o \
			convert.o missingopt.o gen_rpt.o dctrl.o

.include <${RULES_MK}>
