# @(#)12        1.1  src/bldenv/gos/desktop/pass1/build/Makefile, desktop, bos412, GOLDA411a 5/3/94 16:38:52
#
#   COMPONENT_NAME: ade_build
#
#   FUNCTIONS: 
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

VPATH 		= ${MAKETOP}gos/desktop/cde1/dthelp/parser/pass1/build:${MAKETOP}gos/desktop/cde1/dthelp/parser/pass1/helptag

PROGRAMS 	= build1
HFILES		= includes/context.h

OFILES 		= build.o buildutl.o eltree.o except.o fsa.o out.o param.o \
		  scan.o sref.o tree.o

CFLAGS 		= -DSYSV -DAIXV3 -D_ALL_SOURCE -D_AIX -D__AIX -D__aix -D_aix \
		  -Daix -DMULTIBYTE -DNLS16 -DMESSAGE_CAT -DAIX_ILS -DR5TREE \
		  -DVALUE_ADDED -qMAXMEM=50000

INCFLAGS	= -Iincludes -I${MAKETOP}/gos/desktop/cde1/dthelp/parser/pass1/util

LIBS		= -lutil1

ILIST		= ${PROGRAMS}
IDIR 		= /opt/dt/etc/

.include <${RULES_MK}>

includes/context.h: delim.bld context.dat
	${RM} -rf ${.TARGET:H}
	${MKDIR} ${.TARGET:H}
	${CP} ${delim.bld:P} ${.TARGET:H}/delim.dat
	${CP} ${context.dat:P} ${.TARGET:H}
	cd ${.TARGET:H}; ../../util/context1 sparse || ${DTCONTEXT1} sparse
