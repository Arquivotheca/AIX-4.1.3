# @(#)97        1.2  src/bos/usr/lbin/fddi_dd/Makefile, sysxfddi, bos411, 9428A410j 3/31/94 15:37:54
#
# COMPONENT_NAME: sysxfddi
#
# FUNCTIONS: Makefile for Device Driver Dump Formatter
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994, 
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
PROGRAMS        = fddi_dd

VPATH		= ${MAKETOP}/bos/kernext/fddi

OTHERS		= fddi_errids.h

OFILES		= main.o fddi_dmp.o

INCFLAGS	= -I${MAKETOP}/bos/kernext/fddi

ILIST           = fddi_dd

IDIR		= /usr/lib/ras/dmprtns/

fddi_errids.h: devices.mca.8ef4.rte.err.S
		${CP} ${devices.mca.8ef4.rte.err.S:P} .
		${ERRTMPLTBLD} -d devices.mca.8ef4.rte.err.S -o fddi_errids.h
		${RM} -f devices.mca.8ef4.rte.err.S

.include <${RULES_MK}>

