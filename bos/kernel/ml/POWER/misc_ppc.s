# @(#)44        1.23  src/bos/kernel/ml/POWER/misc_ppc.s, sysml, bos41J, 9518A_all 5/2/95 08:52:29
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: misc. functions
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1992, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************
undefine(`_POWER_RS1')
undefine(`_POWER_RSC')
undefine(`_POWER_RS2')
undefine(`_POWER_RS')

	.file "misc_ppc.s"
	.machine "ppc"

        .using  low, 0

include(macros.m4)


#*******************************************************************************
#
# NAME: mfdec_ppc
#
# FUNCTION: read Power PC Decrementer
#
# EXECUTION ENVIRONMENT:
#	Called though the branch table
#
# RETURNS: decrementer value
#
#*******************************************************************************

	S_PROLOG(mfdec_ppc)
	mfspr	r3, MF_DEC_PPC
	br
	FCNDES(mfdec_ppc)
#
#
# NAME: i_mfrr_set
#
# FUNCTION:
#       Set INTOFFLVL into the processor MFRR
#
# EXECUTION ENVIRONMENT:
#       Must be called at INTMAX
#
# ACCESSED:
#       through branch table entry i_soft
#
# INPUT:
#       UP: None
#	MP: r3 - priority to indicate, 0-31 (not checked)
#	  : r4 - processor number.  Also recognizes LOCAL_CPU.
#
# RETURNS:
#       none
#
# MP CONSIDERATIONS:
#	Passed 2 parameters, Priority and CPU to interrupt.
#	MFRR Interrupts are enqueued in PPDA.mfrr_pend.
#	The PPDA.mfrr_pend and physical MFRR write are serialized by
#	PPDA.mfrr_lock, which prevents processors from racing between
#	the writes.
#

	.set	MFRR_UNLOCKED,0

        S_PROLOG(i_mfrr_set)

ifdef(`_POWER_MP',`
	cmpi	cr0, r4, LOCAL_CPU	# Interrupting ourselves?
	liu     r6, 0x8000              # Priority OR mask
	bne	_index			#
	GET_PPDA(unused, r4)		# get ppda pointer
	b	_common			#
_index:					#
	rlwinm	r4, r4,  2, -1		# CpuID times sizeof(pointer)
	lwz	r4, ppda_p_tab(r4)	# PPDA = ppda_p_tab[CpuID]
_common:				#
	srw	r6, r6, r3		# Priority bit (passed)
					# Could set MFRR <= 0xFF here
	addi	r7, r4, ppda_mfrr_lock-ppdarea # to mfrr_lock
_retry:					#
	lwarx	r5,  0, r7		# mfrr_lock - query
	cmpi	cr1, r5, MFRR_UNLOCKED	# Is it unlocked?
	addi	r5,  0,  1		# create a LOCKED value
	bne	cr1, _retry		# Already locked, retry
	stwcx.	r5,  0, r7		# mfrr_lock - reserve
	bne	cr0, _retry		# Atomic modify failed
	isync				# Speculative Execution fence
					#
	lwz	r0, ppda_mfrr_pend(r4)	# mfrr_pend
	lwz	r8, ppda_intr(r4)	# Pointer, our HW intr. area
	or	r0, r0, r6		# Signal new priority
	stw	r0, ppda_mfrr_pend(r4)	# Update mfrr_pend
	cntlzw	r0, r0			# Highest pending priority
	stb	r0, mfrr(r8)		# Write physical MFRR
	addi	r5,  0,  0		# create the UNLOCKED value
	sync				# flush mfrr modifications
	stw	r5, 0(r7)		# mfrr_lock - clear
',`
ifdef(`INTRDEBUG',`
        GET_PPDA( cr0, r7 )             # get address of ppda
        mflr    r0                      # Save lr
        l       r7, ppda_intr(r7)       # get address of hardware registers
        lil     r3, 0x544d              # SETM - Set MFRR
        oriu    r3, r3, 0x5345
        l       r4, xirr_poll(r7)	# get xirr without side effects
        l       r5, mfrr(r7)		# get mfrr
        lil     r6, 0
        bl      ENTRY(mltrace)
        .extern ENTRY(mltrace)

        mtlr    r0
',)
	TRAP
        GET_PPDA( cr0, r4 )             # get address of ppda
        lil     r3, INTOFFLVL           # Load up the INTOFFLVL value
        l       r5, ppda_intr(r4)       # get address of hardware registers
        stb     r3, mfrr(r5)            # store MFRR
')
        br                              # return
        FCNDES(i_mfrr_set)

#
#
# NAME: i_mfrr_reset
#
# FUNCTION:
#       Set the MFRR to empty and clear any MFRR interrupt by
#       raising the CPPR value causing the any pending interrupt
#       to be rejected.
#
# EXECUTION ENVIRONMENT:
#       Must be called at INTMAX
#
# ACCESSED:
#       through branch table entry i_soft_reset
#
# INPUT:
#       UP: none
#	MP: r3 - priority to clear, 0-31 (not checked)
#
# RETURNS:
#       none
#
# MP CONSIDERATIONS:
#	Passed 1 parameter, Priority to clear
#	MFRR Interrupts are dequeued from PPDA.mfrr_pend
#

        S_PROLOG(i_mfrr_reset)

ifdef(`_POWER_MP',`
	GET_PPDA(unused, r4)		# get ppda pointer
	lwz	r8, ppda_intr(r4)	# Pointer, our HW intr. area
					# Could set MFRR <= 0xFF here
	liu     r6, 0x8000              # Priority OR mask
	srw	r6, r6, r3		# Priority bit
	addi	r7, r4, ppda_mfrr_lock-ppdarea # to mfrr_lock
_again:					#
	lwarx	r5,  0, r7		# mfrr_lock - query
	cmpi	cr1, r5, MFRR_UNLOCKED	# Is it unlocked?
	addi	r5,  0,  1		# create a LOCKED value
	bne	cr1, _again		# Already locked, retry
	stwcx.	r5,  0, r7		# mfrr_lock - reserve
	bne	cr0, _again		# Atomic modify failed
	isync				# Speculative Execution fence
					#
	lwz	r0, ppda_mfrr_pend(r4)	# mfrr_pend
	andc	r0, r0, r6		# Clear indicated priority
	cntlzw.	r6, r0			# Highest pending priority
	stw	r0, ppda_mfrr_pend(r4)	# Update mfrr_pend
	addi	r5,  0,  0		# create the UNLOCKED value
	bne	cr0, _setmfrr		# No pri. bits left (will bner below)
	addi	r6, 0, EMPTY_MFRR	# Clear MFRR
_setmfrr:				#
	stb	r6, mfrr(r8)		# Write physical MFRR
        sync                            # push out the MFRR register write
	stw	r5, 0(r7)		# mfrr_lock - clear
        stb     r5, cppr(r8)            # set CPPR to 0 causing rejection
	eieio
        lbz     r3, cppr(r8)            # load back CPPR to ensure rejection
        xor.    r4, r3, r3              # data dependency for load
	beqr                            # return
',`
ifdef(`INTRDEBUG',`
        GET_PPDA( cr0, r7 )
        mflr    r0                      # save lr
        l       r7, ppda_intr(r7)       # get address of hardware registers
        lil     r3, 0x524d              # SETM - Set MFRR
        oriu    r3, r3, 0x434c
        l       r4, xirr_poll(r7)	# get xirr without side effects
        l       r5, mfrr(r7)		# get mfrr
        lil     r6, 0
        bl      ENTRY(mltrace)

        mtlr    r0                      # Restore lr
',)
	TRAP
	GET_PPDA( cr0, r4 )
        lil     r3, EMPTY_MFRR          # MFRR empty value
        l       r5, ppda_intr(r4)       # get address of hardware registers
        lil     r6, INTMAX              # get a 0
        stb     r3, mfrr(r5)            # store MFRR
        sync    			# push it out
        stb     r6, cppr(r5)            # set CPPR to 0 causing rejection
	eieio
        lbz     r3, cppr(r5)            # load back CPPR to ensure rejection
        xor.    r4, r3, r3              # data dependency for load
	beqr                            # return
')

        FCNDES(i_mfrr_reset)

#*******************************************************************************
#
# NAME:	isync_601
#
# FUNCTION: isync_601()
#
# EXECUTION ENVIRONMENT:
#	Called from unlock primitives
#
# RETURNS:
#	none
#
#*******************************************************************************

	S_PROLOG(isync_601)
	isync
	br
	FCNDES(isync_601)

#*******************************************************************************
#
# NAME: get_processor_id
#
# FUNCTION: read Processor Identification register
#
# EXECUTION ENVIRONMENT:
#           Called from locking primitives
#
# RETURNS: Processor Identification value
#
#*******************************************************************************

	S_PROLOG(get_processor_id)
	mfspr	r3, PID
	br
	FCNDES(get_processor_id)

ifdef(`_AIX_325',`
#*******************************************************************************
#
# NAME: __iospace_sync
#
# FUNCTION: provides Power PC sync instruction.  This will be
#	provided by the compiler in a future release
#
# RETURNS: None
#
#*******************************************************************************

	S_PROLOG(__iospace_sync)
	sync
	br

#*******************************************************************************
#
# NAME: __iospace_eieio
#
# FUNCTION: provides Power PC eieio instruction.  This will be
#	provided by the compiler in a future release
#
# RETURNS: None
#
#*******************************************************************************

	S_PROLOG(__iospace_eieio)
	eieio
	br
')

#*******************************************************************************
#
# NAME:
#    get_from_list_ppc
#
# FUNCTION:
#    This function provides an atomic means to remove the first
#    element from a singly linked list.
#
# INPUTS:
#    r3 -- address of list head (anchor)
#    r4 -- offset of the "next" pointer within element.
#
# RETURNS:
#    The address of the element removed.
#
# NOTES:
#    -This function is designed as a lightweight means of removing an
#     element from the head of a singly-linked list.  The head (or anchor)
#     of this list is presumed to be allocated (i.e. _not_ simply a
#     pointer).  When the next field of the head is nil, then the
#     list is empty.  The list is terminated by a nil next pointer.
#
#    -The strategy for accomplishing the list removal involves setting up a
#     reservation (via the lwarx instruction) on the next pointer found in
#     the head of the list.  Upon removal of the element from the list, the
#     next pointer from the removed element is written to the reserved word
#     (via a stwcx).  If the store fails, then another thread is in the
#     process of removing an element from the list, so we loop around to try
#     again.
#
#
#    N.B.  This function depends of the low bytes of memory being zero.
#          Currently, the low 256 bytes of memory are zeroed; thus, the
#          offset passed in via r4 must be less than 256.
#
#*******************************************************************************
	S_PROLOG(get_from_list_ppc)
retry_get:
	lwarx	r5,r4,r3	# load and reserve addr of head->next
	lwzx	r6,r4,r5	# get head->next->next
	stwcx.  r6,r4,r3	# store new head of list
	bne-	retry_get	# try again if reservation gone
	mr	r3,r5		# return old value of anchor
ifdef(`_POWER_MP',`
	isync	    	    	# MP ONLY
')
	blr			# return
	FCNDES(get_from_list_ppc)

#*******************************************************************************
#
# NAME:
#    put_onto_list_ppc
#
# FUNCTION:o
#    This function provides an atomic means to add an element to the
#    head of a singly linked list.
#
# INPUTS:
#    r3 -- address of list head (anchor).
#    r4 -- address of new head of list.
#    r5 -- offset of the "next" pointer within element.
#
# RETURNS:
#    None.
#
# NOTES:
#    -This function is designed as a lightweight means of adding an
#     element to the head of a singly-linked list.  The head (or anchor)
#     of this list is presumed to be allocated (i.e. _not_ simply a
#     pointer).  When the next pointer in the head is nil, then the
#     list is empty.  The list is terminated by a nil next pointer.
#
#    -The strategy for accomplishing the list insertion involves setting up a
#     reservation (via the lwarx instruction) on the next pointer found in
#     the head of the list.  If, upon storing the new element pointer into the
#     reserved word, the store fails, then another thread is in the
#     process of adding an element to the list, so we loop around to try
#     again.
#
#
#    N.B.  This function depends of the low bytes of memory being zero.
#          Currently, the low 256 bytes of memory are zeroed; thus, the
#          offset passed in via r5 must be less than 256.
#
#*******************************************************************************
	S_PROLOG(put_onto_list_ppc)
retry_put:
	lwarx	r6,r5,r3	# load and reserve head->next
	stwx	r6,r5,r4	# chain it to the new list head

ifdef(`_POWER_MP',`
	sync	    	    	# MP ONLY
')
	stwcx.	r4,r5,r3	# set the list anchor
	bne- 	retry_put	# branch if lost reservation
	blr			# return
	FCNDES(put_onto_list_ppc)

#*******************************************************************************
#  NAME: xmemccpy_ppc
#
#  FUNCTION:	Copy source to target, stopping if character c is copied.
# 		Copy no more than n bytes.
#
#  RETURN VALUE DESCRIPTION: returns 0 unless an exception
#		occurs. sets the target pointer to the character
#		following c in the copy or to NULL if c is not found
#		in the first n bytes.
#
#  xmemccpy(void **target, void *source, int c, size_t n)
#  {
#	char *t = *target, *s = source;
#	while (n-- > 0)
#		if ((*t++ = *s++) == c)
#		{
#			*target = t;
#			return (0);
#		}
#	*target = NULL;
#	return (0);
#  }
#
#	r3 = target, r4 = source, r5 = c , r6 = n
#
#       uses r0, r3-r12
#*******************************************************************************

        S_PROLOG(xmemccpy_ppc)

	mflr	r0		# get return address
	GET_CSA(cr1, r8, r7)	# address of current mst
        st	r0,excbranch(r7)    # On exception, branch to this location with
				# r3 set to exception value.
	l	r12, 0(r3)	# r12 =  pointer to target

	cmpi	cr7,r6,0	# check for zero-byte copy
	rlinm.	r0, r4, 0, 3	# check for word boundary
	cmpi	cr1, r0, 2	# is string address 2 mod 4?
	cmpi	cr6, r0, 3	# is string address 3 mod 4?
	mtctr	r6		# move byte count to count register
	beq	cr7, all_done	# return if NUMBER is zero
	ai	r4, r4, -1	# backup to use load with update
	ai	r12, r12, -1	# backup to use store with update
	beq	cr0, word_align	# string is aligned (0 mod 4)
	beq	cr1, res2	# load up next 2 bytes
	beq	cr6, res3	# load up next byte

res1:				# string address is 1 mod 4
	lbzu	r9, 1(r4)	# load byte1
	cmpl	cr0, r9, r5	# test if byte == char
	stbu	r9, 1(r12)	# store byte
	beq	cr0, found_char # branch if char was found
	bdz	all_done	# decrement counter and bail if zero

res2:				# string address is 2 mod 4
	lbzu	r10, 1(r4)	# load byte2
	cmpl	cr0, r10, r5	# test if byte == char
	stbu	r10, 1(r12)	# store byte
	beq	cr0, found_char # branch if char was found
	bdz	all_done	# decrement counter and bail if zero

res3:				# string address is 3 mod 4
	lbzu	r11, 1(r4)	# load byte3
	cmpl	cr0, r11, r5	# test if byte == char
	stbu	r11, 1(r12)	# store byte
	beq	cr0, found_char # branch if char was found
	bdz	all_done	# decrement counter and bail if zero

word_align:
	lbzu	r8, 1(r4)	# load byte 1
	lbzu	r9, 1(r4)	# load byte 2
	lbzu	r10, 1(r4)	# load byte 3
	lbzu	r11, 1(r4)	# load byte 4
	cmpl	cr0, r8, r5	# test if byte1 == char
	cmpl	cr1, r9, r5	# test if byte2 == char
	cmpl	cr6, r10, r5	# test if byte3== char
	cmpl	cr7, r11, r5	# test if byte4== char

no_char:
	stbu	r8, 1(r12)	# store byte 0
	beq	cr0, found_char	# branch if char found
	bdz	all_done	# decrement counter and bail out if zero

	stbu	r9, 1(r12)	# store byte 1
	beq	cr1, found_char	# branch if char found
	bdz	all_done	# decrement counter and bail out if zero

	stbu	r10, 1(r12)	# store byte 2
	beq	cr6, found_char	# branch if char found
	bdz	all_done	# decrement counter and bail out if zero

	stbu	r11, 1(r12)	# store byte 3
	beq	cr7, found_char	# branch if char found
	bdz	all_done	# decrement counter and bail out if zero

	lbzu	r8, 1(r4)	# load byte 1
	lbzu	r9, 1(r4)	# load byte 2
	lbzu	r10, 1(r4)	# load byte 3
	lbzu	r11, 1(r4)	# load byte 4
	cmpl	cr0, r8, r5	# test if byte1 == char
	cmpl	cr1, r9, r5	# test if byte2 == char
	cmpl	cr6, r10, r5	# test if byte3== char
	cmpl	cr7, r11, r5	# test if byte4== char
	b	no_char

found_char:
	ai	r12, r12, 1	# char found; rc = address of next char
	st	r12, 0(r3)	# store point to last char

xmreturn:
	GET_CSA(cr0, r0, r7)	# address of current mst
	cal	r3,0(r0)	# set return value to zero
        st	r3,excbranch(r7)  # clear exception branch
	br

all_done:
	cal	r4, 0(r0)	# r4 = 0
	st	r4, 0(r3)	# set return value to zero
	b	xmreturn	# clear exception branch and return

        FCNDES(xmemccpy_ppc)        # Function Descriptors

#*******************************************************************************
#
# NAME: mttb
#
# FUNCTION: write Power PC Time Base
#
# EXECUTION ENVIRONMENT:
#
# INPUT: 	value in ticks:
#			r3 = high order 32 bits
#			r4 = low order 32 bits
#
# RETURNS: 	nothing
#
# uses:	r0
#
#*******************************************************************************

	S_PROLOG(mttb)
	li	r0, 0
	mttbl	r0	# force TBL to 0
	mttbu	r3	# set TBU
	mttbl	r4	# set TBL
	br
	FCNDES(mttb)

#*******************************************************************************
#
# NAME: mftb
#
# FUNCTION: read Power PC Time Base
#
# EXECUTION ENVIRONMENT:
#
# INPUT: 	nothing
# RETURNS: 	value in ticks:
#			r3 = high order 32 bits
#			r4 = low order 32 bits
#
# decrementer value
#
# uses:	r0
#
#*******************************************************************************

	S_PROLOG(mftb)
loop:
	mftbu	r3		# load TB upper
	mftb	r4		# load TB lower
	mftbu	r5		# load TB upper
	cmpw	r5, r3		# old = new ?
	bne	loop
	br
	FCNDES(mftb)


ifdef(`_POWER_MP',`

# The following functions are overlays and can be inlined, therefore their
# size is kept in overlay.m4 and overlay.h, and their register usage is kept
# in ppda.h

#*******************************************************************************
#
# NAME: my_ppda
#
# FUNCTION: gets ppda
#
# RETURNS: ppda address
#
#*******************************************************************************

	.csect	my_ppda_ppc_overlay[RO]
DATA(my_ppda_ppc):
	.globl	DATA(my_ppda_ppc)

	mfspr	r3, SPRG0
	br


#*******************************************************************************
#
# NAME: my_csa
#
# FUNCTION: gets csa
#
# RETURNS: current mst address
#
#*******************************************************************************

	.csect	my_csa_ppc_overlay[RO]
DATA(my_csa_ppc):
	.globl	DATA(my_csa_ppc)

	mfspr	r3, SPRG3
	br


#*******************************************************************************
#
# NAME: get_curthread
#
# FUNCTION: gets curthread
#
# RETURNS: current thread structure address
#
#*******************************************************************************

	.csect	get_curthread_ppc_overlay[RO]
DATA(get_curthread_ppc):
	.globl	DATA(get_curthread_ppc)

	mfspr	r3, SPRG2
	br


#*******************************************************************************
#
# NAME: set_csa
#
# FUNCTION: sets csa
#
# INPUTS: mst address to make current
#
# NOTES: Caller must ensure that pre-emption is disabled
#
#*******************************************************************************

	.csect	set_csa_ppc_overlay[RO]
DATA(set_csa_ppc):
	.globl	DATA(set_csa_ppc)

	mfspr	r4, SPRG0
	mtspr	SPRG3, r3
	st	r3, ppda_csa(r4)
	l	r4, ppda_mstack(r4)
	st	r3, mstprev(r4)
	br


#*******************************************************************************
#
# NAME: set_curthread
#
# FUNCTION: sets curthread
#
# INPUTS: thread structure address to make current
#
# NOTES: Caller must ensure that pre-emption is disabled
#
#*******************************************************************************

	.csect	set_curthread_ppc_overlay[RO]
DATA(set_curthread_ppc):
	.globl	DATA(set_curthread_ppc)

	mfspr	r4, SPRG0
	mtspr	SPRG2, r3
	st	r3, ppda_curthread(r4)
	br
')

#*******************************************************************************
#
# NAME: i_8259mask,	(void)i_8259mask( volatile char  *iop )
#
# FUNCTION: Construct and write the Primary/Secondary 8259 Interrupt Masks
#
# EXECUTION ENVIRONMENT:
#	Called from ios/POWER/intr_rspc.c in various places.
#	The kernel TOC must already be set.
#	This routine is not performance critical.
#	Called at INTMAX - cannot page fault
#
# PARAMETERS:
#	r3 = Base Address of IO space
#
# RETURNS: void
#
# REGISTER USAGE:
#	r0  : Slave controller mask
#	r3  : passed, IO Base Address
#	r4  : Address, rspc_8259_enables (the word we modify atomically)
#	r5  : Address, rspc_8259_mask_sys
#	r6  : Address, rspc_8259_mask
#	r7  : Value to write, rspc_8259_enables
#	r8  : Value read, rspc_8259_mask_sys
#	r9  : Value read, rspc_8259_mask
#	cr0 : stwcx success/fail
#	cr1 : old rspc_8259_enables equals the new value?  (Early out)
#
# NOTES:
#	The variables (rspc_8259_mask, rspc_8259_mask_sys) contain bits for
#	sources that are disabled.  These are ORed together, accumulating
#	'disabled' bits for writing to the physical controllers.  Both
#	(rspc_8259_mask, rspc_8259_mask_sys) must have a 0 bit for a given
#	source to be physically enabled.
#*******************************************************************************

	S_PROLOG(i_8259mask)
	LTOC(r6,rspc_8259_mask,data)		#
	LTOC(r5,rspc_8259_mask_sys,data)	#
	LTOC(r4,rspc_8259_enables,data)		#
retry:						#
	lhz	 r9, 0(r6)			# r9  = rspc_8259_mask
	lhz	 r8, 0(r5)			# r8  = rspc_8259_mask_sys
	lwarx	 r0,  0, r4			# Reserve rspc_8259_enables
	or	 r7, r8, r9			# Create the disabled mask
	cmp	cr1, r0, r7			# New, Old values the same?
	stwcx.	 r7,  0, r4			# Store the mask
	bne+	cr0, retry			# Unlikely failed reservation
	beqlr-	cr1				# After stwcx to clear reserv.
	rlwinm	 r0, r7, 24, 0xFF		# Isolate slave mask
	stb	 r0, INTB01(r3)			# Slave 8259 enables
	eieio					#
	stb	 r7, INTA01(r3)			# Master 8259 enables
	sync					#
	br					#
	FCNDES(i_8259mask)

#*******************************************************************************
#
# NAME: i_8259elcr,	(void)i_8259elcr( volatile char  *iop )
#
# FUNCTION:  Write the 16 bit ELCR value into the ELCR registers
#
# EXECUTION ENVIRONMENT:
#	Called from ios/POWER/intr_rspc.c in various places.
#	The kernel TOC must already be set.
#	This routine is not performance critical.
#	Called at INTMAX - cannot page fault
#
# PARAMETERS:
#	r3 = Base Address of IO space
#
# RETURNS: void
#
# REGISTER USAGE:
#	r0  : Slave controller mask
#	r3  : passed, IO Base Address
#	r4  : Address then value, rspc_8259_elcr
#
# NOTES:
#	ELCR registers are initialized to be all edge triggered.  This
#	function will write a new value to both ELCR registers.
#	A 0 = edge triggered mode.  A 1 = level triggered mode.
#*******************************************************************************

	S_PROLOG(i_8259elcr)
	LTOC(r4,rspc_8259_elcr,data)		#
	lhz	 r4, 0(r4)			# r4  = rspc_8259_elcr
	stb	 r4, ELCR0(r3)			# Master 8259 ELCR
	rlwinm	 r0, r4, 24, 0xFF		# Isolate slave elcr
	stb	 r0, ELCR1(r3)			# Slave 8259 ELCR
	eieio					#
	br					#
	FCNDES(i_8259elcr)

        .toc
	TOCE(rspc_8259_enables, data)
        TOCE(rspc_8259_mask, data)
        TOCE(rspc_8259_mask_sys, data)
	TOCE(rspc_8259_elcr, data)

#********************************************************************
#
#  FUNCTION: exbcopy_ppc
#
#
#************************************************************************
#
#  NAME: exbcopy_ppc  move (overlapped NOT ok) and check for errors
#		  and don't perform load from source across page boundary
#
#  FUNCTION: Equal length character string move
#
#  EXECUTION ENVIRONMENT:
#  Must be in the kernel with write access to the current mst
#  Standard register usage and linkage convention.
#  Registers used r0,r3-r12
#  Condition registers used: 0,1
#  No stack requirements.
#
#  NOTES:
#  The source string represented by R3 is moved to the target string
#  area represented by R4 and R5.
#
#  The strings may be on any address boundary and may be of any length from 0
#  through (2**31)-1 inclusive.  The strings may NOT overlap.
#
#  The addresses are treated as unsigned quantities,
#  i.e., max is 2**32-1.
#
#  The lengths are treated as unsigned quantities,
#  i.e., max is 2**31-1.
#
#  Use of this logic assumes string lengths <= 2**31-1 - Signed arithmetic
#
#  RETURN VALUE DESCRIPTION: Target string modified.
#			     returns 0 unless exception occurs
#			     in which case exception value is returned.
#
#       exbcopy_ppc( char * source, char * target, int length)
#
#  Calling sequence: exbcopy_ppc 
#       R3   Address of source string
#       R4   Address of target string
#       R5   Length of target and source string
#
#
	  .csect ENTRY(exbcopy_ppc[PR]),5
	  .globl ENTRY(exbcopy_ppc[PR])
.exbcopy_ppc:
	
	mflr   r7			# Get return address
	GET_CSA(cr1, r8, r11)		# address of current mst
	st	r7,excbranch(r11) 	# On exception, branch to
					# return address 

	lil	r6, 0			# initialize count
	cmpi	cr5, r5, 12		# check for small move
	andil.	r9, r4, 0x3		# check alignment of target
	blt-	cr5, ismall		# go handle small move
	beq+	cr0, inofixup		# branch if target aligned
	sfi	r9, r9, 0x4		# number bytes to align target	
	mtctr	r9			# use counter for loop
ifixup:
	lbzx	r7, r6, r3		# load byte
	stbx	r7, r6, r4		# and store it
	addi	r6, r6, 1		# update count
	bdnz+	ifixup			# until done
inofixup:
	subfc	r7, r6, r5		# number bytes left to move
	andil.	r9, r7, 0x7		# number of scraps
	srwi	r10, r7, 0x3		# number of chunks to move
	mtctr	r10			# use counter for loop
	addi	r10, r6, 4		# psuedo counter for unrolled loop
	b	iloop			# skip no-ops
	.align	5
iloop:
	lwzx	r7, r6, r3		# load word
	lwzx	r8, r10, r3		# load word
	stwx	r7, r6, r4		# store word
	stwx	r8, r10, r4		# store word
	addi	r6, r6, 8		# update (real) counter
	addi	r10, r10, 8		# update (psuedo) counter
	bdnz+	iloop			# until done
	beq+	cr0, exbcopy_done	# if no scraps
	mtctr	r9			# number of scraps
iscraps:
	lbzx	r7, r6, r3		# load byte
	stbx	r7, r6, r4		# store byte
	addi	r6, r6, 1		# update counter
	bdnz+	iscraps			# until done
	b	exbcopy_done		# go to cleanup code
	
ismall:
	mtctr	r5			# use counter for loop
	b	ismall_loop		# skip no-ops
	.align 5
ismall_loop:
	lbzx	r7, r6, r3		# load byte
	stbx	r7, r6, r4		# store byte
	addi	r6, r6, 1		# update count
	bdnz+	ismall_loop		# until done
exbcopy_done:
	cal	r3,0(r0)		# Zero return code for successful move
	st	r3,excbranch(r11)	# Clear exception handler
	br				# Return
	FCNDES(exbcopy_ppc)		# Function Descriptors

include(scrs.m4)
include(low_dsect.m4)
include(i_machine.m4)
include(interrupt.m4)
include(mstsave.m4)
include(systemcfg.m4)
include(machine.m4)
include(system_rspc.m4)
