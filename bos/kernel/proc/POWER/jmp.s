# @(#)38	1.14  src/bos/kernel/proc/POWER/jmp.s, sysproc, bos411, 9428A410j 2/14/94 14:29:45
#
#   COMPONENT_NAME: SYSPROC
#
#   FUNCTIONS:  clrjmpx
#		setjmpx
#		
#   ORIGINS: 27, 83
#
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1985, 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************

	.file	"jmp.s"
	.machine "com"

include(systemcfg.m4)
include(macros.m4)

#-------------------------------------------------------------------
# Subroutine Name: setjmpx
#
# Function: Kernel version of setjmp.
#
#	Input:	r3 -> label_t for jump buffer
#
#	Output:	r3 = 0
#
#-------------------------------------------------------------------

        .set    jmpprev, 0      # chain to previous jump buffer
	.set	jmpiar, 4	# return address offset in jump buffer
	.set	jmpstk, 8	# stack pointer offset in jump buffer
	.set	jmptoc, 12	# TOC pointer offset in jump buffer
        .set    jmpcr, 16       # condition register offset in jump buffer
	.set	jmpintpri, 20   # interrupt priority offset in jump buffer
	.set	jmpregs, 24	# registers offset in jump buffer


	S_PROLOG(setjmpx)

	.using	low, r0
	liu	r6, LOCKL_OWNER_MASK>16	# get upper half of owner mask
	GET_CURTHREAD(cr0, r4, r5)	# address of our thread block
        LTOC(r4,kernel_lock,data)       # address of kernel-mode-lock
	oril	r6, r6, (LOCKL_OWNER_MASK & 0xffff) # get lower half
	l	r5, t_tid(r5)		# our thread id
        l       r4, 0(r4)               # contents of kernel-mode-lock
	and	r4, r4, r6		# get lock owner
        cmp     cr0, r4, r5             # compare; record result in
                                        #   field 0 of cr
        mfcr    r5                      # move all cr fields to r5
	mflr	r4                      # move return address to r4
	st	r1, jmpstk(r3)          # store stack pointer
	st	r2, jmptoc(r3)          # store toc pointer
	st	r4, jmpiar(r3)          # store return address
        st      r5, jmpcr(r3)           # store cr fields
	stm	r13, jmpregs(r3)        # store non-volatile regs
	GET_CSA(cr0, r5, r4)		# current save area
	.using  mstsave, r4

        l       r5, mstkjmpbuf          # load ptr to top of stack
        st      r3, mstkjmpbuf          # store new top of stack
        st      r5, jmpprev(r3)         # store back chain
	lbz	r5, mstintpri		# get interrupt priority
	st	r5, jmpintpri(r3)	# store interrupt priority

	cal	r3, 0(0)                # return code = 0

	S_EPILOG
	FCNDES(setjmpx)

#-------------------------------------------------------------------
# Subroutine Name: clrjmpx
#
# Function: Remove top entry from exception stack ancored in u-block
#
#	Input:  r3 -> label_t for jump buffer
#
#	Output: None
#
#------------------------------------------------------------------

	S_PROLOG(clrjmpx)
	GET_CSA(cr0, r5, r4)		# get current mst
	l	r5, mstkjmpbuf(r4)	# get the current exception buffer
	cmp	cr0, r3, r5		# check that we are removeing the
					#  top exception buffer
	l	r10, jmpprev(r3)	# get the next exception buff
	bne	cr0, bad_jmpbuf		# if we are not removing the top
					#  exception buffer then panic

	st	r10, mstkjmpbuf(r4)	# make the it the top buff. ie pop
	br				# return

bad_jmpbuf:
	TRAP

	FCNDES(clrjmpx)

#-------------------------------------------------------------------
# Subroutine Name: longjmp
#
# Function: Kernel version of longjmp.
#
#	Input:	r3 -> label_t for jump buffer
#		r4 = return code for caller of setjmp
#
#	Output:	r3 = return code passed by caller, if not 0
#		   = 1 if caller tried to return 0
#
#-------------------------------------------------------------------

	S_PROLOG(longjmp)

        l       r5, jmpcr(r3)
        mtcr    r5
	cmpi	cr0, r4, 0
	l       r5, jmpiar(r3)
	l       r1, jmpstk(r3)
	l       r2, jmptoc(r3)
	mtlr	r5
	lm      r13, jmpregs(r3)
	mr	r3, r4
	bner
	cal	r3, 1(0)

	S_EPILOG
	FCNDES(longjmp)

#
#  NAME: fst_halt_display
#                                                                    
#  FUNCTION: calls halt_display on a fixed stack
#
#  NOTES:
#	used to avoid having a dump take place on a stack frame
#	which has useful data on it, as in the case of double longjmpx()
#
#	fst_halt_display(
#
# 	Buys an MST;
#	jumps to halt_display() which does not return
# 
#  RETURN VALUE DESCRIPTION: none
# 
	S_PROLOG(fst_halt_display)
	
        .extern ENTRY(buy_mstsave)
	bl	ENTRY(buy_mstsave)	# get new mst

        .extern ENTRY(halt_display)
	b	ENTRY(halt_display)	# jump to halt_display (no return)

	S_EPILOG
	FCNDES(fst_halt_display)


#------------------------#
#   include files
#------------------------#
include(param.m4)
include(mstsave.m4)
include(low_dsect.m4)
include(intr.m4)
include(proc.m4)
include(user.m4)
include(machine.m4)
include(scrs.m4)
include(lock_def.m4)

        .toc
        TOCE(kernel_lock,data)
	TOCE(i_data,data)
