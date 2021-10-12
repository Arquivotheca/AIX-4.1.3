# @(#)79	1.2  src/bos/usr/lib/methods/cfgpscsi/Makefile, cfgmethods, bos411, 9428A410j 5/9/94 16:03:41
# COMPONENT_NAME: CFGMETHODS
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		=../common

PROGRAMS	= cfgpscsi

OFILES		= cfgdevice.o cfgtoolsx.o chkslot.o cfgpscsi.o cfghscsidc.o \
		  put_scsi_id.o location.o pvidtools.o sccheck.o cfgvpd.o

INCFLAGS	= -I../common

ILIST		= cfgpscsi
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
