# @(#)65	1.1  src/bos/kernel/ml/POWER/copyx_pwr.s, sysml, bos41J, 9518A_all 5/2/95 08:56:23
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: copyin_pwr(), copyout_pwr(),
#	uiocopyin_pwr(), uiocopyout_pwr()
#	uiocopyin_chksum_pwr(), uiocopyout_chksum_pwr()
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994, 1995
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

	.file   "copyx_pwr.s"
        .machine "com"                          # Use common instruction set

include(systemcfg.m4)
include(macros.m4)
include(except.m4)
include(copyx_macros.m4)

        .set    CHUNK, 16                       # bytes to move per chunk
        .set    L2CHUNK, 4                      # log base 2 of CHUNK

#****************************************************************************
#
#       FUNCTION: copyin_pwr()
#
#       PURPOSE: Copies the contents of memory from
#                a user/system  address to
#                a trusted system address for a given number of bytes.
#               
#       INPUTS:  r3 -> source address
#                r4 -> destination address
#                r5 -> number of bytes
#
#       OUTPUTS: None
#
#       RETURNS: In r3  0  on success
#                       EINVAL if count is < 0
#       
#       PSEUDOCODE:
#       copyin_pwr(caddr_t src, caddr_t dest, int count)
#       {
#        uint type;
#
#        /* check for negative byte counts
#         */
#        if (count < 0)
#                return(EINVAL);
#
#        /* user or system space?
#         */
#        type = (curthread->t_flags & TKTHREAD)? UIO_SYSSPACE : UIO_USERSPACE ;
#
#        /* copy it in
#         */
#        /* take short path if the following are true
#           1. the source does not cross segment boundary
#           2. kernel lock is not held
#           3. if the number of threads is 1 or source in PRIVSEG
#           call vmcopyin() otherwise */
#
#        if ((((uint)src >> L2SSIZE) != (((uint)src + count -1) >> L2SSIZE))
#              || IS_LOCKED(&kernel_lock)  || 
#		(CURPROC->p_active > 1 && ((uint)src >> L2SSIZE) != PRIVSEG))
#             return(vmcopyin(src, dest, &count, type));
#        else
#             {
#             caddr_t tsrc;
#             int rc;
#
#             if (type == UIO_USERSPACE)
#                tsrc  = vm_att(as_geth(&u.u_adspace,src),src);
#             else
#                tsrc =src;
#
#             rc = xbcopy(tsrc,dest,count);
#
#             if (type == UIO_USERSPACE)
#                 vm_det(tsrc);
#
#             return (rc);
#               }
#         }
#****************************************************************************
#****************************************************************************
#
	
        .csect  copyin_pwr[PR],5
        .globl  ENTRY(copyin_pwr)
.copyin_pwr:
	FRONT_COPY_CODE(`IN', `COPY')
        lil     r9, CHUNK                       # number of bytes to move
        sri.    r7, r11, L2CHUNK                # number of chunks to move
        rlinm   r0, r11, 0, CHUNK-1             # remainder to move
        beq-    cr0, ismall_copy                # branch if less then CHUNK
                                                # bytes to move
        mtxer   r9                              # set for string move
        mtctr   r7                              # loop counter
	b	ic_loop				# skip no-ops
	.align	5				# make sure loop doesn't
						# cross cache line
ic_loop:
        lsx     r7, r6, r3                      # load CHUNK bytes
        stsx    r7, r6, r4                      # store CHUNK bytes
        ai      r6, r6, CHUNK                   # move to next chunk
        bdnz+   ic_loop                         # branch if more CHUNKs
ismall_copy:
        mtxer   r0                              # remaining bytes to move
        lsx     r7, r6, r3
        stsx    r7, r6, r4
	BACK_COPY_CODE(`IN', `COPY')

#**********************************************************************
#****************************************************************************
#
#       FUNCTION: copyout_pwr()
#
#       PURPOSE: Copies the contents of memory from
#                a trusted system address to
#                a user/system address for a given number of bytes.
#
#       INPUTS:  r3 -> source address
#                r4 -> destination address
#                r5 -> number of bytes
#
#       OUTPUTS: None
#
#       RETURNS: In r3  0  on success
#                       EINVAL if count is < 0
#
#       PSEUDOCODE:
#
#       copyout_pwr(caddr_t src, caddr_t dest, int count)
#       {
#        uint    type;
#
#        /* check for negative byte counts
#         */
#        if (count < 0)
#                return(EINVAL);
#
#        /* user or system ?
#         */
#        type = (curthread->t_flags & TKTHREAD)? UIO_SYSSPACE : UIO_USERSPACE ;
#
#        /* copy it out
#         */
#        /* take short path if the following are true
#           1. the destination does not cross segment boundary
#           2. kernel lock is not held
#           3. if the number of threads is 1 or destination in PRIVSEG
#           call vmcopyout() otherwise */
#
#        if ((( (uint)dest >> L2SSIZE) != (((uint)dest + count -1) >> L2SSIZE))
#              || IS_LOCKED(&kernel_lock)  || 
#		(CURPROC->p_active > 1 && ((uint)dest >> L2SSIZE) != PRIVSEG))
#              return(vmcopyout(src, dest, &count, type));
#        else
#             {
#             int rc;
#             caddr_t tdest;
#
#             if (type == UIO_USERSPACE)
#                 tdest  = vm_att(as_geth(&u.u_adspace,dest),dest);
#             else
#                 tdest = dest;
#
#             rc = xbcopy(src,tdest,count);
#
#             if (type == UIO_USERSPACE)
#                 vm_det(tdest);
#              return (rc);
#             }
#        }
#
#**********************************************************************
#
#
        .csect  copyout_pwr[PR],5
        .globl  ENTRY(copyout_pwr)
.copyout_pwr:
	FRONT_COPY_CODE(`OUT', `COPY')
        lil     r9, CHUNK                       # number of bytes to move
        sri.    r7, r11, L2CHUNK                # number of chunks to move
        rlinm   r0, r11, 0, CHUNK-1             # remainder to move
        beq-    cr0, osmall_copy                # branch if less then CHUNK
                                                # bytes to move
        mtxer   r9                              # set for string move
        mtctr   r7                              # loop counter
	b	oc_loop				# skip no-ops
	.align	5
oc_loop:
        lsx     r7, r6, r3                      # load CHUNK bytes
        stsx    r7, r6, r4                      # store CHUNK bytes
        ai      r6, r6, CHUNK                   # move to next chunk
        bdnz+   oc_loop                         # branch if more CHUNKs
osmall_copy:
        mtxer   r0                              # remaining bytes to move
        lsx     r7, r6, r3
        stsx    r7, r6, r4
	BACK_COPY_CODE(`OUT', `COPY')

#**********************************************************************
#
# 	FUNCTION: uiocopyin_pwr()
#
#       PURPOSE: Copies the contents of memory from
#                a user/system  address to
#                a trusted system address for a given number of bytes.
#  		 This routine is called from uiomove.
#               
#       INPUTS:  r3 -> source address
#                r4 -> destination address
#                r5 -> address of number of bytes
#                r6 -> user space or kernel space
#
#       OUTPUTS: None
#
#       RETURNS: In r3  0  on success
#                       EINVAL if count is < 0
#			EFAULT if exception
#			other return codes from vmcopyin()
#
#       PSEUDOCODE:
#	uiocopyin(caddr_t src, caddr_t dest, int *p_nbytes, int type)
#	{
#	/* check for negative byte counts
#	 */
#	if (*p_nbytes < 0)
#		return(EINVAL);
#
#	/* copy it in
#	 */
#	/* take short path if the following are true
#	   1. the source does not cross segment boundary
#	   2. kernel lock is not held
#	   3. process is single threaded or source in PRIVSEG
#	   call vmcopyin/out() otherwise */
#
#	if ((((uint)src >> L2SSIZE) != (((uint)src + nbytes -1) >> L2SSIZE))
#              || IS_LOCKED(&kernel_lock)  || 
#		(CURPROC->p_active > 1 && ((uint)src >> L2SSIZE) != PRIVSEG))
#
#		return(vmcopyin(src, dest, p_nbytes, type));
#
#	else {  /* short path copy */
#		
#		if (not kernel thread)
#			use temporary seg reg
#		/* set up exception handler (excbranch in mst) */
#		mstp->excbranch = &excp_hand
#		move small chunk (up to first 16 byte boundary)
#		loop with string ops (lsx, stsx) to move data chunks (aligned)
#		move remaining bytes (small chunk)
#		/* clear exception handler */
#		mstp->excbranch = 0;
#		if (not kernel thread)
#			restore temporary seg reg
#		return
#	}
#
#	excp_hand:
#		faulting address is in mstp->o_vaddr
#		assert(faulting address is source)
#		*p_nbytes = mstp->o_vaddr - src
#		if (not kernel thread)
#			restore temp seg reg
#		/* exception value is in r3 */
#		rc = (exval > EXCEPT_ERRNO) ? EFAULT : exval;
#		return rc
#	}
#
#
#**********************************************************************
		
	.csect	uiocopyin_pwr[PR],5
	.globl	ENTRY(uiocopyin_pwr)
.uiocopyin_pwr:
	FRONT_COPY_CODE(`IN', `UIO')
#
# copy src=r3, dest=r4, &nbytes=r5, count=r11
# note: during lsx/stsx r5 = &nbytes, r11=nbytes, r12 = saved TEMPSR
#

	rlinm.	r9,r3,0,CHUNK-1			# compute size for alignment
	lil	r10,CHUNK
	sf	r8,r9,r10			# r8=size of first move (align)
	cmp	cr0,r11,r8
	sf	r7,r8,r11			# compute remaining size
	ble-	cr0,uio_i_finish
	mtxer	r8				# set count for string op
	sri	r0,r7,L2CHUNK			# chunks to move
	lsx	r7,0,r3				# load initial bytes
	cmpi	cr0,r0,0			# check if any CHUNKS to move
	stsx	r7,0,r4				# store initial bytes
	mfxer	r6				# initial index for loop
	beq-	cr0,uio_i_finish		# no CHUNKS, jump to last copy
	lil	r9, CHUNK			# bytes to move
	mtctr	r0				# set ctr for loop
	mtxer	r9				# set count for string op
	b	uio_ic_loop			# skip noops
	.align	5
uio_ic_loop:
	lsx	r7,r6,r3			# load CHUNK bytes
	stsx	r7,r6,r4			# store CHUNK bytes
	ai	r6,r6,CHUNK			# increment index
	bdnz+	uio_ic_loop			# loop if more CHUNKs
#
# copy last small chunk
#
uio_i_finish:
	sf	r7,r6,r11			# find num bytes remaining
	mtxer	r7				# set count for string op
	lsx	r7,r6,r3			# load final bytes
	stsx	r7,r6,r4			# store final bytes
	BACK_COPY_CODE(`IN', `UIO')

#****************************************************************************
#
#       FUNCTION: uiocopyout_pwr()
#
#       PURPOSE: Copies the contents of memory from
#                a trusted system address to
#                a user/system address for a given number of bytes.
#                This routine is called from uiomove.
#
#       INPUTS:  r3 -> source address
#                r4 -> destination address
#                r5 -> address of number of bytes
#                r6 -> user space or kernel space
#
#       OUTPUTS: None
#
#       RETURNS: In r3  0  on success
#                       EINVAL if count is < 0
#			EFAULT if exception
#			other return codes from vmcopyin()
#
#       PSEUDOCODE:
#
#	uiocopyout(caddr_t src,caddr_t dest,int *p_nbytes,int type)
#	{
#	/* check for negative byte counts
#	 */
#	if (nbytes < 0)
#		return(EINVAL);
#
#	/* copy it out
#	 */
#	/* take short path if the following are true
#	   1. the destination does not cross segment boundary
#	   2. kernel lock is not held
#	   3. process is single threaded or destination in PRIVSEG
#	   call vmcopyin/out() otherwise */
#
#	if ((((uint)dest >> L2SSIZE) != (((uint)dest + *nbytes -1) >> L2SSIZE))
#              || IS_LOCKED(&kernel_lock)  || 
#		(CURPROC->p_active > 1 && ((uint)dest >> L2SSIZE) != PRIVSEG))
#
#		return(vmcopyout(src, dest, p_nbytes, type));
#
#	else {  /* short path copy */
#		
#		if (not kernel thread)
#			use temporary seg reg
#		/* set up exception handler (excbranch in mst) */
#		mstp->excbranch = &excp_hand;
#		loop with string ops (lsx, stsx) to move data chunks
#		move remaining bytes (small chunk)
#		/* clear exception handler */
#		mstp->excbranch = 0;
#		if (not kernel thread)
#			restore temporary seg reg
#		return
#	}
#
#	excp_hand:
#		faulting address is in mstp->o_vaddr
#		*p_nbytes = mstp->o_vaddr - dest
#		if (not kernel thread)
#			restore temp seg reg
#		/* exception value is in r3 */
#		rc = (exval > EXCEPT_ERRNO) ? EFAULT : exval;
#		return rc
#	}
#
#**********************************************************************

	.csect	uiocopyout_pwr[PR],5
	.globl	ENTRY(uiocopyout_pwr)
.uiocopyout_pwr:
	FRONT_COPY_CODE(`OUT', `UIO')
	sri.	r8,r11,L2CHUNK			# chunks to move
	rlinm	r0,r11,0,CHUNK-1		# remainder to move
	beq-	cr0,uio_o_finish		# no CHUNKS, jump to last copy
	lil	r9, CHUNK			# bytes to move
	mtctr	r8				# set ctr for loop
	mtxer	r9				# set count for string op
	b	uio_oc_loop			# skip noops
	.align	5
uio_oc_loop:
	lsx	r7,r6,r3			# load CHUNK bytes
	stsx	r7,r6,r4			# store CHUNK bytes
	ai	r6,r6,CHUNK			# increment index
	bdnz+	uio_oc_loop			# loop if more CHUNKs
#
# copy last small chunk
#
uio_o_finish:
	mtxer	r0				# set count for string op
	lsx	r7,r6,r3			# load final bytes
	stsx	r7,r6,r4			# store final bytes
	BACK_COPY_CODE(`OUT', `UIO')

#**********************************************************************
#
# 	FUNCTION: uiocopyin_chksum_pwr()
#
#       PURPOSE: Copies the contents of memory from
#                a user/system  address to
#                a trusted system address for a given number of bytes.
#		 The TCP checksum is computed during the move
#  		 This routine is called from uiomove_chksum.
#               
#       INPUTS:  r3 -> source address
#                r4 -> destination address
#                r5 -> address of number of bytes
#                r6 -> user space or kernel space
#		 r7 -> address where the checksum goes
#		       NOTE: if we have to make the call to C code, this
#		       sum will not be valid.  The upper two bytes will
#		       be non-zero if this is the case.
#
#       OUTPUTS: None
#
#       RETURNS: In r3  0  on success
#                       EINVAL if count is < 0
#			EFAULT if exception
#			other return codes from vmcopyin()
#       
#       PSEUDOCODE:
#	uiocopyin_chksum_pwr(caddr_t src, caddr_t dest, int *p_nbytes,
#	    int type, uint *sum)
#	{
#	/* check for negative byte counts
#	 */
#	if (*p_nbytes < 0)
#		return(EINVAL);
#
#	/* copy it in
#	 */
#	/* take short path if the following are true
#	   1. the source does not cross segment boundary
#	   2. kernel lock is not held
#	   3. process is single threaded or source in PRIVSEG
#	   call vmcopyin/out() otherwise */
#
#	if ((((uint)src >> L2SSIZE) != (((uint)src + nbytes -1) >> L2SSIZE))
#              || IS_LOCKED(&kernel_lock)  || 
#		(CURPROC->p_active > 1 && ((uint)src >> L2SSIZE) != PRIVSEG))
#
#		set upper two bytes of sum 0xffff to indicate invalid
#		return(vmcopyin(src, dest, p_nbytes, type));
#	}
#
#	else {  /* short path copy */
#		
#		if (not kernel thread)
#			use temporary seg reg
#		/* set up exception handler (excbranch in mst) */
#		mstp->excbranch = &excp_hand
#		move small chunk (up to first 16 byte boundary)
#		compute checksum of small chunk
#		loop with string ops (lsx, stsx) to move data chunks (aligned)
#		compute checksum in loop
#		move remaining bytes (small chunk)
#		compute checksum of remaining bytes
#		/* clear exception handler */
#		mstp->excbranch = 0;
#		if (not kernel thread)
#			restore temporary seg reg
#		return
#	}
#
#	excp_hand:
#		faulting address is in mstp->o_vaddr
#		assert(faulting address is source)
#		*p_nbytes = mstp->o_vaddr - src
#		if (not kernel thread)
#			restore temp seg reg
#		/* exception value is in r3 */
#		rc = (exval > EXCEPT_ERRNO) ? EFAULT : exval;
#		return rc
#	}
#
#
#**********************************************************************

	.csect	uiocopyin_chksum_pwr[PR],5
	.globl	ENTRY(uiocopyin_chksum_pwr)
.uiocopyin_chksum_pwr:
	FRONT_COPY_CODE(`IN', `UIOC')
#
# copy src=r3, dest=r4, &nbytes=r5, count=r11
# note: during lsx/stsx r5 = &nbytes, r11=nbytes, r12 = saved TEMPSR
#       r13 = addr to store checksum, r14 = checksum,
#       r15 = 0 or 8 to indicate whether sum is shifted or not during loop
#
	mr	r13,r7				# r13=addr where to store cksum

	rlinm.	r9,r3,0,CHUNK-1			# compute size for alignment
	lil	r10,CHUNK
	lil	r14,0				# clear checksum
	sf	r16,r9,r10			# r16=size of first move(align)
	lil	r15,0				# init shift
	cmp	cr0,r11,r16
	sf	r7,r16,r11			# compute remaining size
	ble-	cr0,uioc_i_finish1
	mtxer	r16				# set cnt for str op/clr carry
	sri	r0,r7,L2CHUNK			# chunks to move
	lsx	r7,0,r3				# load initial bytes
	cmpi	cr0,r0,0			# check if any CHUNKS to move
	stsx	r7,0,r4				# store initial bytes

	cmpi	cr1,r16,5			# if only r7 loaded ...
	a 	r14,r14,r7			# start sum
	blt+	cr1,uioc_i_done_sum		# finished sum, jump to done

	cmpi	cr1,r16,9			# if only r7,r8 loaded ...
	ae	r14,r14,r8			# add r8 to sum
	blt+	cr1,uioc_i_done_sum		# finished sum, jump to done

	cmpi	cr1,r16,13			# if only r7,r8,r9 loaded ...
	ae	r14,r14,r9			# add r9 to sum
	blt+	cr1,uioc_i_done_sum		# finished sum, jump to done

	ae	r14,r14,r10			# add r10 to sum

uioc_i_done_sum:
	aze 	r14,r14				# add in carry (we may shift)
	rlinm	r15,r16,3,0x8			# set shift (r15) if odd bytes
	mfxer	r6				# initial index for loop
	rlnm	r14,r14,r15,0xffffffff		# shift if odd offset

	beq-	cr0,uioc_i_finish		# no CHUNKS, jump to last copy
	lil	r9, CHUNK			# bytes to move
	mtctr	r0				# set ctr for loop
	mtxer	r9				# set count for string op
	.align	5
uioc_ic_loop:
	lsx	r7,r6,r3			# load CHUNK bytes
	ae	r14,r14,r7			# add r7 to sum
	ae	r14,r14,r8			# add r8 to sum
	stsx	r7,r6,r4			# store CHUNK bytes
	ae	r14,r14,r9			# add r9 to sum
	ae	r14,r14,r10			# add r10 to sum
	addi	r6,r6,CHUNK			# increment index
	bdnz+	uioc_ic_loop			# loop if more CHUNKs
#
# copy last small chunk
#
uioc_i_finish:
	aze	r14,r14				# add in carry
uioc_i_finish1:
	sf	r16,r6,r11			# find num bytes remaining
	mtxer	r16				# set count for string op
	cmpi	cr1,r16,0			# check for 0 bytes ...
	beq-	cr1,uioc_idone2			# and jump to avoid sum
	lsx	r7,r6,r3			# load final bytes
	stsx	r7,r6,r4			# store final bytes

	cmpi	cr1,r16,5			# if only r7 loaded ...
	a  	r14,r14,r7			# add r7 to sum
	blt+	cr1,uioc_idone			# finished sum, jump to done

	cmpi	cr1,r16,9			# if only r7,r8 loaded ...
	ae	r14,r14,r8			# add r8 to sum
	blt+	cr1,uioc_idone			# finished sum, jump to done

	cmpi	cr1,r16,13			# if only r7,r8,r9 loaded ...
	ae	r14,r14,r9			# add r9 to sum
	blt+	cr1,uioc_idone			# finished sum, jump to done

	ae	r14,r14,r10			# add r10 to sum
#
# copy was successful, number of bytes is already correct
#
uioc_idone:
	aze	r14,r14				# add in carry from last op
uioc_idone2:
	rlnm	r14,r14,r15,0xffffffff		# shift again iff necessary
#
#  Finish up checksum - fold 2 16 bit sums and add with carry
#
	rlinm	r16,r14,16,0xffff0000		# shift lower 2 bytes to upper
	l	r15, -12(r1)			# restore r15
	a	r14,r14,r16			# add lower 2 to upper 2
	l	r16, -16(r1)			# restore r16
	rlinm	r14,r14,16,0x0000ffff		# shift sum to lower 2 bytes
	aze	r14,r14				# add final carry to sum
	xoril	r14,r14,0xffff			# one's complement sum
	st	r14,0(r13)			# store final sum
	l	r13, -4(r1)			# restore r13
	l	r14, -8(r1)			# restore r14
	BACK_COPY_CODE(`IN', `UIOC')
#****************************************************************************
#
#       FUNCTION: uiocopyout_chksum_pwr()
#
#       PURPOSE: Copies the contents of memory from
#                a trusted system address to
#                a user/system address for a given number of bytes.
#                This routine is called from uiomove.
#
#       INPUTS:  r3 -> source address
#                r4 -> destination address
#                r5 -> address of number of bytes
#                r6 -> user space or kernel space
#
#       OUTPUTS: None
#
#       RETURNS: In r3  0  on success
#                       EINVAL if count is < 0
#			EFAULT if exception
#			other return codes from vmcopyin()
#
#       PSEUDOCODE:
#
#	uiocopyout_chksum_pwr(caddr_t src,caddr_t dest,int *p_nbytes,int type)
#	{
#	/* check for negative byte counts
#	 */
#	if (nbytes < 0)
#		return(EINVAL);
#
#	/* copy it out
#	 */
#	/* take short path if the following are true
#	   1. the destination does not cross segment boundary
#	   2. kernel lock is not held
#	   3. process is single threaded or destination in PRIVSEG
#	   call vmcopyin/out() otherwise */
#
#	if ((((uint)dest >> L2SSIZE) != (((uint)dest + *nbytes -1) >> L2SSIZE))
#              || IS_LOCKED(&kernel_lock)  || 
#		(CURPROC->p_active > 1 && ((uint)dest >> L2SSIZE) != PRIVSEG))
#
#		return(vmcopyout(src, dest, p_nbytes, type));
#
#	else {  /* short path copy */
#		
#		if (not kernel thread)
#			use temporary seg reg
#		/* set up exception handler (excbranch in mst) */
#		mstp->excbranch = &excp_hand;
#		loop with string ops (lsx, stsx) to move data chunks
#		move remaining bytes (small chunk)
#		/* clear exception handler */
#		mstp->excbranch = 0;
#		if (not kernel thread)
#			restore temporary seg reg
#		return
#	}
#
#	excp_hand:
#		faulting address is in mstp->o_vaddr
#		*p_nbytes = mstp->o_vaddr - dest
#		if (not kernel thread)
#			restore temp seg reg
#		/* exception value is in r3 */
#		rc = (exval > EXCEPT_ERRNO) ? EFAULT : exval;
#		return rc
#	}
#
#**********************************************************************

	.csect	uiocopyout_chksum_pwr[PR],5
	.globl	ENTRY(uiocopyout_chksum_pwr)
.uiocopyout_chksum_pwr:
	FRONT_COPY_CODE(`OUT', `UIOC')
#
# copy src=r3, dest=r4, count=r11
# note: r5 = &nbytes, r11=nbytes, r12 = saved TEMPSR
#       r13 = addr to store checksum, r14 = checksum,
#

uioc_o_kthread:
	mr	r13,r7				# r13=addr where to store cksum
	lil	r14,0				# clear checksum
	sri.	r8,r11,L2CHUNK			# chunks to move
	rlinm	r0,r11,0,CHUNK-1		# remainder to move
	beq-	cr0,uioc_o_finish		# no CHUNKS, jump to last copy
	lil	r9, CHUNK			# bytes to move
	mtctr	r8				# set ctr for loop
	mtxer	r9				# set cnt for str op/clr carry
	b	uioc_oc_loop			# branch around no-ops
	.align	5
uioc_oc_loop:
	lsx	r7,r6,r3			# load CHUNK bytes
	ae	r14,r14,r7			# add r7 to sum
	ae	r14,r14,r8			# add r8 to sum
	stsx	r7,r6,r4			# store CHUNK bytes
	ae	r14,r14,r9			# add r9 to sum
	ae	r14,r14,r10			# add r10 to sum
	addi	r6,r6,CHUNK			# increment index
	bdnz+	uioc_oc_loop			# loop if more CHUNKs

	aze	r14,r14				# add in carry
#
# copy last small chunk
#
uioc_o_finish:
	cmpi	cr1,r0,0			# check for 0 bytes ...
	mtxer	r0				# set count for string op
	beq-	cr1,uioc_odone2			# ... and jump to avoid sum
	lsx	r7,r6,r3			# load final bytes
	stsx	r7,r6,r4			# store final bytes

	cmpi	cr1,r0,5			# if only r7 loaded ...
	a  	r14,r14,r7			# add r7 to sum
	blt+	cr1,uioc_odone			# finished sum, jump to done

	cmpi	cr1,r0,9			# if only r7,r8 loaded ...
	ae	r14,r14,r8			# add r8 to sum
	blt+	cr1,uioc_odone			# finished sum, jump to done

	cmpi	cr1,r0,13			# if only r7,r8,r9 loaded ...
	ae	r14,r14,r9			# add r9 to sum
	blt+	cr1,uioc_odone			# finished sum, jump to done

	ae	r14,r14,r10			# add r10 to sum
#
# copy was successful, number of bytes is already correct
#
uioc_odone:
	aze	r14,r14				# add in carry from last op
uioc_odone2:
#
#  Finish up checksum - fold 2 16 bit sums and add with carry
#
	rlinm	r7,r14,16,0xffff0000		# shift lower 2 bytes to upper
	a	r14,r14,r7			# add lower 2 to upper 2
	rlinm	r14,r14,16,0x0000ffff		# shift sum to lower 2 bytes
	aze	r14,r14				# add final carry to sum
	xoril	r14,r14,0xffff			# one's complement sum
	st	r14,0(r13)			# store final sum
	l	r13, -4(r1)			# restore r13
	l	r14, -8(r1)			# restore r14
	BACK_COPY_CODE(`OUT', `UIOC')

        .toc
        TOCE(kernel_lock, data)                  # toc entry for ext var

include(mstsave.m4)
include(machine.m4)
include(low_dsect.m4)
include(proc.m4)
include(scrs.m4)
include(user.m4)
include(errno.m4)
include(vmdefs.m4)
include(lock_def.m4)
include(seg.m4)
