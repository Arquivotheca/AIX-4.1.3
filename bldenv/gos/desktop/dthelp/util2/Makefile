# @(#)73        1.1  src/bldenv/gos/desktop/dthelp/util2/Makefile, desktop, bos412, GOLDA411a 1/31/94 06:58:56
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

VPATH 		= ${MAKETOP}/gos/desktop/cde1/dthelp/parser2/pass1/util

PROGRAMS	= context2
context2_OFILES	= context.o

LIBRARIES 	= libutil2.a

EXPLIB_TARGETS 	= export_libutil2.a
EXPDIR 		= /usr/lib/

LIBS		= -lutil2

ILIST 		= context2
IDIR 		= /opt/dt/etc/

libutil2.a_OFILES = allwhite.o inctest.o letter.o triedump.o triepdmp.o \
			trieplk.o trierset.o upcmp8.o upstrcmp.o wchar.o \
			conutil.o error.o exit.o itoa.o lower.o malloc.o \
			openchk.o trie.o triecnt.o upper.o whitesp.o

.include <${RULES_MK}>
