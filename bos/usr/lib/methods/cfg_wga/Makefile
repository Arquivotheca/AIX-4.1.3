# @(#)36        1.4  src/bos/usr/lib/methods/cfg_wga/Makefile, wgadd, bos411, 9428A410j 12/1/93 15:03:25
#
#   COMPONENT_NAME: (WGADD) Whiteoak Graphics Device Driver
#
#   FUNCTIONS: cfg_wga 
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

PROGRAMS	= cfg_wga

OFILES		= wga_cfg_load.o cfgwga.o ddstools.o

cfg_wga_ENTRYPOINT = dev_cfg_entry

#
# Uncomment to get debug information or run dbx
#
# CFLAGS = -g -DDEBUG -DDEBUG_BUS -DCFGDEBUG
# OPT_LEVEL =


ILIST		= cfg_wga

IDIR		= /usr/lib/methods/

LIBS		= -ls -lodm -lcfg -lrts

INCFLAGS	= -I../common -I../graphics

.include <${RULES_MK}>
