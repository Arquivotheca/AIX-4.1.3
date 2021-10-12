# @(#)00	1.3  src/bos/diag/da/fpa/all.s, dafpa, bos411, 9428A410j 3/23/94 06:20:02

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
	.csect	.all[PR]
	.globl	.all[PR]
	.align	2
.all:
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
	LTOC(4, _all, data)		# Loads g4 with the address of _all
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
 	.using	_all[RW],g4		# g4 is the base register to the data
#	 				     section _all[rw]. Eastablishing 
#					     @ability
#
#	
	xor	g3,g3,g3		# g3 = 0. Initialize return code
#
##########################
#
	lfd	rf1,i1fp
	mtfsf	0xff,rf1
#
	lfd	rf0,i1f0
	lfd	rf30,i1f30
#
	fs	r12,r0,r30
#
	ai	g3,g3,1				# RC = 1
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o1fp
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 2
	stfd	rf12,fpsave
	l	g6,fpsave
	l	g7,o1f12
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 3
	l	g6,fpsave+4
	l	g7,o1f12+4
	cmp	0,g6,g7
	bne	FINI
#
##########################
#
	lfd	rf1,i2fp
	mtfsf	0xff,rf1
#
	lfd	rf22,i2f22
#
	fs	r22,r22,r22
#
	ai	g3,g3,1				# RC = 4
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o2fp
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 5
	stfd	rf22,fpsave
	l	g6,fpsave
	l	g7,o2f22
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 6
	l	g6,fpsave+4
	l	g7,o2f22+4
	cmp	0,g6,g7
	bne	FINI
#
##########################
#
	l	g9,i3cr
	mtcrf	0xff,g9
	lfd	rf1,i3fp
	mtfsf	0xff,rf1
#
	lfd	rf27,i3f27
	lfd	rf12,i3f12
#
	fs.	r30,r27,r12
#
	ai	g3,g3,1				# RC = 7
	mfcr	g6
	l	g7,o3cr
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 8
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o3fp
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 9
	stfd	rf30,fpsave
	l	g6,fpsave
	l	g7,o3f30
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 10
	l	g6,fpsave+4
	l	g7,o3f30+4
	cmp	0,g6,g7
	bne	FINI
#
##########################
#
	l	g9,i4cr
	mtcrf	0xff,g9
	lfd	rf1,i4fp
	mtfsf	0xff,rf1
#
	lfd	rf2,i4f2
	lfd	rf17,i4f17
#
	fs.	r23,r17,r2
#
	ai	g3,g3,1				# RC = 11
	mfcr	g6
	l	g7,o4cr
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 12
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o4fp
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 13
	stfd	rf23,fpsave
	l	g6,fpsave
	l	g7,o4f23
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 14
	l	g6,fpsave+4
	l	g7,o4f23+4
	cmp	0,g6,g7
	bne	FINI
#
##########################
#
	l	g9,i5cr
	mtcrf	0xff,g9
	lfd	rf1,i5fp
	mtfsf	0xff,rf1
#
	lfd	rf22,i5f22
	lfd	rf15,i5f15
#
	fs.	r11,r22,r15
#
	ai	g3,g3,1				# RC = 15
	mfcr	g6
	l	g7,o5cr
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 16
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o5fp
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 17
	stfd	rf11,fpsave
	l	g6,fpsave
	l	g7,o5f11
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 18
	l	g6,fpsave+4
	l	g7,o5f11+4
	cmp	0,g6,g7
	bne	FINI
#
##########################
#
	l	g9,i6cr
	mtcrf	0xff,g9
	lfd	rf1,i6fp
	mtfsf	0xff,rf1
#
	lfd	rf1,i6f1
	lfd	rf2,i6f2
#
	fs.	r23,r1,r2
#
	ai	g3,g3,1				# RC = 19
	mfcr	g6
	l	g7,o6cr
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 20
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o6fp
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 21
	stfd	rf23,fpsave
	l	g6,fpsave
	l	g7,o6f23
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 22
	l	g6,fpsave+4
	l	g7,o6f23+4
	cmp	0,g6,g7
	bne	FINI
#
##########################
#
	l	g9,i7cr
	mtcrf	0xff,g9
	lfd	rf1,i7fp
	mtfsf	0xff,rf1
#
	lfd	rf9,i7f9
	lfd	rf27,i7f27
#
	fcmpu	7,r9,r27
#
	ai	g3,g3,1				# RC = 23
	mfcr	g6
	l	g7,o7cr
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 24
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o7fp
	cmp	0,g6,g7
	bne	FINI
#
##########################
#
	l	g9,i8cr
	mtcrf	0xff,g9
	lfd	rf1,i8fp
	mtfsf	0xff,rf1
#
	lfd	rf4,i8f4
	lfd	rf11,i8f11
#
	fcmpu	0,r4,r11
#
	ai	g3,g3,1				# RC = 25
	mfcr	g6
	l	g7,o8cr
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 26
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o8fp
	cmp	0,g6,g7
	bne	FINI
#
##########################
#
#	l	g9,i9cr
#	mtcrf	0xff,g9
#	lfd	rf1,i9fp
#	mtfsf	0xff,rf1
##
#	mcrfs	4,8
##
#	ai	g3,g3,1				# RC = 27
#	mfcr	g6
#	l	g7,o9cr
#	cmp	0,g6,g7
#	bne	FINI
##
#	ai	g3,g3,1				# RC = 28
#	mffs	rf7
#	stfd	rf7,fpsave
#	l	g6,fpsave+4
#	l	g7,o9fp
#	cmp	0,g6,g7
#	bne	FINI
#
##########################
#
	l	g9,i10cr
	mtcrf	0xff,g9
	lfd	rf1,i10fp
	mtfsf	0xff,rf1
#
	mcrfs	7,2
#
	ai	g3,g3,1				# RC = 27
	mfcr	g6
	l	g7,o10cr
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 28
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o10fp
	cmp	0,g6,g7
	bne	FINI
#
##########################
#
#	l	g9,i11cr
#	mtcrf	0xff,g9
#	lfd	rf1,i11fp
#	mtfsf	0xff,rf1
##
#	mcrfs.	5,3
##
#	ai	g3,g3,1				# RC = 31
#	mfcr	g6
#	l	g7,o11cr
#	cmp	0,g6,g7
#	bne	FINI
##
#	ai	g3,g3,1				# RC = 32
#	mffs	rf7
#	stfd	rf7,fpsave
#	l	g6,fpsave+4
#	l	g7,o11fp
#	cmp	0,g6,g7
#	bne	FINI
#
##########################
#
	l	g9,i12cr
	mtcrf	0xff,g9
	lfd	rf1,i12fp
	mtfsf	0xff,rf1
#
	lfd	rf5,i12f5
	lfd	rf9,i12f9
#
	fcmpo	3,r5,r9
#
	ai	g3,g3,1				# RC = 29
	mfcr	g6
	l	g7,o12cr
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 30
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o12fp
	cmp	0,g6,g7
	bne	FINI
#
##########################
#
#	l	g9,i13cr
#	mtcrf	0xff,g9
#	lfd	rf1,i13fp
#	mtfsf	0xff,rf1
##
#	lfd	rf9,i13f9
#	lfd	rf22,i13f22
##
#	fcmpo.	5,r9,r22
##
#	ai	g3,g3,1				# RC = 35
#	mfcr	g6
#	l	g7,o13cr
#	cmp	0,g6,g7
#	bne	FINI
##
#	ai	g3,g3,1				# RC = 36
#	mffs	rf7
#	stfd	rf7,fpsave
#	l	g6,fpsave+4
#	l	g7,o13fp
#	cmp	0,g6,g7
#	bne	FINI
#
##########################
#
#	lfd	rf1,i14fp
#	mtfsf	0xff,rf1
##
#	lfd	rf5,i14f5
#	lfd	rf23,i14f23
#	lfd	rf2,i14f2
##
#	fnma	r29,r5,r23,r2
##
#	ai	g3,g3,1				# RC = 38
#	mffs	rf7
#	stfd	rf7,fpsave
#	l	g6,fpsave+4			### get fpscr=820680a0
#	l	g7,o14fp
#	cmp	0,g6,g7
#	bne	FINI
#
#	ai	g3,g3,1				# RC = 39
#	stfd	rf29,fpsave
#	l	g6,fpsave
#	l	g7,o14f29
#	cmp	0,g6,g7
#	bne	FINI
#
#	ai	g3,g3,1				# RC = 40
#	l	g6,fpsave+4
#	l	g7,o14f29+4
#	cmp	0,g6,g7
#	bne	FINI
#
##########################
#
#	lfd	rf1,i15fp
#	mtfsf	0xff,rf1
##
#	lfd	rf1,i15f1
#	lfd	rf31,i15f31
#	lfd	rf12,i15f12
##
#	fnma	r13,r1,r31,r12
##
#	ai	g3,g3,1				# RC = 42
#	mffs	rf7
#	stfd	rf7,fpsave
#	l	g6,fpsave+4
#	l	g7,o15fp
#	cmp	0,g6,g7
#	bne	FINI
##
#	ai	g3,g3,1				# RC = 43
#	stfd	rf13,fpsave
#	l	g6,fpsave
#	l	g7,o15f13
#	cmp	0,g6,g7
#	bne	FINI
#
#	ai	g3,g3,1				# RC = 44
#	l	g6,fpsave+4
#	l	g7,o15f13+4
#	cmp	0,g6,g7
#	bne	FINI
#
##########################
##
#	l	g9,i16cr
#	mtcrf	0xff,g9
#	lfd	rf1,i16fp
#	mtfsf	0xff,rf1
##
#	lfd	rf6,i16f6
#	lfd	rf10,i16f10
#	lfd	rf15,i16f15
##
#	fnma.	r17,r6,r10,r15
##
#	ai	g3,g3,1				# RC = 31
#	mfcr	g6
#	l	g7,o16cr
#	cmp	0,g6,g7
#	bne	FINI
##
#	ai	g3,g3,1				# RC = 32
#	mffs	rf7
#	stfd	rf7,fpsave
#	l	g6,fpsave+4
#	l	g7,o16fp
#	cmp	0,g6,g7
#	bne	FINI
##
#	ai	g3,g3,1				# RC = 33
#	stfd	rf17,fpsave
#	l	g6,fpsave
#	l	g7,o16f17
#	cmp	0,g6,g7
#	bne	FINI
##
#	ai	g3,g3,1				# RC = 34
#	l	g6,fpsave+4
#	l	g7,o16f17+4
#	cmp	0,g6,g7
#	bne	FINI
#
##########################
##
#	l	g9,i17cr
#	mtcrf	0xff,g9
#	lfd	rf1,i17fp
#	mtfsf	0xff,rf1
##
#	lfd	rf28,i17f28
#	lfd	rf18,i17f18
#	lfd	rf13,i17f13
##
#	fnma.	r15,r28,r18,r13
##
#	ai	g3,g3,1				# RC = 35
#	mfcr	g6
#	l	g7,o17cr
#	cmp	0,g6,g7
#	bne	FINI
##
#	ai	g3,g3,1				# RC = 36
#	mffs	rf7
#	stfd	rf7,fpsave
#	l	g6,fpsave+4
#	l	g7,o17fp
#	cmp	0,g6,g7
#	bne	FINI
##
#	ai	g3,g3,1				# RC = 37
#	stfd	rf15,fpsave
#	l	g6,fpsave
#	l	g7,o17f15
#	cmp	0,g6,g7
#	bne	FINI
##
#	ai	g3,g3,1				# RC = 38
#	l	g6,fpsave+4
#	l	g7,o17f15+4
#	cmp	0,g6,g7
#	bne	FINI
##
##########################
##
#	l	g9,i18cr
#	mtcrf	0xff,g9
#	lfd	rf1,i18fp
#	mtfsf	0xff,rf1
##
#	lfd	rf2,i18f2
#	lfd	rf0,i18f0
#	lfd	rf22,i18f22
##
#	fnma.	r13,r2,r0,r22
##
#	ai	g3,g3,1				# RC = 39
#	mfcr	g6
#	l	g7,o18cr
#	cmp	0,g6,g7
#	bne	FINI
##
#	ai	g3,g3,1				# RC = 40
#	mffs	rf7
#	stfd	rf7,fpsave
#	l	g6,fpsave+4
#	l	g7,o18fp
#	cmp	0,g6,g7
#	bne	FINI
##
#	ai	g3,g3,1				# RC = 41
#	stfd	rf13,fpsave
#	l	g6,fpsave
#	l	g7,o18f13
#	cmp	0,g6,g7
#	bne	FINI
##
#	ai	g3,g3,1				# RC = 42
#	l	g6,fpsave+4
#	l	g7,o18f13+4
#	cmp	0,g6,g7
#	bne	FINI
##
########################## 
#
	lfd	rf1,i19fp
	mtfsf	0xff,rf1
#
	lfd	rf17,i19f17
	lfd	rf29,i19f29
	lfd	rf24,i19f24
#
	fnms	r15,r17,r29,r24
#
	ai	g3,g3,1				# RC = 43
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o19fp
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 44
	stfd	rf15,fpsave
	l	g6,fpsave
	l	g7,o19f15
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 45
	l	g6,fpsave+4
	l	g7,o19f15+4
	cmp	0,g6,g7
	bne	FINI
#
##########################
#
	l	g9,i20cr
	mtcrf	0xff,g9
	lfd	rf1,i20fp
	mtfsf	0xff,rf1
#
	lfd	rf7,i20f7
	lfd	rf14,i20f14
	lfd	rf19,i20f19
#
	fnms.	r31,r7,r14,r19
#
	ai	g3,g3,1				# RC = 46
	mfcr	g6
	l	g7,o20cr
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 47
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o20fp
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 48
	stfd	rf31,fpsave
	l	g6,fpsave
	l	g7,o20f31
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 49
	l	g6,fpsave+4
	l	g7,o20f31+4
	cmp	0,g6,g7
	bne	FINI
#
##########################
##
#	l	g9,i21cr
#	mtcrf	0xff,g9
#	lfd	rf1,i21fp
#	mtfsf	0xff,rf1
##
#	lfd	rf0,i21f0
#	lfd	rf13,i21f13
#	lfd	rf26,i21f26
##
#	fnms.	r11,r0,r13,r26
##
#	ai	g3,g3,1				# RC = 50
#	mfcr	g6
#	l	g7,o21cr
#	cmp	0,g6,g7
#	bne	FINI
##
#	ai	g3,g3,1				# RC = 51
#	mffs	rf7
#	stfd	rf7,fpsave
#	l	g6,fpsave+4
#	l	g7,o21fp
#	cmp	0,g6,g7
#	bne	FINI
##
#	ai	g3,g3,1				# RC = 52
#	stfd	rf11,fpsave
#	l	g6,fpsave
#	l	g7,o21f11
#	cmp	0,g6,g7
#	bne	FINI
##
#	ai	g3,g3,1				# RC = 53
#	l	g6,fpsave+4
#	l	g7,o21f11+4
#	cmp	0,g6,g7
#	bne	FINI
##
##########################
##
#	l	g9,i22cr
#	mtcrf	0xff,g9
#	lfd	rf1,i22fp
#	mtfsf	0xff,rf1
##
#	lfd	rf1,i22f1
#	lfd	rf3,i22f3
#	lfd	rf17,i22f17
##
#	fnms.	r13,r1,r3,r17
##
#	ai	g3,g3,1				# RC = 54
#	mfcr	g6
#	l	g7,o22cr
#	cmp	0,g6,g7
#	bne	FINI
##
#	ai	g3,g3,1				# RC = 55
#	mffs	rf7
#	stfd	rf7,fpsave
#	l	g6,fpsave+4
#	l	g7,o22fp
#	cmp	0,g6,g7
#	bne	FINI
##
#	ai	g3,g3,1				# RC = 56
#	stfd	rf13,fpsave
#	l	g6,fpsave
#	l	g7,o22f13
#	cmp	0,g6,g7
#	bne	FINI
##
#	ai	g3,g3,1				# RC = 57
#	l	g6,fpsave+4
#	l	g7,o22f13+4
#	cmp	0,g6,g7
#	bne	FINI
##
##########################
#
	l	g9,i23cr
	mtcrf	0xff,g9
	lfd	rf1,i23fp
	mtfsf	0xff,rf1
#
	lfd	rf0,i23f0
	lfd	rf22,i23f22
#
	fabs.	r22,r0
#
	ai	g3,g3,1				# RC = 58
	mfcr	g6
	l	g7,o23cr
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 59
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o23fp
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 60
	stfd	rf22,fpsave
	l	g6,fpsave
	l	g7,o23f22
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 61
	l	g6,fpsave+4
	l	g7,o23f22+4
	cmp	0,g6,g7
	bne	FINI
#
##########################
#
	l	g9,i24cr
	mtcrf	0xff,g9
	lfd	rf1,i24fp
	mtfsf	0xff,rf1
#
	lfd	rf21,i24f21
	lfd	rf17,i24f17
#
	fabs.	r21,r17
#
	ai	g3,g3,1				# RC = 62
	mfcr	g6
	l	g7,o24cr
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 63
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o24fp
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 64
	stfd	rf21,fpsave
	l	g6,fpsave
	l	g7,o24f21
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 65
	l	g6,fpsave+4
	l	g7,o24f21+4
	cmp	0,g6,g7
	bne	FINI
#
##########################
#
	lfd	rf1,i25fp
	mtfsf	0xff,rf1
#
	lfd	rf22,i25f22
	lfd	rf27,i25f27
#
	fneg	r22,r27
#
	ai	g3,g3,1				# RC = 66
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o25fp
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 67
	stfd	rf22,fpsave
	l	g6,fpsave
	l	g7,o25f22
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 68
	l	g6,fpsave+4
	l	g7,o25f22+4
	cmp	0,g6,g7
	bne	FINI
#
##########################
#
	lfd	rf1,i26fp
	mtfsf	0xff,rf1
#
	lfd	rf20,i26f20
	lfd	rf24,i26f24
#
	fmr	r20,r24
#
	ai	g3,g3,1				# RC = 69
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o26fp
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 70
	stfd	rf20,fpsave
	l	g6,fpsave
	l	g7,o26f20
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 71
	l	g6,fpsave+4
	l	g7,o26f20+4
	cmp	0,g6,g7
	bne	FINI
#
##########################
#
	l	g9,i27cr
	mtcrf	0xff,g9
	lfd	rf1,i27fp
	mtfsf	0xff,rf1
#
	lfd	rf2,i27f2
	lfd	rf17,i27f17
#
	fmr.	r2,r17
#
	ai	g3,g3,1				# RC = 72
	mfcr	g6
	l	g7,o27cr
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 73
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o27fp
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 74
	stfd	rf2,fpsave
	l	g6,fpsave
	l	g7,o27f2
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 75
	l	g6,fpsave+4
	l	g7,o27f2+4
	cmp	0,g6,g7
	bne	FINI
#
##########################
#
	lfd	rf1,i28fp
	mtfsf	0xff,rf1
#
	mffs	r4
#
	ai	g3,g3,1				# RC = 76
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o28fp
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 77
	stfd	rf4,fpsave
	l	g6,fpsave
	l	g7,o28f4
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 78
	l	g6,fpsave+4
	l	g7,o28f4+4
	cmp	0,g6,g7
	bne	FINI
#
##########################
#
	l	g9,i29cr
	mtcrf	0xff,g9
	lfd	rf1,i29fp
	mtfsf	0xff,rf1
#
	mffs.	r3
#
	ai	g3,g3,1				# RC = 79
	mfcr	g6
	l	g7,o29cr
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 80
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o29fp
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 81
	stfd	rf3,fpsave
	l	g6,fpsave
	l	g7,o29f3
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 82
	l	g6,fpsave+4
	l	g7,o29f3+4
	cmp	0,g6,g7
	bne	FINI
#
##########################
#
	l	g9,i30cr
	mtcrf	0xff,g9
	lfd	rf1,i30fp
	mtfsf	0xff,rf1
#
	mtfsfi.	0,5
#
	ai	g3,g3,1				# RC = 83
	mfcr	g6
	l	g7,o30cr
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 84
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o30fp
	cmp	0,g6,g7
	bne	FINI
#
##########################
#
#	l	g9,i31cr
#	mtcrf	0xff,g9
#	lfd	rf1,i31fp
#	mtfsf	0xff,rf1
##
#	mtfsfi.	3,0x000c
##
#	ai	g3,g3,1				# RC = 99
#	mfcr	g6
#	l	g7,o31cr
#	cmp	0,g6,g7
#	bne	FINI
##
#	ai	g3,g3,1				# RC = 100
#	mffs	rf7
#	stfd	rf7,fpsave
#	l	g6,fpsave+4
#	l	g7,o31fp
#	cmp	0,g6,g7
#	bne	FINI
#
##########################
#
#	l	g9,i32cr
#	mtcrf	0xff,g9
#	lfd	rf1,i32fp
#	mtfsf	0xff,rf1
##
#	mtfsfi.	0,0x000f
##
#	ai	g3,g3,1				# RC = 101
#	mfcr	g6
#	l	g7,o32cr
#	cmp	0,g6,g7
#	bne	FINI
##
#	ai	g3,g3,1				# RC = 102
#	mffs	rf7
#	stfd	rf7,fpsave
#	l	g6,fpsave+4
#	l	g7,o32fp
#	cmp	0,g6,g7
#	bne	FINI
#
##########################
#
	l	g9,i33cr
	mtcrf	0xff,g9
	lfd	rf1,i33fp
	mtfsf	0xff,rf1
#
	mtfsfi.	0,6
#
	ai	g3,g3,1				# RC = 85
	mfcr	g6
	l	g7,o33cr
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 86
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o33fp
	cmp	0,g6,g7
	bne	FINI
#
##########################
#
	l	g9,i34cr
	mtcrf	0xff,g9
	lfd	rf1,i34fp
	mtfsf	0xff,rf1
#
	mtfsfi.	3,6
#
	ai	g3,g3,1				# RC = 87
	mfcr	g6
	l	g7,o34cr
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 88
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o34fp
	cmp	0,g6,g7
	bne	FINI
#
##########################
#
	lfd	rf1,i35fp
	mtfsf	0xff,rf1
#
	mtfsb1	0
#
	ai	g3,g3,1				# RC = 89
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o35fp
	cmp	0,g6,g7
	bne	FINI
#
##########################
#
	l	g9,i36cr
	mtcrf	0xff,g9
	lfd	rf1,i36fp
	mtfsf	0xff,rf1
#
	mtfsb1.	17
#
	ai	g3,g3,1				# RC = 90
	mfcr	g6
	l	g7,o36cr
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 91
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o36fp
	cmp	0,g6,g7
	bne	FINI
#
##########################
#
	l	g9,i37cr
	mtcrf	0xff,g9
	lfd	rf1,i37fp
	mtfsf	0xff,rf1
#
	mtfsb1.	3
#
	ai	g3,g3,1				# RC = 92
	mfcr	g6
	l	g7,o37cr
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 93
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o37fp
	cmp	0,g6,g7
	bne	FINI
#
##########################
#
	l	g9,i38cr
	mtcrf	0xff,g9
	lfd	rf1,i38fp
	mtfsf	0xff,rf1
#
	mtfsb1.	3
#
	ai	g3,g3,1				# RC = 94
	mfcr	g6
	l	g7,o38cr
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 95
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o38fp
	cmp	0,g6,g7
	bne	FINI
#
##########################
#
	lfd	rf1,i39fp
	mtfsf	0xff,rf1
#
	mtfsb0	0
#
	ai	g3,g3,1				# RC = 96
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o39fp
	cmp	0,g6,g7
	bne	FINI
#
##########################
#
	l	g9,i40cr
	mtcrf	0xff,g9
	lfd	rf1,i40fp
	mtfsf	0xff,rf1
#
	mtfsb0.	27
#
	ai	g3,g3,1				# RC = 97
	mfcr	g6
	l	g7,o40cr
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 98
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o40fp
	cmp	0,g6,g7
	bne	FINI
#
##########################
#
	l	g9,i41cr
	mtcrf	0xff,g9
	lfd	rf1,i41fp
	mtfsf	0xff,rf1
#
	mtfsb0.	4
#
	ai	g3,g3,1				# RC = 99
	mfcr	g6
	l	g7,o41cr
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 100
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o41fp
	cmp	0,g6,g7
	bne	FINI
#
##########################
#
	l	g9,i42cr
	mtcrf	0xff,g9
	lfd	rf1,i42fp
	mtfsf	0xff,rf1
#
	mtfsb0.	23
#
	ai	g3,g3,1				# RC = 101
	mfcr	g6
	l	g7,o42cr
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 102
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o42fp
	cmp	0,g6,g7
	bne	FINI
#
##########################
##
#	lfd	rf1,i43fp
#	mtfsf	0xff,rf1
##
#	lfd	rf9,i43f9
#	lfd	rf20,i43f20
#	lfd	rf26,i43f26
##
#	fms	r11,r9,r20,r26
##
#	ai	g3,g3,1				# RC = 103
#	mffs	rf7
#	stfd	rf7,fpsave
#	l	g6,fpsave+4
#	l	g7,o43fp
#	cmp	0,g6,g7
#	bne	FINI
##
#	ai	g3,g3,1				# RC = 104
#	stfd	rf11,fpsave
#	l	g6,fpsave
#	l	g7,o43f11
#	cmp	0,g6,g7
#	bne	FINI
##
#	ai	g3,g3,1				# RC = 105
#	l	g6,fpsave+4
#	l	g7,o43f11+4
#	cmp	0,g6,g7
#	bne	FINI
##
##########################
#
	l	g9,i44cr
	mtcrf	0xff,g9
	lfd	rf1,i44fp
	mtfsf	0xff,rf1
#
	lfd	rf28,i44f28
	lfd	rf16,i44f16
	lfd	rf13,i44f13
#
	fms.	r24,r28,r16,r13
#
	ai	g3,g3,1				# RC = 106
	mfcr	g6
	l	g7,o44cr
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 107
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o44fp
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 108
	stfd	rf24,fpsave
	l	g6,fpsave
	l	g7,o44f24
	cmp	0,g6,g7
	bne	FINI
#
	ai	g3,g3,1				# RC = 109
	l	g6,fpsave+4
	l	g7,o44f24+4
	cmp	0,g6,g7
	bne	FINI
#
#
	xor	g3,g3,g3		# Return code = 0; pass
#
FINI:	br
#
# Data Area
#
	.csect	_all[RW],3
_all: 
fpsave:	.long	0xffffffff, 0xffffffff
#						# fs  r12,r0,r30
i1f0:	.long	0x3ea3420b, 0x41ff7ef0		# f0 3ea3420b41ff7ef0
i1f30:	.long	0xfff00000, 0x00000000		# f30 fff0000000000000
i1fp:	.long	0xffffffff, 0x00000099			# fpscr 00000099
o1f12:	.long	0x7ff00000, 0x00000000		# f12 7ff0000000000000
o1fp:	.long	0x00005099			# fpscr 00005099
#
#						# fs  r22,r22,r22
i2f22:	.long	0xdd3801bf, 0x2e661654		# f22 dd3801bf2e661654
i2fp:	.long	0xffffffff, 0x0000006a			# fpscr 0000006a
o2f22:	.long	0x00000000, 0x00000000		# f22 0000000000000000
o2fp:	.long	0x0000206a			# fpscr 0000206a
#
#						# fsp  r30,r27,r12
i3f12:	.long	0x7ffc8cf3, 0x9a3abad3		# f12 7ffc8cf39a3abad3
i3f27:	.long	0x7ff57706, 0x7b60d98b		# f27 7ff577067b60d98b
i3cr:	.long	0xb003ac23			# cr b003ac23
i3fp:	.long	0xffffffff, 0x00000003			# fpscr 00000003
o3f30:	.long	0x7ffd7706, 0x7b60d98b		# f30 7ffd77067b60d98b
o3cr:	.long	0xba03ac23			# cr ba03ac23
o3fp:	.long	0xa1011003			# fpscr a1011003
#
#						# fsp  r23,r17,r2
i4f2:	.long	0xfff00000, 0x00000000		# f2 fff0000000000000
i4f17:	.long	0xfff3b95e, 0xc3528190		# f17 fff3b95ec3528190
i4cr:	.long	0x38edfe00			# cr 38edfe00
i4fp:	.long	0xffffffff, 0x00000032			# fpscr 00000032
o4f23:	.long	0xfffbb95e, 0xc3528190		# f23 fffbb95ec3528190
o4cr:	.long	0x3aedfe00			# cr 3aedfe00
o4fp:	.long	0xa1011032			# fpscr a1011032
#
#						# fsp  r11,r22,r15
i5f15:	.long	0x7fefffff, 0xffffffff		# f15 7fefffffffffffff
i5f22:	.long	0x7c900000, 0x00000000		# f22 7c90000000000000
i5cr:	.long	0xd15490c5			# cr d15490c5
i5fp:	.long	0xffffffff, 0x00000072			# fpscr 00000072
o5f11:	.long	0xffefffff, 0xfffffffe		# f11 ffeffffffffffffe
o5cr:	.long	0xd85490c5			# cr d85490c5
o5fp:	.long	0x82028072			# fpscr 82028072
#
#						# fsp  r23,r1,r2
i6f1:	.long	0x7fefffff, 0xfffffe00		# f1 7feffffffffffe00
i6f2:	.long	0xff5fffff, 0xffffffff		# f2 ff5fffffffffffff
i6cr:	.long	0xf437409a			# cr f437409a
i6fp:	.long	0xffffffff, 0x00000029			# fpscr 00000029
o6f23:	.long	0x7fefffff, 0xffffffff		# f23 7fefffffffffffff
o6cr:	.long	0xfd37409a			# cr fd37409a
o6fp:	.long	0xd2024029			# fpscr d2024029
#
#						# fcmpu  7,r9,r27
i7f9:	.long	0x7ff00000, 0x00000000		# f9 7ff0000000000000
i7f27:	.long	0x7ffea858, 0xf66e1537		# f27 7ffea858f66e1537
i7cr:	.long	0xd2d72266			# cr d2d72266
i7fp:	.long	0xffffffff, 0x000000c3			# fpscr 000000c3
o7cr:	.long	0xd2d72261			# cr d2d72261
o7fp:	.long	0x000010c3			# fpscr 000010c3
#
#						# fcmpu  0,r4,r11
i8f4:	.long	0xfc900000, 0x00000000		# f4 fc90000000000000
i8f11:	.long	0xffefffff, 0xffffffff		# f11 ffefffffffffffff
i8cr:	.long	0x34840e83			# cr 34840e83
i8fp:	.long	0xffffffff, 0x00000089		# fpscr 00000089
o8cr:	.long	0x44840e83			# cr 44840e83
o8fp:	.long	0x00004089			# fpscr 00004089
#
#						# mcrfs  4,8
i9cr:	.long	0x41e5fb99			# cr 41e5fb99
i9fp:	.long	0xffffffff, 0x000000e1		# fpscr 000000e1
o9cr:	.long	0x41e50b99			# cr 41e50b99
o9fp:	.long	0x000000e1			# fpscr 000000e1
#
#						# mcrfs  7,2
i10cr:	.long	0x8ed7f8c6			# cr 8ed7f8c6
i10fp:	.long	0xffffffff, 0xf7bb00e8		# fpscr f7bb00e8
o10cr:	.long	0x8ed7f8cb			# cr 8ed7f8cb
o10fp:	.long	0xf70b00e8			# fpscr f70b00e8
#
#						# mcrfsp  5,3
i11cr:	.long	0xcc792b56			# cr cc792b56
i11fp:	.long	0xffffffff, 0x604fd0ab		# fpscr 604fd0ab
o11cr:	.long	0xcc792f56			# cr cc792f56
o11fp:	.long	0x6047d0ab			# fpscr 6047d0ab
#
#						# fcmpo  3,r5,r9
i12f5:	.long	0x7ff16a6b, 0x503fda40		# f5 7ff16a6b503fda40
i12f9:	.long	0x7ff00000, 0x00000000		# f9 7ff0000000000000
i12cr:	.long	0x0debf9ad			# cr 0debf9ad
i12fp:	.long	0xffffffff, 0x00000042		# fpscr 00000042
o12cr:	.long	0x0de1f9ad			# cr 0de1f9ad
o12fp:	.long	0xa1081042			# fpscr a1081042
#
#						# fcmpop  5,r9,r22
i13f9:	.long	0xf3307bc1, 0xa798f4f0		# f9 f3307bc1a798f4f0
i13f22:	.long	0x7ffb41d3, 0x6b85e815		# f22 7ffb41d36b85e815
i13cr:	.long	0xc7895146			# cr c7895146
i13fp:	.long	0xffffffff, 0x000000e8		# fpscr 000000e8
o13cr:	.long	0xc7895146			# cr c7895146
o13fp:	.long	0xe00810e8			# fpscr e00810e8
#
#						# fnma  r29,r5,r23,r2
i14f2:	.long	0x1cb13779, 0x9d147390		# f2 1cb137799d147390
i14f5:	.long	0x1ca6723d, 0x138b5b73		# f5 1ca6723d138b5b73
i14f23:	.long	0x80000000, 0x00000001		# f23 8000000000000001
i14fp:	.long	0xffffffff, 0x000000a0		# fpscr 000000a0
o14f29:	.long	0x5cd00000, 0x00000000		# f29 5cd0000000000000
o14fp:	.long	0xca0640a0			# fpscr ca0640a0
#
#						# fnma  r13,r1,r31,r12
i15f1:	.long	0x083243f3, 0x37d8a5fe		# f1 083243f337d8a5fe
i15f12:	.long	0x6958614f, 0xa58389b7		# f12 6958614fa58389b7
i15f31:	.long	0x9ee854df, 0x8859660a		# f31 9ee854df8859660a
i15fp:	.long	0xffffffff, 0x0000007b		# fpscr 0000007b
o15f13:	.long	0xb19bd503, 0xb2bb53e7		# f13 b19bd503b2bb53e7
o15fp:	.long	0xc202807b			# fpscr c202807b
#
#						# fnmap  r17,r6,r10,r15
i16f6:	.long	0x80000000, 0x00000000		# f6 8000000000000000
i16f10:	.long	0x00000000, 0x00000000		# f10 0000000000000000
i16f15:	.long	0x80000000, 0x00000000		# f15 8000000000000000
i16cr:	.long	0x665307f0			# cr 665307f0
i16fp:	.long	0xffffffff, 0x00000093		# fpscr 00000093
o16f17:	.long	0x80000000, 0x00000000		# f17 8000000000000000
o16cr:	.long	0x605307f0			# cr 605307f0
o16fp:	.long	0x00012093			# fpscr 00012093
#
#						# fnmap  r15,r28,r18,r13
i17f13:	.long	0xaabd2057, 0x6af35baa		# f13 aabd20576af35baa
i17f18:	.long	0x2f9418ed, 0xabac51b9		# f18 2f9418edabac51b9
i17f28:	.long	0xfbda06da, 0xa24dcfd2		# f28 fbda06daa24dcfd2
i17cr:	.long	0x1482732b			# cr 1482732b
i17fp:	.long	0xffffffff, 0x00000080		# fpscr 00000080
o17f15:	.long	0xe6a7b084, 0x1757de3d		# f15 e6a7b0841757de3d
o17cr:	.long	0x1882732b			# cr 1882732b
o17fp:	.long	0x82028080			# fpscr 82028080
#
#						# fnmap  r13,r2,r0,r22
i18f0:	.long	0x9bf55356, 0xcb89a74e		# f0 9bf55356cb89a74e
i18f2:	.long	0x16879f87, 0xc3998fc9		# f2 16879f87c3998fc9
i18f22:	.long	0x0286e4f0, 0xa90683c6		# f22 0286e4f0a90683c6
i18cr:	.long	0x78afc497			# cr 78afc497
i18fp:	.long	0xffffffff, 0x000600d2		# fpscr 000600d2
o18f13:	.long	0x1bf55356, 0xcb89a74d		# f13 1bf55356cb89a74d
o18cr:	.long	0x78afc497			# cr 78afc497
o18fp:	.long	0x820240d2			# fpscr 820240d2
#
#						# fnms  r15,r17,r29,r24
i19f17:	.long	0xffd1ffff, 0xffffffff		# f17 ffd1ffffffffffff
i19f24:	.long	0xbff00000, 0x00007fff		# f24 bff0000000007fff
i19f29:	.long	0xffefffff, 0xff800000		# f29 ffefffffff800000
i19fp:	.long	0xffffffff, 0x00000003		# fpscr 00000003
o19f15:	.long	0xffefffff, 0xffffffff		# f15 ffefffffffffffff
o19fp:	.long	0x92028003			# fpscr 92028003
#
#						# fnmsp  r31,r7,r14,r19
i20f7:	.long	0x7ff00000, 0x00000000		# f7 7ff0000000000000
i20f14:	.long	0xfff00000, 0x00000000		# f14 fff0000000000000
i20f19:	.long	0xfff00000, 0x00000000		# f19 fff0000000000000
i20cr:	.long	0xb91a32b2			# cr b91a32b2
i20fp:	.long	0xffffffff, 0x0000003a		# fpscr 0000003a
o20f31:	.long	0x7ff80000, 0x00000000		# f31 7ff8000000000000
o20cr:	.long	0xba1a32b2			# cr ba1a32b2
o20fp:	.long	0xa081103a			# fpscr a081103a
#
#						# fnmsp  r11,r0,r13,r26
i21f0:	.long	0x1cc47227, 0xbac26093		# f0 1cc47227bac26093
i21f13:	.long	0x00000000, 0x00000001		# f13 0000000000000001
i21f26:	.long	0x1cc5b6f8, 0xd7efd404		# f26 1cc5b6f8d7efd404
i21cr:	.long	0xd0f3e25e			# cr d0f3e25e
i21fp:	.long	0xffffffff, 0x000000f0		# fpscr 000000f0
o21f11:	.long	0x5ccfffff, 0xfffffffd		# f11 5ccffffffffffffd
o21cr:	.long	0xdcf3e25e			# cr dcf3e25e
o21fp:	.long	0xca0640f0			# fpscr ca0640f0
#
#						# fnmsp  r13,r1,r3,r17
i22f1:	.long	0x9c5c5356, 0xac5c8a40		# f1 9c5c5356ac5c8a40
i22f3:	.long	0x80000000, 0x00000006		# f3 8000000000000006
i22f17:	.long	0x9c49ddad, 0x0d8ff7c7		# f17 9c49ddad0d8ff7c7
i22cr:	.long	0x572ab96b			# cr 572ab96b
i22fp:	.long	0xffffffff, 0x0000004a		# fpscr 0000004a
o22f13:	.long	0x80000000, 0x00000007		# f13 8000000000000007
o22cr:	.long	0x5c2ab96b			# cr 5c2ab96b
o22fp:	.long	0xca07804a			# fpscr ca07804a
#
#						# fabsp  r22,r0
i23f0:	.long	0xfff50259, 0x3df770d1		# f0 fff502593df770d1
i23f22:	.long	0xacb14e56, 0x84bdab3e		# f22 acb14e5684bdab3e
i23cr:	.long	0x24e872a4			# cr 24e872a4
i23fp:	.long	0xffffffff, 0x000000ba		# fpscr 000000ba
o23f22:	.long	0x7ff50259, 0x3df770d1		# f22 7ff502593df770d1
o23cr:	.long	0x20e872a4			# cr 20e872a4
o23fp:	.long	0x000000ba			# fpscr 000000ba
#
#						# fabsp  r21,r17
i24f17:	.long	0xe2e88787, 0xbf6430ec		# f17 e2e88787bf6430ec
i24f21:	.long	0xbdd32dd4, 0x364a8f6e		# f21 bdd32dd4364a8f6e
i24cr:	.long	0x4e33ca0c			# cr 4e33ca0c
i24fp:	.long	0xffffffff, 0x0005d0e0		# fpscr 0005d0e0
o24f21:	.long	0x62e88787, 0xbf6430ec		# f21 62e88787bf6430ec
o24cr:	.long	0x4033ca0c			# cr 4033ca0c
o24fp:	.long	0x0005d0e0			# fpscr 0005d0e0
#
#						# fneg  r22,r27
i25f22:	.long	0xcc68f6b1, 0x5d537425		# f22 cc68f6b15d537425
i25f27:	.long	0x7ffe7c6c, 0xe9094230		# f27 7ffe7c6ce9094230
i25fp:	.long	0xffffffff, 0x000000c8		# fpscr 000000c8
o25f22:	.long	0xfffe7c6c, 0xe9094230		# f22 fffe7c6ce9094230
o25fp:	.long	0x000000c8			# fpscr 000000c8
#
#						# fmr  r20,r24
i26f20:	.long	0x28aa543a, 0x8342ebbf		# f20 28aa543a8342ebbf
i26f24:	.long	0x617b9ea8, 0x6d4b6baa		# f24 617b9ea86d4b6baa
i26fp:	.long	0xffffffff, 0x0000002b		# fpscr 0000002b
o26f20:	.long	0x617b9ea8, 0x6d4b6baa		# f20 617b9ea86d4b6baa
o26fp:	.long	0x0000002b			# fpscr 0000002b
#
#						# fmrp  r2,r17
i27f2:	.long	0x32f217c6, 0xfefa98d6		# f2 32f217c6fefa98d6
i27f17:	.long	0xfff9600d, 0x2e1cd7ea		# f17 fff9600d2e1cd7ea
i27cr:	.long	0xe5eb70d6			# cr e5eb70d6
i27fp:	.long	0xffffffff, 0x0000001b		# fpscr 0000001b
o27f2:	.long	0xfff9600d, 0x2e1cd7ea		# f2 fff9600d2e1cd7ea
o27cr:	.long	0xe0eb70d6			# cr e0eb70d6
o27fp:	.long	0x0000001b			# fpscr 0000001b
#
#						# mffs  r4
i28fp:	.long	0xffffffff, 0x7242a063		# fpscr 7242a063
o28f4:	.long	0xffffffff, 0x7242a063		# f4 ffffffff7242a063
o28fp:	.long	0x7242a063			# fpscr 7242a063
#
#						# mffsp  r3
i29cr:	.long	0x7697dd4a			# cr 7697dd4a
i29fp:	.long	0xffffffff, 0x21614031		# fpscr 21614031
o29f3:	.long	0xffffffff, 0x21614031		# f3 ffffffff21614031
o29cr:	.long	0x7297dd4a			# cr 7297dd4a
o29fp:	.long	0x21614031			# fpscr 21614031
#
#						# mtfsfip  0,5
i30cr:	.long	0xce2c5d6c			# cr ce2c5d6c
i30fp:	.long	0xffffffff, 0x0000005a		# fpscr 0000005a
o30cr:	.long	0xc52c5d6c			# cr c52c5d6c
o30fp:	.long	0x5000005a			# fpscr 5000005a
#
#						# mtfsfip  3,12
i31cr:	.long	0x002b9ada			# cr 002b9ada
i31fp:	.long	0xffffffff, 0x6649b0ba		# fpscr 6649b0ba
o31cr:	.long	0x062b9ada			# cr 062b9ada
o31fp:	.long	0x664cb0ba			# fpscr 664cb0ba
#
#						# mtfsfip  0,15
i32cr:	.long	0xc40c36c3			# cr c40c36c3
i32fp:	.long	0xffffffff, 0x71fbf0b3		# fpscr 71fbf0b3
o32cr:	.long	0xcf0c36c3			# cr cf0c36c3
o32fp:	.long	0xf1fbf0b3			# fpscr f1fbf0b3
#
#						# mtfsfip  0,6
i33cr:	.long	0x30989fd2			# cr 30989fd2
i33fp:	.long	0xffffffff, 0xf246c052		# fpscr f246c052
o33cr:	.long	0x32989fd2			# cr 32989fd2
o33fp:	.long	0x2246c052			# fpscr 2246c052
#
#						# mtfsfip  3,6
i34cr:	.long	0xe031fcb3			# cr e031fcb3
i34fp:	.long	0xffffffff, 0x04046020		# fpscr 04046020
o34cr:	.long	0xe031fcb3			# cr e031fcb3
o34fp:	.long	0x04066020			# fpscr 04066020
#
#						# mtfsb1  0
i35fp:	.long	0xffffffff, 0x00000000		# fpscr 00000000
o35fp:	.long	0x80000000			# fpscr 80000000
#
#						# mtfsb1p  17
i36cr:	.long	0x2f7b6b16			# cr 2f7b6b16
i36fp:	.long	0xffffffff, 0x000000fa			# fpscr 000000fa
o36cr:	.long	0x207b6b16			# cr 207b6b16
o36fp:	.long	0x000040fa			# fpscr 000040fa
#
#						# mtfsb1p  3
i37cr:	.long	0xddb8d9f1			# cr ddb8d9f1
i37fp:	.long	0xffffffff, 0x00000000		# fpscr 00000000
o37cr:	.long	0xd9b8d9f1			# cr d9b8d9f1
o37fp:	.long	0x90000000			# fpscr 90000000
#
#						# mtfsb1p  3
i38cr:	.long	0x0d3a586e			# cr 0d3a586e
i38fp:	.long	0xffffffff, 0x65a370c3		# fpscr 65a370c3
o38cr:	.long	0x0f3a586e			# cr 0f3a586e
o38fp:	.long	0xf5a370c3			# fpscr f5a370c3
#
#						# mtfsb0  0
i39fp:	.long	0xffffffff, 0xff54b018		# fpscr ff54b018
o39fp:	.long	0x7f54b018			# fpscr 7f54b018
#
#						# mtfsb0p  27
i40cr:	.long	0x8d6b8438			# cr 8d6b8438
i40fp:	.long	0xffffffff, 0x000000b1		# fpscr 000000b1
o40cr:	.long	0x806b8438			# cr 806b8438
o40fp:	.long	0x000000a1			# fpscr 000000a1
#
#						# mtfsb0p  4
i41cr:	.long	0x39c2d4be			# cr 39c2d4be
i41fp:	.long	0xffffffff, 0x00000030		# fpscr 00000030
o41cr:	.long	0x30c2d4be			# cr 30c2d4be
o41fp:	.long	0x00000030			# fpscr 00000030
#
#						# mtfsb0p  23
i42cr:	.long	0xc454e7d2			# cr c454e7d2
i42fp:	.long	0xffffffff, 0xa1ee3070		# fpscr a1ee3070
o42cr:	.long	0xca54e7d2			# cr ca54e7d2
o42fp:	.long	0xa1ee3070			# fpscr a1ee3070
#
#						# fms  r11,r9,r20,r26
i43f9:	.long	0x1ca589ae, 0xf4a3eb9a		# f9 1ca589aef4a3eb9a
i43f20:	.long	0x80000000, 0x00000001		# f20 8000000000000001
i43f26:	.long	0x9cb17a1b, 0x03d6380d		# f26 9cb17a1b03d6380d
i43fp:	.long	0xffffffff, 0x00000020		# fpscr 00000020
o43f11:	.long	0x5cd00000, 0x00000000		# f11 5cd0000000000000
o43fp:	.long	0xca064020			# fpscr ca064020
#
#						# fmsp  r24,r28,r16,r13
i44f13:	.long	0xfff00000, 0x00000000		# f13 fff0000000000000
i44f16:	.long	0x80000000, 0x00000000		# f16 8000000000000000
i44f28:	.long	0x80000000, 0x00000000		# f28 8000000000000000
i44cr:	.long	0x97b731d2			# cr 97b731d2
i44fp:	.long	0xffffffff, 0x67ae1068		# fpscr 67ae1068
o44f24:	.long	0x7ff00000, 0x00000000		# f24 7ff0000000000000
o44cr:	.long	0x96b731d2			# cr 96b731d2
o44fp:	.long	0x67a85068			# fpscr 67a85068
#
	.toc
	TOCE(.all, entry)
	TOCL(_all, data)
