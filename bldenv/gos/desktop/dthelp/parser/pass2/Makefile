# @(#)44	1.1  src/bldenv/gos/desktop/dthelp/parser/pass2/Makefile, desktop, bos412, GOLDA411a 4/14/94 17:36:44
#
#   COMPONENT_NAME: DESKTOP
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

HTAG 		= ${MAKETOP}/gos/desktop/cde1/dthelp/parser

VPATH 		= ${HTAG}/pass2

PROGRAMS 	= htag2

OFILES 		= htag2.o

CFLAGS 		= -DSYSV -DAIXV3 -D_ALL_SOURCE -D_AIX -D__AIX -D__aix -D_aix \
		  -Daix -DMULTIBYTE -DNLS16 -DMESSAGE_CAT -DAIX_ILS -DR5TREE \
		  -DVALUE_ADDED -qMAXMEM=50000

#INCFLAGS 	= -Iincludes -I${HTAG}/parser -I${HTAG}/util -I${HTAG}/helptag \
#		  -I${HTAG}/build -I../eltdef

#LIBS 		=  -L../util -lutil

ILIST 		= htag2
IDIR 		= /opt/dt/etc/

.include <${RULES_MK}>
