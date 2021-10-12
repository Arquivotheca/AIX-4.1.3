# @(#)21	1.3  src/bos/usr/lib/nls/loc/iconv/fold/Makefile, cmdiconv, bos411, 9428A410j 4/7/94 01:49:51
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

PROGRAMS	= ISO8859-1_ct ct_ISO8859-1 IBM-932_fold7 IBM-932_fold8
MSGHDRS		= 
CATFILES	= 
HFILES		= ${MSGHDRS}

DATAFILES	= ISO8859-1_fold7 ISO8859-1_fold8 fold7_ISO8859-1 fold8_ISO8859-1

ILIST		= ${PROGRAMS} ${DATAFILES}
IDIR		= /usr/lib/nls/loc/iconv/

CFLAGS		= -DAIXV3AR

LDFLAGS		= -einstantiate

LIBS		= -liconv

.include <${RULES_MK}>

ISO8859-1_fold7:	ISO8859-1_ct
	${CP} ${ISO8859-1_ct:P} ${.TARGET}

ISO8859-1_fold8:	ISO8859-1_ct
	${CP} ${ISO8859-1_ct:P} ${.TARGET}

fold7_ISO8859-1:	ct_ISO8859-1
	${CP} ${ct_ISO8859-1:P} ${.TARGET}

fold8_ISO8859-1:	ct_ISO8859-1
	${CP} ${ct_ISO8859-1:P} ${.TARGET}
