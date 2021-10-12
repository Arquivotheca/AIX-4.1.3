# @(#)59	1.1  src/bos/usr/lib/methods/cfgmouse_std/Makefile, inputdd, bos41J, 9509A_all 2/14/95 12:56:33
#
#   COMPONENT_NAME: INPUTDD
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		+= ../common:../cfgcommon

PROGRAMS	= cfgmouse_std

OFILES		= cfgmouse_std.o  cfgcommon.o  cfg_runtime_br_none.o \
		  cfg_dvdr.o  cfg_get_parent_dvdr.o  cfgtools.o 

INCFLAGS	= -I../common -I../cfgcommon -I../cfgktsm

ILIST		= cfgmouse_std
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
