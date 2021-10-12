# @(#)99        1.4  src/bos/usr/lib/methods/cfgvscsi/Makefile, cfgmethods, bos412, 9446B 10/28/94 10:27:47
#
#   COMPONENT_NAME: CFGMETHODS
#
#   FUNCTIONS: none
#
#   ORIGINS: 27, 83
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# LEVEL 1,  5 Years Bull Confidential Information

#CFLAGS		= -DCFGDEBUG

VPATH		=../common

PROGRAMS	= cfgvscsi

OFILES		= cfgdevice.o cfgtoolsx.o chkslot.o cfgvscsi.o cfghscsidc.o \
		  rds_common.o rds_power_devices.o put_scsi_id.o location.o pvidtools.o sccheck.o

INCFLAGS	= "-I../common"

CFLAGS          = -D_CFG_RDS

ILIST		= cfgvscsi
IDIR		= /usr/lib/methods/

LIBS		= -lodm -lcfg

.include <${RULES_MK}>
