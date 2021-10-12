# @(#)57	1.4  src/bos/usr/ccs/lib/proflib/Makefile, libc, bos411, 9428A410j 2/22/94 06:24:36
#
#   COMPONENT_NAME: LIBC
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1988,1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


INCFLAGS	= -I${MAKETOP}/bos/usr/ccs/lib/libc -I${MAKETOP}/bos/usr/ccs/lib/libc/${TARGET_MACHINE}
VPATH		=${MAKETOP}/bos/usr/ccs/lib/libc:${MAKETOP}/bos/usr/ccs/lib/libc/${TARGET_MACHINE}

LIBRARIES	=libprof.a
MSGHDRS		=
CATFILES	=
HFILES		= ${MSGHDRS}

EXPLIB_TARGETS	= export_libprof.a

IDIR		= /usr/ccs/lib/

OFILES=	__mcount.o __mcount_construct.o _stk.o _getcall.o _getcall2.o

.include <${RULES_MK}>
