# @(#)48	1.2  src/bos/diag/da/mpa/Makefile, mpada, bos411, 9428A410j 2/21/94 08:39:32
#
#   COMPONENT_NAME: DAMPA
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


VPATH		= ${MAKETOP}/bos/diag/tu/mpa

PROGRAMS	= dampa

MSGHDRS		= dampa_msg.h
CATFILES 	= dampa.cat
HFILES		= ${MSGHDRS}
CFLAGS		+= -D DIAGS ${_AIX3CURFLAGS} ${_AIX3CURSEFLAGS}
LIBFLAGS	+= ${_AIX3CURLIBFLAG} ${_AIX3CURSESLIBFLAG}

LIBS		= -lasl -ldiag -lcur -lcurses -lodm -lcfg -lc

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/dampa

ILIST		= ${PROGRAMS} dampa.cat
IDIR		= /usr/lpp/diagnostics/da/
dampa.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

TUS =           exectu.o mpa_tus.o 

DABASE		= dampa.o dampa_tu.o dampa_asl.o

dampa_OFILES	= ${DABASE} ${TUS}

.include <${RULES_MK}>
