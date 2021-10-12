# @(#)40        1.3  src/bos/usr/lib/methods/cfgtmscsi/Makefile, cfgmethods, bos411, 9432A411a 8/5/94 15:36:32
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ../common

PROGRAMS	= cfgtmscsi

OFILES		= cfgdevice.o cfgtools.o chkslot.o cfgtmscsi.o scvpd.o sccheck.o

INCFLAGS	= -I../common

ILIST		= cfgtmscsi
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
