# @(#)59	1.6  src/bos/diag/da/lpfk/Makefile, dalpfk, bos411, 9428A410j 1/7/94 10:47:17
#
#   COMPONENT_NAME: DALPFK
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1989,1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


VPATH		= ${MAKETOP}/bos/kernext/exp
PROGRAMS	= dlpfk

MSGHDRS		= dlpfk_msg.h
CATFILES 	= dlpfk.cat
HFILES		= ${MSGHDRS}

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/gio
LIBS		= -lasl -ldiag -lodm -lcfg -lc -lgiotu

ILIST		= ${PROGRAMS} dlpfk.cat
IDIR		= /usr/lpp/diagnostics/da/
dlpfk.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

dlpfk_IMPORTS	= -bI:diagex.exp

.include <${RULES_MK}>


