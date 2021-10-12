# @(#)32        1.6  src/bos/usr/lib/methods/cfghscsi/Makefile, cfgmethods, bos412, 9446B 10/28/94 10:24:23
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

VPATH		=../common

PROGRAMS	= cfghscsi

OFILES		= cfgdevice.o cfgtools.o chkslot.o cfghscsi.o cfghscsidc.o \
		  put_scsi_id.o location.o pvidtools.o sccheck.o  cfgvpd.o rds_common.o rds_power_devices.o

INCFLAGS	= -I../common

CFLAGS          = -D_CFG_RDS

ILIST		= cfghscsi
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
