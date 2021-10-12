# @(#)42	1.3  src/bos/diag/da/fpa/fma.s, dafpa, bos411, 9428A410j 3/23/94 06:25:54

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
	.csect	.fma[PR]
	.globl	.fma[PR]
	.align	2
.fma:
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
	LTOC(4, _fma, data)		# Loads g4 with the address of _fma
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
 	.using	_fma[RW],g4		# g4 is the base register to the data
#	 				     section _fma[rw]. Eastablishing 
#					     @ability
#
#	
	xor	g3,g3,g3		# g3 = 0. Initialize return code
#
#				# Loading floating pt registers for computation
#
	st	g4,fpsave
	l	g5,fpsave		# g5 gets pointer to TOC
#
	xor	g9,g9,g9		# g9 = 0
	ai	g9,g9,4
#
fstart:
	lfd	rf6,0x0018(g5)		# Tricky! Does not load from MSR
	mtfsf	0xff,rf6		# load fpscr (input)
#
	lfd	rf0,0x0020(g5)		# preload result to avoid NaN op.
#
	lfd	rf1,0x0000(g5)
	lfd	rf2,0x0010(g5)
	lfd	rf3,0x0008(g5)
#
#  Instructions used for testing
# 
	fma	r0,r1,r2,r3
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
#
	ai	g3,g3,1			
	mffs	rf7
	stfd	rf7,fpsave
	l	g6,fpsave+4
	l	g7,0x002c(g5)
	cmp	0,g6,g7
	bne	0,FINI	
#
	ai	g3,g3,1			
	stfd	rf0,fpsave
	l	g6,fpsave
	l	g7,0x0020(g5)
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1			
	l	g6,fpsave+4
	l	g7,0x0024(g5)
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g5,g5,0x0030		# g5 now points to data of new
	ai	g9,g9,-1		# g9 -= 1
	cmpi	1,g9,0
	bne	1,fstart
#
	xor	g3,g3,g3		# Return code = 0; pass
#
FINI:	br
#
# Data Area
#
	.csect	_fma[RW],3
_fma: 
#
	.long	0x00100000, 0x0000003f		# f22 001000000000003f
	.long	0x00000000, 0x00000000		# f3 0000000000000000
	.long	0x00000000, 0x00000000		# f3 0000000000000000
	.long	0x00007000			# msr 00007000
	.long	0x000000c9			# fpscr 000000c9
	.long	0x00000000, 0x00000000		# f20 0000000000000000
	.long	0x00007000			# msr 00007000
	.long	0x000020c9			# fpscr 000020c9
#
	.long	0x7ff45486, 0x255a69c3		# f14 7ff45486255a69c3
	.long	0x9d67e8fe, 0xc37d9ab3		# f3 9d67e8fec37d9ab3
	.long	0xfff00000, 0x00000000		# f23 fff0000000000000
	.long	0x00006000			# msr 00006000
	.long	0x00000041			# fpscr 00000041
	.long	0x7ffc5486, 0x255a69c3		# f3 7ffc5486255a69c3
	.long	0x00006000			# msr 00006000
	.long	0xa1011041			# fpscr a1011041
#
	.long	0x7ff479a7, 0xebf9689d		# f9 7ff479a7ebf9689d
	.long	0x4ba0b024, 0xbb73156b		# f17 4ba0b024bb73156b
	.long	0x7ff71ced, 0x9bf86794		# f13 7ff71ced9bf86794
	.long	0x00007000			# msr 00007000
	.long	0x000000e8			# fpscr 000000e8
	.long	0x645be66d, 0x8696d46e		# f14 645be66d8696d46e
	.long	0x00007000			# msr 00007000
	.long	0xe10000e8			# fpscr e10000e8
#
	.long	0x1ff00001, 0xffffffff		# f20 1ff00001ffffffff
	.long	0x00000000, 0x0003ffff		# f21 000000000003ffff
	.long	0x00000000, 0x0003ffff		# f21 000000000003ffff
	.long	0x00002080			# msr 00002080
	.long	0x00060011			# fpscr 00060011
	.long	0x00000000, 0x0003ffff		# f3 000000000003ffff
	.long	0x00002080			# msr 00002080
	.long	0x8a034011			# fpscr 8a034011
#
fpsave:	.long	0xffffffff, 0xffffffff
#
	.toc
	TOCE(.fma, entry)
	TOCL(_fma, data)
