# @(#)63	1.2  src/bos/kernel/ml/POWER/copyx_macros.m4, sysml, bos41J, 9523A_all 6/2/95 16:10:15
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS:
#	FRONT_COPY_CODE, BACK_COPY_CODE
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#*****************************************************************************

# These macros are used to create front-end and back-end code for routines
# which perform copyin/copyout like memory movement with an exception
# handler.  The front end checks that the code can use the assembler
# fast path, otherwise it calls vmcopyin or vmcopyout as appropriate.
# If necessary it sets up a segment register for the source/target
# (whichever is NOT the trusted address) and finally sets up an 
# exception handler.
#
# BEFORE the front-end macro the following registers must be set
# up.  It is strongly suggested that the interface to new routines
# be designed so that the arguments are set up in this fashion:
#
# r3 == address of source
# r4 == address of target
# r5 == address of number of bytes
# r6 == 0 for user space, 1 for kernel space
#
# On exit from the front macro, the registers are as follows:
#
# r3, r4, r7 == unchanged
# r6 == 0 (to be used for count of number bytes moved)
# r8, r9, r10 == killed
# r11 == number of bytes to move
# r12 == contents of user segment register, if user space
# cr0, cr1, cr6 == killed
# cr 7 == indicates user/kernel space
#
# In addition, for $2 = UIO, on exit r5 is the CSA address.
#
# After the setup macro follows the code to perform the move.
# After the move is complete, branch to the following label:
#	b	`MOVE_FINISHED'CLEVER_LABEL
# or just fall through to the first instruction in 
# BACK_COPY_CODE.
#
# On Entry to the BACK_COPY_CODE macro the following registers
# should be set up:
#
# r4 -- address of target
# r6 -- bytes already copied (for IN operations)
# cr7 -- eq set for user space, eq off for kernel space
#
# The BACK_COPY_CODE will always return to caller, so no code
# should follow it.

	.set	STKMIN,	96			# minimum stack

##############################################################################
# CLEVER_LABEL is incremented each time the macro is used in order
# to generate unique labels.

define(`CLEVER_LABEL', 0)

##############################################################################

# $1 == "IN" (for copyin) or "OUT" (for copyout)
# $2 == "COPY" (for vanilla copyin/copyout)
#    == "UIO" (for versions of uio copy)
#    == "UIOC" (for versions of uio copy w/ checksum)
# $3 == "isync" (if needed), else null

define(`FRONT_COPY_CODE',`
	ifdef(`COPY_CODE_MACRO_LOCK',
`BreakTheBuild # A previous `FRONT_COPY_CODE' had no matching `BACK_COPY_CODE' ')
	define(`COPY_CODE_MACRO_LOCK')
	define(`CLEVER_LABEL', incr(CLEVER_LABEL))

ifdef(`_MACRO_COPY', `undefine(`_MACRO_COPY')')
ifdef(`_MACRO_UIO', `undefine(`_MACRO_UIO')')
ifdef(`_MACRO_CHECKSUM', `undefine(`_MACRO_CHECKSUM')')

# check that values supplied for $1 and $2 are valid
ifelse($1, IN,  `' , $1, OUT, `', `BreakTheBuild # $1 must be "IN" or "OUT"')

ifelse($2, COPY, `define(`_MACRO_COPY')' , 
       $2, UIO,  `define(`_MACRO_UIO')' ,
       $2, UIOC, `define(`_MACRO_UIO')
		  define(`_MACRO_CHECKSUM')' ,
	 `BreakTheBuild # $2 must be "COPY" or "UIO" or "UIOC"')

ifdef(`_MACRO_UIO', `
	l r11, 0(r5)				# load nbytes
')
ifdef(`_MACRO_COPY', `
	mr	r11, r5				# move nbytes to expected register
')
	LTOC(r8,kernel_lock,data)		# load kernel_lock addr
	cmpi	cr0,r11,0			# compare count to 0
        GET_CURTHREAD(cr1,r10,r9)		# get ptr to current thread 
	ble-	cr0,`ERROR_PATH_'CLEVER_LABEL	# branch if count < 0
# check if kernel lock held
	l	r8,0(r8)			# get *kernel_lock
	l	r10,t_tid(r9)			# get thread id
	cmp	cr6,r8,r10			# is kernel locked?
	l	r8,t_procp(r9)			# get proc ptr
ifdef(`_MACRO_COPY', `
	l	r12, t_flags(r9)		# get thread_flags
	rlinm.	r6, r12, 0, TKTHREAD		# user (=0) or kernel (=1)
	cror	cr7*4+eq, cr0*4+eq,cr0*4+eq	# copy cr0 eq to cr7
')
ifdef(`_MACRO_UIO', `
	cmpi	cr7,r6,0			# user(=0) or kernel(!=0)
')
	beq-	cr6,`SLOW_PATH_'CLEVER_LABEL	# branch if hold kernel_lock
# check if single-threaded or src in PRIVSEG
	lhz	r8,p_active(r8)			# get number of threads
	ai	r10,r11,-1			# count-1
	cmpi	cr0,r8,1			# single-threaded?
ifelse($1, IN,`
	rlinm	r8,r3,4,0x0000000f		# get segreg of src
',`
	rlinm	r8,r4,4,0x0000000f		# get segreg of dest
')
	cmpi	cr6,r8,r_user			# compare segreg with PRIVSEG
        cror    cr0*4+eq, cr0*4+eq, cr6*4+eq    # combine tests

ifelse($1, IN,`
	a	r10,r10,r3			# add count-1 + src
',
	$1, OUT, `
	a	r10,r10,r4			# add count-1 + dest
')
	bne-	cr0,`SLOW_PATH_'CLEVER_LABEL	# branch if multithreaded
						#   and not PRIVSEG
# check if crosses seg boundary
	rlinm	r10,r10,4,0x0000000f		# get segreg of (src+count-1)
ifelse($2, COPY, `
')
	cmp	cr0,r8,r10
	l	r9,t_userp(r9)			# get u_block ptr
	bne-	cr0,`SLOW_PATH_'CLEVER_LABEL	# branch if spans > 1 seg
# seg reg stuff
	bne-	cr7,`NO_SEGREG_'CLEVER_LABEL	# branch if kernel thread
	lil	r10,TEMPSR			# load scratch sreg #
	cal	r9,u_adspace_sr(r9)		# get seg reg array
ifelse($1, IN,`
	rlinm	r8,r3,6,0x0000003c		# mult sreg num by 4 to get
						# offset in sreg array
	rlimi	r3,r10,28,0xF0000000		# put sreg num in hi nibble
',`
	rlinm	r8,r4,6,0x0000003c		# mult sreg num by 4 to get
						# offset in sreg array
	rlimi	r4,r10,28,0xF0000000		# put sreg num in hi nibble
')

	lx	r8,r9,r8			# load sreg value of src
	mfsr	r12,TEMPSR			# save temp sreg
	mtsr	TEMPSR,r8			# load temp sreg
	$3					# isync if necessary
ifdef(`_MACRO_COPY', `
	define(`COPY_CSA_REG', r5)
')
ifdef(`_MACRO_UIO', `
	define(`COPY_CSA_REG', r8)
')
`NO_SEGREG_'CLEVER_LABEL:	
	GET_CSA(cr5,r9,COPY_CSA_REG)			# get mst ptr
	LTOC(r10,`EXCEPTION_HANDLER_'CLEVER_LABEL,data)	# addr of exception handler
	lil	r6,0				# set r6=0 in case of fault
	st	r10,excbranch(COPY_CSA_REG)		# set up exception handler

ifdef(`_MACRO_CHECKSUM', `
#
#  Save volatile regs needed for checksum (r13-r16).
#
	st	r13, -4(r1)			# save r13 on callers stack
	st	r14, -8(r1)			# save r14 on callers stack
	st	r15, -12(r1)			# save r15 on callers stack
	st	r16, -16(r1)			# save r16 on callers stack
')

ifdef(`_MACRO_COPY', `undefine(`_MACRO_COPY')')
ifdef(`_MACRO_UIO', `undefine(`_MACRO_UIO')')
ifdef(`_MACRO_CHECKSUM', `undefine(`_MACRO_CHECKSUM')')
')

##############################################################################
# On entry:
#	r3 -- address of source
#	r4 -- address of target
#	r11 -- number of bytes to copy
#	cr7(eq) -- on for user segment, off for kernel segment

define(`BACK_COPY_CODE',`
	ifdef(`COPY_CODE_MACRO_LOCK',` ',
`BreakTheBuild # No previous `FRONT_COPY_CODE' macro used ')
	undefine(`COPY_CODE_MACRO_LOCK')

ifdef(`_MACRO_COPY', `undefine(`_MACRO_COPY')')
ifdef(`_MACRO_UIO', `undefine(`_MACRO_UIO')')
ifdef(`_MACRO_CHECKSUM', `undefine(`_MACRO_CHECKSUM')')

# check that values supplied for $1 and $2 are valid
ifelse($1, IN,  `' , $1, OUT, `', `BreakTheBuild # $1 must be "IN" or "OUT"')
ifelse($2, COPY, `define(`_MACRO_COPY')' , 
       $2, UIO,  `define(`_MACRO_UIO')' ,
       $2, UIOC, `define(`_MACRO_UIO')
		  define(`_MACRO_CHECKSUM')' ,
	 `BreakTheBuild # $2 must be "COPY" or "UIO" or "UIOC"')

`MOVE_FINISHED'CLEVER_LABEL:
ifdef(`_MACRO_COPY', `
	define(`COPY_CSA_REG', r5)
')
ifdef(`_MACRO_UIO', `
	define(`COPY_CSA_REG', r8)
	GET_CSA(cr5,r9,COPY_CSA_REG)			# get mst ptr
')
	lil	r3,0
	st	r3,excbranch(COPY_CSA_REG)		# clear exception handler
ifdef(`_MACRO_CHECKSUM', `
# restore non-volatile registers
	l	r13, -4(r1)			# restore r13
	l	r14, -8(r1)			# restore r14
	l	r15, -12(r1)			# restore r15
	l	r16, -16(r1)			# restore r16
')

	bnelr-	cr7				# seg reg manipulation?
	$3					# isync if necessary
	mtsr	TEMPSR,r12			# Load temp segment register 
	br					# return to caller      

# count is either 0 (return ok) or negative (return EINVAL)
`ERROR_PATH_'CLEVER_LABEL:
        cmpi    cr0,r11,0                       # Is count = 0 
        lil     r3,EINVAL                       # load EINVAL as ret val
        bnelr-	cr0

# count=0, just return ok
        lil     r3,0                            # zero count OK
        br

# exception handler
#
#   r3=exception code
#   r4=dest
#   r5=&nbytes
#   r6=num bytes moved successfully (for "IN") case
#   r11=nbytes
#   r12=saved TEMPSR
#
`EXCEPTION_HANDLER_'CLEVER_LABEL:
ifdef(`_MACRO_UIO', `
	GET_CSA(cr5,r9,r8)			# get mst ptr
	rlinm	r7,r4,0,0xf0000000		# get sreg num of dest
	l	r9,mstovaddr(r8)		# load addr that caused except.
	rlinm	r10,r9,0,0xf0000000		# get sreg num
ifelse(IN, $1, `
# For IN case, exception must have been in destination
# segment.  If not trap.
	teq	r7,r10				# assert exception sreg!dst sreg
',`
# For OUT case, exception may have been either source or
# target segment -- figure out which.  Actually, the source
# address should be trusted, but it was discovered that
# some application (crash?) was loading a seg register for
# source and not checking it, and when it got an exception
# on the source it would crash the machine.
	cmp	cr0, r7, r10			# occured or source or dest?
	beq	`EXC_ON_DEST_'CLEVER_LABEL	# branch if on destination
	l	r4, mstoarg1(r8)		# get src address from mst
`EXC_ON_DEST_'CLEVER_LABEL:
	sf	r6, r4, r9			# compute number bytes moved
')
	st	r6,0(r5)			# store num bytes moved
')
	cmpi	cr0,r3,EXCEPT_ERRNO		# check for valid errno
	ble+	cr0,`RETURN_2_'CLEVER_LABEL	# branch if errno already good
	lil	r3,EFAULT			# if invalid, set to EFAULT
`RETURN_2_'CLEVER_LABEL:
ifdef(`_MACRO_CHECKSUM', `
# restore non-volatile registers
	l	r13, -4(r1)			# restore r13
	l	r14, -8(r1)			# restore r14
	l	r15, -12(r1)			# restore r15
	l	r16, -16(r1)			# restore r16
')
	bnelr-	cr7
	$3					# isync if necessary
	mtsr	TEMPSR,r12			# Load temp segment register 
	br					# return to caller      

ifdef(`_MACRO_COPY', `
define(`COPY_STACKSIZE', 8+STKMIN)
')
ifdef(`_MACRO_UIO', `
define(`COPY_STACKSIZE', STKMIN)
')

# fastpath will not  work, call vmcopyin or vmcopyout
`SLOW_PATH_'CLEVER_LABEL:
ifdef(`_MACRO_CHECKSUM', `
	oris	r8, r8, 0xffff			# generate invalid checksup
	st	r8, 0(r7)			# store invalid checkum
')
        mflr    r0				# Save link reg
        st      r0,stklink(r1)			# Store return addr to caller
        stu     r1,-(COPY_STACKSIZE)(r1)	# Back up the stack pointer
						# (buy stack frame)
ifdef(`_MACRO_COPY', `
# vmcopyin/vmcopyout need r6=0 for user or r6=1 for kthread,
# and r5 is the address of the count, not the count itself.
	st	r5, STKMIN(r1)			# put count on stack
	cal	r5, STKMIN(r1)			# load address of count
	lil	r6, 1				# 1 for kthread
	bne-	cr7, `COPY_KTHREAD'CLEVER_LABEL	# skip if kthread
	lil	r6, 0				# 0 for user proc
`COPY_KTHREAD'CLEVER_LABEL:
')
ifelse(IN, $1, `
	bl	ENTRY(vmcopyin)			# branch to vmcopyin
	.extern ENTRY(vmcopyin)
',`
	bl	ENTRY(vmcopyout)		# branch to vmcopyout
	.extern ENTRY(vmcopyout)
')
	l	r0, (COPY_STACKSIZE+stklink)(r1)	# Reload return address
	mtlr	r0				# Return addr to LR
	cal	r1, (COPY_STACKSIZE)(r1)	# Pop the stack frame
	S_EPILOG
	.toc
        TOCL(`EXCEPTION_HANDLER_'CLEVER_LABEL, data)	# toc entry for label

ifdef(`_MACRO_COPY', `undefine(`_MACRO_COPY')')
ifdef(`_MACRO_UIO', `undefine(`_MACRO_UIO')')
ifdef(`_MACRO_CHECKSUM', `undefine(`_MACRO_CHECKSUM')')
')
