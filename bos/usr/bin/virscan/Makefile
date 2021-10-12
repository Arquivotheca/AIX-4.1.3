# @(#)06        1.2  src/bos/usr/bin/virscan/Makefile, cmdsvir, bos411, 9428A410j 9/2/92 19:53:55
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

PROGRAMS	= virscan
MSGHDRS		= virscan_msg.h
CATFILES	= virscan.cat
HFILES		= ${MSGHDRS} default.h

ILIST		= virscan 648clean.lst darclean.lst ghoclean.lst \
		  vacclean.lst virsig.lst xa1clean.lst
IDIR		= /usr/lib/security/scan/
virscan_IDIR	= /usr/bin/

OFILES		= virscan.o traverse.o vsutil.o

LIBS		= -ls

.include <${RULES_MK}>
