# @(#)34	1.3  src/bos/diag/da/fpa/fap.s, dafpa, bos411, 9428A410j 3/23/94 06:23:24

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
	.csect	.fap[PR]
	.globl	.fap[PR]
	.align	2
.fap:
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
	LTOC(4, _fap, data)		# Loads g4 with the address of _fap
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
 	.using	_fap[RW],g4		# g4 is the base register to the data
#	 				     section _fap[rw]. Eastablishing 
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
	ai	g9,g9,26
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
	fa.	r0,r1,r2
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
	.csect	_fap[RW],3
_fap: 
	.long	0xfff61f00, 0x05fb8328		# f3 fff61f0005fb8328
	.long	0xfff00000, 0x00000000		# f25 fff0000000000000
	.long	0x00007080			# msr 00007080
	.long	0x4847fc2d			# cr 4847fc2d
	.long	0x00000083			# fpscr 00000083
	.long	0x308866d6, 0xeb1987d2		# f30 308866d6eb1987d2
	.long	0x00007080			# msr 00007080
	.long	0x4e47fc2d			# cr 4e47fc2d
	.long	0xe1000083			# fpscr e1000083
#
	.long	0x7ffec729, 0xc39862ef		# f22 7ffec729c39862ef
	.long	0x8a82a7db, 0x38570a6c		# f24 8a82a7db38570a6c
	.long	0x0000a000			# msr 0000a000
	.long	0xeec10fb4			# cr eec10fb4
	.long	0x00000079			# fpscr 00000079
	.long	0x7ffec729, 0xc39862ef		# f24 7ffec729c39862ef
	.long	0x0000a000			# msr 0000a000
	.long	0xe0c10fb4			# cr e0c10fb4
	.long	0x00011079			# fpscr 00011079
#
	.long	0x7e0fffff, 0xffffffff		# f26 7e0fffffffffffff
	.long	0x7e0fffff, 0xffffffff		# f26 7e0fffffffffffff
	.long	0x00007000			# msr 00007000
	.long	0x3ea81164			# cr 3ea81164
	.long	0x00000082			# fpscr 00000082
	.long	0x7e1fffff, 0xffffffff		# f22 7e1fffffffffffff
	.long	0x00007000			# msr 00007000
	.long	0x30a81164			# cr 30a81164
	.long	0x00004082			# fpscr 00004082
#
	.long	0xfcac0000, 0x00000000		# f13 fcac000000000000
	.long	0xffefffff, 0xffffffff		# f17 ffefffffffffffff
	.long	0x0000b080			# msr 0000b080
	.long	0x4f8b3c96			# cr 4f8b3c96
	.long	0x000000d2			# fpscr 000000d2
	.long	0x9ff00000, 0x00000000		# f21 9ff0000000000000
	.long	0x0000b080			# msr 0000b080
	.long	0x4d8b3c96			# cr 4d8b3c96
	.long	0xd20280d2			# fpscr d20280d2
#
	.long	0xdc6eb108, 0xa593999f		# f29 dc6eb108a593999f
	.long	0x00018f0b, 0x37695f60		# f30 00018f0b37695f60
	.long	0x0000b000			# msr 0000b000
	.long	0x68ddeb1e			# cr 68ddeb1e
	.long	0x000000f3			# fpscr 000000f3
	.long	0xdc6eb108, 0xa593999f		# f29 dc6eb108a593999f
	.long	0x0000b000			# msr 0000b000
	.long	0x68ddeb1e			# cr 68ddeb1e
	.long	0x820680f3			# fpscr 820680f3
#
	.long	0x9f01845e, 0xc4374ada		# f0 9f01845ec4374ada
	.long	0xffefffff, 0xffffffff		# f18 ffefffffffffffff
	.long	0x0000e000			# msr 0000e000
	.long	0x3783a878			# cr 3783a878
	.long	0x0000006b			# fpscr 0000006b
	.long	0x9ff00000, 0x00000000		# f0 9ff0000000000000
	.long	0x0000e000			# msr 0000e000
	.long	0x3d83a878			# cr 3d83a878
	.long	0xd206806b			# fpscr d206806b
#
	.long	0xee6494ed, 0x1fecb852		# f9 ee6494ed1fecb852
	.long	0xee8e7aae, 0x658fb84a		# f18 ee8e7aae658fb84a
	.long	0x0000a080			# msr 0000a080
	.long	0xbb2b9fae			# cr bb2b9fae
	.long	0x00000068			# fpscr 00000068
	.long	0xee91cff4, 0xd6c5732f		# f20 ee91cff4d6c5732f
	.long	0x0000a080			# msr 0000a080
	.long	0xbc2b9fae			# cr bc2b9fae
	.long	0xc2028068			# fpscr c2028068
#
	.long	0x80000000, 0x00000000		# f16 8000000000000000
	.long	0x80000000, 0x00000000		# f25 8000000000000000
	.long	0x0000a000			# msr 0000a000
	.long	0x2c6d8060			# cr 2c6d8060
	.long	0x00000032			# fpscr 00000032
	.long	0x80000000, 0x00000000		# f13 8000000000000000
	.long	0x0000a000			# msr 0000a000
	.long	0x206d8060			# cr 206d8060
	.long	0x00012032			# fpscr 00012032
#
	.long	0x000ffff8, 0x00000000		# f7 000ffff800000000
	.long	0x800fffff, 0xf8000000		# f24 800ffffff8000000
	.long	0x00003000			# msr 00003000
	.long	0x875cc596			# cr 875cc596
	.long	0x00000012			# fpscr 00000012
	.long	0x80000007, 0xf8000000		# f18 80000007f8000000
	.long	0x00003000			# msr 00003000
	.long	0x805cc596			# cr 805cc596
	.long	0x00018012			# fpscr 00018012
#
	.long	0xab85e88d, 0xd25b6789		# f26 ab85e88dd25b6789
	.long	0x89666918, 0x037bad70		# f29 89666918037bad70
	.long	0x00002000			# msr 00002000
	.long	0xc96715a3			# cr c96715a3
	.long	0x000000d2			# fpscr 000000d2
	.long	0xab85e88d, 0xd25b6789		# f29 ab85e88dd25b6789
	.long	0x00002000			# msr 00002000
	.long	0xc86715a3			# cr c86715a3
	.long	0x820280d2			# fpscr 820280d2
#
	.long	0x7fefffff, 0xffffffff		# f7 7fefffffffffffff
	.long	0x7c880000, 0x00000000		# f29 7c88000000000000
	.long	0x0000e080			# msr 0000e080
	.long	0xec4bf666			# cr ec4bf666
	.long	0x00000050			# fpscr 00000050
	.long	0x7fefffff, 0xffffffff		# f27 7fefffffffffffff
	.long	0x0000e080			# msr 0000e080
	.long	0xe84bf666			# cr e84bf666
	.long	0x82024050			# fpscr 82024050
#
	.long	0xffefffff, 0xffffffff		# f9 ffefffffffffffff
	.long	0xfca40000, 0x00000000		# f12 fca4000000000000
	.long	0x0000b000			# msr 0000b000
	.long	0xb1a8ec15			# cr b1a8ec15
	.long	0x00000013			# fpscr 00000013
	.long	0xfff00000, 0x00000000		# f23 fff0000000000000
	.long	0x0000b000			# msr 0000b000
	.long	0xb9a8ec15			# cr b9a8ec15
	.long	0x92069013			# fpscr 92069013
#
	.long	0x7c940000, 0x00000000		# f11 7c94000000000000
	.long	0x7fefffff, 0xffffffff		# f26 7fefffffffffffff
	.long	0x00003000			# msr 00003000
	.long	0x2f98cbbf			# cr 2f98cbbf
	.long	0x0000009a			# fpscr 0000009a
	.long	0x7ff00000, 0x00000000		# f2 7ff0000000000000
	.long	0x00003000			# msr 00003000
	.long	0x2d98cbbf			# cr 2d98cbbf
	.long	0xd206509a			# fpscr d206509a
#
	.long	0xffefffff, 0xffffffff		# f17 ffefffffffffffff
	.long	0xfc8c0000, 0x00000000		# f28 fc8c000000000000
	.long	0x0000b000			# msr 0000b000
	.long	0x59c6e810			# cr 59c6e810
	.long	0x00000039			# fpscr 00000039
	.long	0xffefffff, 0xffffffff		# f25 ffefffffffffffff
	.long	0x0000b000			# msr 0000b000
	.long	0x5cc6e810			# cr 5cc6e810
	.long	0xc2028039			# fpscr c2028039
#
	.long	0xac4dcf1b, 0x2a68baf7		# f13 ac4dcf1b2a68baf7
	.long	0xfff12261, 0x68a3f8f2		# f29 fff1226168a3f8f2
	.long	0x0000b080			# msr 0000b080
	.long	0xd7c7e7ae			# cr d7c7e7ae
	.long	0x000000fb			# fpscr 000000fb
	.long	0xac4dcf1b, 0x2a68baf7		# f13 ac4dcf1b2a68baf7
	.long	0x0000b080			# msr 0000b080
	.long	0xdec7e7ae			# cr dec7e7ae
	.long	0xe10000fb			# fpscr e10000fb
#
	.long	0xbffba833, 0x50b4e3ca		# f0 bffba83350b4e3ca
	.long	0xbff36265, 0xca7d8bba		# f7 bff36265ca7d8bba
	.long	0x0000b000			# msr 0000b000
	.long	0x15c0700f			# cr 15c0700f
	.long	0x00000023			# fpscr 00000023
	.long	0xc007854c, 0x8d9937c2		# f11 c007854c8d9937c2
	.long	0x0000b000			# msr 0000b000
	.long	0x10c0700f			# cr 10c0700f
	.long	0x00008023			# fpscr 00008023
#
	.long	0xfff00000, 0x00000000		# f4 fff0000000000000
	.long	0x7ff00000, 0x00000000		# f28 7ff0000000000000
	.long	0x00006000			# msr 00006000
	.long	0x5048443c			# cr 5048443c
	.long	0x000000f2			# fpscr 000000f2
	.long	0x95b63ae0, 0x9a2a7f1f		# f7 95b63ae09a2a7f1f
	.long	0x00006000			# msr 00006000
	.long	0x5e48443c			# cr 5e48443c
	.long	0xe08000f2			# fpscr e08000f2
#
	.long	0x80000000, 0x00000000		# f3 8000000000000000
	.long	0x00000000, 0x00000000		# f4 0000000000000000
	.long	0x00006000			# msr 00006000
	.long	0x1c7428ac			# cr 1c7428ac
	.long	0x000000d0			# fpscr 000000d0
	.long	0x00000000, 0x00000000		# f11 0000000000000000
	.long	0x00006000			# msr 00006000
	.long	0x107428ac			# cr 107428ac
	.long	0x000020d0			# fpscr 000020d0
#
	.long	0x8ae12832, 0xf7a749b6		# f24 8ae12832f7a749b6
	.long	0x8ae12832, 0xf7a749b6		# f24 8ae12832f7a749b6
	.long	0x0000e080			# msr 0000e080
	.long	0x036e43c8			# cr 036e43c8
	.long	0x00000000			# fpscr 00000000
	.long	0x8af12832, 0xf7a749b6		# f22 8af12832f7a749b6
	.long	0x0000e080			# msr 0000e080
	.long	0x006e43c8			# cr 006e43c8
	.long	0x00008000			# fpscr 00008000
#
	.long	0xfca80000, 0x00000000		# f9 fca8000000000000
	.long	0xffefffff, 0xffffffff		# f21 ffefffffffffffff
	.long	0x0000b000			# msr 0000b000
	.long	0x6729d549			# cr 6729d549
	.long	0x00040018			# fpscr 00040018
	.long	0xfff00000, 0x00000000		# f25 fff0000000000000
	.long	0x0000b000			# msr 0000b000
	.long	0x6d29d549			# cr 6d29d549
	.long	0xd2029018			# fpscr d2029018
#
	.long	0x7fefffff, 0xffffffff		# f7 7fefffffffffffff
	.long	0x7fe00000, 0x00000001		# f29 7fe0000000000001
	.long	0x0000e080			# msr 0000e080
	.long	0xec4bf666			# cr ec4bf666
	.long	0x00000050			# fpscr 00000050
	.long	0x1ff80000, 0x00000000		# f27 1ff8000000000000
	.long	0x0000e080			# msr 0000e080
	.long	0xed4bf666			# cr ed4bf666
	.long	0xd0004050			# fpscr d0004050
#
	.long	0x7fefffff, 0xfffffe00		# f6 7feffffffffffe00
	.long	0x7f5fffff, 0xffffffff		# f11 7f5fffffffffffff
	.long	0x00007080			# msr 00007080
	.long	0xa9b53204			# cr a9b53204
	.long	0x7c7a00da			# fpscr 7c7a00da
	.long	0x1ff007ff, 0xffffff00		# f17 1ff007ffffffff00
	.long	0x00007080			# msr 00007080
	.long	0xafb53204			# cr afb53204
	.long	0xfe7e40da			# fpscr fe7e40da
#
	.long	0x8001619c, 0x338b0df6		# f1 8001619c338b0df6
	.long	0x800afee7, 0xd4df0dc4		# f6 800afee7d4df0dc4
	.long	0x00007000			# msr 00007000
	.long	0x5c2f610f			# cr 5c2f610f
	.long	0x72c000ba			# fpscr 72c000ba
	.long	0xe008c108, 0x10d43774		# f4 e008c10810d43774
	.long	0x00007000			# msr 00007000
	.long	0x5f2f610f			# cr 5f2f610f
	.long	0xfac080ba			# fpscr fac080ba
#
	.long	0x7fefffff, 0xffffffff		# f7 7fefffffffffffff
	.long	0x7fe00000, 0x00000001		# f29 7fe0000000000001
	.long	0x0000e080			# msr 0000e080
	.long	0xec4bf666			# cr ec4bf666
	.long	0x10000050			# fpscr 10000050
	.long	0x1ff80000, 0x00000000		# f27 1ff8000000000000
	.long	0x0000e080			# msr 0000e080
	.long	0xe54bf666			# cr e54bf666
	.long	0x50004050			# fpscr 50004050
#
	.long	0x7fefffff, 0xfffffe00		# f6 7feffffffffffe00
	.long	0x7f5fffff, 0xffffffff		# f11 7f5fffffffffffff
	.long	0x00007080			# msr 00007080
	.long	0xa9b53204			# cr a9b53204
	.long	0x7e7a00da			# fpscr 7e7a00da
	.long	0x1ff007ff, 0xffffff00		# f17 1ff007ffffffff00
	.long	0x00007080			# msr 00007080
	.long	0xa7b53204			# cr a7b53204
	.long	0x7e7e40da			# fpscr 7e7e40da
#
	.long	0x8001619c, 0x338b0df6		# f1 8001619c338b0df6
	.long	0x800afee7, 0xd4df0dc4		# f6 800afee7d4df0dc4
	.long	0x00007000			# msr 00007000
	.long	0x5c2f610f			# cr 5c2f610f
	.long	0x7ac000ba			# fpscr 7ac000ba
	.long	0xe008c108, 0x10d43774		# f4 e008c10810d43774
	.long	0x00007000			# msr 00007000
	.long	0x572f610f			# cr 572f610f
	.long	0x7ac080ba			# fpscr 7ac080ba
#
#
#
#
fpsave:	.long	0xffffffff, 0xffffffff
#
	.toc
	TOCE(.fap, entry)
	TOCL(_fap, data)
