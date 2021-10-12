# @(#)15        1.1  src/bldenv/gos/desktop/pass1/util/Makefile, desktop, bos412, GOLDA411a 5/3/94 16:40:03
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

VPATH 		= ${MAKETOP}gos/desktop/cde1/dthelp/parser/pass1/util

PROGRAMS	= context1

LIBRARIES 	= libutil1.a
EXPLIB_TARGETS 	= export_libutil1.a

EXPDIR 		= /usr/lib/

ILIST 		= ${PROGRAMS} 
IDIR 		= /opt/dt/etc/

LIBS		= -lutil1

CFLAGS		= -DSYSV -DAIXV3 -D_ALL_SOURCE -D_AIX -D__AIX -D__aix \
		  -D_aix -Daix -DMULTIBYTE -DNLS16 -DMESSAGE_CAT -DAIX_ILS

LIBOBJS		= allwhite.o inctest.o letter.o triedump.o triepdmp.o \
        	  trieplk.o trierset.o upcmp8.o upstrcmp.o wchar.o

CTXOBJS		= conutil.o error.o exit.o itoa.o lower.o malloc.o openchk.o \
        	  trie.o triecnt.o upper.o whitesp.o

libutil1.a_OFILES = ${LIBOBJS} ${CTXOBJS}

${PROGRAMS}_OFILES = context.o

.include <${RULES_MK}>
