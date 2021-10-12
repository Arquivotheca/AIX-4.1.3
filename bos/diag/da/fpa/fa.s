# @(#)27	1.3  src/bos/diag/da/fpa/fa.s, dafpa, bos411, 9428A410j 3/23/94 06:23:12

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
	.csect	.fa[PR]
	.globl	.fa[PR]
	.align	2
.fa:
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
	LTOC(4, _fa, data)		# Loads g4 with the address of _fa
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
 	.using	_fa[RW],g4		# g4 is the base register to the data
#	 				     section _fa[rw]. Eastablishing 
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
	ai	g9,g9,27
#
fstart:
	lfd	rf6,0x0010(g5)		# Tricky! Does not load from CR
	mtfsf	0xff,rf6		# load fpscr (input)
#
	lfd	rf0,0x0018(g5)		# preload result in rf0, to avoid
					#   instructions involving NaNs
	lfd	rf1,0x0000(g5)
	lfd	rf2,0x0008(g5)
#
#  Instructions used for testing
# 
	fa	r0,r1,r2
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
	l	g7,0x0024(g5)
	cmp	0,g6,g7
	bne	0,FINI	
#
	ai	g3,g3,1			
	stfd	rf0,fpsave
	l	g6,fpsave
	l	g7,0x0018(g5)
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1			
	l	g6,fpsave+4
	l	g7,0x001c(g5)
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g5,g5,0x0028		# g5 now points to data of new
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
	.csect	_fa[RW],3
_fa: 
#	.long	0xfff2c7ef, 0x80436d2d		# f23 fff2c7ef80436d2d
#	.long	0xfff255c8, 0x871efd0b		# f29 fff255c8871efd0b
#	.long	0x00006080			# msr 00006080
#	.long	0x6a18001a			# fpscr 6a18001a
#	.long	0xfffa55c8, 0x871efd0b		# f13 fffa55c8871efd0b/fffac7ef
#	.long	0x00006080			# msr 00006080
#	.long	0xeb19101a			# fpscr eb19101a
#
#	.long	0xfff2c7ef, 0x80436d2d		# f23 fff2c7ef80436d2d
#	.long	0xfff255c8, 0x871efd0b		# f29 fff255c8871efd0b
#	.long	0x00006080			# msr 00006080
#	.long	0x6b18001a			# fpscr 6b18001a
#	.long	0xfffa55c8, 0x871efd0b		# f13 fffa55c8871efd0b/fffac7ef
#	.long	0x00006080			# msr 00006080
#	.long	0x6b19101a			# fpscr 6b19101a
#
	.long	0x7c940000, 0x00000000		# f15 7c94000000000000
	.long	0x7fefffff, 0xffffffff		# f25 7fefffffffffffff
	.long	0x00002080			# msr 00002080
	.long	0x77be00e8			# fpscr 77be00e8
	.long	0x1ff00000, 0x00000000		# f25 1ff0000000000000
	.long	0x00002080			# msr 00002080
	.long	0x77be40e8			# fpscr 77be40e8
#
	.long	0xfff7d887, 0xfe2a313c		# f3 fff7d887fe2a313c
	.long	0x7e7309c4, 0x3518b075		# f13 7e7309c43518b075
	.long	0x0000b000			# msr 0000b000
	.long	0x00000060			# fpscr 00000060
	.long	0xffffd887, 0xfe2a313c		# f13 ffffd887fe2a313c
	.long	0x0000b000			# msr 0000b000
	.long	0xa1011060			# fpscr a1011060
#
	.long	0x7ff00000, 0x00000000		# f8 7ff0000000000000
	.long	0xa24593c3, 0x0df534e2		# f18 a24593c30df534e2
	.long	0x0000b080			# msr 0000b080
	.long	0x000000c2			# fpscr 000000c2
	.long	0x7ff00000, 0x00000000		# f1 7ff0000000000000
	.long	0x0000b080			# msr 0000b080
	.long	0x000050c2			# fpscr 000050c2
#
	.long	0x7ff00000, 0x00000000		# f16 7ff0000000000000
	.long	0xfff00000, 0x00000000		# f18 fff0000000000000
	.long	0x0000f080			# msr 0000f080
	.long	0x00000050			# fpscr 00000050
	.long	0x7ff80000, 0x00000000		# f24 7ff8000000000000
	.long	0x0000f080			# msr 0000f080
	.long	0xa0811050			# fpscr a0811050
#
	.long	0x00338117, 0xe23a79f6		# f9 00338117e23a79f6
	.long	0x0001acb5, 0xb351bb44		# f15 0001acb5b351bb44
	.long	0x0000b080			# msr 0000b080
	.long	0x00000081			# fpscr 00000081
	.long	0x0033ec45, 0x4f0ee8c7		# f19 0033ec454f0ee8c7
	.long	0x0000b080			# msr 0000b080
	.long	0x00004081			# fpscr 00004081
#
	.long	0x800a1ac5, 0xac4dc177		# f6 800a1ac5ac4dc177
	.long	0x0009027c, 0x1a2907cd		# f13 0009027c1a2907cd
	.long	0x0000e000			# msr 0000e000
	.long	0x0000009b			# fpscr 0000009b
	.long	0x80011849, 0x9224b9aa		# f21 800118499224b9aa
	.long	0x0000e000			# msr 0000e000
	.long	0x0001809b			# fpscr 0001809b
#
	.long	0x00000000, 0x00000000		# f10 0000000000000000
	.long	0x00000000, 0x00000000		# f27 0000000000000000
	.long	0x00007080			# msr 00007080
	.long	0x000000a1			# fpscr 000000a1
	.long	0x00000000, 0x00000000		# f18 0000000000000000
	.long	0x00007080			# msr 00007080
	.long	0x000020a1			# fpscr 000020a1
#
	.long	0x80000000, 0x00000001		# f19 8000000000000001
	.long	0xffefffff, 0xffffffff		# f25 ffefffffffffffff
	.long	0x0000f000			# msr 0000f000
	.long	0x0000002b			# fpscr 0000002b
	.long	0xfff00000, 0x00000000		# f2 fff0000000000000
	.long	0x0000f000			# msr 0000f000
	.long	0xd206902b			# fpscr d206902b
#
	.long	0xd4ab8d88, 0x93da335d		# f2 d4ab8d8893da335d
	.long	0x8000b2f5, 0x2608ceba		# f31 8000b2f52608ceba
	.long	0x00002000			# msr 00002000
	.long	0x0000002b			# fpscr 0000002b
	.long	0xd4ab8d88, 0x93da335e		# f2 d4ab8d8893da335e
	.long	0x00002000			# msr 00002000
	.long	0xc206802b			# fpscr c206802b
#
	.long	0x810ffff0, 0x00000000		# f1 810ffff000000000
	.long	0x010fffff, 0xfffffffe		# f29 010ffffffffffffe
	.long	0x0000b000			# msr 0000b000
	.long	0x000000b1			# fpscr 000000b1
	.long	0x5fffffff, 0xfffc0000		# f16 5ffffffffffc0000
	.long	0x0000b000			# msr 0000b000
	.long	0xc80040b1			# fpscr c80040b1
#
	.long	0x3c27d5e5, 0xbf6b88db		# f6 3c27d5e5bf6b88db
	.long	0xbc27d5e5, 0xbf6b88db		# f22 bc27d5e5bf6b88db
	.long	0x00006000			# msr 00006000
	.long	0x000000aa			# fpscr 000000aa
	.long	0x00000000, 0x00000000		# f5 0000000000000000
	.long	0x00006000			# msr 00006000
	.long	0x000020aa			# fpscr 000020aa
#
	.long	0x2763b8f5, 0x12ce18bf		# f17 2763b8f512ce18bf
	.long	0xa740b162, 0xfedc13c1		# f19 a740b162fedc13c1
	.long	0x00006080			# msr 00006080
	.long	0x00000041			# fpscr 00000041
	.long	0x275f1938, 0xa62e279d		# f20 275f1938a62e279d
	.long	0x00006080			# msr 00006080
	.long	0x82024041			# fpscr 82024041
#
	.long	0x7fefffff, 0xffffffff		# f20 7fefffffffffffff
	.long	0x7c940000, 0x00000000		# f29 7c94000000000000
	.long	0x0000a000			# msr 0000a000
	.long	0x000000c8			# fpscr 000000c8
	.long	0x1ff00000, 0x00000000		# f25 1ff0000000000000
	.long	0x0000a000			# msr 0000a000
	.long	0xd20640c8			# fpscr d20640c8
#
	.long	0xffefffff, 0xffffff00		# f14 ffefffffffffff00
	.long	0xff6fffff, 0xffffffff		# f20 ff6fffffffffffff
	.long	0x00007000			# msr 00007000
	.long	0x00000049			# fpscr 00000049
	.long	0x9ff00fff, 0xffffff7f		# f7 9ff00fffffffff7f
	.long	0x00007000			# msr 00007000
	.long	0xd2028049			# fpscr d2028049
#
	.long	0xffefffff, 0xffffffff		# f9 ffefffffffffffff
	.long	0x80000000, 0x00000001		# f29 8000000000000001
	.long	0x0000e000			# msr 0000e000
	.long	0x000000fb			# fpscr 000000fb
	.long	0x9ff00000, 0x00000000		# f24 9ff0000000000000
	.long	0x0000e000			# msr 0000e000
	.long	0xd20680fb			# fpscr d20680fb
#
	.long	0x7fefffff, 0xffffffff		# f13 7fefffffffffffff
	.long	0x00000000, 0x00000001		# f19 0000000000000001
	.long	0x00002080			# msr 00002080
	.long	0x0000008a			# fpscr 0000008a
	.long	0x7ff00000, 0x00000000		# f25 7ff0000000000000
	.long	0x00002080			# msr 00002080
	.long	0xd206508a			# fpscr d206508a
#
	.long	0xfca00000, 0x00000000		# f0 fca0000000000000
	.long	0xffefffff, 0xffffffff		# f24 ffefffffffffffff
	.long	0x0000f000			# msr 0000f000
	.long	0x00000032			# fpscr 00000032
	.long	0xffefffff, 0xffffffff		# f19 ffefffffffffffff
	.long	0x0000f000			# msr 0000f000
	.long	0x92028032			# fpscr 92028032
#
	.long	0x7ca40000, 0x00000000		# f8 7ca4000000000000
	.long	0x7fefffff, 0xffffffff		# f31 7fefffffffffffff
	.long	0x0000b000			# msr 0000b000
	.long	0x000000b9			# fpscr 000000b9
	.long	0x7fefffff, 0xffffffff		# f13 7fefffffffffffff
	.long	0x0000b000			# msr 0000b000
	.long	0xd20240b9			# fpscr d20240b9
#
	.long	0xfffaaf52, 0x09c928e4		# f4 fffaaf5209c928e4
	.long	0x7ff00000, 0x00000000		# f17 7ff0000000000000
	.long	0x0000f000			# msr 0000f000
	.long	0x0000007b			# fpscr 0000007b
	.long	0xfffaaf52, 0x09c928e4		# f28 fffaaf5209c928e4
	.long	0x0000f000			# msr 0000f000
	.long	0x0001107b			# fpscr 0001107b
#
	.long	0x38da8d18, 0x20f23635		# f9 38da8d1820f23635
	.long	0x38dfcab5, 0x8e5861e7		# f30 38dfcab58e5861e7
	.long	0x00003000			# msr 00003000
	.long	0x00000082			# fpscr 00000082
	.long	0x38ed2be6, 0xd7a54c0e		# f29 38ed2be6d7a54c0e
	.long	0x00003000			# msr 00003000
	.long	0x00004082			# fpscr 00004082
#
	.long	0x003e0000, 0x00000000		# f13 003e000000000000
	.long	0x803fffff, 0xff000000		# f15 803fffffff000000
	.long	0x00007000			# msr 00007000
	.long	0x000000b3			# fpscr 000000b3
	.long	0xdfffffff, 0xf0000000		# f12 dffffffff0000000
	.long	0x00007000			# msr 00007000
	.long	0xc80080b3			# fpscr c80080b3
#
	.long	0x80000000, 0x00000001		# f12 8000000000000001
	.long	0xffefffff, 0xffffffff		# f18 ffefffffffffffff
	.long	0x00002000			# msr 00002000
	.long	0x0002007b			# fpscr 0002007b
	.long	0x9ff00000, 0x00000000		# f26 9ff0000000000000
	.long	0x00002000			# msr 00002000
	.long	0xd206807b			# fpscr d206807b
#
	.long	0x7c940000, 0x00000000		# f15 7c94000000000000
	.long	0x7fefffff, 0xffffffff		# f25 7fefffffffffffff
	.long	0x00002080			# msr 00002080
	.long	0x67be00e8			# fpscr 67be00e8
	.long	0x1ff00000, 0x00000000		# f25 1ff0000000000000
	.long	0x00002080			# msr 00002080
	.long	0xf7be40e8			# fpscr f7be40e8
#
	.long	0x7ff00000, 0x00000000		# f6 7ff0000000000000
	.long	0xfff00000, 0x00000000		# f9 fff0000000000000
	.long	0x0000f080			# msr 0000f080
	.long	0x6d0a00b0			# fpscr 6d0a00b0
	.long	0x398f8d0c, 0xa8fe9308		# f16 398f8d0ca8fe9308
	.long	0x0000f080			# msr 0000f080
	.long	0xed8800b0			# fpscr ed8800b0
#
	.long	0x4d2050a5, 0xa4cec7f3		# f14 4d2050a5a4cec7f3
	.long	0xcd453fa7, 0x9f6c18aa		# f22 cd453fa79f6c18aa
	.long	0x00006000			# msr 00006000
	.long	0x8002006b			# fpscr 8002006b
	.long	0xcd412b7e, 0x363866ae		# f28 cd412b7e363866ae
	.long	0x00006000			# msr 00006000
	.long	0xc206806b			# fpscr c206806b
#
	.long	0x7ff00000, 0x00000000		# f6 7ff0000000000000
	.long	0xfff00000, 0x00000000		# f9 fff0000000000000
	.long	0x0000f080			# msr 0000f080
	.long	0x6d8a00b0			# fpscr 6d8a00b0
	.long	0x398f8d0c, 0xa8fe9308		# f16 398f8d0ca8fe9308
	.long	0x0000f080			# msr 0000f080
	.long	0x6d8800b0			# fpscr 6d8800b0
#
	.long	0x4d2050a5, 0xa4cec7f3		# f14 4d2050a5a4cec7f3
	.long	0xcd453fa7, 0x9f6c18aa		# f22 cd453fa79f6c18aa
	.long	0x00006000			# msr 00006000
	.long	0x0202006b			# fpscr 0202006b
	.long	0xcd412b7e, 0x363866ae		# f28 cd412b7e363866ae
	.long	0x00006000			# msr 00006000
	.long	0x4206806b			# fpscr 4206806b
#
fpsave:	.long	0xffffffff, 0xffffffff
#
	.toc
	TOCE(.fa, entry)
	TOCL(_fa, data)
