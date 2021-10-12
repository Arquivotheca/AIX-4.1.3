# @(#)45	1.1  src/bos/usr/lpp/Unicode/methods/Makefile, cfgnls, bos411, 9428A410j 1/21/94 10:14:04
#
#   COMPONENT_NAME: CFGNLS
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

SHARED_LIBRARIES	= uni.o

EXPLIB_TARGETS		= export_uni.o

SHARED_OFILES		= \
	__mblen_unistd.o \
	__mbstopcs_unistd.o \
	__mbstowcs_unistd.o \
	__mbtopc_unistd.o \
	__mbtowc_unistd.o \
	__pcstombs_unistd.o \
	__pctomb_unistd.o \
	__wcstombs_unistd.o \
	__wcswidth_unistd.o \
	__wctomb_unistd.o \
	__wcwidth_unistd.o

LIBS = libc.a
EXPORTS	= uni.exp

ILIST = uni.o
IDIR = /usr/lpp/Unicode/methods/

.include <${RULES_MK}>

