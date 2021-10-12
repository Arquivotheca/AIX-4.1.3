# @(#)47	1.1  src/bos/diag/da/ice/Makefile, daice, bos41J, 9513A_all 3/22/95 12:18:11
#
#   COMPONENT_NAME: DAICE
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
VPATH		= ${MAKETOP}/bos/diag/tu/ice:${MAKETOP}/bos/kernext/exp
PROGRAMS	= dice 

MSGHDRS		= dice_msg.h
CATFILES 	= dice.cat
HFILES		= ${MSGHDRS}

INCFLAGS	= -I${MAKETOP}/bos/usr/include \
		-I${MAKETOP}/bos/diag/tu/ice   \
	      -I${MAKETOP}/bos/kernext/disp/bbl/inc   #  the dir. for the bbl errid file

LIBS		= -lasl -ldiag -lodm -lcfg -lc -ltu_ice

ILIST		= ${PROGRAMS} dice.cat
IDIR		= /usr/lpp/diagnostics/da/
dice.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

dice_IMPORTS	= -bI:rcm.exp 

dice_OFILES	= dice.o 

.include <${RULES_MK}>
