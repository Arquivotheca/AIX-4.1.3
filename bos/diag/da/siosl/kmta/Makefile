# @(#)33	1.7  src/bos/diag/da/siosl/kmta/Makefile, dasiosl, bos411, 9428A410j 2/21/94 08:39:47
#
#   COMPONENT_NAME: DASIOSL
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


VPATH		= ${MAKETOP}/bos/diag/tu/siosl/kbda/slih:${MAKETOP}/bos/kernext/exp

PROGRAMS	= dkmta

CFLAGS          += -DDIAGNOSTICS ${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURLIBFLAG} ${_AIX3CURSESLIBFLAG}

MSGHDRS		= dkbd_a_msg.h mouse_a_msg.h tab_a_msg.h
CATFILES 	= dkbd_a.cat mouse_a.cat tab_a.cat
HFILES		= ${MSGHDRS}


INCFLAGS	= -I${MAKETOP}/bos/usr/include
LIBS		= -lasl -ldiag -lcur -lcurses -lodm -lcfg -lc -lmouseatu -ltabletatu -lkbdatu

ILIST		= ${PROGRAMS} dkbd_a.cat mouse_a.cat tab_a.cat
IDIR		= /usr/lpp/diagnostics/da/
dkbd_a.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/
mouse_a.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/
tab_a.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

dkmta_OFILES	= dkmta.o

dkmta_IMPORTS = -bI:diagex.exp -bI:kbd_slih.exp

.include <${RULES_MK}>
