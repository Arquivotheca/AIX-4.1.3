# @(#)58	1.3  src/bos/kernel/epost/POWER/fpu.s, sysepost, bos411, 9428A410j 3/25/94 16:16:54
#
# COMPONENT_NAME: (SYSEPOST) power-on self tests
#
# FUNCTIONS: fpu_epost
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

	.file "fpu.s"

#	Testing floating point unit. The return code, which is saved in
#	   general purpose register g3, is the error code.
#
# 	Defining symbols for general and floating point registers
#	g0 - g31	:	32 general purpose registers
#	f0 - f31	:	32 floating point registers
#
#
S_PROLOG(fpu_epost)

	mfspr   r7,lr           # save link reg
    #	mfmsr   r8              # get MSR
    #	oril    r6,r8,0x2000    # set FP - allow access to fpu
    #	mtmsr   r6
	bl      fpu_skip        # get addr of data
				# DATA START
C1:	.long	0xFFFFFFFF		# All one's
	.long	0xFFFFFFFF
C2:	.long	0x00000000		# All zero's
	.long	0x00000000
C.6:	.long	0x3FE33333		# 0.6
	.long	0x33333333
C.2:	.long	0x3FC99999		# 0.9
	.long	0x99999999
C3S:	.long	0x40400000		# 3.0 single precision
CM2S:	.long	0xC0000000		# -2.0 single precision
SM:	.long	0x00100000		# approx = 2^(1-1023), small number
	.long	0xC0000000
JUNK:	.long	0x98765432		# This will be written over
	.long	0x10FEDCBA
FSC:	.long	0xBA411000		# Should be the content of FPSCR
				# DATA END
fpu_skip:
	mfspr   r4,lr           # mv address of data to reg
 	.using	C1,r4		
#  	 			# Test data bus on/off
#
	lfd	f1,C1			# Get one's
	lfd	f2,C2			# Get zero's
	stfd	f1,C2			# Flip for store
	stfd	f2,C1
	mtfsf	0xff,f2			# Zero to FPSCR
	cal	r3, 0(0)		# r3 = 0
#
	cal	r3,1(r3)	# Error code = 1 
	l	r0,C1			# If DBUS bits stuck on
	cmpi	0,r0,0			# Check value = 0
	bne	FINI			# If not, exit
	l	r0,C1+4			# r0 = next 4 bytes of zero's
	cmpi	0,r0,0			# r0 = 0 ?
	bne	FINI			# If not, exit
#
	cal	r3,1(r3)	# Error code = 2
	l	r0,C2			# r0 gets all one's, i.e. -1
	cmpi	0,r0,-1			# r0 = -1 ?
	bne	FINI			# If not, exit
	l	r0,C2+4			# r0 = next 4 bytes of one's, i.e. -1
	cmpi	0,r0,-1			# r0 = -1 ?
	bne	FINI			# If not, exit
#
#				# Test each bit of CR bus; FI = 0
#
	cal	r3,1(r3)		# Error code = 3
	lfd	f2,C.6			# f2 = 0.6
	lfd	f3,C.2			# f3 = 0.2
	fcmpu	0,f3,f2			# f3 is compared with f2, "unordered"
	bnl	FINI			# Exit if f3 is not less than f2
	bgt	FINI			# Exit if f3 is greater than f2
	beq	FINI			# Exit if f3 is equal to f2 
	bso	FINI			# Exit on summary overflow
#
	cal	r3,1(r3)		# Error code = 4
	fcmpu	0,f2,f3			# f2 is compared with f3, "unordered"
	bng	FINI			# Exit if f2 is not greater than f3
	blt	FINI			# Exit if f2 is less than f3
	cal	r3,1(r3)		# Error code = 5
	fcmpu	0,f2,f2			# f2 is compared with f2, "unordered"
	bne	FINI			# Exit if f2 is not equal to f2
	cal	r3,1(r3)		# Error code = 6
	fcmpu	0,f1,f2			# f1 (-1) is compared with f2,         
	bns	FINI			# Exit on not summary overflow
#
#				# Test each bit of FI bus
#
	cal	r3,1(r3)		# Error code = 7
	fcmpu	2,f2,f2			# f2 is compared with f2, set field 2
	bne	2,FINI			# Check field 2, exit if not equal
	fcmpu	3,f3,f3			# f3 is compared with f3, set field 3
	bne	3,FINI			# Check field 3, exit if not equal
	fcmpu	4,f2,f2			# f2 is compared with f2, set field 4
	bne	4,FINI			# Check field 4, exit if not equal
#
#				# Arithmetic check
#
	lfs	f0,C3S			# f0 = 3.0, single precision
	cal	r3,1(r3)		# Error code = 8
	fd	f1,f2,f3		# f1=f2/f3 or f1 = 0.6 / 0.2 = 3.0
	fcmpu	0,f0,f1			# f0 is compared with f1
	bne	FINI			# Exit if not equal
#
#				# Check single words
#
	cal	r3,1(r3)		# Error code = 9
	stfs	f0,C2			# 3.0 is stored in 4 bytes @C2
	l	r0,C2			# r0 = 3.0
	lil	r5,0x0000		# r5 = 'xxxx0000' 
	liu	r5,0x4040		# r5 = '40400000' = 3
	cmp	0,r0,r5			# r0 is compared with r5
	bne	FINI			# Exit if not equal
#
	lfs	f5,CM2S			# f5 = -2.0 single precision
	lil	r5,0x0000		# r5 = 'xxxx0000'
	liu	r5,0x4000		# r5 = '40000000' 
	fneg	f6,f5			# f6 = -f5 = 2
	stfs	f6,C2+4			# 2 is stored @(C2+4)
 	l	r6,C2+4			# r6 = 2
	cmp	0,r5,r6			# r5 is compared with r6
	bne	FINI			# Exit if not equal
#
#				# Set exception bits
#
	cal	r3,1(r3)		# Error code = 10
	lfd	f5,SM			# f5 = 2^(1-1023), i.e. small number
	fm.	f1,f5,f3		# f1 = f5 * f3 = very small number
					#    The "." sets Rc = 1 
	fma	f0,f0,f0,f0		# f0 = f0*f0 + f0 = 3*3 + 3 = 12
	fd.	f4,f0,f5		# f4 = f0 / f5, create overflow
	bc	4,7,FINI		# Exit if not overflow. 4 means branch
					#    if false; 7 means overflow
	fms.	f0,f2,f3,f0		# f0=f2*f3-f0; set inexact flag
	mffs	f7			# f7 = (FPSCR)||X'FFFFFFFF'
	fd.	f8,f4,f4    		# f8=f4/f4; invalid for f4 is overflow
 	bc	4,6,FINI		# Exit if not invalid; 4 for false  
					#    condition; 6 for invalid bit 
	cal	r3,1(r3)		# Error code = 11
	l	r6,FSC			# r6 = X'BA411000'
	mffs	f9			# f9 = X'FFFFFFFF'||(FPSCR)
	stfd	f9,JUNK			# f9 is stored @JUNK
	l	r5,JUNK+4		# r5 = (FPSCR) 
	cmp	0,r5,r6			# r5 is compared with r6
	bne	FINI			# branch to FINI if not equal
#
	cal	r3, 0(0)		# r3 = 0 ; OK, passes all tests
FINI:	mtspr   lr,r7
	br                      # return

	S_EPILOG
