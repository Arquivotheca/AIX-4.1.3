# @(#)62	1.6  src/bos/diag/da/sol/Makefile, dasol, bos411, 9428A410j 2/3/94 14:56:00
#
#   COMPONENT_NAME: DASOL
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992,1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


VPATH		= ${MAKETOP}/bos/kernext/sol
PROGRAMS	= doptics

MSGHDRS		= optics_msg.h
CATFILES 	= optics.cat
HFILES		= ${MSGHDRS}

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/sol

LIBS		= -lasl -ldiag -lodm -lcfg -lc -lsoltu

ILIST		= ${PROGRAMS} optics.cat
IDIR		= /usr/lpp/diagnostics/da/
optics.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

OTHERS		= sol_errids.h

.include <${RULES_MK}>

sol_errids.h: devices.sys.slc.rte.err.S
		${CP} ${devices.sys.slc.rte.err.S:P} .
		${ERRTMPLTBLD} -d devices.sys.slc.rte.err.S -o sol_errids.h
		${RM} -f devices.sys.slc.rte.err.S
