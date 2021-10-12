# @(#)18        1.27  src/bos/kernel/ml/POWER/machdep.mk, sysml, bos41J, 9521A_all 5/23/95 13:58:37
#
#   COMPONENT_NAME: SYSML
#
#   FUNCTIONS: NONE
#
#   ORIGINS: 27, 83
#
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992, 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************

POWER_OBJECTS =	\
	low.o \
	ppc_end.o \
	pwr_end.o

POWER_PINNED_OFILES = \
	clock.o \
	copyx.o \
	copyx_pwr.o \
	disown_fp.o \
	forksave.o \
	misc.o \
	i_able.o \
	vmvcs.o \
	vmrte.o \
	wipl.o \
	flih_util.o \
	initutil.o \
	forkstack.o \
	mc_flih.o \
	sr_flih.o \
	dsis_flih.o \
	pr_flih.o \
	fp_flih.o \
	mltrace.o \
	dma_pwr.o \
	stubs.o \
	execexit.o \
	ppda.o \
        db_mp.o \
	misc_mp.o

POWER_DBG_OFILES = \
	debug.o

POWER_INIT_OFILES= \
	atomic_pwr.o \
	atomic_ppc.o \
	misc_init.o \
	disable_lock_rs1.o \
	disable_lock_rs2.o \
	disable_lock_ppc.o \
	disable_lock_instr_ppc.o \
	disable_lock_sof.o \
	lockl_ppc.o \
	lockl_instr_ppc.o \
	lockl_pwr.o \
	millicode.o \
	simple_lock_ppc.o \
	simple_lock_instr_ppc.o \
	simple_lock_pwr.o \
	tlb_flih.o

PWR_OFILES = \
	state_pwr.o \
	vmrte_pwr.o \
	fpi_flih.o \
	ex_flih_pwr.o \
	misc_pwr.o \
	cache_pwr.o \
	cpage_pwr.o \
	zpage_pwr.o

PPC_OFILES = \
	state_ppc.o \
	vmrte_ppc.o \
	flih_601.o \
	dec_flih.o \
	ex_flih_ppc.o \
	ex_flih_rspc.o \
	misc_ppc.o \
	cache_ppc.o \
	cpage_ppc_comb.o \
	cpage_ppc_splt.o \
	cpage_603.o \
	zpage_ppc.o \
	zpage_603.o \
	emulate.o \
	dsebackt.o \
 	clock_ppc.o \
	copyx_ppc.o

simple_lock_instr_ppc.o_M4FLAGS = ${M4FLAGS} -D_INSTRUMENTATION

simple_lock_instr_ppc.s:	simple_lock_ppc.s
	${RM} -f ${.TARGET}
	${CP} ${simple_lock_ppc.s:P} ${.TARGET}

disable_lock_instr_ppc.o_M4FLAGS = ${M4FLAGS} -D_INSTRUMENTATION

disable_lock_instr_ppc.s:	disable_lock_ppc.s
	${RM} -f ${.TARGET}
	${CP} ${disable_lock_ppc.s:P} ${.TARGET}

lockl_instr_ppc.o_M4FLAGS = ${M4FLAGS} -D_INSTRUMENTATION

lockl_instr_ppc.s:	lockl_ppc.s
	${RM} -f ${.TARGET}
	${CP} ${lockl_ppc.s:P} ${.TARGET}

