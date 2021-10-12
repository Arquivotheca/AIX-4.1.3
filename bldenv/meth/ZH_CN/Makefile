# @(#)71	1.1  src/bldenv/meth/zh_CN/Makefile, ils-zh_CN, bos41B, 9504A 12/16/94 16:53:06
#
#   COMPONENT_NAME: CFGNLS
#
#   FUNCTIONS: Makefile for Chinese Methods
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

VPATH			= ${MAKETOP}/bos/usr/lib/nls/loc/locale/zh_CN:${MAKETOP}/bos/kernel:${MAKETOP}/bos/usr/include
HFILES			= local_includes
INCFLAGS		= -I./includes
PROGRAMS		= zh_CN_meth

OFILES		= \
	zh_CN_meth.o \
	__mblen_euccn.o \
	__mbstopcs_euccn.o \
	__mbstowcs_euccn.o \
	__mbtopc_euccn.o \
	__mbtowc_euccn.o \
	__pcstombs_euccn.o \
	__pctomb_euccn.o \
	__wcstombs_euccn.o \
	__wcswidth_euccn.o \
	__wctomb_euccn.o \
	__wcwidth_euccn.o

STRIP_FLAG	=
LDFLAGS = -bnso -H8 -bh:5 -bI:/lib/syscalls.exp -estd_method_hdl

ILIST 		= ${PROGRAMS}
IDIR		= /usr/lib/nls/loc/methods/

.include <${RULES_MK}>

local_includes : stdlib.h sys/localedef.h sys/lc_core.h sys/lc_sys.h
	${TOUCH} ${.TARGET}
	${RM} -fr includes
	${MKDIR} -p includes/sys
	${CP} ${stdlib.h:P} includes
	${CP} ${sys/localedef.h:P} includes/sys
	${CP} ${sys/lc_core.h:P} includes/sys
	${CP} ${sys/lc_sys.h:P} includes/sys

