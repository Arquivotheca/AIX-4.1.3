# @(#)57	1.5  src/bos/diag/da/gio/Makefile, dagio, bos411, 9428A410j 1/7/94 10:47:48
#
#   COMPONENT_NAME: DAGIO
#
#   FUNCTIONS: none
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


ILIST		= dgioa dgioa.cat
IDIR		= /usr/lpp/diagnostics/da/
dgioa.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

OTHERS =
OTHERS += dgioa
dgioa: dgioa.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS += dgioa.cat
dgioa.cat: dgioa.cat.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
