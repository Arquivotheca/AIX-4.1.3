# @(#)79	1.8  src/bos/diag/da/tok/Makefile, datok, bos411, 9428A410j 2/19/94 09:18:25
#
#   COMPONENT_NAME: DATOK
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

VPATH		= ${MAKETOP}/bos/kernext/tok
PROGRAMS	= dtoken

MSGHDRS		= dtoken_msg.h
CATFILES 	= dtoken.cat
HFILES		= ${MSGHDRS}
INCFLAGS	= -I${MAKETOP}/bos/diag/tu/tok

LIBS		= -lasl -ldiag -lodm -lcfg -lc -ltoktu

ILIST		= ${PROGRAMS} dtoken.cat
IDIR		= /usr/lpp/diagnostics/da/
dtoken.cat_IDIR = /usr/lpp/diagnostics/catalog/default/

dtoken_OFILES	= dtoken.o check_rc.o extra.o tokfrus.o trndisp_mn.o

OTHERS		= tr_mon_errids.h

.include <${RULES_MK}>

tr_mon_errids.h: devices.mca.8fc8.rte.err.S
		${CP} ${devices.mca.8fc8.rte.err.S:P} .
		${ERRTMPLTBLD} -d devices.mca.8fc8.rte.err.S -o tr_mon_errids.h
		${RM} -f devices.mca.8fc8.rte.err.S


