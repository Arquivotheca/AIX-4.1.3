# @(#)91	1.15  src/bos/kernel/ml/POWER/copyx.s, sysml, bos41J, 9518A_all 5/2/95 08:52:21
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: copyinstr()
#
# ORIGINS: 27 83
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
        .file   "copyx.s"
        .machine "com"                          # Use common instruction set
include(systemcfg.m4)
include(macros.m4)
include(except.m4)

	.set    CHUNK, 16                       # bytes to move per chunk
        .set    L2CHUNK, 4                      # log base 2 of CHUNK
	.set	STKMIN,	96			# minimum stack


#****************************************************************************
#
#       FUNCTION: copyinstr()
#
#       PURPOSE: Copies the contents of memory from
#                a user/system  address to
#                a trusted system address for a given number of bytes
#                or until 0 is found.
#
#       INPUTS:  r3 -> source address
#                r4 -> destination address
#                r5 -> max number of bytes to copy
#		 r6 -> address to return actual count of bytes copied
#
#       NOTES:   1. This code is processor architecture independent.
# 		 2. The following labels are used to "noop"
#                   isync instructions for PPC (603/604)
#                   isync_copyinstr1
#                   isync_copyinstr2
#		In order to guarantee integrity on PPC processors
#		post 601, the isync's should be on both sides
#		of the mtsr/mfsr.  However, since this is being
#		called from the kernel and for user mode a
#		scratch segment register is used, only one
#		isync is needed.
#
#		This program uses memccpy in libcsys to accomplish
#		the actual copy.  Some information has to be
#		saved on the stack prior to the call to memccpy.
#		If an exception occurs, it is assumed that memccpy
#		will not have modified the stack pointer.  Further,
#		any changes to memccpy which would change this must
#		be coordinated with the owner of this module.
#
#       PSEUDOCODE:
#copyinstr(caddr_t src, caddr_t dest, int max_bytes, int *ptr)
#{
#	/* check for negative byte counts
#	 */
#	if (max_bytes =< 0)
#		return(E2BIG);
#
#	/* copy it in
#	 */
#	/* take short path if the following are true
#	   1. the source does not cross segment boundary
#	   2. kernel lock is not held
#	   3. process is single threaded or source in PRIVSEG
#	   call long_copyinstr() otherwise */
#
#	*ptr = max_bytes;	/* initialize return to max */
#	if ((((uint)src >> L2SSIZE) != (((uint)src + nbytes -1) >> L2SSIZE))
#              || IS_LOCKED(&kernel_lock)  || 
#		(CURPROC->p_active > 1 && ((uint)src >> L2SSIZE) != PRIVSEG))
#
#		return(long_copyinstr(src, dest, maxbytes, *ptr));
#
#	else {  /* short path copy */
#		
#		if (not kernel thread)
#			use temporary seg reg
#		/* set up exception handler (excbranch in mst) */
#		mstp->excbranch = &excp_hand
#		buy a stack frame;
#		save volatile information on the stack;
#		rc=memccpy(dest,src,0,max_bytes);
#		if (rc == 0)
#		{
#			set return code = E2BIG /* null not found */;
#		}
#		else
#		{
#			compute number bytes moved;
#			set return code = successful;
#		}	
#		restore data from stack;
#		/* clear exception handler */
#		mstp->excbranch = 0;
#		if (not kernel thread)
#			restore temporary seg reg
#		return
#	}
#
#	excp_hand:
#		if (not kernel thread)
#			restore temp seg reg
#		/* exception value is in r3 */
#		rc = (exval > EXCEPT_ERRNO) ? EFAULT : exval;
#		return rc
#}
#****************************************************************************

        .machine "com"                          # Use common instruction set

        .csect  copyinstr[PR],5
        .globl  ENTRY(copyinstr)

.copyinstr:
        LTOC(r10,kernel_lock,data)              # load kernel_lock addr -> r10
        GET_CURTHREAD(cr1,r8,r7)                # get ptr to current thread->r7
        l       r10,0(r10)                      # get *kernel_lock -> r10
	l	r11,t_procp(r7)			# get proc ptr -> r11
        l       r8,t_tid(r7)                    # get thread id -> r8

#
# find if user or kernel proc  - type
#
        l       r12,t_flags(r7)                 # get thread flags -> r12
        cmp     cr1,r8,r10                      # is kernel_lock(ed)
        rlinm.  r10,r12,0,TKTHREAD              # user(=0) or kernel(!=0)
#
# Check if single or multi threaded and source in PRIVSEG
#
	lhz	r11,p_active(r11)		# get number of threads
	l	r9,t_userp(r7)			# get u_block ptr
	cmpi	cr6,r11,1			# single threaded ?
        ai      r11,r5,-1                       # max = max - 1
        cror    cr7*4+eq, cr0*4+eq, cr0*4+eq    # move cr0 eq to cr7
	rlinm	r8,r3,4,0x0000000f		# get segreg for source
	cmpi	cr0,r8,r_user			# compare segreg with PRIVSEG
        cror    cr6*4+eq, cr0*4+eq, cr6*4+eq    # combine tests
	bne-	cr6,long_call			# branch if multithreaded
						#   and not in PRIVSEG
        a       r11,r11,r3                      # add max-1 +src
        beq-    cr1,long_call                   # if kernel_lock(ed) call c-code
#
# Check if source spans more than one segment
#
        xor   	r11,r3,r11  	                # determine if seg reg changes 
        andiu.  r11,r11,0xf000                  # compare  seg regs
        lil     r11,TEMPSR			# scratch seg reg  # in r11
        cal     r9,u_adspace_sr(r9)             # get seg reg array
        bne-    long_call                       # if spans > 1 seg call c-code
        rlinm   r7, r3,6, 0x0000003C            # multiply  sreg number by 4
                                                # to get offset in seg reg array
        bne-    cr7,start_copy                  # branch if kernel thread
        rlimi   r3,r11,28,0xF0000000            # put seg reg num in hi nibble

        lx      r7,r9,r7                        # load seg reg val of src
        mfsr    r12,TEMPSR                      # save temp seg reg val
        mtsr    TEMPSR, r7                      # Load temp segment register
                                                # with source seg reg val
DATA(isync_copyinstr1):				# at si time, isync is nop'ed
						# for rs1, rs2, rsc, and 601.
	.globl	DATA(isync_copyinstr1)
	isync

start_copy:
        LTOC(r0,cstr_iexcbch,data)   # get address of exception handler
        .using  mstsave,r7
        GET_CSA(cr0, r8, r7)    # address of current mst
        cmpi    cr0,r5,0        # insure number bytes is gt zero
        st      r0,excbranch    # On exception, branch to this location with
                                # r3 set to exception value.
        .drop   r7
        st      r5,0(r6)        # set number of bytes copied to max
        ble-    cstr_ierout          # branch if number of bytes le zero
#
# now copy(src r3, dest r4,max r5) update *r6 with how many moved
#
        mflr    r0                              # Save link reg
	st	r6,-20(r1)			# save actual for later
        st      r0, stklink(r1)                 # Store return addr to caller
        st      r4,-4(r1)	                # put target on stack
	st	r10,-8(r1)			# save user/kernel flag
	st	r12,-12(r1)			# save seg reg value
	st	r7,-16(r1)			# save mst pointer
#
# memccpy expects r3 to point to the target
#
	mr	r4,r3				# source to r4 for memccpy
	mr	r6,r5				# max to r6
	lil	r5,0				# search char is null
	l	r3,-4(r1)			# load  target
	bl	ENTRY(memccpy)			# branch to memccpy
	.extern	ENTRY(memccpy)
#
#	Now must adjust return from memccpy.
#
	cmpi	cr0,r3,0			# check if okay
	l	r5,-4(r1)			# get original target address
	l	r6,-20(r1)			# addr to return actual bytes 
	beq-	nullnotfound			# branch if null not found
#
#	If here, string was null terminated.  Must compute number chars moved. 
#
	sf	r5,r5,r3			# compute actual number moved
	st	r5,0(r6)			# return
	lil	r3,0				# set rc to 0

nullfound:
	l	r0,stklink(r1)			# Reload return address
	l	r10,-8(r1)			# get user/kernel flag
	l	r12,-12(r1)			# get saved seg reg value
	l	r7,-16(r1)			# get mst pointer
	mtlr	r0				# Return addr to LR
nullfound10:
        .using  mstsave,r7
	cmpi	cr0,r10,0			# check if user/kernel
        lil     r4,0            # to clear exception branch
        st      r4,excbranch    # clear exception branch in MST
        .drop   r7
	bnelr-	cr0				# exit - kernel
#
#	If here, this is user - must restore seg reg
#
DATA(isync_copyinstr2):				# at si time, isync is nop'ed
						# for rs1, rs2, rsc, and 601.
	.globl	DATA(isync_copyinstr2)
	isync

        mtsr    TEMPSR, r12
	br					# exit
#
#	If here, null character was not found
#
nullnotfound:
	lil	r3,E2BIG			# set for null not found
	b	nullfound
#
#  exception handler.  r3 = exception code
#
cstr_iexcbch:
        cmpi    cr0,r3,EXCEPT_ERRNO             # check for valid errno
        ble+    cr0,nullfound
        lil     r3,EFAULT                       # if invalid, set to EFAULT
	b	nullfound
#
#	If here, invalid length
#
cstr_ierout:
	lil	r3,E2BIG			# set for no null
	b	nullfound10
#
#   if the move may be complex, we call long_copyinstr to do the whole job.
#   to prepare for this, we have left all the registers that matter
#   (r3, r4, r5, r6, lr) the same as we were called, so we can just branch
#   directly to long_copyinstr
#
long_call:
        b       ENTRY(long_copyinstr)           # branch to long_copyinstr
        .extern ENTRY(long_copyinstr)
        S_EPILOG

        .toc

       .machine "com"
        TOCL(cstr_iexcbch, data)                # toc entry for label
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
