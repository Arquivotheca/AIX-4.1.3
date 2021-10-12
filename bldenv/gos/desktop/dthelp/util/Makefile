# @(#)72        1.1  src/bldenv/gos/desktop/dthelp/util/Makefile, desktop, bos412, GOLDA411a 1/31/94 06:58:32
#
#   COMPONENT_NAME: DESKTOP
#
#   FUNCTIONS: I
#		ODE_NormalLibraryTarget
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

VPATH 		= ${MAKETOP}/gos/desktop/cde1/dthelp/parser/htag/util

PROGRAMS	= context

LIBRARIES 	= libutil.a
EXPLIB_TARGETS 	= export_libutil.a

EXPDIR 		= /usr/lib/

ILIST 		= context
IDIR 		= /opt/dt/etc/

LIBS		= -lutil

libutil.a_OFILES = allwhite.o inctest.o letter.o triedump.o triepdmp.o \
			trieplk.o trierset.o upcmp8.o upstrcmp.o wchar.o \
			conutil.o error.o exit.o itoa.o lower.o malloc.o \
			openchk.o trie.o triecnt.o upper.o whitesp.o

.include <${RULES_MK}>
