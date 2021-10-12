# @(#)38	1.5  src/bos/diag/da/fpa/mstfs.s, dafpa, bos411, 9428A410j 3/23/94 06:28:43
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
	include(fpa_s.h)

	S_PROLOG(mstfs)
	.csect	.mstfs[PR]
	.globl	.mstfs[PR]
	.align	2
.mstfs:
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
	LTOC(4, _mstfs, data)		# Loads g4 with the address of _mstfs
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
 	.using	_mstfs[RW],g4		# g4 is the base register to the data
#	 				     section _mstfs[rw]. Eastablishing 
#					     @ability
#
#	
	l	g8,T.pegas(2)		# g8 = pointer to machine type
	l	g8,0(g8)		# g8 = machine type
	xor	g3,g3,g3		# g3 = 0. Initialize return code
#
#				# Loading floating pt registers for computation
#
#  Instructions used for testing
# 
#### 1 ####
#
	ai	g3,g3,1				# RC = 1
	lfd	r7,i1fp
	mtfsf	0xff,r7
	lfd	r0,i1
	stfs	r0,tmp
	l	g6,tmp
	l	g7,o1
	cmp	0,g6,g7
	bne	0,FINI
#
#### 2 ####
#
	ai	g3,g3,1				# RC = 2
	lfd	r7,i2fp
	mtfsf	0xff,r7
	lfd	r0,i2
	stfs	r0,tmp
	l	g6,tmp
	l	g7,o2
	cmp	0,g6,g7
	bne	0,FINI
#
#### 3 ####
#
	ai	g3,g3,1				# RC = 3
	lfd	r7,i3fp
	mtfsf	0xff,r7
	lfd	r0,i3
	stfs	r0,tmp
	l	g6,tmp
	l	g7,o3
	cmp	0,g6,g7
	bne	0,FINI
#
#### 4 ####
#
	ai	g3,g3,1				# RC = 4
	lfd	r7,i4fp
	mtfsf	0xff,r7
	lfd	r0,i4
	stfs	r0,tmp
	l	g6,tmp
	l	g7,o4
	cmp	0,g6,g7
	bne	0,FINI
#
#### 5 ####
#
	ai	g3,g3,1				# RC = 5
	lfd	r7,i5fp
	mtfsf	0xff,r7
	lfd	r0,i5
	stfs	r0,tmp
	l	g6,tmp
	l	g7,o5
	cmp	0,g6,g7
	bne	0,FINI
#
##### 6 ####
#
	ai	g3,g3,1				# RC = 6
	lfd	r7,i6fp
	mtfsf	0xff,r7
	lfd	r0,i6
	stfs	r0,tmp
	l	g6,tmp
	l	g7,o6
	cmp	0,g6,g7
	bne	0,FINI
#
#### 7 ####
#
	ai	g3,g3,1				# RC = 7
	lfd	r7,i7fp
	mtfsf	0xff,r7
	lfd	r0,i7
	stfs	r0,tmp
	l	g6,tmp
	l	g7,o7
	cmp	0,g6,g7
	bne	0,FINI
#
##### 8 ####
#
	ai	g3,g3,1				# RC = 8
	lfd	r7,i8fp
	mtfsf	0xff,r7
	lfd	r0,i8
	stfs	r0,tmp
	l	g6,tmp
	l	g7,o8
	cmp	0,g6,g7
	bne	0,FINI
#
#### 9 ####
#
	ai	g3,g3,1				# RC = 9
	lfd	r7,i9fp
	mtfsf	0xff,r7
	lfd	r0,i9
	stfs	r0,tmp
	l	g6,tmp
	l	g7,o9
	cmp	0,g6,g7
	bne	0,FINI
#
##### 10 ####
#
	ai	g3,g3,1				# RC = 10
	lfd	r7,i10fp
	mtfsf	0xff,r7
	lfd	r0,i10
	stfs	r0,tmp
	l	g6,tmp
	l	g7,o10
	cmp	0,g6,g7
	bne	0,FINI
#
#### 11 ####
#
	ai	g3,g3,1				# RC = 11
	cmpi	0,g8,POWER_MP_A_MODEL_CODE
	bne	0,OTHER1
	lfd	r7,i11fp
	mtfsf	0xff,r7
	lfd	r0,i11
	stfs	r0,tmp
	l	g6,tmp
	l	g7,o11p				# Chip 601
	cmp	0,g6,g7
	bne	0,FINI
OTHER1:
#	lfd	r7,i11fp
#	mtfsf	0xff,r7
#	lfd	r0,i11
#	stfs	r0,tmp
#	l	g6,tmp
#	l	g7,o11
#	cmp	0,g6,g7
#	bne	0,FINI
#
##### 12 ####
#
	ai	g3,g3,1				# RC = 12
	cmpi	0,g8,POWER_MP_A_MODEL_CODE
	bne	0,OTHER2
	lfd	r7,i12fp
	mtfsf	0xff,r7
	lfd	r0,i12
	stfs	r0,tmp
	l	g6,tmp
	l	g7,o12p				# Chip 601
	cmp	0,g6,g7
	bne	0,FINI
OTHER2:
#	lfd	r7,i12fp
#	mtfsf	0xff,r7
#	lfd	r0,i12
#	stfs	r0,tmp
#	l	g6,tmp
#	l	g7,o12
#	cmp	0,g6,g7
#	bne	0,FINI
#
#### 13 ####
#
	ai	g3,g3,1				# RC = 13
	lfd	r7,i13fp
	mtfsf	0xff,r7
	lfd	r0,i13
	stfs	r0,tmp
	l	g6,tmp
	l	g7,o13
	cmp	0,g6,g7
	bne	0,FINI
#
#### 14 ####
#
	ai	g3,g3,1				# RC = 14
	cmpi	0,g8,POWER_MP_A_MODEL_CODE
	bne	0,OTHER3
	lfd	r7,i14fp
	mtfsf	0xff,r7
	lfd	r0,i14
	stfs	r0,tmp
	l	g6,tmp
	l	g7,o14p				# Chip 601
	cmp	0,g6,g7
	bne	0,FINI
OTHER3:
#	lfd	r7,i14fp
#	mtfsf	0xff,r7
#	lfd	r0,i14
#	stfs	r0,tmp
#	l	g6,tmp
#	l	g7,o14
#	cmp	0,g6,g7
#	bne	0,FINI
#
#### 15 ####
#
	ai	g3,g3,1				# RC = 15
	lfd	r7,i15fp
	mtfsf	0xff,r7
	lfd	r0,i15
	stfs	r0,tmp
	l	g6,tmp
	l	g7,o15
	cmp	0,g6,g7
	bne	0,FINI
#
#### 16 ####
#
	ai	g3,g3,1				# RC = 16
	lfd	r7,i16fp
	mtfsf	0xff,r7
	lfd	r0,i16
	stfs	r0,tmp
	l	g6,tmp
	l	g7,o16
	cmp	0,g6,g7
	bne	0,FINI
#
	xor	g3,g3,g3		# Return code = 0; pass
#
FINI:	br
	S_EPILOG
#
# Data Area
#
	.csect	_mstfs[RW],3
_mstfs: 
i1:	.long	0xb6eff462, 0x3364c103
o1:	.long	0x8000001f
i1fp:	.long	0xffffffff, 0x00000018
i2:	.long	0xbbb08a7a, 0x0361ab3f
o2:	.long	0x9d8453d0
i2fp:	.long	0xffffffff, 0x00000008
i3:	.long	0x453af538, 0xac4b465a
o3:	.long	0x69d7a9c5
i3fp:	.long	0xffffffff, 0x000000eb
i4:	.long	0x800ffff6, 0x4fb9807d
o4:	.long	0x80000000
i4fp:	.long	0xffffffff, 0x00000008
i5:	.long	0x2fafffff, 0xffffffff
o5:	.long	0x00000000
i5fp:	.long	0xffffffff, 0x0000005a
i6:	.long	0x7ff00000, 0x00000000
o6:	.long	0x7f800000
i6fp:	.long	0xffffffff, 0x000000fb
i7:	.long	0x7fefffff, 0xffffffff
o7:	.long	0x7f7fffff
i7fp:	.long	0xffffffff, 0x000000f8
i8:	.long	0x7ff80000, 0x00000000
o8:	.long	0x7fc00000
i8fp:	.long	0xffffffff, 0x000000e2
i9:	.long	0x000fffff, 0xffffffff
o9:	.long	0x00000000
i9fp:	.long	0xffffffff, 0x0000000a
i10:	.long	0xfff67585, 0xc260e445
o10:	.long	0xffb3ac2e
i10fp:	.long	0xffffffff, 0x000000fb
i11:	.long	0xb6afffff, 0xff133af5
o11:	.long	0xb57fffff
o11p:	.long	0x80000001		# Chip 601
i11fp:	.long	0xffffffff, 0x000000b3
i12:	.long	0x36affff1, 0x0c2491a0
o12:	.long	0x357fff88
o12p:	.long	0x00000001		# Chip 601
i12fp:	.long	0xffffffff, 0x000000b2
i13:	.long	0xfff80000, 0x00000000
o13:	.long	0xffc00000
i13fp:	.long	0xffffffff, 0x000000e8
i14:	.long	0xb6afffff, 0x0dca25f4
o14:	.long	0xb57ffff8
o14p:	.long	0x80000001		# Chip 601
i14fp:	.long	0xffffffff, 0x00000070
i15:	.long	0xfffbddd5, 0x9c6ec85e
o15:	.long	0xffdeeeac
i15fp:	.long	0xffffffff, 0x00000051
i16:	.long	0x380fffff, 0xfd96a9b7
o16:	.long	0x007fffff
i16fp:	.long	0xffffffff, 0x00000061
tmp:	.long	0xffffffff
#
	.toc
	TOCE(.mstfs, entry)
	TOCL(_mstfs, data)
T.pegas:.tc     pegas[tc],pegas
	.extern pegas
