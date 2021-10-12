# @(#)31	1.38.1.16  src/bos/kernel/ml/POWER/vmrte.s, sysml, bos41J, 9522A_all 5/30/95 18:12:55
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: ldsr ldfsr ldtid ldftid tlbi ldsdr1 clz32
#	dclst hyperbranch xlateon xlateoff ml_vm_seth
#        vm_att ml_as_att vm_det ml_as_det mtsr
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1994
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

	.file	"vmrte.s"
	.machine "com"
        .using  low, 0

include(systemcfg.m4)
include(macros.m4)

#*****************************************************************************
#
# NAME: block_zero
#
# FUNCTION:
#
#	void	block_zero(
#	void *addr,		/* address to zero */
#	uint  size)		/* number of bytes to zero */
#	
#	zero a block of memory using cache line/block zero
#
# NOTES:
#	address and size must be cache aligned
#       LINES MARKED 603e ARE FOR 603e ERRATA 16 WORKAROUND
#
# RETURNS: None
#
#******************************************************************************

	S_PROLOG(block_zero)
	lwz	r6, syscfg_arch(0)		# get processor type

#	get cache line/block.  Note for Power RS block and line are
#	equal in size

	lwz	r5, syscfg_dcb(0)		
	cmpi	cr7, r6, POWER_PC		# check for POWER PC

#	check that range to be zeroed starts and ends on a cache line
#	boundary

ifdef(`DEBUG',`

	# assert((addr & (block_size-1)) == 0)

	addi	r12, r5, -1			# block_size - 1
	and	r11, r3, r12			# (addr & (block_size-1))
	tnei	r11, 0

	# assert((addr+size) & (block_size-1))
	
	add	r11, r3, r4			# addr+size
	and	r11, r11, r12			# (addr+size) & (block_size-1)
	tnei	r11, 0
')

	# set loop counter = size/block_size = (size) >> (31 - clz(block_size))

	# If a 603e 1.4 and below then do the that special thing.
	# 603s and 603e 1.5 and beyond do the normal ppc thing.
	# This decision is made in hardinit() and if not a 1.4 the branch
	# below will be a nop.
DATA(blkz_603_patch):
	.globl	DATA(blkz_603_patch)
	b       _bz_603e                        # 603e branch if special 603

	cntlzw	r6, r5				# clz(block_size)
	subfc	r3, r5, r3			# subtract for first block zero
	sfi	r7, r6, 31			# 31 - clz(block_size)
	srw	r8, r4, r7			# size >> (31 - clz(block_size)
	mtctr	r8				# set loop counter

	bne-	cr7, bloop_pwr			# branch for Power RS

bloop_ppc:
	.machine "ppc"
	dcbz	r3, r5				# zero cache block
	.machine "com"
	a	r3, r3, r5			# advance for next zero
	bdnz+	bloop_ppc		
	blr

bloop_pwr:
	.machine "pwr"
	dclz	r3, r5				# zero cache line
	.machine "com"
	bdnz+	bloop_pwr			# do next line
	blr

_bz_603e:					# 603e zero 32 byte chunks
	li	r0, 32				# 603e 32 bytes
	srwi	r4, r4, 5			# 603e number of chunks to zero
	mtctr	r4				# 603e set loop counter
	li	r4, 0				# 603e 1.4 load up some zeros
	li	r5, 0				# 603e
	li	r6, 0				# 603e
	li	r7, 0				# 603e
	li	r8, 0				# 603e
	li	r9, 0				# 603e
	li	r10, 0				# 603e
	li	r11, 0				# 603e

bloop_603e:
	stswi	r4, r3, 0			# 603e push out the zeros
	a	r3, r3, r0			# 603e bump address by chunk
	bdnz+   bloop_603e                      # 603e repeat if not all done
	blr                                     # 603e return to caller


#**********************************************************************
#
#  NAME: ldtid
#
#  FUNCTION: load transaction id register
#
#       ldtid(tidval)           rc = none
#
#  INPUT STATE:
#     r3 = transaction id value         (16 bits)
#
#  OUTPUT STATE:
#     The transaction id register is loaded with the input value.
#
#  EXECUTION ENVIRONMENT:
#       Supervisor state  : Yes
#
#**********************************************************************

        S_PROLOG(ldtid)
        mtspr   TID,r3          # move (r3) to spr=tid
	S_EPILOG

#**********************************************************************
#
#  NAME: ldftid
#
#  FUNCTION: load from transaction id register
#
#       ldftid()                rc = tid value
#
#  INPUT STATE:
#
#  OUTPUT STATE:
#     r3 = contents of the transaction id register
#
#  EXECUTION ENVIRONMENT:
#       Supervisor state  : Yes
#
#**********************************************************************

        S_PROLOG(ldftid)
        mfspr   r3,TID          # move from spr=tid to r3
	S_EPILOG

#**********************************************************************
#
#  NAME: ldsdr1
#
#  FUNCTION: load sdr1
#
#       ldsdr1(value)           rc = none
#
#  INPUT STATE:
#     r3 = value
#
#  OUTPUT STATE:
#       sdr1 contains the new value.
#
#  EXECUTION ENVIRONMENT:
#       Supervisor state  : Yes
#
#**********************************************************************

        S_PROLOG(ldsdr1)
        mtspr   SDR1,r3         # load sdr1 with (r3)
	br
	S_EPILOG

#**********************************************************************
#
#  NAME: clz32
#
#  FUNCTION: count leading zeroes for 32 bit word
#
#       clz32(value)            rc = number of leading zeroes
#
#  INPUT STATE:
#       r3 = value
#
#  OUTPUT STATE:
#       r3 = count of leading zeroes
#
#**********************************************************************

        S_PROLOG(clz32)
        cntlz   r3,r3           # count leading zeroes
	S_EPILOG

#**********************************************************************
#
#  NAME: hyperbranch
#
#  FUNCTION: branch to address without changing link register
#
#       hyperbranch(eaddr)      rc = none
#
#  INPUT STATE:
#       r3 = eaddr              effective address
#
#  OUTPUT STATE:
#       control branches to (r3), using the count register.  the link
#       register remains unchanged.
#
#**********************************************************************

        S_PROLOG(hyperbranch)
        mtspr   CTR,r3          # move eaddr = (r3) to count register
        bctr                    # branch to count register
	S_EPILOG

#**********************************************************************
#
#  NAME: xlateon
#
#  FUNCTION: turn on virtual address translation
#
#       xlateon()               rc = none
#
#  INPUT STATE:
#
#  OUTPUT STATE:
#       address translation has been turned on for instructions and data
#
#  EXECUTION ENVIRONMENT:
#       Supervisor state  : Yes
#
#  NOTE:
#       this routine and its caller must be in V=R memory, so that
#       instructions after the mtmsr instruction are addressable in
#       either mode.
#
#**********************************************************************

        S_PROLOG(xlateon)
        mfmsr   r3              # get current msr flags
        oril    r3, r3, MSR_IR | MSR_DR # or in the IR, DR flags
        mtmsr   r3              # set it in the msr
	isync
	S_EPILOG

#**********************************************************************
#
#  NAME: xlateoff
#
#  FUNCTION: turn off virtual address translation
#
#       xlateoff()              rc = none
#
#  INPUT STATE:
#
#  OUTPUT STATE:
#       address translation has been turned off for instructions and data
#
#  EXECUTION ENVIRONMENT:
#       Supervisor state  : Yes
#
#  NOTE:
#       this routine and its caller must be in V=R memory, so that
#       instructions after the mtmsr instruction are addressable in
#       either mode.
#
#**********************************************************************

        S_PROLOG(xlateoff)
        mfmsr   r3              # get current msr flags
        rlinm   r3, r3, 0, ~(MSR_IR | MSR_DR) # clear IR and DR flags
        mtmsr   r3              # set it in the msr
	isync
	S_EPILOG

#**********************************************************************
#
#  NAME: vm_seth
#
#  FUNCTION:  Put a "handle" into a segment register, given a handle
#             and a 32-bit virtual address.  The high order 4 bits of
#             the 32-bit VA specify which segment register is to be loaded.
#
#       vm_seth(vmhandle_t,caddr_t)
#
#  INPUT STATE:
#     r3 = handle (new contents for segment register)
#     r4 = 32-bit virtual address
#
#  RETURNED VALUE:
#     none
#
#  EXECUTION ENVIRONMENT:
#       Supervisor state  : Yes
#
#
#**********************************************************************

        S_PROLOG(vm_seth)
	rlinm.	r12, r3, 0, SR_KsKp
	isync

ifdef(`PWRSBIT',`',`
	beq	cr0, vm_set_keyclear
	l	r10, DATA(key_value)
	rlinm	r3, r3, 0, ~SR_KsKp
	or	r3, r3, r10
vm_set_keyclear:
')
	
        mtsri   r3, 0, r4               # Set SR contents, and return
	isync
	S_EPILOG

#**********************************************************************
#
#  NAME: vm_att
#
#  FUNCTION:  convert (srval,offset) to 32-bit address by allocating
#             a segment register, loading it with "srval", and returning
#             the 32-bit value that will selects the sr with "offset"
#             as the offset.
#
#             Never allocate sr's 0, 1, 2, or 14
#
#       caddr_t = vm_att(vmhandle_t,caddr_t)
#
#  INPUT STATE:
#     r3 = segment register value ("vmhandle_t" type)
#     r4 = offset within segment ("caddr_t" type, hi 4 bits IGNORED)
#
#  RETURNED VALUE:
#     32-bit value made up of 4-bit sr number || 28-bit offset
#
#  EXECUTION ENVIRONMENT:
#       Supervisor state  : Yes
#
#  NOTE:
#     This routine is also invoked via the following macros in
#     <sys/adspace.h>:
#         vmu_att, io_att
#
#**********************************************************************

        S_PROLOG(vm_att)

#       Find a free segment register, using the flag bits stored in
#       the current mstsave.  A '1' bit indicates an allocated register.
#       The mask is inverted, and the "count leading zeros" instruction is used
#       to find the number of leading '0' bits and thus the index of the first
#       '1' bit in the inverted mask, which is the number of the segment
#       register to use.

#       Segment registers 0, 1, 2, and 14 are never allocated by this
#       routine:
#               sr0   Always addresses the kernel segment
#               sr1   Always addresses the user text segment
#               sr2   Always addresses the user private segment
#               sr14  Always addresses the kernel extension segment
#       These bits are cleared when the mask is loaded to prevent selecting
#       any of these segment registers.

	rlinm.	r12, r3, 0, SR_KsKp	# check for key bit(s) set
	GET_CSA(cr1, r6, r5)		# Point to current mstsave area
       .using   mstsave, r5
        l       r6, mstsralloc          # Pick up current allocation state
        xoriu   r7, r6, 0xFFFF          # Invert mask so that 0's represent
                                        #   allocated registers

ifdef(`PWRSBIT',`',`
	beq	cr0, vm_att_key_clear	# branch if no key bits
	l	r10, DATA(key_value)	# load correct key bits
	rlinm	r3, r3, 0, ~SR_KsKp	# clear key bits
	or	r3, r3, r10		# put in correct key value
vm_att_key_clear:
')
	
        andiu.  r7, r7, 0x1FFD          # Strike out 0, 1, 2, and 14
        cntlz   r7, r7                  # Count leading zeros
        beq     vm_no_regs              # Panic if no regs left after "and"

#       Segment register number is in r7; show this register is allocated
#       by turning on its bit in the mask

	liu	r8, 0x8000
	sr	r8, r8, r7		# Create a '1' bit in selected position

        or      r6, r6, r8              # Turn on selected bit
        st      r6, mstsralloc          # Store updated allocation state
       .drop    r5

#       Form the 32-bit address, and load the sr value into the seg reg

        rlimi   r4, r7, 28, 0xF0000000  # Force seg reg number into high
                                        #   part of address
        mtsri   r3, 0, r4               # Load segment register
DATA(isync_vmatt):
	.globl	DATA(isync_vmatt)
	isync
        mr      r3, r4                  # Return address in reg 3
        br                              # Return to caller

#       Disaster:  Request to load more segment registers than exist.

vm_no_regs:
        mfspr   r0, LR                  # Save caller's return address
        bl      vm_call_panic_1         # Branch around,
					# developing addr of string
       .byte    'v,'m,'_,'a,'t,'t,':,' ,'s,'r,'e,'g,'s
       .byte    ' ,'e,'x,'h,'a,'u,'s,'t,'e,'d, 0
       .long                            # Pad to fullword boundary

vm_call_panic_1:
        mfspr   r3, LR                  # Point r3 to panic string
        mtspr   LR, r0                  # Put return address back
        ba      ENTRY(panic)            # Branch to "panic" routine, making
       .extern  ENTRY(panic)            #   it appear that our caller panicked.

        S_EPILOG
        FCNDES(vm_att)

#**********************************************************************
#
#  NAME: vm_det
#
#  FUNCTION:  Given a 32-bit virtual address, release the segment
#             register associated with that address.
#
#             Never release sr's 0, 1, 2, or 14.
#
#       vm_det(caddr_t)
#
#  INPUT STATE:
#     r3 = 32-bit virtual address (caddr_t)
#
#  RETURNED VALUE:
#     none
#
#  EXECUTION ENVIRONMENT:
#       Supervisor state  : Yes
#
#**********************************************************************

        S_PROLOG(vm_det)

	GET_CSA(cr0, r6, r7)		# Point to current mstsace area
       .using   mstsave, r7

#       Convert segment register number into a mask position.
#       Panic if number is any of 0, 1, 2, or 14.

        rlinm   r4, r3, 4, 0x0000000F   # Isolate segment register number
	liu	r5, 0x8000
	sr	r5, r5, r4		# Build a 1-bit mask from number


        andiu.  r0, r5, 0xE002          # Test for any reserved values
        l       r6, mstsralloc          # Pick up current allocation state
        bne     vm_bad_reg              # Panic if trying to free a reserved reg

#       Set bit off for newly-released segment register.

        and.    r0, r6, r5              # Make sure bit is currently on
        andc    r6, r6, r5              # Turn off bit, thus freeing register
        st      r6, mstsralloc          # Save updated mask
       .drop    r7

        beq     vm_bad_reg              # Branch if reg not allocated

#       Load the newly-freed segment register with "NULLSEGVAL"
#       to catch accidental uses of segment subsequent to its release.

        cau     r5, 0, u.NULLSEGVAL     # Upper half of literal
        oril    r5, r5, l.NULLSEGVAL    # Lower half of literal
DATA(isync_vmdet):
	.globl	DATA(isync_vmdet)
	isync
        mtsri   r5, 0, r3               # Put value into sr selected by r3
        br                              # Return to caller

#       Disaster:  Request to free a reserved segment register
#                  or one not in use.

vm_bad_reg:
        mfspr   r0, LR                  # Save caller's return address
        bl      vm_call_panic_2         # Branch around,
					# developing addr of string

       .byte    'v,'m,'_,'d,'e,'t,':,' ,'b,'a,'d,' # significant blank
       .byte    'a,'d,'d,'r,'e,'s,'s, 0
       .long                            # Pad to fullword boundary

vm_call_panic_2:
        mfspr   r3, LR                  # Point r3 to panic string
        mtspr   LR, r0                  # Put return address back
        ba      ENTRY(panic)            # Branch to "panic" routine, making
       .extern  ENTRY(panic)            #   it appear that our caller panicked.

        S_EPILOG
        FCNDES(vm_det)

#**********************************************************************
#
#  NAME: ml_as_att
#
#  FUNCTION:  convert (srval,offset) to 32-bit address by allocating
#             a segment register entry in an address space structure,
#             loading it with "srval", and returning the 32-bit value
#             that will selects the sr with "offset" as the offset.
#	      If the adspace being attached to is the current adspace
#	      then load the segmenent register.
#
#             Never allocate sr's 0, 1, 2, or 14
#
#       caddr_t = ml_as_att(adspace_t *,vmhandle_t,caddr_t)
#
#  INPUT STATE:
#     r3 = ptr to address space structure ("adspace_t" type)
#     r4 = segment register value ("vmhandle_t" type)
#     r5 = offset within segment ("caddr_t" type, hi 4 bits IGNORED)
#
#  NOTES:
#     This function corrects key bit values for the platform it
#     executes on
#
#  RETURNED VALUE:
#     32-bit value made up of 4-bit sr number || 28-bit offset
#     NULL if not successful
#
#  EXECUTION ENVIRONMENT:
#       Supervisor state  : Yes
#
#  NOTE:
#     This routine is also invoked via the following macros in
#     <sys/adspace.h>:
#         vm_att, io_att
#
#**********************************************************************

        S_PROLOG(ml_as_att)

#       Find a free segment register, using the flag bits stored in
#       the current mstsave.  A '1' bit indicates an allocated register.
#       The mask is inverted, and the "count leading zeros" instruction is used
#       to find the number of leading '0' bits and thus the index of the first
#       '1' bit in the inverted mask, which is the number of the segment
#       register to use.

#       Segment registers 0, 1, 2, and 14 are never allocated by this
#       routine:
#               sr0   Always addresses the kernel segment
#               sr1   Always addresses the user text segment
#               sr2   Always addresses the user private segment
#               sr14  Always addresses the kernel extension segment
#       These bits are cleared when the mask is loaded to prevent selecting
#       any of these segment registers.

       .using   adspace_t, r3
	rlinm.	r9, r4, 0, SR_KsKp	# check for key bit(s) set
        l       r6, adsp_alloc          # Pick up current allocation state
        xoriu   r7, r6, 0xFFFF          # Invert mask so that 0's represent
                                        #   allocated registers

	beq	cr0, as_att_key_clear	# branch if key bits are clear
	l	r10, DATA(key_value)	# load correct key value
	rlinm	r4, r4, 0, ~SR_KsKp	# force key bit(s) to zero
	or	r4, r4, r10		# or in correct key bit
as_att_key_clear:

        andiu.  r7, r7, 0x1FFD          # Strike out 0, 1, 2, and 14
	GET_CSA(cr1, r0, r9)		# get the current save area
        cntlz   r7, r7                  # Count leading zeros
        beq     cr0, as_no_regs         # Fail if no regs left after "and"

#       Segment register number is in r7; show this register is allocated
#       by turning on its bit in the mask

	cal	r9, mstsralloc(r9)	# calculate the current adspace
	liu	r8, 0x8000
	sr	r8, r8, r7		# Create a '1' bit in selected position

        or      r6, r6, r8              # Turn on selected bit
	cmp	cr0, r9, r3		# Compare current adspace with adspace
					#   being attached to
        st      r6, adsp_alloc          # Store updated allocation state

#       Form the 32-bit address, and store the sr value into the adspace entry

        rlinm   r7, r7, 2, 0x0000003C   # Multiply SR number by 4
        cal     r8, adsp_srval          # Point to start of array of SR's
        stx     r4, r7, r8              # Store value into proper entry
       .drop    r3

        rlimi   r5, r7, 26, 0xF0000000  # Force seg reg number into high
                                        #   part of address
        mr      r3, r5                  # Return address in reg 3
	bner				# If the address space are not the
					#    same then don't load sreg
	mtsri	r4, 0, r5		# Load with handle
	isync
        br                              # Return to caller

# return error if there are no segment registers left

as_no_regs:
	lil	r3, NULL

        S_EPILOG
        FCNDES(ml_as_att)

#**********************************************************************
#
#  NAME: ml_as_det
#
#  FUNCTION:  Given an address space structure and a 32-bit virtual
#             address, release the segment register associated with
#             that address.
#	      Will invalidate the segment register if detaching
#	      from current segment register
#
#             Never release sr's 0, 1, 2, or 14.
#
#       ml_as_det(adspace_t *, caddr_t)
#
#  INPUT STATE:
#     r3 = ptr to address space structure (adspace_t)
#     r4 = 32-bit virtual address (caddr_t)
#
#  RETURNED VALUE:
#     0 = successful
#     EINVAL = detaching invalid address
#
#  EXECUTION ENVIRONMENT:
#       Supervisor state  : Yes
#
#**********************************************************************

        S_PROLOG(ml_as_det)

       .using   adspace_t, r3
        l       r6, adsp_alloc          # Load allocation mask

#       Convert segment register number into a mask position.
#       Panic if number is any of 0, 1, 2, or 14.

        rlinm   r8, r4, 4, 0x0000000F   # Isolate segment register number
	liu	r5, 0x8000
	sr	r5, r5, r8		# Build a 1-bit mask from number


        andiu.  r0, r5, 0xE002          # Test for any reserved values
	GET_CSA(cr1, r0, r7)		# get current save area
        bne     as_bad_reg              # Fail if trying to free a reserved
					#   reg

#       Set bit off for newly-released segment register.

        and.    r0, r6, r5              # Make sure bit is currently on
        andc    r6, r6, r5              # Turn off bit, thus freeing register
        st      r6, adsp_alloc          # Save updated mask
	cal	r7, mstsralloc(r7)	# Calculate the current save area
        beq     as_bad_reg              # Branch if reg not allocated

#       Load the newly-freed segment register entry with "NULLSEGVAL"
#       to catch accidental uses of segment subsequent to its release.

        cau     r5, 0, u.NULLSEGVAL     # Upper half of literal
	cmp	cr0, r3, r7		# @
        oril    r5, r5, l.NULLSEGVAL    # Lower half of literal
        rlinm   r8, r8, 2, 0x0000003C   # Multiply sr number by 4
        cal     r6, adsp_srval          # Point to array of sr entries
        stx     r5, r8, r6              # Store NULLSEGVAL into sr entry
	lil	r3, 0			# get return code
	bner				# Branch if not detaching from
					#   current adspace
	isync
	mtsri	r5, 0, r4		# Invalidate segment register
        br                              # Return to caller
       .drop    r3

#       Error:  Request to free a reserved segment register
#                  or one not in use.

as_bad_reg:
	lil	r3, EINVAL

        S_EPILOG

        FCNDES(ml_as_det)


#**********************************************************************
#
#  NAME: ml_as_seth
#
#  FUNCTION:  Put a "handle" into a adspace, given a handle
#             and a 32-bit virtual address.  The high order 4 bits of
#             the 32-bit VA specify which segment register is to be loaded.
#	      if the adspace is the calling process's current adspace
#	      then the handle will be loaded into a segment register
#
#       ml_as_seth(adspace_t,vmhandle_t,caddr_t)
#
#  NOTES:
#	Corrects key bit values for the platform it executes on
#
#  INPUT STATE:
#     r3 = pointer to user address space
#     r4 = handle (new contents for segment register)
#     r5 = 32-bit virtual address
#
#  RETURNED VALUE:
#     none
#
#  EXECUTION ENVIRONMENT:
#       Supervisor state  : Yes
#
#
#**********************************************************************

        S_PROLOG(ml_as_seth)

	rlinm.	r6, r4, 0, SR_KsKp	# check for key bit(s) set
	GET_CSA(cr7, r0, r6)		# get the current save area
	cal	r6, mstsralloc(r6)	# calculate the current adspace

	beq	cr0, as_set_keyclear	# branch if key bit(s) are note set
	l	r8, DATA(key_value)	# get key bit value
	rlinm	r4, r4, 0, ~SR_KsKp	# force key bits to zero
	or	r4, r4, r8		# or the correct key in
as_set_keyclear:

	cmp	cr1, r6, r3		# compare it with the adspace
					#   we are setting
	cal	r3, adsp_srval(r3)	# get address to start of segregs
	rlinm	r7, r5,	6, 0x0000003C	# multiply the sreg number by 4
	stx	r4, r7, r3		# new sereg value
	bner	cr1			# branch if not same adspace
	isync
        mtsri   r4, 0, r5		# Set SR contents, and return
	isync

        S_EPILOG
        FCNDES(ml_as_seth)

#**************************************************************************
#
#  NAME: getadsp
#
#  FUNCTION:  Return a pointer to the current process's address spave
#
#	adspace_t *getadsp()
#
#  RETURNED VALUE:
#	pointer ot adspace of current process
#
#  EXECUTION ENVIRONMENT:
#	Supervisor state  : Yes
#
#**************************************************************************

	S_PROLOG(getadsp)

	GET_CURTHREAD(cr0, r0, r4)
	l	r3, t_flags(r4)		# Get thread flags from thread table
	rlinm.	r3, r3, 0, TKTHREAD	# Check for the kthread bit
	l	r6, t_userp(r4)		# get user struct address
	cal	r3, u_adspace(r6)	# Set to user address space
	beqr				# if its not a kthread then return
	l	r6, t_uthreadp(r4)	# get uthread struct address
	cal	r3, mstsralloc(r6)	# Kthreads use the kernel
	
	S_EPILOG
	FCNDES(getadsp)

#**********************************************************************
#
#  NAME: v_maxstring
#
#  FUNCTION: returns the longest string of zeros in a double word
#	     at address eaddr
#
#       v_maxstring(eaddr)      rc = longest string of zeros
#
#  INPUT STATE:
#       r3 = eaddr              effective address
#
#  RETURN VALUE:
#       longest string of zeros
#
#**********************************************************************
	.set	ONES,0xffffffff

	S_PROLOG(v_maxstring)

	l	r4,0(r3)	# get first word
	cal	r0,0(r0)	# set r0 to 0
	nand	r5,r4,r4	# r5 = complement of first word.
	cal	r6,0(r0)	# r6 = cumulative shift
	mr	r7,r4		# r7 = shifted input
loop:	cntlz	r9,r7		# r9 = leading zeros in shifted input
	a	r6,r6,r9	# cumulate shift amount
	rlnm	r8,r5,r6,ONES	# r8 = shifted complement
	cntlz	r10,r8		# r10 = leading zeros in shifted complement
	cmp	cr1,r9,r0	# test for new maximum
	a	r6,r6,r10	# cumulate shift amount
	cmpi	cr0,r6,32	# test for done
	mr	r11,r9		# r11 = last string of zeros.
	ble	cr1,omax1	# branch if not new maximum
	mr	r0,r9		# r0 = new maximum
omax1:	oriu	r4,r4,0x8000	# set high bit to one to stop wrap-around 
	rlnm	r7,r4,r6,ONES	# r7 = shifted input
	blt	cr0,loop	# done if total shifts >= 32
#
# calculate trailing zeros in first word in r11
#
	andil.	r4,r4,0x1	# test if word ended in zeros
	l	r4,4(r3)	# get second word.
	beq	cr0,next	# branch if first word ended in zeros
	cal	r11,0(r0)	# set trailing zero count to 0
#
# add to r11 the leading zeros of the second word
#
next:	cntlz	r9,r4		# r9 = leading zeros of second word.
	a	r9,r9,r11	# add trailing zeros of first word
	cmp	cr1,r9,r0	# test for new maximum
	ble	cr1,omax2	# branch if not new maximum
	mr	r0,r9		# r0 = new maximum
#
# process second word
#
omax2:	mr	r7,r4		# r7 = shifted input
	cal	r6,0(r0)	# r6 = 0 = cumulative shift
	nand	r5,r4,r4	# r5 = complement of first word.
loop1:	cntlz	r9,r7		# r9 = leading zeros in shifted input
	a	r6,r6,r9	# cumulate shift amount
	rlnm	r8,r5,r6,ONES	# r8 = shifted complement
	cmp	cr1,r9,r0	# test for new maximum
	cntlz	r10,r8		# r10 = leading zeros in shifted complement
	a	r6,r6,r10	# cumulate shift amount
	cmpi	cr0,r6,32	# test for done
	ble	cr1,omax3	# branch if not new maximum
	mr	r0,r9		# r0 = new maximum
omax3:	oriu	r4,r4,0x8000	# set high bit to one to stop wrap-around 
	rlnm	r7,r4,r6,ONES	# r7 = shifted input
	blt	cr0,loop1	# done if total shifts >= 32
	mr	r3,r0		# move answer to r3

	S_EPILOG

	FCNDES(v_maxstring)

#**********************************************************************
#
#  NAME: v_findstring
#
#  FUNCTION:  returns leftmost bit position in double word at address
#	      eaddr where there a string of at least nbits zeros.
#
#       v_findstring(eaddr)      rc = longest string of zeros
#
#  INPUT STATE:
#       r3 = eaddr              effective address
#       r4 = nbits              number of contiguous zeros bit to be
#				searched for
#
#  RETURN VALUE:
#       leftmost bit position where the string was found.  a value of
#	64 means no such string was found
#
#**********************************************************************

	S_PROLOG(v_findstring)

	l	r5,0(r3)	# get first word
	mr	r11,r3		# r11->double word
	nand	r6,r5,r5	# r6 = complement of first word
	mr	r8,r5		# r8 = shifted input
	cal	r3,0(r0)	# set r3 to 0 = cumulative shift
loop2:	cntlz	r9,r8		# r9 = leading zeros in shifted input
	cmp	cr0,r9,r4	# compare with nbits
	bger	cr0             # return if r9 >= nbits
	a	r3,r3,r9	# cumulate shift amount
	cmpi	cr0,r3,32	# test for done
	rlnm	r8,r6,r3,ONES	# r8 = shifted complement
	bge	next1		# branch if 32 bits processed
	cntlz	r10,r8		# r10 = leading zeros in shifted complement
	a	r3,r3,r10	# cumulate shift amount
	cmpi	cr0,r3,32	# test for done
	oriu	r5,r5,0x8000	# set high bit to one to stop wrap-around 
	rlnm	r8,r5,r3,ONES	# r8 = shifted input
	blt	cr0,loop2	# done if total shifts >= 32
#
#       fall-thru here means no trailing zeros in first word.
#
	l	r5,4(r11)	# get next word
	b	next2
#
#  r9 = trailing zeros of first word. r3 = position of zeros + r9.
#
next1:	l	r5,4(r11)	# get next word
	sf	r3,r9,r3	# r3 = r3 - r9
	cntlz	r0,r5		# count leading zeros
	a	r9,r9,r0	# r9 = trailing + leading zeros
	cmp	cr0,r9,r4	# compare with nbits
	bger	cr0		# return if r9 >= nbits
#
# process second word. note that shifts of 32 or greater ignore
# high order bit of shift amount.
#
next2:	nand	r6,r5,r5	# r6 = complement of second word
	mr	r8,r5		# r8 = shifted input
	cal	r3,32(r0)	# r3 = cumulative shift
loop3:	cntlz	r9,r8		# r9 = leading zeros in shifted input
	cmp	cr0,r9,r4	# compare with nbits
	bger	cr0		# return if r9 >= nbits
	a	r3,r3,r9	# cumulate shift amount
	cmpi	cr0,r3,64	# test for done
	rlnm	r8,r6,r3,ONES	# r8 = shifted complement
	bger	cr0		# return if all 64 bits processed
	cntlz	r10,r8		# r10 = leading zeros in shifted complement
	a	r3,r3,r10	# cumulate shift amount
	cmpi	cr0,r3,64	# test for done
	oriu	r5,r5,0x8000	# set high bit to one to stop wrap-around 
	rlnm	r8,r5,r3,ONES	# r8 = shifted input
	blt	cr0,loop3	# done if total shifts >= 64
#
# fall thru here will return 64 or greater
#
	S_EPILOG

	FCNDES(v_findstring)

include(vmdefs.m4)
include(scrs.m4)
include(seg.m4)
include(errno.m4)
include(m_types.m4)
include(low_dsect.m4)
include(mstsave.m4)
include(machine.m4)
include(proc.m4)
include(user.m4)
include(param.m4)

