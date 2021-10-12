# @(#)13        1.3  src/bos/usr/ccs/lib/librts/Makefile, librts, bos411, 9428A410j 9/16/92 15:48:01
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

VPATH		= ${MAKETOP}/bos/usr/bin/errlg/msg

LIBRARIES	= librts.a
MSGHDRS         = cmderrlg_msg.h
CATFILES        =
HFILES          = ${MSGHDRS}

EXPLIB_TARGETS  = export_librts.a

ILIST           = librts.a
IDIR            = /usr/lib/

OFILES		= \
	errlog.o trcstart.o

.include <${RULES_MK}>
