# @(#)96	1.3  src/bos/diag/da/fpa/mlfsx.s, dafpa, bos411, 9428A410j 3/23/94 06:28:04

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
#		Appropriate registers are loaded with input values. Then
#		instructions for this module will be executed and results
#		will be compared with expected results. If any test fails,
#		a particular return code will be returned to the caller.
#
	.csect	.mlfsx[PR]
	.globl	.mlfsx[PR]
	.align	2
.mlfsx:
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
	LTOC(4, _mlfsx, data)		# Loads g4 with the address of _mlfsx
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
 	.using	_mlfsx[RW],g4		# g4 is the base register to the data
#	 				     section _mlfsx[rw]. Eastablishing 
#					     @ability
#
	st	g4,fpsave
	l	g10,fpsave
#	
	xor	g11,g11,g11
	xor	g3,g3,g3		# g3 = 0. Initialize return code
#
#				# Loading floating pt registers for computation
#
#  Instructions used for testing
# 
#
#### 12 ####
#
	lfsx	r0,g10,g11
	stfd	r0,fpsave
	ai	g3,g3,1				# RC = 1
	l	g6,fpsave
	l	g7,o12
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 2
	l	g6,fpsave+4
	l	g7,o12+4
	cmp	0,g6,g7
	bne	0,FINI
	ai	g10,g10,4
	ai	g11,g11,8
#
#### 13 ####
#
	lfsx	r0,g10,g11
	stfd	r0,fpsave
	ai	g3,g3,1				# RC = 3
	l	g6,fpsave
	l	g7,o13
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 4
	l	g6,fpsave+4
	l	g7,o13+4
	cmp	0,g6,g7
	bne	0,FINI
	ai	g10,g10,4
	ai	g11,g11,8
#
#### 14 ####
#
	lfsx	r0,g10,g11
	stfd	r0,fpsave
	ai	g3,g3,1				# RC = 5
	l	g6,fpsave
	l	g7,o14
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 6
	l	g6,fpsave+4
	l	g7,o14+4
	cmp	0,g6,g7
	bne	0,FINI
	ai	g10,g10,4
	ai	g11,g11,8
#
#### 15 ####
#
	lfsx	r0,g10,g11
	stfd	r0,fpsave
	ai	g3,g3,1				# RC = 7
	l	g6,fpsave
	l	g7,o15
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 8
	l	g6,fpsave+4
	l	g7,o15+4
	cmp	0,g6,g7
	bne	0,FINI
	ai	g10,g10,4
	ai	g11,g11,8
#
#### 16 ####
#
	lfsx	r0,g10,g11
	stfd	r0,fpsave
	ai	g3,g3,1				# RC = 9
	l	g6,fpsave
	l	g7,o16
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 10
	l	g6,fpsave+4
	l	g7,o16+4
	cmp	0,g6,g7
	bne	0,FINI
	ai	g10,g10,4
	ai	g11,g11,8
#
#### 17 ####
#
	lfsx	r0,g10,g11
	stfd	r0,fpsave
	ai	g3,g3,1				# RC = 11
	l	g6,fpsave
	l	g7,o17
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 12
	l	g6,fpsave+4
	l	g7,o17+4
	cmp	0,g6,g7
	bne	0,FINI
	ai	g10,g10,4
	ai	g11,g11,8
#
#### 18 ####
#
	lfsx	r0,g10,g11
	stfd	r0,fpsave
	ai	g3,g3,1				# RC = 13
	l	g6,fpsave
	l	g7,o18
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 14
	l	g6,fpsave+4
	l	g7,o18+4
	cmp	0,g6,g7
	bne	0,FINI
	ai	g10,g10,4
	ai	g11,g11,8
#
#### 19 ####
#
	lfsx	r0,g10,g11
	stfd	r0,fpsave
	ai	g3,g3,1				# RC = 15
	l	g6,fpsave
	l	g7,o19
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 16
	l	g6,fpsave+4
	l	g7,o19+4
	cmp	0,g6,g7
	bne	0,FINI
	ai	g10,g10,4
	ai	g11,g11,8
#
#### 20 ####
#
	lfsx	r0,g10,g11
	stfd	r0,fpsave
	ai	g3,g3,1				# RC = 17
	l	g6,fpsave
	l	g7,o20
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 18
	l	g6,fpsave+4
	l	g7,o20+4
	cmp	0,g6,g7
	bne	0,FINI
	ai	g10,g10,4
	ai	g11,g11,8
#
#### 21 ####
#
	lfsx	r0,g10,g11
	stfd	r0,fpsave
	ai	g3,g3,1				# RC = 19
	l	g6,fpsave
	l	g7,o21
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 20
	l	g6,fpsave+4
	l	g7,o21+4
	cmp	0,g6,g7
	bne	0,FINI
	ai	g10,g10,4
	ai	g11,g11,8
#
#### 22 ####
#
	lfsx	r0,g10,g11
	stfd	r0,fpsave
	ai	g3,g3,1				# RC = 21
	l	g6,fpsave
	l	g7,o22
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 22
	l	g6,fpsave+4
	l	g7,o22+4
	cmp	0,g6,g7
	bne	0,FINI
	ai	g10,g10,4
	ai	g11,g11,8
#
#### 23 ####
#
	lfsx	r0,g10,g11
	stfd	r0,fpsave
	ai	g3,g3,1				# RC = 23
	l	g6,fpsave
	l	g7,o23
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 24
	l	g6,fpsave+4
	l	g7,o23+4
	cmp	0,g6,g7
	bne	0,FINI
	ai	g10,g10,4
	ai	g11,g11,8
#
	xor	g3,g3,g3		# Return code = 0; pass
#
FINI:	br
#
# Data Area
#
	.csect	_mlfsx[RW],3
_mlfsx: 
	.long	0xffc00000
o12:	.long	0xfff80000, 0x00000000
	.long	0xff7fffff
o13:	.long	0xc7efffff, 0xe0000000
	.long	0x755691f4
o14:	.long	0x46aad23e, 0x80000000
	.long	0x0c2c4ce9
o15:	.long	0x3985899d, 0x20000000
	.long	0x007ff40f
o16:	.long	0x380ffd03, 0xc0000000
	.long	0x807fffe8
o17:	.long	0xb80ffffa, 0x00000000
	.long	0x7f800000
o18:	.long	0x7ff00000, 0x00000000
	.long	0x80000000
o19:	.long	0x80000000, 0x00000000
	.long	0xff882481
o20:	.long	0xfff10490, 0x20000000
	.long	0x007ffffa
o21:	.long	0x380ffffe, 0x80000000
	.long	0x00000001
o22:	.long	0x36a00000, 0x00000000
	.long	0x807fffff
o23:	.long	0xb80fffff, 0xc0000000
fpsave:	.long	0xffffffff, 0xffffffff
#
	.toc
	TOCE(.mlfsx, entry)
	TOCL(_mlfsx, data)
