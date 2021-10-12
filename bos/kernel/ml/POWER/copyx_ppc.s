# @(#)64	1.1  src/bos/kernel/ml/POWER/copyx_ppc.s, sysml, bos41J, 9518A_all 5/2/95 08:56:17
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: copyin_ppc(), copyout_ppc()
#	uiocopyin_ppc(), uiocopyout_ppc()
#	uiocopyin_chksum_ppc(), uiocopyout_chksum_ppc()
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

	.file   "copyx_ppc.s"
        .machine "ppc"

include(systemcfg.m4)
include(macros.m4)
include(except.m4)
include(copyx_macros.m4)

#****************************************************************************
#
#       FUNCTION: copyin_ppc()
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
#       copyin_ppc(caddr_t src, caddr_t dest, int count)
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
        .csect  copyin_ppc[PR],5
        .globl  ENTRY(copyin_ppc)

.copyin_ppc:
	FRONT_COPY_CODE(`IN', `COPY', isync)
	cmpi	cr5, r11, 12			# check for small move
	andil.	r9, r4, 0x3			# check alignment of target
	blt-	cr5, ismall			# go handle small move
	beq+	cr0, inofixup			# branch if target aligned
	sfi	r9, r9, 0x4			# number bytes to align target	
	mtctr	r9				# use counter for loop
ifixup:
	lbzx	r7, r6, r3			# load byte
	stbx	r7, r6, r4			# and store it
	addi	r6, r6, 1			# update count
	bdnz	ifixup				# until done
inofixup:
	subfc	r7, r6, r11			# number bytes left to move
	andil.	r9, r7, 0x7			# number of scraps
	srwi	r10, r7, 0x3			# number of chunks to move
	mtctr	r10				# use counter for loop
	addi	r10, r6, 4			# psuedo counter for unrolled loop
	b	iloop				# skip no-ops
	.align	5
iloop:
	lwzx	r7, r6, r3			# load word
	lwzx	r8, r10, r3			# load word
	stwx	r7, r6, r4			# store word
	stwx	r8, r10, r4			# store word
	addi	r6, r6, 8			# update (real) counter
	addi	r10, r10, 8			# update (psuedo) counter
	bdnz+	iloop				# until done
	beq+	cr0, `MOVE_FINISHED'CLEVER_LABEL	# if no scraps
	mtctr	r9				# number of scraps
iscraps:
	lbzx	r7, r6, r3			# load byte
	stbx	r7, r6, r4			# store byte
	addi	r6, r6, 1			# update counter
	bdnz+	iscraps				# until done
	b	`MOVE_FINISHED'CLEVER_LABEL	# go to cleanup code
	
ismall:
	mtctr	r11				# use counter for loop
	b	ismall_loop			# skip no-ops
	.align 5
ismall_loop:
	lbzx	r7, r6, r3			# load byte
	stbx	r7, r6, r4			# store byte
	addi	r6, r6, 1			# update count
	bdnz+	ismall_loop			# until done
# NOTE -- fall thru to cleanup code
	
	BACK_COPY_CODE(`IN', `COPY', isync)

#****************************************************************************
#
#       FUNCTION: copyout_ppc()
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
#       copyout_ppc(caddr_t src, caddr_t dest, int count)
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
        .csect  copyout_ppc[PR],5
        .globl  ENTRY(copyout_ppc)
.copyout_ppc:
	FRONT_COPY_CODE(`OUT', `COPY', isync)
	cmpi	cr5, r11, 12			# check for small move
	andil.	r9, r4, 0x3			# check alignment of target
	blt-	cr5, osmall			# go handle small move
	beq+	cr0, onofixup			# branch if target aligned
	sfi	r9, r9, 0x4			# number bytes to align target	
	mtctr	r9				# use counter for loop
ofixup:
	lbzx	r7, r6, r3			# load byte
	stbx	r7, r6, r4			# and store it
	addi	r6, r6, 1			# update count
	bdnz	ofixup				# until done
onofixup:
	subfc	r7, r6, r11			# number bytes left to move
	andil.	r9, r7, 0x7			# number of scraps
	srwi	r10, r7, 0x3			# number of chunks to move
	mtctr	r10				# use counter for loop
	addi	r10, r6, 4			# psuedo counter for unrolled loop
	b	oloop				# skip no-ops
	.align	5
oloop:
	lwzx	r7, r6, r3			# load word
	lwzx	r8, r10, r3			# load word
	stwx	r7, r6, r4			# store word
	stwx	r8, r10, r4			# store word
	addi	r6, r6, 8			# update (real) counter
	addi	r10, r10, 8			# update (psuedo) counter
	bdnz+	oloop				# until done
	beq+	cr0, `MOVE_FINISHED'CLEVER_LABEL	# if no scraps
	mtctr	r9				# number of scraps
oscraps:
	lbzx	r7, r6, r3			# load byte
	stbx	r7, r6, r4			# store byte
	addi	r6, r6, 1			# update counter
	bdnz+	oscraps				# until done
	b	`MOVE_FINISHED'CLEVER_LABEL	# go to cleanup code
	
osmall:
	mtctr	r11				# use counter for loop
	b	osmall_loop			# skip no-ops
	.align 5
osmall_loop:
	lbzx	r7, r6, r3			# load byte
	stbx	r7, r6, r4			# store byte
	addi	r6, r6, 1			# update count
	bdnz+	osmall_loop			# until done
# NOTE -- fall thru to cleanup code
	
	BACK_COPY_CODE(`OUT', `COPY', isync)

#**********************************************************************
#
# 	FUNCTION: uiocopyin_ppc()
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

	.csect	uiocopyin_ppc[PR],5
	.globl	ENTRY(uiocopyin_ppc)
.uiocopyin_ppc:
	FRONT_COPY_CODE(`IN', `UIO', isync)
	cmpi	cr5, r11, 12			# check for small move
	andil.	r9, r4, 0x3			# check alignment of target
	blt-	cr5, uio_ismall			# go handle small move
	beq+	cr0, uio_inofixup		# branch if target aligned
	sfi	r9, r9, 0x4			# number bytes to align target	
	mtctr	r9				# use counter for loop
uio_ifixup:
	lbzx	r7, r6, r3			# load byte
	stbx	r7, r6, r4			# and store it
	addi	r6, r6, 1			# update count
	bdnz	uio_ifixup			# until done
uio_inofixup:
	subfc	r7, r6, r11			# number bytes left to move
	andil.	r9, r7, 0x7			# number of scraps
	srwi	r10, r7, 0x3			# number of chunks to move
	mtctr	r10				# use counter for loop
	addi	r10, r6, 4			# psuedo counter for unrolled loop
	b	uio_iloop			# skip no-ops
	.align	5
uio_iloop:
	lwzx	r7, r6, r3			# load word
	lwzx	r8, r10, r3			# load word
	stwx	r7, r6, r4			# store word
	stwx	r8, r10, r4			# store word
	addi	r6, r6, 8			# update (real) counter
	addi	r10, r10, 8			# update (psuedo) counter
	bdnz+	uio_iloop			# until done
	beq+	cr0, `MOVE_FINISHED'CLEVER_LABEL	# if no scraps
	mtctr	r9				# number of scraps
uio_iscraps:
	lbzx	r7, r6, r3			# load byte
	stbx	r7, r6, r4			# store byte
	addi	r6, r6, 1			# update counter
	bdnz+	uio_iscraps			# until done
	b	`MOVE_FINISHED'CLEVER_LABEL	# go to cleanup code
	
uio_ismall:
	mtctr	r11				# use counter for loop
	b	uio_ismall_loop			# skip no-ops
	.align 5
uio_ismall_loop:
	lbzx	r7, r6, r3			# load byte
	stbx	r7, r6, r4			# store byte
	addi	r6, r6, 1			# update count
	bdnz+	uio_ismall_loop			# until done
# NOTE -- fall thru to cleanup code
	
	BACK_COPY_CODE(`IN', `UIO', isync)
	
#****************************************************************************
#
#       FUNCTION: uiocopyout_ppc()
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

	.csect	uiocopyout_ppc[PR],5
	.globl	ENTRY(uiocopyout_ppc)
.uiocopyout_ppc:
	FRONT_COPY_CODE(`OUT', `UIO', isync)
	cmpi	cr5, r11, 12			# check for small move
	andil.	r9, r4, 0x3			# check alignment of target
	blt-	cr5, uio_osmall			# go handle small move
	beq+	cr0, uio_onofixup		# branch if target aligned
	sfi	r9, r9, 0x4			# number bytes to align target	
	mtctr	r9				# use counter for loop
uio_ofixup:
	lbzx	r7, r6, r3			# load byte
	stbx	r7, r6, r4			# and store it
	addi	r6, r6, 1			# update count
	bdnz	uio_ofixup			# until done
uio_onofixup:
	subfc	r7, r6, r11			# number bytes left to move
	andil.	r9, r7, 0x7			# number of scraps
	srwi	r10, r7, 0x3			# number of chunks to move
	mtctr	r10				# use counter for loop
	addi	r10, r6, 4			# psuedo counter for unrolled loop
	b	uio_oloop			# skip no-ops
	.align	5
uio_oloop:
	lwzx	r7, r6, r3			# load word
	lwzx	r8, r10, r3			# load word
	stwx	r7, r6, r4			# store word
	stwx	r8, r10, r4			# store word
	addi	r6, r6, 8			# update (real) counter
	addi	r10, r10, 8			# update (psuedo) counter
	bdnz+	uio_oloop			# until done
	beq+	cr0, `MOVE_FINISHED'CLEVER_LABEL	# if no scraps
	mtctr	r9				# number of scraps
uio_oscraps:
	lbzx	r7, r6, r3			# load byte
	stbx	r7, r6, r4			# store byte
	addi	r6, r6, 1			# update counter
	bdnz+	uio_oscraps			# until done
	b	`MOVE_FINISHED'CLEVER_LABEL	# go to cleanup code
	
uio_osmall:
	mtctr	r11				# use counter for loop
	b	uio_osmall_loop			# skip no-ops
	.align 5
uio_osmall_loop:
	lbzx	r7, r6, r3			# load byte
	stbx	r7, r6, r4			# store byte
	addi	r6, r6, 1			# update count
	bdnz+	uio_osmall_loop			# until done
# NOTE -- fall thru to cleanup code
	
	BACK_COPY_CODE(`OUT', `UIO', isync)

#**********************************************************************
#
# 	FUNCTION: uiocopyin_chksum_ppc()
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
#	uiocopyin_chksum_ppc(caddr_t src, caddr_t dest, int *p_nbytes,
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

# NOTE:	  Be careful with instruction that set XER(CA) as a
#	  side effect, such as addic, subfc, sfi, etc.
#	  The checksum algorithm uses the carry, and use
#	  of instructions that can generate or clear a
#	  carry, which are not part of the checksum computation,
#	  will introduce an error into the checksum computation.
	
	.csect	uiocopyin_chksum_ppc[PR],5
	.globl	ENTRY(uiocopyin_chksum_ppc)
.uiocopyin_chksum_ppc:
	FRONT_COPY_CODE(`IN', `UIOC', isync)
#
# copy src=r3, dest=r4, &nbytes=r5, count=r11
# note: during lsx/stsx r5 = &nbytes, r11=nbytes, r12 = saved TEMPSR
#       r13 = addr to store checksum, r14 = checksum,
#       r15 = 0 or 8 to indicate whether sum is shifted or not during loop
	mr	r13,r7				# r13=addr where to store cksum
	cmpi	cr5, r11, 12			# check for small move
	andil.	r9, r4, 0x3			# check alignment of target
	lil	r14, 0				# initialize checksum
	lil	r15, 0				# initialize shift
	aze	r7, r14				# clear carry
	blt-	cr5, uio_ismall_ck		# go handle small move
	beq+	cr0, uio_inofixup_ck		# branch if target aligned
	lil	r7, 0x4				# load constant
	subf	r9, r9, r7			# number bytes to align target
	mtctr	r9				# use counter for loop
	lil	r9, 0x0				# clear register
uio_ifixup_ck:
	lbzx	r7, r6, r3			# load byte
	stbx	r7, r6, r4			# and store it
	addi	r6, r6, 1			# update count
	rlinm	r9, r6, 3, 0x8			# was last move odd/even?
	rlnm	r7, r7, r9, 0xffff		# shift if last move was even
	ae	r14, r14, r7			# add to sum
	bdnz	uio_ifixup_ck			# until done
	aze	r14, r14			# add in carry
	rlinm	r15, r6, 3, 0x8			# set shift (r15) if odd bytes
	rlnm	r14, r14, r15, 0xffffffff	# shift if odd offset
uio_inofixup_ck:
	subf	r7, r6, r11			# number bytes left to move
	andil.	r9, r7, 0x7			# number of scraps
	srwi	r10, r7, 0x3			# number of chunks to move
	mtctr	r10				# use counter for loop
	addi	r10, r6, 4			# psuedo counter for unrolled loop
	b	uio_iloop_ck			# skip no-ops
	.align	5
uio_iloop_ck:
	lwzx	r7, r6, r3			# load word
	lwzx	r8, r10, r3			# load word
	ae	r14, r14, r7			# add to sum
	stwx	r7, r6, r4			# store word
	ae	r14, r14, r8			# add to sum
	stwx	r8, r10, r4			# store word
	addi	r6, r6, 8			# update (real) counter
	addi	r10, r10, 8			# update (psuedo) counter
	bdnz+	uio_iloop_ck			# until done
	aze	r14, r14			# add in carry
	beq+	cr0, uio_istore_checksum		# if no scraps
	mtctr	r9				# number of scraps
	lil	r9, 0x0				# clear register
	lil	r10, 0x1			# clear register
uio_iscraps_ck:
	lbzx	r7, r6, r3			# load byte
	rlinm	r9, r10, 3, 0x8			# was last move even/odd?
	stbx	r7, r6, r4			# store byte
	addi	r10, r10, 1			# keep track of odd/even
	rlnm	r7, r7, r9, 0xffff		# shift if last move was even
	addi	r6, r6, 1			# update counter
	ae	r14, r14, r7			# add to sum
	bdnz+	uio_iscraps_ck			# until done
	aze	r14, r14			# add in carry
	b	uio_istore_checksum		# go finish up
uio_ismall_ck:
	mtctr	r11				# use counter for loop
	lil	r9, 0x0				# clear register
	b	uio_ismall_loop_ck		# skip no-ops
	.align 5
uio_ismall_loop_ck:
	lbzx	r7, r6, r3			# load byte
	stbx	r7, r6, r4			# store byte
	addi	r6, r6, 1			# update count
	rlinm	r9, r6, 3, 0x8			# was last move odd/even?
	rlnm	r7, r7, r9, 0xffff		# shift if last move was even
	ae	r14, r14, r7			# add to sum
	bdnz+	uio_ismall_loop_ck		# until done
	aze	r14, r14			# add in carry
uio_istore_checksum:
	rlnm	r14, r14, r15, 0xffffffff	# shift again if needed
	rlinm	r16, r14, 16, 0xffff0000	# shift lower 2 bytes to upper
	a	r14, r14, r16			# add lower 2 to upper 2
	rlinm	r14, r14, 16, 0x0000ffff	# shit sum to lower 2 bytes
	aze	r14, r14			# add final carry to sum
	xoril	r14, r14, 0xffff		# one's complement sum
	st	r14, 0(r13)			# store final sum
# NOTE -- fall thru to cleanup code
	BACK_COPY_CODE(`IN', `UIOC', isync)

#****************************************************************************
#
#       FUNCTION: uiocopyout_chksum_ppc()
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
#	uiocopyout_chksum_ppc(caddr_t src,caddr_t dest,int *p_nbytes,int type)
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

	.csect	uiocopyout_chksum_ppc[PR],5
	.globl	ENTRY(uiocopyout_chksum_ppc)
.uiocopyout_chksum_ppc:
	FRONT_COPY_CODE(`OUT', `UIOC', isync)
#
# copy src=r3, dest=r4, count=r11
# note: r5 = &nbytes, r11=nbytes, r12 = saved TEMPSR
#       r13 = addr to store checksum, r14 = checksum,
#
	mr	r13,r7				# r13=addr where to store cksum
	cmpi	cr5, r11, 12			# check for small move
	andil.	r9, r4, 0x3			# check alignment of target
	lil	r14,0				# clear checksum
	lil	r15, 0				# initialize shift
	aze	r7, r14				# clear carry
	blt-	cr5, uio_osmall_ck		# go handle small move
	beq+	cr0, uio_onofixup_ck		# branch if target aligned
	lil	r7, 0x4				# load constant
	subf	r9, r9, r7			# number of bytes to align target
	mtctr	r9				# use counter for loop
	lil	r9, 0x0				# clear register
uio_ofixup_ck:
	lbzx	r7, r6, r3			# load byte
	stbx	r7, r6, r4			# and store it
	addi	r6, r6, 1			# update count
	rlinm	r9, r6, 3, 0x8			# was last move odd/even?
	rlnm	r7, r7, r9, 0xffff		# shift if last move was even
	ae	r14, r14, r7			# add to sum
	bdnz	uio_ofixup_ck			# until done
	aze	r14, r14			# add in carry
	rlinm	r15, r6, 3, 0x8			# set shift (r15) if odd bytes
	rlnm	r14, r14, r15, 0xffffffff	# shift if odd offset
uio_onofixup_ck:
	subf	r7, r6, r11			# number bytes left to move
	andil.	r9, r7, 0x7			# number of scraps
	srwi	r10, r7, 0x3			# number of chunks to move
	mtctr	r10				# use counter for loop
	addi	r10, r6, 4			# psuedo counter for unrolled loop
	b	uio_oloop_ck			# skip no-ops
	.align	5
uio_oloop_ck:
	lwzx	r7, r6, r3			# load word
	lwzx	r8, r10, r3			# load word
	ae	r14, r14, r7			# add to sum
	stwx	r7, r6, r4			# store word
	ae	r14, r14, r8			# add to sum
	addi	r6, r6, 8			# update (real) counter
	stwx	r8, r10, r4			# store word
	addi	r10, r10, 8			# update (psuedo) counter
	bdnz+	uio_oloop_ck			# until done
	aze	r14, r14			# add in carry
	beq+	cr0, uio_ostore_checksum	# if no scraps
	mtctr	r9				# number of scraps
	lil	r9, 0x0				# clear register
	lil	r10, 0x1			# clear register
uio_oscraps_ck:
	lbzx	r7, r6, r3			# load byte
	rlinm	r9, r10, 3, 0x8			# was last move even/odd?
	stbx	r7, r6, r4			# store byte
	addi	r10, r10, 1			# keep track of odd/even
	rlnm	r7, r7, r9, 0xffff		# shift if last move was even
	addi	r6, r6, 1			# update counter
	ae	r14, r14, r7			# add to sum
	bdnz+	uio_oscraps_ck			# until done
	aze	r14, r14			# add in carry
	b	uio_ostore_checksum		# go funish up
uio_osmall_ck:
	mtctr	r11				# use counter for loop
	lil	r9, 0x0				# clear register
	b	uio_osmall_loop_ck		# skip no-ops
	.align 5
uio_osmall_loop_ck:
	lbzx	r7, r6, r3			# load byte
	stbx	r7, r6, r4			# store byte
	addi	r6, r6, 1			# update count
	rlinm	r9, r6, 3, 0x8			# was last move odd/even?
	rlnm	r7, r7, r9, 0xffff		# shift if last move was even
	ae	r14, r14, r7			# add to sum
	bdnz+	uio_osmall_loop_ck		# until done
	aze	r14, r14			# add in carry
uio_ostore_checksum:
	rlnm	r14, r14, r15, 0xffffffff	# shift again if needed
	rlinm	r16, r14, 16, 0xffff0000	# shift lower 2 bytes to upper
	a	r14, r14, r16			# add lower 2 to upper 2
	rlinm	r14, r14, 16, 0x0000ffff	# shit sum to lower 2 bytes
	aze	r14, r14			# add final carry to sum
	xoril	r14, r14, 0xffff		# one's complement sum
	st	r14, 0(r13)			# store final sum
# NOTE -- fall thru to cleanup code
	BACK_COPY_CODE(`OUT', `UIOC', isync)
		
        .toc
        TOCE(kernel_lock, data)                 # toc entry for ext var

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
