# @(#)40	1.1  src/bos/usr/lib/methods/cfgsgio/Makefile, inputdd, bos41J, 9510A_all 2/28/95 14:49:06
#   COMPONENT_NAME: INPUTDD
#
#   FUNCTIONS:
#
#   ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994,1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/usr/lib/methods/common
VPATH      := ${VPATH}:${MAKETOP}/bos/kernext/inputdd/inc/sys

PROGRAMS	= cfgsgio

OFILES		= cfgsgio.o cfgtools.o ddstools.o

INCFLAGS	= -I${MAKETOP}/bos/usr/lib/methods/common
INCFLAGS   += -I${MAKETOP}/bos/kernext/inputdd/inc/sys

ILIST		= cfgsgio
IDIR		= /usr/lib/methods/

LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
