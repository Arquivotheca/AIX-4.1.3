# @(#)34        1.1  src/bos/usr/lib/methods/cfgserdasdc/Makefile, cfgmethods, bos411, 9428A410j 8/5/92 16:59:51
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		=../common

PROGRAMS	= cfgserdasdc

OFILES		= cfgdevice.o cfgtools.o chkslot.o cfgserdasdc.o \
		  put_scsi_id.o location.o pvidtools.o serdasdtools.o \
		  dldserdasdc.o

INCFLAGS	= -I../common

ILIST		= cfgserdasdc
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
