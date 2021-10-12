# @(#)14	1.5  src/bos/diag/da/fpa/mstfsx.s, dafpa, bos411, 9428A410j 3/23/94 06:29:18
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
#
#		Appropriate registers are loaded with input values. Then
#		instructions for this module will be executed and results
#		will be compared with expected results. If any test fails,
#		a particular return code will be returned to the caller.
#
	include(fpa_s.h)

	S_PROLOG(mstfsx)
	.csect	.mstfsx[PR]
	.globl	.mstfsx[PR]
	.align	2
.mstfsx:
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
	LTOC(4, _mstfsx, data)		# Loads g4 with the address of _mstfsx
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
 	.using	_mstfsx[RW],g4		# g4 is the base register to the data
#	 				     section _mstfsx[rw]. Eastablishing 
	st	g4,tmp
	l	g10,tmp
#					     @ability
#
#	
	l       g8,T.pegas(2)           # g8 = pointer to machine type
	l       g8,0(g8)                # g8 = machine type
	xor	g11,g11,g11
	xor	g3,g3,g3		# g3 = 0. Initialize return code
#
#				# Loading floating pt registers for computation
#
#  Instructions used for testing
# 
#
#### 1 ####
#
	ai	g3,g3,1				# RC = 1
	lfd	r7,i1fp
	mtfsf	0xff,r7
	lfd	r0,i1
	stfsx	r0,g10,g11
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
	stfsx	r0,g10,g11
	l	g6,tmp
	l	g7,o2
	cmp	0,g6,g7
	bne	0,FINI
#
#### 3 ####
#
	ai	g3,g3,1				# RC = 3
	cmpi	0,g8,POWER_MP_A_MODEL_CODE
	bne	0,OTHER1
	lfd	r7,i3fp
	mtfsf	0xff,r7
	lfd	r0,i3
	stfsx	r0,g10,g11
	l	g6,tmp
	l	g7,o3p
	cmp	0,g6,g7
	bne	0,FINI
OTHER1:
#	lfd	r7,i3fp
#	mtfsf	0xff,r7
#	lfd	r0,i3
#	stfsx	r0,g10,g11
#	l	g6,tmp
#	l	g7,o3
#	cmp	0,g6,g7
#	bne	0,FINI
#
#### 4 ####
#
	ai	g3,g3,1				# RC = 4
	lfd	r7,i4fp
	mtfsf	0xff,r7
	lfd	r0,i4
	stfsx	r0,g10,g11
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
	stfsx	r0,g10,g11
	l	g6,tmp
	l	g7,o5
	cmp	0,g6,g7
	bne	0,FINI
#
#### 6 ####
#
	ai	g3,g3,1				# RC = 6
	lfd	r7,i6fp
	mtfsf	0xff,r7
	lfd	r0,i6
	stfsx	r0,g10,g11
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
	stfsx	r0,g10,g11
	l	g6,tmp
	l	g7,o7
	cmp	0,g6,g7
	bne	0,FINI
#
#### 8 ####
#
	ai	g3,g3,1				# RC = 8
	cmpi	0,g8,POWER_MP_A_MODEL_CODE
	bne	0,OTHER2
	lfd	r7,i8fp
	mtfsf	0xff,r7
	lfd	r0,i8
	stfsx	r0,g10,g11
	l	g6,tmp
	l	g7,o8p
	cmp	0,g6,g7
	bne	0,FINI
OTHER2:
#	lfd	r7,i8fp
#	mtfsf	0xff,r7
#	lfd	r0,i8
#	stfsx	r0,g10,g11
#	l	g6,tmp
#	l	g7,o8
#	cmp	0,g6,g7
#	bne	0,FINI
#
#### 9 ####
#
	ai	g3,g3,1				# RC = 9
	lfd	r7,i9fp
	mtfsf	0xff,r7
	lfd	r0,i9
	stfsx	r0,g10,g11
	l	g6,tmp
	l	g7,o9
	cmp	0,g6,g7
	bne	0,FINI
#
#### 10 ####
#
	ai	g3,g3,1				# RC = 10
	lfd	r7,i10fp
	mtfsf	0xff,r7
	lfd	r0,i10
	stfsx	r0,g10,g11
	l	g6,tmp
	l	g7,o10
	cmp	0,g6,g7
	bne	0,FINI
#
#### 11 ####
#
	ai	g3,g3,1				# RC = 11
	lfd	r7,i11fp
	mtfsf	0xff,r7
	lfd	r0,i11
	stfsx	r0,g10,g11
	l	g6,tmp
	l	g7,o11
	cmp	0,g6,g7
	bne	0,FINI
#
#### 12 ####
#
	ai	g3,g3,1				# RC = 12
	lfd	r7,i12fp
	mtfsf	0xff,r7
	lfd	r0,i12
	stfsx	r0,g10,g11
	l	g6,tmp
	l	g7,o12
	cmp	0,g6,g7
	bne	0,FINI
#
#### 13 ####
#
	ai	g3,g3,1				# RC = 13
	lfd	r7,i13fp
	mtfsf	0xff,r7
	lfd	r0,i13
	stfsx	r0,g10,g11
	l	g6,tmp
	l	g7,o13
	cmp	0,g6,g7
	bne	0,FINI
#
#### 14 ####
#
	ai	g3,g3,1				# RC = 14
	lfd	r7,i14fp
	mtfsf	0xff,r7
	lfd	r0,i14
	stfsx	r0,g10,g11
	l	g6,tmp
	l	g7,o14
	cmp	0,g6,g7
	bne	0,FINI
#
#### 15 ####
#
	ai	g3,g3,1				# RC = 15
	lfd	r7,i15fp
	mtfsf	0xff,r7
	lfd	r0,i15
	stfsx	r0,g10,g11
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
	stfsx	r0,g10,g11
	l	g6,tmp
	l	g7,o16
	cmp	0,g6,g7
	bne	0,FINI
#
#### 17 ####
#
	ai	g3,g3,1				# RC = 17
	lfd	r7,i17fp
	mtfsf	0xff,r7
	lfd	r0,i17
	stfsx	r0,g10,g11
	l	g6,tmp
	l	g7,o17
	cmp	0,g6,g7
	bne	0,FINI
#
#### 18 ####
#
	ai	g3,g3,1				# RC = 18
	lfd	r7,i18fp
	mtfsf	0xff,r7
	lfd	r0,i18
	stfsx	r0,g10,g11
	l	g6,tmp
	l	g7,o18
	cmp	0,g6,g7
	bne	0,FINI
#
#### 19 ####
#
	ai	g3,g3,1				# RC = 19
	lfd	r7,i19fp
	mtfsf	0xff,r7
	lfd	r0,i19
	stfsx	r0,g10,g11
	l	g6,tmp
	l	g7,o19
	cmp	0,g6,g7
	bne	0,FINI
#
#### 20 ####
#
	ai	g3,g3,1				# RC = 20
	cmpi	0,g8,POWER_MP_A_MODEL_CODE
	bne	0,OTHER3
	lfd	r7,i20fp
	mtfsf	0xff,r7
	lfd	r0,i20
	stfsx	r0,g10,g11
	l	g6,tmp
	l	g7,o20p
	cmp	0,g6,g7
	bne	0,FINI
OTHER3:
#	lfd	r7,i20fp
#	mtfsf	0xff,r7
#	lfd	r0,i20
#	stfsx	r0,g10,g11
#	l	g6,tmp
#	l	g7,o20
#	cmp	0,g6,g7
#	bne	0,FINI
#
	xor	g3,g3,g3		# Return code = 0; pass
#
FINI:	br
	S_EPILOG
#
# Data Area
#
	.csect	_mstfsx[RW],3
_mstfsx: 
tmp:	.long	0xffffffff
i1:	.long	0x379fffff, 0xed910a9e
o1:	.long	0x0000ffff
i1fp:	.long	0xffffffff, 0x000000c0
i2:	.long	0x800fffff, 0xa8277444
o2:	.long	0x80000000
i2fp:	.long	0xffffffff, 0x0000005a
i3:	.long	0xa67e1d1d, 0x822fa30d
o3:	.long	0xb3f0e8ec
o3p:	.long	0x80000000		# Chip 601
i3fp:	.long	0xffffffff, 0x0000002b
i4:	.long	0x6cff2e8e, 0x8bbba396
o4:	.long	0x67f97474
i4fp:	.long	0xffffffff, 0x000000f0
i5:	.long	0xfff9d7c5, 0xde452324
o5:	.long	0xffcebe2e
i5fp:	.long	0xffffffff, 0x000000a2
i6:	.long	0x7fefffff, 0xffffffff
o6:	.long	0x7f7fffff
i6fp:	.long	0xffffffff, 0x00000050
i7:	.long	0x36af3974, 0xb9409f2c
o7:	.long	0x00000001
i7fp:	.long	0xffffffff, 0x00000009
i8:	.long	0xb6afffff, 0xf8600866
o8:	.long	0xb57fffff
o8p:	.long	0x80000001		# Chip 601
i8fp:	.long	0xffffffff, 0x000000f9
i9:	.long	0xfff00000, 0x00000000
o9:	.long	0xff800000
i9fp:	.long	0xffffffff, 0x0000000b
i10:	.long	0x7fefffff, 0xffffffff
o10:	.long	0x7f7fffff
i10fp:	.long	0xffffffff, 0x00000092
i11:	.long	0x7ff80000, 0x00000000
o11:	.long	0x7fc00000
i11fp:	.long	0xffffffff, 0x00000028
i12:	.long	0x380fffff, 0xfffddafe
o12:	.long	0x007fffff
i12fp:	.long	0xffffffff, 0x00000089
i13:	.long	0xb81fffff, 0xe5aaafa9
o13:	.long	0x80ffffff
i13fp:	.long	0xffffffff, 0x00000093
i14:	.long	0x36afb1f1, 0x8f174f61
o14:	.long	0x00000001
i14fp:	.long	0xffffffff, 0x00000099
i15:	.long	0xb81fffff, 0xf419e0e0
o15:	.long	0x80ffffff
i15fp:	.long	0xffffffff, 0x000000da
i16:	.long	0x7ff00000, 0x00000000
o16:	.long	0x7f800000
i16fp:	.long	0xffffffff, 0x00000019
i17:	.long	0x7ff00000, 0x00000000
o17:	.long	0x7f800000
i17fp:	.long	0xffffffff, 0x000000f2
i18:	.long	0xfff2d54d, 0x7b0b89a8
o18:	.long	0xff96aa6b
i18fp:	.long	0xffffffff, 0x000000d1
i19:	.long	0xb69fffff, 0xfffa3a30
o19:	.long	0x80000000
i19fp:	.long	0xffffffff, 0x0000001b
i20:	.long	0x36afff9c, 0xa72bf22b
o20:	.long	0x357ffce5
o20p:	.long	0x00000001		# Chip 601
i20fp:	.long	0xffffffff, 0x00000032
#
	.toc
	TOCE(.mstfsx, entry)
	TOCL(_mstfsx, data)
T.pegas:.tc     pegas[tc],pegas
	.extern pegas
