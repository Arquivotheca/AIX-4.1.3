# @(#)68        1.1  src/bldenv/gos/desktop/dthelp/build2/Makefile, desktop, bos412, GOLDA411a 1/31/94 06:56:46
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

VPATH 		= ${MAKETOP}/gos/desktop/cde1/dthelp/parser2/pass1/build:${MAKETOP}gos/desktop/cde1/dthelp/parser2/pass1/helptag

PROGRAMS 	= build2
HFILES		= context.h

OFILES 		= build.o buildutl.o eltree.o except.o fsa.o out.o param.o \
		  scan.o sref.o tree.o

CFLAGS 		= -DSYSV -DAIXV3 -D_ALL_SOURCE -D_AIX -D__AIX -D__aix -D_aix \
		  -Daix -DMULTIBYTE -DNLS16 -DMESSAGE_CAT -DAIX_ILS -DR5TREE \
		  -DVALUE_ADDED -qMAXMEM=50000

INCFLAGS 	= -I${MAKETOP}/gos/desktop/cde1/dthelp/parser/htag/util

LIBS 		= -lutil

ILIST 		= build2
IDIR 		= /opt/dt/etc/


.include <${RULES_MK}>

context.h : delim.bld context.dat
	- ${RM} -f context.h delim.h error delim.dat context.dat
	- ${CP} ${delim.bld:P} delim.dat
	- ${CP} ${context.dat:P} .
	${DTCONTEXT2} sparse

