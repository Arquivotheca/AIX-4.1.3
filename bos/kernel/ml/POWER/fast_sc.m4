# @(#)23	1.7  src/bos/kernel/ml/POWER/fast_sc.m4, sysml, bos41J, 9513A_all 3/24/95 15:08:31
#
#   COMPONENT_NAME: SYSML
#
#   FUNCTIONS: DATA
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
#******************************************************************************
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************

#
# NOTES:
#	This must be in user readable memory
#
# To add a fast SC:
#	1 Define a new key
#	2 Add a function descriptor
#	3 Add condition branch base on key in fast_sc
#

# Keys for fast SCs

	.set	FPSC_UTRCHK, 1
	.set	FPSC_FPSCRX, 2
	.set    FPSC_PRIMXID, 3
	.set	FPSC_THREADDATA, 4

#*******************************************************************************
#
# NAME: fast_sc (Entry Point)
#
# FUNCTION:
#
#	fast path system calls are implemented by exporting function
# descriptors from the kernel.  The function descriptors contain the
# address of fast_sc_inst (IAR) and an index (TOC).  Application calling
# these functions reach the fast_sc_inst by glink.  cr1(eq) is cleared
# to flag a fast path call and the sc instruction is executed.
#
# sc_flih branch to fast_sc if cr1(eq) = 0.  fast_sc then takes the
# appropriate actions based on the index number (r2).
#
#*******************************************************************************

ENTRY(fast_sc_inst):				# for mtrace tool
fast_sc_inst:
	crxor	cr1*4+eq, cr1*4+eq, cr1*4+eq
	sc

#*******************************************************************************
#
# NAME: fast_sc
#
# FUNCTION:
#	system call handler for fast path SCs.  The function passes control
# to the correct service.  It the system call key is invalid the SC is
# reated like a bad system call
#
# REGISTERS:
#	INPUT:
#		r0 = g_kxsrval
#		r11 = user sr2 value
#		r12 = g_ksrval
#		ctr = user mode msr value
#		lr = user mode return addresss
#		r2 - system call key
#		r3-r10 parameters
#
#*******************************************************************************

ENTRY(fast_sc):					# for mtrace tool
fast_sc:
	cmpi	cr0, r2, FPSC_THREADDATA	# check for thread_userdata
	cmpi	cr1, r2, FPSC_PRIMXID		# check for _set_primxid

	beq	cr0, threaddata_sc
	beq	cr1, primxid_sc

	cmpi	cr0, r2, FPSC_UTRCHK		# check for user trace hook
	cmpi	cr1, r2, FPSC_FPSCRX		# check fpscr stuff

	beq	cr0, utrc_sc
	beq	cr1, fpscrx_sc

	b	bad_sc				# treat as a bad SVC

DATA(utrchook):
	.long	fast_sc_inst	# address of code
	.long	FPSC_UTRCHK	# TOC - not needed
	.long	0
DATA(_fp_fpscrx):
	.long	fast_sc_inst
	.long	FPSC_FPSCRX
	.long	0
DATA(_set_primxid):
	.long	fast_sc_inst
	.long	FPSC_PRIMXID
	.long	0
DATA(thread_userdata):
	.long	fast_sc_inst
	.long	FPSC_THREADDATA
	.long	0

	.globl ENTRY(fast_sc_inst)
	.globl ENTRY(fast_sc)
