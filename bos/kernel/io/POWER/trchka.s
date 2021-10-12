# @(#)52        1.27  src/bos/kernel/io/POWER/trchka.s, systrace, bos41J, 9521A_all 5/23/95 13:58:57
# COMPONENT_NAME: SYSTRACE  /dev/systrace pseudo-device driver
# 
# FUNCTIONS: trchook
#            trchk, trchkt, trchkl, trchklt, trchkg, trchkgt
#            trc_timestamp
# 
# ORIGINS: 27 83
# 
#  This module contains IBM CONFIDENTIAL code. -- (IBM
#  Confidential Restricted when combined with the aggregated
#  modules for this product)
#                   SOURCE MATERIALS
#  (C) COPYRIGHT International Business Machines Corp. 1985, 1993
#  All Rights Reserved
# 
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# LEVEL 1,  5 Years Bull Confidential Information
#
# 
        .file   "trchka.s"

ifdef(`_POWER_MP',`
include(../../kernel/ml/POWER/low_dsect.m4)
',` #else _POWER_MP
include(low_dsect.m4)
')  #endif _POWER_MP
include(scrs.m4)
ifdef(`_POWER_MP',`
include(../../kernel/ml/POWER/seg.m4)
',` #else _POWER_MP
include(seg.m4)
')  #endif _POWER_MP
include(machine.m4)
include(proc.m4)
ifdef(`_POWER_MP',`
include(../../kernel/ml/POWER/mstsave.m4)
',` #else _POWER_MP
include(mstsave.m4)
')  #endif _POWER_MP
include(i_machine.m4)
ifdef(`_POWER_MP',`
include(../../kernel/io/POWER/trchdr.m4)
include(../../kernel/io/POWER/trchk.m4)
',` #else _POWER_MP
include(trchdr.m4)
include(trchk.m4)
')  #endif _POWER_MP
include(../../kernel/ml/POWER/systemcfg.m4)
include(../../kernel/ml/POWER/macros.m4)

# WARNING:
# If you modify this file, it may be necessary to modify
#  kernext/trace/trcgenasm.s
#


#
# Global routines and variables defined here
#
	.globl ENTRY(trc_timestamp)
	.globl ENTRY(trchook)

#
# NAME:	    trchook
# FUNCTION: common trace hook routine
# INPUT:
#	kernel mode
#	translation on
#	sreg 0 has key 0
#	LR = return address
#	r2 = kernel TOC
#	r3 = hookword with hooktype set
#	r4-r9 = possible data words
#
# OUTPUT:
#
# Trace data including hookword, datawords, and optional timestamp written
# to trace buffer.
#
# Common linkage conventions observed (only volatile registers modified).
#

define(`_TRCHOOK')

	S_PROLOG(trchook)
        lbz     r0,DATA(Trconflag)      # get current trace mode

        mfmsr   r11               	# r11 = current msr
	cmpi	cr0,r0,1		# see what the mode is
	LTOC(r12,trchdr_ptr,data)	# get address of trace header struct
	blt	cr0,trc_exit		# mode = 0 (trace off), return
	l	r12,0(r12)
	cal	r12,trc_inptr(r12)
        rlinm   r10,r11,0,~MSR_EE   	# mask off EE bit
	cmpi	cr1,r0,3		# 
        mtmsr   r10                	# set msr with interrupts disabled
	beq	cr0,mem			# mode = 1 (to memory)
	blt	cr1,cmem		# mode = 2 (conditional to memory)
	beq	cr1,bus			# mode = 3 (to bus)

	# Spill mode (or default if unrecognized mode)
spill:
	cal	r10,trc_ovfcount - trc_inptr(r12)
	ATOMIC_INC(cr0,r12,r10)
	b	trc_exit		#

	# To-bus trace mode
bus:
	mfsr	r12,sr15		# save current sreg 15 value in r11
	LTOC(r10,trc_bussreg,data)	# r10 = address of bus sreg value
	l	r10,0(r10)		# r10 = bus sreg value
	mtsr	sr15,r10		# set sreg 15
	LTOC(r10,trc_busaddr,data)	# r10 = address of bus address
	l	r10,0(r10)		# r10 = bus address
	oriu	r10,r10,0xF000		# set r10 to use sreg 15
	rlinm	r0,r3,16+2,27,29	# r0 = # bytes to trace
					#       (hookword and data words )
					#    = lower 3 bits of hooktype x 4
	mtspr	XER,r0			# set byte count for store string
	stsx	r3,0,r10		# store hookword and data words
	mtsr	sr15,r12		# restore sreg 15 value
	b	trc_exit		#

	# Conditional to-memory trace mode
	# Determine if trace enabled for this hookid by checking
	# bit in event bitmap corresponding to this hookid
cmem:

	.machine "any"			
	sri r10,r3,20+3			
	.machine "com"			

pwpc_end:
	ai	r10,r10,trc_events - trc_inptr	# r10 = byte offset in event bitmap
	lbzx	r0,r12,r10		# load byte from bitmap into r0
	rlinm	r10,r3,32-20,29,31	# r10 = bit # within byte
	sr	r0,r0,r10		# shift to correct bit to first position
	andil.	r0,r0, 1		# mask bit from bitmap
	beq	trc_exit		# bit is off, exit
	b	mem			

	# To-memory trace mode (or continuation of conditional mode above)
	#
	# Note that it is OK for trace to store beyond the end of the buffer
	# (trace pointer goes beyond trc_end) since additional space is
	# allocated for the buffer (space beyond trc_end) to allow for this.
begin_trchk:
	l	r2, DATA(g_toc)(0)	# load kernel toc
        lbz     r0,DATA(Trconflag)      # get current trace mode

	cmpi	cr0,r0,1		# see what the mode is
	cmpi	cr1,r0,3		# 
	blt	cr0,trc_exit
	bgt	cr1,spill

mem:
	rlinm	r2,r3,16+2,27,29	# r0 = # bytes to trace (hookword and
					#       data words but not timestamp)
					#    = lower 3 bits of hooktype x 4
	mtspr	XER,r2			# set byte count for store string
	andiu.	r10,r3,HKTY_TMASK	# see if timestamp required
	cal	r2,4(r2)		# update trace pointer for tid
	beq	no_time			# no timestamp required, branch
	cal	r2,4(r2)		# update trace pointer for timestamp
no_time:
ifdef(`_POWER_MP',`
ifdef(`_POWER_RS',`
ifdef(`_POWER_PC',`
	l 	r10,syscfg_arch(0)
	cmpi 	cr0,r10,POWER_PC
	beq 	cr0,pwpc_bis
',) #endif _power_pc
	l 	r10,0(r12) 		# current trace pointer
	l       r0,trc_end - trc_inptr(r12)     # get end of trace buffer
	cmpl    cr0,r10,r0              # see if pointer is beyond end
	bge     begin_trchk             # at end of buffer, goto begin
	a       r2,r2,r10               # update trace pointer
	cmpl    cr1,r2,r0               # see if pointer is beyond end
	TIMER(r0)			# get current time in r0 
	st      r2,0(r12)
	ifdef(`_POWER_PC',`
		b 	pwpc_bis_end
	',) #endif _power_pc
',) #endif _power_rs
pwpc_bis:
	ifdef(`_POWER_PC',`
		.machine "ppc"
		lwarx   r10,r0,r12 		# current trace pointer
		l       r0,trc_end - trc_inptr(r12)     # get end of trace buffer
		cmpl    cr0,r10,r0              # see if pointer is beyond end
		bge     begin_trchk             # at end of buffer, goto begin
		a       r2,r2,r10               # update trace pointer
		cmpl    cr1,r2,r0               # see if pointer is beyond end
		TIMER(r0)			# get current time in r0 
		stwcx.  r2,r0,r12
		bne     mem	
		.machine "com"
	',) #endif _power_pc
',` #else _power_mp
	l 	r10,0(r12)              # current trace pointer
	l       r0,trc_end - trc_inptr(r12)     # get end of trace buffer
	cmpl    cr0,r10,r0              # see if pointer is beyond end
	a       r2,r2,r10               # update trace pointer 
	bge     begin_trchk             # at end of buffer, goto begin
	cmpl    cr1,r2,r0               # see if pointer is beyond end
	TIMER(r0)			# get current time in r0 
	st      r2,0(r12)
') #endif _power_mp
pwpc_bis_end:

	stsx	r3,0,r10		# store hookword and data words
ifdef(`_THREADS',`
	GET_CURTHREAD(cr0,r6,r4)
	l       r6, t_tid(r4)
',` # else _THREAD
	GET_CURPROC(cr0,r6,r4)
	l       r6, p_pid(r4)
') # endif _THREAD
	andiu.	r3,r3,HKTY_TMASK	# see if timestamp required
	beq	no_time_end		# no timestamp required, branch

	st	r6,-8(r2)
	mr	r6, r0			# get current time in r6 
no_time_end:
	st	r6,-4(r2)
	l	r2, DATA(g_toc)(0)	# load kernel toc
	blt	cr1,trc_exit		# not at end of buffer, exit

	# Buffer full condition: set up to call kernel 'C' trcbuffull routine.
	#
	# This must run with interrupts disabled:
	# - to guarantee consistent changes in trace state
	# - to avoid re-entrancy since a local stack is used.
	#
	# This code does the following:
	#
	# - sets stack pointer to a local fixed stack
	#    (so 'C' routine doesn't page-fault).
	# - sets mstintpri to INTMAX so any 'C' calls to i_disable, i_enable
	#   don't inadvertently enable interrupts.
	# - saves current stack pointer, link register, and mstintpri.
	# - passes pointer to trace header struct as only parameter

	LTOC(r9,trc_savefp,data)        # get pointer to fixed stack
	LTOC(r12,trchdr_ptr,data)       # get address of trace header struct
	mflr    r0                      # get link register
	l       r9, 0(r9)
	l       r12,0(r12)
	st      r1,0(r9)                # save current stack pointer
	st      r0,4(r9)                # save link register
	GET_CSA(cr0,r0,r10)
	mr      r3,r12                  # pass trace header ptr as 1st param
	lbz     r0,mstintpri(r10)       # get interrupt priority
	l       r4,trc_inptr(r12)       # pass the ptr to the buffer 2nd param
	st      r0,8(r9)                # save intpri
	LTOC(r12,_trcbuffullfp,data)    # get address of _trcbuffullfp
	l       r0,INTMAX(0)            # get INTMAX value
	l       r12, 0(r12)             # get function descriptor address
	stb     r0,mstintpri(r10)       # set intpri in mst to INTMAX
	cal     r1,-stkmin(r9)          # set stack pointer
	l       r0,0(r12)               # move the func desc to r0
	st      r11,16(r9)              # save r11
	mtctr   r0                      # move address to counter reg
	l       r2,4(r12)               # load TOC of the function to

        bctrl				# branch to the function

	# 'C' routine returns here

	l	r2, DATA(g_toc)(0)	# load kernel toc
	cal	r12,stkmin(r1)		# set r12 with stack pointer
	l	r0,4(r12)		# get saved link register
	mtlr	r0			# restore link register
	l	r11,16(r12)		# restore r11
	l	r1,0(r12)		# restore stack pointer
	GET_CSA(cr0,r0,r10)
	l	r0,8(r12)		# get interrupt priority
	stb	r0,mstintpri(r10)	# restore intpri in mst
	
trc_exit:
	# RESTORE INTERRUPTS
        mtmsr   r11                	# restore msr
	cror	0,0,0			# 603e workaound
	S_EPILOG


#
# NAME:	    trc_timestamp
# FUNCTION: get current timestamp
#
# Return contents of real-time clock in nanoseconds.
#
	S_PROLOG(trc_timestamp)
	TIMER(r3)
	S_EPILOG
	

#
# NAME:	    trchk, trchkt, trchkl, trchklt, trchkg, trchkgt
# FUNCTION: trace hook system calls
#
# These routines are defined for compatibility purposes.  All trace hook
# calls should use the TRCHK macros which insert the hooktype into the
# hookword and call the common trace routine (trchook for kernel callers,
# or utrchk for user-level callers).
#

	S_PROLOG(trchk)
	TYPE_INSERT(r3,HKTY_Sr)
	b	ENTRY(trchook)
        .extern ENTRY(trchook)

	S_PROLOG(trchkt)
	TYPE_INSERT(r3,HKTY_STr)
	b	ENTRY(trchook)
        .extern ENTRY(trchook)

	S_PROLOG(trchkl)
	TYPE_INSERT(r3,HKTY_Lr)
	b	ENTRY(trchook)
        .extern ENTRY(trchook)

	S_PROLOG(trchklt)
	TYPE_INSERT(r3,HKTY_LTr)
	b	ENTRY(trchook)
        .extern ENTRY(trchook)

	S_PROLOG(trchkg)
	TYPE_INSERT(r3,HKTY_Gr)
	b	ENTRY(trchook)
        .extern ENTRY(trchook)

	S_PROLOG(trchkgt)
	TYPE_INSERT(r3,HKTY_GTr)
	b	ENTRY(trchook)
        .extern ENTRY(trchook)

	
#
# TOC references
#
	.toc

        TOCE(_trcbuffullfp,data)
	TOCE(trc_savefp,data)
	TOCE(trchdr_ptr,data)
	TOCE(trc_bussreg,data)
	TOCE(trc_busaddr,data)

# Function Descriptors
	FCNDES(trchook)
	FCNDES(trchk)
	FCNDES(trchkt)
	FCNDES(trchkl)
	FCNDES(trchklt)
	FCNDES(trchkg)
	FCNDES(trchkgt)
	FCNDES(trc_timestamp)
