# @(#)50	1.7.1.14  src/bos/kernel/proc/Makefile, sysproc, bos41J, 9507A 2/8/95 16:09:02
#
#   COMPONENT_NAME: SYSPROC
#
#   FUNCTIONS: none
#
#   ORIGINS: 27, 83
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1988, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernel/Kernel.mk>

VPATH		=${TARGET_MACHINE}

INCFLAGS	=-I${TARGET_MACHINE}

PAGED_OFILES_PRIM =             \
        sysproc.o
PAGED_OFILES    =               \
        acct.o                  \
        getproc.o               \
        resource.o              \
        sbreak.o                \
	sigsvcs.o		\
        ulimit.o                \
        ${${TARGET_MACHINE}_PAGED_OFILES}
PAGED_OFILES_BACK =             \
	cmp_swap.o		\
        encap.o                 \
        plock.o                 \
        profil.o                \
        uprintf.o               \
        usrinfo.o

PINNED_OFILES	=		\
	clock.o			\
	core.o			\
	complex_lock_com.o	\
	complex_lock_instr_com.o \
	simple_lock_com.o	\
	simple_lock_instr_com.o	\
	dispatch.o		\
	encap_pn.o		\
	exec.o			\
	exit.o			\
	fork.o          	\
	forkstack.o		\
	gettod.o		\
        jmpx.o          	\
	ksendsig.o		\
	lock_alloc.o		\
	lock_alloc_instr.o	\
	lockl.o			\
	lockl_instr.o		\
	newproc.o		\
	newthread.o		\
	pmzone.o		\
	pri_funcs.o     	\
	psmon.o			\
	ptrace.o		\
	pwrmgt.o		\
	resource_pn.o		\
	sched.o			\
	setpinit.o		\
	sig.o			\
	sigsvcs_pn.o       	\
	sleep.o			\
	sleep2.o		\
	sysproc_pn.o    	\
	thread.o		\
	times.o			\
	timevalops.o		\
        tscalls.o               \
	tscalls_pn.o		\
	uexcept.o		\
	uprintf_pn.o		\
	v_waitrdy.o		\
	watchdog.o		\
	${${TARGET_MACHINE}_PINNED_OFILES}

INIT_OFILES	=		\
	init_lock.o		\
	strtdisp.o		\
	tinit.o			\
	${${TARGET_MACHINE}_INIT_OFILES}

complex_lock_instr_com.o_CFLAGS = ${CFLAGS} -D_INSTRUMENTATION

complex_lock_instr_com.c:       complex_lock_com.c
	${RM} -f ${.TARGET}
	${CP} ${complex_lock_com.c:P} ${.TARGET}

init_lock.o_CFLAGS = ${CFLAGS} -D_INSTRUMENTATION

lock_alloc_instr.o_CFLAGS = ${CFLAGS} -D_INSTRUMENTATION

lock_alloc_instr.c:	lock_alloc.c
	${RM} -f ${.TARGET}
	${CP} ${lock_alloc.c:P} ${.TARGET}

lockl_instr.o_CFLAGS = ${CFLAGS} -D_INSTRUMENTATION

lockl_instr.c:  lockl.c
	${RM} -f ${.TARGET}
	${CP} ${lockl.c:P} ${.TARGET}

simple_lock_instr_com.o_CFLAGS = ${CFLAGS} -D_INSTRUMENTATION

simple_lock_instr_com.c:        simple_lock_com.c
	${RM} -f ${.TARGET}
	${CP} ${simple_lock_com.c:P} ${.TARGET}

.include <${TARGET_MACHINE}/machdep.mk>

.include <${RULES_MK}>
