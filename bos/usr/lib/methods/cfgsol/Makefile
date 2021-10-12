# @(#)81	1.2  src/bos/usr/lib/methods/cfgsol/Makefile, cfgmethods, bos411, 9428A410j 2/2/94 19:59:31
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential	Restricted when
# combined with	the aggregated modules for this	product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT	International Business Machines	Corp. 1988, 1989
# All Rights Reserved
#
# US Government	Users Restricted Rights	- Use, duplication or
# disclosure restricted	by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ../common:${MAKETOP}/bos/kernext/sol

PROGRAMS	= cfgsol
HFILES		= sol_errids.h

OFILES		= cfgdevice.o cfgtools.o chkslot.o cfgsol.o

INCFLAGS	= -I../common -I${MAKETOP}/bos/kernext/sol

ILIST		= cfgsol
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>

sol_errids.h: devices.sys.slc.rte.err.S
		${CP} ${devices.sys.slc.rte.err.S:P} .
		${ERRTMPLTBLD} -d devices.sys.slc.rte.err.S -o sol_errids.h
		${RM} -f devices.sys.slc.rte.err.S

