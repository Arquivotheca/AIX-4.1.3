# @(#)83        1.1  src/bos/usr/lib/methods/cfgddtok/Makefile, cfgmethods, bos411, 9428A410j 10/26/93 16:40:58
# COMPONENT_NAME: DIAGDDTOK
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH           = ../common

PROGRAMS        = cfgddtok

OFILES          = cfgddcom.o cfgtoolsx.o chkslot.o cfgddtok.o comtok.o

INCFLAGS        = -I../common -I${MAKETOP}/bos/kernext

ILIST           = cfgddtok
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
