# @(#)82	1.3  src/bos/usr/lib/methods/cfgops/Makefile, cfgmethods, bos411, 9428A410j 2/2/94 19:36:27
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

PROGRAMS	= cfgops
HFILES		= sol_errids.h

OFILES		= cfgtools.o cfgops.o

INCFLAGS	= -I../common -I${MAKETOP}/bos/kernext/sol

ILIST		= cfgops
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>

sol_errids.h: devices.sys.slc.rte.err.S
		${CP} ${devices.sys.slc.rte.err.S:P} .
		${ERRTMPLTBLD} -d devices.sys.slc.rte.err.S -o sol_errids.h
		${RM} -f devices.sys.slc.rte.err.S

