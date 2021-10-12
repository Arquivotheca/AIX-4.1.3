# @(#)21        1.14  src/bos/kernext/pse/Makefile, sysxpse, bos41B, 412_41B_sync 1/6/95 08:08:51
#
#   COMPONENT_NAME: SYSXPSE - - STREAMS framework
#
#   FUNCTIONS: 
#
#   ORIGINS: 83
#
#   LEVEL 1, 5 Years Bull Confidential Information
#

.include <${MAKETOP}bos/kernext/Kernext.mk>


SUBDIRS		= mods

PSE		= pse
KERNEL_EXT	= ${PSE}
IDIR		= /usr/lib/drivers/${PSE}/
ILIST		= ${PSE} README.pse

${PSE}_ENTRYPOINT	= str_config

.if defined(KDB) && ${KDB} == "-D_KDB"
${PSE}_EXPORTS		= -bE:${PSE}.exp -bE:${PSE}_int.exp -bE:${PSE}_kdb.exp
.else
${PSE}_EXPORTS		= -bE:${PSE}.exp -bE:${PSE}_int.exp
.endif

${PSE}_IMPORTS		= -bI:tty.exp -bI:dbtty.exp -bI:debug.exp -bI:statcmd.exp -bI:threads.exp

${PSE}_LIBS = ${KERNEXT_LIBS}

#CDEBUGFLAGS	=\
#		-DSTREAMS_DEBUG


LOCALCFLAGS 	= \
		-DSTR_DEFINES \
		-D_KERNSYS \
		${CDEBUGFLAGS} \
		-DSVR4_STYLE

PORTFILES	= \
		str_select.o \
		str_clone.o \
		str_gpmsg.o \
		str_funnel.o \
		str_info.o

DEBUGFILES	= \
		str_kdb.o \
		str_debug.o 

STRFILES	= \
		sfix.o \
		str_init.o \
		str_memory.o \
		str_modsw.o \
		str_osr.o \
		str_runq.o \
		str_scalls.o \
		str_shead.o \
		str_subr.o \
		str_synch.o \
		str_util.o \
		str_weld.o \
		str_env.o \
		str_filesys.o\
		str_tty.o\
		mi.o \
		nd.o \
		str_config.o

MODSFILES	= \
		sad.o \
		log.o

OBJECTS		= \
		${PORTFILES} \
		${DEBUGFILES} \
		${STRFILES} \
		${MODSFILES}

${PSE}_OFILES	= \
		${OBJECTS} 

.include <${RULES_MK}>
