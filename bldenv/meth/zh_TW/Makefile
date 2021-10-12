# @(#)83	1.3  src/bldenv/meth/zh_TW/Makefile, cfgnls, bos412, GOLDA411a 10/6/94 20:59:33
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

VPATH			= ${MAKETOP}/bos/usr/lib/nls/loc/locale/zh_TW:${MAKETOP}/bos/kernel:${MAKETOP}/bos/usr/include:${MAKETOP}/bos/usr/ccs/lib/libc
HFILES			= local_includes
INCFLAGS		= -I./includes
PROGRAMS		= zh_TW_meth

CFLAGS			= -D_ILS_MACROS

OFILES		= \
	zh_TW_meth.o \
	__mblen_euctw.o \
	__mbstopcs_euctw.o \
	__mbstowcs_euctw.o \
	__mbtopc_euctw.o \
	__mbtowc_euctw.o \
	__pcstombs_euctw.o \
	__pctomb_euctw.o \
	__wcstombs_euctw.o \
	__wcswidth_euctw.o \
	__wctomb_euctw.o \
	__wcwidth_euctw.o

STRIP_FLAG	=
LDFLAGS = -H8 -bh:5 -estd_method_hdl

ILIST 		= ${PROGRAMS}
IDIR		= /usr/lib/nls/loc/methods/

.include <${RULES_MK}>

local_includes : stdlib.h sys/localedef.h sys/lc_core.h sys/lc_sys.h ctype.h
	${TOUCH} ${.TARGET}
	${RM} -fr includes
	${MKDIR} -p includes/sys
	${CP} ${stdlib.h:P} includes
	${CP} ${sys/localedef.h:P} includes/sys
	${CP} ${sys/lc_core.h:P} includes/sys
	${CP} ${sys/lc_sys.h:P} includes/sys
	${CP} ${ctype.h:P} includes
