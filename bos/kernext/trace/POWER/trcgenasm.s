# @(#)49        1.13  src/bos/kernext/trace/POWER/trcgenasm.s, systrace, bos41J, 9519A_all 5/4/95 09:20:27
#
# COMPONENT_NAME: SYSTRACE  /dev/systrace pseudo-device driver
#
# FUNCTIONS: trcgenasm
#
# ORIGINS: 27 83
#
#  This module contains IBM CONFIDENTIAL code. -- (IBM
#  Confidential Restricted when combined with the aggregated
#  modules for this product)
#                   SOURCE MATERIALS
#  (C) COPYRIGHT International Business Machines Corp. 1985, 1994
#  All Rights Reserved
#
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# LEVEL 1,  5 Years Bull Confidential Information
#

include(low_dsect.m4)
#include(sys/comlink.m4)
include(scrs.m4)
#include(seg.m4)
include(machine.m4)
include(mstsave.m4)
include(i_machine.m4)
include(trchdr.m4)
include(trchk.m4)
include(thread.m4)
#include(proc.m4)
include(systemcfg.m4)
#include(macros.m4)

# WARNING:
# If you modify this file, it may be necessary to modify io/POWER/trchka.s
#



#
# Global routines and variables defined here
#
ifdef(`_TRCGENASM_ON',`
	.globl ENTRY(trcgenasm_on)
',`
	.globl DATA(trc_stack)
	.globl DATA(trc_save)
	.globl ENTRY(trcgenasm)
	.globl ENTRY(mem_isync)
	.globl ENTRY(mem_sync)
')

#
# NAME:	    trcgenasm
# FUNCTION: common trace hook routine
# INPUT:
#	kernel mode
#	translation on
#	sreg 0 has key 0
#	LR = return address
#	r2 = kernel TOC
#	r3 = hookword with hooktype set
#	r4 = d1
#	r5 = ibuf
#	r6 = wlen
#	r7 = tp ( = trchdr)
#	r8 = chan
#
# OUTPUT:
#
# Trace data including hookword, datawords, and optional timestamp written
# to trace buffer.
#
# Common linkage conventions observed (only volatile registers modified).
#
ifdef(`_TRCGENASM_ON',`
	S_PROLOG(trcgenasm_on)

	mfmsr   r12               	# r12 = current msr
	rlinm   r10,r12,0,~MSR_EE   	# mask off EE bit
	mtmsr   r10                	# set msr with interrupts disabled
        mr      r11,r12           	# save current msr in r11
	cal	r7,trc_inptr(r7)
	b	mem			# mode = 2 (to memory)
',`
	S_PROLOG(trcgenasm)

	LTOC(r11,Trconflag,data)	# get address of Trconflag
	mfmsr   r12               	# r12 = current msr
	a	r11, r11, r8
	rlinm   r10,r12,0,~MSR_EE   	# mask off EE bit
	mtmsr   r10                	# set msr with interrupts disabled
	lbz	r0, 0(r11)		# load r8-st byte of trace flags 
	cmpi	cr0,r0,1                # see what the mode is mem
	cmpi	cr1,r0,2                # see what the mode is mem
        mr      r11,r12           	# save current msr in r11
	cal	r7,trc_inptr(r7)
	beq	cr0,mem			# mode = 1 (to memory)
	beq	cr1,mem			# mode = 2 (to memory)

')
	# Spill mode (or default if no mem mode)
spill:
define(ATOMIC_INC, `
	ifdef(`_POWER_RS',`
	ifdef(`_POWER_PC',`
		LTOC($2,_system_configuration,data) 	# get address of
							# _system_configuration
        	l 	$2,scfg_arch($2)
		cmpi	$1, $2, POWER_PC	# check	for PPC
		beq	$1, $+20
	') #endif _POWER_PC
		lx	$2, 0, $3
		addi	$2, $2, 1
		stx	$2, 0, $3
	ifdef(`_POWER_PC',`
		b	$+20
	') #endif _POWER_PC
	') #endif _POWER_RS
	ifdef(`_POWER_PC',`
		.machine "ppc"
		lwarx	$2, 0, $3
		addi	$2, $2, 1
		stwcx.	$2, 0, $3
		bne	cr0, $-12
		.machine "com"
	') #endif _POWER_PC
')
	cal     r10,trc_ovfcount - trc_inptr(r7)
	ATOMIC_INC(cr0,r12,r10)
	b	trc_exit		#

	# To-memory trace mode 
	#
	# Note that it is OK for trace to store beyond the end of the buffer
	# (trace pointer goes beyond trc_end) since additional space is
	# allocated for the buffer (space beyond trc_end) to allow for this.
begin_trchk:
	LTOC(r9,Trconflag,data)		# get address of Trconflag
	a	r9, r9, r8
	lbz	r0, 0(r9)		# load r8-st byte of trace flags 
	cmpi	cr0,r0,1		# see what the mode is mem
	cmpi	cr1,r0,2		# 
	beq	cr0,mem			# mode = 1 (to memory)
	beq	cr1,mem			# mode = 2 (to memory)
	b	spill
mem:
	sli	r12,r6,2		# init the lenght of the event
					# and the tid
	andiu.	r0,r3,HKTY_TMASK	# see if timestamp required
	cal	r12,12(r12)		# add 12 for the hook word , the data
	beq	no_time			# no timestamp required, branch
	cal	r12,4(r12)		# update the lenght
no_time:
ifdef(`_POWER_MP',`
ifdef(`_POWER_RS',`
ifdef(`_POWER_PC',`
	LTOC(r10,_system_configuration,data) 	# get address of
						# _system_configuration
        l r10,scfg_arch(r10)
        cmpi cr0,r10,POWER_PC
        beq cr0,pwpc
',) #endif _power_pc
	l       r10,0(r7)               # current trace pointer ie
					# l r10,trc_inptr - trc_inptr(r7)
	l       r9,trc_end - trc_inptr(r7)      # get end of trace buffer
	cmpl    cr0,r10,r9              # see if pointer is beyond end
	bge     begin_trchk             # at end of buffer, goto begin
	a       r0,r10,r12              # update trace pointer
	cmpl    cr1,r0,r9              # see if pointer is beyond end
	TIMER(r9)			# get current time in r9
	st      r0,0(r7)
	ifdef(`_POWER_PC',`
		b pwpc_end
	',) #endif _power_pc
',) #endif _power_rs
pwpc:
	ifdef(`_POWER_PC',`
                .machine "ppc"
tryagain:						
		lwarx   r10,r0,r7
		lwz     r9,trc_end - trc_inptr(r7)
		cmpl    cr0,r10,r9              # see if pointer is beyond end
		bge     begin_trchk             # at end of buffer, goto begin
		a       r0,r10,r12              # update trace pointer
		cmpl    cr1,r0,r9              # see if pointer is beyond end
		TIMER(r9)			# get current time in r9
		stwcx.  r0,r0,r7
		bne tryagain			
		.machine "com"
	',) #endif _power_pc
',` #else _power_mp
	l       r10,0(r7)               # current trace pointer ie
					# l r10,trc_inptr - trc_inptr(r7)
	l       r9,trc_end - trc_inptr(r7)      # get end of trace buffer
	cmpl    cr0,r10,r9              # see if pointer is beyond end
	bge     begin_trchk             # at end of buffer, goto begin
	a       r0,r10,r12              # update trace pointer
	cmpl    cr1,r0,r9              # see if pointer is beyond end
	TIMER(r9)			# get current time in r9
	st      r0,0(r7)
') #endif _power_mp
pwpc_end:
ifdef(`_TRCGENASM_ON',`
',`
	bgt	cr1,full		# end of buffer, goto full
')
	st	r3,0(r10)		# store hookword
	st	r4,4(r10)		# and data words
	cal	r10,8(r10)
	cmpi	cr0,r6,0
	ble	end_copy
begin_copy:
# Reorder instructions <bui>
	l	r0,0(r5)
	cal	r6,-1(r6)
	cal	r5,4(r5)
	st	r0,0(r10)
	cmpi	cr0,r6,0
	cal	r10,4(r10)
	bgt	begin_copy
#
end_copy:
define(GET_CURTHREAD_LOCAL, `
		LTOC($2,_system_configuration,data) 	# get address of
						# _system_configuration
        	l 	$2,scfg_ncpus($2)
                cmpi    $1, $2, 1        # check fo UP
                bgt     $1, $+16
                l       $3, proc_arr_addr(0)
                l       $3, ppda_curthread($3)
                b       $+8
                .machine "ppc"
                mfspr   $3, SPRG2
')
	GET_CURTHREAD_LOCAL(cr0,r6,r4)
	l	r6, t_tid(r4)

	andiu.	r3,r3,HKTY_TMASK	# see if timestamp required <bui>
	st	r6,0(r10)
	cal	r10,4(r10)
	beq	no_time_end		# no timestamp required, branch
	st	r9,0(r10)		# store timestamp
	cal	r10,4(r10)
no_time_end:
	blt	cr1,trc_exit		# not at end of buffer, exit

	# Buffer full condition: set up to call kernel 'C' trcbuffullgen routine.
	#
full:
define(GET_CSA_LOCAL, `
		LTOC($2,_system_configuration,data) 	# get address of
						# _system_configuration
        	l 	$2,scfg_ncpus($2)
                cmpi    $1, $2, 1        # check fo UP
                bgt     $1, $+16
                l       $3, proc_arr_addr(0)
                l       $3, ppda_csa($3)
                b       $+8
                .machine "ppc"
                mfspr   $3, SPRG3
')

	LTOC(r9,trc_save,data)		# get address of trc_save
	mulli	r12,r8,4140		# offset of the trc_save entry
	a	r9,r9,r12		# ptr to trc_save entry
	st	r1,0(r9)		# save stkp
	mr	r1,r9
	mflr	r0			# get link register
	st	r0,4(r1)		# save link register
	GET_CSA_LOCAL(cr0,r12,r9)	# get csa
	lbz	r12,mstintpri(r9)	# get interrupt priority
	st	r12,8(r1)		# save intpri
	st	r11,12(r1)		# save r11 msr
	mfcr	r0
	st	r0,16(r1)		# save cr
	st	r3,20(r1)		# save parameter
	st	r4,24(r1)		# save parameter
	st	r5,28(r1)		# save parameter
	st	r6,32(r1)		# save parameter
	st	r7,36(r1)		# save parameter
	st	r8,40(r1)		# save parameter
	l	r0,INTMAX(0)		# get INTMAX value
	stb	r0,mstintpri(r9)	# set intpri in mst to INTMAX
	cal	r3,-trc_inptr(r7)	# pass trace header ptr as 1st param
	mr	r4,r10			# pass trc_intptr before the
					# incrementation as 2nd param
	cal	r1,-stkmin(r1)		# set stack pointer
	bl	ENTRY(trcbuffullgen)	# call 'C' routine 
       .extern	ENTRY(trcbuffullgen)      

	# 'C' routine returns here

	cal	r1,stkmin(r1)		
	l	r0,4(r1)		# restore link register
	mtlr	r0			# restore link register
	l	r3,20(r1)		# restore parameter
	l	r4,24(r1)		# restore parameter
	l	r5,28(r1)		# restore parameter
	l	r6,32(r1)		# restore parameter
	l	r7,36(r1)		# restore parameter
	l	r8,40(r1)		# restore parameter
	l	r11,12(r1)		# restore r11 msr
	l	r0,16(r1)		# restore cr
	mtcr	r0
	GET_CSA_LOCAL(cr0,r12,r9)        # get csa
	l	r12,8(r1)		# restore intpri
	stb	r12,mstintpri(r9)	# set interrupt priority
	l	r1,0(r1)		# set r1 with  stack pointer
ifdef(`_TRCGENASM_ON',`
',`
	beq	cr1,trc_exit
	b begin_trchk
')
	
trc_exit:
	# RESTORE INTERRUPTS
        mtmsr   r11                	# restore msr
	S_EPILOG

ifdef(`_TRCGENASM_ON',`
# already defined
',` #else 
S_PROLOG(mem_isync)
                                # Called after lock taken
	isync                   # stop pre-fetch
	S_EPILOG


S_PROLOG(mem_sync)
                                # Called from unlock primitives
	sync
	S_EPILOG
') #endif
#
# Local fixed stack for calling 'C' trcbuffull routine.
# Note that this must be large enough to handle the stack required
# by trcbuffull and the routines that it calls.
# there is one area trc_stack + trc_save per channel
# trchook uses only channel 0 and trc_save.No computation needed
# trcgenasm needs to identify trc_save associated to the used channel (r8)
# trc_save is used as follows:
# 0,4,8 : trcbuffull set-up code to save stack pointer, link register, intpri
# 12    : save area for counter register (used by user-level trace hook code)
# 16 : r11
#
# old value: 512
ifdef(`_TRCGENASM_ON',`
# already defined
',` #else 

define(STACKSIZE,4096)
	
			.align 2
DATA(trc_stack):	.space STACKSIZE
DATA(trc_save):		.space 11*4
DATA(trc_stack1):	.space STACKSIZE
DATA(trc_save1):	.space 11*4
DATA(trc_stack2):	.space STACKSIZE
DATA(trc_save2):	.space 11*4	
DATA(trc_stack3):	.space STACKSIZE
DATA(trc_save3):	.space 11*4
DATA(trc_stack4):	.space STACKSIZE
DATA(trc_save4):	.space 11*4
DATA(trc_stack5):	.space STACKSIZE
DATA(trc_save5):	.space 11*4
DATA(trc_stack6):	.space STACKSIZE
DATA(trc_save6):	.space 11*4
DATA(trc_stack7):	.space STACKSIZE
DATA(trc_save7):	.space 11*4				
') #endif

#
# TOC references
#
	.toc

ifdef(`_TRCGENASM_ON',`
	TOCE(trc_save,data)
',`
	TOCL(trc_save,data)
')
	TOCE(Trconflag,data)    
	TOCE(_system_configuration,data)    

# Function Descriptors
ifdef(`_TRCGENASM_ON',`
	FCNDES(trcgenasm_on)
',`
	FCNDES(trcgenasm)
	FCNDES(mem_isync)
	FCNDES(mem_sync)
')
