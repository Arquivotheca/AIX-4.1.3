# @(#)32	1.3  src/bos/diag/da/fpa/startup.s, dafpa, bos411, 9428A410j 3/23/94 06:29:37

#
#   COMPONENT_NAME: DAFPA
#
#   FUNCTIONS: 
#
#   ORIGINS: 27, 83
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#   LEVEL 1, 5 Years Bull Confidential Information
#
#
#	Testing floating point unit. The return code, which is saved in
#	   general purpose register g3, is the error code.
#
# 	Defining symbols for general and floating point registers
#	g0 - g31	:	32 general purpose registers
#	rf0 - rf31	:	32 floating point registers
#
#
	.csect	.startup[PR]
	.globl	.startup[PR]
	.align	2
.startup:
	.set	g0,0
	.set	rf0,0
	.set	g1,1
	.set	rf1,1
	.set	g2,2
	.set	rf2,2
	.set	g3,3
	.set	rf3,3
	.set	g4,4
	.set	rf4,4
	.set	g5,5
	.set	rf5,5
	.set	g6,6
	.set	rf6,6
	.set	g7,7
	.set	rf7,7
	.set	g8,8
	.set	rf8,8
	.set	g9,9
	.set	rf9,9
	.set	g10,10
	.set	rf10,10
	.set	g11,11
	.set	rf11,11
	.set	g12,12
	.set	rf12,12
	.set	g13,13
	.set	rf13,13
	.set	g14,14
	.set	rf14,14
	.set	g15,15
	.set	rf15,15
	.set	g16,16
	.set	rf16,16
	.set	g17,17
	.set	rf17,17
	.set	g18,18
	.set	rf18,18
	.set	g19,19
	.set	rf19,19
	.set	g20,20
	.set 	rf20,20
	.set	g21,21
	.set	rf21,21
	.set	g22,22
	.set	rf22,22
	.set	g23,23
	.set	rf23,23
	.set	g24,24
	.set	rf24,24
	.set	g25,25
	.set	rf25,25
	.set	g26,26
	.set	rf26,26
	.set	g27,27
	.set	rf27,27
	.set	g28,28
	.set	rf28,28
	.set	g29,29
	.set	rf29,29
	.set	g30,30
	.set	rf30,30
	.set	g31,31
	.set	rf31,31
#
	LTOC(4, _startup, data)		# Loads g4 with the address of _startup
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
 	.using	_startup[RW],g4		# g4 is the base register to the data
#	 				     section _startup[rw]. Eastablishing 
#					     @ability
#
#  	 			# Test data bus on/off
#
	lfd	rf1,C1			# Get one's
	lfd	rf2,C2			# Get zero's
	stfd	rf1,C2			# Flip for store
	stfd	rf2,C1
	mtfsf	0xff,rf2		# Zero to FPSCR
#
	xor	g3,g3,g3  		# g3 = 0
	cal	g3,1(g3)		# Error code = 1 
	l	g0,C1			# If DBUS bits stuck on
	cmpi	0,g0,0			# Check value = 0
	bne	FINI			# If not, exit
	l	g0,C1+4			# g0 = next 4 bytes of zero's
	cmpi	0,g0,0			# g0 = 0 ?
	bne	FINI			# If not, exit
#
	cal	g3,1(g3)		# Error code = 2
	l	g0,C2			# g0 gets all one's, i.e. -1
	cmpi	0,g0,-1			# g0 = -1 ?
	bne	FINI			# If not, exit
	l	g0,C2+4			# g0 = next 4 bytes of one's, i.e. -1
	cmpi	0,g0,-1			# g0 = -1 ?
	bne	FINI			# If not, exit
#
#				# Test each bit of CR bus; FI = 0
#
	cal	g3,1(g3)		# Error code = 3
	lfd	rf2,C.6			# f2 = 0.6
	lfd	rf3,C.2			# f3 = 0.2
	fcmpu	0,rf3,rf2		# f3 is compared with f2, "unordered"
	bnl	FINI			# Exit if f3 is not less than f2
	bgt	FINI			# Exit if f3 is greater than f2
	beq	FINI			# Exit if f3 is equal to f2 
	bso	FINI			# Exit on summary overflow
#
	cal	g3,1(g3)		# Error code = 4
	fcmpu	0,rf2,rf3		# f2 is compared with f3, "unordered"
	bng	FINI			# Exit if f2 is not greater than f3
	blt	FINI			# Exit if f2 is less than f3
	cal	g3,1(g3)		# Error code = 5
	fcmpu	0,rf2,rf2		# f2 is compared with f2, "unordered"
	bne	FINI			# Exit if f2 is not equal to f2
	cal	g3,1(g3)		# Error code = 6
	fcmpu	0,rf1,rf2		# f1 (-1) is compared with f2,         
	bns	FINI			# Exit on not summary overflow
#
#				# Test each bit of FI bus
#
	cal	g3,1(g3)		# Error code = 7
	fcmpu	2,rf2,rf2		# f2 is compared with f2, set field 2
	bne	2,FINI			# Check field 2, exit if not equal
	fcmpu	3,rf3,rf3		# f3 is compared with f3, set field 3
	bne	3,FINI			# Check field 3, exit if not equal
	fcmpu	4,rf2,rf2		# f2 is compared with f2, set field 4
	bne	4,FINI			# Check field 4, exit if not equal
#
#				# Arithmetic check
#
	lfs	f0,C3S			# f0 = 3.0, single precision
	cal	g3,1(g3)		# Error code = 8
	fd	rf1,rf2,rf3		# f1=f2/f3 or f1 = 0.6 / 0.2 = 3.0
	fcmpu	0,rf0,rf1		# f0 is compared with f1
	bne	FINI			# Exit if not equal
#
#				# Check single words
#
	cal	g3,1(g3)		# Error code = 9
	stfs	rf0,C2			# 3.0 is stored in 4 bytes @C2
	l	g0,C2			# g0 = 3.0
	lil	g5,0x0000		# g5 = 'xxxx0000' 
	liu	g5,0x4040		# g5 = '40400000' = 3
	cmp	0,g0,g5			# g0 is compared with g5
	bne	FINI			# Exit if not equal
#
	lfs	rf5,CM2S		# f5 = -2.0 single precision
	lil	g6,0x0000		# g6 = 'xxxx0000'
	liu	g6,0x4000		# g6 = '40000000' 
	fneg	rf6,rf5			# f6 = -f5 = 2
	stfs	rf6,C2+4		# 2 is stored @(C2+4)
 	l	g7,C2+4			# g7 = 2
	cmp	0,g6,g7			# g6 is compared with g7
	bne	FINI			# Exit if not equal
#
#				# Set exception bits
#
	cal	g3,1(g3)		# Error code = 10
	lfd	rf5,SM			# f5 = 2^(1-1023), i.e. small number
	fm.	rf1,rf5,rf3		# f1 = f5 * f3 = very small number
#					    The "." sets Rc = 1 
	fma	rf0,rf0,rf0,rf0		# f0 = f0*f0 + f0 = 3*3 + 3 = 12
	fd.	rf4,rf0,rf5		# f4 = f0 / f5, create overflow
	bc	4,7,FINI		# Exit if not overflow. 4 means branch
#					    if false; 7 means overflow
	fms.	rf0,rf2,rf3,rf0		# f0=f2*f3-f0; set inexact flag
	mffs	rf7			# f7 = (FPSCR)||X'FFFFFFFF'
	fd.	rf8,rf4,rf4    		# f8=f4/f4; invalid for f4 is overflow
 	bc	4,6,FINI		# Exit if not invalid; 4 for false  
#					    condition; 6 for invalid bit 
	cal	g3,1(g3)		# Error code = 11
	l	g10,FSC			# g10 = X'BA411000'
	mffs	rf9			# f9 = X'FFFFFFFF'||(FPSCR)
	stfd	rf9,JUNK		# f9 is stored @JUNK
	l	g9,JUNK+4		# g9 = (FPSCR) 
	cmp	0,g9,g10		# g9 is compared with g10
	bne	FINI			# branch to FINI if not equal
#
	xor	g3,g3,g3		# g3 = 0 ; OK, passes all tests
#
FINI:	br				# Return to caller. Return code in g3
#
#				# Data area
	.csect _startup[RW]
_startup:
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
#
	.toc
	TOCE(.startup, entry)
	TOCL(_startup, data)

