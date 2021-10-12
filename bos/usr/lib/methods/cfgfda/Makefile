# @(#)29        1.3  src/bos/usr/lib/methods/cfgfda/Makefile, cfgmethods, bos411, 9439A411c 9/27/94 15:02:36
# COMPONENT_NAME:  cfgmethods
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988,1994 
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

CFLAGS		+=-DBUS_TYPE
VPATH		=../common:../cfgcommon

PROGRAMS	= cfgfda

OFILES		= cfgcommon.o  cfg_runtime_br_rs6k.o  chkslot.o \
		  cfg_dvdr.o  cfg_get_dvdr.o  cfgtools.o  cfgfda.o

INCFLAGS	= -I../common \
		  -I../cfgcommon

ILIST		= cfgfda
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
