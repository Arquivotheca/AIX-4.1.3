# @(#)95	1.3  src/bos/diag/da/fpa/mlfdu.s, dafpa, bos411, 9428A410j 3/23/94 06:27:01

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
	.csect	.mlfdu[PR]
	.globl	.mlfdu[PR]
	.align	2
.mlfdu:
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
	LTOC(4, _mlfdu, data)		# Loads g4 with the address of _mlfdu
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
 	.using	_mlfdu[RW],g4		# g4 is the base register to the data
#	 				     section _mlfdu[rw]. Eastablishing 
#					     @ability
	st	g4,fpsave
	l	g5,fpsave
#
#	
	xor	g3,g3,g3		# g3 = 0. Initialize return code
#
#				# Loading floating pt registers for computation
#
#  Instructions used for testing
# 
#### 1 ####
#
	st	g5,tmp
	lfdu	r0,0(g5)
	stfd	r0,fpsave
#
	ai	g3,g3,1				# RC = 1
	l	g6,fpsave
	l	g7,o1
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 2
	l	g6,fpsave+4
	l	g7,o1+4
	cmp	0,g6,g7
	bne	0,FINI
	l	g5,tmp
	ai	g5,g5,16
#
#### 2 ####
#
	st	g5,tmp
	lfdu	r0,0(g5)
	stfd	r0,fpsave
#
	ai	g3,g3,1				# RC = 3
	l	g6,fpsave
	l	g7,o2
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 4
	l	g6,fpsave+4
	l	g7,o2+4
	cmp	0,g6,g7
	bne	0,FINI
	l	g5,tmp
	ai	g5,g5,16
#
#### 3 ####
#
	st	g5,tmp
	lfdu	r0,0(g5)
	stfd	r0,fpsave
#
	ai	g3,g3,1				# RC = 5
	l	g6,fpsave
	l	g7,o3
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 6
	l	g6,fpsave+4
	l	g7,o3+4
	cmp	0,g6,g7
	bne	0,FINI
	l	g5,tmp
	ai	g5,g5,16
#
#### 4 ####
#
	st	g5,tmp
	lfdu	r0,0(g5)
	stfd	r0,fpsave
#
	ai	g3,g3,1				# RC = 7
	l	g6,fpsave
	l	g7,o4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 8
	l	g6,fpsave+4
	l	g7,o4+4
	cmp	0,g6,g7
	bne	0,FINI
	l	g5,tmp
	ai	g5,g5,16
#
#### 5 ####
#
	st	g5,tmp
	lfdu	r0,0(g5)
	stfd	r0,fpsave
#
	ai	g3,g3,1				# RC = 9
	l	g6,fpsave
	l	g7,o5
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 10
	l	g6,fpsave+4
	l	g7,o5+4
	cmp	0,g6,g7
	bne	0,FINI
	l	g5,tmp
	ai	g5,g5,16
#
#### 6 ####
#
	st	g5,tmp
	lfdu	r0,0(g5)
	stfd	r0,fpsave
#
	ai	g3,g3,1				# RC = 11
	l	g6,fpsave
	l	g7,o6
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 12
	l	g6,fpsave+4
	l	g7,o6+4
	cmp	0,g6,g7
	bne	0,FINI
	l	g5,tmp
	ai	g5,g5,16
#
#### 7 ####
#
	st	g5,tmp
	lfdu	r0,0(g5)
	stfd	r0,fpsave
#
	ai	g3,g3,1				# RC = 13
	l	g6,fpsave
	l	g7,o7
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 14
	l	g6,fpsave+4
	l	g7,o7+4
	cmp	0,g6,g7
	bne	0,FINI
	l	g5,tmp
	ai	g5,g5,16
#
#### 8 ####
#
	st	g5,tmp
	lfdu	r0,0(g5)
	stfd	r0,fpsave
#
	ai	g3,g3,1				# RC = 15
	l	g6,fpsave
	l	g7,o8
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 16
	l	g6,fpsave+4
	l	g7,o8+4
	cmp	0,g6,g7
	bne	0,FINI
	l	g5,tmp
	ai	g5,g5,16
#
#### 9 ####
#
	st	g5,tmp
	lfdu	r0,0(g5)
	stfd	r0,fpsave
#
	ai	g3,g3,1				# RC = 17
	l	g6,fpsave
	l	g7,o9
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 18
	l	g6,fpsave+4
	l	g7,o9+4
	cmp	0,g6,g7
	bne	0,FINI
	l	g5,tmp
	ai	g5,g5,16
#
#### 10 ####
#
	st	g5,tmp
	lfdu	r0,0(g5)
	stfd	r0,fpsave
#
	ai	g3,g3,1				# RC = 19
	l	g6,fpsave
	l	g7,o10
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 20
	l	g6,fpsave+4
	l	g7,o10+4
	cmp	0,g6,g7
	bne	0,FINI
	l	g5,tmp
	ai	g5,g5,16
#
#### 11 ####
#
	st	g5,tmp
	lfdu	r0,0(g5)
	stfd	r0,fpsave
#
	ai	g3,g3,1				# RC = 21
	l	g6,fpsave
	l	g7,o11
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 22
	l	g6,fpsave+4
	l	g7,o11+4
	cmp	0,g6,g7
	bne	0,FINI
	l	g5,tmp
	ai	g5,g5,16
#
#### 12 ####
#
	st	g5,tmp
	lfdu	r0,0(g5)
	stfd	r0,fpsave
#
	ai	g3,g3,1				# RC = 23
	l	g6,fpsave
	l	g7,o12
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 24
	l	g6,fpsave+4
	l	g7,o12+4
	cmp	0,g6,g7
	bne	0,FINI
	l	g5,tmp
	ai	g5,g5,16
#
#### 13 ####
#
	st	g5,tmp
	lfdu	r0,0(g5)
	stfd	r0,fpsave
#
	ai	g3,g3,1				# RC = 25
	l	g6,fpsave
	l	g7,o13
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 26
	l	g6,fpsave+4
	l	g7,o13+4
	cmp	0,g6,g7
	bne	0,FINI
	l	g5,tmp
	ai	g5,g5,16
#
#### 14 ####
#
	st	g5,tmp
	lfdu	r0,0(g5)
	stfd	r0,fpsave
#
	ai	g3,g3,1				# RC = 27
	l	g6,fpsave
	l	g7,o14
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 28
	l	g6,fpsave+4
	l	g7,o14+4
	cmp	0,g6,g7
	bne	0,FINI
	l	g5,tmp
	ai	g5,g5,16
#
#### 15 ####
#
	st	g5,tmp
	lfdu	r0,0(g5)
	stfd	r0,fpsave
#
	ai	g3,g3,1				# RC = 29
	l	g6,fpsave
	l	g7,o15
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 30
	l	g6,fpsave+4
	l	g7,o15+4
	cmp	0,g6,g7
	bne	0,FINI
	l	g5,tmp
	ai	g5,g5,16
#
#### 16 ####
#
	st	g5,tmp
	lfdu	r0,0(g5)
	stfd	r0,fpsave
#
	ai	g3,g3,1				# RC = 31
	l	g6,fpsave
	l	g7,o16
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 32
	l	g6,fpsave+4
	l	g7,o16+4
	cmp	0,g6,g7
	bne	0,FINI
	l	g5,tmp
	ai	g5,g5,16
#
#### 17 ####
#
	st	g5,tmp
	lfdu	r0,0(g5)
	stfd	r0,fpsave
#
	ai	g3,g3,1				# RC = 33
	l	g6,fpsave
	l	g7,o17
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 34
	l	g6,fpsave+4
	l	g7,o17+4
	cmp	0,g6,g7
	bne	0,FINI
	l	g5,tmp
	ai	g5,g5,16
#
#### 18 ####
#
	st	g5,tmp
	lfdu	r0,0(g5)
	stfd	r0,fpsave
#
	ai	g3,g3,1				# RC = 35
	l	g6,fpsave
	l	g7,o18
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 36
	l	g6,fpsave+4
	l	g7,o18+4
	cmp	0,g6,g7
	bne	0,FINI
	l	g5,tmp
	ai	g5,g5,16
#
#### 19 ####
#
	st	g5,tmp
	lfdu	r0,0(g5)
	stfd	r0,fpsave
#
	ai	g3,g3,1				# RC = 37
	l	g6,fpsave
	l	g7,o19
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 38
	l	g6,fpsave+4
	l	g7,o19+4
	cmp	0,g6,g7
	bne	0,FINI
	l	g5,tmp
	ai	g5,g5,16
#
	xor	g3,g3,g3		# Return code = 0; pass
#
FINI:	br
#
# Data Area
#
	.csect	_mlfdu[RW],3
_mlfdu: 
	.long	0x80000001
	.long	0x09422bb2
o1:	.long	0x80000001, 0x09422bb2
	.long	0xffefffff
	.long	0xffffffff
o2:	.long	0xffefffff, 0xffffffff
	.long	0x1f900000
	.long	0x00000000
o3:	.long	0x1f900000, 0x00000000
	.long	0x80000000
	.long	0x000008ed
o4:	.long	0x80000000, 0x000008ed
	.long	0xfff80000
	.long	0x00000000
o5:	.long	0xfff80000, 0x00000000
	.long	0x800fffbc
	.long	0x2be4e204
o6:	.long	0x800fffbc, 0x2be4e204
	.long	0x80000000
	.long	0x00000029
o7:	.long	0x80000000, 0x00000029
	.long	0x800fffff
	.long	0xffffffff
o8:	.long	0x800fffff, 0xffffffff
	.long	0x1398ed95
	.long	0x2ecc0638
o9:	.long	0x1398ed95, 0x2ecc0638
	.long	0x80000000
	.long	0x00000000
o10:	.long	0x80000000, 0x00000000
	.long	0x7ff00000
	.long	0x00000000
o11:	.long	0x7ff00000, 0x00000000
	.long	0xbeb1c108
	.long	0x56855bc1
o12:	.long	0xbeb1c108, 0x56855bc1
	.long	0x778d2f15
	.long	0x6b913ca2
o13:	.long	0x778d2f15, 0x6b913ca2
	.long	0x7ff00000
	.long	0x00000000
o14:	.long	0x7ff00000, 0x00000000
	.long	0x00000000
	.long	0x016242b6
o15:	.long	0x00000000, 0x016242b6
	.long	0xfff00000
	.long	0x00000000
o16:	.long	0xfff00000, 0x00000000
	.long	0x7ffa1a4a
	.long	0xef86f114
o17:	.long	0x7ffa1a4a, 0xef86f114
	.long	0x80000000
	.long	0x00000000
o18:	.long	0x80000000, 0x00000000
	.long	0x000fffff
	.long	0xffffffff
o19:	.long	0x000fffff, 0xffffffff
fpsave:	.long	0xffffffff, 0xffffffff
tmp:	.long	0xffffffff
#
	.toc
	TOCE(.mlfdu, entry)
	TOCL(_mlfdu, data)
