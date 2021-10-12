# @(#)30        1.2  src/bos/usr/lib/methods/cfgfdd/Makefile, cfgmethods, bos411, 9428A410j 6/27/94 19:28:10
# COMPONENT_NAME:  cfgmethods
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		+= ${MAKETOP}/bos/usr/lib/methods/common
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/lib/methods/cfgcommon

PROGRAMS	= cfgfdd

OFILES		= cfgfdd.o  cfgcommon.o  cfg_runtime_br_none.o \
		  cfg_dvdr.o  cfg_get_parent_dvdr.o  cfgtools.o 

INCFLAGS	= -I../common \
		  -I../cfgcommon

ILIST		= cfgfdd
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
