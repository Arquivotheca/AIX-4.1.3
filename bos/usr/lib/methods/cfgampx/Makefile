# @(#)49        1.1  src/bos/usr/lib/methods/cfgampx/Makefile, cfgmethods, bos411, 9428A410j 2/2/94 09:40:47
#
#   COMPONENT_NAME: cfgmethods
#
#   FUNCTIONS: Makefile
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

VPATH           = ../common

PROGRAMS        = cfgampx

OFILES          = cfgdevice.o cfgtools.o chkslot.o ddstools.o \
	          lducode.o ldutils.o cfgampx.o

INCFLAGS        = -I../common

ILIST           = cfgampx
IDIR            = /usr/lib/methods/


LIBS            = -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
