# @(#)65        1.4  src/bos/usr/lib/methods/cfgeth/Makefile, cfgmethods, bos411, 9428A410j 5/18/94 16:38:30
#
# COMPONENT_NAME: SYSXIENT
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH           = ../common

PROGRAMS        = cfgeth

OFILES          = cfgcommo.o cfgtoolsx.o chkslot.o cfgeth.o cfgvpd.o

INCFLAGS        = -I../common -I${MAKETOP}/bos/kernext -I../cfgcie

ILIST           = cfgeth
IDIR            = /usr/lib/methods/


LIBS            = -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
