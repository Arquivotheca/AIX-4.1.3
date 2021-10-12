# @(#)14        1.4  src/bos/usr/ccs/lib/libqb/Makefile, libqb, bos411, 9428A410j 4/25/94 16:50:57
#
# COMPONENT_NAME: LIBQB
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

SHARED_LIBRARIES	= libqb.a
MSGHDRS         = libqb_msg.h
CATFILES        = libqb.cat
HFILES          = ${MSGHDRS}

EXPLIB_TARGETS  = export_libqb.a

ILIST           = libqb.a
IDIR            = /usr/lib/

LIBS		= libc.a
EXPORTS		= -bE:libqb.exp

SHARED_OFILES		= \
	notify.o burstaux.o log.o logaux.o piogetstatus.o pioputstatus.o

.include <${RULES_MK}>
