# @(#)27      1.10  src/bos/kernel/ml/POWER/utrc_svc.m4, sysml, bos41J, 9511A_all 3/10/95 06:36:16
#
#   COMPONENT_NAME: SYSML
#
#   FUNCTIONS: none
#
#   ORIGINS: 27, 83
#
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1989, 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************


#******************************************************************************
#
# NAME: utrc_svc
#
# FUNCTION: SVC handler for user-level trace hooks
#
# INPUT:
#	kernel mode
#	translation on
#	interrupts disabled
#	LR = return address to user-level code
#	CTR bits 16-31 = caller's msr
#	r1 = caller's stack pointer
#	r2 (user's TOC) has be saved by glue code (and will be restored by it)
#	r3 = hookword with hooktype set
#	r4-r9 = possible data words
#	r0 = g_kxsrval
#	r11 = user sr2 value
#	r12  = g_ksrval
#	ctr = user mode msr value
#	lr = user mode return address
#
# OUTPUT:
#	Trace data including hookword, data words, and optional timestamp
#	written to kernel trace buffer.
#
# NOTES:
#	This code must reside in non-privileged memory (read access with key 1)
#	since it executes a few instructions at the beginning and some at the
#	end using the user-level segment register 0.
#
#	This routine and routines it calls are run with interrupts disabled.
#	This simplifies the logic somewhat but care should be taken to
#	minimize path length to avoid an impact to real-time performance.
#
#*******************************************************************************

ENTRY(utrc_sc):					# for mtrace tool
utrc_sc:
	mtsr	sr0, r12			# load kernel segreg
	rlinm	r11, r11, 0, ~SR_KsKp		# clear user key bit
	mtsr	sr14, r0			# load kernel extension seg
	mtsr	sr2, r11			# load new proc-private seg
	isync					# wait for seg reg loads

	GET_CURTHREAD(cr0, r11, r12)		# get curthread pointer
	l	r11, t_uthreadp(r12)		# get uthread address
	mflr	r2				# get user return address
	st	r2, t_stackp(r12)		# save it in thread struct.
	mfctr	r12				# get user msr
	st	r12, ut_msr(r11)		# save it in uthread struct.
	l	r2, DATA(g_toc)			# load kernel toc

	bl	ENTRY(trchook)			# call system trace hook
	.extern	ENTRY(trchook)

	GET_CURTHREAD(cr0, r11, r12)		# get curthread pointer
	l	r11, t_uthreadp(r12)		# get uthread address
	l	r7, t_stackp(r12)		# get user return address
	l	r4, ut_msr(r11)			# get user msr
	mtlr	r7				# restore return address
	lil	r0, -1				# kill volatile reg
	lil	r3, -1
	

#	kill misc. volatile registers

	l	r7, syscfg_impl(0)		# get implementation
	mtxer	r0				# kill xer
	andil.	r6, r7, POWER_RS_ALL|POWER_601	# machines having a MQ
	mfcr	r5
	st	r0, mstmq(r11)			# kill soft mq
	mtctr	r0				# kill counter
	beq	cr0, no_mq_trc			# branch if no mq
	.machine "any"
	mtmq	r0				# kill mq
	.machine "com"
no_mq_trc:
	l	r11, t_userp(r12)		# user structure
	rlinm	r5, r5, 0, 0x00ffff00		# kill volatile cr's
	mtcr	r5

	l	r8, u_adspace_sr+0*4(r11)	# get user segment 0
	l	r9, u_adspace_sr+2*4(r11)	# get user segment 2
	l	r10, u_adspace_sr+14*4(r11)	# get user segment 14

	isync
	mtsr	sr0, r8				# load segregs with problem
	mtsr	sr2, r9				# state values
	mtsr	sr14, r10

	mtmsr	r4				# back to problem state
	isync
	br					# return to application

	.globl ENTRY(utrc_sc)

