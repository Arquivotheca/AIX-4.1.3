# @(#)82  1.2  src/bos/usr/bin/mkslip/Makefile, rcs, bos411, 9436B411a 9/6/94 10:36:34
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

PROGRAMS	= mkslip

OFILES		=  mkslip.o slips.o

VPATH		= ${MAKETOP}/bos/objclass

MSGHDRS		= mkslip_msg.h

HFILES		= ${MSGHDRS} 

CATFILES	=  mkslip.cat

LIBS		= -lodm 

INCFLAGS	= 

ILIST		= ${PROGRAMS}

IDIR		= /usr/bin/

.include <${RULES_MK}>
