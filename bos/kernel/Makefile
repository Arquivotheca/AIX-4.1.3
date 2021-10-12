# @(#)60        1.30  src/bos/kernel/Makefile, kernel, bos41J, 9521B_all 5/26/95 13:11:28
#
# COMPONENT_NAME: KERNEL
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernel/Kernel.mk>

EXPINC_SUBDIRS	= include
EXPLIB_SUBDIRS	= lib

.if defined(KDB) && ${KDB} == "-D_KDB"
KDBDIR	= kdb
.else
KDBDIR	=
.endif

SUBDIRS		= \
	include ml lfs ipc io ios ldr net pfs proc s si specfs uipc \
	vmm epost db exp lib ${KDBDIR}

.if defined(MP)
.if (${MP} == "_mp") || (${MP} == "_mp_real") || (${MP} == "_603_DEC")
KERNEL          = unix${MP}
.else
KERNEL          = unix_up
.endif
.endif

ILIST		= ${KERNEL} ${KERNEL}.strip
IDIR		= /
${KERNEL}_IDIR  = /usr/lib/boot/
NOUPDATE_LIST	= ${IDIR}${KERNEL}.strip

KSTART		= start
PIN		=${KERNEL}pin.o
PAGE_PRIM       =${KERNEL}pg_prim.o
PAGE            =${KERNEL}pg.o
PAGE_BACK       =${KERNEL}pg_back.o
DEBUG		= dbgpg.o
PPCO		=${KERNEL}ppc.o
PWRO		=${KERNEL}pwr.o
INITO		=${KERNEL}init.o

PLDFLAGS	= -r -bloadmap:${.PREFIX}.out -bR:${.PREFIX}.map \
			${PLDENV} ${PLDARGS}
KLDFLAGS	= -bnoreorder -bnoglink -T0 -D-1 -H 8 -bdbg:loadabs \
                        -bloadmap:${.PREFIX}.out -bR:${.PREFIX}.map \
                        ${KLDENV} ${KLDARGS}

STRIPFLAGS	= -Kernel

.if defined(KDB) && ${KDB} == "-D_KDB"
KDBEXTS = kdb/kdb.exp
.else
KDBEXTS =
.endif

IMPORTS         = exp/${TARGET_MACHINE}/ipl.exp exp/lowsys.exp
EXPORTS		= exp/tracecalls.exp exp/kernex.exp exp/dead.exp \
		  exp/syscalls.exp exp/statcmd.exp exp/threads.exp \
		  exp/dbtty.exp exp/sockets.exp exp/loadfs.exp exp/debug.exp \
		  exp/tty.exp exp/streams.exp exp/pm.exp ${KDBEXTS}
MISSING		= exp/${TARGET_MACHINE}/missing.exp

DUMMY=xx
SYSLIB		=${DUMMY:!${FINDFILE} ${LIBDIRS} libsysp.a!}
CSYSLIB		=${DUMMY:!${FINDFILE} ${LIBDIRS} libcsys.a!}

K_OFILES	= \
	ml/low.o	\
	${PIN}		\
	${SYSLIB}	\
	${CSYSLIB}	\
	ml/vr_end.o	\
	${PPCO}		\
	ml/ppc_end.o	\
	${PWRO}		\
	ml/pwr_end.o	\
	${DEBUG}	\
	db/dbg_end.o	\
        ${PAGE_PRIM}    \
        ${PAGE}         \
        ${PAGE_BACK}    \
	ml/pg_end.o	\
	${INITO}	\
	ml/last.o

KERN_OFILES	= ${K_OFILES:!${ECHO} ${K_OFILES}!}

.include <${RULES_MK}>

${PIN}	:	PINNED
	${RM} -f ${.TARGET} ${.PREFIX}.out ${.PREFIX}.map
	${LD} ${PLDFLAGS} -o ${.TARGET} -f PINNED

${PAGE_PRIM} :  PAGED_PRIM
	${RM} -f ${.TARGET} ${.PREFIX}.out ${.PREFIX}.map
	${LD} ${PLDFLAGS} -o ${.TARGET} -f PAGED_PRIM

${PAGE} :       PAGED
	${RM} -f ${.TARGET} ${.PREFIX}.out ${.PREFIX}.map
	${LD} ${PLDFLAGS} -o ${.TARGET} -f PAGED

${PAGE_BACK} :  PAGED_BACK
	${RM} -f ${.TARGET} ${.PREFIX}.out ${.PREFIX}.map
	${LD} ${PLDFLAGS} -o ${.TARGET} -f PAGED_BACK

${DEBUG}:	DBG
	${RM} -f ${.TARGET} ${.PREFIX}.out ${.PREFIX}.map
	${LD} ${PLDFLAGS} -o ${.TARGET} -f DBG

${PPCO} :	PPC
	${RM} -f ${.TARGET} ${.PREFIX}.out ${.PREFIX}.map
	${LD} ${PLDFLAGS} -o ${.TARGET} -f PPC

${PWRO} :	PWR
	${RM} -f ${.TARGET} ${.PREFIX}.out ${.PREFIX}.map
	${LD} ${PLDFLAGS} -o ${.TARGET} -f PWR

${INITO}:	INIT
	${RM} -f ${.TARGET} ${.PREFIX}.out ${.PREFIX}.map
	${LD} ${PLDFLAGS} -o ${.TARGET} -f INIT


${KERNEL}: ${K_OFILES} ${IMPORTS} ${EXPORTS} ${MISSING} ${MAKEFILE_DEPENDENCY}
	${RM} -f ${.TARGET} ${.PREFIX}.out ${.PREFIX}.map
	@[ -L ${.PREFIX}.map ] && ${RM} -f ${.PREFIX}.map ; true
	${LD} ${KLDFLAGS} -e ${KSTART} -o ${.TARGET} ${KERN_OFILES} \
	-bf ${MISSING:@TMP@-bI:${${TMP}:P}@} \
	${IMPORTS:@TMP@-bI:${${TMP}:P}@} \
	${EXPORTS:@TMP@-bE:${${TMP}:P}@}

${KERNEL}.strip:	${KERNEL}
	${RM} -f ${.TARGET} ${.PREFIX}.out ${.PREFIX}.map
	${CP} ${KERNEL} ${.TARGET}
	${STRIP} ${STRIPFLAGS} ${.TARGET}

