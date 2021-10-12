# @(#)43	1.3  src/bos/diag/da/fpa/lzafull.s, dafpa, bos411, 9428A410j 3/23/94 06:26:50

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
#	First module to test the "leading zero anticipation" functionality.
#		Appropriate registers are loaded with input values. Then
#		instructions for this module will be executed and results
#		will be compared with expected results. 
#
	.csect	.lzafull[PR]
	.globl	.lzafull[PR]
	.align	2
.lzafull:
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
	LTOC(4, _lzafull, data)		# Loads g4 with the address of _lzafull
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
 	.using	_lzafull[RW],g4		# g4 is the base register to the data
#	 				     section _lzafull[rw]. Eastablishing
#					     @ability
#
#	
	xor	g3,g3,g3		# g3 = 0. Initialize return code
	ai	g3,g3,1			# Preset return code = 1; fail
#
#				# Loading floating pt registers for computation
#
	lfd	rf0,z1if0
	lfd	rf10,z1if10
	lfd	rf13,z1if13
	lfd	rf17,z1if17
	lfd	rf20,z1if20
	lfd	rf21,z1if21
	lfd	rf23,z1if23
	lfd	rf24,z1if24
	lfd	rf29,z1if29
#
	lfd	rf1,z1fps
	mtfsf	0xff,rf1
	l	g6,z1icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
	fma.    rf11,rf17,rf20,rf10	# r11 = r17*r20 + r10         
	fma.    rf8,rf29,rf21,rf20     	# r8  = r29*r21 + r20       
	fa      rf29,rf0,rf23          	# r29 = r0 + r23      
	fms     rf13,rf21,rf24,rf13    	# r13 = r21*r24 + r13    
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	lfd	rf1,z1of8		# Load expected result of rf8 in rf1
	fcmpu	3,rf1,rf8		# Compare results
	bne	3,FINI			# Exit if not equal
#
	ai	g3,g3,1			# RC = 2
	lfd	rf1,z1of11		# Check rf11
	fcmpu	3,rf1,rf11
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 3
	lfd	rf1,z1of13		# Check rf13
	fcmpu	3,rf1,rf13
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 4
	lfd	rf1,z1of29		# Check rf29
	fcmpu	3,rf1,rf29
	bne	3,FINI
#
#	Second module to test the "leading zero anticipation" functionality.
#		Appropriate registers are loaded with input values. Then
#		instructions for this module will be executed and results
#		will be compared with expected results. If any test fails,
#		a particular return code will be returned to the caller.
#
#
#				# Loading floating pt registers for computation
#
	lfd	rf0,z2if0
	lfd	rf4,z2if4
	lfd	rf8,z2if8
	lfd	rf10,z2if10
	lfd	rf17,z2if17
	lfd	rf20,z2if20
	lfd	rf22,z2if22
	lfd	rf26,z2if26
	lfd	rf27,z2if27
#
	lfd	rf1,z2fps
	mtfsf	0xff,rf1
#
#  Instructions used for testing
# 
	fma     rf16,rf10,rf10,rf20	# r16 = r10*r10 + r20
	fms     rf21,rf22,rf26,rf4	# r21 = r22*r26 - r4
	fnms    rf11,rf0,rf17,rf27	# r11 = -( r0*r17 - r27 )
	fs      rf7,rf0,rf8		# r7 = r0 - r8
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1			# RC = 5 
	lfd	rf1,z2of7		# Load expected result of rf7 in rf1
	fcmpu	3,rf1,rf7		# Compare results
	bne	3,FINI			# Exit if not equal
#
	ai	g3,g3,1			# RC = 6
	lfd	rf1,z2of11		# Check rf11
	fcmpu	3,rf1,rf11
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 7
	lfd	rf1,z2of16		# Check rf16
	fcmpu	3,rf1,rf16
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 8
	lfd	rf1,z2of21		# Check rf21
	fcmpu	3,rf1,rf21
	bne	3,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf0,z3if0
	lfd	rf5,z3if5
	lfd	rf8,z3if8
	lfd	rf10,z3if10
	lfd	rf13,z3if13
	lfd	rf15,z3if15
	lfd	rf16,z3if16
	lfd	rf17,z3if17
	lfd	rf27,z3if27
	lfd	rf30,z3if30
#
	lfd	rf1,z3fps
	mtfsf	0xff,rf1
	l	g6,z3icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
	fnms.   rf22,rf8,rf0,rf15	# r22 = -(r8*r0 - r15) 
	fma     rf31,rf5,rf30,rf13	# r31 = r5*r30 + r13
	fs.     rf16,rf17,rf27		# r16 = r17 - r27
	fms     rf10,rf0,rf31,rf10	# r10 = r0*r31 - r10
#
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1			# RC = 9 
	lfd	rf1,z3of10		# Load expected result of rf10 in rf1
	fcmpu	0,rf1,rf10		# Compare results
	bne	FINI			# Exit if not equal
#
	ai	g3,g3,1			# RC = 10
	lfd	rf1,z3of16		# Check rf16
	fcmpu	0,rf1,rf16
	bne	FINI
#
	ai	g3,g3,1			# RC = 11
	lfd	rf1,z3of22		# Check rf22
	fcmpu	0,rf1,rf22
	bne	FINI
#
	ai	g3,g3,1			# RC = 12
	lfd	rf1,z3of31		# Check rf31
	fcmpu	0,rf1,rf31
	bne	FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf2,z4if2
	lfd	rf6,z4if6
	lfd	rf13,z4if13
	lfd	rf14,z4if14
	lfd	rf20,z4if20
	lfd	rf23,z4if23
	lfd	rf25,z4if25
	lfd	rf26,z4if26
#
	lfd	rf1,z4fps
	mtfsf	0xff,rf1
	l	g6,z4icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
	fnms.   rf0,rf6,rf14,rf26	# r0  = -(r6*r14 - r26)
	fnma    rf24,rf13,rf14,rf20  	# r24 = -(r13*r14 - r20) 
	fms.    rf30,rf2,rf23,rf2	# r30 = r2*r23 - r2 
	fd.     rf15,rf25,rf0		# r15 = r25/r0
#
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1			# RC = 13 
	lfd	rf1,z4of0		# Load expected result of rf0 in rf1
	fcmpu	3,rf1,rf0		# Compare results
	bne	3,FINI			# Exit if not equal
#
	ai	g3,g3,1			# RC = 14
	lfd	rf1,z4of15		# Check rf15
	fcmpu	3,rf1,rf15
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 15
	lfd	rf1,z4of24		# Check rf24
	fcmpu	3,rf1,rf24
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 16
	lfd	rf1,z4of30		# Check 30
	fcmpu	3,rf1,rf30
	bne	3,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf0,z5if0
	lfd	rf8,z5if8
	lfd	rf9,z5if9
	lfd	rf12,z5if12
	lfd	rf17,z5if17
	lfd	rf20,z5if20
	lfd	rf30,z5if30
#
	lfd	rf1,z5fps
	mtfsf	0xff,rf1
	l	g6,z5icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
	fms.    rf25,rf20,rf0,rf20	# r25 = r20*r0 - r20
	fa.     rf0,rf0,rf17		# r0  = r0 + r17
	fnms.   rf28,rf0,rf9,rf12	# r28 = -(r0*r9 - r12)
	fa      rf30,rf8,rf30		# r30 = r8 + r30
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1			# RC = 17 
	lfd	rf1,z5of0		# Load expected result of rf0 in rf1
	fcmpu	3,rf1,rf0		# Compare results
	bne	3,FINI			# Exit if not equal
#
	ai	g3,g3,1			# RC = 18
	lfd	rf1,z5of25		# Check rf25
	fcmpu	3,rf1,rf25
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 19
	lfd	rf1,z5of28		# Check rf28
	fcmpu	3,rf1,rf28
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 20
	lfd	rf1,z5of30		# Check rf30
	fcmpu	3,rf1,rf30
	bne	3,FINI
#
#
	xor	g3,g3,g3		# Return code = 0; pass
FINI:	br
#
	.csect	_lzafull[RW],3
_lzafull: 
z1fps:	.long	0xffffffff
	.long	0xebb9b018
z1icr:	.long	0xc2d2cc4c
z1if0:	.long	0x280af396		# Inputs
	.long	0x2f0ac31d
z1if10:	.long	0x000007ff
	.long	0xffffffff
z1if13:	.long	0xffefffff
	.long	0xffffffff
z1if17:	.long	0x9ff00000
	.long	0xffffffff
z1if20:	.long	0x9ff00000
	.long	0x0000000f
z1if21:	.long	0xc01000ff
	.long	0xffffffff
z1if23:	.long	0x00000000
	.long	0x00000000
z1if24:	.long	0xc0007fff
	.long	0xffffffff
z1if29:	.long	0x7fd00001
	.long	0xffffffff
#
# results   
#
z1of8:	.long	0xfff00000		# Outputs
	.long	0x00000000
z1of11:	.long	0x00040800
	.long	0x40000003
z1of13:	.long	0x7fefffff
	.long	0xffffffff
z1of29:	.long	0x280af396
	.long	0x2f0ac31d
# 
z2fps:	.long	0xffffffff	
	.long	0x000400d2
z2if0:	.long	0xd89ab588
	.long	0x9deda87b
z2if4:	.long	0x80000000
	.long	0xbdb51c2b
z2if8:	.long	0xfc800000
	.long	0x00000000
z2if10:	.long	0xc010003f
	.long	0xffffffff
z2if17:	.long	0xadc53bb6
	.long	0xe383ba5c
z2if20:	.long	0x7fefffff
	.long	0xffffffff
z2if22:	.long	0x9c2aa4b5
	.long	0xdc2f682c
z2if26:	.long	0x1c6f0df5
	.long	0xf1dd6188
z2if27:	.long	0x318d6052
	.long 	0xff21687b
#
# Results           
#
z2of7:	.long	0x7c800000
	.long	0x00000000
z2of11:	.long	0xc671b8f9
	.long	0x01e5dacb
z2of16:	.long	0x1ff00000
	.long	0x00000000
z2of21:	.long	0x00000000
	.long	0xbdb51c2b
#
z3fps:	.long	0xffffffff	
	.long	0xf50050d2    
z3icr:	.long	0x47b88cab
z3if0:	.long	0x392fa0c1
	.long	0x1ba1caea
z3if5:	.long	0x722a4e9d
	.long	0xe7685d9f
z3if8:	.long	0x43c45e35
	.long	0x4376320b
z3if10:	.long	0x7fefffff
	.long	0xffffc000
z3if13:	.long	0xdd26ad48
	.long	0x90ca3382
z3if15:	.long	0xdddda804
	.long	0x4bc9d51e
z3if16:	.long	0xf0202c51
	.long	0xcbeedee9
z3if17:	.long	0xb39c4d30
	.long	0x4ecb2271
z3if27:	.long	0xfff238b6
	.long	0x26dbb0d9
z3if30:	.long	0x2f7ffe91
	.long	0xd1e990c4
#
# results 
#
z3of10:	.long	0xffefffff
	.long	0xffffbfff
z3of16:	.long	0xf0202c51
	.long	0xcbeedee9
z3of22:	.long	0xdddda804
	.long	0x4bc9d51f
z3of31:	.long	0x61ba4d70
	.long	0xde56e26b
#
z4fps:	.long	0xffffffff
	.long	0x00000042
z4icr:	.long	0x1afc82be
z4if2:	.long	0x80000000
	.long	0x00000001
z4if6:	.long	0xd52408e1
	.long	0xcc03f383
z4if13:	.long	0xffd00000
	.long	0x0000ffff
z4if14:	.long	0xfe5f4501
	.long	0x97a595e8
z4if20:	.long	0xffefffff
	.long	0xffffff00
z4if23:	.long	0x1cd81a7c
	.long	0x1b677e84
z4if25:	.long	0xa0f87e30
	.long	0x24ea3003
z4if26:	.long	0xc0da1971
	.long	0x50a639a5
#
# results               
#
z4of0:	.long	0xb39393ce
	.long	0xe3bc5fee
z4of15:	.long	0x2d54046f
	.long	0x9f73c320
z4of24:	.long	0xde3f4501
	.long	0x97a78a37
z4of30:	.long	0x00000000
	.long	0x00000001
#
z5fps:	.long 	0xffffffff	
	.long	0x7aa470a3        
z5icr:	.long	0xe782d6d1
z5if0:	.long	0x9cb0c201
	.long	0xbde5a61f
z5if8:	.long	0x82b0b15e
	.long	0x863dbb2b
z5if9:	.long	0xf8d94ef1
	.long	0x205041e7
z5if12:	.long	0x88297929
	.long	0xa4ac5f7b
z5if17:	.long	0x7fefffff
	.long	0xffffffff
z5if20:	.long	0x00000000
	.long	0x00000001
z5if30:	.long	0x02b0b15e
	.long	0x863dbb2a
#
# results               
#
z5of0:	.long	0x7fefffff
	.long	0xfffffffe
z5of25:	.long	0xdcd00000
	.long	0x00000001
z5of28:	.long	0x7ff00000
	.long	0x00000000
z5of30:	.long	0xdf700000
	.long	0x00000000
#
#
	.toc
	TOCE(.lzafull, entry)
	TOCL(_lzafull, data)
