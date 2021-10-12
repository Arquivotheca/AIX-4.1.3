# @(#)25	1.3  src/bos/diag/da/fpa/mtfsf.s, dafpa, bos411, 9428A410j 3/23/94 06:29:28

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
#		will be compared with expected results. If any test mtfsfils,
#		a particular return code will be returned to the caller.
#
	.csect	.mtfsf[PR]
	.globl	.mtfsf[PR]
	.align	2
.mtfsf:
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
	LTOC(4, _mtfsf, data)		# Loads g4 with the address of _mtfsf
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
 	.using	_mtfsf[RW],g4		# g4 is the base register to the data
#	 				     section _mtfsf[rw]. Eastablishing 
#					     @ability
#
#	
	xor	g3,g3,g3		# g3 = 0. Initialize return code
#
#				# Loading floating pt registers for computation
#
#	First instruction : -
#
	lfd	rf0,i1fp
	mtfsf	0xff,rf0
#
	lfd	rf0,i1
	mtfsf	0x79,rf0
#
	ai	g3,g3,1			
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o1fp+4
	cmp	0,g6,g7
	bne	0,FINI	
#
#	Second instruction : -
#
	lfd	rf0,i2fp
	mtfsf	0xff,rf0
#
	lfd	rf0,i2
	mtfsf	0x93,rf0
#
	ai	g3,g3,1			
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o2fp+4
	cmp	0,g6,g7
	bne	0,FINI	
#
#	Third instruction : -
#
	lfd	rf0,i3fp
	mtfsf	0xff,rf0
#
	lfd	rf0,i3
	mtfsf	0x09,rf0
#
	ai	g3,g3,1			
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o3fp+4
	cmp	0,g6,g7
	bne	0,FINI	

#
#	Fourth instruction : -
#
	lfd	rf0,i4fp
	mtfsf	0xff,rf0
#
	l	g5,i4cr
	mtcrf	0xff,g5
#
	lfd	rf0,i4
	mtfsf.	0x81,rf0
#
	ai	g3,g3,1
	mfcr	g6
	l	g7,o4cr
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1			
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o4fp+4
	cmp	0,g6,g7
	bne	0,FINI	
#
#	Fifth instruction : -
#
	lfd	rf0,i5fp
	mtfsf	0xff,rf0
#
	l	g5,i5cr
	mtcrf	0xff,g5
#
	lfd	rf0,i5
	mtfsf.	0xde,rf0
#
	ai	g3,g3,1
	mfcr	g6
	l	g7,o5cr
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1			
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o5fp+4
	cmp	0,g6,g7
	bne	0,FINI	
#
#	Sixth instruction : -
#
	lfd	rf0,i6fp
	mtfsf	0xff,rf0
#
	l	g5,i6cr
	mtcrf	0xff,g5
#
	lfd	rf0,i6
	mtfsf.	0xe2,rf0
#
	ai	g3,g3,1
	mfcr	g6
	l	g7,o6cr
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1			
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o6fp+4
	cmp	0,g6,g7
	bne	0,FINI	
#
#	Seventh instruction : -
#
	lfd	rf0,i7fp
	mtfsf	0xff,rf0
#
	l	g5,i7cr
	mtcrf	0xff,g5
#
	lfd	rf0,i7
	mtfsf.	0xa5,rf0
#
	ai	g3,g3,1
	mfcr	g6
	l	g7,o7cr
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1			
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,o7fp+4
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
	.csect	_mtfsf[RW],3
_mtfsf: 
#						# mtfsf   b'0011110011',r3
i1:	.long	0x18896066, 0xffffffff		# f3 18896066ffffffff
i1fp:	.long	0xffffffff, 0x00000002		# fpscr 00000002
o1fp:	.long	0xffffffff, 0x6ffff00b		# fpscr 6ffff00b
#
#						# mtfsf   b'1100100110',r6
i2:	.long	0xef2ea821, 0xd987c813		# f6 ef2ea821d987c813
i2fp:	.long	0xffffffff, 0x00000059		# fpscr 00000059
o2fp:	.long	0xffffffff, 0x90070013		# fpscr 90070013
#
#						# mtfsf   b'0000010011',r12
i3:	.long	0x927b4b8e, 0xcbf6f5bb		# f12 927b4b8ecbf6f5bb
i3fp:	.long	0xffffffff, 0x00000020		# fpscr 00000020
o3fp:	.long	0xffffffff, 0x0000f02b		# fpscr 0000f02b
#
#						# mtfsf  b'0100000011',r4
i4:	.long	0xffffffff, 0x00000000		# f4 ffffffff00000000
i4cr:	.long	0x6ce70d5a			# cr 6ce70d5a
i4fp:	.long	0xffffffff, 0x00000091		# fpscr 00000091
o4cr:	.long	0x60e70d5a			# cr 60e70d5a
o4fp:	.long	0xffffffff, 0x00000090		# fpscr 00000090
#
#						# mtfsf  b'1110111100',r0
i5:	.long	0x00000000, 0x33819471		# f0 0000000033819471
i5cr:	.long	0xe2886d75			# cr e2886d75
i5fp:	.long	0xffffffff, 0x000000d1		# fpscr 000000d1
o5cr:	.long	0xe7886d75			# cr e7886d75
o5fp:	.long	0xffffffff, 0x73019071		# fpscr 73019071
#
#						# mtfsf  b'1111000101',r13
i6:	.long	0x5d9e61dd, 0xffffffff		# f13 5d9e61ddffffffff
i6cr:	.long	0x80813f64			# cr 80813f64
i6fp:	.long	0xffffffff, 0x0000002b		# fpscr 0000002b
o6cr:	.long	0x8f813f64			# cr 8f813f64
o6fp:	.long	0xffffffff, 0xfff000fb		# fpscr fff000fb
#
#						# mtfsf  b'0101001011',r18
i7:	.long	0x53dbd6db, 0x1b694015		# f18 53dbd6db1b694015
i7cr:	.long	0x2630a1b2			# cr 2630a1b2
i7fp:	.long	0xffffffff, 0x80000069		# fpscr 80000069
o7cr:	.long	0x2730a1b2			# cr 2730a1b2
o7fp:	.long	0xffffffff, 0x70600061		# fpscr 70600061
#
#
fpsave:	.long	0xffffffff, 0xffffffff
#
	.toc
	TOCE(.mtfsf, entry)
	TOCL(_mtfsf, data)
