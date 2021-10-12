# @(#)37	1.3  src/bos/diag/da/fpa/mlfsu.s, dafpa, bos411, 9428A410j 3/23/94 06:27:45

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
	.csect	.mlfsu[PR]
	.globl	.mlfsu[PR]
	.align	2
.mlfsu:
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
	LTOC(4, _mlfsu, data)		# Loads g4 with the address of _mlfsu
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
 	.using	_mlfsu[RW],g4		# g4 is the base register to the data
#	 				     section _mlfsu[rw]. Eastablishing 
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
	lfsu	r0,0(g5)
	stfd	r0,fpsave
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
	ai	g5,g5,12
#
#### 2 ####
#
	lfsu	r0,0(g5)
	stfd	r0,fpsave
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
	ai	g5,g5,12
#
#### 3 ####
#
	lfsu	r0,0(g5)
	stfd	r0,fpsave
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
	ai	g5,g5,12
#
#### 4 ####
#
	lfsu	r0,0(g5)
	stfd	r0,fpsave
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
	ai	g5,g5,12
#
#### 5 ####
#
	lfsu	r0,0(g5)
	stfd	r0,fpsave
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
	ai	g5,g5,12
#
#### 6 ####
#
	lfsu	r0,0(g5)
	stfd	r0,fpsave
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
	ai	g5,g5,12
#
#### 7 ####
#
	lfsu	r0,0(g5)
	stfd	r0,fpsave
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
	ai	g5,g5,12
#
#### 8 ####
#
	lfsu	r0,0(g5)
	stfd	r0,fpsave
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
	ai	g5,g5,12
#
#### 9 ####
#
	lfsu	r0,0(g5)
	stfd	r0,fpsave
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
	ai	g5,g5,12
#
#### 10 ####
#
	lfsu	r0,0(g5)
	stfd	r0,fpsave
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
	ai	g5,g5,12
#
	xor	g3,g3,g3		# Return code = 0; pass
#
FINI:	br
#
# Data Area
#
	.csect	_mlfsu[RW],3
_mlfsu: 
i1:	.long	0x80000000
o1:	.long	0x80000000, 0x00000000
i2:	.long	0x7fc00000
o2:	.long	0x7ff80000, 0x00000000
i3:	.long	0x9e059753
o3:	.long	0xbbc0b2ea, 0x60000000
i4:	.long	0x71ebce6b
o4:	.long	0x463d79cd, 0x60000000
i5:	.long	0x007fffff
o5:	.long	0x380fffff, 0xc0000000
i6:	.long	0x807ffffe
o6:	.long	0xb80fffff, 0x80000000
i7:	.long	0xffc00000
o7:	.long	0xfff80000, 0x00000000
i8:	.long	0x42f36e62
o8:	.long	0x405e6dcc, 0x40000000
i9:	.long	0x807fffff
o9:	.long	0xb80fffff, 0xc0000000
i10:	.long	0x80000001
o10:	.long	0xb6a00000, 0x00000000
fpsave:	.long	0xffffffff, 0xffffffff
#
	.toc
	TOCE(.mlfsu, entry)
	TOCL(_mlfsu, data)
