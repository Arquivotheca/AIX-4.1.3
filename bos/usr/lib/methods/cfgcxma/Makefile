# @(#)27 1.3 src/bos/usr/lib/methods/cfgcxma/Makefile, cfgtty, bos41J, 9512A_all 3/21/95 18:30:02
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
#PROGRAMS	= cfgcxma

VPATH		= ../common

PROGRAMS	= cfgcxma

LOCALCFLAGS =
OFILES		= cfgcxma.o cfgdevcxma.o cfgtools.o chkslot.o cxmadds.o

INCFLAGS	= -I../common\
              -I${MAKETOP}/bos/kernext/isa_async\
              -I${MAKETOP}/bos/kernext/isa_async/${TARGET_MACHINE}

ILIST		= cfgcxma
IDIR		= /usr/lib/methods/


LIBS		= -lodm -lcfg

.include <${RULES_MK}>
