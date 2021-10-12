# @(#)80  1.2  src/bos/usr/bin/chslip/Makefile, rcs, bos411, 9436B411a 9/6/94 10:36:29
# 
# COMPONENT_NAME: (RCS) Remote Customer Support and Services
#
# FUNCTIONS: none
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

PROGRAMS	= chslip

OFILES		= chslip.o slips.o

VPATH		= ${MAKETOP}/bos/objclass

MSGHDRS		= chslip_msg.h

HFILES		= ${MSGHDRS} 

CATFILES	= chslip.cat

LIBS		= -lodm 

ILIST		= ${PROGRAMS}

IDIR		= /usr/bin/

.include <${RULES_MK}>
