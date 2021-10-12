# @(#)19	1.5  src/bos/kernel/ml/POWER/fmap_svc.m4, sysml, bos411, 9428A410j 2/4/94 18:16:08
#
#   COMPONENT_NAME: SYSML
#
#   FUNCTIONS: fmap_svc (fshmat)
#
#   ORIGINS: 27, 83
#
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1989, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#******************************************************************************
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************

ifdef(`_POWER_RS',`
	.machine "pwr"
#
# char *fshmat(fd,addr)
# int fd;
# char *addr;
#
# INPUT:
#   r3 = file descriptor of file to map
#   r4 = the high-order 4 bits identify which segment register
#	 to start mapping at
# 
# OUTPUT:
#   consecutive segment registers loaded in users address space
#   to map the requested file (up to 8 may be loaded)
#
#   on success:
#   r3 = the high-order 4 bits identify the segment register 
#	 at which to start the next mapping (i.e. the segment
#	 register following the mapping just completed)
#
#   on failure:
#   r3 = -1 and errno set as following:
#   EBADF  if bad file descriptor, file not open,
#	   or no shmat with FMAP option was performed previously
#   EINVAL if high-order bits of r4 specify segment register
#	   outside range sr3-sr12
#   ENOMEM if file cant be entirely mapped starting at the
#	   specified segment register
#
fmap_svc:
	l	r5,DATA(g_ksrval)	# get kernel segreg
	mfsr	r6,sr2			# get callers sreg 2
	l	r0,DATA(g_kxsrval)	# get kernext segreg
	mtsr	sr0,r5			# load kernel segreg
	rlinm	r6,r6,0,~sr_k_bit	# clear K bit
	mtsr	sr14,r0			# load kernext segreg
	mtsr	sr2,r6			# load new proc-private seg
	GET_CURTHREAD(cr0, r5, r6)	# get curthread pointer
	.machine "pwr"			# trashed by macro
	l	r5,t_userp(r6)		# get address of ublock
	mr	r0,r6			# keep curthread pointer
       .using   user,r5
        lhz     r6,u_maxofile		# get last open file descriptor + 1
        sli     r7,r3,3			# multiply arg file descriptor by 8
        cmpl    cr0,r3,r6		# compare arg fd with last open + 1
        cax     r7,r7,r5		# add addr of ublock to arg fd * 8
        bnl     cr0,badfd		# branch if arg descriptor too big
        l       r3,u_ufd_fp(r7)		# get file pointer from ufdtab
        l       r8,u_ufd_flags(r7)	# get flags with seg id from ufdtab
        cmpli   cr1,r3,0		# test for null file pointer
        beq     cr1,badfd		# branch if file is not open
getsid: rlinm.  r6,r8,24,0xfffff	# extract segment id as sreg value
        bz      badfd			# branch if segment id not in ufdtab
        rlinm   r3,r4,6,15*4		# multiply arg sreg number by 4
        cmpli   cr1,r3,3*4		# compare it with minimum loadable
        rlimi   r6,r8,31,sr_k_bit	# extract insert K into sreg value
        blt     cr1,badsr		# branch if arg sreg too small
        cmpli   cr6,r3,13*4		# compare it with shared text sreg
        cax     r5,r5,r3		# add it to address of ublock
        bnl     cr6,badsr		# branch if arg sreg too big
        rlinm.  r7,r8,4,7		# extract maximum for sid bits 9-11
        st      r6,u_adspace_sr		# store sreg value in ublock
        mtsri   r6,0,r4			# load sreg value into arg sreg
        bnz     moresr			# branch if more than 1 sreg needed
return:	mr	r6,r0			# get back address of curthread
	l	r9,t_userp(r6)		# get address of ublock
	l	r6,u_adspace_sr+0*4(r9)	# get user segment 0
	l	r7,u_adspace_sr+2*4(r9)	# get user segment 2
	l	r8,u_adspace_sr+14*4(r9)# get user segment 14
	mtsr	sr0,r6
	mtsr	sr2,r7
	mtsr	sr14,r8
        cau     r3,r4,0x1000		# return next sreg to load
        rfsvc				# return to caller
 
moresr: mfctr   r8			# save svc return msr from ctreg
        mtctr   r7			# number more sregs needed to ctreg
nextsr: cau     r4,r4,0x1000		# increment sreg number
        rlinm   r3,r4,6,15*4		# isolate sreg number
        cmpli   cr1,r3,13*4		# compare it with shared text sreg
        cal     r5,4(r5)		# increment ublock address
        cau     r6,r6,0x10		# incr sid bits 9-11 in sreg value
        beq     cr1,nofit		# branch if next sreg is shared text
        st      r6,u_adspace_sr		# store sreg value in ublock
        mtsri   r6,0,r4			# load sreg value into next sreg
        bdn     nextsr			# decr, loop if more sreg(s) needed
        mtctr   r8			# reload svc return msr into ctreg
        b       return			# loop to test and return next sreg
       .drop    r5
 
nofit:	mtctr	r8			# reload svc return msr into ctreg
        cal     r4,ENOMEM(0)		# file wont fit
	b	reterr
badsr:  cal     r4,EINVAL(0)		# bad arg sreg
	b	reterr
badfd:  cal     r4,EBADF(0)		# bad arg fd

reterr:	mr	r6,r0			# get back address of curthread
	l	r5,t_uthreadp(r6)	# get address of uthread
	l	r5,ut_errnopp(r5)	# get address of pointer to errno
	l	r9,t_userp(r6)		# get address of ublock
	l	r6,u_adspace_sr+0*4(r9)	# get user segment 0
	l	r7,u_adspace_sr+2*4(r9)	# get user segment 2
	l	r8,u_adspace_sr+14*4(r9)# get user segment 14
	mtsr	sr0,r6
	mtsr	sr2,r7
	mtsr	sr14,r8
	cal	r3,-1(0)		# return -1
	l	r5,0(r5)		# get address of errno
	st	r4,0(r5)		# set errno
        rfsvc                           # return to caller

	.machine "com"
',)
