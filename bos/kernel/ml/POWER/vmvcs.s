# @(#)51        1.68.1.37  src/bos/kernel/ml/POWER/vmvcs.s, sysml, bos41J, 9522A_all 5/30/95 18:13:00
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS:
#
# ORIGINS: 27 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************

	.file	"vmvcs.s"
	.machine "com"

include(macros.m4)

#****************************************************************************
#
#     Local macro definitions						    
#
#****************************************************************************

# Default MP is MP-efficient.

ifdef(`_POWER_MP',`
	define(`_VMM_MP_EFF',)
',)

# Disable machine via MSR, regardless of state of EIM

define(	DISABLE_MSR,
       `cal	$1, DISABLED_MSR(0)
	mtmsr	$1' )
define(	REAL_MSR,
       `cal	$1, DISABLED_REAL_MSR(0)
	mtmsr	$1' )
##page;

	.csect	ENTRY(vmvcs[PR])
	.set	VM_WAIT,-1
	.set	VM_NOWAIT,-9
	.set	VM_NOBACKT,0
	.set 	VM_BACKT,1
	.set	VM_INLRU,2
	.using	low,0
#***************************************************************************
# Begin	back-track setup						  
#									  
# On entry:								    
#									   
# r0	= address of subroutine	to invoke				  
# r1	= stack	pointer							   
# r3-r7	= parameters to	subroutine (note this limits any BTCALL	to 5 params)
#									  
# sr0,sr2 loaded with key 0 access					 
# msr in privileged state						
# running on process level						  
#   (no	BTCALL routine can be called on	an interrupt level --		  
#    since we enable to	INTPAGER we would be running more enabled than our 
#    caller. also, if return value is VM_WAIT we would lose machine state  
#    since it is not in	u-block	mst but	in interrupt mst which will get	   
#    stomped on	when another process runs).				  
#									 
# On exit:								   
#									    
# If return value from subroutine is VM_WAIT then the dispatcher is called the
# resume a different process. Current process is resumed some time later using
# the back-track mst (starts executing at label	backt).			   
# If return value is not VM_WAIT we return to caller with the return value in
# r3 and all non-volatile registers restored (the standard linkage conventions
# are observed).							      
#									      
#*****************************************************************************
#
# remember callers return address and machine state on stack
#
begbt:	mflr	r10			# get link register
	mfmsr	r11			# get current msr
	st	r10,-4(r1)		# save on stack
	st	r11,-8(r1)		# save on stack
	st	r13, -12(r1)		# save r13 on stack
#
# disable interrupts
#
	DISABLE_MSR(r12)
	GET_PPDA(cr0, r13)		# load PPDA pointer
#
# save back-track state	in callers mstsave area
#
	l	r11,ppda_csa(r13)	# r11->mstsave of caller
	cal	r8,0(0)			# zero first word of mst
	st	r8,except(r11)		#  except struct (org vaddr)
	cal	r8,EXCEPT_DSI(0)	# set except pfault type to
	st	r8,except4(r11)		#  DSI (in case	of an exception).

	st	r12,mstmsr(r11)		# save msr
	stm	r0,mstgpr(r11)		# save all gprs
	mfcr	r8			# get condition	register
	LTOC(r12,backt,entry)		# back-track resume addr
	st	r8, mstcr(r11)		# save condition register
	st	r12,mstiar(r11)		# set mst's IAR	to point to
					#   back-track resume address
ifdef(`_KDB',`
	st	r10,mstlr(r11)		# save link register
') #endif _KDB

#
# save all sregs
#
	mfsr	r16,sr0			# get sreg 0
	mfsr	r17,sr1			# get sreg 1
	mfsr	r18,sr2			# get sreg 2
	mfsr	r19,sr3			# get sreg 3
	mfsr	r20,sr4			# get sreg 4
	mfsr	r21,sr5			# get sreg 5
	mfsr	r22,sr6			# get sreg 6
	mfsr	r23,sr7			# get sreg 7
	mfsr	r24,sr8			# get sreg 8
	mfsr	r25,sr9			# get sreg 9
	mfsr	r26,sr10		# get sreg 10
	mfsr	r27,sr11		# get sreg 11
	mfsr	r28,sr12		# get sreg 12
	mfsr	r29,sr13		# get sreg 13
	mfsr	r30,sr14		# get sreg 14
	mfsr	r31,sr15		# get sreg 15
	stm	r16,mstsr(r11)		# save sreg 0 -	15
	lm	r16,mstgpr+16*4(r11)	# restore r16-r31
##page;
#
# back-track resume address, we	can get	here 3 different ways:
# (1) falling through from code	above
# (2) v_xxxx routine page-faults and we	resume back-track mst
# (3) v_xxxx routine returns VM_WAIT and we resume back-track mst sometime later
# r0 has subroutine address, interrupts	disabled.
#
	.using	vmkerdata,r12
ENTRY(backt):
	LTOC(r12,vmker,data)
	l	r11,vmmsrval		# get sreg val for vmmdseg
	l	r12,ptasrval		# get sreg val for ptaseg
	.drop	r12
	mtsr	vmmsr,r11		# load vmmsr
	mtsr	ptasr,r12		# load ptasr
DATA(isync_vcs1):
	.globl	DATA(isync_vcs1)	# patched at SI time
	isync
	mtctr	r0			# move ep of routine to	ctr
#
# get new mstsave mark as in pager back-track
#
	GET_PPDA(cr0, r13)		# re-load PPDA pointer (thread could
					# have been redispatched on another cpu)
	l	r11,ppda_csa(r13)	# r11->current mstsave
	l	r10,ppda_mstack(r13)	# r10 -> new mstsave
	l	r9,mstsralloc(r11)	# get current sralloc state
	oriu	r9,r9,0x0018		# mark vmmsr, ptasr allocated
	st	r10,ppda_csa(r13)	# set csa pointer
	CSA_UPDATED(cr0, r12, r10)

	st	r9,mstsralloc(r10)	# set mstsralloc in new	mstsave
	cal	r12,pgbackt(0)		# r12 =	INTPAGER*256 +	1
	sth	r12,mstintpri(r10)	# mark mstsave
	cal	r12,0(0)		# load r12 with	zero
	st	r12,mstkjmpbuf(r10)	# clear	jump buffer pointer
	st	r12,excbranch(r10)	# clear	exception branch addr
	st	r12,mststackfix(r10)	# clear stackfix pointer
	cal	r11,-framesize(r10)	# r11 -> next mstsave
	st	r11,ppda_mstack(r13)	# set pointer to next mstsave
ifdef(`_VMM_MP_EFF',`
	lil	r12,VM_BACKT		# indicate we are in backtrack
	stb	r12,ppda_lru(r13)	#   via the ppda field
')

#
# code must be disabled	to here; now we	enable to proper
# interrupt priority level (INTPAGER)
#
	l	r11, syscfg_arch(0)	# load architecture type
	l	r12, syscfg_impl(0)	# load implementation type
	cmpi	cr0, r11, POWER_PC	# check	for Power PC
	cmpi	cr1, r12, POWER_RS2	# check	for RS2
	beq	cr0, be_05		# branch if we are PPC
	LTOC(r8,i_data,data)		# get priority mapping array
	beq	cr1, be_dis_rs2		# branch if RS2

#
#	RS1/RSC	machine	specific function
#
	
	l	r11, INTPAGER*8(r8)	# load EIM values for INTPAGER
	l	r12, INTPAGER*8+4(r8)

	mfsr	r9, sr15		# save callers'	seg reg	15
	cau	r8, 0, BUID0		# gain access to BUID0,	where the
	mtsr	sr15, r8		#   EIS	and EIM	reside
	cau	r8, 0, 0xF000		# fabricate address of I/O space
	stsi	r11, r8, 8		# write	new EIM	values
	mtsr	sr15, r9		# restore caller's segment reg 15
	b	be_05

#
#	RS2 machine specific function
#

be_dis_rs2:
	lbz	r11, INTPAGER(r8)	# get new CIL value
	rlinm	r11, r11, 8, 0xFF00	# generate ILCR	command
	mtspr	ILCR, r11		# update CIL value
	muli	r3, r3, 1		# delay after CIL update
	muli	r3, r3, 1

be_05:
	mfmsr	r8			# get MSR value	into r8
	oril	r8, r8,	MSR_EE		# turn on EE bit
	mtmsr	r8			#   in MSR
	cal	r1,-stkmin(r10)		# set initial stack pointer
#
# call target routine:
#
# r1 = fixed stack in new mststack frame
# r3-r7	= parameters
# VMMSR, PTASR loaded
# interrupts enabled
#
	bctrl				# go to	routine

ifdef(`_VMM_MP_EFF',`
#
# call v_pfsio if some i/o has been scheduled in the critical section
#
	lbz 	r6,ppda_sio(r13)	# get sio flag
	cmpi	0,r6,0			# test for set
	beq	bnosio
	mr	r14,r3			# save return code
	bl	ENTRY(v_pfsio)		# call start i/o 
       .extern	ENTRY(v_pfsio)

	mr	r3,r14			# restore return code
bnosio:
')

#
# normal return	from routine is	here. give back	mstsave
#
	DISABLE_MSR(r6)			# disable interrupts
	l	r6,ppda_csa(r13)	# get current mstsave
	l	r7,mstprev(r6)		# get previous mstsave pointer
	l	r1,mstgpr+4(r7)		# get original stack frame ptr

ifdef(`_VMM_MP_EFF',`
	lil	r14,VM_NOBACKT		# indicate no longer in backtrack
	stb	r14,ppda_lru(r13)	#   via ppda field
')
	l	r14,mstgpr+14*4(r7)	# restore r14 (used to save r3)
	st	r7,ppda_csa(r13)	# set csa pointer to previous
	st	r6,ppda_mstack(r13)	# put mstsave back on stack
	CSA_UPDATED(cr0, r4, r7)

#
# if return value is VM_WAIT then dispatch some	other process.
# this process will be resumed later using the back-track state	mst
# (starts execution at label backt)
#
	cmpi	0,r3,VM_WAIT		# test for vm_wait
	beq	be_10			# or
	cmpi	0,r3,VM_NOWAIT		# vm_nowait
	bne	be_15
be_10:
	b	ENTRY(call_dispatch)	# call dispatcher to resume some
       .extern	ENTRY(call_dispatch)	# process

be_15:
#
# returning to caller so restore non-volatile registers
# that we have altered.
# restore sregs	for vmmsr,ptasr, and tempsr. restore msr.
# r1 = original	stack pointer, r7->callers mstsave.
#
	l	r10,mstsr+4*vmmsr(r7)	# get vmmsr sreg value
	l	r11,mstsr+4*ptasr(r7)	# get ptasr sreg value
	l	r12,mstsr+4*tempsr(r7)	# get tempsr sreg value
	mtsr	vmmsr,r10		# restore vmmsr
	mtsr	ptasr,r11		# restore ptasr
	mtsr	tempsr,r12		# restore tempsr
DATA(isync_vcs2):			# patched at SI time
	.globl	DATA(isync_vcs2)
	isync

	l	r10, syscfg_arch(0)	# get processor	arch. type
	l	r11, syscfg_impl(0)	# get processor	implementation
	cmpi	cr0, r10, POWER_PC	# check	for Power PC arch.
	cmpi	cr1, r11, POWER_RS2	# check	for RS2	implementation
	lbz	r9, mstintpri(r7)	# get caller's int. prio
	beq	cr0, be_ret		# branch if a Power PC
	LTOC(r8,i_data,data)		# address priority masking table
	beq	cr1, be_en_rs2		# branch if a RS2

#
#	RS1/RSC	specific code
#

	mfsr	r12, sr15		# save caller's	seg reg	15
	rlinm	r9, r9,	3, 0x7F8	# multiply intpri * 8
	a	r9, r9,	r8		# get address of masks
	lsi	r10, r9, 8		# load new mask	value
	cau	r0, 0, BUID0		# gain access to BUID0,	where the
	mtsr	sr15, r0		#   EIS	and EIM	reside
	cau	r9, 0, 0xF000		# create EIM address
	stsi	r10, r9, 8		# write	new mask values
	mtsr	sr15, r12		# restore caller's segment reg 15
	b	be_ret

#
#	RS2 specific code
#

be_en_rs2:
	lbzx	r10, r9, r8		# get new CIL value
	rlinm	r10, r10, 8, 0xFF00	# generate ILCR	command
	mtspr	ILCR, r10		# update CIL
	muli	r3, r3, 1		# delay after CIL update
	muli	r3, r3, 1

#	If a runrun is set to a non zero value run the dispatcher.  Otherwise
#	restore altered registers and return
be_ret:
	l	r4, DATA(runrun)	# get runrun flag
	lhz	r7, ppda_softpri(r13)	# get software interrupt priority
be_sw_ret:
	l	r5, -4(r1)		# get link register
	or.	r8, r4, r7		# check if runrun and softpri are zero	
	l	r6, -8(r1)		# get msr
	mtlr	r5			# load link register
	l	r13, -12(r1)		# restore r13
	bne-	be_csoft		# runrun or softpri set
	mtmsr	r6			# restore msr
DATA(vmvcs_nop):			# 603e workaround
	.globl	DATA(vmvcs_nop)
	cror	0,0,0			# patched to br on non 603e 1.4
	br				# return if runrun == 0

#	call do_soft or swtch do not require a stack frame.  do_soft
#	will run dispatcher (if required) after servicing software
#	interrupt.
#
#	Critical sections are always called at INTBASE (except si)
#	so there is no check on callers priority

be_csoft:
	cmpi	cr1, r7, 0		# r7 - pending interrupts
	mr	r13, r3			# save return value
	beq+	cr1, be_cswtch		# call swtch()

	bl	ENTRY(i_dosoft)		# call i_dsoft
	.extern	ENTRY(i_dosoft)
	b	be_cont

be_cswtch:
	bl	ENTRY(swtch)
	.extern	ENTRY(swtch)

be_cont:
	mr	r3, r13			# restore return value
	lil	r4, 0			# don't run switch again
	lil	r7, 0			# don't run i_dosoft again
	b	be_sw_ret		# exit

#
# exception handler for vcs_excp calls.  On a exception in
# backtracking functions.  Controll is passed here.  Error
# code is in r3.  Clean up and return to caller
#
btexcp:
	l	r11, -4(r1)		# get link register
	l	r12, -8(r1)		# get caller's msr
	l	r13, -12(r1)
	mtlr	r11
	mtmsr	r12
	cror	0,0,0			# 603e 1.4 workaround
	br

##page;
#****************************************************************************
#									     
# Begin	fixed-stack setup						      
#									     
# On entry:								      
#									      
# r0	= address of subroutine	to invoke				      
# r1	= stack	pointer							      
# r3-r8	= parameters to	subroutine (note this limits any FSTCALL to 6 params) 
#									      
# sr0,sr2 loaded with key 0 access					      
# msr in privileged state						      
# running on process level OR interrupt	level				      
#   (since we run at caller's interrupt	level if he is more disabled it	is    
#    ok	to call	FSTCALL	routine	on an interrupt	level -- HOWEVER, it is	not   
#    legal to call an FSTCALL routine which might return VM_WAIT since the    
#    machine state is not in u-block mst but in	interrupt level	mst which     
#    will get stomped on when another process runs).			      
#									      
# On exit:								      
#									      
# If return value from subroutine is VM_WAIT then we call swtch	to save	the   
# state	of this	process	and to resume a	different process. Current process wil
# be resumed at	some later time	at the return from the call to the vcs routine
# If return value is not VM_WAIT we return to caller with the return value in 
# r3 and all non-volatile registers restored (the standard linkage conventions
# are observed).							      
#									      
#*****************************************************************************
#
# get a	mstsave	area. set priority to INTPAGER unless callers priority
# is higher (more disabled).
#
begfst:	mfmsr	r9			# get msr
	st	r13, -4(r1)		# save r13 on callers stack
	st	r14, -8(r1)		# save r14 on callers stack
	DISABLE_MSR(r12)		# disable
	GET_PPDA(cr0, r13)		# load address of ppda
	l	r11,ppda_csa(r13)	# r11 -> current mstsave
	l	r10,ppda_mstack(r13)	# r10 -> new mstsave
	l	r12,mstsralloc(r11)	# get current mstsralloc
	oriu	r12,r12,0x0010		# mark vmmsr allocated
	st	r12,mstsralloc(r10)	# set in new mstsave
	cal	r12,0(0)		# load zero in r12
	st	r12,mstkjmpbuf(r10)	# clear	jump buffer pointer
	st	r12,excbranch(r10)	# clear	exception branch addr
	lbz	r12,mstintpri(r11)	# callers interrupt priority	
	st	r10,ppda_csa(r13)	# set csa pointer to next mstsave
	CSA_UPDATED(cr0, r11, r10)

	cmpi	cr0,r12,INTPAGER	# compare callers priority and INTPAGER
	ble	fs_10			# branch if use	callers
	cal	r12,INTPAGER(0)		# use INTPAGER
fs_10:	rlinm	r12,r12,8,0xFF00	# shift	priority to right location
	sth	r12,mstintpri(r10)	# set priority - no back-track flag
	cal	r11,-framesize(r10)	# r11 -> next mstsave
	st	r11,ppda_mstack(r13)	# set pointer to next mstsave
#
# save msr, link reg, stack pointer and	VMMSR in new mstsave.
# set up stack pointer and ctr register	for called program
#
	mtctr	r0			# set ctr to entry address
	mflr	r11			# get link register
	mfsr	r0,vmmsr		# get contents of VMMSR	sreg
	st	r1,-16(r10)		# save stack pointer here
	st	r9,-12(r10)		# save callers msr here
	st	r11,-8(r10)		# save link register here
	st	r0,-4(r10)		# save sreg here
	cal	r1,-(stkmin+16)(r10)	# stack	pointer	for called program

#
# 	Set interrupt priority
#

	l	r10, syscfg_arch(0)	# get processor	arch.
	l	r11, syscfg_impl(0)	# get processor	implementation
	cmpi	cr0, r10, POWER_PC	# check	for Power PC arch.
	cmpi	cr1, r11, POWER_RS2	# check	for RS2	impl.
	mr	r0, r9			# save msr value
	beq	cr0, fs_20		# branch if Power PC
	LTOC(r10,i_data,data)		# address priority mapping array
	beq	cr1, fs_dis_rs2		# branch if RS2

	rlinm	r12,r12,27,0x7F8	# priority * 8
	a	r12, r10, r12		# get offset into mask array
	lsi	r10, r12, 8		# load EIMs
	mfsr	r12, sr15		# save callers'	seg reg	15
	cau	r9, 0, BUID0		# get io addressability
	mtsr	sr15, r9		# set up IO segment
	cau	r9, 0, 0xF000		# get address of EIMs
	stsi	r10, r9, 8		# write	new EIMs
	mtsr	sr15, r12		# restore caller's segment reg 15
	b	fs_20

fs_dis_rs2:
	rlinm	r12, r12, 24, 0xff	# fix priority
	lbzx	r11, r12, r10		# get new CIL value
	rlinm	r11, r11, 8, 0xFF00	# generate ILCR	command
	mtspr	ILCR, r11		# update CIL
	muli	r3, r3, 1		# delay after CIL update
	muli	r3, r3, 1

fs_20:
	mtmsr	r0
#
# get addressability to	vmmdseg
#
	.using	vmkerdata,r12
	LTOC(r12,vmker,data)		# r12->vmker
	l	r11,vmmsrval		# get sreg val for vmmdseg
	.drop	r12
	mtsr	vmmsr,r11		# load vmmsr
DATA(isync_vcs3):			# patched at SI time
	.globl	DATA(isync_vcs3)
	isync
#
# call target routine:
#
# r1 = fixed stack in new mststack frame
# r3-r8	= parameters
# VMMSR	loaded
# msr as set by	caller to vcs routine
#
	bctrl				# go to	routine
#
# return from routine is here. give back mstsave
#
	DISABLE_MSR(r6)			# disable interrupts
	l	r6,ppda_csa(r13)	# get current mstsave
	l	r7,mstprev(r6)		# get previous mstsave pointer
	st	r7,ppda_csa(r13)	# set csa pointer to previous
	st	r6,ppda_mstack(r13)	# put mstsave back on stack
	CSA_UPDATED(cr0, r4, r7)

#
# restore contents of sreg, callers stack pointer and set link reg
# to return address.
#
	l	r1,-16(r6)		# callers stack	pointer
	l	r5,-12(r6)		# callers msr
	l	r0,-8(r6)		# callers link reg
	l	r4,-4(r6)		# callers VMMSR	sreg

	l	r10, syscfg_arch(0)	# get processor arch
	l	r11, syscfg_impl(0)	# get processor impl
	cmpi	cr0, r10, POWER_PC	# check for Power PC arch.
	cmpi	cr1, r11, POWER_RS2	# check for RS2 impl.
	mtlr	r0			# restore link reg
	mtsr	vmmsr,r4		# restore VMMSR	sreg
DATA(isync_vcs4):			# patched at SI time
	.globl	DATA(isync_vcs4)
	isync
	lbz	r9, mstintpri(r7)	# get caller's int. prio
	beq	cr0, fs_30		# branch if Power PC
	LTOC(r8,i_data,data)		# address of EIM mask table
	beq	cr1, fs_en_rs2		# branch if a RS2	

#
#	RSC/RS1 specific code
#

	mfsr	r12, sr15		# save caller's	seg reg	15
	rlinm	r9, r9,	3, 0x7F8	# multiply prio	by 4 for word index
	a	r9, r8,	r9		# get offset of	EIMs
	lsi	r10, r9, 8		# load new EIMS
	cau	r0, 0, BUID0		# gain access to BUID0,	where the
	mtsr	sr15, r0		#   EIS	and EIM	reside
	cau	r9, 0, 0xF000		# get address of EIMs
	stsi	r10, r9, 8		# write	new EIMs
	mtsr	sr15, r12		# restore caller's segment reg 15
	b	fs_30

#
#	RS2 specific code
#
fs_en_rs2:
	lbzx	r10, r8, r9		# get new CIL value
	rlinm	r10, r10, 8, 0xFF00	# generate ILCR command
	mtspr	ILCR, r10		# update CIL
	muli	r3, r3, 1		# delay after CIL update
	muli	r3, r3, 1
#
# return to caller unless return value was VM_WAIT, or there are soft
# interrupts pending
#
fs_30:
	lhz	r6, ppda_softpri(r13)	# check for software interrupts
	cmpi	cr0,r3,VM_WAIT		# test for vm_wait
	cmpi	cr1, r6, 0		# check for software interrupts
	crorc	cr7*4+eq, cr0*4+eq, cr1*4+eq
	beq-	cr7, fs_wait		# branch if softint or VM_WAIT
	mtmsr	r5			# restore msr
	l	r13, -4(r1)		# restore registers
	l	r14, -8(r1)
	br


# must stay disabled until another thread is dispatched.  Call swtch()
# if return was VM_WAIT or i_dsoft if ppda_softpri is set

fs_wait:
	mflr	r13			# save link register
	mr	r14, r5			# save callers msr

	bne	cr0, fs_dosoft		# branch if !VM_WAIT
	bl	ENTRY(swtch)		# on return from swtch there will
	lil	r3, VM_WAIT		# return code
	b	fs_return		# be no servicable softints pending

fs_dosoft:
	lbz	r8, mstintpri(r7)	# get current priority
	sli	r6, r6, 16		# covert to bitindex to priority
	cntlz	r9, r6
	cmpi	cr7, r8, r9		# compare current againt pending
	ble-	cr7, fs_return		# return if
	st	r3, -12(r1)		# save return code
	bl	ENTRY(i_dosoft)
	l	r3, -12(r1)		# restore return code

fs_return:
	mtlr	r13			# restore link register
	mtmsr	r14
	l	r13, -4(r1)		# restore r14
	l	r14, -8(r1)
	br				# return

#
#	This is exception handler for fixed stack vcs calls with
#	exception handler.  The exceptions are delivered while
#	in the above swtch.
#

fsexcp:

#	Do a sanity check on MSR value before loading it

	andil.	r4, r14, MSR_IR|MSR_DR|MSR_ME	# these bits should always be on
	cmpi	cr1, r4, MSR_IR|MSR_DR|MSR_ME
	beq+	cr1, fs_msrok			# branch if MSR good
	TRAP
fs_msrok:
	mtlr	r13				# restore state and return
	mtmsr	r14
	l	r13, -4(r1)
	l	r14, -8(r1)
	br

##page;
#****************************************************************************
#									     
# Begin	pcs setup						      
#									     
# On entry:								      
#									      
# r0	= address of machine-dependent routine to invoke
# r1	= stack	pointer							      
# r3-r8	= parameters to	routine (note this limits any PCSCALL to 6 params) 
#									      
# sr0,sr2 loaded with key 0 access					      
# msr in privileged state						      
# running on process level OR interrupt	level				      
#									      
# On exit:								      
#									      
#****************************************************************************
begpcs:
ifdef(`_KDB',`
	mflr	r9			# get and save link register
	st	r9,stklink(r1)		# for debug, before Xlate is off
') #endif _KDB
	mfmsr	r9			# get msr
	REAL_MSR(r12)			# disable and turn xlate off
DATA(isync_vcs5):			# patched at SI time
	.globl	DATA(isync_vcs5)
	isync

#
# save msr, link reg, and stack pointer in local fixed stack
# set up stack pointer and ctr register	for called program
#
	GET_PPDA(cr0, r10)		# get address of PPDA
	l	r10,ppda_pmapstk(r10)	# r10->V=R stack for this processor

	mtctr	r0			# set ctr to entry address
	mflr	r11			# get link register
	st	r1,-12(r10)		# save stack pointer here
	st	r9,-8(r10)		# save callers msr here
	st	r11,-4(r10)		# save link register here
ifdef(`_KDB',`
	st	r1,(stkback-stkmin-12)(r10)# set back-chain for debug
') #endif _KDB
	cal	r1,-stkmin-12(r10)	# stack	pointer	for called program


#
# call target routine:
#
# r1 = local fixed stack
# r3-r8	= parameters
# msr set with interrupts disabled, xlate off
#
	bctrl				# go to	routine

#
# restore contents of sreg, callers stack pointer and set link reg
# to return address.
#
	l	r0,stkmin+8(r1)		# callers link reg
	l	r6,stkmin+4(r1)		# callers msr
	l	r1,stkmin+0(r1)		# callers stack	pointer

	mtlr	r0			# restore link reg
	mtmsr	r6			# restore msr
DATA(isync_vcs6):			# patched at SI time
	.globl	DATA(isync_vcs6)
	isync
	br				# return to pcs_ caller

#
# macro	for generating calls with switch to fixed-stack
#

# FSTCALL	is used for fixed stack.
#		in mp, we use a wrapper to take and release the vmm lock.
#		since this lock is a blocking lock we need to follow the
#		backtracking assembler path.
#
ifdef(`_POWER_MP',`
define(FSTCALL,
	       `.globl ENTRY(vcs_$1)
ENTRY(vcs_$1):	 LTOC(r0,v_pre$1,entry)
ifdef(`_VMM_MP_EFF',`
		 b    begfst
',`
		 b    begbt
')
		.toc
		 TOCE(v_pre$1,entry)
		.csect ENTRY(vmvcs[PR])')
',`
define(FSTCALL,
	       `.globl ENTRY(vcs_$1)
ENTRY(vcs_$1):	 LTOC(r0,v_$1,entry)
		 b    begfst
		.toc
		 TOCE(v_$1,entry)
		.csect ENTRY(vmvcs[PR])')
')

# FSTRCALL	is used for fixed stack cs that are called from ih or where
#		VM_WAIT return code has a special meaning.
#		they cannot block and thus are real fixed stack using
#		the fixed stack assembler path.
#		if the mpsafe lock must be taken, it is taken spin lock
#		to avoid returning VM_WAIT as with the blocking lock.
#		im mp, we use a wrapper even if it is a direct call to the
#		v_ routine.
#
ifdef(`_POWER_MP',`
define(FSTRCALL,
	       `.globl ENTRY(vcs_$1)
ENTRY(vcs_$1):	 LTOC(r0,v_pre$1,entry)
		 b    begfst
		.toc
		 TOCE(v_pre$1,entry)
		.csect ENTRY(vmvcs[PR])')
',`
define(FSTRCALL,
	       `.globl ENTRY(vcs_$1)
ENTRY(vcs_$1):	 LTOC(r0,v_$1,entry)
		 b    begfst
		.toc
		 TOCE(v_$1,entry)
		.csect ENTRY(vmvcs[PR])')
')

#
# macro	for generating calls with back-track
#

# BTCALL	is used for backtrackable critical section.
#		in mp, we use a wrapper to take and release the vmm lock.
#
ifdef(`_POWER_MP',`
define(BTCALL,
	       `.globl ENTRY(vcs_$1)
ENTRY(vcs_$1):	 LTOC(r0,v_pre$1,entry)
		 b    begbt
		.toc
		 TOCE(v_pre$1,entry)
		.csect ENTRY(vmvcs[PR])')
',`
define(BTCALL,
	       `.globl ENTRY(vcs_$1)
ENTRY(vcs_$1):	 LTOC(r0,v_$1,entry)
		 b    begbt
		.toc
		 TOCE(v_$1,entry)
		.csect ENTRY(vmvcs[PR])')
')

#
# macro for generating vcs (back-tracking or fixed stack)
# calls with exception handling.  A fast exception handler
# is set up before calling the normal vcs entry.  This avoids
# a setjmpx/clrjmpx around performance critical vcs calls
#
# USAGE: VCS_EXCP(function, BT|FS)
#	BT for back-tracking functions
#	FS for Fixed stack functions
#
define(VCS_EXCP,`
ENTRY(vcs_$1_excp):
		.globl	ENTRY(vcs_$1_excp)
		mflr	r0
		st	r31, -8(r1)
		st	r0, stklink(r1)
		GET_CSA(cr0, r12, r31)
		ifelse($2,BT,
			`LTOC(r0, btexcp, data)',
		$2,FS,
			`LTOC(r0, fsexcp, data)',
		`errprint(ERROR:illegal argument)
		 mexit(1)')
		stu	r1, -(8+stkmin)(r1)
		st	r0, excbranch(r31)
		bl	ENTRY(vcs_$1)
		l	r4, 8+stkmin+stklink(r1)
		lil	r0, 0
		mtlr	r4
		st	r0, excbranch(r31)
		cal	r1, (8+stkmin)(r1)
		l	r31, -8(r1)
		br
')
		

#
# macro	for generating calls to machine-dependent routines
#
define(PCSCALL,
	       `.globl ENTRY(pcs_$1)
ENTRY(pcs_$1):	 LTOC(r0,p_$1,entry)
		 b    begpcs
		.toc
		 TOCE(p_$1,entry)
		.csect ENTRY(vmvcs[PR])')
#
# back track and fix-stack entry points
# NOTE:	BTCALL routines	cannot take more than 5	parameters
#	FSTCALL	routines cannot	take more than 6 parameters
#	(due to	registers used in setup	code above)
#
	BTCALL(allocfree)
	BTCALL(frealloc)
	VCS_EXCP(frealloc,BT)
	BTCALL(getcio)
	BTCALL(requeio)
	BTCALL(freefrags)
	BTCALL(movedaddr)
	BTCALL(cleardata)
	BTCALL(create)
	BTCALL(delete)
	BTCALL(deletelws)
	BTCALL(extendag)
	BTCALL(freeseg)
	BTCALL(dqlock)
	BTCALL(frlock)
	BTCALL(getxpt)
	BTCALL(gettblk)
	BTCALL(getvmap)
	BTCALL(growxpt)
	BTCALL(inherit)
	VCS_EXCP(makep,BT)
	BTCALL(makep)
	BTCALL(makelogp)
	BTCALL(mapd)
	BTCALL(mapv)
	BTCALL(mvfork)
	BTCALL(promote)
	BTCALL(pdtdelete)
	BTCALL(pbitfree)
	BTCALL(pbitalloc)
	VCS_EXCP(pin,BT)
	BTCALL(pin)
	BTCALL(allociblk)
	BTCALL(freeiblk)
	BTCALL(release)
	BTCALL(setsize)
	BTCALL(write)
	BTCALL(writelogp)
	BTCALL(protect)
	BTCALL(protectp)
	BTCALL(cpfork)
	BTCALL(flush)
	BTCALL(movep)
	BTCALL(freedq)
	BTCALL(unlockdq)
	BTCALL(config)
	BTCALL(clrfrag)
	BTCALL(dirfrag)
	BTCALL(movefrag)
	BTCALL(relfrag)
ifdef(`_VMM_MP_EFF',`
	BTCALL(lru)
')
	FSTCALL(devpdtx)
	FSTRCALL(freescb)
	FSTCALL(frtblk)
	VCS_EXCP(iowait,FS)
	FSTRCALL(iowait)
	FSTRCALL(interrupt)
	FSTCALL(lockseg)
	FSTCALL(pdtinsert)
	FSTCALL(qmodify)
	FSTCALL(qpages)
	FSTCALL(setlog)
	FSTCALL(unlockseg)
	FSTCALL(defer)
	FSTCALL(pscount)
	FSTCALL(inactive)
	VCS_EXCP(powait,FS)
	FSTRCALL(powait)
	BTCALL(getame)
	FSTCALL(freeame)
	FSTCALL(relalias)
	FSTRCALL(protectap)
	BTCALL(invalidate)
	BTCALL(gettlock)
	BTCALL(vmm_lock)
	BTCALL(vmm_unlock)
	BTCALL(`sync')
	VCS_EXCP(unpin,BT)
	BTCALL(unpin)
	FSTCALL(limits) 
	FSTCALL(psearlyalloc)
	FSTCALL(pslatealloc)
	FSTCALL(getfree)
	FSTCALL(insfree)
	FSTCALL(syncwait)
	BTCALL(hfblru)
	BTCALL(lruwait)
#
# pmap entry points
# NOTE:	PCSCALL routines cannot take more than 6 parameters
#	(due to	registers used in setup	code above)
#
	PCSCALL(enter)
	PCSCALL(rename)
	PCSCALL(remove)
	PCSCALL(remove_all)
	PCSCALL(is_modified)
	PCSCALL(is_referenced)
	PCSCALL(clear_modify)
	PCSCALL(protect)
	PCSCALL(page_protect)
	PCSCALL(lookup)

#
# Define function descriptor to be modified by bigfoot performance tool to
# call out to kernel extension rather than to dummy routine defined here.
#
	.globl ENTRY(BF_kext)
	ENTRY(BF_kext):
	br
	FCNDES(BF_kext,label)

##page;
	.csect ENTRY(vmvcs[PR])

include(mstsave.m4)
include(low_dsect.m4)
include(scrs.m4)
include(intr.m4)
include(vmker.m4)
include(machine.m4)
include(except.m4)
include(systemcfg.m4)
	.csect ENTRY(vmvcs[PR])
	.toc
	TOCL(backt,entry)
	TOCE(vmker,data)
	TOCE(i_data,data)
	TOCL(btexcp,data)
	TOCL(fsexcp,data)
