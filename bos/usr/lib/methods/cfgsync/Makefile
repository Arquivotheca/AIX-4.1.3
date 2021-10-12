# @(#)30 1.2 src/bos/usr/lib/methods/cfgsync/Makefile, cfgtty, bos41J, 9512A_all 3/21/95 18:30:11
# COMPONENT_NAME:
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

VPATH		= ../common

PROGRAMS	= cfgsync

OFILES		= cfgtools.o cfgsync.o cfgdevsync.o

INCFLAGS	= -I../common\
              -I${MAKETOP}/bos/kernext/isa_async\
              -I${MAKETOP}/bos/kernext/isa_async/${TARGET_MACHINE}

ILIST		= cfgsync
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
