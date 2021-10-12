# @(#)70        1.4  src/bos/kernel/ml/POWER/dma_pwr.s, sysios, bos411, 9428A410j 7/27/93 20:53:31

#*****************************************************************************
#
# COMPONENT_NAME: (SYSIOS) 
#
# FUNCTIONS: d_kmove iocc_rw iocc_str master_flush
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1990,1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************


	.file "dma_pwr.s"
	.machine "pwr"
	.using	low, r0

include(macros.m4)

ifdef(`_POWER_RS',`

#------------------------------------------------------------------------------
#
# NAME: iocc_rw
#
# FUNCTION: word access to the IOCC.  Sets up an exception handler, and will
#	retry the load/store if an exception occurs.
#
# CALL: int iocc_rw(direction, address, data)
#	ulong direction;	/* 0 - read	1 - write		*/
#	ulong *address;		/* IOCC address				*/
#	ulong data;		/* for write contains data to write	*/
#
# EXECUTION ENVORONMENT:
#	called only by the DMA services
#
# RETURNS:
#	for read returns value loaded
#	for write returns 0
#
#------------------------------------------------------------------------------

	S_PROLOG(iocc_rw)
	LTOC(r7, iocc_excpt, data)	# get address of exception handler
	cmpi	cr1, r3, 0		# check the read/write flag
	GET_CSA(cr0, r8, r6)		# get pointer to current mst
	lil	r8, PIO_RETRY_COUNT	# initialize retry counter

iocc_retry:
	st	r7, excbranch(r6)	# set up exception handler
	beq	cr1, iocc_read		# branch if direction == 0

	st	r5, 0(r4)		# careful, this could except
	lil	r3, 0			
	st	r3, excbranch(r6)	# clear exception handler
	br				# return success

iocc_read:
	l	r3, 0(r4)		# carefull, this could except
	lil	r4, 0
	st	r4, excbranch(r6)	# clear exception handler
	br				# return success

iocc_excpt:
	cmpi	cr0, r3, EXCEPT_IO	# check exception code
	bne	cr0, iocc_trap		# if its not an IO exception then trap
	ai.	r8, r8, -1		# decrement retry count
	bge	cr0, iocc_retry		# if retry_count >= 0 then retry it
iocc_trap:
	TRAP

#-----------------------------------------------------------------------------
#
# NAME: d_kmove_rs1
#
# FUNCTION: This service provides consistent access to memory
#	when a device and its driver both have access to system
#	memory.
#
# CALL:	int d_kmove(baddr, daddr, count, channel_id, bid, flags)
#	char *baddr;		/* buffer address		*/
#	char *daddr;		/* bus address			*/
#	int count;		/* number of bytes to move	*/
#	int channel_id;		/* channel to use		*/
#	int bid;
#	int flags;		/* control flags		*/
#	
#
# EXECUTION ENVIRONMENT:
#	This service can be called on either the process or
#	interrupt level
#
#	baddr must be in kernel address space
#
#	It only page faults on the stack when under a process
#
# NOTES:
#	This is a specialized interface for the d_move service.
#
# RETURN VALUE:
#	0 	=	successful
#	-1	=	not successful
#	EINVAL  =	Operation not supported
#
#-----------------------------------------------------------------------------
	.set	ICF_BUF_MSK, 0x30	# mask for type of iocc buffering
	.set	ICF_UNBUFF, 0x10	# un-bufferd iocc

	.set	DMA_READ, 0x80

	.set	CSR15_HI, 0xb04f	# effective address for csr15.
	.set	CSR15_LO, 0x0060	#  Using segment 12

	.set	BUID_MASK, 0x0ff00000	# mask bid to buid value

	.set	IO_SEG_HI, 0x800c	# srval used to access csr15
	.set	IO_SEG_LO, 0x00e0

	S_PROLOG(d_kmove_rs1)

# Save some registers for use as local variables.
# Register usage:
#	r31 - saved msr value
#	r30 - seg reg 12 save value
#	r29 - seg reg 11 save value
#	r28 - old interrupt priority
#	r27 - csr save value


	LTOC(r9, iocc_config, data)	# get shadow copy of iocc config reg
	l	r9, 0(r9)
	stm	r27, -5*4(r1)
	rlinm	r9, r9, 0, ICF_BUF_MSK	# mask out bufferd bits
	cmpi	cr0, r9, ICF_UNBUFF	# check for unbuffed iocc
	mfmsr	r31			# save the old msr value

					#  in cr0 for later use.
	rlinm	r0, r31, 0, ~MSR_EE	# calculate the INTMAX msr value
	GET_CSA(cr1, r10, r12)		# get pointer to our state save area
	beq	cr0, bad_move		# branch if RSC (unbuffered iocc)
	mfsr	r30, sr12		# save sr12
	lbz	r28, mstintpri(r12)	# save old interrupt priority
	mtmsr	r0			# disable the machine
	cmpi	cr5, r28, INTBASE	# check if we are at intbase
	mfsr	r29, sr11		# save sr11
	rlinm.	r8, r8, 0, DMA_READ	# check if read or write. Save result
	lil	r9, INTMAX		# new interrupt priority
	bne	cr5, fixed_stack	# branch if oldpri != INTBASE. All
					#  ready has a fixed stack

# set stack fix, and touch the stack. If this faults resume will carefully
# touch the stack before we run again

	st	r1, mststackfix(r12)
	l	r0, -STACKTOUCH(r1)
	l	r0, 0(r1)
	l	r0, STACKTOUCH(r1)

fixed_stack:
	stb	r9, mstintpri(r12)	# set new interrupt priority

	rlinm	r9, r7, 0, BUID_MASK	# mask the given buid
	oriu	r9, r9, IO_SEG_HI	# set io seg address check and
					#  increment
	oril	r10, r9, IO_SEG_LO	# set IOCC select, 24-bit mode and
					#  bypass mode
	mtsr	sr11, r10		# load seg for access to to csr15

	liu	r8, CSR15_HI		# upper 16 bits of csr15 address
	l	r27, CSR15_LO(r8)	# save old value of CSR15
	sli	r11, r6, 16		# new csr val
					#  CSR(0, channel, MASTER_NO_AUTHORITY)
	st	r11, CSR15_LO(r8)	# set the CSR15 value

	mtsr	sr12, r9		# load bus segment, address check,
					#  address increment, bypass bit off
	oriu	r4, r4, 0xc000		# calculate the effective address for
					#  bus

	beq	cr0, dma_write		# branch if DMA_READ was not set in
					#  flags
	mr	r0, r3			
	mr	r3, r4
	mr	r4, r0

dma_write:
	LTOC(r7, kdexcpt, data)		# get address of exception handler
	sri.	r6, r5, 5		# r6 = count / 32
	rlinm	r0, r5, 0, 0x1f		# r0 = count % 32
	st	r7, excbranch(r12)	# set up exception handler
	mtxer	r0			# setup xer for copy of remainder
	beq	small			# branch if count < 32

	mtctr	r6			# count of 32 byte chunks to move
loop:
	lsi	r5, r3, 32		# move memory
	ai	r3, r3, 32	
	stsi	r5, r4, 32
	ai	r4, r4, 32
	bdn	loop

small:
	lsx	r5, 0, r3		# move the remainder
	stsx	r5, 0, r4

	GET_CSA(cr0, r11, r12)		# get current mst pointer
	lil	r3, 0			# get return code
	st	r3, excbranch(r12)	# clear the exception handler

cleanup:
	liu	r11, CSR15_HI		# get high part of csr15 address
	st	r27, CSR15_LO(r11)	# retore CSR15 value
	mtsr	sr12, r30
	mtsr	sr11, r29
	stb	r28, mstintpri(r12)
	bne	cr5, still_fixed	# branch if we were called on a fixed
					#  stack

	cal	r4, NULL(0)		# clear mststackfix to return to
	st	r4, mststackfix(r12)	# non - fixed stack

still_fixed:
	mtmsr	r31			# enable interrupts
exit:
	lm	r27, -5*4(r1)		# restore non-volatile registers
	br				# return

kdexcpt:
	l	r4, syscfg_impl(0)	# get implementation
	andil.	r4, r4, POWER_RS2	# check if its an RS2
        LTOC(r4, i_data, data)          # Load addr of interrupt mask table
	bne	kdrs2			# branch if on RS2

        mfsr    r9, sr15                # Save segment reg 15
        cau     r8, 0, BUID0            # Set segment register 15
        mtsr    sr15, r8                #   to allow I/O access
        rlinm   r5, r28, 3, 0x7F8       # Mult by 8 to get mask index
	a	r5, r5, r4		# get address of map
	lsi	r6, r5, 8		# load masks
        cau     r5, 0, 0xF000           # address of EIMs
	stsi	r6, r5, 8		# write EIMs
        mtsr    sr15, r9                # Restore segment reg 15
	b	kdfin

kdrs2:
	lbzx	r5, r28, r4		# get new CIL level
	rlinm	r5, r5, 8, 0xFF00	# generate ILCR command
	mtspr	ILCR, r5		# update CIL value
	muli	r3, r3, 1		# wait for update
	muli	r3, r3, 1

kdfin:
	GET_CSA(cr0, r11, r12)		# set up mst pointer
	cal	r3, -1(0)		# set return code
	b	cleanup			# cleanup and exit

bad_move:
	lil	r3, EINVAL		# set return code
	b	exit		# return to caller

	FCNDES(d_kmove_rs1)

#-----------------------------------------------------------------------------
#
# NAME: master_flush
#
# FUNCTION: write buffer flush command to iocc.  disables inline to INTMAX.
#	and wraps exception handler around buffer flush.
#
# CALL:	int master_flush(csr15addr, csr15val, tcwaddr, iocc_config_addr,
#				errstate)
#	ulong *csr15addr;	/* effective address of CSR 15		*/
#	ulong car15val;		/* value for CSR 15 while executing flush */
#	ulong *tcwaddr;		/* address to write flush command to	*/
#	ulong *iocc_config_addr; /* must do a load from this addres	*/
#	struc {			/* info. for caller to clean up after error */
#		int intpri;
#		ulong csr15save;
#	} *errstate;
#
# EXECUTION ENVIRONMENT:
#	See MASTER_FLUSH macro in dma_hw.h
#
# RETURNS:
#	0 - on success
#	EXCPT_IO - error occurred during flush.  It is up to the caller to
#		clean up. Channel status register 15 should be set to
#		errstate->csr15save,  then i_enable should be called to
#		reset interrupt priority to errstate->intpri
#
#------------------------------------------------------------------------------

	# register usage:
	#	r12 - mst
	#	r11 - msr save
	#	r10 - old intpri
	#	r8 - old  csr15 value


	S_PROLOG(master_flush)
	GET_CSA(cr0, r11, r12)		# get mst pointer
	lbz	r10, mstintpri(r12)	# get old interrupt priority
	mfmsr	r11			# get old msr
	cmpi	cr1, r10, INTBASE	# check if we are at INTBASE
	rlinm	r0, r11, 0, ~MSR_EE	# generate disabled MSR
	mtmsr	r0
	LTOC(r9, mf_excpt, data)	# get address of exception handler
	bne	cr1, mf_fixed_stack	# branch if we are already on a fixed
					#  stack
	st	r1, mststackfix(r12)	# fix the stack
	l	r0, STACKTOUCH(r1)	# touch the stack to be sure it
	l	r0, 0(r1)		# is there
	l	r0, -STACKTOUCH(r1)

mf_fixed_stack:

#	It is important that the software interrupt priority is not set
#	until the stack is touched.  This way the EIMs will not change
#	if the stack touch page faults.

	lil	r0, INTMAX		# Now set software interrupt priority
	stb	r0, mstintpri(r12)

	l	r8, 0(r3)		# get old csr15 value
	lil	r0, 0
	st	r4, 0(r3)		# set new csr15 value

	st	r9, excbranch(r12)	# set up exception handler
	st	r0, 0(r5)		# do flush -- this could except
	st	r8, 0(r3)		# restore csr15
	l	r8, 0(r6)		# for IOCC bug see LOAD_IOCC_CONFIG

	st	r0, excbranch(r12)	# clear exception handler
	bne	cr1, mf_unfix_stack	# branch if we entered on a fixed stack
	st	r0, mststackfix(r12)	# clear stack fix
mf_unfix_stack:
	lil	r3, 0			# set return code
	stb	r10, mstintpri(r12)	# restore software priority
	mtmsr	r11			# restore hardware priority
	br

mf_excpt:
	cmpi	cr0, r3, EXCEPT_IO	# Check for IO exception
	bne	cr0, mf_trap		# if not except_io then trap
	st	r10, 0(r7)		# save interrupt priority
	st	r8, 4(r7)		# save old csr15
	br				# return to C
mf_trap:
	TRAP

#------------------------------------------------------------------------------
#
# NAME: disable_chan
#
# FUNCTION: issue the disable command
#
# CALL:	void disable_chan(disable_addr, io_addr)
#	volatile ulong *disable_addr;	/* address of disable command */
#	volatile ulong *io_addr;	/* address of any load io address */
#
# NOTES:
#	A load to any IO address is issued before the store-disable command.
#	If the disable command is surrounded by word aligned fixed point
#	stores an RSC bug will show up
#
# RETURNS: None
#
#------------------------------------------------------------------------------

	S_PROLOG(disable_chan)
	l	r5, 0(r4)		# do dummy io load
	st	r5, 0(r3)		# issue store command
	br

	_DF(_DF_NOFRAME)	

	.toc
	TOCE(i_data, data)
	TOCE(iocc_config, data)
	TOCL(kdexcpt, data)
	TOCL(iocc_excpt, data)
	TOCL(mf_excpt, data)

',)

include(param.m4)
include(mstsave.m4)
include(low_dsect.m4)
include(i_machine.m4)
include(machine.m4)
include(scrs.m4)
include(except.m4)
include(errno.m4)
include(systemcfg.m4)
