# @(#)35        1.1  src/bos/usr/ccs/bin/as/Makefile, cmdas, bos411, 9428A410j 8/5/92 16:25:57
# COMPONENT_NAME:
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

VPATH			= ${TARGET_MACHINE}

PROGRAMS		= as
MSGHDRS                 = as_msg.h
CATFILES                = as.cat

ILIST			= as
IDIR			= /usr/ccs/bin/

INCFLAGS		= -I${TARGET_MACHINE}
CFLAGS			= 
HFILES			= ${MSGHDRS}  as0.h

OFILES			= as0.o    as1.o   as2.o    as3.o    asst.o \
			  aspso.o  asl.o   aslst.o  asomf.o  asmsgs.o

LIBS			= -lrts -lIN


.include <${RULES_MK}>
