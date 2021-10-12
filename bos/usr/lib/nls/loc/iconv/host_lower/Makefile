# @(#)23        1.6  src/bos/usr/lib/nls/loc/iconv/host_lower/Makefile, cmdiconv, bos41J, 9509A_all 2/20/95 09:29:05
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

PROGRAMS	= IBM-932_IBM-300    IBM-300_IBM-932	\
		  IBM-eucJP_IBM-300  IBM-300_IBM-eucJP	\
		  IBM-eucJP_IBM-1027 IBM-1027_IBM-eucJP	\
		  IBM-eucJP_IBM-290  IBM-290_IBM-eucJP	\
		  IBM-eucKR_IBM-833  IBM-833_IBM-eucKR	\
		  IBM-eucKR_IBM-834  IBM-834_IBM-eucKR  \
 		  IBM-eucTW_IBM-037  IBM-037_IBM-eucTW	\
 		  IBM-eucTW_IBM-835  IBM-835_IBM-eucTW	\
 		  IBM-938_IBM-037    IBM-037_IBM-938	\
 		  IBM-938_IBM-835    IBM-835_IBM-938	\
 		  big5_IBM-037       IBM-037_big5	\
 		  big5_IBM-835       IBM-835_big5       \
		  IBM-PC_IBM-eucCN IBM-eucCN_IBM-PC \
		  IBM-eucCN_IBM-eucTW IBM-eucTW_IBM-eucCN \
		  IBM-eucCN_big5 big5_IBM-eucCN

ILIST		= ${PROGRAMS}
IDIR		= /usr/lib/nls/loc/iconv/

CFLAGS		= -DAIXV3AR

LDFLAGS		= -einstantiate

LIBS		= -liconv

.include <${RULES_MK}>

IBM-eucKR_IBM-833.o: ASCII_IBM-833.h
IBM-833_IBM-eucKR.o: IBM-833_ASCII.h
IBM-834_IBM-eucKR.o: IBM-834_KSC5601.h
IBM-eucKR_IBM-834.o: KSC5601_IBM-834.h

