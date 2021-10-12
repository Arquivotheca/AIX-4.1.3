# @(#)47	1.6  src/bos/diag/da/ethsal/Makefile, daethsal, bos411, 9428A410j 2/3/94 14:12:36
#
#   COMPONENT_NAME: DAETHSAL
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

VPATH		= ${MAKETOP}/bos/kernext/ientdiag
PROGRAMS	= dsaleth

MSGHDRS		= dsaleth_msg.h
CATFILES 	= dsaleth.cat
HFILES		= ${MSGHDRS}

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/ethsal

LIBS		= -ldiag -lasl -lodm -lcfg -lc -lethsaltu

ILIST		= ${PROGRAMS} dsaleth.cat
IDIR		= /usr/lpp/diagnostics/da/
dsaleth.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

dsaleth_OFILES	= dsaleth.o

OTHERS 		= ient_comio_errids.h

.include <${RULES_MK}>

ient_comio_errids.h: devices.mca.8ef2.diag.com.err.S
		${CP} ${devices.mca.8ef2.diag.com.err.S:P} .
		${ERRTMPLTBLD} -d devices.mca.8ef2.diag.com.err.S -o ient_comio_errids.h
		${RM} -f devices.mca.8ef2.diag.com.err.S


