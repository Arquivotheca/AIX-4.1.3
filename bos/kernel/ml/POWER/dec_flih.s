# @(#)45	1.15  src/bos/kernel/ml/POWER/dec_flih.s, sysml, bos41J, 9511A_all 3/7/95 13:14:29
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: dec_flih
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************

	.file "dec_flih.s"
	.machine "ppc"
include(systemcfg.m4)
include(macros.m4)
ifdef(`_POWER_MP',`
include(low_dsect.m4)
',`
include(ppda.m4)
') #endif _POWER_MP

#*******************************************************************************
#
# NAME: dec_flih
#
# FUNCTION:
#       This flih is PPC specific.  The decrementer is serviced
#       at INTTIMER, so this priority must be respected by the flih.
#       The decrementer flih simply posts a software interrupt that
#       gets processed in finish_interrupt().
#
# PSEUDO CODE:
#       dec_flih()
#       {
#               extern struct ppd *ppdp;        /* ppda for this processor */
#
#               state_save();
#               begin_interrupt(DC_LEVEL, INTMAX<<8);
#               ppdp->i_softpri |= 0x80000000 >> INTTIMER;
#               finish_interrupt();
#       }
#
#*******************************************************************************

	.csect	dec_sect[PR]
DATA(dec_flih):
	.globl	DATA(dec_flih)
ENTRY(dec_flih):
	.globl	ENTRY(dec_flih)

	mflr	r0				# do a state save
	bl	ENTRY(state_save_pc)
	.extern ENTRY(state_save_pc)

	lil	r3, DC_LEVEL			# trace decrementer interrupt
	lil	r4, INTMAX*256
	bl	ENTRY(begin_interrupt)
	.extern	ENTRY(begin_interrupt)

ifdef(`_KDB',`
	LTOC(r3, kdb_avail, data)		# get address of kdb_avail
	l    	r3,0(r3)
	cmpi	cr0,r3,0
	beq	nokdb
	bl	ENTRY(kdb_check_action)
	.extern	ENTRY(kdb_check_action)
nokdb:
') #endif _KDB

ifdef(`INTRDEBUG',`
        lil     r3, 0x4320                      # DEC - Decrementer
        oriu    r3, r3, 0x4445
        lhz     r4, ppda_softpri(PPDA_ADDR)     # get current softpri value
	l	r5, ppda_intr(PPDA_ADDR)     	# address of intr control regs
        l       r6, xirr_poll(r5)		# get xirr without side effects
        bla     ENTRY(mltrace)
        .extern ENTRY(mltrace)
 ')

    	LTOC(r5,misc_intrs,data)	        # Load addr of misc_intrs
        lhz     r4, ppda_softpri(PPDA_ADDR)     # get current softpri value
	ATOMIC_INC(cr0,r3,r5)                   # Bump interrupt count
        ori     r4, r4, 0x8000>INTTIMER         # or in TIMER bit
        sth     r4, ppda_softpri(PPDA_ADDR)     # update softpri

	b	ENTRY(finish_interrupt)		# all done
	.extern	ENTRY(finish_interrupt)


	.toc
	TOCE(misc_intrs,data)
ifdef(`_KDB',`
	TOCE(kdb_avail, data)
') #endif _KDB

include(flihs.m4)
include(i_machine.m4)
include(mstsave.m4)
include(except.m4)

ifdef(`_POWER_MP',`
include(scrs.m4)
include(interrupt.m4)
') #endif _POWER_MP
