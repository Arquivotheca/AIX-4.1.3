# @(#)70 1.4 src/bos/usr/lib/methods/chgtty/Makefile, cfgtty, bos41J, 9520A_all 4/27/95 14:04:00
#
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# LEVEL 1, 5 Years Bull Confidential Information
#

VPATH       = ../common:../cfgtty

PROGRAMS    = chgtty

OFILES      = chgtty.o cfgtools.o location.o chgttyutil.o \
              set_inittab.o mattrval.o utility.o

INCFLAGS    = -I../common \
	      -I${MAKETOP}/bos/kernext/tty/${TARGET_MACHINE}

ILIST       = chgtty
IDIR        = /usr/lib/methods/


LIBS        = -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
