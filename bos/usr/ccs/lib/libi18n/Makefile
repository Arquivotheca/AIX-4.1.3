# @(#)13	1.1  src/bos/usr/ccs/lib/libi18n/Makefile, libi18n, bos411, 9428A410j 8/25/93 11:12:15
#
#   COMPONENT_NAME: LIBI18N
#
#   FUNCTIONS: None
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


SHARED_LIBRARIES	= libi18n.a
CFLAGS			= -DAIX321
HFILES			= ${MSGHDRS}

EXPLIB_TARGETS		= export_libi18n.a

ILIST			= ${SHARED_LIBRARIES}
IDIR			= /usr/ccs/lib/

LIBS			= libc.a libiconv.a
EXPORTS			= -bE:libi18n.exp

SHARED_OFILES		= i18n.o defaults.o

.include <${RULES_MK}>

