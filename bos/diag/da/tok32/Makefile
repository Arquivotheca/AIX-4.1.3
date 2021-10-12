# @(#)21	1.3  src/bos/diag/da/tok32/Makefile, dtok32, bos411, 9437C411a 9/15/94 17:20:49
#
#   COMPONENT_NAME: DTOK32
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


VPATH		= ${MAKETOP}/bos/kernext/exp
PROGRAMS	= dtok32

MSGHDRS		= dtok32_msg.h
CATFILES 	= dtok32.cat
HFILES		= ${MSGHDRS}

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/mps

LIBS		= -lasl -ldiag -lodm -lcfg -lc -lmpstu

ILIST		= ${PROGRAMS} dtok32.cat
IDIR		= /usr/lpp/diagnostics/da/
dtok32.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

dtok32_OFILES		= dtok32.o

dtok32_IMPORTS	= -bI:diagex.exp -bI:dtok32_kext.exp

.include <${RULES_MK}>

