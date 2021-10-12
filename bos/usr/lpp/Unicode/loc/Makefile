# @(#)60      1.5  src/bos/usr/lpp/Unicode/loc/Makefile, cfgnls, bos412, 9443ct.makefile 10/27/94 23:31:14
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
UTF_LOCALES = \
	UNIVERSAL.src

OTHERS = ${UTF_LOCALES:.src=}

IDIR = /usr/lpp/Unicode/loc/
ILIST =	${OTHERS} ${UTF_LOCALES} UNIVERSAL.m

.include <${RULES_MK}>

#
# Build	rules for locales.
#

BASEFLAGS   = -v -c -C '${_CCFLAGS_} -F${ODE_TOOLS}/etc/xlC.cfg' \
             -P ${ODE_TOOLS}/usr/
LDBINDFLAGS = ${LIBDIRS} -bbinder:${ODE_TOOLS}/usr/lib/bind \
                -bglink:${ODE_TOOLS}/usr/lib/glink.o

METH_PATH   = -blibpath:/usr/lpp/Unicode/methods:/usr/lib:/lib

TMP_METH    = ${ODE_TOOLS}/usr/lpp/Unicode/methods/Unicode_meth

LOCDEF_METH = ${BASEFLAGS} -L '${LDBINDFLAGS} ${METH_PATH} -lc'

# Location of .cm files	and methods
VPATH =	${MAKETOP}/bos/usr/lpp/Unicode/charmap:${MAKETOP}/bos/usr/lpp/Unicode/methods:${MAKETOP}../export/${target_machine}/usr/lib/nls/loc/methods

UNIVERSAL: UNIVERSAL.src UNIVERSAL.m UNIVERSAL.cm uni.o
	${CP} ${uni.o:P} .
	${LOCDEF} -f ${UNIVERSAL.cm:P} -i ${UNIVERSAL.src:P} \
        -m ${UNIVERSAL.m:P} -M ${TMP_METH} ${LOCDEF_METH} \
	 ${.TARGET} || [ $$? -lt 2 ]
	${RM} ${_RMFLAGS_} uni.o
