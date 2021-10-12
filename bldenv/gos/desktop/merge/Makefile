# @(#)75        1.1  src/bldenv/gos/desktop/merge/Makefile, desktop, bos412, GOLDA411a 1/31/94 06:59:46
#
#   COMPONENT_NAME: ade_build
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH 		= ${MAKETOP}/gos/desktop/cde1/localized/util

PROGRAMS 	= merge

CFLAGS 		= -DSYSV -DAIXV3 -D_ALL_SOURCE -D_AIX -D__AIX -D__aix -D_aix \
		  -Daix -DMULTIBYTE -DNLS16 -DMESSAGE_CAT -DAIX_ILS -DR5TREE \
		  -DVALUE_ADDED -qMAXMEM=50000

ILIST 		= merge
IDIR  		= /opt/dt/etc/

.include <${RULES_MK}>
