# @(#)98	1.5  src/bos/diag/da/fpa/mstfsux.s, dafpa, bos411, 9428A410j 3/23/94 06:29:06
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
#
	include(fpa_s.h)

	S_PROLOG(mstfsux)
	.csect	.mstfsux[PR]
	.globl	.mstfsux[PR]
	.align	2
.mstfsux:
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
	LTOC(4, _mstfsux, data)		# Loads g4 with the address of _mstfsux
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
 	.using	_mstfsux[RW],g4		# g4 is the base register to the data
#	 				     section _mstfsux[rw]. Eastablishing 
#					     @ability
	l       g8,T.pegas(2)           # g8 = pointer to machine type
	l       g8,0(g8)                # g8 = machine type
	st	g4,tmp
	l	g10,tmp
#
#	
	xor 	g11,g11,g11
	xor	g3,g3,g3		# g3 = 0. Initialize return code
#
#				# Loading floating pt registers for computation
#
#  Instructions used for testing
# 
#### 1 ####
#
	st	g10,tmp
	ai	g3,g3,1				# RC = 1
	lfd	r7,i1fp
	mtfsf	0xff,r7
	lfd	r0,i1
	stfsux	r0,g10,g11
	l	g6,fpsave
	l	g7,o1
	cmp	0,g6,g7
	bne	0,FINI
	l	g10,tmp
#
#### 2 ####
#
	st	g10,tmp
	ai	g3,g3,1				# RC = 2
	lfd	r7,i2fp
	mtfsf	0xff,r7
	lfd	r0,i2
	stfsux	r0,g10,g11
	l	g6,fpsave
	l	g7,o2
	cmp	0,g6,g7
	bne	0,FINI
	l	g10,tmp
#
#### 3 ####
#
	st	g10,tmp
	ai	g3,g3,1				# RC = 3
	lfd	r7,i3fp
	mtfsf	0xff,r7
	lfd	r0,i3
	stfsux	r0,g10,g11
	l	g6,fpsave
	l	g7,o3
	cmp	0,g6,g7
	bne	0,FINI
	l	g10,tmp
#
#### 4 ####
#
	st	g10,tmp
	ai	g3,g3,1				# RC = 4
	lfd	r7,i4fp
	mtfsf	0xff,r7
	lfd	r0,i4
	stfsux	r0,g10,g11
	l	g6,fpsave
	l	g7,o4
	cmp	0,g6,g7
	bne	0,FINI
	l	g10,tmp
#
#### 5 ####
#
	st	g10,tmp
	ai	g3,g3,1				# RC = 5
	lfd	r7,i5fp
	mtfsf	0xff,r7
	lfd	r0,i5
	stfsux	r0,g10,g11
	l	g6,fpsave
	l	g7,o5
	cmp	0,g6,g7
	bne	0,FINI
	l	g10,tmp
#
#### 6 ####
#
	st	g10,tmp
	ai	g3,g3,1				# RC = 6
	lfd	r7,i6fp
	mtfsf	0xff,r7
	lfd	r0,i6
	stfsux	r0,g10,g11
	l	g6,fpsave
	l	g7,o6
	cmp	0,g6,g7
	bne	0,FINI
	l	g10,tmp
#
#### 7 ####
#
	st	g10,tmp
	ai	g3,g3,1				# RC = 7
	lfd	r7,i7fp
	mtfsf	0xff,r7
	lfd	r0,i7
	stfsux	r0,g10,g11
	l	g6,fpsave
	l	g7,o7
	cmp	0,g6,g7
	bne	0,FINI
	l	g10,tmp
#
#### 8 ####
#
	st	g10,tmp
	ai	g3,g3,1				# RC = 8
	lfd	r7,i8fp
	mtfsf	0xff,r7
	lfd	r0,i8
	stfsux	r0,g10,g11
	l	g6,fpsave
	l	g7,o8
	cmp	0,g6,g7
	bne	0,FINI
	l	g10,tmp
#
#### 9 ####
#
	st	g10,tmp
	ai	g3,g3,1				# RC = 9
	lfd	r7,i9fp
	mtfsf	0xff,r7
	lfd	r0,i9
	stfsux	r0,g10,g11
	l	g6,fpsave
	l	g7,o9
	cmp	0,g6,g7
	bne	0,FINI
	l	g10,tmp
#
#### 10 ####
#
	cmpi	0,g8,POWER_MP_A_MODEL_CODE
	bne	0,OTHER1
	st	g10,tmp
	ai	g3,g3,1				# RC = 10
	lfd	r7,i10fp
	mtfsf	0xff,r7
	lfd	r0,i10
	stfsux	r0,g10,g11
	l	g6,fpsave
	l	g7,o10p
	cmp	0,g6,g7
	bne	0,FINI
	l	g10,tmp
OTHER1:
#	st	g10,tmp
 	ai	g3,g3,1				# RC = 10
#	lfd	r7,i10fp
#	mtfsf	0xff,r7
#	lfd	r0,i10
#	stfsux	r0,g10,g11
#	l	g6,fpsave
#	l	g7,o10
#	cmp	0,g6,g7
#	bne	0,FINI
#	l	g10,tmp
#
#### 11 ####
#
	st	g10,tmp
	ai	g3,g3,1				# RC = 11
	lfd	r7,i11fp
	mtfsf	0xff,r7
	lfd	r0,i11
	stfsux	r0,g10,g11
	l	g6,fpsave
	l	g7,o11
	cmp	0,g6,g7
	bne	0,FINI
	l	g10,tmp
#
#### 12 ####
#
	st	g10,tmp
	ai	g3,g3,1				# RC = 12
	lfd	r7,i12fp
	mtfsf	0xff,r7
	lfd	r0,i12
	stfsux	r0,g10,g11
	l	g6,fpsave
	l	g7,o12
	cmp	0,g6,g7
	bne	0,FINI
	l	g10,tmp
#
#### 13 ####
#
	st	g10,tmp
	ai	g3,g3,1				# RC = 13
	lfd	r7,i13fp
	mtfsf	0xff,r7
	lfd	r0,i13
	stfsux	r0,g10,g11
	l	g6,fpsave
	l	g7,o13
	cmp	0,g6,g7
	bne	0,FINI
	l	g10,tmp
#
#### 14 ####
#
	st	g10,tmp
	ai	g3,g3,1				# RC = 14
	lfd	r7,i14fp
	mtfsf	0xff,r7
	lfd	r0,i14
	stfsux	r0,g10,g11
	l	g6,fpsave
	l	g7,o14
	cmp	0,g6,g7
	bne	0,FINI
	l	g10,tmp
#
#### 15 ####
#
	st	g10,tmp
	ai	g3,g3,1				# RC = 15
	lfd	r7,i15fp
	mtfsf	0xff,r7
	lfd	r0,i15
	stfsux	r0,g10,g11
	l	g6,fpsave
	l	g7,o15
	cmp	0,g6,g7
	bne	0,FINI
	l	g10,tmp
#
#### 16 ####
#
	cmpi	0,g8,POWER_MP_A_MODEL_CODE
	bne	0,OTHER2
	st	g10,tmp
	ai	g3,g3,1				# RC = 16
	lfd	r7,i16fp
	mtfsf	0xff,r7
	lfd	r0,i16
	stfsux	r0,g10,g11
	l	g6,fpsave
	l	g7,o16p
	cmp	0,g6,g7
	bne	0,FINI
	l	g10,tmp
OTHER2:
#	st	g10,tmp
 	ai	g3,g3,1				# RC = 16
#	lfd	r7,i16fp
#	mtfsf	0xff,r7
#	lfd	r0,i16
#	stfsux	r0,g10,g11
#	l	g6,fpsave
#	l	g7,o16
#	cmp	0,g6,g7
#	bne	0,FINI
#	l	g10,tmp
#
#### 17 ####
#
	st	g10,tmp
	ai	g3,g3,1				# RC = 17
	lfd	r7,i17fp
	mtfsf	0xff,r7
	lfd	r0,i17
	stfsux	r0,g10,g11
	l	g6,fpsave
	l	g7,o17
	cmp	0,g6,g7
	bne	0,FINI
	l	g10,tmp
#
#### 18 ####
#
	st	g10,tmp
	ai	g3,g3,1				# RC = 18
	lfd	r7,i18fp
	mtfsf	0xff,r7
	lfd	r0,i18
	stfsux	r0,g10,g11
	l	g6,fpsave
	l	g7,o18
	cmp	0,g6,g7
	bne	0,FINI
	l	g10,tmp
#
#### 19 ####
#
	st	g10,tmp
	ai	g3,g3,1				# RC = 19
	lfd	r7,i19fp
	mtfsf	0xff,r7
	lfd	r0,i19
	stfsux	r0,g10,g11
	l	g6,fpsave
	l	g7,o19
	cmp	0,g6,g7
	bne	0,FINI
	l	g10,tmp
#
#### 20 ####
#
	st	g10,tmp
	ai	g3,g3,1				# RC = 20
	lfd	r7,i20fp
	mtfsf	0xff,r7
	lfd	r0,i20
	stfsux	r0,g10,g11
	l	g6,fpsave
	l	g7,o20
	cmp	0,g6,g7
	bne	0,FINI
	l	g10,tmp
#
#### 21 ####
#
	cmpi	0,g8,POWER_MP_A_MODEL_CODE
	bne	0,OTHER3
	st	g10,tmp
	ai	g3,g3,1				# RC = 21
	lfd	r7,i21fp
	mtfsf	0xff,r7
	lfd	r0,i21
	stfsux	r0,g10,g11
	l	g6,fpsave
	l	g7,o21p
	cmp	0,g6,g7
	bne	0,FINI
	l	g10,tmp
OTHER3:
#	st	g10,tmp
 	ai	g3,g3,1				# RC = 21
#	lfd	r7,i21fp
#	mtfsf	0xff,r7
#	lfd	r0,i21
#	stfsux	r0,g10,g11
#	l	g6,fpsave
#	l	g7,o21
#	cmp	0,g6,g7
#	bne	0,FINI
#	l	g10,tmp
#
#### 22 ####
#
	st	g10,tmp
	ai	g3,g3,1				# RC = 22
	lfd	r7,i22fp
	mtfsf	0xff,r7
	lfd	r0,i22
	stfsux	r0,g10,g11
	l	g6,fpsave
	l	g7,o22
	cmp	0,g6,g7
	bne	0,FINI
	l	g10,tmp
#
#### 23 ####
#
	st	g10,tmp
	ai	g3,g3,1				# RC = 23
	lfd	r7,i23fp
	mtfsf	0xff,r7
	lfd	r0,i23
	stfsux	r0,g10,g11
	l	g6,fpsave
	l	g7,o23
	cmp	0,g6,g7
	bne	0,FINI
	l	g10,tmp
#
#### 24 ####
#
	st	g10,tmp
	ai	g3,g3,1				# RC = 24
	lfd	r7,i24fp
	mtfsf	0xff,r7
	lfd	r0,i24
	stfsux	r0,g10,g11
	l	g6,fpsave
	l	g7,o24
	cmp	0,g6,g7
	bne	0,FINI
	l	g10,tmp
#
#### 25 ####
#
	st	g10,tmp
	ai	g3,g3,1				# RC = 25
	lfd	r7,i25fp
	mtfsf	0xff,r7
	lfd	r0,i25
	stfsux	r0,g10,g11
	l	g6,fpsave
	l	g7,o25
	cmp	0,g6,g7
	bne	0,FINI
	l	g10,tmp
#
#### 26 ####
#
	st	g10,tmp
	ai	g3,g3,1				# RC = 26
	lfd	r7,i26fp
	mtfsf	0xff,r7
	lfd	r0,i26
	stfsux	r0,g10,g11
	l	g6,fpsave
	l	g7,o26
	cmp	0,g6,g7
	bne	0,FINI
	l	g10,tmp
#
#### 27 ####
#
	cmpi	0,g8,POWER_MP_A_MODEL_CODE
	bne	0,OTHER4
	st	g10,tmp
	ai	g3,g3,1				# RC = 27
	lfd	r7,i27fp
	mtfsf	0xff,r7
	lfd	r0,i27
	stfsux	r0,g10,g11
	l	g6,fpsave
	l	g7,o27p
	cmp	0,g6,g7
	bne	0,FINI
	l	g10,tmp
OTHER4:
#	st	g10,tmp
 	ai	g3,g3,1				# RC = 27
#	lfd	r7,i27fp
#	mtfsf	0xff,r7
#	lfd	r0,i27
#	stfsux	r0,g10,g11
#	l	g6,fpsave
#	l	g7,o27
#	cmp	0,g6,g7
#	bne	0,FINI
#	l	g10,tmp
#
	xor	g3,g3,g3		# Return code = 0; pass
#
FINI:	br
	S_EPILOG
#
# Data Area
#
	.csect	_mstfsux[RW],3
_mstfsux: 
fpsave:	.long	0xffffffff, 0xffffffff
i1:	.long	0x377db0aa, 0xc80d5af1
o1:	.long	0x00003b61
i1fp:	.long	0xffffffff, 0x00000049
i2:	.long	0x905c1f41, 0x0e636974
o2:	.long	0x80000000
i2fp:	.long	0xffffffff, 0x00000048
i3:	.long	0xd7000000, 0x00000000
o3:	.long	0xf8000000
i3fp:	.long	0xffffffff, 0x000000bb
i4:	.long	0x3f74faf4, 0x8c9b5cdf
o4:	.long	0x3ba7d7a4
i4fp:	.long	0xffffffff, 0x000000ab
i5:	.long	0x381fffff, 0xfd85284d
o5:	.long	0x00ffffff
i5fp:	.long	0xffffffff, 0x000000a0
i6:	.long	0x944fffff, 0xffff494c
o6:	.long	0x80000000
i6fp:	.long	0xffffffff, 0x00000058
i7:	.long	0xffff574e, 0x4e2a25fd
o7:	.long	0xfffaba72
i7fp:	.long	0xffffffff, 0x000000a2
i8:	.long	0x800fffff, 0xffffffff
o8:	.long	0x80000000
i8fp:	.long	0xffffffff, 0x00000043
i9:	.long	0xfff61577, 0x18ece065
o9:	.long	0xffb0abb8
i9fp:	.long	0xffffffff, 0x00000050
i10:	.long	0x36afffd1, 0x89bf341a
o10:	.long	0x357ffe8c
o10p:	.long	0x00000001		# Chip 601
i10fp:	.long	0xffffffff, 0x000000e8
i11:	.long	0xfff00000, 0x00000000
o11:	.long	0xff800000
i11fp:	.long	0xffffffff, 0x000000ea
i12:	.long	0x36afffff, 0xff4d11e1
o12:	.long	0x00000001
i12fp:	.long	0xffffffff, 0x00000012
i13:	.long	0x7fefffff, 0xffffffff
o13:	.long	0x7f7fffff
i13fp:	.long	0xffffffff, 0x0000009a
i14:	.long	0xfff00000, 0x00000000
o14:	.long	0xff800000
i14fp:	.long	0xffffffff, 0x00000052
i15:	.long	0xfff80000, 0x00000000
o15:	.long	0xffc00000
i15fp:	.long	0xffffffff, 0x00000061
i16:	.long	0x00000003, 0xcaa284e7
o16:	.long	0x0000001e
o16p:	.long	0x00000000		# Chip 601
i16fp:	.long	0xffffffff, 0x000000fb
i17:	.long	0x7ffeba1c, 0x1f29f495
o17:	.long	0x7ff5d0e0
i17fp:	.long	0xffffffff, 0x000000c1
i18:	.long	0x380ffffd, 0x9fdf4f1a
o18:	.long	0x007ffff6
i18fp:	.long	0xffffffff, 0x0000004a
i19:	.long	0xb6afffe3, 0xf315b61f
o19:	.long	0x80000001
i19fp:	.long	0xffffffff, 0x00000002
i20:	.long	0xb81fffff, 0xffff6f79
o20:	.long	0x80ffffff
i20fp:	.long	0xffffffff, 0x000000a8
i21:	.long	0x000fffff, 0xffffffff
o21:	.long	0x007fffff
o21p:	.long	0x00000000		# Chip 601
i21fp:	.long	0xffffffff, 0x000000f8
i22:	.long	0x380fffff, 0xffe92acf
o22:	.long	0x007fffff
i22fp:	.long	0xffffffff, 0x00000039
i23:	.long	0x381fffff, 0xfffaaade
o23:	.long	0x00ffffff
i23fp:	.long	0xffffffff, 0x00000073
i24:	.long	0x7ff143c9, 0x77c839da
o24:	.long	0x7f8a1e4b
i24fp:	.long	0xffffffff, 0x00000018
i25:	.long	0x80000000, 0x00c02227
o25:	.long	0x80000000
i25fp:	.long	0xffffffff, 0x000000e1
i26:	.long	0xb80fffff, 0xf666de49
o26:	.long	0x807fffff
i26fp:	.long	0xffffffff, 0x000000e1
i27:	.long	0x000fffff, 0xffffffe9
o27:	.long	0x007fffff
o27p:	.long	0x00000000		# Chip 601
i27fp:	.long	0xffffffff, 0x000000fb
tmp:	.long	0xffffffff
#
	.toc
	TOCE(.mstfsux, entry)
	TOCL(_mstfsux, data)
T.pegas:.tc     pegas[tc],pegas
	.extern pegas
