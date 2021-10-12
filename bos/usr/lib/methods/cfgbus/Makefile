# @(#)03        1.5  src/bos/usr/lib/methods/cfgbus/Makefile, cfgmethods, bos411, 9428A410j 3/27/94 14:58:13
#
# COMPONENT_NAME: (CMDBUSCF)
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
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

VPATH		=../common:../graphics

PROGRAMS	= cfgbus

CFLAGS		= -DDEBUG_BUS

OFILES		= busquery.o sync.o bcm.o \
		  cfg_graphics_60x_frs_tools.o cfg_graphics_frs_tools.o

ILIST		= cfgbus

IDIR		= /usr/lib/methods/

INCFLAGS	= -I../common -I../graphics

LIBS		= -lodm -lcfg

.include <${RULES_MK}>


