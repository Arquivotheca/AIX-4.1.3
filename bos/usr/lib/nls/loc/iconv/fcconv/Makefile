# @(#)18	1.9  src/bos/usr/lib/nls/loc/iconv/fcconv/Makefile, cmdiconv, bos41J, 9509A_all 2/19/95 22:46:16
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
#

PROGRAMS	= IBM-932_IBM-eucJP IBM-eucJP_IBM-932 \
		  IBM-934_IBM-eucKR IBM-eucKR_IBM-934 \
 		  IBM-938_IBM-eucTW IBM-eucTW_IBM-938 \
 		  big5_IBM-eucTW IBM-eucTW_big5 \
 		  IBM-938_big5      big5_IBM-938 \
		  IBM-eucCN_CNS11643.1986-1 \
		  IBM-eucCN_CNS11643.1986-2 \
		  IBM-eucCN_IBM-sbdCN \
		  IBM-eucCN_IBM-udcCN
		  

ILIST		= ${PROGRAMS}
IDIR		= /usr/lib/nls/loc/iconv/

CFLAGS		= -DAIXV3AR

LDFLAGS		= -einstantiate

.include <${RULES_MK}>
