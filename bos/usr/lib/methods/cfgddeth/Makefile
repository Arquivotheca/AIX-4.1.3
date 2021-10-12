# @(#)34        1.2  src/bos/usr/lib/methods/cfgddeth/Makefile, diagddient, bos411, 9428A410j 11/11/93 11:21:10
# COMPONENT_NAME: DIAGDDIENT
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
VPATH           := ${VPATH}:../cfgeth

PROGRAMS        = cfgddeth

OFILES          = cfgddcom.o cfgtools.o chkslot.o cfgddeth.o eth_vpd.o

INCFLAGS        = -I../common -I${MAKETOP}/bos/kernext

ILIST           = cfgddeth
IDIR		= /usr/lib/methods/

LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
