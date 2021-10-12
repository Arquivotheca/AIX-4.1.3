# @(#)20        1.3  src/bos/usr/lib/nls/loc/iconv/host/Makefile, cmdiconv, bos411, 9428A410j 3/6/93 14:14:10
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= IBM-930_IBM-932 IBM-932_IBM-930

OTHERS		= IBM-933_IBM-eucKR IBM-eucKR_IBM-933 \
		  IBM-937_IBM-938 IBM-938_IBM-937

ILIST		= ${PROGRAMS} ${OTHERS}
IDIR		= /usr/lib/nls/loc/iconv/

CFLAGS		= -DAIXV3AR

LDFLAGS		= -einstantiate

LIBS		= -liconv

.include <${RULES_MK}>

IBM-933_IBM-eucKR: IBM-930_IBM-932
	${CP} ${IBM-930_IBM-932:P} ${.TARGET}

IBM-eucKR_IBM-933: IBM-932_IBM-930
	${CP} ${IBM-932_IBM-930:P} ${.TARGET}

IBM-937_IBM-938: IBM-930_IBM-932
	${CP} ${IBM-930_IBM-932:P} ${.TARGET}

IBM-938_IBM-937: IBM-932_IBM-930
	${CP} ${IBM-932_IBM-930:P} ${.TARGET}

