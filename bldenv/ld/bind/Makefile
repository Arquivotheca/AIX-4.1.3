# @(#)19        1.8  src/bldenv/ld/bind/Makefile, ade_build, bos412, GOLDA411a 1/10/94 13:53:34
#
#   COMPONENT_NAME: BOSBUILD
#
#   FUNCTIONS:
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}bos/usr/ccs/bin/ld:${MAKETOP}bos/usr/ccs/bin/ld/bind
INCFLAGS	= -I${MAKETOP}bos/usr/ccs/bin/ld/bind
PROGRAMS	= bind
MSGHDRS		= bind_msg.h ld_msg.h
CATFILES	= bind.cat  ld.cat

ILIST		= bind
IDIR		= /usr/lib/

CCTYPE		= ansi
CFLAGS		= -D_ALL_SOURCE -U__STR31__ -Uaiws -DAIX_REL3 \
		  -DREAD_FILE -DSTATS -DXLC_BUG

HFILES		= ${MSGHDRS}  bind_msg.c

OFILES		= alloc.o	archive.o	bindmain.o \
		  cmd_list.o	code_save.o	commands.o \
		  dump.o	dump_cmd.o	error.o	\
		  global.o	glue.o		ifiles.o \
		  impexp.o	insert.o	match.o	\
		  objects.o	print.o		resolve.o \
		  save.o	shared.o	signal.o \
		  stab.o	strs.o		symbols.o \
		  symtab_save.o	util.o		xcoff.o

.include <${RULES_MK}>

bind_msg.c : bind.msg
	${RM} ${_RMFLAGS_} ${.TARGET}
	${SED} -e '/^$$/d;/^\$$/d;s/^[A-Z_a-z][A-Z_a-z0-9]*[ 	]*"/,"/' \
		${bind.msg:P} >${.TARGET}
