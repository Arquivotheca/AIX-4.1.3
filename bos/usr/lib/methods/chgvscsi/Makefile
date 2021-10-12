# @(#)12        1.1  src/bos/usr/lib/methods/chgvscsi/Makefile, cfgmethods, bos411, 9428A410j 9/27/93 11:43:20
#
#   COMPONENT_NAME: CFGMETHODS
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#CFLAGS		= -DCFGDEBUG

VPATH		= ../common

PROGRAMS	= chgvscsi

OFILES		= chgdevice.o cfgtools.o location.o chgvscsi.o put_scsi_id.o

INCFLAGS	= -I../common

ILIST		= chgvscsi
IDIR		= /usr/lib/methods/

LIBS		= -lodm -lcfg

.include <${RULES_MK}>
