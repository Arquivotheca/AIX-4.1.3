# @(#)61        1.1  src/bos/usr/ccs/bin/size/Makefile, cmdaout, bos411, 9428A410j 8/5/92 16:28:54
# COMPONENT_NAME: CMDAOUT (size command)
#
# FUNCTIONS: none
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1991
# All Rights Reserved
#
# US Goversizeent Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS		= size
MSGHDRS                 = size_msg.h
CATFILES                = size.cat

ILIST			= size
IDIR			= /usr/ccs/bin/

CFLAGS			= -DAIXV3AR -DUNIX
HFILES			= ${MSGHDRS} 

OFILES			= fcns.o  process.o  size.o 

LIBS			= -lld

.include <${RULES_MK}>
