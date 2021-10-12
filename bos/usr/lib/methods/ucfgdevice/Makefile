# @(#)91        1.4  src/bos/usr/lib/methods/ucfgdevice/Makefile, cfgmethods, bos411, 9431A411a 7/29/94 08:59:51
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27, 83
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
# LEVEL 1,  5 Years Bull Confidential Information

#CFLAGS		= -D_CFGDEBUG
PROGRAMS	= ucfgdevice

INCFLAGS	= -I../common

CFLAGS          = -D_CFG_RDS

OFILES		= ucfgdevice.o ucfg_dvdr.o ucfg_gen.o rds_power_off_slot.o

ILIST		= ucfgdevice
IDIR		= /usr/lib/methods/

LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
