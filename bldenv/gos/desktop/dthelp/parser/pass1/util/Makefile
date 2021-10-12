# @(#)49	1.1  src/bldenv/gos/desktop/dthelp/parser/pass1/util/Makefile, desktop, bos412, GOLDA411a 4/14/94 17:38:53
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

PROGRAMS	= context fclndir

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

context_OFILES	= context.o

fclndir_OFILES	= fclndir.o

.include <${RULES_MK}>
