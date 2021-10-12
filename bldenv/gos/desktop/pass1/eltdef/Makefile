# @(#)13        1.1  src/bldenv/gos/desktop/pass1/eltdef/Makefile, desktop, bos412, GOLDA411a 5/3/94 16:39:06
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

VPATH 		= ${MAKETOP}/gos/desktop/cde1/dthelp/parser/pass1/eltdef:${MAKETOP}/gos/desktop/cde1/dthelp/parser/pass1/helptag

PROGRAMS 	= eltdef1
HFILES		= context.h dtd.h

OFILES 		=  eltdef.o eltutil.o entity.o scan.o

CFLAGS 		= -DSYSV -DAIXV3 -D_ALL_SOURCE -D_AIX -D__AIX -D__aix -D_aix \
		  -Daix -DMULTIBYTE -DNLS16 -DMESSAGE_CAT -DAIX_ILS -DR5TREE \
		  -DVALUE_ADDED -qMAXMEM=50000

INCFLAGS 	= -I${MAKETOP}/gos/desktop/cde1/dthelp/parser/pass1/eltdef \
		  -I${MAKETOP}/gos/desktop/cde1/dthelp/parser/pass1/build \
		  -I${MAKETOP}/gos/desktop/cde1/dthelp/parser/pass1/util

LIBS 		= -lutil1

ILIST 		= ${PROGRAMS}
IDIR 		= /opt/dt/etc/

.include <${RULES_MK}>

context.h:  delim.elt context.dat
	${RM} -f case.c context.h delim.h delim.dat error
	${CP} ${delim.elt:P} delim.dat
	${CP} ${context.dat:P} .
	../util/context1 || ${DTCONTEXT1}

dtd.h:	hptag.dtd
	${RM} -f arc.h dtd.h entity.h
	(../build/build1 < ${hptag.dtd:P}) || (${DTBUILD1} < ${hptag.dtd:P})
