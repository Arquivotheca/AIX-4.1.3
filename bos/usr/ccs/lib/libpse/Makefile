# @(#)41	1.3  src/bos/usr/ccs/lib/libpse/Makefile, cmdpse, bos411, 9436C411a 9/7/94 17:46:19
# COMPONENT_NAME:
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

VPATH		= ${MAKETOP}bos/usr/sbin/execerror
VPATH		:= ${VPATH}:${MAKETOP}bos/usr/sbin/strload

MSGHDRS		= execerr_msg.h strload_msg.h
HFILES		= ${MSGHDRS}

CFLAGS		+= -DAIX41

OFILES		= strconfig.o loaderr.o

LIBRARIES	= libpse.a
EXPLIB_TARGETS	= export_libpse.a
EXPDIR		= /usr/lib/

.include <${RULES_MK}>
