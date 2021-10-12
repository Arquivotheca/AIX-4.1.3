# @(#)24	1.3  src/bos/diag/da/fpa/mstfdux.s, dafpa, bos411, 9428A410j 3/23/94 06:28:23

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
	.csect	.mstfdux[PR]
	.globl	.mstfdux[PR]
	.align	2
.mstfdux:
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
	LTOC(4, _mstfdux, data)		# Loads g4 with the address of _mstfdux
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
 	.using	_mstfdux[RW],g4		# g4 is the base register to the data
#	 				     section _mstfdux[rw]. Eastablishing
#					     @ability
	st	g4,fpsave
	l	g10,fpsave
#
#	
	xor	g11,g11,g11
	xor	g3,g3,g3		# g3 = 0. Initialize return code
#
#				# Loading floating pt registers for computation
#
#  Instructions used for testing
# 
#### 1 ####
#
	ai	g3,g3,1				# RC = 1
	lfd	r0,i1
	stfdux	r0,g10,g11
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
#
#### 2 ####
#
	ai	g3,g3,1				# RC = 3
	lfd	r0,i2
	stfdux	r0,g10,g11
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
#
#### 3 ####
#
	ai	g3,g3,1				# RC = 5
	lfd	r0,i3
	stfdux	r0,g10,g11
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
#
#### 4 ####
#
	ai	g3,g3,1				# RC = 7
	lfd	r0,i4
	stfdux	r0,g10,g11
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
#
#### 5 ####
#
	ai	g3,g3,1				# RC = 9
	lfd	r0,i5
	stfdux	r0,g10,g11
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
#
#### 6 ####
#
	ai	g3,g3,1				# RC = 11
	lfd	r0,i6
	stfdux	r0,g10,g11
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
#
#### 7 ####
#
	ai	g3,g3,1				# RC = 13
	lfd	r0,i7
	stfdux	r0,g10,g11
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
#
#### 8 ####
#
	ai	g3,g3,1				# RC = 15
	lfd	r0,i8
	stfdux	r0,g10,g11
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
#
#### 9 ####
#
	ai	g3,g3,1				# RC = 17
	lfd	r0,i9
	stfdux	r0,g10,g11
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
#
#### 10 ####
#
	ai	g3,g3,1				# RC = 19
	lfd	r0,i10
	stfdux	r0,g10,g11
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
#
#### 11 ####
#
	ai	g3,g3,1				# RC = 21
	lfd	r0,i11
	stfdux	r0,g10,g11
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
#
#### 12 ####
#
	ai	g3,g3,1				# RC = 23
	lfd	r0,i12
	stfdux	r0,g10,g11
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
#
#### 13 ####
#
	ai	g3,g3,1				# RC = 25
	lfd	r0,i13
	stfdux	r0,g10,g11
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
#
	xor	g3,g3,g3		# Return code = 0; pass
#
FINI:	br
#
# Data Area
#
	.csect	_mstfdux[RW],3
_mstfdux: 
fpsave:	.long	0xffffffff, 0xffffffff
i1:	.long	0xfff00000, 0x00000000
o1:	.long	0xfff00000
	.long	0x00000000
i2:	.long	0xfff80000, 0x00000000
o2:	.long	0xfff80000
	.long	0x00000000
i3:	.long	0xb8efffff, 0xffffffff
o3:	.long	0xb8efffff
	.long	0xffffffff
i4:	.long	0x6ba26c20, 0xeb1bdd3d
o4:	.long	0x6ba26c20
	.long	0xeb1bdd3d
i5:	.long	0xc2572144, 0x563afd41
o5:	.long	0xc2572144
	.long	0x563afd41
i6:	.long	0x28bb5524, 0x9291653e
o6:	.long	0x28bb5524
	.long	0x9291653e
i7:	.long	0xffefffff, 0xffffffff
o7:	.long	0xffefffff
	.long	0xffffffff
i8:	.long	0xb6afffff, 0xfe873417
o8:	.long	0xb6afffff
	.long	0xfe873417
i9:	.long	0x00000000, 0x0000000e
o9:	.long	0x00000000
	.long	0x0000000e
i10:	.long	0x7ffdb38d, 0x47af8d89
o10:	.long	0x7ffdb38d
	.long	0x47af8d89
i11:	.long	0x7ff34c39, 0xe2de3f43
o11:	.long	0x7ff34c39
	.long	0xe2de3f43
i12:	.long	0x7ff00000, 0x00000000
o12:	.long	0x7ff00000
	.long	0x00000000
i13:	.long	0xb69fffff, 0xffcb5c38
o13:	.long	0xb69fffff
	.long	0xffcb5c38
#
	.toc
	TOCE(.mstfdux, entry)
	TOCL(_mstfdux, data)
