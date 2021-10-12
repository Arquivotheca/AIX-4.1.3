# @(#)36        1.1  src/bos/usr/lbin/kent_dd/Makefile, pcient, bos41J, 9511A_all 3/3/95 15:19:08
#
# COMPONENT_NAME: pcient
#
# FUNCTIONS: Makefile for Device Driver Dump Formatter
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
PROGRAMS        = kent_dd

VPATH		= ${MAKETOP}/rspc/kernext/pci/kent

OTHERS		= kent_errids.h

OFILES		= main.o kent_dmp.o

INCFLAGS	= -I${MAKETOP}/rspc/kernext/pci/kent

ILIST           = kent_dd

IDIR		= /usr/lib/ras/dmprtns/

kent_errids.h: devices.pci.22100020.rte.err.S
		${CP} ${devices.pci.22100020.rte.err.S:P} .
		${ERRTMPLTBLD} -d devices.pci.22100020.rte.err.S -o kent_errids.h
		${RM} -f devices.pci.22100020.rte.err.S

.include <${RULES_MK}>

