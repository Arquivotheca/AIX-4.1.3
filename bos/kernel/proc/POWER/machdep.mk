# @(#)20        1.11  src/bos/kernel/proc/POWER/machdep.mk, Makefiles, bos411, 9430C411a 7/27/94 02:30:00
#
#   COMPONENT_NAME: SYSPROC
#
#   FUNCTIONS: none
#
#   ORIGINS: 27,83
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1988,1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


POWER_PINNED_OFILES	= 	\
	exception.o		\
	int_hproc.o		\
	jmp.o			\
	m_berk.o		\
	m_clock.o 		\
	m_fork.o		\
	m_ptrace.o		\
	m_thread.o		\
	p_slih.o		\
	reboot.o		\
	sig_slih.o		\
	scrub.o

POWER_PAGED_OFILES	= 	\
	fp_cpusync.o		\
	fp_trapstate_ker.o	\
	m_exec.o

POWER_INIT_OFILES	= 	\
	m_tinit.o 		\
	m_start.o

PWR_OFILES	= 		\
	complex_lock_pwr.o	\
	int_hproc_pwr.o		\
	simple_lock_pwr.o

PPC_OFILES	= 		\
	complex_lock_ppc.o	\
	complex_lock_instr_ppc.o \
	int_hproc_ppc.o 	\
	m_clock_ppc.o		\
	simple_lock_ppc.o	\
	simple_lock_instr_ppc.o \
	power_emul.o

complex_lock_instr_ppc.o_CFLAGS = ${CFLAGS} -D_INSTRUMENTATION

complex_lock_instr_ppc.c:       complex_lock_ppc.c
	${RM} -f ${.TARGET}
	${CP} ${complex_lock_ppc.c:P} ${.TARGET}

jmp.o_INCFLAGS          =-I../ml -I../ml/${TARGET_MACHINE}

simple_lock_instr_ppc.o_CFLAGS = ${CFLAGS} -D_INSTRUMENTATION

simple_lock_instr_ppc.c:        simple_lock_ppc.c
	${RM} -f ${.TARGET}
	${CP} ${simple_lock_ppc.c:P} ${.TARGET}
