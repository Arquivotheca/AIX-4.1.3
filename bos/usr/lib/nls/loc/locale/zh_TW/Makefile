# @(#)56	1.7  src/bos/usr/lib/nls/loc/locale/zh_TW/Makefile, libmeth, bos411, 9439A411b 9/26/94 16:45:56
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
#   (C) COPYRIGHT International Business Machines Corp. 1993,1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH 		= ${MAKETOP}bos/usr/lib/nls/charmap

CFLAGS=-D_ILS_MACROS

LOCALE_SRC	= zh_TW.IBM-eucTW.src

LOCALES		= ${LOCALE_SRC:R}

OTHERS		= ${LOCALES}

SHARED_LIBRARIES	= zh_TW.o

SHARED_OFILES	= \
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

LIBS = libc.a
EXPORTS	= methods.exp

ILIST = zh_TW.m ${SHARED_LIBRARIES} ${LOCALE_SRC} ${LOCALES}
IDIR = /usr/lib/nls/loc/
zh_TW.m_IDIR	= /usr/lib/nls/loc/methods/
zh_TW.o_IDIR	= /usr/lib/nls/loc/methods/

.include <${RULES_MK}>

BASEFLAGS	=-v -c -C '${_CCFLAGS_} -F${ODE_TOOLS}/etc/xlC.cfg' \
		 -P ${ODE_TOOLS}/usr
LDBINDFLAGS	=${LIBDIRS} -bbinder:${ODE_TOOLS}/usr/lib/bind \
		 -bglink:${ODE_TOOLS}/usr/lib/glink.o

METH_PATH	=-blibpath:/usr/lib/nls/loc/methods:/usr/lib:/lib

TMP_METH	=${ODE_TOOLS}/usr/lib/nls/loc/methods/zh_TW_meth

LOCDEF_METH	=${BASEFLAGS} -L '${LDBINDFLAGS} ${METH_PATH}'

${LOCALES} : $${.TARGET:E} $${.TARGET:=.src} ${zh_TW.m:P} ${SHARED_LIBRARIES}
	${LOCDEF} -f ${IBM-eucTW:P} -i ${.ALLSRC:M*.src} -m ${zh_TW.m:P} \
	-M ${TMP_METH} ${LOCDEF_METH} ${.TARGET} || [ $$? -lt 2 ]
