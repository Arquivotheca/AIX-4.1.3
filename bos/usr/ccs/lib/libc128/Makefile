# @(#)81	1.4  src/bos/usr/ccs/lib/libc128/Makefile, libc128, bos411, 9428A410j 10/19/93 10:01:10
#   COMPONENT_NAME: libc128
#
#   FUNCTIONS:
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

SHARED_LIBRARIES = libc128.a

VPATH		= ${TARGET_MACHINE}

EXPLIB_TARGETS	= export_libc128.a 

shr_EXPORTS=libc128.exp

printf.o_INCFLAGS	= -I${MAKETOP}/bos/usr/ccs/lib/libc
scanf.o_INCFLAGS	= -I${MAKETOP}/bos/usr/ccs/lib/libc

ILIST		= libc128.a 
IDIR		= /usr/lib/

LIBS = libc.a

EXPORTS = -bE:libc128.exp

SHARED_OFILES	= 		\
	doprnt.o		\
	printf.o		\
	scanf.o

.include <${RULES_MK}>
