# @(#)44	1.7  src/bos/diag/da/dial/Makefile, dadial, bos411, 9428A410j 2/21/94 08:39:06
#
#   COMPONENT_NAME: DADIAL
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1992,1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


VPATH		= ${MAKETOP}/bos/kernext/exp
PROGRAMS	= ddials

MSGHDRS		= ddials_msg.h
CATFILES 	= ddials.cat
HFILES		= ${MSGHDRS}

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/gio
CFLAGS		+= ${_AIX3CURFLAGS}
LIBFLAGS 	+= ${_AIX3CURLIBFLAG}

LIBS		= -lasl -ldiag -lodm -lcfg -lc -lcur -lgiotu

ILIST		= ${PROGRAMS} ddials.cat
IDIR		= /usr/lpp/diagnostics/da/
ddials.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

ddials_IMPORTS	= -bI:diagex.exp


.include <${RULES_MK}>

