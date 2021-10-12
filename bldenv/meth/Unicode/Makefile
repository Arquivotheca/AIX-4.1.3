# @(#)94	1.3  src/bldenv/meth/Unicode/Makefile, cfgnls, bos412, GOLDA411a 10/6/94 21:02:28
#
#   COMPONENT_NAME: CFGNLS
#
#   FUNCTIONS: Makefile for Unicode Methods
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

VPATH			= ${MAKETOP}/bos/usr/lpp/Unicode/methods:${MAKETOP}/bos/kernel:${MAKETOP}/bos/usr/include
HFILES			= local_includes
INCFLAGS		= -I./includes
PROGRAMS		= Unicode_meth

CFLAGS			= -D_ILS_MACROS

OFILES		= \
	Unicode_meth.o \
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


STRIP_FLAG	=
LDFLAGS = -H8 -bh:5 -estd_method_hdl

ILIST 		= ${PROGRAMS}
IDIR		= /usr/lpp/Unicode/methods/

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

