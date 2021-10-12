# @(#)42	1.3  src/bos/kernel/ml/POWER/thread_svc.m4, sysproc, bos41J, 9513A_all 3/24/95 15:15:25
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
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

#******************************************************************************
#
# NAME: threaddata_sc
#
# FUNCTION: SVC handler for getting thread-specific user data
#
# INPUT:
#	kernel mode
#	translation on
#	interrupts disabled
#	LR = return address to user-level code
#	CTR bits 16-31 = caller's msr
#	r1 = caller's stack pointer
#	r2 (user's TOC) has be saved by glue code (and will be restored by it)
#	r0 = g_kxsrval
#	r11 = user sr2 value
#	r12  = g_ksrval
#
# OUTPUT:
#	returns content of ut_userdata
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
# register usage:
# 	r4 = saved user msr
#	r5 = saved sr0
#	r6 = saved sr14
# 	r12 = ptr to the current thread
#
# 	int thread_userdata()
# 	{
#	    r5=mfsr0();
#	    r6=mfsr14();
#
#	    /* set up kernel addressibility
#            */
#	    mtsr0(r12);
#	    mtsr14(r0);
#	    isync;
#	    ctr = mfctr();
#
#	    rc = curthread->t_userdata;
#
#           /* restore user mode segment registers and msr 
#            */	
#	    mtsr0(r5);
#	    mtsr14(r6);
#	    mtmsr(ctr);
#	    isync
#	    return(rc);
#       }
#
#*******************************************************************************

	.machine "com"
ENTRY(threaddata_sc):				# for mtrace tool
threaddata_sc:
	mfsr	r5, sr0				# save kernel segreg
	mfsr	r6, sr14			# save kernel extension seg
	mtsr	sr0, r12			# load kernel segreg
	mtsr	sr14, r0			# load kernel extension seg
	isync					# wait for seg reg loads

	GET_CURTHREAD(cr0, r11, r12)		# r12 = current thread pointer
	mfctr	r4				# r4 = user msr
	l	r3, t_userdata(r12)		# user data

	mtsr	sr0, r5				# restore segregs
	mtsr	sr14, r6
	mtmsr	r4				# back to problem state
	isync
	br					# return to application

	.globl ENTRY(threaddata_sc)
