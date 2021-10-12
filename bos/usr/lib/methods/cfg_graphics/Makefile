# @(#)44        1.4  src/bos/usr/lib/methods/cfg_graphics/Makefile, dispcfg, bos411, 9428A410j 3/1/94 16:18:05
#
#   COMPONENT_NAME: dispcfg
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#

VPATH		= ../common:../graphics

PROGRAMS	= cfg_graphics 

OFILES		= cfg_graphics.o cfg_graphics_tools.o           \
                  cfg_graphics_frs_tools.o                      \
                  cfg_ccmdd.o cfgtools.o ddstools.o chkslot.o   \
                  cfg_graphics_60x_frs_tools.o iplcb_tools.o

# CFLAGS = -g -DDEBUG -DDEBUG_BUS -DCFGDEBUG
# OPT_LEVEL =

ILIST		= cfg_graphics 

IDIR		= /usr/lib/methods/

LIBS		= -ls -lodm -lcfg -lrts

INCFLAGS =      -I../common                                     \
                -I../graphics

#                -I${MAKETOP}/bos/kernext/disp/inc               \

.include <${RULES_MK}>
