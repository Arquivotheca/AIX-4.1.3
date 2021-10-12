# @(#)47	1.1  src/bldenv/gos/desktop/dthelp/parser/pass1/eltdef/Makefile, desktop, bos412, GOLDA411a 4/14/94 17:37:58
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

VPATH 		= ${MAKETOP}/gos/desktop/cde1/dthelp/parser/htag/eltdef:${MAKETOP}/gos/desktop/cde1/dthelp/parser/htag/helptag

PROGRAMS 	= eltdef
HFILES		= context.h dtd.h

OFILES 		=  eltdef.o eltutil.o entity.o scan.o

CFLAGS 		= -DSYSV -DAIXV3 -D_ALL_SOURCE -D_AIX -D__AIX -D__aix -D_aix \
		  -Daix -DMULTIBYTE -DNLS16 -DMESSAGE_CAT -DAIX_ILS -DR5TREE \
		  -DVALUE_ADDED -qMAXMEM=50000

INCFLAGS 	= -I${MAKETOP}/gos/desktop/cde1/dthelp/parser/htag/eltdef \
		  -I${MAKETOP}/gos/desktop/cde1/dthelp/parser/htag/build \
		  -I${MAKETOP}/gos/desktop/cde1/dthelp/parser/htag/util

LIBS 		= -lutil

ILIST 		= eltdef
IDIR 		= /opt/dt/etc/

.include <${RULES_MK}>

context.h:  delim.elt context.dat
	${RM} -f case.c context.h delim.h delim.dat error
	${CP} ${delim.elt:P} delim.dat
	${CP} ${context.dat:P} .
	${DTCONTEXT}

dtd.h:	hptag.dtd
	${RM} -f arc.h dtd.h entity.h
	${DTBUILD} < ${hptag.dtd:P}
