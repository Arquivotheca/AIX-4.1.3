# @(#)28        1.1  src/bos/usr/lib/methods/cfgddfddi/Makefile, diagddfddi, bos411, 9428A410j 11/8/93 10:00:16
# COMPONENT_NAME: DIAGDDFDDI
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

VPATH           = ../common

PROGRAMS        = cfgddfddi

OFILES          = cfgddcom.o cfgtools.o chkslot.o cfgddfddi.o

INCFLAGS        = -I../common -I${MAKETOP}/bos/kernext \
		  -I${MAKETOP}/bos/kernext

ILIST           = cfgddfddi
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
