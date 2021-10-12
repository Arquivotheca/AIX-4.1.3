# @(#)03        1.1  src/bos/usr/lib/methods/cfgddent/Makefile, diagddent, bos411, 9428A410j 10/28/93 15:59:45
# COMPONENT_NAME: DIAGDDENT
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

PROGRAMS        = cfgddent

OFILES          = cfgddcom.o cfgtools.o chkslot.o cfgddent.o

INCFLAGS        = -I../common -I${MAKETOP}/bos/kernext

ILIST           = cfgddent
IDIR		= /usr/lib/methods/

LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
