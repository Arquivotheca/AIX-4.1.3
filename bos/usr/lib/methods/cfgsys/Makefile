# @(#)27        1.3  src/bos/usr/lib/methods/cfgsys/Makefile, cfgmethods, bos411, 9428A410j 11/22/93 14:08:16
#
# COMPONENT_NAME:
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

VPATH		=../common:../graphics

#CFLAGS		=-DCFGDEBUG			

PROGRAMS	= cfgsys

OFILES		= cfgsys.o cfgmem.o cfgtools.o setvar.o \
		cfg_graphics_60x_frs_tools.o cfg_graphics_frs_tools.o

INCFLAGS	= -I../common				\
		  -I../graphics

#		  -I${MAKETOP}/bos/kernext/disp/inc

ILIST		= cfgsys
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
