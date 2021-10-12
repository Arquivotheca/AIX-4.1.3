# @(#)22        1.8  src/bos/usr/ccs/lib/libiconv/Makefile, libiconv, bos411, 9428A410j 4/6/94 05:06:17
#
#   COMPONENT_NAME: LIBICONV
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1988, 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

VPATH		= ${TARGET_MACHINE}

SHARED_LIBRARIES= libiconv.a

MSGHDRS		=
CATFILES	=
HFILES		= ${MSGHDRS}

EXPLIB_TARGETS	= export_libiconv.a

ILIST		= libiconv.a
IDIR		= /usr/lib/

CFLAGS		= -DAIXV3AR -D_ILS_MACROS
LIBS		= libc.a
EXPORTS		= -bE:libiconv.exp

SHARED_OFILE_NAME = shr4.o
SHARED_OFILE_LIST = shr.o

SHARED_OFILES	= iconv.o ascii.o fcs.o hcs.o ccsid.o

.include <${RULES_MK}>

shr.o : shr.oB
	${RM} -f ${.TARGET}
	${CP} ${.ALLSRC} ${.TARGET}

