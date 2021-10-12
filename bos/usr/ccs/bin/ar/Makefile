# # @(#)40        1.2  src/bos/usr/ccs/bin/ar/Makefile, cmdar, bos411, 9428A410j 8/18/93 10:59:08
# COMPONENT_NAME: ar
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1991
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS		= ar
MSGHDRS                 = ar_msg.h 
CATFILES                = ar.cat

ILIST			= ar
IDIR			= /usr/ccs/bin/

CFLAGS			= -DTOBEY_BUG -DLIBCIO_FIX -D_STD_ARG 
HFILES			= ${MSGHDRS} 

OFILES			= ar.o 

LIBS			= -lld


.include <${RULES_MK}>
