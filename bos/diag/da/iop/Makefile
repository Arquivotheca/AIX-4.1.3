# @(#)36	1.8  src/bos/diag/da/iop/Makefile, daiop, bos411, 9428A410j 4/26/94 15:06:10
#
#   COMPONENT_NAME: DAIOP
#
#   FUNCTIONS: 
#
#   ORIGINS: 27, 83
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
#   LEVEL 1, 5 Years Bull Confidential Information
#


VPATH		= ${MAKETOP}/bos/diag/tu/iop
PROGRAMS	= diop

MSGHDRS		= diop_msg.h
CATFILES 	= diop.cat
HFILES		= ${MSGHDRS}

INCFLAGS = -I${MAKETOP}/bos/diag/tu/iop

CFLAGS = -DNEW_TOD_CODE

LIBS		= -lasl -ldiag -lodm -lcfg -lc -lioptu

ILIST		= ${PROGRAMS} diop.cat
IDIR		= /usr/lpp/diagnostics/da/
diop.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

diop_OFILES	= diop_menus.o asl_rtn.o get_mode.o doioptest.o do_iop_tu.o \
			asniopfru.o iop_main.o 

.include <${RULES_MK}>
