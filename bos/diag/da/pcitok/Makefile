# @(#)95	1.1  src/bos/diag/da/tokpci/Makefile, datokpci, bos41J 3/21/95 16:44:00
#
#   COMPONENT_NAME: datokpci
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


VPATH		= ${MAKETOP}/bos/kernext/exp
PROGRAMS	= dtokpci

MSGHDRS		= dtokpci_msg.h
CATFILES 	= dtokpci.cat
HFILES		= ${MSGHDRS}

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/pcitok 

LIBS		= -lasl -ldiag -lodm -lcfg -lc -lskytu

ILIST		= ${PROGRAMS} dtokpci.cat
IDIR		= /usr/lpp/diagnostics/da/
dtokpci.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

dtokpci_OFILES		= dtokpci.o

dtokpci_IMPORTS	= -bI:diagex.exp 

.include <${RULES_MK}>

