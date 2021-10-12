# @(#)08	1.3  src/bos/diag/da/fpa/fmp.s, dafpa, bos411, 9428A410j 3/23/94 06:26:18

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
	.csect	.fmp[PR]
	.globl	.fmp[PR]
	.align	2
.fmp:
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
	LTOC(4, _fmp, data)		# Loads g4 with the address of _fmp
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
 	.using	_fmp[RW],g4		# g4 is the base register to the data
#	 				     section _fmp[rw]. Eastablishing 
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
	ai	g9,g9,14
#
fstart:
	l	g8,0x0014(g5)
	mtcrf	0xff,g8
#
	lfd	rf6,0x0014(g5)		# Tricky! Does not load from CR
	mtfsf	0xff,rf6		# load fpscr (input)
#
	lfd	rf0,0x001c(g5)		# preload result to avoid NaN op.
#
	lfd	rf1,0x0000(g5)
	lfd	rf2,0x0008(g5)
#
#  Instructions used for testing
# 
	fm.	r0,r1,r2
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
	l	g7,0x001c(g5)
	cmp	0,g6,g7
	bne	0,FINI
	ai	g3,g3,1			
	l	g6,fpsave+4
	l	g7,0x0020(g5)
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
	.csect	_fmp[RW],3
_fmp: 
	.long	0x7ff00000, 0x00000000		# f5 7ff0000000000000
	.long	0x00000000, 0x00000000		# f24 0000000000000000
	.long	0x00003000			# msr 00003000
	.long	0x0fab45fb			# cr 0fab45fb
	.long	0x79ea00ca			# fpscr 79ea00ca
	.long	0xd05ad5c7, 0x5c9b017c		# f15 d05ad5c75c9b017c
	.long	0x00003000			# msr 00003000
	.long	0x0fab45fb			# cr 0fab45fb
	.long	0xf9f800ca			# fpscr f9f800ca
#
	.long	0x7ff00000, 0x00000000		# f5 7ff0000000000000
	.long	0x00000000, 0x00000000		# f24 0000000000000000
	.long	0x00003000			# msr 00003000
	.long	0x0fab45fb			# cr 0fab45fb
	.long	0x79fa00ca			# fpscr 79fa00ca
	.long	0xd05ad5c7, 0x5c9b017c		# f15 d05ad5c75c9b017c
	.long	0x00003000			# msr 00003000
	.long	0x07ab45fb			# cr 07ab45fb
	.long	0x79f800ca			# fpscr 79f800ca
#
	.long	0x00000000, 0x00000000		# f1 0000000000000000
	.long	0x7ff00000, 0x00000000		# f16 7ff0000000000000
	.long	0x0000e000			# msr 0000e000
	.long	0x58b80754			# cr 58b80754
	.long	0x00000061			# fpscr 00000061
	.long	0x7ff80000, 0x00000000		# f0 7ff8000000000000
	.long	0x0000e000			# msr 0000e000
	.long	0x5ab80754			# cr 5ab80754
	.long	0xa0111061			# fpscr a0111061
#
	.long	0x7ffa1509, 0xad7de0ce		# f13 7ffa1509ad7de0ce
	.long	0x7ff00000, 0x00000000		# f26 7ff0000000000000
	.long	0x00006000			# msr 00006000
	.long	0xb812bdce			# cr b812bdce
	.long	0x00000058			# fpscr 00000058
	.long	0x7ffa1509, 0xad7de0ce		# f31 7ffa1509ad7de0ce
	.long	0x00006000			# msr 00006000
	.long	0xb012bdce			# cr b012bdce
	.long	0x00011058			# fpscr 00011058
#
	.long	0xaca6c9db, 0x7b33c0cb		# f6 aca6c9db7b33c0cb
	.long	0xaca6c9db, 0x7b33c0cb		# f6 aca6c9db7b33c0cb
	.long	0x00007000			# msr 00007000
	.long	0x9b5d468a			# cr 9b5d468a
	.long	0x000000f9			# fpscr 000000f9
	.long	0x19603a87, 0x1c84436b		# f6 19603a871c84436b
	.long	0x00007000			# msr 00007000
	.long	0x9c5d468a			# cr 9c5d468a
	.long	0xc20240f9			# fpscr c20240f9
#
	.long	0x7ff3348d, 0xdcd0b844		# f0 7ff3348ddcd0b844
	.long	0x7ff3348d, 0xdcd0b844		# f0 7ff3348ddcd0b844
	.long	0x0000a000			# msr 0000a000
	.long	0x752a3368			# cr 752a3368
	.long	0x000000d9			# fpscr 000000d9
	.long	0xec1aaf6d, 0xad3b1879		# f29 ec1aaf6dad3b1879
	.long	0x0000a000			# msr 0000a000
	.long	0x7e2a3368			# cr 7e2a3368
	.long	0xe10000d9			# fpscr e10000d9
#
	.long	0x80000000, 0x00000000		# f8 8000000000000000
	.long	0x00000000, 0x00000000		# f30 0000000000000000
	.long	0x00002000			# msr 00002000
	.long	0x83d6da3a			# cr 83d6da3a
	.long	0x00000089			# fpscr 00000089
	.long	0x80000000, 0x00000000		# f3 8000000000000000
	.long	0x00002000			# msr 00002000
	.long	0x80d6da3a			# cr 80d6da3a
	.long	0x00012089			# fpscr 00012089
#
	.long	0x7ff823d2, 0x6a4afe40		# f9 7ff823d26a4afe40
	.long	0x7ffe72a6, 0xf2d5abba		# f27 7ffe72a6f2d5abba
	.long	0x00002080			# msr 00002080
	.long	0x257c1a40			# cr 257c1a40
	.long	0x0000006a			# fpscr 0000006a
	.long	0x7ff823d2, 0x6a4afe40		# f21 7ff823d26a4afe40
	.long	0x00002080			# msr 00002080
	.long	0x207c1a40			# cr 207c1a40
	.long	0x0001106a			# fpscr 0001106a
#
	.long	0xb11a3ea1, 0x847d0a9c		# f11 b11a3ea1847d0a9c
	.long	0xbd4131c4, 0xc381b3d9		# f27 bd4131c4c381b3d9
	.long	0x00002000			# msr 00002000
	.long	0x0ca9ef4e			# cr 0ca9ef4e
	.long	0x0000007b			# fpscr 0000007b
	.long	0x2e6c342e, 0x2b5541a7		# f15 2e6c342e2b5541a7
	.long	0x00002000			# msr 00002000
	.long	0x0ca9ef4e			# cr 0ca9ef4e
	.long	0xc202407b			# fpscr c202407b
#
	.long	0x1ad898ce, 0x4de474e7		# f6 1ad898ce4de474e7
	.long	0x1ad898ce, 0x4de474e7		# f6 1ad898ce4de474e7
	.long	0x0000b000			# msr 0000b000
	.long	0x9baaea47			# cr 9baaea47
	.long	0x00000043			# fpscr 00000043
	.long	0x00000000, 0x00000000		# f5 0000000000000000
	.long	0x0000b000			# msr 0000b000
	.long	0x98aaea47			# cr 98aaea47
	.long	0x8a022043			# fpscr 8a022043
#
	.long	0xfca8b43d, 0x72de1941		# f7 fca8b43d72de1941
	.long	0x44821f73, 0x969794f0		# f16 44821f73969794f0
	.long	0x00006080			# msr 00006080
	.long	0xacb81b0b			# cr acb81b0b
	.long	0x000000b9			# fpscr 000000b9
	.long	0xffefffff, 0xffffffff		# f16 ffefffffffffffff
	.long	0x00006080			# msr 00006080
	.long	0xadb81b0b			# cr adb81b0b
	.long	0xd20280b9			# fpscr d20280b9
#
	.long	0x00000021, 0x321fedfa		# f12 00000021321fedfa
	.long	0x00000000, 0x0000001b		# f29 000000000000001b
	.long	0x00002000			# msr 00002000
	.long	0xc0b312a3			# cr c0b312a3
	.long	0x000000d9			# fpscr 000000d9
	.long	0x00000000, 0x00000000		# f10 0000000000000000
	.long	0x00002000			# msr 00002000
	.long	0xccb312a3			# cr ccb312a3
	.long	0xca0220d9			# fpscr ca0220d9
#
	.long	0xf5200000, 0x00000000		# f3 f520000000000000
	.long	0x6182a714, 0xa00b0bca		# f24 6182a714a00b0bca
	.long	0x0000a000			# msr 0000a000
	.long	0xcac73c34			# cr cac73c34
	.long	0x00000012			# fpscr 00000012
	.long	0xffefffff, 0xffffffff		# f29 ffefffffffffffff
	.long	0x0000a000			# msr 0000a000
	.long	0xc9c73c34			# cr c9c73c34
	.long	0x92028012			# fpscr 92028012
#
	.long	0xf15486b9, 0x89daf0a8		# f15 f15486b989daf0a8
	.long	0x7a552e60, 0x8dc8099d		# f19 7a552e608dc8099d
	.long	0x0000b080			# msr 0000b080
	.long	0xa6ffe135			# cr a6ffe135
	.long	0x00060041			# fpscr 00060041
	.long	0xcbbb2c52, 0xb898cfe5		# f16 cbbb2c52b898cfe5
	.long	0x0000b080			# msr 0000b080
	.long	0xadffe135			# cr adffe135
	.long	0xd2028041			# fpscr d2028041
#
#
#
fpsave:	.long	0xffffffff, 0xffffffff
#
	.toc
	TOCE(.fmp, entry)
	TOCL(_fmp, data)
