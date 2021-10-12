# @(#)38        1.8  src/bos/usr/lib/methods/cfgsccd/Makefile, cfgmethods, bos41J, 9513A_all 3/24/95 15:48:37
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

PROGRAMS	= cfgsccd

OFILES		= cfgdevice.o cfgtools.o chkslot.o cfgsccd.o pvidtools.o \
		  scvpd.o sccheck.o rds_common.o rds_power_on_slot.o

INCFLAGS	= -I../common

CFLAGS		= -D_CFG_RDS

ILIST		= cfgsccd
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
