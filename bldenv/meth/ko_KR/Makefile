# @(#)82	1.3  src/bldenv/meth/ko_KR/Makefile, cfgnls, bos412, GOLDA411a 10/6/94 20:54:48
#
#   COMPONENT_NAME: CFGNLS
#
#   FUNCTIONS: Makefile for Korean Methods
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

VPATH			= ${MAKETOP}/bos/usr/lib/nls/loc/locale/ko_KR:${MAKETOP}/bos/kernel:${MAKETOP}/bos/usr/include:${MAKETOP}/bos/usr/ccs/lib/libc
HFILES			= local_includes
INCFLAGS		= -I./includes
PROGRAMS		= ko_KR_meth

CFLAGS			= -D_ILS_MACROS

OFILES		= \
	ko_KR_meth.o \
	__mblen_euckr.o \
	__mbstopcs_euckr.o \
	__mbstowcs_euckr.o \
	__mbtopc_euckr.o \
	__mbtowc_euckr.o \
	__pcstombs_euckr.o \
	__pctomb_euckr.o \
	__wcstombs_euckr.o \
	__wcswidth_euckr.o \
	__wctomb_euckr.o \
	__wcwidth_euckr.o

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


