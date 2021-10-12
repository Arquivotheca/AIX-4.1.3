# @(#)39	1.3  src/bos/diag/da/fpa/sys.s, dafpa, bos411, 9428A410j 3/23/94 06:30:30

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
#		will be compared with expected results. If any test sysils,
#		a particular return code will be returned to the caller.
#
	.csect	.sys[PR]
	.globl	.sys[PR]
	.align	2
.sys:
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
	LTOC(4, _sys, data)		# Loads g4 with the address of _sys
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
	.using	_sys[RW],g4		# g4 is the base register to the data
#	 				     section _sys[rw]. Eastablishing 
#					     @ability
#
#	
	xor	g3,g3,g3		# g3 = 0. Initialize return code
#
#				# Loading floating pt registers for computation
#
#
# 1 #
#
#	lfd	rf1,i1f1
#	lfd	rf2,i1f2
#	lfd	rf4,i1f4
#	lfd	rf5,i1f5
#	lfd	rf6,i1f6
#	lfd	rf7,i1f7
#	lfd	rf8,i1f8
#	lfd	rf13,i1f13
#	lfd	rf15,i1f15
#	lfd	rf18,i1f18
#	lfd	rf19,i1f19
#	lfd	rf21,i1f21
#	lfd	rf22,i1f22
#	lfd	rf23,i1f23
#	lfd	rf25,i1f25
#	lfd	rf26,i1f26
#	lfd	rf28,i1f28
#	lfd	rf29,i1f29
#	lfd	rf30,i1f30
#	l	g9,i1cr
#	mtcrf	0xff,g9
#	lfd	rf3,i1fp
#	mtfsf	0xff,rf3
##
#### instructions for 1 ###
##
#	fd.	r11,r4,r7
#	fa.	r24,r25,r11
#	fcmpo	1,r22,r1
#	fa.	r3,r24,r30
#	fa.	r16,r3,r8
#	fcmpu	2,r3,r24
#	mffs	r16
#	mtfsf.	0x87,r6				# b'1100001110',r6
#	frsp.	r31,r16
#	mcrfs	3,7
#	fma	r0,r19,r21,r30
#	frsp	r0,r28
#	mtfsb0	4
#	mtfsb1.	5
#	fcmpu	1,r1,r2
#	mtfsf	0x3c,r6				# b'1001111001',r6
#	fm	r12,r18,r15
#	mcrfs	3,1
#	fnma.	r22,r23,r13,r24
#	fnms.	r25,r15,r22,r22
#	fabs.	r29,r25
#	fneg.	r24,r29
#	fnma	r20,r7,r24,r29
#	mtfsb1.	25
#	fcmpu	6,r5,r24
#	fcmpo	7,r7,r26
#	fnabs.	r28,r31
#	fa	r23,r28,r22
#	fd.	r7,r22,r23
#	fcmpo	5,r6,r7
##
#	ai	g3,g3,1				# RC = 
#	mfcr	g6				# gets cr = 8f264144
#	l	g7,o1cr				# o1cr = 8f264c44
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 1
#	stfd	rf1,hold
#	mffs	rf1
#	stfd	rf1,fpsave
#	l	g6,fpsave+4
#	lfd	rf1,hold
#	l	g7,o1fp
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 2
#	stfd	rf0,fpsave
#	l	g6,fpsave
#	l	g7,o1f0
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 3
#	l	g6,fpsave+4
#	l	g7,o1f0+4
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 4
#	stfd	rf3,fpsave
#	l	g6,fpsave
#	l	g7,o1f3
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 5
#	l	g6,fpsave+4
#	l	g7,o1f3+4
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 6
#	stfd	rf7,fpsave
#	l	g6,fpsave
#	l	g7,o1f7
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 7
#	l	g6,fpsave+4
#	l	g7,o1f7+4
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 8
#	stfd	rf11,fpsave
#	l	g6,fpsave
#	l	g7,o1f11
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 9
#	l	g6,fpsave+4
#	l	g7,o1f11+4
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 10
#	stfd	rf12,fpsave
#	l	g6,fpsave
#	l	g7,o1f12
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 11
#	l	g6,fpsave+4
#	l	g7,o1f12+4
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 12
#	stfd	rf16,fpsave
#	l	g6,fpsave
#	l	g7,o1f16
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 13
#	l	g6,fpsave+4			# gets = a20a2000
#	l	g7,o1f16+4
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 14
#	stfd	rf20,fpsave
#	l	g6,fpsave
#	l	g7,o1f20
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 15
#	l	g6,fpsave+4
#	l	g7,o1f20+4
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 16
#	stfd	rf22,fpsave
#	l	g6,fpsave
#	l	g7,o1f22
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 17
#	l	g6,fpsave+4
#	l	g7,o1f22+4
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 18
#	stfd	rf23,fpsave
#	l	g6,fpsave
#	l	g7,o1f23
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 19
#	l	g6,fpsave+4
#	l	g7,o1f23+4
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 20
#	stfd	rf24,fpsave
#	l	g6,fpsave
#	l	g7,o1f24
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 21
#	l	g6,fpsave+4
#	l	g7,o1f24+4
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 22
#	stfd	rf25,fpsave
#	l	g6,fpsave
#	l	g7,o1f25
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 23
#	l	g6,fpsave+4
#	l	g7,o1f25+4
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 24
#	stfd	rf28,fpsave
#	l	g6,fpsave
#	l	g7,o1f28
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 25
#	l	g6,fpsave+4
#	l	g7,o1f28+4
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 26
#	stfd	rf29,fpsave
#	l	g6,fpsave
#	l	g7,o1f29
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 27
#	l	g6,fpsave+4
#	l	g7,o1f29+4
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 28
#	stfd	rf31,fpsave
#	l	g6,fpsave
#	l	g7,o1f31
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 29
#	l	g6,fpsave+4
#	l	g7,o1f31+4
#	cmp	0,g6,g7
#	bne	0,FINI
#
#
# 2 #
#
	lfd	rf0,i2f0
	lfd	rf1,i2f1
	lfd	rf2,i2f2
	lfd	rf6,i2f6
	lfd	rf9,i2f9
	lfd	rf10,i2f10
	lfd	rf12,i2f12
	lfd	rf13,i2f13
	lfd	rf14,i2f14
	lfd	rf17,i2f17
	lfd	rf19,i2f19
	lfd	rf20,i2f20
	lfd	rf22,i2f22
	lfd	rf24,i2f24
	lfd	rf25,i2f25
	lfd	rf27,i2f27
	lfd	rf28,i2f28
	lfd	rf29,i2f29
	l	g9,i2cr
	mtcrf	0xff,g9
	lfd	rf3,i2fp
	mtfsf	0xff,rf3
#
### instructions for 2 ###
#
	fmr.	r29,r20
	fcmpo	3,r25,r29
	fcmpu	0,r28,r29
	fnma	r27,r22,r27,r27
	fnma.	r11,r20,r27,r24
	fcmpu	3,r11,r1
	fnabs.	r11,r20
	fms.	r23,r13,r11,r9
	mtfsb1.	1
	fneg	r19,r11
	fcmpu	1,r19,r0
	frsp.	r4,r19
	fnma.	r31,r12,r2,r4
	fcmpo	7,r4,r6
	mcrfs	0,1
	frsp	r12,r31
	fa.	r13,r12,r28
	fm	r12,r12,r17
	mtfsf.	0xd8,r12			# b'0110110001',r12
	fnma	r6,r28,r12,r12
	fm.	r25,r6,r0
	fcmpo	4,r23,r25
	fma	r25,r6,r14,r20
	fd	r21,r25,r4
	fd	r29,r27,r21
	fnma	r1,r10,r21,r29
	fnma.	r1,r13,r0,r31
	mffs	r7
	mtfsb1.	17
	fabs.	r19,r1
#
	ai	g3,g3,1				# RC = 30
	mfcr	g6
	l	g7,o2cr
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 31
	stfd	rf1,hold
	mffs	rf1
	stfd	rf1,fpsave
	l	g6,fpsave+4
	lfd	rf1,hold
	l	g7,o2fp
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 32
	stfd	rf1,fpsave
	l	g6,fpsave
	l	g7,o2f1
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 33
	l	g6,fpsave+4
	l	g7,o2f1+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 34
	stfd	rf4,fpsave
	l	g6,fpsave
	l	g7,o2f4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 35
	l	g6,fpsave+4
	l	g7,o2f4+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 36
	stfd	rf6,fpsave
	l	g6,fpsave
	l	g7,o2f6
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 37
	l	g6,fpsave+4
	l	g7,o2f6+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 38
	stfd	rf7,fpsave
	l	g6,fpsave
	l	g7,o2f7
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 39
	l	g6,fpsave+4
	l	g7,o2f7+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 40
	stfd	rf11,fpsave
	l	g6,fpsave
	l	g7,o2f11
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 41
	l	g6,fpsave+4
	l	g7,o2f11+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 42
	stfd	rf12,fpsave
	l	g6,fpsave
	l	g7,o2f12
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 43
	l	g6,fpsave+4
	l	g7,o2f12+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 44
	stfd	rf13,fpsave
	l	g6,fpsave
	l	g7,o2f13
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 45
	l	g6,fpsave+4
	l	g7,o2f13+4
	cmp	0,g6,g7
	bne	0,FINI
#
#	ai	g3,g3,1				# RC = 46
#	stfd	rf19,fpsave
#	l	g6,fpsave			# gets = 7ff8d6f7
#	l	g7,o2f19			# o2f19 = 7ff0d6f7
#	cmp	0,g6,g7
#	bne	0,FINI
#
#	ai	g3,g3,1				# RC = 47
#	l	g6,fpsave+4			# gets = 80000000
#	l	g7,o2f19+4			# o2f19 = 98c6a021
#	cmp	0,g6,g7
#	bne	0,FINI
#
	ai	g3,g3,1				# RC = 48
	stfd	rf21,fpsave
	l	g6,fpsave
	l	g7,o2f21
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 49
	l	g6,fpsave+4
	l	g7,o2f21+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 50
	stfd	rf23,fpsave
	l	g6,fpsave
	l	g7,o2f23
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 51
	l	g6,fpsave+4
	l	g7,o2f23+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 52
	stfd	rf25,fpsave
	l	g6,fpsave
	l	g7,o2f25
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 53
	l	g6,fpsave+4
	l	g7,o2f25+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 54
	stfd	rf27,fpsave
	l	g6,fpsave
	l	g7,o2f27
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 55
	l	g6,fpsave+4
	l	g7,o2f27+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 56
	stfd	rf29,fpsave
	l	g6,fpsave
	l	g7,o2f29
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 57
	l	g6,fpsave+4
	l	g7,o2f29+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 58
	stfd	rf31,fpsave
	l	g6,fpsave
	l	g7,o2f31
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 59
	l	g6,fpsave+4
	l	g7,o2f31+4
	cmp	0,g6,g7
	bne	0,FINI
#
#
# 3 #
#
	lfd	rf3,i3f3
	lfd	rf4,i3f4
	lfd	rf5,i3f5
	lfd	rf6,i3f6
	lfd	rf10,i3f10
	lfd	rf12,i3f12
	lfd	rf17,i3f17
	lfd	rf18,i3f18
	lfd	rf19,i3f19
	lfd	rf20,i3f20
	lfd	rf23,i3f23
	lfd	rf27,i3f27
	lfd	rf29,i3f29
	lfd	rf30,i3f30
	l	g9,i3cr
	mtcrf	0xff,g9
	lfd	rf1,i3fp
	mtfsf	0xff,rf1
#
### instructions for 3 ###
#
	fd.	r21,r12,r23
	fnma.	r2,r21,r6,r21
	fm	r22,r4,r2
	fnms.	r16,r12,r22,r17
	fs.	r11,r5,r22
	fnma	r13,r18,r11,r18
	fs	r17,r11,r30
	fnms	r25,r13,r17,r18
	fnms.	r3,r17,r25,r3
	fnma.	r3,r30,r25,r29
	fs	r3,r27,r3
	fma.	r3,r20,r30,r22
	fs.	r15,r3,r10
	fd.	r21,r25,r15
	fm.	r14,r21,r12
	fd	r30,r19,r21
	fd	r21,r14,r14
	fnma	r22,r30,r21,r10
	fs.	r8,r6,r21
	fnms.	r6,r8,r22,r6
#
	ai	g3,g3,1				# RC = 60
	mfcr	g6
	l	g7,o3cr
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 61
	stfd	rf1,hold
	mffs	rf1
	stfd	rf1,fpsave
	l	g6,fpsave+4
	lfd	rf1,hold
	l	g7,o3fp
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 62
	stfd	rf2,fpsave
	l	g6,fpsave
	l	g7,o3f2
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 63
	l	g6,fpsave+4
	l	g7,o3f2+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 64
	stfd	rf3,fpsave
	l	g6,fpsave
	l	g7,o3f3
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 65
	l	g6,fpsave+4
	l	g7,o3f3+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 66
	stfd	rf8,fpsave
	l	g6,fpsave
	l	g7,o3f8
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 67
	l	g6,fpsave+4
	l	g7,o3f8+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 68
	stfd	rf11,fpsave
	l	g6,fpsave
	l	g7,o3f11
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 69
	l	g6,fpsave+4
	l	g7,o3f11+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 70
	stfd	rf13,fpsave
	l	g6,fpsave
	l	g7,o3f13
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 71
	l	g6,fpsave+4
	l	g7,o3f13+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 72
	stfd	rf14,fpsave
	l	g6,fpsave
	l	g7,o3f14
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 73
	l	g6,fpsave+4
	l	g7,o3f14+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 74
	stfd	rf15,fpsave
	l	g6,fpsave
	l	g7,o3f15
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 75
	l	g6,fpsave+4
	l	g7,o3f15+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 76
	stfd	rf16,fpsave
	l	g6,fpsave
	l	g7,o3f16
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 77
	l	g6,fpsave+4
	l	g7,o3f16+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 78
	stfd	rf17,fpsave
	l	g6,fpsave
	l	g7,o3f17
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 79
	l	g6,fpsave+4
	l	g7,o3f17+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 80
	stfd	rf21,fpsave
	l	g6,fpsave
	l	g7,o3f21
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 81
	l	g6,fpsave+4
	l	g7,o3f21+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 82
	stfd	rf22,fpsave
	l	g6,fpsave
	l	g7,o3f22
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 83
	l	g6,fpsave+4
	l	g7,o3f22+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 84
	stfd	rf25,fpsave
	l	g6,fpsave
	l	g7,o3f25
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 85
	l	g6,fpsave+4
	l	g7,o3f25+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 86
	stfd	rf30,fpsave
	l	g6,fpsave
	l	g7,o3f30
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 87
	l	g6,fpsave+4
	l	g7,o3f30+4
	cmp	0,g6,g7
	bne	0,FINI
#
#
# 4 #
#
	lfd	rf0,i4f0
	lfd	rf1,i4f1
	lfd	rf3,i4f3
	lfd	rf5,i4f5
	lfd	rf8,i4f8
	lfd	rf9,i4f9
	lfd	rf10,i4f10
	lfd	rf13,i4f13
	lfd	rf14,i4f14
	lfd	rf15,i4f15
	lfd	rf16,i4f16
	lfd	rf17,i4f17
	lfd	rf21,i4f21
	lfd	rf31,i4f31
	l	g9,i4cr
	mtcrf	0xff,g9
	lfd	rf2,i4fp
	mtfsf	0xff,rf2
#
### instructions for 4 ###
#
	fnms.	r2,r16,r14,r1
	fa.	r19,r31,r2
	fs	r31,r2,r2
	fm.	r18,r19,r15
	fa	r6,r17,r18
	fnms	r17,r14,r18,r6
	fms	r22,r8,r13,r17
	fnma	r16,r22,r22,r22
	fnma	r25,r9,r16,r1
	fnma.	r24,r25,r16,r25
	fma	r20,r25,r15,r13
	fnms.	r16,r20,r8,r10
	fm	r25,r10,r20
	fm.	r25,r10,r17
	fa	r26,r25,r25
	fms.	r18,r25,r21,r5
	fm	r19,r26,r3
	fm.	r18,r21,r19
	fm	r19,r14,r10
	fs.	r29,r18,r0
#
	ai	g3,g3,1				# RC = 88
	mfcr	g6
	l	g7,o4cr
	cmp	0,g6,g7
	bne	0,FINI
#
#	ai	g3,g3,1				# RC = 89
#	stfd	rf1,hold
#	mffs	rf1
#	stfd	rf1,fpsave
#	l	g6,fpsave+4			# gets = 8a014000
#	lfd	rf1,hold
#	l	g7,o4fp				# o4fp = 8a002000
#	cmp	0,g6,g7
#	bne	0,FINI
#
	ai	g3,g3,1				# RC = 90
	stfd	rf2,fpsave
	l	g6,fpsave
	l	g7,o4f2
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 91
	l	g6,fpsave+4
	l	g7,o4f2+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 92
	stfd	rf6,fpsave
	l	g6,fpsave
	l	g7,o4f6
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 93
	l	g6,fpsave+4
	l	g7,o4f6+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 94
	stfd	rf16,fpsave
	l	g6,fpsave
	l	g7,o4f16
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 95
	l	g6,fpsave+4
	l	g7,o4f16+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 96
	stfd	rf17,fpsave
	l	g6,fpsave
	l	g7,o4f17
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 97
	l	g6,fpsave+4
	l	g7,o4f17+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 98
	stfd	rf18,fpsave
	l	g6,fpsave
	l	g7,o4f18
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 99
	l	g6,fpsave+4
	l	g7,o4f18+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 100
	stfd	rf19,fpsave
	l	g6,fpsave
	l	g7,o4f19
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 101
	l	g6,fpsave+4
	l	g7,o4f19+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 102
	stfd	rf20,fpsave
	l	g6,fpsave
	l	g7,o4f20
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 103
	l	g6,fpsave+4
	l	g7,o4f20+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 104
	stfd	rf22,fpsave
	l	g6,fpsave
	l	g7,o4f22
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 105
	l	g6,fpsave+4
	l	g7,o4f22+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 106
	stfd	rf24,fpsave
	l	g6,fpsave
	l	g7,o4f24
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 107
	l	g6,fpsave+4
	l	g7,o4f24+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 108
	stfd	rf25,fpsave
	l	g6,fpsave
	l	g7,o4f25
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 109
	l	g6,fpsave+4
	l	g7,o4f25+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 110
	stfd	rf26,fpsave
	l	g6,fpsave
	l	g7,o4f26
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 111
	l	g6,fpsave+4
	l	g7,o4f26+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 112
	stfd	rf31,fpsave
	l	g6,fpsave
	l	g7,o4f31
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 113
	l	g6,fpsave+4
	l	g7,o4f31+4
	cmp	0,g6,g7
	bne	0,FINI
#
#
# 5 #
#
#	lfd	rf0,i5f0
#	lfd	rf2,i5f2
#	lfd	rf4,i5f4
#	lfd	rf7,i5f7
#	lfd	rf8,i5f8
#	lfd	rf9,i5f9
#	lfd	rf12,i5f12
#	lfd	rf14,i5f14
#	lfd	rf15,i5f15
#	lfd	rf20,i5f20
#	lfd	rf23,i5f23
#	lfd	rf24,i5f24
#	lfd	rf25,i5f25
#	lfd	rf27,i5f27
#	lfd	rf30,i5f30
#	l	g9,i5cr
#	mtcrf	0xff,g9
#	lfd	rf1,i5fp
#	mtfsf	0xff,rf1
##
#### instructions for 5 ###
##
#	fnms	r3,r14,r8,r8
#	fma.	r6,r12,r24,r3
#	fnms.	r1,r25,r6,r3
#	fnma	r11,r1,r0,r0
#	fma	r24,r1,r11,r23
#	frsp.	r31,r24
#	fnms.	r17,r9,r24,r1
#	fd	r31,r9,r17
#	fnma	r17,r25,r31,r30
#	fnma	r9,r2,r31,r20
#	fnms	r31,r27,r12,r17
#	fnma	r24,r0,r31,r4
#	fms	r24,r30,r3,r25
#	fnma	r4,r15,r6,r24
#	fms	r20,r15,r4,r9
#	fnma.	r21,r4,r20,r31
#	frsp	r16,r21
#	fs	r1,r7,r21
#	fs	r27,r12,r1
#	fd.	r28,r20,r27
##
##	ai	g3,g3,1				# RC = 114
##	mfcr	g6				# gets = bd756cea
##	l	g7,o5cr				# o5cr = b6756cea
##	cmp	0,g6,g7
##	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 115
#	stfd	rf1,hold
#	mffs	rf1
#	stfd	rf1,fpsave
#	l	g6,fpsave+4
#	lfd	rf1,hold
#	l	g7,o5fp
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 116
#	stfd	rf1,fpsave
#	l	g6,fpsave
#	l	g7,o5f1
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 117
#	l	g6,fpsave+4
#	l	g7,o5f1+4
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 118
#	stfd	rf3,fpsave
#	l	g6,fpsave
#	l	g7,o5f3
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 119
#	l	g6,fpsave+4
#	l	g7,o5f3+4
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 120
#	stfd	rf4,fpsave
#	l	g6,fpsave
#	l	g7,o5f4
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 121
#	l	g6,fpsave+4
#	l	g7,o5f4+4
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 122
#	stfd	rf6,fpsave
#	l	g6,fpsave
#	l	g7,o5f6
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 123
#	l	g6,fpsave+4
#	l	g7,o5f6+4
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 124
#	stfd	rf9,fpsave
#	l	g6,fpsave
#	l	g7,o5f9
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 125
#	l	g6,fpsave+4
#	l	g7,o5f9+4
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 126
#	stfd	rf11,fpsave
#	l	g6,fpsave
#	l	g7,o5f11
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 127
#	l	g6,fpsave+4
#	l	g7,o5f11+4
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 128
#	stfd	rf16,fpsave
#	l	g6,fpsave
#	l	g7,o5f16
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 129
#	l	g6,fpsave+4
#	l	g7,o5f16+4
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 130
#	stfd	rf17,fpsave
#	l	g6,fpsave
#	l	g7,o5f17
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 131
#	l	g6,fpsave+4
#	l	g7,o5f17+4
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 132
#	stfd	rf20,fpsave
#	l	g6,fpsave
#	l	g7,o5f20
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 133
#	l	g6,fpsave+4
#	l	g7,o5f20+4
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 134
#	stfd	rf21,fpsave
#	l	g6,fpsave
#	l	g7,o5f21
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 135
#	l	g6,fpsave+4
#	l	g7,o5f21+4
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 136
#	stfd	rf24,fpsave
#	l	g6,fpsave
#	l	g7,o5f24
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 137
#	l	g6,fpsave+4
#	l	g7,o5f24+4
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 138
#	stfd	rf27,fpsave
#	l	g6,fpsave
#	l	g7,o5f27
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 139
#	l	g6,fpsave+4
#	l	g7,o5f27+4
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 140
#	stfd	rf31,fpsave
#	l	g6,fpsave
#	l	g7,o5f31
#	cmp	0,g6,g7
#	bne	0,FINI
##
#	ai	g3,g3,1				# RC = 141
#	l	g6,fpsave+4
#	l	g7,o5f31+4
#	cmp	0,g6,g7
#	bne	0,FINI
##
#
# 6 #
#
	lfd	rf1,i6f1
	lfd	rf2,i6f2
	lfd	rf8,i6f8
	lfd	rf9,i6f9
	lfd	rf11,i6f11
	lfd	rf15,i6f15
	lfd	rf16,i6f16
	lfd	rf17,i6f17
	lfd	rf18,i6f18
	lfd	rf23,i6f23
	lfd	rf25,i6f25
	lfd	rf26,i6f26
	l	g9,i6cr
	mtcrf	0xff,g9
	lfd	rf3,i6fp
	mtfsf	0xff,rf3
#
### instructions for 6 ###
#
	fs	r5,r15,r8
	fms	r19,r2,r16,r5
	fnma	r6,r19,r5,r5
	fa.	r20,r1,r19
	fma.	r14,r20,r20,r20
	fs	r4,r18,r20
	frsp.	r4,r14
	fd	r18,r4,r4
	fa	r7,r23,r18
	fa.	r13,r11,r18
	fma.	r31,r1,r13,r13
	fma	r29,r7,r8,r31
	fs	r18,r26,r29
	fs.	r12,r18,r18
	fma	r4,r9,r18,r2
	fm.	r12,r25,r18
	fm	r12,r1,r7
	fd	r27,r14,r12
	frsp.	r9,r27
	fma.	r27,r9,r17,r9
#
	ai	g3,g3,1				# RC = 142
	mfcr	g6				# gets = ed4d1ea0
	l	g7,o6cr				# o6cr = ef4d1ea0
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 143
	stfd	rf1,hold
	mffs	rf1
	stfd	rf1,fpsave
	l	g6,fpsave+4
	lfd	rf1,hold
	l	g7,o6fp
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 144
	stfd	rf4,fpsave
	l	g6,fpsave
	l	g7,o6f4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 145
	l	g6,fpsave+4
	l	g7,o6f4+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 146
	stfd	rf5,fpsave
	l	g6,fpsave
	l	g7,o6f5
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 147
	l	g6,fpsave+4
	l	g7,o6f5+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 148
	stfd	rf6,fpsave
	l	g6,fpsave
	l	g7,o6f6
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 149
	l	g6,fpsave+4
	l	g7,o6f6+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 150
	stfd	rf7,fpsave
	l	g6,fpsave
	l	g7,o6f7
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 151
	l	g6,fpsave+4
	l	g7,o6f7+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 152
	stfd	rf9,fpsave
	l	g6,fpsave
	l	g7,o6f9
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 153
	l	g6,fpsave+4
	l	g7,o6f9+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 154
	stfd	rf12,fpsave
	l	g6,fpsave
	l	g7,o6f12
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 155
	l	g6,fpsave+4
	l	g7,o6f12+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 156
	stfd	rf13,fpsave
	l	g6,fpsave
	l	g7,o6f13
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 157
	l	g6,fpsave+4
	l	g7,o6f13+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 158
	stfd	rf14,fpsave
	l	g6,fpsave
	l	g7,o6f14
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 159
	l	g6,fpsave+4
	l	g7,o6f14+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 160
	stfd	rf18,fpsave
	l	g6,fpsave
	l	g7,o6f18
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 161
	l	g6,fpsave+4
	l	g7,o6f18+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 162
	stfd	rf19,fpsave
	l	g6,fpsave
	l	g7,o6f19
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 163
	l	g6,fpsave+4
	l	g7,o6f19+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 164
	stfd	rf20,fpsave
	l	g6,fpsave
	l	g7,o6f20
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 165
	l	g6,fpsave+4
	l	g7,o6f20+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 166
	stfd	rf27,fpsave
	l	g6,fpsave
	l	g7,o6f27
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 167
	l	g6,fpsave+4
	l	g7,o6f27+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 168
	stfd	rf29,fpsave
	l	g6,fpsave
	l	g7,o6f29
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 169
	l	g6,fpsave+4
	l	g7,o6f29+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 170
	stfd	rf31,fpsave
	l	g6,fpsave
	l	g7,o6f31
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 171
	l	g6,fpsave+4
	l	g7,o6f31+4
	cmp	0,g6,g7
	bne	0,FINI
#
#
# 7 #
#
	lfd	rf1,i7f1
	lfd	rf2,i7f2
	lfd	rf3,i7f3
	lfd	rf4,i7f4
	lfd	rf5,i7f5
	lfd	rf6,i7f6
	lfd	rf7,i7f7
	lfd	rf8,i7f8
	lfd	rf9,i7f9
	lfd	rf10,i7f10
	lfd	rf11,i7f11
	lfd	rf12,i7f12
	lfd	rf13,i7f13
	lfd	rf14,i7f14
	lfd	rf15,i7f15
	lfd	rf16,i7f16
	lfd	rf17,i7f17
	lfd	rf18,i7f18
	lfd	rf19,i7f19
	lfd	rf20,i7f20
	lfd	rf21,i7f21
	lfd	rf22,i7f22
	lfd	rf23,i7f23
	lfd	rf24,i7f24
	lfd	rf25,i7f25
	lfd	rf26,i7f26
	lfd	rf27,i7f27
	lfd	rf28,i7f28
	lfd	rf29,i7f29
	lfd	rf30,i7f30
	lfd	rf31,i7f31
	l	g9,i7cr
	mtcrf	0xff,g9
	lfd	rf0,i7fp
	mtfsf	0xff,rf0
#
### instructions for 7 ###
#
	fma.	r0,r1,r3,r2
	fma.	r1,r4,r6,r5
	fma	r2,r7,r9,r8
	fma.	r3,r10,r12,r11
	fma.	r4,r13,r15,r14
	fma.	r5,r16,r18,r17
	fma.	r6,r19,r21,r20
	fma.	r7,r22,r24,r23
	fma	r8,r25,r27,r26
	fma.	r9,r28,r30,r29
	fma.	r10,r31,r1,r0
	fma	r11,r2,r4,r3
	fma	r12,r5,r7,r6
	fma.	r13,r8,r10,r9
	fma.	r14,r11,r13,r12
	fma.	r15,r14,r16,r15
	fma	r16,r17,r19,r18
	fma	r17,r20,r22,r21
	fma	r18,r23,r25,r24
	fma.	r19,r26,r28,r27
	fma	r20,r29,r31,r30
	fma	r21,r0,r2,r1
	fma.	r22,r3,r5,r4
	fma.	r23,r6,r8,r7
	fma	r24,r9,r11,r10
	fma.	r25,r12,r14,r13
	fma.	r26,r15,r17,r16
	fma.	r27,r18,r20,r19
	fma	r28,r21,r23,r22
	fma.	r29,r24,r26,r25
	fma	r30,r27,r29,r28
	fma.	r31,r30,r0,r31
#
	ai	g3,g3,1				# RC = 172
	mfcr	g6
	l	g7,o7cr
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 173
	stfd	rf1,hold
	mffs	rf1
	stfd	rf1,fpsave
	l	g6,fpsave+4			# gets = ffb11042
	lfd	rf1,hold
	l	g7,o7fp				# o7fp = ffb28042
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 174
	stfd	rf0,fpsave
	l	g6,fpsave
	l	g7,o7f0
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 175
	l	g6,fpsave+4
	l	g7,o7f0+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 176
	stfd	rf1,fpsave
	l	g6,fpsave
	l	g7,o7f1
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 177
	l	g6,fpsave+4
	l	g7,o7f1+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 178
	stfd	rf2,fpsave
	l	g6,fpsave
	l	g7,o7f2
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 179
	l	g6,fpsave+4
	l	g7,o7f2+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 180
	stfd	rf3,fpsave
	l	g6,fpsave
	l	g7,o7f3
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 181
	l	g6,fpsave+4
	l	g7,o7f3+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 182
	stfd	rf4,fpsave
	l	g6,fpsave
	l	g7,o7f4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 183
	l	g6,fpsave+4
	l	g7,o7f4+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 184
	stfd	rf5,fpsave
	l	g6,fpsave
	l	g7,o7f5
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 185
	l	g6,fpsave+4
	l	g7,o7f5+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 186
	stfd	rf6,fpsave
	l	g6,fpsave
	l	g7,o7f6
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 187
	l	g6,fpsave+4
	l	g7,o7f6+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 188
	stfd	rf7,fpsave
	l	g6,fpsave
	l	g7,o7f7
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 189
	l	g6,fpsave+4
	l	g7,o7f7+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 190
	stfd	rf8,fpsave
	l	g6,fpsave
	l	g7,o7f8
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 191
	l	g6,fpsave+4
	l	g7,o7f8+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 192
	stfd	rf9,fpsave
	l	g6,fpsave
	l	g7,o7f9
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 193
	l	g6,fpsave+4
	l	g7,o7f9+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 194
	stfd	rf10,fpsave
	l	g6,fpsave
	l	g7,o7f10
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 195
	l	g6,fpsave+4
	l	g7,o7f10+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 196
	stfd	rf11,fpsave
	l	g6,fpsave
	l	g7,o7f11
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 197
	l	g6,fpsave+4
	l	g7,o7f11+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 198
	stfd	rf12,fpsave
	l	g6,fpsave
	l	g7,o7f12
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 199
	l	g6,fpsave+4
	l	g7,o7f12+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 200
	stfd	rf13,fpsave
	l	g6,fpsave
	l	g7,o7f13
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 201
	l	g6,fpsave+4
	l	g7,o7f13+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 202
	stfd	rf14,fpsave
	l	g6,fpsave
	l	g7,o7f14
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 203
	l	g6,fpsave+4
	l	g7,o7f14+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 204
	stfd	rf15,fpsave
	l	g6,fpsave
	l	g7,o7f15
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 205
	l	g6,fpsave+4
	l	g7,o7f15+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 206
	stfd	rf16,fpsave
	l	g6,fpsave
	l	g7,o7f16
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 207
	l	g6,fpsave+4
	l	g7,o7f16+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 208
	stfd	rf17,fpsave
	l	g6,fpsave
	l	g7,o7f17
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 209
	l	g6,fpsave+4
	l	g7,o7f17+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 210
	stfd	rf18,fpsave
	l	g6,fpsave
	l	g7,o7f18
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 211
	l	g6,fpsave+4
	l	g7,o7f18+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 212
	stfd	rf19,fpsave
	l	g6,fpsave
	l	g7,o7f19
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 213
	l	g6,fpsave+4
	l	g7,o7f19+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 214
	stfd	rf20,fpsave
	l	g6,fpsave
	l	g7,o7f20
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 215
	l	g6,fpsave+4
	l	g7,o7f20+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 216
	stfd	rf21,fpsave
	l	g6,fpsave
	l	g7,o7f21
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 217
	l	g6,fpsave+4
	l	g7,o7f21+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 218
	stfd	rf22,fpsave
	l	g6,fpsave
	l	g7,o7f22
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 219
	l	g6,fpsave+4
	l	g7,o7f22+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 220
	stfd	rf23,fpsave
	l	g6,fpsave
	l	g7,o7f23
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 221
	l	g6,fpsave+4
	l	g7,o7f23+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 222
	stfd	rf24,fpsave
	l	g6,fpsave
	l	g7,o7f24
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 223
	l	g6,fpsave+4
	l	g7,o7f24+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 224
	stfd	rf25,fpsave
	l	g6,fpsave
	l	g7,o7f25
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 225
	l	g6,fpsave+4
	l	g7,o7f25+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 226
	stfd	rf26,fpsave
	l	g6,fpsave
	l	g7,o7f26
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 227
	l	g6,fpsave+4
	l	g7,o7f26+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 228
	stfd	rf27,fpsave
	l	g6,fpsave
	l	g7,o7f27
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 229
	l	g6,fpsave+4
	l	g7,o7f27+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 230
	stfd	rf28,fpsave
	l	g6,fpsave
	l	g7,o7f28
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 231
	l	g6,fpsave+4
	l	g7,o7f28+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 232
	stfd	rf29,fpsave
	l	g6,fpsave
	l	g7,o7f29
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 233
	l	g6,fpsave+4
	l	g7,o7f29+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 234
	stfd	rf30,fpsave
	l	g6,fpsave
	l	g7,o7f30
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 235
	l	g6,fpsave+4
	l	g7,o7f30+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 236
	stfd	rf31,fpsave
	l	g6,fpsave
	l	g7,o7f31
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 237
	l	g6,fpsave+4
	l	g7,o7f31+4
	cmp	0,g6,g7
	bne	0,FINI
#
#
# 8 #
#
	lfd	rf0,i8f0
	lfd	rf1,i8f1
	lfd	rf2,i8f2
	lfd	rf4,i8f4
	lfd	rf5,i8f5
	lfd	rf7,i8f7
	lfd	rf8,i8f8
	lfd	rf10,i8f10
	lfd	rf11,i8f11
	lfd	rf13,i8f13
	lfd	rf14,i8f14
	lfd	rf16,i8f16
	lfd	rf17,i8f17
	lfd	rf19,i8f19
	lfd	rf20,i8f20
	lfd	rf22,i8f22
	lfd	rf23,i8f23
	lfd	rf25,i8f25
	lfd	rf26,i8f26
	lfd	rf28,i8f28
	lfd	rf29,i8f29
	lfd	rf31,i8f31
	l	g9,i8cr
	mtcrf	0xff,g9
	lfd	rf3,i8fp
	mtfsf	0xff,rf3
#
### instructions for 8 ###
#
	fma	r3,r1,r0,r2
	fma	r6,r4,r1,r5
	fma.	r9,r7,r2,r8
	fma	r12,r10,r3,r11
	fma	r15,r13,r4,r14
	fma	r18,r16,r5,r17
	fma.	r21,r19,r6,r20
	fma	r24,r22,r7,r23
	fma	r27,r25,r8,r26
	fma	r30,r28,r9,r29
	fma	r1,r31,r10,r0
	fma	r4,r2,r11,r3
	fma.	r7,r5,r12,r6
	fma.	r10,r8,r13,r9
	fma.	r13,r11,r14,r12
	fma.	r16,r14,r15,r15
	fma	r19,r17,r16,r18
	fma	r22,r20,r17,r21
	fma.	r25,r23,r18,r24
	fma	r28,r26,r19,r27
	fma	r31,r29,r20,r30
	fma	r2,r0,r21,r1
	fma.	r5,r3,r22,r4
	fma	r8,r6,r23,r7
	fma.	r11,r9,r24,r10
	fma	r14,r12,r25,r13
	fma.	r17,r15,r26,r16
	fma	r20,r18,r27,r19
	fma	r23,r21,r28,r22
	fma	r27,r24,r29,r25
	fma.	r31,r27,r30,r28
	fma	r2,r30,r31,r31
#
	ai	g3,g3,1				# RC = 238
	mfcr	g6
	l	g7,o8cr
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 239
	stfd	rf1,hold
	mffs	rf1
	stfd	rf1,fpsave
	l	g6,fpsave+4
	lfd	rf1,hold
	l	g7,o8fp
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 240
	stfd	rf1,fpsave
	l	g6,fpsave
	l	g7,o8f1
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 241
	l	g6,fpsave+4
	l	g7,o8f1+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 242
	stfd	rf2,fpsave
	l	g6,fpsave
	l	g7,o8f2
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 243
	l	g6,fpsave+4
	l	g7,o8f2+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 244
	stfd	rf3,fpsave
	l	g6,fpsave
	l	g7,o8f3
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 245
	l	g6,fpsave+4
	l	g7,o8f3+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 246
	stfd	rf4,fpsave
	l	g6,fpsave
	l	g7,o8f4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 247
	l	g6,fpsave+4
	l	g7,o8f4+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 248
	stfd	rf5,fpsave
	l	g6,fpsave
	l	g7,o8f5
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 249
	l	g6,fpsave+4
	l	g7,o8f5+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 250
	stfd	rf6,fpsave
	l	g6,fpsave
	l	g7,o8f6
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 251
	l	g6,fpsave+4
	l	g7,o8f6+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 252
	stfd	rf7,fpsave
	l	g6,fpsave
	l	g7,o8f7
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 253
	l	g6,fpsave+4
	l	g7,o8f7+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 254
	stfd	rf8,fpsave
	l	g6,fpsave
	l	g7,o8f8
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 255
	l	g6,fpsave+4
	l	g7,o8f8+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 256
	stfd	rf9,fpsave
	l	g6,fpsave
	l	g7,o8f9
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 257
	l	g6,fpsave+4
	l	g7,o8f9+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 258
	stfd	rf10,fpsave
	l	g6,fpsave
	l	g7,o8f10
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 259
	l	g6,fpsave+4
	l	g7,o8f10+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 260
	stfd	rf11,fpsave
	l	g6,fpsave
	l	g7,o8f11
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 261
	l	g6,fpsave+4
	l	g7,o8f11+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 262
	stfd	rf12,fpsave
	l	g6,fpsave
	l	g7,o8f12
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 263
	l	g6,fpsave+4
	l	g7,o8f12+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 264
	stfd	rf13,fpsave
	l	g6,fpsave
	l	g7,o8f13
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 265
	l	g6,fpsave+4
	l	g7,o8f13+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 266
	stfd	rf14,fpsave
	l	g6,fpsave
	l	g7,o8f14
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 267
	l	g6,fpsave+4
	l	g7,o8f14+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 268
	stfd	rf15,fpsave
	l	g6,fpsave
	l	g7,o8f15
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 269
	l	g6,fpsave+4
	l	g7,o8f15+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 270
	stfd	rf16,fpsave
	l	g6,fpsave
	l	g7,o8f16
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 271
	l	g6,fpsave+4
	l	g7,o8f16+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 272
	stfd	rf17,fpsave
	l	g6,fpsave
	l	g7,o8f17
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 273
	l	g6,fpsave+4
	l	g7,o8f17+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 274
	stfd	rf18,fpsave
	l	g6,fpsave
	l	g7,o8f18
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 275
	l	g6,fpsave+4
	l	g7,o8f18+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 276
	stfd	rf19,fpsave
	l	g6,fpsave
	l	g7,o8f19
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 277
	l	g6,fpsave+4
	l	g7,o8f19+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 278
	stfd	rf20,fpsave
	l	g6,fpsave
	l	g7,o8f20
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 279
	l	g6,fpsave+4
	l	g7,o8f20+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 280
	stfd	rf21,fpsave
	l	g6,fpsave
	l	g7,o8f21
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 281
	l	g6,fpsave+4
	l	g7,o8f21+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 282
	stfd	rf22,fpsave
	l	g6,fpsave
	l	g7,o8f22
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 283
	l	g6,fpsave+4
	l	g7,o8f22+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 284
	stfd	rf23,fpsave
	l	g6,fpsave
	l	g7,o8f23
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 285
	l	g6,fpsave+4
	l	g7,o8f23+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 286
	stfd	rf24,fpsave
	l	g6,fpsave
	l	g7,o8f24
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 287
	l	g6,fpsave+4
	l	g7,o8f24+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 288
	stfd	rf25,fpsave
	l	g6,fpsave
	l	g7,o8f25
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 289
	l	g6,fpsave+4
	l	g7,o8f25+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 290
	stfd	rf27,fpsave
	l	g6,fpsave
	l	g7,o8f27
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 291
	l	g6,fpsave+4
	l	g7,o8f27+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 292
	stfd	rf28,fpsave
	l	g6,fpsave
	l	g7,o8f28
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 293
	l	g6,fpsave+4
	l	g7,o8f28+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 294
	stfd	rf30,fpsave
	l	g6,fpsave
	l	g7,o8f30
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 295
	l	g6,fpsave+4
	l	g7,o8f30+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 296
	stfd	rf31,fpsave
	l	g6,fpsave
	l	g7,o8f31
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 297
	l	g6,fpsave+4
	l	g7,o8f31+4
	cmp	0,g6,g7
	bne	0,FINI
#
#
# 9 #
#
	lfd	rf0,i9f0
	lfd	rf1,i9f1
	lfd	rf2,i9f2
	lfd	rf4,i9f4
	lfd	rf5,i9f5
	lfd	rf7,i9f7
	lfd	rf8,i9f8
	lfd	rf10,i9f10
	lfd	rf11,i9f11
	lfd	rf13,i9f13
	lfd	rf14,i9f14
	lfd	rf16,i9f16
	lfd	rf17,i9f17
	lfd	rf19,i9f19
	lfd	rf20,i9f20
	lfd	rf22,i9f22
	lfd	rf23,i9f23
	lfd	rf25,i9f25
	lfd	rf26,i9f26
	lfd	rf28,i9f28
	lfd	rf29,i9f29
	lfd	rf31,i9f31
	l	g9,i9cr
	mtcrf	0xff,g9
	lfd	rf3,i9fp
	mtfsf	0xff,rf3
#
### instructions for 9 ###
#
	fma.	r3,r1,r2,r0
	fma.	r6,r4,r5,r1
	fma.	r9,r7,r8,r2
	fma	r12,r10,r11,r3
	fma	r15,r13,r14,r4
	fma	r18,r16,r17,r5
	fma	r21,r19,r20,r6
	fma	r24,r22,r23,r7
	fma	r27,r25,r26,r8
	fma.	r30,r28,r29,r9
	fma	r1,r31,r0,r10
	fma.	r4,r2,r3,r11
	fma.	r7,r5,r6,r12
	fma.	r10,r8,r9,r13
	fma	r13,r11,r12,r14
	fma.	r16,r14,r15,r15
	fma.	r19,r17,r18,r16
	fma	r22,r20,r21,r17
	fma.	r25,r23,r24,r18
	fma.	r28,r26,r27,r19
	fma	r31,r29,r30,r20
	fma	r2,r0,r1,r21
	fma.	r5,r3,r4,r22
	fma	r8,r6,r7,r23
	fma.	r11,r9,r10,r24
	fma	r14,r12,r13,r25
	fma	r17,r15,r16,r26
	fma.	r20,r18,r19,r27
	fma	r23,r21,r22,r28
	fma	r27,r24,r25,r29
	fma	r31,r27,r28,r30
	fma	r2,r30,r31,r31
#
	ai	g3,g3,1				# RC = 298
	mfcr	g6
	l	g7,o9cr
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 299
	stfd	rf1,hold
	mffs	rf1
	stfd	rf1,fpsave
	l	g6,fpsave+4
	lfd	rf1,hold
	l	g7,o9fp
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 300
	stfd	rf1,fpsave
	l	g6,fpsave
	l	g7,o9f1
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 301
	l	g6,fpsave+4
	l	g7,o9f1+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 302
	stfd	rf2,fpsave
	l	g6,fpsave
	l	g7,o9f2
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 303
	l	g6,fpsave+4
	l	g7,o9f2+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 304
	stfd	rf3,fpsave
	l	g6,fpsave
	l	g7,o9f3
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 305
	l	g6,fpsave+4
	l	g7,o9f3+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 306
	stfd	rf4,fpsave
	l	g6,fpsave
	l	g7,o9f4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 307
	l	g6,fpsave+4
	l	g7,o9f4+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 308
	stfd	rf5,fpsave
	l	g6,fpsave
	l	g7,o9f5
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 309
	l	g6,fpsave+4
	l	g7,o9f5+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 310
	stfd	rf6,fpsave
	l	g6,fpsave
	l	g7,o9f6
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 311
	l	g6,fpsave+4
	l	g7,o9f6+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 312
	stfd	rf7,fpsave
	l	g6,fpsave
	l	g7,o9f7
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 313
	l	g6,fpsave+4
	l	g7,o9f7+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 314
	stfd	rf8,fpsave
	l	g6,fpsave
	l	g7,o9f8
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 315
	l	g6,fpsave+4
	l	g7,o9f8+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 316
	stfd	rf9,fpsave
	l	g6,fpsave
	l	g7,o9f9
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 317
	l	g6,fpsave+4
	l	g7,o9f9+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 318
	stfd	rf10,fpsave
	l	g6,fpsave
	l	g7,o9f10
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 319
	l	g6,fpsave+4
	l	g7,o9f10+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 320
	stfd	rf11,fpsave
	l	g6,fpsave
	l	g7,o9f11
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 321
	l	g6,fpsave+4
	l	g7,o9f11+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 322
	stfd	rf12,fpsave
	l	g6,fpsave
	l	g7,o9f12
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 323
	l	g6,fpsave+4
	l	g7,o9f12+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 324
	stfd	rf13,fpsave
	l	g6,fpsave
	l	g7,o9f13
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 325
	l	g6,fpsave+4
	l	g7,o9f13+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 326
	stfd	rf14,fpsave
	l	g6,fpsave
	l	g7,o9f14
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 327
	l	g6,fpsave+4
	l	g7,o9f14+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 328
	stfd	rf15,fpsave
	l	g6,fpsave
	l	g7,o9f15
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 329
	l	g6,fpsave+4
	l	g7,o9f15+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 330
	stfd	rf16,fpsave
	l	g6,fpsave
	l	g7,o9f16
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 331
	l	g6,fpsave+4
	l	g7,o9f16+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 332
	stfd	rf17,fpsave
	l	g6,fpsave
	l	g7,o9f17
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 333
	l	g6,fpsave+4
	l	g7,o9f17+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 334
	stfd	rf18,fpsave
	l	g6,fpsave
	l	g7,o9f18
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 335
	l	g6,fpsave+4
	l	g7,o9f18+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 336
	stfd	rf19,fpsave
	l	g6,fpsave
	l	g7,o9f19
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 337
	l	g6,fpsave+4
	l	g7,o9f19+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 338
	stfd	rf20,fpsave
	l	g6,fpsave
	l	g7,o9f20
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 339
	l	g6,fpsave+4
	l	g7,o9f20+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 340
	stfd	rf21,fpsave
	l	g6,fpsave
	l	g7,o9f21
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 341
	l	g6,fpsave+4
	l	g7,o9f21+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 342
	stfd	rf22,fpsave
	l	g6,fpsave
	l	g7,o9f22
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 343
	l	g6,fpsave+4
	l	g7,o9f22+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 344
	stfd	rf23,fpsave
	l	g6,fpsave
	l	g7,o9f23
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 345
	l	g6,fpsave+4
	l	g7,o9f23+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 346
	stfd	rf24,fpsave
	l	g6,fpsave
	l	g7,o9f24
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 347
	l	g6,fpsave+4
	l	g7,o9f24+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 348
	stfd	rf25,fpsave
	l	g6,fpsave
	l	g7,o9f25
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 349
	l	g6,fpsave+4
	l	g7,o9f25+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 350
	stfd	rf27,fpsave
	l	g6,fpsave
	l	g7,o9f27
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 351
	l	g6,fpsave+4
	l	g7,o9f27+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 352
	stfd	rf28,fpsave
	l	g6,fpsave
	l	g7,o9f28
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 353
	l	g6,fpsave+4
	l	g7,o9f28+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 354
	stfd	rf30,fpsave
	l	g6,fpsave
	l	g7,o9f30
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 355
	l	g6,fpsave+4
	l	g7,o9f30+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 356
	stfd	rf31,fpsave
	l	g6,fpsave
	l	g7,o9f31
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 357
	l	g6,fpsave+4
	l	g7,o9f31+4
	cmp	0,g6,g7
	bne	0,FINI
#
#
# 10 #
#
	lfd	rf0,i10f0
	lfd	rf1,i10f1
	lfd	rf2,i10f2
	lfd	rf4,i10f4
	lfd	rf5,i10f5
	lfd	rf7,i10f7
	lfd	rf8,i10f8
	lfd	rf10,i10f10
	lfd	rf11,i10f11
	lfd	rf12,i10f12
	lfd	rf13,i10f13
	lfd	rf14,i10f14
	lfd	rf16,i10f16
	lfd	rf17,i10f17
	lfd	rf19,i10f19
	lfd	rf20,i10f20
	lfd	rf22,i10f22
	lfd	rf23,i10f23
	lfd	rf25,i10f25
	lfd	rf26,i10f26
	lfd	rf28,i10f28
	lfd	rf29,i10f29
	lfd	rf31,i10f31
	l	g9,i10cr
	mtcrf	0xff,g9
	lfd	rf3,i10fp
	mtfsf	0xff,rf3
#
### instructions for 10 ###
#
	fma.	r3,r0,r2,r1
	fma	r6,r1,r5,r4
	fma.	r9,r2,r8,r7
	fma.	r12,r3,r11,r10
	fma.	r15,r4,r14,r13
	fma.	r18,r5,r17,r16
	fma	r21,r6,r20,r19
	fma	r24,r7,r23,r22
	fma.	r27,r8,r26,r25
	fma.	r30,r9,r29,r28
	fma	r1,r10,r0,r31
	fma.	r4,r11,r3,r2
	fma.	r7,r12,r6,r5
	fma.	r10,r13,r9,r8
	fma.	r13,r14,r12,r11
	fma.	r16,r15,r15,r14
	fma	r19,r16,r18,r17
	fma.	r22,r17,r21,r20
	fma	r25,r18,r24,r23
	fma.	r28,r19,r27,r26
	fma	r31,r20,r30,r29
	fma.	r2,r21,r1,r0
	fma	r5,r22,r4,r3
	fma.	r8,r23,r7,r6
	fma.	r11,r24,r10,r9
	fma.	r14,r25,r13,r12
	fma.	r17,r26,r16,r15
	fma	r20,r27,r19,r18
	fma	r23,r28,r22,r21
	fma	r27,r29,r25,r24
	fma	r31,r30,r28,r27
	fma	r2,r31,r31,r30
#
	ai	g3,g3,1				# RC = 358
	mfcr	g6
	l	g7,o10cr
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 359
	stfd	rf1,hold
	mffs	rf1
	stfd	rf1,fpsave
	l	g6,fpsave+4
	lfd	rf1,hold
	l	g7,o10fp
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 360
	stfd	rf1,fpsave
	l	g6,fpsave
	l	g7,o10f1
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 361
	l	g6,fpsave+4
	l	g7,o10f1+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 362
	stfd	rf2,fpsave
	l	g6,fpsave
	l	g7,o10f2
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 363
	l	g6,fpsave+4
	l	g7,o10f2+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 364
	stfd	rf3,fpsave
	l	g6,fpsave
	l	g7,o10f3
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 365
	l	g6,fpsave+4
	l	g7,o10f3+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 366
	stfd	rf4,fpsave
	l	g6,fpsave
	l	g7,o10f4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 367
	l	g6,fpsave+4
	l	g7,o10f4+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 368
	stfd	rf5,fpsave
	l	g6,fpsave
	l	g7,o10f5
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 369
	l	g6,fpsave+4
	l	g7,o10f5+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 370
	stfd	rf6,fpsave
	l	g6,fpsave
	l	g7,o10f6
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 371
	l	g6,fpsave+4
	l	g7,o10f6+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 372
	stfd	rf7,fpsave
	l	g6,fpsave
	l	g7,o10f7
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 373
	l	g6,fpsave+4
	l	g7,o10f7+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 374
	stfd	rf8,fpsave
	l	g6,fpsave
	l	g7,o10f8
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 375
	l	g6,fpsave+4
	l	g7,o10f8+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 376
	stfd	rf9,fpsave
	l	g6,fpsave
	l	g7,o10f9
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 377
	l	g6,fpsave+4
	l	g7,o10f9+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 378
	stfd	rf10,fpsave
	l	g6,fpsave
	l	g7,o10f10
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 379
	l	g6,fpsave+4
	l	g7,o10f10+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 380
	stfd	rf11,fpsave
	l	g6,fpsave
	l	g7,o10f11
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 381
	l	g6,fpsave+4
	l	g7,o10f11+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 382
	stfd	rf12,fpsave
	l	g6,fpsave
	l	g7,o10f12
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 383
	l	g6,fpsave+4
	l	g7,o10f12+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 384
	stfd	rf13,fpsave
	l	g6,fpsave
	l	g7,o10f13
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 385
	l	g6,fpsave+4
	l	g7,o10f13+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 386
	stfd	rf14,fpsave
	l	g6,fpsave
	l	g7,o10f14
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 387
	l	g6,fpsave+4
	l	g7,o10f14+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 388
	stfd	rf15,fpsave
	l	g6,fpsave
	l	g7,o10f15
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 389
	l	g6,fpsave+4
	l	g7,o10f15+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 390
	stfd	rf16,fpsave
	l	g6,fpsave
	l	g7,o10f16
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 391
	l	g6,fpsave+4
	l	g7,o10f16+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 392
	stfd	rf17,fpsave
	l	g6,fpsave
	l	g7,o10f17
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 393
	l	g6,fpsave+4
	l	g7,o10f17+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 394
	stfd	rf18,fpsave
	l	g6,fpsave
	l	g7,o10f18
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 395
	l	g6,fpsave+4
	l	g7,o10f18+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 396
	stfd	rf19,fpsave
	l	g6,fpsave
	l	g7,o10f19
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 397
	l	g6,fpsave+4
	l	g7,o10f19+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 398
	stfd	rf20,fpsave
	l	g6,fpsave
	l	g7,o10f20
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 399
	l	g6,fpsave+4
	l	g7,o10f20+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 400
	stfd	rf21,fpsave
	l	g6,fpsave
	l	g7,o10f21
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 401
	l	g6,fpsave+4
	l	g7,o10f21+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 402
	stfd	rf22,fpsave
	l	g6,fpsave
	l	g7,o10f22
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 403
	l	g6,fpsave+4
	l	g7,o10f22+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 404
	stfd	rf23,fpsave
	l	g6,fpsave
	l	g7,o10f23
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 405
	l	g6,fpsave+4
	l	g7,o10f23+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 406
	stfd	rf24,fpsave
	l	g6,fpsave
	l	g7,o10f24
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 407
	l	g6,fpsave+4
	l	g7,o10f24+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 408
	stfd	rf25,fpsave
	l	g6,fpsave
	l	g7,o10f25
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 409
	l	g6,fpsave+4
	l	g7,o10f25+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 410
	stfd	rf27,fpsave
	l	g6,fpsave
	l	g7,o10f27
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 411
	l	g6,fpsave+4
	l	g7,o10f27+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 412
	stfd	rf28,fpsave
	l	g6,fpsave
	l	g7,o10f28
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 413
	l	g6,fpsave+4
	l	g7,o10f28+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 414
	stfd	rf30,fpsave
	l	g6,fpsave
	l	g7,o10f30
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 415
	l	g6,fpsave+4
	l	g7,o10f30+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 416
	stfd	rf31,fpsave
	l	g6,fpsave
	l	g7,o10f31
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 417
	l	g6,fpsave+4
	l	g7,o10f31+4
	cmp	0,g6,g7
	bne	0,FINI
#
#
# 11 #
#
	lfd	rf1,i11f1
	lfd	rf2,i11f2
	lfd	rf4,i11f4
	lfd	rf5,i11f5
	lfd	rf7,i11f7
	lfd	rf8,i11f8
	lfd	rf10,i11f10
	lfd	rf11,i11f11
	lfd	rf13,i11f13
	lfd	rf14,i11f14
	lfd	rf15,i11f15
	lfd	rf16,i11f16
	lfd	rf17,i11f17
	lfd	rf18,i11f18
	lfd	rf19,i11f19
	lfd	rf20,i11f20
	lfd	rf21,i11f21
	lfd	rf22,i11f22
	lfd	rf23,i11f23
	lfd	rf24,i11f24
	lfd	rf25,i11f25
	lfd	rf26,i11f26
	lfd	rf27,i11f27
	lfd	rf28,i11f28
	lfd	rf29,i11f29
	lfd	rf30,i11f30
	lfd	rf31,i11f31
	l	g9,i11cr
	mtcrf	0xff,g9
	lfd	rf3,i11fp
	mtfsf	0xff,rf3
#
### instructions for 11 ###
#
	fd.	r0,r1,r2
	fd	r1,r4,r5
	fd	r2,r7,r8
	fd.	r3,r10,r11
	fd.	r4,r13,r14
	fd.	r5,r16,r17
	fd	r6,r19,r20
	fd.	r7,r22,r23
	fd.	r8,r25,r26
	fd.	r9,r28,r29
	fd.	r10,r31,r0
	fd.	r11,r2,r3
	fd	r12,r5,r6
	fd.	r13,r8,r9
	fd	r14,r11,r12
	fd.	r15,r14,r15
	fd.	r16,r17,r18
	fd	r17,r20,r21
	fd.	r18,r23,r24
	fd	r19,r26,r27
	fd	r20,r29,r30
	fd.	r21,r0,r1
	fd	r22,r3,r4
	fd	r23,r6,r7
	fd	r24,r9,r10
	fd.	r25,r12,r13
	fd	r26,r15,r16
	fd	r27,r18,r19
	fd.	r28,r21,r22
	fd	r29,r24,r25
	fd.	r30,r27,r28
	fd	r31,r30,r31
#
	ai	g3,g3,1				# RC = 418
	mfcr	g6
	l	g7,o11cr
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 419
	stfd	rf1,hold
	mffs	rf1
	stfd	rf1,fpsave
	l	g6,fpsave+4
	lfd	rf1,hold
	l	g7,o11fp
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 420
	stfd	rf0,fpsave
	l	g6,fpsave
	l	g7,o11f0
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 421
	l	g6,fpsave+4
	l	g7,o11f0+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 422
	stfd	rf1,fpsave
	l	g6,fpsave
	l	g7,o11f1
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 423
	l	g6,fpsave+4
	l	g7,o11f1+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 424
	stfd	rf2,fpsave
	l	g6,fpsave
	l	g7,o11f2
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 425
	l	g6,fpsave+4
	l	g7,o11f2+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 426
	stfd	rf3,fpsave
	l	g6,fpsave
	l	g7,o11f3
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 427
	l	g6,fpsave+4
	l	g7,o11f3+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 428
	stfd	rf4,fpsave
	l	g6,fpsave
	l	g7,o11f4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 429
	l	g6,fpsave+4
	l	g7,o11f4+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 430
	stfd	rf5,fpsave
	l	g6,fpsave
	l	g7,o11f5
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 431
	l	g6,fpsave+4
	l	g7,o11f5+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 432
	stfd	rf6,fpsave
	l	g6,fpsave
	l	g7,o11f6
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 433
	l	g6,fpsave+4
	l	g7,o11f6+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 434
	stfd	rf7,fpsave
	l	g6,fpsave
	l	g7,o11f7
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 435
	l	g6,fpsave+4
	l	g7,o11f7+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 436
	stfd	rf8,fpsave
	l	g6,fpsave
	l	g7,o11f8
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 437
	l	g6,fpsave+4
	l	g7,o11f8+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 438
	stfd	rf9,fpsave
	l	g6,fpsave
	l	g7,o11f9
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 439
	l	g6,fpsave+4
	l	g7,o11f9+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 440
	stfd	rf10,fpsave
	l	g6,fpsave
	l	g7,o11f10
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 441
	l	g6,fpsave+4
	l	g7,o11f10+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 442
	stfd	rf11,fpsave
	l	g6,fpsave
	l	g7,o11f11
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 443
	l	g6,fpsave+4
	l	g7,o11f11+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 444
	stfd	rf12,fpsave
	l	g6,fpsave
	l	g7,o11f12
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 445
	l	g6,fpsave+4
	l	g7,o11f12+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 446
	stfd	rf13,fpsave
	l	g6,fpsave
	l	g7,o11f13
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 447
	l	g6,fpsave+4
	l	g7,o11f13+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 448
	stfd	rf14,fpsave
	l	g6,fpsave
	l	g7,o11f14
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 449
	l	g6,fpsave+4
	l	g7,o11f14+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 450
	stfd	rf15,fpsave
	l	g6,fpsave
	l	g7,o11f15
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 451
	l	g6,fpsave+4
	l	g7,o11f15+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 452
	stfd	rf16,fpsave
	l	g6,fpsave
	l	g7,o11f16
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 453
	l	g6,fpsave+4
	l	g7,o11f16+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 454
	stfd	rf17,fpsave
	l	g6,fpsave
	l	g7,o11f17
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 455
	l	g6,fpsave+4
	l	g7,o11f17+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 456
	stfd	rf18,fpsave
	l	g6,fpsave
	l	g7,o11f18
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 457
	l	g6,fpsave+4
	l	g7,o11f18+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 458
	stfd	rf19,fpsave
	l	g6,fpsave
	l	g7,o11f19
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 459
	l	g6,fpsave+4
	l	g7,o11f19+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 460
	stfd	rf20,fpsave
	l	g6,fpsave
	l	g7,o11f20
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 461
	l	g6,fpsave+4
	l	g7,o11f20+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 462
	stfd	rf21,fpsave
	l	g6,fpsave
	l	g7,o11f21
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 463
	l	g6,fpsave+4
	l	g7,o11f21+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 464
	stfd	rf22,fpsave
	l	g6,fpsave
	l	g7,o11f22
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 465
	l	g6,fpsave+4
	l	g7,o11f22+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 466
	stfd	rf23,fpsave
	l	g6,fpsave
	l	g7,o11f23
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 467
	l	g6,fpsave+4
	l	g7,o11f23+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 468
	stfd	rf24,fpsave
	l	g6,fpsave
	l	g7,o11f24
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 469
	l	g6,fpsave+4
	l	g7,o11f24+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 470
	stfd	rf25,fpsave
	l	g6,fpsave
	l	g7,o11f25
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 471
	l	g6,fpsave+4
	l	g7,o11f25+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 472
	stfd	rf26,fpsave
	l	g6,fpsave
	l	g7,o11f26
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 473
	l	g6,fpsave+4
	l	g7,o11f26+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 474
	stfd	rf27,fpsave
	l	g6,fpsave
	l	g7,o11f27
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 475
	l	g6,fpsave+4
	l	g7,o11f27+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 476
	stfd	rf28,fpsave
	l	g6,fpsave
	l	g7,o11f28
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 477
	l	g6,fpsave+4
	l	g7,o11f28+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 478
	stfd	rf29,fpsave
	l	g6,fpsave
	l	g7,o11f29
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 479
	l	g6,fpsave+4
	l	g7,o11f29+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 480
	stfd	rf30,fpsave
	l	g6,fpsave
	l	g7,o11f30
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 481
	l	g6,fpsave+4
	l	g7,o11f30+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 482
	stfd	rf31,fpsave
	l	g6,fpsave
	l	g7,o11f31
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 483
	l	g6,fpsave+4
	l	g7,o11f31+4
	cmp	0,g6,g7
	bne	0,FINI
#
#
# 12 #
#
	lfd	rf0,i12f0
	lfd	rf1,i12f1
	lfd	rf2,i12f2
	lfd	rf4,i12f4
	lfd	rf5,i12f5
	lfd	rf7,i12f7
	lfd	rf8,i12f8
	lfd	rf10,i12f10
	lfd	rf11,i12f11
	lfd	rf13,i12f13
	lfd	rf14,i12f14
	lfd	rf16,i12f16
	lfd	rf17,i12f17
	lfd	rf19,i12f19
	lfd	rf20,i12f20
	lfd	rf22,i12f22
	lfd	rf23,i12f23
	lfd	rf25,i12f25
	lfd	rf26,i12f26
	lfd	rf27,i12f27
	lfd	rf28,i12f28
	lfd	rf29,i12f29
	lfd	rf31,i12f31
	l	g9,i12cr
	mtcrf	0xff,g9
	lfd	rf3,i12fp
	mtfsf	0xff,rf3
#
### instructions for 12 ###
#
	fd	r3,r0,r1
	fd.	r6,r1,r4
	fd	r9,r2,r7
	fd	r12,r3,r10
	fd	r15,r4,r13
	fd	r18,r5,r16
	fd.	r21,r6,r19
	fd	r24,r7,r22
	fd	r27,r8,r25
	fd	r30,r9,r28
	fd.	r1,r10,r31
	fd	r4,r11,r2
	fd	r7,r12,r5
	fd	r10,r13,r8
	fd	r13,r14,r11
	fd	r16,r15,r14
	fd.	r19,r16,r17
	fd.	r22,r17,r20
	fd	r25,r18,r23
	fd.	r28,r19,r26
	fd	r31,r20,r29
	fd.	r2,r21,r0
	fd	r5,r22,r3
	fd.	r8,r23,r6
	fd.	r11,r24,r9
	fd.	r14,r25,r12
	fd.	r17,r26,r15
	fd	r20,r27,r18
	fd.	r23,r28,r21
	fd.	r27,r29,r24
	fd.	r31,r30,r27
	fd.	r2,r31,r30
#
	ai	g3,g3,1				# RC = 484
	mfcr	g6
	l	g7,o12cr
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 485
	stfd	rf1,hold
	mffs	rf1
	stfd	rf1,fpsave
	l	g6,fpsave+4
	lfd	rf1,hold
	l	g7,o12fp
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 486
	stfd	rf1,fpsave
	l	g6,fpsave
	l	g7,o12f1
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 487
	l	g6,fpsave+4
	l	g7,o12f1+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 488
	stfd	rf2,fpsave
	l	g6,fpsave
	l	g7,o12f2
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 489
	l	g6,fpsave+4
	l	g7,o12f2+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 490
	stfd	rf3,fpsave
	l	g6,fpsave
	l	g7,o12f3
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 491
	l	g6,fpsave+4
	l	g7,o12f3+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 492
	stfd	rf4,fpsave
	l	g6,fpsave
	l	g7,o12f4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 493
	l	g6,fpsave+4
	l	g7,o12f4+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 494
	stfd	rf5,fpsave
	l	g6,fpsave
	l	g7,o12f5
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 495
	l	g6,fpsave+4
	l	g7,o12f5+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 496
	stfd	rf6,fpsave
	l	g6,fpsave
	l	g7,o12f6
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 497
	l	g6,fpsave+4
	l	g7,o12f6+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 498
	stfd	rf7,fpsave
	l	g6,fpsave
	l	g7,o12f7
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 499
	l	g6,fpsave+4
	l	g7,o12f7+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 500
	stfd	rf8,fpsave
	l	g6,fpsave
	l	g7,o12f8
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 501
	l	g6,fpsave+4
	l	g7,o12f8+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 502
	stfd	rf9,fpsave
	l	g6,fpsave
	l	g7,o12f9
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 503
	l	g6,fpsave+4
	l	g7,o12f9+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 504
	stfd	rf10,fpsave
	l	g6,fpsave
	l	g7,o12f10
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 505
	l	g6,fpsave+4
	l	g7,o12f10+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 506
	stfd	rf11,fpsave
	l	g6,fpsave
	l	g7,o12f11
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 507
	l	g6,fpsave+4
	l	g7,o12f11+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 508
	stfd	rf12,fpsave
	l	g6,fpsave
	l	g7,o12f12
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 509
	l	g6,fpsave+4
	l	g7,o12f12+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 510
	stfd	rf13,fpsave
	l	g6,fpsave
	l	g7,o12f13
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 511
	l	g6,fpsave+4
	l	g7,o12f13+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 512
	stfd	rf14,fpsave
	l	g6,fpsave
	l	g7,o12f14
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 513
	l	g6,fpsave+4
	l	g7,o12f14+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 514
	stfd	rf15,fpsave
	l	g6,fpsave
	l	g7,o12f15
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 515
	l	g6,fpsave+4
	l	g7,o12f15+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 516
	stfd	rf16,fpsave
	l	g6,fpsave
	l	g7,o12f16
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 517
	l	g6,fpsave+4
	l	g7,o12f16+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 518
	stfd	rf17,fpsave
	l	g6,fpsave
	l	g7,o12f17
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 519
	l	g6,fpsave+4
	l	g7,o12f17+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 520
	stfd	rf18,fpsave
	l	g6,fpsave
	l	g7,o12f18
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 521
	l	g6,fpsave+4
	l	g7,o12f18+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 522
	stfd	rf19,fpsave
	l	g6,fpsave
	l	g7,o12f19
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 523
	l	g6,fpsave+4
	l	g7,o12f19+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 524
	stfd	rf20,fpsave
	l	g6,fpsave
	l	g7,o12f20
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 525
	l	g6,fpsave+4
	l	g7,o12f20+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 526
	stfd	rf21,fpsave
	l	g6,fpsave
	l	g7,o12f21
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 527
	l	g6,fpsave+4
	l	g7,o12f21+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 528
	stfd	rf22,fpsave
	l	g6,fpsave
	l	g7,o12f22
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 529
	l	g6,fpsave+4
	l	g7,o12f22+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 530
	stfd	rf23,fpsave
	l	g6,fpsave
	l	g7,o12f23
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 531
	l	g6,fpsave+4
	l	g7,o12f23+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 532
	stfd	rf24,fpsave
	l	g6,fpsave
	l	g7,o12f24
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 533
	l	g6,fpsave+4
	l	g7,o12f24+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 534
	stfd	rf25,fpsave
	l	g6,fpsave
	l	g7,o12f25
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 535
	l	g6,fpsave+4
	l	g7,o12f25+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 536
	stfd	rf27,fpsave
	l	g6,fpsave
	l	g7,o12f27
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 537
	l	g6,fpsave+4
	l	g7,o12f27+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 538
	stfd	rf28,fpsave
	l	g6,fpsave
	l	g7,o12f28
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 539
	l	g6,fpsave+4
	l	g7,o12f28+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 540
	stfd	rf30,fpsave
	l	g6,fpsave
	l	g7,o12f30
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 541
	l	g6,fpsave+4
	l	g7,o12f30+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 542
	stfd	rf31,fpsave
	l	g6,fpsave
	l	g7,o12f31
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 543
	l	g6,fpsave+4
	l	g7,o12f31+4
	cmp	0,g6,g7
	bne	0,FINI
#
#
# 13 #
#
	lfd	rf0,i13f0
	lfd	rf1,i13f1
	lfd	rf2,i13f2
	lfd	rf4,i13f4
	lfd	rf5,i13f5
	lfd	rf7,i13f7
	lfd	rf8,i13f8
	lfd	rf10,i13f10
	lfd	rf11,i13f11
	lfd	rf13,i13f13
	lfd	rf14,i13f14
	lfd	rf16,i13f16
	lfd	rf17,i13f17
	lfd	rf19,i13f19
	lfd	rf20,i13f20
	lfd	rf22,i13f22
	lfd	rf23,i13f23
	lfd	rf25,i13f25
	lfd	rf26,i13f26
	lfd	rf28,i13f28
	lfd	rf29,i13f29
	lfd	rf31,i13f31
	l	g9,i13cr
	mtcrf	0xff,g9
	lfd	rf3,i13fp
	mtfsf	0xff,rf3
#
### instructions for 13 ###
#
	fd.	r3,r1,r0
	fd	r6,r4,r1
	fd	r9,r7,r2
	fd	r12,r10,r3
	fd	r15,r13,r4
	fd	r18,r16,r5
	fd	r21,r19,r6
	fd.	r24,r22,r7
	fd.	r27,r25,r8
	fd	r30,r28,r9
	fd	r1,r31,r10
	fd	r4,r2,r11
	fd	r7,r5,r12
	fd.	r10,r8,r13
	fd.	r13,r11,r14
	fd.	r16,r14,r15
	fd.	r19,r17,r16
	fd	r22,r20,r17
	fd	r25,r23,r18
	fd.	r28,r26,r19
	fd	r31,r29,r20
	fd	r2,r0,r21
	fd	r5,r3,r22
	fd	r8,r6,r23
	fd	r11,r9,r24
	fd	r14,r12,r25
	fd	r17,r15,r26
	fd	r20,r18,r27
	fd.	r23,r21,r28
	fd	r27,r24,r29
	fd	r31,r27,r30
	fd.	r2,r30,r31
#
	ai	g3,g3,1				# RC = 544
	mfcr	g6
	l	g7,o13cr
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 545
	stfd	rf1,hold
	mffs	rf1
	stfd	rf1,fpsave
	l	g6,fpsave+4
	lfd	rf1,hold
	l	g7,o13fp
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 546
	stfd	rf1,fpsave
	l	g6,fpsave
	l	g7,o13f1
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 547
	l	g6,fpsave+4
	l	g7,o13f1+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 548
	stfd	rf2,fpsave
	l	g6,fpsave
	l	g7,o13f2
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 549
	l	g6,fpsave+4
	l	g7,o13f2+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 550
	stfd	rf3,fpsave
	l	g6,fpsave
	l	g7,o13f3
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 551
	l	g6,fpsave+4
	l	g7,o13f3+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 552
	stfd	rf4,fpsave
	l	g6,fpsave
	l	g7,o13f4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 553
	l	g6,fpsave+4
	l	g7,o13f4+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 554
	stfd	rf5,fpsave
	l	g6,fpsave
	l	g7,o13f5
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 555
	l	g6,fpsave+4
	l	g7,o13f5+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 556
	stfd	rf6,fpsave
	l	g6,fpsave
	l	g7,o13f6
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 557
	l	g6,fpsave+4
	l	g7,o13f6+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 558
	stfd	rf7,fpsave
	l	g6,fpsave
	l	g7,o13f7
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 559
	l	g6,fpsave+4
	l	g7,o13f7+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 560
	stfd	rf8,fpsave
	l	g6,fpsave
	l	g7,o13f8
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 561
	l	g6,fpsave+4
	l	g7,o13f8+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 562
	stfd	rf9,fpsave
	l	g6,fpsave
	l	g7,o13f9
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 563
	l	g6,fpsave+4
	l	g7,o13f9+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 564
	stfd	rf10,fpsave
	l	g6,fpsave
	l	g7,o13f10
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 565
	l	g6,fpsave+4
	l	g7,o13f10+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 566
	stfd	rf11,fpsave
	l	g6,fpsave
	l	g7,o13f11
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 567
	l	g6,fpsave+4
	l	g7,o13f11+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 568
	stfd	rf12,fpsave
	l	g6,fpsave
	l	g7,o13f12
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 569
	l	g6,fpsave+4
	l	g7,o13f12+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 570
	stfd	rf13,fpsave
	l	g6,fpsave
	l	g7,o13f13
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 571
	l	g6,fpsave+4
	l	g7,o13f13+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 572
	stfd	rf14,fpsave
	l	g6,fpsave
	l	g7,o13f14
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 573
	l	g6,fpsave+4
	l	g7,o13f14+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 574
	stfd	rf15,fpsave
	l	g6,fpsave
	l	g7,o13f15
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 575
	l	g6,fpsave+4
	l	g7,o13f15+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 576
	stfd	rf16,fpsave
	l	g6,fpsave
	l	g7,o13f16
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 577
	l	g6,fpsave+4
	l	g7,o13f16+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 578
	stfd	rf17,fpsave
	l	g6,fpsave
	l	g7,o13f17
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 579
	l	g6,fpsave+4
	l	g7,o13f17+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 580
	stfd	rf18,fpsave
	l	g6,fpsave
	l	g7,o13f18
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 581
	l	g6,fpsave+4
	l	g7,o13f18+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 582
	stfd	rf19,fpsave
	l	g6,fpsave
	l	g7,o13f19
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 583
	l	g6,fpsave+4
	l	g7,o13f19+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 584
	stfd	rf20,fpsave
	l	g6,fpsave
	l	g7,o13f20
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 585
	l	g6,fpsave+4
	l	g7,o13f20+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 586
	stfd	rf21,fpsave
	l	g6,fpsave
	l	g7,o13f21
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 587
	l	g6,fpsave+4
	l	g7,o13f21+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 588
	stfd	rf22,fpsave
	l	g6,fpsave
	l	g7,o13f22
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 589
	l	g6,fpsave+4
	l	g7,o13f22+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 590
	stfd	rf23,fpsave
	l	g6,fpsave
	l	g7,o13f23
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 591
	l	g6,fpsave+4
	l	g7,o13f23+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 592
	stfd	rf24,fpsave
	l	g6,fpsave
	l	g7,o13f24
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 593
	l	g6,fpsave+4
	l	g7,o13f24+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 594
	stfd	rf25,fpsave
	l	g6,fpsave
	l	g7,o13f25
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 595
	l	g6,fpsave+4
	l	g7,o13f25+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 596
	stfd	rf27,fpsave
	l	g6,fpsave
	l	g7,o13f27
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 597
	l	g6,fpsave+4
	l	g7,o13f27+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 598
	stfd	rf28,fpsave
	l	g6,fpsave
	l	g7,o13f28
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 599
	l	g6,fpsave+4
	l	g7,o13f28+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 600
	stfd	rf30,fpsave
	l	g6,fpsave
	l	g7,o13f30
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 601
	l	g6,fpsave+4
	l	g7,o13f30+4
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 602
	stfd	rf31,fpsave
	l	g6,fpsave
	l	g7,o13f31
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1				# RC = 603
	l	g6,fpsave+4
	l	g7,o13f31+4
	cmp	0,g6,g7
	bne	0,FINI
#
#
	xor	g3,g3,g3		# Return code = 0; pass
#
FINI:	br
#
# Data Area
#
	.csect	_sys[RW],3
_sys: 
fpsave:	.long	0xffffffff, 0xffffffff
hold:	.long	0xffffffff, 0xffffffff
#
i1f1:	.long	0x0312467f, 0x810a7de7		# f1 0312467f810a7de7
i1f2:	.long	0x359c7da2, 0x1ac09561		# f2 359c7da21ac09561
i1f4:	.long	0x8c8df3e4, 0xc1522ad7		# f4 8c8df3e4c1522ad7
i1f5:	.long	0x7ad7cda7, 0x3155b8ed		# f5 7ad7cda73155b8ed
i1f6:	.long	0xffffffff, 0xfffff0fb		# f6 fffffffffffff0fb
i1f7:	.long	0x268131a2, 0xcd250dba		# f7 268131a2cd250dba
i1f8:	.long	0x819b7308, 0x92448f82		# f8 819b730892448f82
i1f13:	.long	0x30544678, 0x12350dcd		# f13 3054467812350dcd
i1f15:	.long	0xbefe68ca, 0x39f70856		# f15 befe68ca39f70856
i1f18:	.long	0x60c8f0fd, 0x9b318d7c		# f18 60c8f0fd9b318d7c
i1f19:	.long	0x4008fe70, 0xc1392d9f		# f19 4008fe70c1392d9f
i1f21:	.long	0x1b539db6, 0x93e0c109		# f21 1b539db693e0c109
i1f22:	.long	0x0312467f, 0x010a7de6		# f22 0312467f010a7de6
i1f23:	.long	0x9af46fa9, 0x170d193d		# f23 9af46fa9170d193d
i1f25:	.long	0x8232dfc4, 0x46b9ea0b		# f25 8232dfc446b9ea0b
i1f26:	.long	0xc60363d6, 0x46c2ef76		# f26 c60363d646c2ef76
i1f28:	.long	0xc7dffffe, 0x30a97bca		# f28 c7dffffe30a97bca
i1f29:	.long	0xe63f4908, 0xea550045		# f29 e63f4908ea550045
i1f30:	.long	0x80082b1f, 0x5eefe148		# f30 80082b1f5eefe148
i1cr:	.long	0x84ea4cf2			# cr 84ea4cf2
i1fp:	.long	0xffffffff, 0x00000000			# fpscr 00000000
o1cr:	.long	0x8f264c44			# cr 8f264c44
o1fp:	.long	0xf2f910fb			# fpscr f2f910fb
o1f0:	.long	0xc7dffffe, 0x40000000		# f0 c7dffffe40000000
o1f3:	.long	0xa5fbdf75, 0x764e0f57		# f3 a5fbdf75764e0f57
o1f7:	.long	0xffffffff, 0x80000000		# f7 ffffffff80000000
o1f11:	.long	0xa5fbdf75, 0x764e0f57		# f11 a5fbdf75764e0f57
o1f12:	.long	0xdfd7b39a, 0x982587bb		# f12 dfd7b39a982587bb
o1f16:	.long	0xffffffff, 0x82022000		# f16 ffffffff82022000
o1f20:	.long	0xa5fbdfaa, 0x6fd712d0		# f20 a5fbdfaa6fd712d0
o1f22:	.long	0x25fbdf75, 0x764e0f58		# f22 25fbdf75764e0f58
o1f23:	.long	0xffffffff, 0x80000000		# f23 ffffffff80000000
o1f24:	.long	0xa5fbdfaa, 0x6fd712d1		# f24 a5fbdfaa6fd712d1
o1f25:	.long	0x25fbdfaa, 0x6fd712d1		# f25 25fbdfaa6fd712d1
o1f28:	.long	0xffffffff, 0x80000000		# f28 ffffffff80000000
o1f29:	.long	0x25fbdfaa, 0x6fd712d1		# f29 25fbdfaa6fd712d1
o1f31:	.long	0xffffffff, 0x80000000		# f31 ffffffff80000000
#
i2f0:	.long	0x7fefffff, 0xffffffff		# f0 7fefffffffffffff
i2f1:	.long	0xffefffff, 0xffffffff		# f1 ffefffffffffffff
i2f2:	.long	0xe65ee3d2, 0x8559df93		# f2 e65ee3d28559df93
i2f6:	.long	0xfc840000, 0x00000000		# f6 fc84000000000000
i2f9:	.long	0xffeffffc, 0x00000000		# f9 ffeffffc00000000
i2f10:	.long	0x7fd00000, 0x0000007f		# f10 7fd000000000007f
i2f12:	.long	0x699e5a1d, 0x7e2bbc87		# f12 699e5a1d7e2bbc87
i2f13:	.long	0xc40fffff, 0xffffffff		# f13 c40fffffffffffff
i2f14:	.long	0xbff00000, 0x07ffffff		# f14 bff0000007ffffff
i2f17:	.long	0x7cb857dd, 0xd1f3fa57		# f17 7cb857ddd1f3fa57
i2f19:	.long	0x6a164082, 0xc489b16e		# f19 6a164082c489b16e
i2f20:	.long	0xfff0d6f7, 0x98c6a021		# f20 fff0d6f798c6a021
i2f22:	.long	0xffd00000, 0x00000003		# f22 ffd0000000000003
i2f24:	.long	0x7fefffff, 0xffffffff		# f24 7fefffffffffffff
i2f25:	.long	0xfc800000, 0x00000000		# f25 fc80000000000000
i2f27:	.long	0x7feffffe, 0x00000000		# f27 7feffffe00000000
i2f28:	.long	0x7fefffff, 0xf8000000		# f28 7feffffff8000000
i2f29:	.long	0xe106d519, 0x90b4ca9d		# f29 e106d51990b4ca9d
i2cr:	.long	0x09bdf4af			# cr 09bdf4af
i2fp:	.long	0xffffffff, 0x00000000			# fpscr 00000000
o2cr:	.long	0x3ab114a1			# cr 3ab114a1
o2fp:	.long	0xa1095000			# fpscr a1095000
o2f1:	.long	0x7ff8d6f7, 0x80000000		# f1 7ff8d6f780000000
o2f4:	.long	0x7ff8d6f7, 0x80000000		# f4 7ff8d6f780000000
o2f6:	.long	0x7ff8d6f7, 0x80000000		# f6 7ff8d6f780000000
o2f7:	.long	0xffffffff, 0xa1091000		# f7 ffffffffa1091000
o2f11:	.long	0xfff0d6f7, 0x98c6a021		# f11 fff0d6f798c6a021
o2f12:	.long	0x7ff8d6f7, 0x80000000		# f12 7ff8d6f780000000
o2f13:	.long	0x7ff8d6f7, 0x80000000		# f13 7ff8d6f780000000
o2f19:	.long	0x7ff0d6f7, 0x98c6a021		# f19 7ff0d6f798c6a021
o2f21:	.long	0x7ff8d6f7, 0x80000000		# f21 7ff8d6f780000000
o2f23:	.long	0xfff8d6f7, 0x98c6a021		# f23 fff8d6f798c6a021
o2f25:	.long	0x7ff8d6f7, 0x80000000		# f25 7ff8d6f780000000
o2f27:	.long	0x7ff00000, 0x00000000		# f27 7ff0000000000000
o2f29:	.long	0x7ff8d6f7, 0x80000000		# f29 7ff8d6f780000000
o2f31:	.long	0x7ff8d6f7, 0x80000000		# f31 7ff8d6f780000000
#
i3f3:	.long	0x0000045a, 0x12a30e98		# f3 0000045a12a30e98
i3f4:	.long	0x8329e082, 0xfac575e6		# f4 8329e082fac575e6
i3f5:	.long	0x800fff80, 0x00000000		# f5 800fff8000000000
i3f6:	.long	0x1c26884d, 0x6449d907		# f6 1c26884d6449d907
i3f10:	.long	0x825fffff, 0xffff8000		# f10 825fffffffff8000
i3f12:	.long	0x0000008d, 0x01406415		# f12 0000008d01406415
i3f17:	.long	0x80000000, 0x0019d649		# f17 800000000019d649
i3f18:	.long	0x00000000, 0x00000000		# f18 0000000000000000
i3f19:	.long	0x800887c7, 0xf35adddf		# f19 800887c7f35adddf
i3f20:	.long	0x9c49661c, 0x2ee53eb7		# f20 9c49661c2ee53eb7
i3f23:	.long	0x3d3b76d5, 0x066f5b5f		# f23 3d3b76d5066f5b5f
i3f27:	.long	0x000ffffe, 0x00000000		# f27 000ffffe00000000
i3f29:	.long	0x80000000, 0x00ab5427		# f29 8000000000ab5427
i3f30:	.long	0x000c0000, 0x00000000		# f30 000c000000000000
i3cr:	.long	0x716b3d00			# cr 716b3d00
i3fp:	.long	0xffffffff, 0x00000000			# fpscr 00000000
o3cr:	.long	0x7a6b3d00			# cr 7a6b3d00
o3fp:	.long	0xae211000			# fpscr ae211000
o3f2:	.long	0x81f4895b, 0x10f2aae4		# f2 81f4895b10f2aae4
o3f3:	.long	0x80000000, 0x00000000		# f3 8000000000000000
o3f8:	.long	0x7ff80000, 0x00000000		# f8 7ff8000000000000
o3f11:	.long	0x800fff80, 0x00000000		# f11 800fff8000000000
o3f13:	.long	0x80000000, 0x00000000		# f13 8000000000000000
o3f14:	.long	0x80000000, 0x00000000		# f14 8000000000000000
o3f15:	.long	0x025fffff, 0xffff8000		# f15 025fffffffff8000
o3f16:	.long	0x80000000, 0x0019d649		# f16 800000000019d649
o3f17:	.long	0x801bff80, 0x00000000		# f17 801bff8000000000
o3f21:	.long	0x7ff80000, 0x00000000		# f21 7ff8000000000000
o3f22:	.long	0x7ff80000, 0x00000000		# f22 7ff8000000000000
o3f25:	.long	0x80000000, 0x00000000		# f25 8000000000000000
o3f30:	.long	0x7ff00000, 0x00000000		# f30 7ff0000000000000
#
i4f0:	.long	0x800fffff, 0xfff00000		# f0 800ffffffff00000
i4f1:	.long	0x80000000, 0x006f7ab6		# f1 80000000006f7ab6
i4f3:	.long	0x12ab79b8, 0xdf8803fb		# f3 12ab79b8df8803fb
i4f5:	.long	0x00000000, 0x00000146		# f5 0000000000000146
i4f8:	.long	0x9c2a9573, 0x534b57f5		# f8 9c2a9573534b57f5
i4f9:	.long	0x9c44c48d, 0x7fc0467b		# f9 9c44c48d7fc0467b
i4f10:	.long	0x80000000, 0x00000000		# f10 8000000000000000
i4f13:	.long	0x9c45f0a0, 0xbfdce717		# f13 9c45f0a0bfdce717
i4f14:	.long	0x9c39f5ac, 0x82fa8f67		# f14 9c39f5ac82fa8f67
i4f15:	.long	0x9d604c64, 0x33fbcd2c		# f15 9d604c6433fbcd2c
i4f16:	.long	0x1c4ce1d0, 0x165b8469		# f16 1c4ce1d0165b8469
i4f17:	.long	0x006fffff, 0xfffffc00		# f17 006ffffffffffc00
i4f21:	.long	0x1c39ebfb, 0x94ee74a6		# f21 1c39ebfb94ee74a6
i4f31:	.long	0x00cfffff, 0xe0000000		# f31 00cfffffe0000000
i4cr:	.long	0x3dda901f			# cr 3dda901f
i4fp:	.long	0xffffffff, 0x00000000			# fpscr 00000000
o4cr:	.long	0x38da901f			# cr 38da901f
o4fp:	.long	0x8a002000			# fpscr 8a002000
o4f2:	.long	0x80000000, 0x006f7ab6		# f2 80000000006f7ab6
o4f6:	.long	0x006fffff, 0xfffffc00		# f6 006ffffffffffc00
o4f16:	.long	0x80000000, 0x00000000		# f16 8000000000000000
o4f17:	.long	0x006fffff, 0xfffffc00		# f17 006ffffffffffc00
o4f18:	.long	0x80000000, 0x00000000		# f18 8000000000000000
o4f19:	.long	0x00000000, 0x00000000		# f19 0000000000000000
o4f20:	.long	0x9c45f0a0, 0xbfdce717		# f20 9c45f0a0bfdce717
o4f22:	.long	0x806fffff, 0xfffffc00		# f22 806ffffffffffc00
o4f24:	.long	0x80000000, 0x006f7ab6		# f24 80000000006f7ab6
o4f25:	.long	0x80000000, 0x00000000		# f25 8000000000000000
o4f26:	.long	0x80000000, 0x00000000		# f26 8000000000000000
o4f31:	.long	0x00000000, 0x00000000		# f31 0000000000000000
#
i5f0:	.long	0x000004ff, 0x74998385		# f0 000004ff74998385
i5f2:	.long	0x9c3997d2, 0x32e0b049		# f2 9c3997d232e0b049
i5f4:	.long	0x00000000, 0x00135f83		# f4 0000000000135f83
i5f7:	.long	0x006fffff, 0xf0000000		# f7 006ffffff0000000
i5f8:	.long	0x80000000, 0x00000000		# f8 8000000000000000
i5f9:	.long	0x9c255853, 0x6a99b23e		# f9 9c2558536a99b23e
i5f12:	.long	0x9c581530, 0xf97f81a5		# f12 9c581530f97f81a5
i5f14:	.long	0x1c6daea1, 0xe31bf684		# f14 1c6daea1e31bf684
i5f15:	.long	0x1c3bb491, 0xefec2166		# f15 1c3bb491efec2166
i5f20:	.long	0x00000622, 0x40eeffde		# f20 0000062240eeffde
i5f23:	.long	0x80000000, 0x00000007		# f23 8000000000000007
i5f24:	.long	0x9c2ae93a, 0xce39f1b7		# f24 9c2ae93ace39f1b7
i5f25:	.long	0x9c5e397c, 0xd3aec90c		# f25 9c5e397cd3aec90c
i5f27:	.long	0x9c48fac9, 0xed1bcf65		# f27 9c48fac9ed1bcf65
i5f30:	.long	0x80000001, 0x62a8c21a		# f30 8000000162a8c21a
i5cr:	.long	0xb6756cea			# cr b6756cea
i5fp:	.long	0xffffffff, 0x00000020			# fpscr 00000020
o5cr:	.long	0xbd756cea			# cr bd756cea
o5fp:	.long	0xda028020			# fpscr da028020
o5f1:	.long	0x0620a346, 0x68b5b10e		# f1 0620a34668b5b10e
o5f3:	.long	0x80000000, 0x00000000		# f3 8000000000000000
o5f4:	.long	0xb4e188e9, 0x26f3bb82		# f4 b4e188e926f3bb82
o5f6:	.long	0x589440be, 0x58ed0ce4		# f6 589440be58ed0ce4
o5f9:	.long	0x835c8ebd, 0x57da8c4a		# f9 835c8ebd57da8c4a
o5f11:	.long	0xdf73fdd2, 0x660e1400		# f11 df73fdd2660e1400
o5f16:	.long	0x9220a346, 0x60000000		# f16 9220a34660000000
o5f17:	.long	0x8380dcdf, 0x735e7cd5		# f17 8380dcdf735e7cd5
o5f20:	.long	0x912e5ced, 0xa0625629		# f20 912e5ceda0625629
o5f21:	.long	0x8620a346, 0x68b5b10e		# f21 8620a34668b5b10e
o5f24:	.long	0x1c5e397c, 0xd3aec90c		# f24 1c5e397cd3aec90c
o5f27:	.long	0x9c581530, 0xf97f81a5		# f27 9c581530f97f81a5
o5f31:	.long	0x8380dcdf, 0x735e7cd5		# f31 8380dcdf735e7cd5
#
i6f1:	.long	0x013fffff, 0xfffffffe		# f1 013ffffffffffffe
i6f2:	.long	0x1c482270, 0x9d9407d0		# f2 1c4822709d9407d0
i6f8:	.long	0x00affc00, 0x00000000		# f8 00affc0000000000
i6f9:	.long	0x1c2dad3c, 0x3b0f57ee		# f9 1c2dad3c3b0f57ee
i6f11:	.long	0x00dfffc0, 0x00000000		# f11 00dfffc000000000
i6f15:	.long	0x00afffff, 0x00000000		# f15 00afffff00000000
i6f16:	.long	0x1c253ec6, 0xd51558ad		# f16 1c253ec6d51558ad
i6f17:	.long	0x9c2733af, 0x1833fece		# f17 9c2733af1833fece
i6f18:	.long	0x800fffff, 0xff800000		# f18 800fffffff800000
i6f23:	.long	0x800fffff, 0xfffe0000		# f23 800ffffffffe0000
i6f25:	.long	0x84eb1c08, 0xf866334c		# f25 84eb1c08f866334c
i6f26:	.long	0x800fff80, 0x00000000		# f26 800fff8000000000
i6cr:	.long	0xea4d1ea0			# cr ea4d1ea0
i6fp:	.long	0xffffffff, 0x00000020			# fpscr 00000020
o6cr:	.long	0xef4d1ea0			# cr ef4d1ea0
o6fp:	.long	0xfa411020			# fpscr fa411020
o6f4:	.long	0x7ff80000, 0x00000000		# f4 7ff8000000000000
o6f5:	.long	0x5ffff800, 0x00000000		# f5 5ffff80000000000
o6f6:	.long	0x7ff00000, 0x00000000		# f6 7ff0000000000000
o6f7:	.long	0x7ff80000, 0x00000000		# f7 7ff8000000000000
o6f9:	.long	0x7ff80000, 0x00000000		# f9 7ff8000000000000
o6f12:	.long	0x7ff80000, 0x00000000		# f12 7ff8000000000000
o6f13:	.long	0x7ff80000, 0x00000000		# f13 7ff8000000000000
o6f14:	.long	0x7ff00000, 0x00000000		# f14 7ff0000000000000
o6f18:	.long	0x7ff80000, 0x00000000		# f18 7ff8000000000000
o6f19:	.long	0xdffff800, 0x00000000		# f19 dffff80000000000
o6f20:	.long	0xdffff800, 0x00000000		# f20 dffff80000000000
o6f27:	.long	0x7ff80000, 0x00000000		# f27 7ff8000000000000
o6f29:	.long	0x7ff80000, 0x00000000		# f29 7ff8000000000000
o6f31:	.long	0x7ff80000, 0x00000000		# f31 7ff8000000000000
#
i7f1:	.long	0x65b00000, 0x00000001		# f1 65b0000000000001
i7f2:	.long	0x7c412eb1, 0x43c2976d		# f2 7c412eb143c2976d
i7f3:	.long	0xd6812eb1, 0x43c2976c		# f3 d6812eb143c2976c
i7f4:	.long	0x5a496e60, 0x01c0824e		# f4 5a496e6001c0824e
i7f5:	.long	0xa7be051c, 0x673bd741		# f5 a7be051c673bd741
i7f6:	.long	0x92528cba, 0x0e5a1d94		# f6 92528cba0e5a1d94
i7f7:	.long	0x8e2001ff, 0xffffffff		# f7 8e2001ffffffffff
i7f8:	.long	0x00000000, 0x0001ffff		# f8 000000000001ffff
i7f9:	.long	0x85f00000, 0x3fffffff		# f9 85f000003fffffff
i7f10:	.long	0x1db322dd, 0xa2cee1c5		# f10 1db322dda2cee1c5
i7f11:	.long	0x9f028ed8, 0x4f230e2a		# f11 9f028ed84f230e2a
i7f12:	.long	0x9c548aa0, 0x8611a884		# f12 9c548aa08611a884
i7f13:	.long	0x8532421d, 0x2eed54bf		# f13 8532421d2eed54bf
i7f14:	.long	0x1c23965b, 0xcb46bb8a		# f14 1c23965bcb46bb8a
i7f15:	.long	0x035a69ec, 0xe56893be		# f15 035a69ece56893be
i7f16:	.long	0xf25be224, 0xcac2ee57		# f16 f25be224cac2ee57
i7f17:	.long	0x74e6aece, 0x8b9dcbf3		# f17 74e6aece8b9dcbf3
i7f18:	.long	0xe8899a6c, 0xe2a58bd1		# f18 e8899a6ce2a58bd1
i7f19:	.long	0xe67ca23e, 0x4c96ede6		# f19 e67ca23e4c96ede6
i7f20:	.long	0xbba03db2, 0x0e8fe2d8		# f20 bba03db20e8fe2d8
i7f21:	.long	0x194b0d1e, 0xba90e149		# f21 194b0d1eba90e149
i7f22:	.long	0x1c6e393a, 0x06d95ad8		# f22 1c6e393a06d95ad8
i7f23:	.long	0x80000000, 0x00000004		# f23 8000000000000004
i7f24:	.long	0x9c519107, 0x94f66c2b		# f24 9c51910794f66c2b
i7f25:	.long	0x945c972e, 0xbe9e6bd7		# f25 945c972ebe9e6bd7
i7f26:	.long	0x048a7988, 0x01bd9508		# f26 048a798801bd9508
i7f27:	.long	0x8b611883, 0xb9d4827f		# f27 8b611883b9d4827f
i7f28:	.long	0xee4692da, 0x50cef0d0		# f28 ee4692da50cef0d0
i7f29:	.long	0x70a41bf3, 0xefaba891		# f29 70a41bf3efaba891
i7f30:	.long	0x7417885e, 0x3cfc3aa5		# f30 7417885e3cfc3aa5
i7f31:	.long	0x23abfd32, 0x5cff66c8		# f31 23abfd325cff66c8
i7cr:	.long	0x05f58b0e			# cr 05f58b0e
i7fp:	.long	0xffffffff, 0xadb53042			# fpscr adb53042
o7cr:	.long	0x0ff58b0e			# cr 0ff58b0e
o7fp:	.long	0xffb28042			# fpscr ffb28042
o7f0:	.long	0xf8c2eb14, 0x3c2976c0		# f0 f8c2eb143c2976c0
o7f1:	.long	0xacad7bd9, 0x83f3cb4a		# f1 acad7bd983f3cb4a
o7f2:	.long	0x00000000, 0x00020000		# f2 0000000000020000
o7f3:	.long	0x9f028ed8, 0x4f230e2a		# f3 9f028ed84f230e2a
o7f4:	.long	0x1c23965b, 0xcb46bb8a		# f4 1c23965bcb46bb8a
o7f5:	.long	0x3af64f3b, 0xf031a45c		# f5 3af64f3bf031a45c
o7f6:	.long	0xbfd834a1, 0xf8f98ba3		# f6 bfd834a1f8f98ba3
o7f7:	.long	0x80000000, 0x00000004		# f7 8000000000000004
o7f8:	.long	0x048a7988, 0x01bd9509		# f8 048a798801bd9509
o7f9:	.long	0xc27099bf, 0x8452c122		# f9 c27099bf8452c122
o7f10:	.long	0xf8c2eb14, 0x3c2976c0		# f10 f8c2eb143c2976c0
o7f11:	.long	0x9f028ed8, 0x4f230e29		# f11 9f028ed84f230e29
o7f12:	.long	0xbfd834a1, 0xf8f98ba3		# f12 bfd834a1f8f98ba3
o7f13:	.long	0xc27099bf, 0x8452c122		# f13 c27099bf8452c122
o7f14:	.long	0xbfd834a1, 0xf8f98ba2		# f14 bfd834a1f8f98ba2
o7f15:	.long	0x72451778, 0x3696da27		# f15 724517783696da27
o7f16:	.long	0xbb744bf5, 0xd8c73664		# f16 bb744bf5d8c73664
o7f17:	.long	0x194b0d1a, 0xe4d84fc2		# f17 194b0d1ae4d84fc2
o7f18:	.long	0x9c519107, 0x94f66c2a		# f18 9c51910794f66c2a
o7f19:	.long	0xb2e2ad0c, 0x9c83dce4		# f19 b2e2ad0c9c83dce4
o7f20:	.long	0x7417885e, 0x3cfc3aa6		# f20 7417885e3cfc3aa6
o7f21:	.long	0xb6b2eb14, 0x3c2976c0		# f21 b6b2eb143c2976c0
o7f22:	.long	0x1c23965b, 0xcb404376		# f22 1c23965bcb404376
o7f23:	.long	0x847406b1, 0x7fc19fd9		# f23 847406b17fc19fd9
o7f24:	.long	0xf8c2eb14, 0x3c2976bf		# f24 f8c2eb143c2976bf
o7f25:	.long	0xc27099bf, 0x8452bed8		# f25 c27099bf8452bed8
o7f26:	.long	0x4ba1d470, 0xb0cced37		# f26 4ba1d470b0cced37
o7f27:	.long	0xd079d633, 0x1d95be45		# f27 d079d6331d95be45
o7f28:	.long	0x1c23965b, 0xcb404377		# f28 1c23965bcb404377
o7f29:	.long	0xa47514f5, 0x8aa30505		# f29 a47514f58aa30505
o7f30:	.long	0x3501057d, 0xab437889		# f30 3501057dab437889
o7f31:	.long	0xedd42043, 0x8d4d2051		# f31 edd420438d4d2051
#
i8f0:	.long	0x829c5c0b, 0xfaa25218		# f0 829c5c0bfaa25218
i8f1:	.long	0xece45826, 0x6171ea29		# f1 ece458266171ea29
i8f2:	.long	0x42f98df8, 0x5c781fad		# f2 42f98df85c781fad
i8f4:	.long	0x00000000, 0x00000000		# f4 0000000000000000
i8f5:	.long	0xfff00000, 0x00000000		# f5 fff0000000000000
i8f7:	.long	0x02400000, 0x00000001		# f7 0240000000000001
i8f8:	.long	0x03300065, 0xbc77a74d		# f8 03300065bc77a74d
i8f10:	.long	0xfcdfffff, 0xffffffff		# f10 fcdfffffffffffff
i8f11:	.long	0x7fefffff, 0xffffff80		# f11 7fefffffffffff80
i8f13:	.long	0xbf85c716, 0x696c34ea		# f13 bf85c716696c34ea
i8f14:	.long	0xafbeb702, 0xd9736a62		# f14 afbeb702d9736a62
i8f16:	.long	0xffd00000, 0x0000001f		# f16 ffd000000000001f
i8f17:	.long	0x7feffffe, 0x00000000		# f17 7feffffe00000000
i8f19:	.long	0x03645e15, 0xa77b140f		# f19 03645e15a77b140f
i8f20:	.long	0x4be7beda, 0x1891d0d1		# f20 4be7beda1891d0d1
i8f22:	.long	0x1caa1dbe, 0xda581de6		# f22 1caa1dbeda581de6
i8f23:	.long	0x80000000, 0x00000001		# f23 8000000000000001
i8f25:	.long	0x4f7fffff, 0xffffffff		# f25 4f7fffffffffffff
i8f26:	.long	0xffefffff, 0xf0000000		# f26 ffeffffff0000000
i8f28:	.long	0x00000000, 0x0000000f		# f28 000000000000000f
i8f29:	.long	0x00000000, 0x00000000		# f29 0000000000000000
i8f31:	.long	0x6c5881b7, 0x84ef94aa		# f31 6c5881b784ef94aa
i8cr:	.long	0x75458592			# cr 75458592
i8fp:	.long	0xffffffff, 0x7c865022			# fpscr 7c865022
o8cr:	.long	0x7f458592			# cr 7f458592
o8fp:	.long	0xfe809022			# fpscr fe809022
o8f1:	.long	0xffefffff, 0xffffffff		# f1 ffefffffffffffff
o8f2:	.long	0xfff00000, 0x00000000		# f2 fff0000000000000
o8f3:	.long	0x42f98df8, 0x5c781fae		# f3 42f98df85c781fae
o8f4:	.long	0x7ff00000, 0x00000000		# f4 7ff0000000000000
o8f5:	.long	0x7ff80000, 0x00000000		# f5 7ff8000000000000
o8f6:	.long	0xfff00000, 0x00000000		# f6 fff0000000000000
o8f7:	.long	0xfff00000, 0x00000000		# f7 fff0000000000000
o8f8:	.long	0x7ff80000, 0x00000000		# f8 7ff8000000000000
o8f9:	.long	0x05498df8, 0x5c801fe2		# f9 05498df85c801fe2
o8f10:	.long	0x05498df8, 0x5c800a1b		# f10 05498df85c800a1b
o8f11:	.long	0xa2298df8, 0x5c801fe1		# f11 a2298df85c801fe1
o8f12:	.long	0x7fc9c81e, 0x8e1f7f4c		# f12 7fc9c81e8e1f7f4c
o8f13:	.long	0x7fc9c81e, 0x8e1f7f4c		# f13 7fc9c81e8e1f7f4c
o8f14:	.long	0xfff00000, 0x00000000		# f14 fff0000000000000
o8f15:	.long	0xafbeb702, 0xd9736a62		# f15 afbeb702d9736a62
o8f16:	.long	0xafbeb702, 0xd9736a61		# f16 afbeb702d9736a61
o8f17:	.long	0x6fbeb702, 0xca17e8f6		# f17 6fbeb702ca17e8f6
o8f18:	.long	0x7ff00000, 0x00000000		# f18 7ff0000000000000
o8f19:	.long	0x7ff00000, 0x00000000		# f19 7ff0000000000000
o8f20:	.long	0x7ff80000, 0x00000000		# f20 7ff8000000000000
o8f21:	.long	0xfff00000, 0x00000000		# f21 fff0000000000000
o8f22:	.long	0xfff00000, 0x00000000		# f22 fff0000000000000
o8f23:	.long	0x7ff80000, 0x00000000		# f23 7ff8000000000000
o8f24:	.long	0xdccfffff, 0xffffffff		# f24 dccfffffffffffff
o8f25:	.long	0xfff00000, 0x00000000		# f25 fff0000000000000
o8f27:	.long	0xfff00000, 0x00000000		# f27 fff0000000000000
o8f28:	.long	0xfff00000, 0x00000000		# f28 fff0000000000000
o8f30:	.long	0x2267f518, 0xd6b81de4		# f30 2267f518d6b81de4
o8f31:	.long	0xfff00000, 0x00000000		# f31 fff0000000000000
#
i9f0:	.long	0x5b75277c, 0xe3364685		# f0 5b75277ce3364685
i9f1:	.long	0xfff00000, 0x00000000		# f1 fff0000000000000
i9f2:	.long	0xfff00000, 0x00000000		# f2 fff0000000000000
i9f4:	.long	0xffd000ff, 0xffffffff		# f4 ffd000ffffffffff
i9f5:	.long	0xc0001fff, 0xffffffff		# f5 c0001fffffffffff
i9f7:	.long	0x80000000, 0x0671ae77		# f7 800000000671ae77
i9f8:	.long	0x000009d9, 0x00ca4052		# f8 000009d900ca4052
i9f10:	.long	0x2dffffff, 0xffffffff		# f10 2dffffffffffffff
i9f11:	.long	0x921fffff, 0xfffffffd		# f11 921ffffffffffffd
i9f13:	.long	0x930ff87b, 0x7f8215a7		# f13 930ff87b7f8215a7
i9f14:	.long	0xbbf8a202, 0x952d56b0		# f14 bbf8a202952d56b0
i9f16:	.long	0x5c8fffff, 0xffffffff		# f16 5c8fffffffffffff
i9f17:	.long	0x84efdfff, 0xffffffff		# f17 84efdfffffffffff
i9f19:	.long	0xffd0ffff, 0xffffffff		# f19 ffd0ffffffffffff
i9f20:	.long	0x40100000, 0x0000007f		# f20 401000000000007f
i9f22:	.long	0x4a5fffff, 0xffffffff		# f22 4a5fffffffffffff
i9f23:	.long	0xf5800000, 0x00000001		# f23 f580000000000001
i9f25:	.long	0x9a549b9a, 0x94d5a510		# f25 9a549b9a94d5a510
i9f26:	.long	0xc4e85f8c, 0xff261e49		# f26 c4e85f8cff261e49
i9f28:	.long	0x76d09cbf, 0x67809f9f		# f28 76d09cbf67809f9f
i9f29:	.long	0x759b40e5, 0xa0ff5664		# f29 759b40e5a0ff5664
i9f31:	.long	0x6d25eb5d, 0xa0a7c00c		# f31 6d25eb5da0a7c00c
i9cr:	.long	0xd73cc3a5			# cr d73cc3a5
i9fp:	.long	0xffffffff, 0xfcf740b1			# fpscr fcf740b1
o9cr:	.long	0xdf3cc3a5			# cr df3cc3a5
o9fp:	.long	0xfef090b1			# fpscr fef090b1
o9f1:	.long	0x7fefffff, 0xffffffff		# f1 7fefffffffffffff
o9f2:	.long	0xfff00000, 0x00000000		# f2 fff0000000000000
o9f3:	.long	0x7ff00000, 0x00000000		# f3 7ff0000000000000
o9f4:	.long	0xfff00000, 0x00000000		# f4 fff0000000000000
o9f5:	.long	0xfff00000, 0x00000000		# f5 fff0000000000000
o9f6:	.long	0xfff00000, 0x00000000		# f6 fff0000000000000
o9f7:	.long	0x7ff00000, 0x00000000		# f7 7ff0000000000000
o9f8:	.long	0xfff00000, 0x00000000		# f8 fff0000000000000
o9f9:	.long	0xfff00000, 0x00000000		# f9 fff0000000000000
o9f10:	.long	0xfff00000, 0x00000000		# f10 fff0000000000000
o9f11:	.long	0x7ff00000, 0x00000000		# f11 7ff0000000000000
o9f12:	.long	0x7ff00000, 0x00000000		# f12 7ff0000000000000
o9f13:	.long	0xfff00000, 0x00000000		# f13 fff0000000000000
o9f14:	.long	0xfff00000, 0x00000000		# f14 fff0000000000000
o9f15:	.long	0xffd000ff, 0xfffffffe		# f15 ffd000fffffffffe
o9f16:	.long	0xffd000ff, 0xfffffffd		# f16 ffd000fffffffffd
o9f17:	.long	0x7fefffff, 0xffffffff		# f17 7fefffffffffffff
o9f18:	.long	0xc0001fff, 0xffffffff		# f18 c0001fffffffffff
o9f19:	.long	0xffd000ff, 0xfffffffc		# f19 ffd000fffffffffc
o9f20:	.long	0x7fe02101, 0xfffffffa		# f20 7fe02101fffffffa
o9f21:	.long	0xfff00000, 0x00000000		# f21 fff0000000000000
o9f22:	.long	0xfff00000, 0x00000000		# f22 fff0000000000000
o9f23:	.long	0x7ff00000, 0x00000000		# f23 7ff0000000000000
o9f24:	.long	0xffefffff, 0xffffffff		# f24 ffefffffffffffff
o9f25:	.long	0x7fefffff, 0xffffffff		# f25 7fefffffffffffff
o9f27:	.long	0xffefffff, 0xffffffff		# f27 ffefffffffffffff
o9f28:	.long	0xffd000ff, 0xfffffffc		# f28 ffd000fffffffffc
o9f30:	.long	0xfff00000, 0x00000000		# f30 fff0000000000000
o9f31:	.long	0xfff00000, 0x00000000		# f31 fff0000000000000
#
i10f0:	.long	0xffe66ae0, 0xba43c23d		# f0 ffe66ae0ba43c23d
i10f1:	.long	0x651e1e54, 0xefc46cc7		# f1 651e1e54efc46cc7
i10f2:	.long	0x2543fb91, 0x3c1798a4		# f2 2543fb913c1798a4
i10f4:	.long	0x80000000, 0x00000000		# f4 8000000000000000
i10f5:	.long	0x20001fff, 0xffffffff		# f5 20001fffffffffff
i10f7:	.long	0x49100000, 0x00000099		# f7 4910000000000099
i10f8:	.long	0xf2c00000, 0x00000098		# f8 f2c0000000000098
i10f10:	.long	0x9d2e4cda, 0xb0dae2f7		# f10 9d2e4cdab0dae2f7
i10f11:	.long	0x7ff66f1b, 0x50a39c8d		# f11 7ff66f1b50a39c8d
i10f12:	.long	0x58d06984, 0x7c31a8c3		# f12 58d069847c31a8c3
i10f13:	.long	0x800fffff, 0xffffffff		# f13 800fffffffffffff
i10f14:	.long	0x00100000, 0x0000001f		# f14 001000000000001f
i10f16:	.long	0x00000000, 0x00000000		# f16 0000000000000000
i10f17:	.long	0x1ff0ffff, 0xffffffff		# f17 1ff0ffffffffffff
i10f19:	.long	0x80000000, 0x00000000		# f19 8000000000000000
i10f20:	.long	0x1c48c3c4, 0xd1228e78		# f20 1c48c3c4d1228e78
i10f22:	.long	0x2abfffff, 0xff7ffffe		# f22 2abfffffff7ffffe
i10f23:	.long	0x014fffff, 0xff7fffff		# f23 014fffffff7fffff
i10f25:	.long	0x7a5794d0, 0xdc913bdd		# f25 7a5794d0dc913bdd
i10f26:	.long	0x687e8755, 0x94d2b0ab		# f26 687e875594d2b0ab
i10f28:	.long	0x80000071, 0xca63101b		# f28 80000071ca63101b
i10f29:	.long	0x8000003e, 0x3b6a4c5a		# f29 8000003e3b6a4c5a
i10f31:	.long	0x00000000, 0x00000000		# f31 0000000000000000
i10cr:	.long	0x8fea416c			# cr 8fea416c
i10fp:	.long	0xffffffff, 0x6cebd0f3			# fpscr 6cebd0f3
o10cr:	.long	0x8fea416c			# cr 8fea416c
o10fp:	.long	0xffea40f3			# fpscr ffea40f3
o10f1:	.long	0x5d253a09, 0xb85a070f		# f1 5d253a09b85a070f
o10f2:	.long	0x3fd57a16, 0xaa153194		# f2 3fd57a16aa153194
o10f3:	.long	0xe53477cd, 0xc36fd4af		# f3 e53477cdc36fd4af
o10f4:	.long	0x80000000, 0x00000000		# f4 8000000000000000
o10f5:	.long	0xe53477cd, 0xc36fd4af		# f5 e53477cdc36fd4af
o10f6:	.long	0x452e5a91, 0x99a3f59e		# f6 452e5a9199a3f59e
o10f7:	.long	0x5e0f22bf, 0x4bf18b10		# f7 5e0f22bf4bf18b10
o10f8:	.long	0x452e5a91, 0x99a3f59e		# f8 452e5a9199a3f59e
o10f9:	.long	0xd813fb91, 0x3c179962		# f9 d813fb913c179962
o10f10:	.long	0xf2c00000, 0x00000098		# f10 f2c0000000000098
o10f11:	.long	0xdd8fffff, 0xff80012e		# f11 dd8fffffff80012e
o10f12:	.long	0x58d06984, 0x7c31a8c3		# f12 58d069847c31a8c3
o10f13:	.long	0x800fffff, 0xffffffff		# f13 800fffffffffffff
o10f14:	.long	0x58d06984, 0x7c31a8c2		# f14 58d069847c31a8c2
o10f15:	.long	0xe00fffff, 0xfffffffe		# f15 e00ffffffffffffe
o10f16:	.long	0x202fffff, 0xfffffffc		# f16 202ffffffffffffc
o10f17:	.long	0xe00fffff, 0xfffffffe		# f17 e00ffffffffffffe
o10f18:	.long	0x600121ff, 0xfffffffd		# f18 600121fffffffffd
o10f19:	.long	0x404121ff, 0xfffffffa		# f19 404121fffffffffa
o10f20:	.long	0x600121ff, 0xfffffffc		# f20 600121fffffffffc
o10f21:	.long	0x21877d9f, 0xcb9752b5		# f21 21877d9fcb9752b5
o10f22:	.long	0x1c48c3c4, 0xd1228e78		# f22 1c48c3c4d1228e78
o10f23:	.long	0x44d7a044, 0xa591f698		# f23 44d7a044a591f698
o10f24:	.long	0x2abfffff, 0xff7ffffe		# f24 2abfffffff7ffffe
o10f25:	.long	0x4ad121ff, 0xffbb77fb		# f25 4ad121ffffbb77fb
o10f27:	.long	0x2abfffff, 0xff7ffffd		# f27 2abfffffff7ffffd
o10f28:	.long	0x687e8755, 0x94d2b0aa		# f28 687e875594d2b0aa
o10f30:	.long	0x17536e41, 0xcc99a41b		# f30 17536e41cc99a41b
o10f31:	.long	0x3fe2898a, 0xcca3d8a5		# f31 3fe2898acca3d8a5
#
i11f1:	.long	0xadb612b4, 0x5a33772e		# f1 adb612b45a33772e
i11f2:	.long	0x73a3fbc4, 0xd39f47da		# f2 73a3fbc4d39f47da
i11f4:	.long	0xfff00000, 0x00000000		# f4 fff0000000000000
i11f5:	.long	0xfff00000, 0x00000000		# f5 fff0000000000000
i11f7:	.long	0xbd3273b7, 0xc049f4d4		# f7 bd3273b7c049f4d4
i11f8:	.long	0x80000006, 0x3032714b		# f8 800000063032714b
i11f10:	.long	0xbb53c5ff, 0x700c97f2		# f10 bb53c5ff700c97f2
i11f11:	.long	0x51354dd7, 0x2c1c8403		# f11 51354dd72c1c8403
i11f13:	.long	0x6227aaab, 0xb95aec3b		# f13 6227aaabb95aec3b
i11f14:	.long	0xa1fab73d, 0xb8e1e8b7		# f14 a1fab73db8e1e8b7
i11f15:	.long	0x80000000, 0x07a8cd66		# f15 8000000007a8cd66
i11f16:	.long	0x80000000, 0x008a5d6c		# f16 80000000008a5d6c
i11f17:	.long	0x3d4e984c, 0xa94a1628		# f17 3d4e984ca94a1628
i11f18:	.long	0xcccacf23, 0x30ed00dc		# f18 cccacf2330ed00dc
i11f19:	.long	0x62194a6b, 0x38df40c1		# f19 62194a6b38df40c1
i11f20:	.long	0x78835e09, 0x6a0017b1		# f20 78835e096a0017b1
i11f21:	.long	0x7abf49f0, 0xc1773000		# f21 7abf49f0c1773000
i11f22:	.long	0x5f6e0e3a, 0x77513d49		# f22 5f6e0e3a77513d49
i11f23:	.long	0x95185ddc, 0x643232f0		# f23 95185ddc643232f0
i11f24:	.long	0x80000000, 0x02a94647		# f24 8000000002a94647
i11f25:	.long	0xb0ea3a4d, 0x6b8905bd		# f25 b0ea3a4d6b8905bd
i11f26:	.long	0xa837c114, 0xee01c24e		# f26 a837c114ee01c24e
i11f27:	.long	0x54f23c38, 0xa8e4ac91		# f27 54f23c38a8e4ac91
i11f28:	.long	0x18a0e71e, 0x1f70058d		# f28 18a0e71e1f70058d
i11f29:	.long	0x582eb35b, 0xb998f24c		# f29 582eb35bb998f24c
i11f30:	.long	0xfc6e6db7, 0x1f47417d		# f30 fc6e6db71f47417d
i11f31:	.long	0x0b604b1a, 0xe805d436		# f31 0b604b1ae805d436
i11cr:	.long	0x50601180			# cr 50601180
i11fp:	.long	0xffffffff, 0xf82e50b3			# fpscr f82e50b3
o11cr:	.long	0x5f601180			# cr 5f601180
o11fp:	.long	0xfe6890b3			# fpscr fe6890b3
o11f0:	.long	0xda01ac4d, 0x6e5a98bc		# f0 da01ac4d6e5a98bc
o11f1:	.long	0xadb612b4, 0x5a33772e		# f1 adb612b45a33772e
o11f2:	.long	0x7e27daac, 0x6721abf6		# f2 7e27daac6721abf6
o11f3:	.long	0xaa0db36d, 0xad3370fa		# f3 aa0db36dad3370fa
o11f4:	.long	0xfff00000, 0x00000000		# f4 fff0000000000000
o11f5:	.long	0x80e21705, 0x236ef752		# f5 80e21705236ef752
o11f6:	.long	0x2984e4bb, 0xe352fdfc		# f6 2984e4bbe352fdfc
o11f7:	.long	0xfff00000, 0x00000000		# f7 fff0000000000000
o11f8:	.long	0x48a1aa83, 0xcf10d231		# f8 48a1aa83cf10d231
o11f9:	.long	0x00619e42, 0x835aa179		# f9 00619e42835aa179
o11f10:	.long	0xd14d807a, 0x43b21259		# f10 d14d807a43b21259
o11f11:	.long	0xfff00000, 0x00000000		# f11 fff0000000000000
o11f12:	.long	0x974bb4c4, 0x02b36604		# f12 974bb4c402b36604
o11f13:	.long	0x7fefffff, 0xffffffff		# f13 7fefffffffffffff
o11f14:	.long	0x7ff00000, 0x00000000		# f14 7ff0000000000000
o11f15:	.long	0xfff00000, 0x00000000		# f15 fff0000000000000
o11f16:	.long	0xb0724268, 0x38833f85		# f16 b072426838833f85
o11f17:	.long	0x3db3ceba, 0x9f0538db		# f17 3db3ceba9f0538db
o11f18:	.long	0x56a24ff4, 0xa9a877d3		# f18 56a24ff4a9a877d3
o11f19:	.long	0x9334d7ac, 0xbf67e984		# f19 9334d7acbf67e984
o11f20:	.long	0x9bb0249e, 0xa9c3aa7d		# f20 9bb0249ea9c3aa7d
o11f21:	.long	0x6c399f05, 0x3566fe67		# f21 6c399f053566fe67
o11f22:	.long	0x00000000, 0x00000000		# f22 0000000000000000
o11f23:	.long	0x80000000, 0x00000000		# f23 8000000000000000
o11f24:	.long	0xcf031c2d, 0x03cdc00b		# f24 cf031c2d03cdc00b
o11f25:	.long	0xb74bb4c4, 0x02b36605		# f25 b74bb4c402b36605
o11f26:	.long	0x7ff00000, 0x00000000		# f26 7ff0000000000000
o11f27:	.long	0xfff00000, 0x00000000		# f27 fff0000000000000
o11f28:	.long	0x18a0e71e, 0x1f70058d		# f28 18a0e71e1f70058d
o11f29:	.long	0x57a6125d, 0x109034c6		# f29 57a6125d109034c6
o11f30:	.long	0xfff00000, 0x00000000		# f30 fff0000000000000
o11f31:	.long	0xfff00000, 0x00000000		# f31 fff0000000000000
#
i12f0:	.long	0xf5480b11, 0x8cb600ee		# f0 f5480b118cb600ee
i12f1:	.long	0xbd9c47af, 0xc4139b98		# f1 bd9c47afc4139b98
i12f2:	.long	0x3d2a2777, 0x42a1758b		# f2 3d2a277742a1758b
i12f4:	.long	0x17f52c57, 0xbf8512cf		# f4 17f52c57bf8512cf
i12f5:	.long	0xbc7e944b, 0xa7fafd30		# f5 bc7e944ba7fafd30
i12f7:	.long	0xfe388658, 0x55c3d69c		# f7 fe38865855c3d69c
i12f8:	.long	0xfff00000, 0x00000000		# f8 fff0000000000000
i12f10:	.long	0x3d5838b3, 0x46d6ff61		# f10 3d5838b346d6ff61
i12f11:	.long	0xfff00000, 0x00000000		# f11 fff0000000000000
i12f13:	.long	0x000000cc, 0x612a01f2		# f13 000000cc612a01f2
i12f14:	.long	0x3e832f2b, 0xda6be16f		# f14 3e832f2bda6be16f
i12f16:	.long	0xfecf40df, 0x51a5ff63		# f16 fecf40df51a5ff63
i12f17:	.long	0x3c91bf12, 0xb44e80ac		# f17 3c91bf12b44e80ac
i12f19:	.long	0x78afa8e3, 0x1704f770		# f19 78afa8e31704f770
i12f20:	.long	0xda949624, 0xbec697e9		# f20 da949624bec697e9
i12f22:	.long	0xbdb546ab, 0x8169c38f		# f22 bdb546ab8169c38f
i12f23:	.long	0xf394c53b, 0xa19f886f		# f23 f394c53ba19f886f
i12f25:	.long	0xfff00000, 0x00000000		# f25 fff0000000000000
i12f26:	.long	0x3e171f50, 0x98e88803		# f26 3e171f5098e88803
i12f27:	.long	0x71fe59ce, 0x88786f50		# f27 71fe59ce88786f50
i12f28:	.long	0x27cf363f, 0xe78cf409		# f28 27cf363fe78cf409
i12f29:	.long	0xfff00000, 0x00000000		# f29 fff0000000000000
i12f31:	.long	0x7ff00000, 0x00000000		# f31 7ff0000000000000
i12cr:	.long	0xa32c5a93			# cr a32c5a93
i12fp:	.long	0xffffffff, 0x78ad80eb			# fpscr 78ad80eb
o12cr:	.long	0xaf2c5a93			# cr af2c5a93
o12fp:	.long	0xfae920eb			# fpscr fae920eb
o12f1:	.long	0x00000000, 0x00000000		# f1 0000000000000000
o12f2:	.long	0x80000000, 0x00000000		# f2 8000000000000000
o12f3:	.long	0x779b34b5, 0xe7081d69		# f3 779b34b5e7081d69
o12f4:	.long	0xfff00000, 0x00000000		# f4 fff0000000000000
o12f5:	.long	0xca40391d, 0xc85413ae		# f5 ca40391dc85413ae
o12f6:	.long	0xe5955ed3, 0xa68ffb70		# f6 e5955ed3a68ffb70
o12f7:	.long	0xfda2ce6e, 0x75b3513d		# f7 fda2ce6e75b3513d
o12f8:	.long	0x4def1a02, 0x516b00f5		# f8 4def1a02516b00f5
o12f9:	.long	0xdee11021, 0x4fa5c5d0		# f9 dee110214fa5c5d0
o12f10:	.long	0x80000000, 0x00000000		# f10 8000000000000000
o12f11:	.long	0x81814b4b, 0x7e8c1a94		# f11 81814b4b7e8c1a94
o12f12:	.long	0x7a31f8ae, 0xc3c42f5e		# f12 7a31f8aec3c42f5e
o12f13:	.long	0x80000000, 0x00000000		# f13 8000000000000000
o12f14:	.long	0xcfb57913, 0x785a5b72		# f14 cfb57913785a5b72
o12f15:	.long	0x589a855d, 0xf1ff372b		# f15 589a855df1ff372b
o12f16:	.long	0x5a061e72, 0x6d8bb93f		# f16 5a061e726d8bb93f
o12f17:	.long	0x256be633, 0x96bfcbef		# f17 256be63396bfcbef
o12f18:	.long	0x5d9f4f4c, 0xf536d3be		# f18 5d9f4f4cf536d3be
o12f19:	.long	0x5d63f138, 0x0f465b77		# f19 5d63f1380f465b77
o12f20:	.long	0x544f0518, 0x1c0607ab		# f20 544f05181c0607ab
o12f21:	.long	0xacd599a0, 0xd789f80b		# f21 acd599a0d789f80b
o12f22:	.long	0xa1eb95d4, 0x533ab509		# f22 a1eb95d4533ab509
o12f23:	.long	0xf254718e, 0x60c4b2aa		# f23 f254718e60c4b2aa
o12f24:	.long	0x2072716f, 0x83561063		# f24 2072716f83561063
o12f25:	.long	0xa9f81e63, 0xe9181e94		# f25 a9f81e63e9181e94
o12f27:	.long	0xfff00000, 0x00000000		# f27 fff0000000000000
o12f28:	.long	0x5f3b9956, 0x43757d90		# f28 5f3b995643757d90
o12f30:	.long	0xf7017e6c, 0x6c982330		# f30 f7017e6c6c982330
o12f31:	.long	0x00000000, 0x00000000		# f31 0000000000000000
#
i13f0:	.long	0xd2e5c94d, 0xe191100e		# f0 d2e5c94de191100e
i13f1:	.long	0x52d07edd, 0x9a427bd0		# f1 52d07edd9a427bd0
i13f2:	.long	0x3e1849e7, 0x9e1bbc49		# f2 3e1849e79e1bbc49
i13f4:	.long	0xbf02a0e4, 0xc1144245		# f4 bf02a0e4c1144245
i13f5:	.long	0xf16d9bd6, 0x5095d29c		# f5 f16d9bd65095d29c
i13f7:	.long	0x80000000, 0x3d2717ae		# f7 800000003d2717ae
i13f8:	.long	0x14b2a570, 0x5b10921e		# f8 14b2a5705b10921e
i13f10:	.long	0x9e3c894e, 0x3f9a205d		# f10 9e3c894e3f9a205d
i13f11:	.long	0x83425b7c, 0xa8c9a35b		# f11 83425b7ca8c9a35b
i13f13:	.long	0x9efaf3df, 0x8ff01c7e		# f13 9efaf3df8ff01c7e
i13f14:	.long	0xfff00000, 0x00000000		# f14 fff0000000000000
i13f16:	.long	0x2fcd7354, 0x4b8fc2ed		# f16 2fcd73544b8fc2ed
i13f17:	.long	0xbd9aede4, 0xd30816ed		# f17 bd9aede4d30816ed
i13f19:	.long	0x7159d958, 0x9bda5836		# f19 7159d9589bda5836
i13f20:	.long	0x386a32f9, 0x4209542a		# f20 386a32f94209542a
i13f22:	.long	0x08120297, 0xf77627e5		# f22 08120297f77627e5
i13f23:	.long	0xfff948e9, 0xf7097711		# f23 fff948e9f7097711
i13f25:	.long	0x3f085d4e, 0x2faee22e		# f25 3f085d4e2faee22e
i13f26:	.long	0x8d4f7059, 0xa87368bf		# f26 8d4f7059a87368bf
i13f28:	.long	0x84e8e94b, 0x226ff525		# f28 84e8e94b226ff525
i13f29:	.long	0x82f8a161, 0xfa5f6e2f		# f29 82f8a161fa5f6e2f
i13f31:	.long	0xecc73384, 0x3799c397		# f31 ecc733843799c397
i13cr:	.long	0x6d4ff9b5			# cr 6d4ff9b5
i13fp:	.long	0xffffffff, 0x00025081			# fpscr 00025081
o13cr:	.long	0x694ff9b5			# cr 694ff9b5
o13fp:	.long	0x9e024081			# fpscr 9e024081
o13f1:	.long	0x7fefffff, 0xffffffff		# f1 7fefffffffffffff
o13f2:	.long	0x08b87969, 0xbd56951e		# f2 08b87969bd56951e
o13f3:	.long	0xbfd83a96, 0x4cd5fa95		# f3 bfd83a964cd5fa95
o13f4:	.long	0xfac52b6d, 0xef12dc97		# f4 fac52b6def12dc97
o13f5:	.long	0x4508e773, 0x45541d1b		# f5 4508e77345541d1b
o13f6:	.long	0xac22119f, 0xb5ba7228		# f6 ac22119fb5ba7228
o13f7:	.long	0xffefffff, 0xffffffff		# f7 ffefffffffffffff
o13f8:	.long	0xfff948e9, 0xf7097711		# f8 fff948e9f7097711
o13f9:	.long	0x80842456, 0xf9a7a651		# f9 80842456f9a7a651
o13f10:	.long	0xb5a6235a, 0x2340c60d		# f10 b5a6235a2340c60d
o13f11:	.long	0x00000000, 0x00000000		# f11 0000000000000000
o13f12:	.long	0x1e52d833, 0xa9f47965		# f12 1e52d833a9f47965
o13f13:	.long	0x00000000, 0x00000000		# f13 0000000000000000
o13f14:	.long	0xfff948e9, 0xf7097711		# f14 fff948e9f7097711
o13f15:	.long	0x1fe7264c, 0x1a7f7419		# f15 1fe7264c1a7f7419
o13f16:	.long	0xfff00000, 0x00000000		# f16 fff0000000000000
o13f17:	.long	0xd2879012, 0x689998d5		# f17 d2879012689998d5
o13f18:	.long	0x80000000, 0x00fea1c3		# f18 8000000000fea1c3
o13f19:	.long	0x00000000, 0x00000000		# f19 0000000000000000
o13f20:	.long	0x80000000, 0x00000000		# f20 8000000000000000
o13f21:	.long	0xffefffff, 0xffffffff		# f21 ffefffffffffffff
o13f22:	.long	0xbabf21e2, 0x14cb546c		# f22 babf21e214cb546c
o13f23:	.long	0x00000000, 0x00000000		# f23 0000000000000000
o13f24:	.long	0xc952d943, 0xc61fdf7c		# f24 c952d943c61fdf7c
o13f25:	.long	0xfff948e9, 0xf7097711		# f25 fff948e9f7097711
o13f27:	.long	0x7fefffff, 0xffffffff		# f27 7fefffffffffffff
o13f28:	.long	0xfff00000, 0x00000000		# f28 fff0000000000000
o13f30:	.long	0x4453c9e1, 0x11ba9545		# f30 4453c9e111ba9545
o13f31:	.long	0x7b89df9d, 0x5fc71446		# f31 7b89df9d5fc71446
#
	.toc
	TOCE(.sys, entry)
	TOCL(_sys, data)
