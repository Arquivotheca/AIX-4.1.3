# @(#)81	1.7  src/bos/kernel/ml/POWER/fp_fpscrx.m4, sysml, bos41J, 9511A_all 3/10/95 06:36:08
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

#
# fpstat_t _fp_fpscrx( int operation, fpstat_t status );
#
# SVC to perform operations on the in-memory 32-bit extension
# to the hardware fpscr. This is known as the `fpscrx', and
# is kept in: "mstfpscrx". 
#
# The hardware fpscr, together with "mstfpscrx" make up the
# user's complete floating point status.
#
# INPUT: 
#   gpr3: option - Desired operation to perform on "mstfpscrx".
#
#     The following input operations are valid:
#
	.set FP_FPSCRX_LD,  0x0	   	# read fpscrx
	.set FP_FPSCRX_ST,  0x1		# write fpscrx
	.set FP_FPSCRX_OR,  0x2		# "or" with fpscrx
	.set FP_FPSCRX_AND, 0x3		# "and" with fpscrx
#
#   gpr4: Value to use on operations. This is not used on the 
#         LD option, for example, which only Returns data.
#         For a ST`ORE', it is the value to write to "mstfpscrx".
#         For logical operations, such as AND (and) OR, gpr4
#         is the logical "read-modify-write" mask.
#
#   No error conditions are detected. If the operation is out
#   of range, it will be treated just like a LOAD operation.
# 
# OUTPUT: 
#   gpr3: For LD, & ST, the value of "mstfpscrx" on input.
#         For OR, & AND, the new, updated value of "mstfpscrx".
#
# NOTES:
#   This code will NOT page fault and runs with interrupts 
#   disabled. It is therefore not preemptable, thus requires
#   no special case logic to recompute the MSR(FP,FE,IE) bits.
#
#   This is quite performance-sensitive code, especially the
#   LD path, which is used to query floating point sticky bits,
#   typically.
#
ENTRY(fpscrx_sc):			# for mtrace tool
fpscrx_sc:
	rlinm	r11, r11, 0, ~SR_KsKp	# clear user key bit
	mfsr	r6, sr14		# save caller's sreg 14
	mfsr	r2, sr2			# save caller's sreg 2
	mtsr	sr14, r0		# load kernext segreg
	mfsr	r0, sr0			# save caller's sreg 0
	mtsr	sr2, r11		# load new proc-private seg
	mtsr	sr0, r12		# load kernel segreg
	isync
	GET_CURTHREAD(cr0, r11, r12)	# get curthread pointer
	l	r5, t_uthreadp(r12)	# get address of mstsave area
	mr      r7,r3			# copy operation to r7
       .using   mstsave,r5
        l       r3,mstfpscrx            # get software fpscr xtn
	cmpi    cr0,r7,FP_FPSCRX_LD     # see if Load operation
        bne     cr0,fpscrx_notld	# jump if not load
fpscrx_exit:
	mfctr	r12			# get caller's msr
	isync
	mtsr	sr0, r0			# reload caller's sreg 0
	mtsr    sr2, r2                 # reload caller's sreg 2
	mtsr	sr14, r6
	mtmsr	r12			# load msr
	isync
	br				# return

#
#	check for store operation type
#
fpscrx_notld:
	cmpi    cr0,r7,FP_FPSCRX_ST	# check for store operation
	bne     cr0,fpscrx_notst	# jump if not store
	st      r4,mstfpscrx            # write new sw fpscr xtn
	b	fpscrx_exit
#
#	check for logical OR operation
#
fpscrx_notst:
	cmpi	cr1,r7,FP_FPSCRX_OR	# check for logical "or"
	bne	cr1,fpscrx_notor	# jump if not "or"
	or 	r3,r3,r4		# perform logical "or"
	st	r3,mstfpscrx		# write back result
	b	fpscrx_exit		# exit
#
#	check for logical AND operation
#
fpscrx_notor:
	cmpi	cr1,r7,FP_FPSCRX_AND	# check for logical "and"
	bne	cr1,fpscrx_exit		# bail out if error
	and     r3,r3,r4		# perform logical "and"
	st	r3,mstfpscrx		# write back result
	b       fpscrx_exit		# exit
	.drop   r5			# drop u-blk addressability

	.globl ENTRY(fpscrx_sc)
