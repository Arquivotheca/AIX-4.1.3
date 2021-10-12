# @(#)69	1.4  src/bos/kernel/ml/POWER/disable_lock_sof.s, sysml, bos41J, 9521A_all 5/23/95 14:46:52
#
#*****************************************************************************
#
#   COMPONENT_NAME: SYSML
#
#   FUNCTIONS:	i_disable_sof
#		disable_lock_sof
#		i_enable_sof
#		unlock_enable_sof
#		i_pri_or_run
#
#   ORIGINS: 27
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

include(macros.m4)
	.machine "com"

#****************************************************************************
# NOTES:
#	These procedures are pinned and only fault if called on a
#	pageable stack. Touches the stack while @ INTMAX and disabled
#	to ensure the pages are pinned.
#
#	No vm critical section
#	No fixed stack
#	No backtrack
#	Can page fault on callers stack
#
#	These functions are used by the software managed interrupt model.
#	The MSR(EE) bit is not turned off at INTMAX.
#
#	THESE FUNCTIONS HAVE NOT BEEN MPed.  The pseudo code reflects
#	how to MP it.
#
#-----------------------------------------------------------------------#
# /*
#  *  i_disable -- disable interrupts to selected priority.
#  *
#  *  disable_lock -- disable interrupts to selected priority and lock
#  *		      the selected simple_lock.
#  *
#  *  NOTE: i_disable/disable_lock are overlays, therefore the external
#  *	    labels will be different.
#  *
#  *	On entry:
#  *	   r2 = Kernels TOC pointer
#  *	   r3 = New interrupt prio level (parameter 1) i_disable()
#  *	   r4 = simple lock addr (prarmeter 2) disable_lock()
#  *	Returns:
#  *	   r3 = Old interrupt priority level
#  *
#  *	exit:	selected interrupts logically disabled
#  *		current kernel stack region logically pinned
#  */
# int disable_lock(new,lockaddr)
# int new;				/* desired interrupt priority */
# simple_lock_t lockaddr;		/* selected lock */
# {
#	register int old;		/* previous interrupt priority */
#	register char *fp;		/* current stack pointer */
#	volatile char touch;		/* used to touch stack pages */
#
# i_disable_ep:			/* entry point for i_disable() */
#	lockaddr = NULL;
#
# disable_lock_ep:		/* entry point for disable_lock() */
#	old = csa->intpri;	
#	if (old > new)	{	
#		if (old == INTBASE)  {	
#			csa->stackfix = fp;	/* mark stack "fixed" */
#			touch = fp[STACKTOUCH];
#			touch = fp[0];
#			touch = fp[-STACKTOUCH];
#		}
#		csa->intpri = new;
# ifdef _POWER_MP
#		ppdp->stackfix = 1;
# endif
#		if (new == INTMAX)
#			MSR(EE) = 0;
#	}
#	if (lockaddr)
#		simple_lock(lockaddr);
#	return(old);
# }
#-----------------------------------------------------------------------#
ifdef(`_POWER_MP',`',`
	.csect	disable_lock_sof_ovl[PR] 
DATA(disable_lock_sof):
	.globl	DATA(disable_lock_sof)

	.using	low, r0
#
# NOTE:  No other instructions should be placed before i_disable_ep
#        or between i_disable_ep and disable_lock_ep.
#        This "hardcodes" the disable_lock entry point to be 4 bytes
#        from the i_disable entry point.
#	 This must exist whether or not we are compiled with _POWER_MP!
#
i_disable_ep:
	li	r4, 0			# i_disable() - lockaddr == NULL

disable_lock_ep:
	GET_CSA(cr1, r11, r6)		# r6 -> current save area
	cmpi	cr6, r3, INTMAX		# check for INTMAX
	mr	r5, r3			# r5 = new int priority
	lbz	r3, mstintpri(r6)	# r3 = old int priority

ifdef(`INTRDEBUG',`
#	mltrace:    whacks r7, r8, r9, r10, cr0 
#		    stores r3, r4, r5, r6
	mr	r11, r4			# save lockaddr
	mfcr	r12			# save condition register
	mr	r4, r3
	mflr	r0
	lil	r3, 0x4953		# IDIS for i_disable
        oriu	r3, r3, 0x4944
	bla	ENTRY(mltrace)
	.extern	ENTRY(mltrace)

	mr	r3, r4			# move it back
	mr	r4, r11			# restore lockaddr
	mtcrf	0xff, r12		# restore condition register
	mtlr	r0
')

	cmp	cr7, r3, r5
	cmpi	cr1, r3, INTBASE

ifdef(`_POWER_MP',`
	cmpi	cr0, r4, 0		# check for NULL lockaddr
	li	r9, 1			# used for ppda_stackfix
	ble+	cr7, i_dis_lock 	# if already sufficiently disabled
',`
	blelr+	cr7			# UP return
')
	bne-	cr1, i_dis001		# if previously disabled

#	Fix the pages around the current stack pointer.
#	This sequence may fault on the stack.
#	Whenever anyone faults with mststackfix non-null, resume()
#	will carefully touch the same area we are touching here.

	stw	r1, mststackfix(r6)	# indicate stack fix in effect
	lwz	r0, STACKTOUCH(r1)
	lwz	r10, 0(r1)
	lwz	r11, -STACKTOUCH(r1)

#	Set new interrupt priority.

i_dis001:
	stb	r5, mstintpri(r6)	# Set new interrupt priority
ifdef(`_POWER_MP',`
	GET_PPDA(cr7, r8)		# r8 -> PPDA
	stb	r9, ppda_stackfix(r8)	# Set PPDA stackfix for LRU
	bne-	cr6, i_dis_lock
',`
	bnelr-	cr6			# On UP return.
')
	mfmsr	r7			# at INTMAX turn off EE bit
	rlwinm	r7, r7, 0, ~MSR_EE
	mtmsr	r7

i_dis_lock:
ifdef(`_POWER_MP',`
	beqlr+				# no lock so return
	mflr	r7			# go get lock - save return addr	
	stw	r7, stklink(r1)
	stw	r3, -4(r1)		# save old priority
	mr	r3, r4
	stwu	r1, -(stkmin+8)(r1)	# buy a stack frame
	bla	ENTRY(simple_lock)
	.extern	ENTRY(simple_lock)
	la	r1, (stkmin+8)(r1)
	lwz	r3, -4(r1)		# get old priority
	lwz	r7, stklink(r1)		# get return addr
	mtlr	r7
')
	# If a 603e 1.4 and below then do the that special thing.
	# 603s and 603e 1.5 and beyond do the normal ppc thing.
	# This decision is made in hardinit() and if not a 1.4 the cror
	# below will be a blr
DATA(ids_603_patch):
	.globl	DATA(ids_603_patch)		
	cror	0, 0, 0
	blr				# return from i_disable()
')

#-----------------------------------------------------------------------#
# /*
#  *  i_enable -- enables interrupts to selected priority.
#  *
#  *  unlock_enable -- for compatiblity
#  *
#  *  NOTE: i_enable/unlock_enable are overlays, therefore the external
#  *	    labels will be different.
#  *
#  *	On entry:
#  *	   r2 = Kernels TOC pointer
#  *	   r3 = Old interrupt prio level (parameter 1) i_enable()
#  *	   r4 = Lock address (parameter 2) unlock_enable()
#  *	Returns:
#  *	   none
#  *
#  *	exit:
#  *		selected interrupts enabled based on priority
#  *		stack unfixed if level is restored to INTBASE (ie. if
#  *		at process level)
#  */
#
# NOTE: This version should work on the MP kernel that is running on
#	an UP box.  It will not work when the MP kernel is running on
#	a MP box.  More care in getting and holding the PPDA address
#	is needed.
#
# void unlock_enable(old,lockaddr)
# {
#
# i_enable_ep:			/* entry point for i_enable()		    */
#	if( MP )
#		goto i_su_enable;
#	else
#		nop		/* just fall through			    */
#
# unlock_enable_ep:		/* entry point for unlock_enable()	    */
#	if( MP )
#		simple_unlock(lockaddr);
#
# i_su_enable:
#	current = csa->intpri;	
#	if (old > current)	
#	{
#		csa->intpri = old;	  /* restore old priority	     */
#		if( current == INTMAX )
#			MSR(EE) = 1
#		if ( i_softpri || runrun )
#			i_pri_or_run()
#		if (old == INTBASE) {
#			if( MP )
#				ppda->stackfix = 0
#			mststackfix = NULL;	
#		}
#	}	
# }
#-----------------------------------------------------------------------#

ifdef(`_POWER_MP',`',`
	.csect	unlock_enable_sof_ovl[PR] 
DATA(unlock_enable_sof):
	.globl	DATA(unlock_enable_sof)

       .using	low, r0
#
# NOTE:  No other instructions should be placed before i_enable_ep
#        or between i_enable_ep and unlock_enable_ep.
#        This "hardcodes" the unlock_enable entry point to be 4 bytes
#        from the i_enable entry point.
#        This must exist whether or not we are compiled with _POWER_MP!
i_enable_ep:
ifdef(`_POWER_MP',`
	b	i_su_enable		# on MP do not unlock
',`
	nop				# fall into i_su_enable on UP
')

unlock_enable_ep:
ifdef(`_POWER_MP',`
	mflr	r5			# save return address
	stw	r3, -4(r1)		# save target priority
	stw	r5, stklink(r1)
	stwu	r1, -(stkmin+8)(r1)	# buy a stack frame
	mr	r3, r4			# get lock address
	bla	ENTRY(simple_unlock)	# go unlock
	.extern	ENTRY(simple_unlock)
	la	r1, (stkmin+8)(r1)
	lwz	r5, stklink(r1)		# restore return address
	lwz	r3, -4(r1)		# restore target priority
	mtlr	r5
')


i_su_enable:
	GET_CSA(cr0, r6, r5)		# r5 -> current save area
	lbz	r6, mstintpri(r5)	# r6 = current interrupt priority

ifdef(`INTRDEBUG',`
#	mltrace:    whacks r7, r8, r9, r10, cr0 
#		    stores r3, r4, r5, r6
	mr	r4, r3			# Save target priority & lr
	mflr	r0
	lil	r3, 0x4e41		# IENA for i_enable
	oriu	r3, r3, 0x4945

	bla	ENTRY(mltrace)

	mr	r3, r4			# move it back
	mtlr	r0
')

ifdef(`_POWER_MP',`
	GET_PPDA(cr0, r7)		# r7 -> PPDA
')
	cmpi	cr6, r3, INTBASE        # check for INTBASE
	cmp	cr1, r3, r6             # if current priority more favored than
	cmpi	cr0, r6, INTMAX		# check for moving off of INTMAX
	blelr+	cr1                     #   old, do not change, return
	stb     r3, mstintpri(r5)	# restore old priority
	bne-	cr0, i_enable_01	# go round setting EE should be on
	mfmsr	r4			# coming off of INTMAX - set EE bit
	ori	r4, r4, MSR_EE
	mtmsr	r4

i_enable_01:
ifdef(`_POWER_MP',`
	lhz	r8, ppda_softpri(r7)	# get interrupt queue
',`
	lhz	r8, softpri		# get interrupt queue
')
	lwz	r9, DATA(runrun)	# dispatchers "runrun" flag
	or.	r4, r8, r9		# if either set go off to figure it out
	bne-	i_pri_or_run

	bnelr-	cr6			# not going back to INTBASE
	li	r0, 0			# clear fixed stack indicators
ifdef(`_POWER_MP',`
	stb	r0, ppda_stackfix(r7)	# MP LRU
')
	stw	r0, mststackfix(r5)	#
	blr				# return to caller
')

#-----------------------------------------------------------------------#
# i_pri_or_run -- An interrupt is queued or runrun is set
#
# FUNCTION: If an interrupt is queued, i_softpri, then set and go
#	process it.  If runrun is set then set up and call swtch().
#
# NOTE: Only called from i_enable_sof().  Must immediately follow it
#	so it can be copied down in the overlay for i_enable_sof().
#	This way the relative branches still work.
#
# On entry:
#	r2 = kernels TOC pointer
#	r3 = current priority
#	r4 = mstprev value
#	r5 = csa pointer
#	r8 = i_softpri
#	cr6 = EQ == INTBASE
#
# Destorys:
#	r0
# Returns:
#	none
#
# exit:
#
# void i_pri_or_run()
# {
#	q_pri = clz32(i_softpri)
#	if( q_pri ) {
#		i_dosoft()
#	if( (priority == INTBASE) && runrun ) {
#		swtch()
#	}	
# }
#-----------------------------------------------------------------------#

ifdef(`_POWER_MP',`',`
i_pri_or_run:

	lwz	r4, mstprev(r5)		# process level pointer
	rlwinm	r8, r8, 16, 0xffff0000	# move to upper bits
	cntlzw	r0, r8			# find most favored priority
	cmp	cr0, r3, r0		# if current <= most favored queued
					#	then no interrupts to do
	mflr	r0			# save return addr to i_enable() caller
	ble-	i_runrun

	stw	r0, stklink(r1)		# buy a stack frame to save
	stw	r3, -4(r1)		# current priority
	stw	r5, -8(r1)		# r5 -> csa

ifdef(`_POWER_MP',`
	stw	r7, -12(r1)		# r7 -> ppda
	stwu	r1, -(stkmin+16)(r1)
',`
	stwu	r1, -(stkmin+12)(r1)
')
	bla	ENTRY(i_dosoft)		# go process queued interrupts
	.extern	ENTRY(i_dosoft)

ifdef(`_POWER_MP',`
	la	r1, (stkmin+16)(r1)	# restore info
	lwz	r7, -12(r1)		# r7 -> ppda
',`
	la	r1, (stkmin+12)(r1)	# restore info
')
	lwz	r3, -4(r1)		# current priority
	lwz	r0, stklink(r1)		# reload return address
	lwz	r5, -8(r1)		# r5 -> csa
	cmpi	cr6, r3, INTBASE	# check for INTBASE
	mtlr	r0			# put return addr in LR
i_runrun:
	bnelr-	cr6			# return to i_enable caller
					#	if not at INTBASE
	lwz	r8, DATA(runrun)
	li	r0, 0			# clear fixed stack indicators
	cmpi	cr1, r8, 0		# check for runrun set
ifdef(`_POWER_MP',`
	stb	r0, ppda_stackfix(r7)	# MP LRU
')
	stw	r0, mststackfix(r5)	#

	beqlr+	cr1			# return to i_enable() caller 
					#	if runrun not set
	tnei	r4, nullA		# die if not at process level
	mflr	r0			# save return addr to i_enable() caller
	stw	r0, stklink(r1)
	bla	ENTRY(swtch)		# call swtch()
	.extern	ENTRY(swtch)

	lwz	r0, stklink(r1)		# reload return address
	mtlr	r0			# put return addr in LR
	blr				# return to i_enable() caller
')


	_DF(_DF_NOFRAME)

	.toc
	TOCE(i_data,data)

include(param.m4)
include(systemcfg.m4)
include(scrs.m4)
include(low_dsect.m4)
include(mstsave.m4)
include(intr.m4)
include(machine.m4)
include(flihs.m4)
