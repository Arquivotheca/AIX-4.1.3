# @(#)06	1.3  src/bos/diag/da/fpa/fdd.s, dafpa, bos411, 9428A410j 3/23/94 06:25:05

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
	.csect	.fdd[PR]
	.globl	.fdd[PR]
	.align	2
.fdd:
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
	LTOC(4, _fdd, data)		# Loads g4 with the address of _fdd
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
 	.using	_fdd[RW],g4		# g4 is the base register to the data
#	 				     section _fdd[rw]. Eastablishing 
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
	ai	g9,g9,15
#
fstart:
	lfd	rf6,0x0010(g5)		# Tricky! Does not load from MSR
	mtfsf	0xff,rf6		# load fpscr (input)
#
	lfd	rf0,0x0018(g5)		# preload result in rf0, to avoid
					#   instructions involving NaNs
	lfd	rf1,0x0000(g5)
	lfd	rf2,0x0008(g5)
#
#  Instructions used for testing
# 
	fd	r0,r1,r2
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
#
##
##
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
	ai	g3,g3,1			
	l	g6,fpsave+4
	l	g7,0x001c(g5)
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g5,g5,0x0028		# g5 now points to data of new
	ai	g9,g9,-1		# g9 -= 1
	cmpi	0,g9,0
	bne	0,fstart
#
	xor	g3,g3,g3		# Return code = 0; pass
#
FINI:	br
#
# Data Area
#
	.csect	_fdd[RW],3
_fdd: 
	.long	0x21b777f8, 0xf15c91a5		# f19 21b777f8f15c91a5
	.long	0x80000000, 0x00000000		# f24 8000000000000000
	.long	0x0000f000			# msr 0000f000
	.long	0x00000022			# fpscr 00000022
	.long	0xfff00000, 0x00000000		# f27 fff0000000000000
	.long	0x0000f000			# msr 0000f000
	.long	0x84009022			# fpscr 84009022
#
	.long	0xfff00000, 0x00000000		# f30 fff0000000000000
	.long	0xfff00000, 0x00000000		# f30 fff0000000000000
	.long	0x00007000			# msr 00007000
	.long	0x00000022			# fpscr 00000022
	.long	0x7ff80000, 0x00000000		# f21 7ff8000000000000
	.long	0x00007000			# msr 00007000
	.long	0xa0411022			# fpscr a0411022
#
	.long	0x7ff00000, 0x00000000		# f26 7ff0000000000000
	.long	0x7ff00000, 0x00000000		# f11 7ff0000000000000
	.long	0x0000e080			# msr 0000e080
	.long	0x77b6006b			# fpscr 77b6006b
	.long	0x7ff80000, 0x00000000		# f14 7ff8000000000000
	.long	0x0000e080			# msr 0000e080
	.long	0xf7f1106b			# fpscr f7f1106b
#
	.long	0x7ff00000, 0x00000000		# f26 7ff0000000000000
	.long	0x7ff00000, 0x00000000		# f11 7ff0000000000000
	.long	0x0000e080			# msr 0000e080
	.long	0x77f6006b			# fpscr 77f6006b
	.long	0x7ff80000, 0x00000000		# f14 7ff8000000000000
	.long	0x0000e080			# msr 0000e080
	.long	0x77f1106b			# fpscr 77f1106b
#
	.long	0x00000000, 0x00000000		# f27 0000000000000000
	.long	0x7ff00000, 0x00000000		# f21 7ff0000000000000
	.long	0x0000b000			# msr 0000b000
	.long	0x00000081			# fpscr 00000081
	.long	0x00000000, 0x00000000		# f29 0000000000000000
	.long	0x0000b000			# msr 0000b000
	.long	0x00002081			# fpscr 00002081
#
	.long	0x7ffb54a1, 0x2284e424		# f1 7ffb54a12284e424
	.long	0x1637dca9, 0xb42b1f78		# f27 1637dca9b42b1f78
	.long	0x0000b000			# msr 0000b000
	.long	0x000000d8			# fpscr 000000d8
	.long	0x7ffb54a1, 0x2284e424		# f16 7ffb54a12284e424
	.long	0x0000b000			# msr 0000b000
	.long	0x000110d8			# fpscr 000110d8
#
	.long	0xfffccc36, 0xe9954e6e		# f6 fffccc36e9954e6e
	.long	0xfff4df1f, 0xc5ad2944		# f25 fff4df1fc5ad2944
	.long	0x0000a000			# msr 0000a000
	.long	0x00000023			# fpscr 00000023
	.long	0xfffccc36, 0xe9954e6e		# f4 fffccc36e9954e6e
	.long	0x0000a000			# msr 0000a000
	.long	0xa1011023			# fpscr a1011023
#
	.long	0xfff00000, 0x00000000		# f25 fff0000000000000
	.long	0xfff00000, 0x00000000		# f26 fff0000000000000
	.long	0x00002000			# msr 00002000
	.long	0x0000002b			# fpscr 0000002b
	.long	0x7ff80000, 0x00000000		# f30 7ff8000000000000
	.long	0x00002000			# msr 00002000
	.long	0xa041102b			# fpscr a041102b
#
	.long	0x5acc6059, 0x76b02605		# f23 5acc605976b02605
	.long	0x80000000, 0x00000000		# f26 8000000000000000
	.long	0x0000f080			# msr 0000f080
	.long	0x00000063			# fpscr 00000063
	.long	0xfff00000, 0x00000000		# f1 fff0000000000000
	.long	0x0000f080			# msr 0000f080
	.long	0x84009063			# fpscr 84009063
#
	.long	0x31553220, 0xe38477c2		# f23 31553220e38477c2
	.long	0x80000000, 0x00000000		# f2 8000000000000000
	.long	0x0000e080			# msr 0000e080
	.long	0x00000090			# fpscr 00000090
	.long	0x722fc06f, 0xe57c2766		# f13 722fc06fe57c2766
	.long	0x0000e080			# msr 0000e080
	.long	0xc4000090			# fpscr c4000090
#
	.long	0x7ff3fb3f, 0x39843c23		# f2 7ff3fb3f39843c23
	.long	0x6f725ad1, 0x211340c7		# f30 6f725ad1211340c7
	.long	0x00002000			# msr 00002000
	.long	0x0000006a			# fpscr 0000006a
	.long	0x7ffbfb3f, 0x39843c23		# f15 7ffbfb3f39843c23
	.long	0x00002000			# msr 00002000
	.long	0xa101106a			# fpscr a101106a
#
	.long	0x19902bf6, 0xe005ac17		# f16 19902bf6e005ac17
	.long	0x2a6dc1f7, 0x8ada6335		# f6 2a6dc1f78ada6335
	.long	0x0000a000			# msr 0000a000
	.long	0x00000042			# fpscr 00000042
	.long	0x2f1163eb, 0xce717a1b		# f14 2f1163ebce717a1b
	.long	0x0000a000			# msr 0000a000
	.long	0x82064042			# fpscr 82064042
#
	.long	0x29e34f2a, 0x8fc02b83		# f6 29e34f2a8fc02b83
	.long	0xffa3cf97, 0xa0199e89		# f19 ffa3cf97a0199e89
	.long	0x0000b000			# msr 0000b000
	.long	0x000000cb			# fpscr 000000cb
	.long	0x80000000, 0x00000001		# f10 8000000000000001
	.long	0x0000b000			# msr 0000b000
	.long	0xca0780cb			# fpscr ca0780cb
#
	.long	0x21f7c099, 0x5437b4e4		# f23 21f7c0995437b4e4
	.long	0xce40032f, 0xb3e86b0a		# f8 ce40032fb3e86b0a
	.long	0x0000e000			# msr 0000e000
	.long	0x00000060			# fpscr 00000060
	.long	0x93a7bbdf, 0x5798a164		# f28 93a7bbdf5798a164
	.long	0x0000e000			# msr 0000e000
	.long	0x82068060			# fpscr 82068060
#
	.long	0xe86e5982, 0xd1d89308		# f2 e86e5982d1d89308
	.long	0x9f5bed74, 0xd1982a41		# f25 9f5bed74d1982a41
	.long	0x0000e080			# msr 0000e080
	.long	0x0000009b			# fpscr 0000009b
	.long	0x7fefffff, 0xffffffff		# f23 7fefffffffffffff
	.long	0x0000e080			# msr 0000e080
	.long	0xd202409b			# fpscr d202409b
#
fpsave:	.long	0xffffffff, 0xffffffff
#
	.toc
	TOCE(.fdd, entry)
	TOCL(_fdd, data)
