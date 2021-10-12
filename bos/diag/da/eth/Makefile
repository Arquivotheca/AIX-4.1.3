# @(#)45	1.6  src/bos/diag/da/eth/Makefile, daeth, bos411, 9428A410j 2/3/94 15:06:05
#   COMPONENT_NAME: DAETH
#
#   FUNCTIONS: None
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


VPATH		= ${MAKETOP}/bos/kernext/entdiag

PROGRAMS	= denet

MSGHDRS		= denet_msg.h
CATFILES 	= denet.cat
HFILES		= ${MSGHDRS}

LIBS		= -ldiag -lasl -lodm -lcfg -lc -lethtu

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/eth

ILIST		= ${PROGRAMS} denet.cat
IDIR		= /usr/lpp/diagnostics/da/
denet.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

OTHERS 		= ent_comio_errids.h 

.include <${RULES_MK}>

ent_comio_errids.h: devices.mca.8ef5.diag.err.S
		${CP} ${devices.mca.8ef5.diag.err.S:P} .
		${ERRTMPLTBLD} -d devices.mca.8ef5.diag.err.S -o ent_comio_errids.h
		${RM} -f devices.mca.8ef5.diag.err.S

