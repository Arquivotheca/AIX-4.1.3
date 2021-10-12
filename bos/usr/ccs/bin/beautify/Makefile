# # @(#)47        1.2  src/bos/usr/ccs/bin/beautify/Makefile, cmdprog, bos411, 9428A410j 1/19/93 12:56:39
# COMPONENT_NAME: beautify
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

VPATH			=../structure

PROGRAMS		= beautify 

ILIST			= beautify 
EXPLIST			= b.h 
IDIR			= /usr/lib/struct/
EXPDIR			= /usr/include/

MSGHDRS			= struct_msg.h
CATFILES		= struct.cat

HFILES			= ${MSGHDRS} b.h
CFLAGS			= -DBSD_INCLUDES
YFLAGS			= -d
HFILES			= ${MSGHDRS} 

OFILES			= beauty.o tree.o lextab.o bdef.o 

LIBS			= -ll -lbsd


.include <${RULES_MK}>

tree.o lextab.o: beauty.h

