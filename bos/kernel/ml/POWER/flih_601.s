# @(#)35	1.1.1.2  src/bos/kernel/ml/POWER/flih_601.s, sysml, bos411, 9428A410j 3/7/94 22:28:22
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: dse_flih run_flih
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential	Restricted when
# combined with	the aggregated modules for this	product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT	International Business Machines	Corp. 1992, 1993
# All Rights Reserved
#
# US Government	Users Restricted Rights	- Use, duplication or
# disclosure restricted	by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

	.file "flih_601.s"
	.machine "601"

#*******************************************************************************
#
# NAME:	dse_flih
#
#
#       The direct store error flih is only present on 601.  It is a 
#       front end for the DSI flih.  The dse_flih corrects changes   
#       status registers to make the DSE appear to be a DSI, and     
#       branches to the DSI flih.  This is done so that the 601 DSE  
#       interrupt has the exact same semantics as a T=1 DSI on other 
#       PPC processors.                                              
#
#       The DSE error is a synchronous interrupt used to report      
#       exceptions that occur on processor load/stores to T=1        
#       addresses.  It has the following characteristics:            
#
#          o Interrupt Vector 0xA00                                  
#
#          o SRR0 - Faulting IAR + 4                                 
#
#          o SRR1 - Saved MSR                                        
#
#          o DAR - Faulting effective address                         
#
#          o DSIER - Set as required by PPC arch.                     
#
# PSEUDO CODE:
#       dse_flih()                                                    
#       {                                                             
#
#               mtsrr0(mfsrr0()-4);     /* correct IAR */             
#               mtdsisr(DSISR_IO);      /* Set DSISR */               
#               call_ds_flih();         /* Branch to ds_flih */       
#       }                                                             
#
#*******************************************************************************

	.csect dse_flih_sect[PR]
DATA(dse_flih):
	.globl	DATA(dse_flih)
ENTRY(dse_flih):
	.globl	ENTRY(dse_flih)
	mtspr	SPRG1, r15		# save r15
	mfspr	r15, SRR0		# get fauling IAR
	addi	r15, r15, -4		# subtrace 4
	mtspr	SRR0, r15		# update SRR0
	lil	r15, 0			# T=1 DSI value
	oriu	r15, r15, (DSISR_IO>16)&0xFFFF
	mtspr	DSISR, r15		# create new DSISR
	mfspr	r15, SPRG0		# load with ppda pointer
	b	ds_flih			# jump to ds_flih
	.globl	ds_flih

#******************************************************************************
#
# NAME:	run_flih
#
# FUNCTION:
#	The 601	provides a machine dependent interrupt at vector
#	0x2000.	 This is the run mode interrupt.  The AIX debugger
#	uses this interrupt to implement the break on target
#	function.  The flih passes the interrupt to the	debugger for
#	processing.  If	the debugger does not claim the	interrupt
#	the system crashes.
#
# NOTES:
#	This flih is 601 specific.
#
# EXECUTION ENVIRONMENT:
#	Interrupt handler
#
#	this code should only execute if the kernel debugger
#	is in use
#
# PSEUDO CODE:
#
#	run_flih()
#	{
#		extern struct ppd *ppdp;	/* ppda	for this processor */
#		int rc;
#
#		state_save();
#		begin_interrupt(RUN_LEVEL, INTMAX<<8);
#
#		rc = debugger(ppdp->csa->prev, DBG_BTARGET, 0);
#
#		assert(rc == 0);
#
#		finish_interrupt();
#
#	}
#
#******************************************************************************

	.csect run_flih_sect[PR]
DATA(run_flih):
	.globl	DATA(run_flih)
ENTRY(run_flih):
	.globl	ENTRY(run_flih)

	mflr	r0				# call state save
	bl	ENTRY(state_save)
	.extern	ENTRY(state_save)

	lil	r3, RUN_LEVEL			# call begin interrupt
	lil	r4, INTMAX*256
	bl	ENTRY(begin_interrupt)
	.extern	ENTRY(begin_interrupt)

	mr	r3, INTR_MST			# get faulting mst
	lil	r4, DBG_BTARGET			# debugger reason code
	lil	r5, 0
	bl	ENTRY(debugger)			# call debugger
	.extern	ENTRY(debugger)

	tnei	r3, 0				# trap if debugger does	not
						# claim	interrupt

	b	ENTRY(finish_interrupt)
	.extern	ENTRY(finish_interrupt)



include(flihs.m4)
include(ppda.m4)
include(mstsave.m4)
include(scrs.m4)
include(i_machine.m4)
include(dbg_codes.m4)
include(machine.m4)
