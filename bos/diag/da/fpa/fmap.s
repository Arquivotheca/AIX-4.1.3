# @(#)93	1.3  src/bos/diag/da/fpa/fmap.s, dafpa, bos411, 9428A410j 3/23/94 06:26:04

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
	.csect	.fmap[PR]
	.globl	.fmap[PR]
	.align	2
.fmap:
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
	LTOC(4, _fmap, data)		# Loads g4 with the address of _fmap
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
 	.using	_fmap[RW],g4		# g4 is the base register to the data
#	 				     section _fmap[rw]. Eastablishing 
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
	ai	g9,g9,5
#
fstart:
	l	g8,0x0018(g5)
	mtcrf	0xff,g8
#
	lfd	rf6,0x0018(g5)		# Tricky! Does not load from CR
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
	fma.	r0,r1,r2,r3
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
#
	ai	g3,g3,1
	l	g6,0x0028(g5)
	mfcr	g7
	cmp	0,g6,g7
	bne	0,FINI
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
	.csect	_fmap[RW],3
_fmap: 
##
#	.long	0x7ff00000, 0x00000000		# f1 7ff0000000000000
#	.long	0x00000000, 0x00000000		# f5 0000000000000000
#	.long	0x7ff00000, 0x00000000		# f26 7ff0000000000000
#	.long	0x014f491f			# cr 014f491f
#	.long	0x00000043			# fpscr 00000043
#	.long	0x7ff80000, 0x00000000		# f4 7ff8000000000000
#	.long	0x0a4f491f			# cr 0a4f491f
#	.long	0xa0111043			# fpscr a0111043
#
	.long	0xfff00000, 0x00000000		# f24 fff0000000000000
	.long	0x00000000, 0x00000000		# f17 0000000000000000
	.long	0x00000000, 0x00000000		# f26 0000000000000000
	.long	0x84d755db			# cr 84d755db
	.long	0x0000002b			# fpscr 0000002b
	.long	0x7ff80000, 0x00000000		# f3 7ff8000000000000
	.long	0x8ad755db			# cr 8ad755db
	.long	0xa011102b			# fpscr a011102b
##
#	.long	0x80007fff, 0xffffffff		# f7 80007fffffffffff
#	.long	0x92e00000, 0x0000000f		# f11 92e000000000000f
#	.long	0x17c00000, 0x3fffffff		# f14 17c000003fffffff
#	.long	0xa57e4488			# cr a57e4488
#	.long	0x00000008			# fpscr 00000008
#	.long	0x80007fff, 0xffffffff		# f28 80007fffffffffff
#	.long	0xac7e4488			# cr ac7e4488
#	.long	0xca038008			# fpscr ca038008/c2028008
##
#	.long	0x3f8e700f, 0xe78e4e15		# f3 3f8e700fe78e4e15
#	.long	0x3f8e700f, 0xe78e4e15		# f3 3f8e700fe78e4e15
#	.long	0x7ccc64fa, 0x8a17deea		# f18 7ccc64fa8a17deea
#	.long	0x0e5a46b1			# cr 0e5a46b1
#	.long	0x00000069			# fpscr 00000069
#	.long	0x7ccc64fa, 0x8a17deea		# f3 7ccc64fa8a17deea/7c6b021a
#	.long	0x0c5a46b1			# cr 0c5a46b1
#	.long	0xc2024069			# fpscr c2024069
##
#	.long	0x80000000, 0x00000000		# f7 8000000000000000
#	.long	0x00000000, 0x00000000		# f2 0000000000000000
#	.long	0x80000000, 0x00000000		# f28 8000000000000000
#	.long	0x97f5a89c			# cr 97f5a89c
#	.long	0x00000028			# fpscr 00000028
#	.long	0x80000000, 0x00000000		# f22 8000000000000000
#	.long	0x90f5a89c			# cr 90f5a89c
#	.long	0x00012028			# fpscr 00012028
#
	.long	0x7ff00000, 0x00000000		# f7 7ff0000000000000
	.long	0xfff00000, 0x00000000		# f24 fff0000000000000
	.long	0x7ff00000, 0x00000000		# f21 7ff0000000000000
	.long	0x70d4b022			# cr 70d4b022
	.long	0x00000090			# fpscr 00000090
	.long	0x21792b86, 0xa7f2c720		# f15 21792b86a7f2c720
	.long	0x7ed4b022			# cr 7ed4b022
	.long	0xe0800090			# fpscr e0800090
#
	.long	0x7ff00000, 0x00000000		# f3 7ff0000000000000
	.long	0x7ff00000, 0x00000000		# f25 7ff0000000000000
	.long	0x7ff00000, 0x00000000		# f4 7ff0000000000000
	.long	0x80b5297d			# cr 80b5297d
	.long	0x000000cb			# fpscr 000000cb
	.long	0x7ff00000, 0x00000000		# f21 7ff0000000000000
	.long	0x80b5297d			# cr 80b5297d
	.long	0x000050cb			# fpscr 000050cb
##
#	.long	0x6ab8625a, 0xdec4cc98		# f6 6ab8625adec4cc98
#	.long	0xed897655, 0xdaef4106		# f25 ed897655daef4106
#	.long	0x79bb4d56, 0xdb0b7835		# f26 79bb4d56db0b7835
#	.long	0x6e7e6de1			# cr 6e7e6de1
#	.long	0x00000042			# fpscr 00000042
#	.long	0xb8536703, 0x1934f074		# f8 b85367031934f074
#	.long	0x6d7e6de1			# cr 6d7e6de1
#	.long	0xd2028042			# fpscr d2028042
##
	.long	0x7ff86241, 0x37111197		# f2 7ff8624137111197
	.long	0x0cb5bce9, 0x395526fd		# f26 0cb5bce9395526fd
	.long	0x7ff00000, 0x00000000		# f5 7ff0000000000000
	.long	0x9773d19d			# cr 9773d19d
	.long	0x00000050			# fpscr 00000050
	.long	0x7ff86241, 0x37111197		# f13 7ff8624137111197
	.long	0x9073d19d			# cr 9073d19d
	.long	0x00011050			# fpscr 00011050
##
#	.long	0xfff00000, 0x00000000		# f13 fff0000000000000
#	.long	0xdef257c3, 0x8c75ce53		# f31 def257c38c75ce53
#	.long	0xfff00000, 0x00000000		# f14 fff0000000000000
#	.long	0xe292bd20			# cr e292bd20
#	.long	0x00000093			# fpscr 00000093
#	.long	0x72ee8ca3, 0xbff0adc4		# f28 72ee8ca3bff0adc4
#	.long	0xee92bd20			# cr ee92bd20
#	.long	0xe0800093			# fpscr e0800093
#
	.long	0x00000000, 0x00000000		# f19 0000000000000000
	.long	0x80000000, 0x00000000		# f24 8000000000000000
	.long	0x00000000, 0x00000000		# f28 0000000000000000
	.long	0x6e1c252d			# cr 6e1c252d
	.long	0x00000079			# fpscr 00000079
	.long	0x00000000, 0x00000000		# f6 0000000000000000
	.long	0x601c252d			# cr 601c252d
	.long	0x00002079			# fpscr 00002079
##
#	.long	0x80000000, 0x00000000		# f0 8000000000000000
#	.long	0x1c43bc1d, 0x0c2067ed		# f25 1c43bc1d0c2067ed
#	.long	0x1c43bc1d, 0x0c2067ed		# f25 1c43bc1d0c2067ed
#	.long	0x1870709c			# cr 1870709c
#	.long	0x000000a9			# fpscr 000000a9
#	.long	0x58985768, 0xa79eac5b		# f29 58985768a79eac5b
#	.long	0x1c70709c			# cr 1c70709c
#	.long	0xca0240a9			# fpscr ca0240a9
#
fpsave:	.long	0xffffffff, 0xffffffff
#
	.toc
	TOCE(.fmap, entry)
	TOCL(_fmap, data)
