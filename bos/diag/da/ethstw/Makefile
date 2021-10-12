# @(#)49	1.8  src/bos/diag/da/ethstw/Makefile, daethstw, bos411, 9428A410j 2/3/94 14:17:32
#
#   COMPONENT_NAME: DAETHSTW
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1990,1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/kernext/ientdiag
PROGRAMS	= dstileth

MSGHDRS		= dstileth_msg.h
CATFILES 	= dstileth.cat
HFILES		= ${MSGHDRS}

LIBS		= -ldiag -lasl -lodm -lcfg -lc -lethstwtu

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/ethstw

ILIST		= ${PROGRAMS} dstileth.cat
IDIR		= /usr/lpp/diagnostics/da/
dstileth.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

dstileth_OFILES	= dstileth.o

OTHERS		= ient_comio_errids.h

.include <${RULES_MK}>

ient_comio_errids.h: devices.mca.8ef2.diag.com.err.S
		${CP} ${devices.mca.8ef2.diag.com.err.S:P} .
		${ERRTMPLTBLD} -d devices.mca.8ef2.diag.com.err.S -o ient_comio_errids.h
		${RM} -f devices.mca.8ef2.diag.com.err.S


