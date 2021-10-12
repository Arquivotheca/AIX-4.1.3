# @(#)21        1.2  src/bos/usr/ccs/lib/libim/Makefile, libim, bos411, 9428A410j 2/2/94 14:27:25
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



SHARED_LIBRARIES	= libIM.a
MSGHDRS         =
CATFILES        =
HFILES          = ${MSGHDRS}

EXPLIB_TARGETS  = export_libIM.a

ILIST           = libIM.a
IDIR            = /usr/ccs/lib/

INCFLAGS	= -I${MAKETOP}/bos/usr/include

LIBS		= libc.a
EXPORTS		= -bE:libIM.exp

SHARED_OFILES	= \
	imcore.o aixmapping.o mapping.o

.include <${RULES_MK}>

mapping.o: deadtable.h
