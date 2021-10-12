# @(#)21        1.1  src/bldenv/gos/desktop/canon1/parser/Makefile, desktop, bos412, GOLDA411a 5/3/94 16:51:58
#
#   COMPONENT_NAME: ade_build
#
#   FUNCTIONS: ODE_MakeSubdirs
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

HTAG 		= ${MAKETOP}gos/desktop/cde1/dthelp/parser/canon1

VPATH 		= ${HTAG}/parser:${HTAG}/helptag:${HTAG}/eltdef

PROGRAMS 	= parserc1
OTHERS		= ctag1
HFILES		= includes/context.h includes/if.h

OFILES		=  parser.o actutil.o closent.o code.o cyclent.o dcurelt.o \
        	   end.o entity.o error.o getsign.o input.o level.o lookent.o \
        	   minim.o optstrg.o param.o parent.o parutil.o pi.o \
        	   scan.o setpar.o start.o string.o struct.o text.o \
		   custom.o default.o help.o make.o option.o out.o xref.o

CFLAGS 		= -DSYSV -DAIXV3 -D_ALL_SOURCE -D_AIX -D__AIX -D__aix -D_aix \
		  -Daix -DMULTIBYTE -DNLS16 -DMESSAGE_CAT -DAIX_ILS -DR5TREE \
		  -DVALUE_ADDED -qMAXMEM=50000

INCFLAGS 	= -Iincludes -I${HTAG}/parser -I${HTAG}/util -I${HTAG}/helptag \
		  -I${HTAG}/build -I../eltdef

LIBS 		= -lutilc1 -bloadmap:test.map -lc -lm

ILIST 		= ${PROGRAMS} ${OTHERS} 
IDIR 		= /opt/dt/etc/

.include <${RULES_MK}>

includes/context.h includes/if.h: delim.par context.dat help.if
	${RM} -rf ${.TARGET:H}
	${MKDIR} ${.TARGET:H}
	${CP} ${delim.par:P} ${.TARGET:H}/delim.dat
	${CP} ${context.dat:P} ${.TARGET:H}
	cd ${.TARGET:H}; ${DTCONTEXTC1} || ../../util/contextc1
	${CP} ${help.if:P} ${.TARGET:H}
	cd ${.TARGET:H}; ${DTELTDEFC1} help.if || ../../eltdef/eltdefc1 help.if

${OTHERS}:  ${PROGRAMS} 
	${RM} -f ${OTHERS} 
	${CP} ${${PROGRAMS}:P} ${OTHERS} 
