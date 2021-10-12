# @(#)43	1.1  src/bos/usr/lib/nls/loc/locale/zh_CN/Makefile, ils-zh_CN, bos41B, 9504A 12/20/94 10:24:53
#
#   COMPONENT_NAME: cfgnls
#
#   FUNCTIONS:	none
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

VPATH 		= ${MAKETOP}bos/usr/lib/nls/charmap

CFLAGS		= -D_ILS_MACROS

LOCALE_SRC	= zh_CN.IBM-eucCN.src

LOCALES		= ${LOCALE_SRC:R}

OTHERS		= ${LOCALES}

SHARED_LIBRARIES	= zh_CN.o

SHARED_OFILES	= \
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

LIBS = libc.a
EXPORTS	= methods.exp

ILIST = zh_CN.m ${SHARED_LIBRARIES} ${LOCALE_SRC} ${LOCALES}
IDIR = /usr/lib/nls/loc/
zh_CN.m_IDIR	= /usr/lib/nls/loc/methods/
zh_CN.o_IDIR	= /usr/lib/nls/loc/methods/

.include <${RULES_MK}>

BASEFLAGS	=-v -c -C '${_CCFLAGS_} -F${ODE_TOOLS}/etc/xlC.cfg' \
		 -P ${ODE_TOOLS}/usr
LDBINDFLAGS	=${LIBDIRS} -bbinder:${ODE_TOOLS}/usr/lib/bind \
		 -bglink:${ODE_TOOLS}/usr/lib/glink.o

METH_PATH	=-blibpath:/usr/lib/nls/loc/methods:/usr/lib:/lib

TMP_METH	=${ODE_TOOLS}/usr/lib/nls/loc/methods/zh_CN_meth

LOCDEF_METH	=${BASEFLAGS} -L '${LDBINDFLAGS} ${METH_PATH}'

${LOCALES} : $${.TARGET:E} $${.TARGET:=.src} ${zh_CN.m:P} ${SHARED_LIBRARIES}
	${LOCDEF} -f ${IBM-eucCN:P} -i ${.ALLSRC:M*.src} -m ${zh_CN.m:P} \
	-M ${TMP_METH} ${LOCDEF_METH} ${.TARGET} || [ $$? -lt 2 ]
