# @(#)37        1.5  src/bos/usr/lib/methods/cfgsctape/Makefile, cfgmethods, bos412, 9446B 10/28/94 10:35:28
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# LEVEL 1,  5 Years Bull Confidential Information

VPATH		= ../common

PROGRAMS	= cfgsctape

OFILES		= cfgdevice.o cfgtools.o chkslot.o cfgsctape.o scvpd.o sccheck.o rds_common.o rds_power_on_slot.o

INCFLAGS	= -I../common

CFLAGS		= -D_CFG_RDS

ILIST		= cfgsctape
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
