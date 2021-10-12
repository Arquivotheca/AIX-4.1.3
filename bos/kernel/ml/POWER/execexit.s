# @(#)31	1.3  src/bos/kernel/ml/POWER/execexit.s, sysml, bos411, 9428A410j 8/31/93 13:48:58
#
#   COMPONENT_NAME: SYSML
#
#   FUNCTIONS: execexit
#
#   ORIGINS: 27,83
#
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1989,1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************


	.machine "com"
	.file "execexit.s"

include(systemcfg.m4)
include(scrs.m4)
include(macros.m4)

#******************************************************************************#
#                                                                              #
#       execexit -- Exit from "exec()"                                         #
#                                                                              #
#       This assembly code patches things up so that the exit from exec()      #
#       is as much like a normal end of svc() as possible.                     #
#                                                                              #
#       Called from execve(), the following parameters are passed:             #
#  *       r3 -> structure on user stack containing the following:             #
#               User stack pointer                                             #
#               User TOC pointer                                               #
#               User argument #1                                               #
#               User argument #2                                               #
#               User argument #3                                               #
#               User LR and IAR value (same)                                   #
#  *    N.B.: FOR NOW, r3 doesn't point to such a structure, but instead:      #
#               r3 -> regs 0..31 followed by LR                                #
#                                                                              #
#       This routine does the following:                                       #
#       -- Pick up parameters from the stack                                   #
#       -- Disable, so that mstsave area can be modified                       #
#       -- Set p_suspend, the signal suspension count, to zero                 #
#       -- Compute user's MSR value = default value + MSR_FP, if               #
#               this thread has FP ownership. FE & IE are always zeroed.       #
#       -- Move user's segment register set to area used to save SRs           #
#               on interrupt                                                   #
#       -- Store user's parm regs, toc reg, stack reg, LR and IAR in mstsave   #
#       -- Kill other user GP regs in mstsave                                  #
#       -- Branch to resume                                                    #
#                                                                              #
#******************************************************************************#

	S_PROLOG(execexit)

#       Pick up parameters passed by execve

### Hack for now:  r3-> regs 0..31, lr
        l       r4, r2*4(r3)            # toc
        l       r5, r3*4(r3)            # arg 1
        l       r6, r4*4(r3)            # arg 2
        l       r7, r5*4(r3)            # arg 3
        l       r8, 32*4(r3)            # lr
        l       r3, r1*4(r3)            # stack

#       Disable interrupts; resume() will enable them again

        cal     r0, DISABLED_MSR(0)
        mtmsr   r0

	GET_PPDA(cr0, r15)		# get ppda address

	l	r9, ppda_curthread(r15) # address of our thread structure
       .using	thread, r9
        l       r10, ppda_csa(r15)	# address of our mstsave structure
       .using   mstsave, r10



#       Decrement t_suspend, the signal suspension count

	lha	r0, t_suspend
	ai	r0, r0, -1
	sth	r0, t_suspend

#       Compute user's MSR value (default, plus MSR_FP if appropriate)

	l	r11, ppda_fpowner(r15)	# get -> current owning thrd, or NULL
        cal     r0, 0(0)                # set default MSR value
        oril    r0, r0, DEFAULT_USER_MSR
        cmp     cr0, r11, r9            # compare owning thrd : curthread
        mfcr    r11                     # put results of compare (cr) in r11
        rlimi   r0, r11, 16, MSR_FP     # put eq compare result into msr FP bit
        st      r0, mstmsr              # store user's MSR value

#       Load user's segment register set from user adspace

	l	r11, t_userp(r9)
	lm	r16, u_adspace_sr(r11)

#       Store user's stack, toc, parameters, and iar values

        st      r3, r1*4 + mstgpr       # stack pointer
        st      r4, r2*4 + mstgpr       # toc pointer
        st      r5, r3*4 + mstgpr       # user parm 1
        st      r6, r4*4 + mstgpr       # user parm 2
        st      r7, r5*4 + mstgpr       # user parm 3
        st      r8, mstiar              # iar value

#       Store segment register set as if interrupted

        stm     r16, mstsr

#       Clear out all the user GP regs which don't have information in them

        cau     r24, 0, u.DEFAULT_GPR
        oril    r24, r24, l.DEFAULT_GPR
        st      r24, mstlr              # lr points nowhere
        st      r24, mstctr             # ctr is killed across call
        mr      r25, r24
        mr      r26, r24
        mr      r27, r24
        mr      r28, r24
        st      r24, mstmq              # mq
        st      r24, r0* 4 + mstgpr     # the unused GP regs
        mr      r29, r24
        mr      r30, r24
        mr      r31, r24
        stm     r30, r6 * 4 + mstgpr
        stm     r24, r8 * 4 + mstgpr
        stm     r24, r16 * 4 + mstgpr
        stm     r24, r24 * 4 + mstgpr

#       Branch to resume(), which will restart the thread in user mode

        b       ENTRY(resume)
	.extern	ENTRY(resume)

       .drop    r9
       .drop    r10


# Treate this as a START point.
# I don't think we can trace it back.

	_DF(_DF_START,0)

include(mstsave.m4)
include(proc.m4)
include(machine.m4)
include(user.m4)
include(low_dsect.m4)
