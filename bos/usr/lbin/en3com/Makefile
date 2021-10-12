# @(#)76        1.1  src/bos/usr/lbin/en3com/Makefile, sysxent, bos411, 9428A410j 3/18/94 14:27:45
#
# COMPONENT_NAME: sysxent - High Performance Ethernet Device Driver
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
PROGRAMS        = en3com 

en3com_OFILES	= main.o en3com_dmp.o 

INCFLAGS	= -I${MAKETOP}/bos/kernext/ent

ILIST           = en3com 

en3com_IDIR	= /usr/lib/ras/dmprtns/

.include <${RULES_MK}>
