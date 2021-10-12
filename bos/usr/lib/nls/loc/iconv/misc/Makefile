# @(#)24        1.6  src/bos/usr/lib/nls/loc/iconv/misc/Makefile, cmdiconv, bos411, 9428A410j 3/16/94 16:28:06
#
#   COMPONENT_NAME: CMDICONV
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1988, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

PROGRAMS=			\
	IBM-850_uucode		\
	uucode_IBM-850		\
	Universal_UCS_Conv

ILIST	= ${PROGRAMS}
IDIR	= /usr/lib/nls/loc/iconv/

CFLAGS	= -DAIXV3AR

LDFLAGS	= -einstantiate

LIBS	= -liconv

.include <${RULES_MK}>
