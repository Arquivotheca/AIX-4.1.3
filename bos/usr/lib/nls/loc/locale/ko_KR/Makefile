# @(#)55        1.8  src/bos/usr/lib/nls/loc/locale/ko_KR/Makefile, libmeth, bos411, 9439A411b 9/26/94 16:45:41
#
#   COMPONENT_NAME: CFGNLS
#
#   FUNCTIONS: Makefile for Korean Locale and Methods
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

VPATH 		=${MAKETOP}bos/usr/lib/nls/charmap

LOCALE_SRC	= \
	  ko_KR.IBM-eucKR.src

LOCALES		= ${LOCALE_SRC:R}

CFLAGS=-D_ILS_MACROS

OTHERS		= ${LOCALES}

SHARED_LIBRARIES	= ko_KR.o

SHARED_OFILES	= \
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

LIBS = libc.a
EXPORTS	= methods.exp

ILIST 		= ko_KR.m ${SHARED_LIBRARIES} ${LOCALE_SRC} ${LOCALES}
IDIR 		= /usr/lib/nls/loc/
ko_KR.m_IDIR	= /usr/lib/nls/loc/methods/
ko_KR.o_IDIR	= /usr/lib/nls/loc/methods/

.include <${RULES_MK}>

BASEFLAGS	=-v -c -C '${_CCFLAGS_} -F${ODE_TOOLS}/etc/xlC.cfg' \
		 -P ${ODE_TOOLS}/usr
LDBINDFLAGS	=${LIBDIRS} -bbinder:${ODE_TOOLS}/usr/lib/bind \
		 -bglink:${ODE_TOOLS}/usr/lib/glink.o

METH_PATH	=-blibpath:/usr/lib/nls/loc/methods:/usr/lib:/lib

TMP_METH	=${ODE_TOOLS}/usr/lib/nls/loc/methods/ko_KR_meth

LOCDEF_METH	=${BASEFLAGS} -L '${LDBINDFLAGS} ${METH_PATH}'

${LOCALES} : $${.TARGET:E} $${.TARGET:=.src} ${ko_KR.m:P} ${SHARED_LIBRARIES}
	${LOCDEF} -f ${IBM-eucKR:P} -i ${.ALLSRC:M*.src} -m ${ko_KR.m:P} \
	-M ${TMP_METH} ${LOCDEF_METH} ${.TARGET} || [ $$? -lt 2 ]
