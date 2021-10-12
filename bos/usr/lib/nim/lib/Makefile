#  @(#)02 1.18  src/bos/usr/lib/nim/lib/Makefile, cmdnim, bos411, 9428A410j  12/8/93  15:39:52
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./lib/nim/lib/Makefile
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

OPT_LEVEL	= -g
STRIP_FLAG	= 

LIBRARIES   	= libnim.a libmstr.a

# NOT shipping the NIM libraries
ILIST		= 

IDIR		= /usr/lib/
EXPLIB_TARGETS  = export_libnim.a export_libmstr.a 

CFLAGS		= -D_BSD -qdbxextra

libnim.a_OFILES	=	nim_net.o \
			nim_gen.o \
			nim_exec.o \
			nim_err.o \
			cmdnim_msgdefaults.o

libmstr.a_OFILES= 	mstr_gen.o \
			mstr_obj.o \
			mstr_lock.o \
			mstr_state.o \
			mstr_exec.o \
			mstr_net.o \
			mstr_res.o \
			buglog.o

cmdnim_msgdefaults.c: cmdnim_msg defaults.awk 
	${ECHO} "#include \"cmdnim.h\" " > cmdnim_msgdefaults.c
	${AWK} -f defaults.awk cmdnim_msg >> cmdnim_msgdefaults.c 

defaults.awk: ../objrepos/cmdnim_mdefs.awk
	${CP} ${.ALLSRC} ${.TARGET} 

cmdnim_msg: ../objrepos/cmdnim.msg
	${CP} ${.ALLSRC} ${.TARGET} 

.include <${RULES_MK}>
