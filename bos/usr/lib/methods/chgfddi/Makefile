# @(#)40        1.1  src/bos/usr/lib/methods/chgfddi/Makefile, sysxfddi, bos411, 9428A410j 11/29/93 10:16:38
#
# COMPONENT_NAME: SYSXFDDI
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ../common

PROGRAMS        = chgfddi

OFILES          = chgdevice.o cfgtoolsx.o location.o chgfddi.o

INCFLAGS	= -I../common -I${MAKETOP}/bos/kernext

ILIST           = chgfddi
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
