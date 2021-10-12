# @(#)09	1.2  src/bos/usr/ccs/lib/libdes/Makefile, libdes, bos411, 9428A410j 10/27/92 10:06:43
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential	Restricted when
# combined with	the aggregated modules for this	product)
# 
# (C) COPYRIGHT	International Business Machines	Corp. 1988, 1989
# All Rights Reserved
#
# US Government	Users Restricted Rights	- Use, duplication or
# disclosure restricted	by GSA ADP Schedule Contract with IBM Corp.
#

.if exists (${MAKETOP}/des/libdes/des_crypt.c)
VPATH		=${MAKETOP}/des/libdes
.else
VPATH		=${MAKETOP}/bos/usr/ccs/lib/libc
.endif

LIBRARIES	= libdes_nshr.a
MSGHDRS		= 
CATFILES	=
HFILES		= ${MSGHDRS}

EXPLIB_TARGETS	= export_libdes_nshr.a

IDIR		= /usr/ccs/lib/

.if exists (${MAKETOP}/des/libdes/des_crypt.c)
INCFLAGS	=-I${MAKETOP}/des/libdes
.else
INCFLAGS	=-I${MAKETOP}/bos/usr/ccs/lib/libc
.endif

INCLUDES	= des_crypt.h
des_crypt.h_IDIR = /usr/include/

.if exists (${MAKETOP}/des/libdes/des_crypt.c)
OFILES=	\
	des_crypt.o des_soft.o
.else
OFILES= \
	des_crnull.o des_sonull.o
.endif
.include <${RULES_MK}>
