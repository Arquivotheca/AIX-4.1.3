# @(#)60        1.1  src/bos/usr/lib/methods/cfgmps/Makefile, sysxmps, bos411, 9431A411a 7/15/94 15:58:15
# COMPONENT_NAME:
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

PROGRAMS        = cfgmps

OFILES          = cfgcommo.o cfgtoolsx.o chkslot.o cfgmps.o comtok.o

INCFLAGS        = -I../common -I${MAKETOP}/bos/kernext

ILIST           = cfgmps
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
