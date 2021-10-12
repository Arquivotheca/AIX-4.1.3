# @(#)20	1.3  src/bos/diag/da/fpa/fdpp.s, dafpa, bos411, 9428A410j 3/23/94 06:25:20

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
	.csect	.fdpp[PR]
	.globl	.fdpp[PR]
	.align	2
.fdpp:
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
	LTOC(4, _fdpp, data)		# Loads g4 with the address of _fdpp
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
 	.using	_fdpp[RW],g4		# g4 is the base register to the data
#	 				     section _fdpp[rw]. Eastablishing 
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
	ai	g9,g9,16
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
	fd.	r0,r1,r2
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
	.csect	_fdpp[RW],3
_fdpp: 
	.long	0x80000000, 0x00000000		# f17 8000000000000000
	.long	0x00000000, 0x00000000		# f0 0000000000000000
	.long	0x00003000			# msr 00003000
	.long	0xd7a7914d			# cr d7a7914d
	.long	0x000000d0			# fpscr 000000d0
	.long	0xd7ee9ea9, 0xfc67dac9		# f18 d7ee9ea9fc67dac9
	.long	0x00003000			# msr 00003000
	.long	0xdea7914d			# cr dea7914d
	.long	0xe02000d0			# fpscr e02000d0
#
	.long	0x2a866a93, 0x5fe31e3b		# f17 2a866a935fe31e3b
	.long	0x00000000, 0x00000000		# f4 0000000000000000
	.long	0x00002080			# msr 00002080
	.long	0x611e1b9c			# cr 611e1b9c
	.long	0x78720051			# fpscr 78720051
	.long	0xb2005fe6, 0x4dbe165b		# f6 b2005fe64dbe165b
	.long	0x00002080			# msr 00002080
	.long	0x6f1e1b9c			# cr 6f1e1b9c
	.long	0xfc700051			# fpscr fc700051
#
	.long	0x00000000, 0x00000000		# f26 0000000000000000
	.long	0x00000000, 0x00000000		# f17 0000000000000000
	.long	0x0000b000			# msr 0000b000
	.long	0xe3045f1d			# cr e3045f1d
	.long	0x34c20008			# fpscr 34c20008
	.long	0x7ff80000, 0x00000000		# f3 7ff8000000000000
	.long	0x0000b000			# msr 0000b000
	.long	0xeb045f1d			# cr eb045f1d
	.long	0xb4e11008			# fpscr b4e11008
#
	.long	0x2a866a93, 0x5fe31e3b		# f17 2a866a935fe31e3b
	.long	0x00000000, 0x00000000		# f4 0000000000000000
	.long	0x00002080			# msr 00002080
	.long	0x611e1b9c			# cr 611e1b9c
	.long	0x7c720051			# fpscr 7c720051
	.long	0xb2005fe6, 0x4dbe165b		# f6 b2005fe64dbe165b
	.long	0x00002080			# msr 00002080
	.long	0x671e1b9c			# cr 671e1b9c
	.long	0x7c700051			# fpscr 7c700051
#
	.long	0x00000000, 0x00000000		# f26 0000000000000000
	.long	0x00000000, 0x00000000		# f17 0000000000000000
	.long	0x0000b000			# msr 0000b000
	.long	0xe3045f1d			# cr e3045f1d
	.long	0x34e20008			# fpscr 34e20008
	.long	0x7ff80000, 0x00000000		# f3 7ff8000000000000
	.long	0x0000b000			# msr 0000b000
	.long	0xe3045f1d			# cr e3045f1d
	.long	0x34e11008			# fpscr 34e11008
#
	.long	0xfff00000, 0x00000000		# f13 fff0000000000000
	.long	0x80000000, 0x00000000		# f23 8000000000000000
	.long	0x0000b000			# msr 0000b000
	.long	0xc210e89b			# cr c210e89b
	.long	0x00000003			# fpscr 00000003
	.long	0x7ff00000, 0x00000000		# f22 7ff0000000000000
	.long	0x0000b000			# msr 0000b000
	.long	0xc010e89b			# cr c010e89b
	.long	0x00005003			# fpscr 00005003
#
	.long	0x00000000, 0x00000000		# f1 0000000000000000
	.long	0x00000000, 0x00000000		# f15 0000000000000000
	.long	0x0000a000			# msr 0000a000
	.long	0x584749c0			# cr 584749c0
	.long	0x00000012			# fpscr 00000012
	.long	0x7ff80000, 0x00000000		# f9 7ff8000000000000
	.long	0x0000a000			# msr 0000a000
	.long	0x5a4749c0			# cr 5a4749c0
	.long	0xa0211012			# fpscr a0211012
#
	.long	0x1057941e, 0xbb0d85fd		# f3 1057941ebb0d85fd
	.long	0xfff07436, 0x7cb91712		# f22 fff074367cb91712
	.long	0x0000b000			# msr 0000b000
	.long	0xb67f8f96			# cr b67f8f96
	.long	0x00000011			# fpscr 00000011
	.long	0xfff87436, 0x7cb91712		# f8 fff874367cb91712
	.long	0x0000b000			# msr 0000b000
	.long	0xba7f8f96			# cr ba7f8f96
	.long	0xa1011011			# fpscr a1011011
#
	.long	0xfff08cfd, 0x174613a7		# f17 fff08cfd174613a7
	.long	0xff19013d, 0xbf73fdfd		# f24 ff19013dbf73fdfd
	.long	0x0000a000			# msr 0000a000
	.long	0x070b0b17			# cr 070b0b17
	.long	0x000000aa			# fpscr 000000aa
	.long	0x0ecd8277, 0xb727eef9		# f3 0ecd8277b727eef9
	.long	0x0000a000			# msr 0000a000
	.long	0x0e0b0b17			# cr 0e0b0b17
	.long	0xe10000aa			# fpscr e10000aa
#
	.long	0xfff00000, 0x00000000		# f19 fff0000000000000
	.long	0xfff00000, 0x00000000		# f17 fff0000000000000
	.long	0x00007080			# msr 00007080
	.long	0x035216e6			# cr 035216e6
	.long	0x00000013			# fpscr 00000013
	.long	0x7ff80000, 0x00000000		# f11 7ff8000000000000
	.long	0x00007080			# msr 00007080
	.long	0x0a5216e6			# cr 0a5216e6
	.long	0xa0411013			# fpscr a0411013
#
	.long	0x7ff0cd74, 0x4da6a893		# f4 7ff0cd744da6a893
	.long	0x7ff521fc, 0x504fa031		# f1 7ff521fc504fa031
	.long	0x0000e000			# msr 0000e000
	.long	0x874b24ef			# cr 874b24ef
	.long	0x00000022			# fpscr 00000022
	.long	0x7ff8cd74, 0x4da6a893		# f29 7ff8cd744da6a893
	.long	0x0000e000			# msr 0000e000
	.long	0x8a4b24ef			# cr 8a4b24ef
	.long	0xa1011022			# fpscr a1011022
#
	.long	0x3d4c987e, 0xdf825e7f		# f1 3d4c987edf825e7f
	.long	0x7d8a6efb, 0x11cd7b8e		# f30 7d8a6efb11cd7b8e
	.long	0x0000e080			# msr 0000e080
	.long	0x2ceac568			# cr 2ceac568
	.long	0x000000f1			# fpscr 000000f1
	.long	0x5fb14f09, 0xc5a2801d		# f8 5fb14f09c5a2801d
	.long	0x0000e080			# msr 0000e080
	.long	0x2ceac568			# cr 2ceac568
	.long	0xca0240f1			# fpscr ca0240f1
#
	.long	0x3cd43109, 0x622755ee		# f10 3cd43109622755ee
	.long	0x80000000, 0x00000001		# f16 8000000000000001
	.long	0x00002000			# msr 00002000
	.long	0x54622824			# cr 54622824
	.long	0x00000058			# fpscr 00000058
	.long	0x9ff43109, 0x622755ee		# f16 9ff43109622755ee
	.long	0x00002000			# msr 00002000
	.long	0x5d622824			# cr 5d622824
	.long	0xd0008058			# fpscr d0008058
#
	.long	0x347087fe, 0xe7980257		# f25 347087fee7980257
	.long	0xf68087fe, 0xe7980257		# f2 f68087fee7980257
	.long	0x0000a000			# msr 0000a000
	.long	0xbed3ef7a			# cr bed3ef7a
	.long	0x00000081			# fpscr 00000081
	.long	0x80000000, 0x00020000		# f28 8000000000020000
	.long	0x0000a000			# msr 0000a000
	.long	0xb0d3ef7a			# cr b0d3ef7a
	.long	0x00018081			# fpscr 00018081
#
	.long	0xfa1c7226, 0x55f0d4ba		# f8 fa1c722655f0d4ba
	.long	0x8dfc7226, 0x55f0d4ba		# f19 8dfc722655f0d4ba
	.long	0x0000a000			# msr 0000a000
	.long	0x218f8f36			# cr 218f8f36
	.long	0x00000030			# fpscr 00000030
	.long	0x7ff00000, 0x00000000		# f8 7ff0000000000000
	.long	0x0000a000			# msr 0000a000
	.long	0x298f8f36			# cr 298f8f36
	.long	0x92025030			# fpscr 92025030
#
	.long	0xf4830bb0, 0xb6202458		# f20 f4830bb0b6202458
	.long	0x98c88df3, 0x5837ff60		# f25 98c88df35837ff60
	.long	0x0000e000			# msr 0000e000
	.long	0x39c25adb			# cr 39c25adb
	.long	0x000600b3			# fpscr 000600b3
	.long	0x7fefffff, 0xffffffff		# f23 7fefffffffffffff
	.long	0x0000e000			# msr 0000e000
	.long	0x39c25adb			# cr 39c25adb
	.long	0x920240b3			# fpscr 920240b3
#
#
#
fpsave:	.long	0xffffffff, 0xffffffff
#
	.toc
	TOCE(.fdpp, entry)
	TOCL(_fdpp, data)
