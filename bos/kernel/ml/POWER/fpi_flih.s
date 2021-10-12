# @(#)41        1.3  src/bos/kernel/ml/POWER/fpi_flih.s, sysml, bos411, 9428A410j 3/7/94 22:19:39
#
#   COMPONENT_NAME: SYSML
#
#   FUNCTIONS: ENTRY
#		ppda_curthread
#
#   ORIGINS: 27,83
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************

	.file "fpi_flih.s"
	.machine "pwrx"

ifdef(`_POWER_RS2',`
#*******************************************************************************
#
# NAME:	fpi_flih
#
# FUNCTION:
#	This flih is only available on RS2 machines.  It passes
#	control	to p_slih() to do all the real work. The floating
#	unit must be synced before calling p_slih(), to	be sure	all
#	exceptions have been occurred.
#
# PSEUDO CODE:
#	fpi_flih()
#	{
#		struct mstsave *intr_mst;
#		extern struct ppd *ppdp;	/* ppda	for this processor */
#		int oldmsr;
#
#		state_save();
#		begin_interrupt(FI_LEVEL, INTMAX<<8);
#
#		/* sync	floating point unit
#		 */
#		ics();				/* sync	fpu */
#
#		intr_mst = ppdp->csa->prev;
#	#ifdef _THREADS
#		p_slih(intr_mst, intr_mst->iar,	EXCEPT_FLOAT_IMPRECISE,
#								ppd->curthread);
#	#else
#		p_slih(intr_mst, intr_mst->iar,	EXCEPT_FLOAT_IMPRECISE,
#								ppd->curproc);
#	#endif
#		finish_interrupt();
#	}
#
#*******************************************************************************

	.csect	fpi_sect[PR]
DATA(fpi_flih):
	.globl	DATA(fpi_flih)
ENTRY(fpi_flih):
	.globl	ENTRY(fpi_flih)

	mflr	r0				# do full state	save
	bl	ENTRY(state_save)
	.extern	ENTRY(state_save)

#	sync fpu
	ics					# sync floating	point unit

	lil	r3, FI_LEVEL			# call begin interrupt
	lil	r4, INTMAX*256
	bl	ENTRY(begin_interrupt)
	.extern	ENTRY(begin_interrupt)

	mr	r3, INTR_MST			# call p_slih
	l	r4, mstiar(INTR_MST)
	lil	r5, EXCEPT_FLOAT_IMPRECISE
ifdef(`_THREADS',`
	l	r6, ppda_curthread(PPDA_ADDR)
',`
	l	r6, ppda_curproc(PPDA_ADDR)
')
	bl	ENTRY(p_slih)
	.extern	ENTRY(p_slih)

	b	ENTRY(finish_interrupt)		# all done
	.extern	ENTRY(finish_interrupt)
',)

include(flihs.m4)
include(i_machine.m4)
include(mstsave.m4)
include(except.m4)
include(ppda.m4)
include(machine.m4)
