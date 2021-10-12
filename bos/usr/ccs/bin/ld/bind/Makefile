# @(#)34        1.7  src/bos/usr/ccs/bin/ld/bind/Makefile, cmdld, bos411, 9428A410j 9/24/93 09:35:58
# COMPONENT_NAME: (CMDLD) - xcoff linkage editor
#
# FUNCTIONS: Makefile for the binder
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1991, 1993 
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH			= ../.

PROGRAMS		= bind
MSGHDRS                 = bind_msg.h ld_msg.h
CATFILES                = bind.cat ld.cat

ILIST			= bind
IDIR			= /usr/ccs/bin/

CCTYPE			= ansi
CFLAGS			= -D_ALL_SOURCE -DREAD_FILE -DSTATS -DXLC_BUG

HFILES			= ${MSGHDRS}  bind_msg.c

OFILES			= alloc.o	archive.o	bindmain.o	\
			cmd_list.o	code_save.o	commands.o	\
			dump.o		dump_cmd.o	error.o		\
			global.o	glue.o		ifiles.o	\
			impexp.o	insert.o	match.o		\
			objects.o	print.o		resolve.o	\
			save.o		shared.o	signal.o	\
			stab.o		strs.o		symbols.o	\
			symtab_save.o	util.o		xcoff.o

GARBAGE			=

.include <${RULES_MK}>


bind_msg.c: bind.msg
	${SED} -e '/^$$/d;/^\$$/d;s/^[A-Z_a-z][A-Z_a-z0-9]*[ 	]*"/,"/' \
		${bind.msg:P} > ${.TARGET}
