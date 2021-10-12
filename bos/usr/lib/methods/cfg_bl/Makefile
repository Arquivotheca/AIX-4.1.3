# @(#)18        1.1 8/15/94 src/bos/usr/lib/methods/cfg_bl/Makefile, bluedd, bos411, 9433B411a 13:17:51
#
#   COMPONENT_NAME: (bluedd) BL Device Driver
#
#   FUNCTIONS: cfg_bl 
#
#   ORIGINS: 27
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ../common:../graphics

PROGRAMS	= cfg_bl

OFILES	 	= bl_cfg_load.o cfgbl.o ddstools.o iplcb_tools.o

ENTRYPOINT 	= dev_cfg_entry

#
# Uncomment to get debug information or run dbx
#
# CFLAGS = -g -DDEBUG -DDEBUG_BUS -DCFGDEBUG
# OPT_LEVEL =
#

INCFLAGS	= -I../graphics 				\
		  -I../common

ILIST		= cfg_bl
IDIR		= /usr/lib/methods/

LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
