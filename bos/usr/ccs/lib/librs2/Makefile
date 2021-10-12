# @(#)21	1.1  src/bos/usr/ccs/lib/librs2/Makefile, librs2, bos411, 9428A410j 8/5/93 11:03:46
#
#   COMPONENT_NAME: LIBRS2
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		=${TARGET_MACHINE}

LIBRARIES	=librs2.a
MSGHDRS		=
CATFILES	=
HFILES		= ${MSGHDRS}

EXPLIB_TARGETS	= export_librs2.a

ILIST		=librs2.a
IDIR		= /usr/lib/

INCFLAGS= -I${TARGET_MACHINE}

OFILES=	 ${${TARGET_MACHINE}_OFILES}

.include <${TARGET_MACHINE}/machdep.mk>

.include <${RULES_MK}>
