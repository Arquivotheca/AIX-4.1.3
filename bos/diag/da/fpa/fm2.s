# @(#)35	1.3  src/bos/diag/da/fpa/fm2.s, dafpa, bos411, 9428A410j 3/23/94 06:25:42

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
	.csect	.fm2[PR]
	.globl	.fm2[PR]
	.align	2
.fm2:
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
	LTOC(4, _fm2, data)		# Loads g4 with the address of _fm2
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
 	.using	_fm2[RW],g4		# g4 is the base register to the data
#	 				     section _fm2[rw]. Eastablishing 
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
	ai	g9,g9,2
#
fst:
	lfd	rf6,0x0010(g5)		# Tricky! Does not load from MSR
	mtfsf	0xff,rf6		# load fpscr (input)
#
	lfd	rf0,0x0018(g5)		# preload result to avoid NaN op.
#
	lfd	rf1,0x0000(g5)
	lfd	rf2,0x0008(g5)
#
#  Instructions used for testing
# 
	fm	r0,r1,r2
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
	bne	1,fst
#
	xor	g3,g3,g3		# Return code = 0; pass
#
FINI:	br
#
# Data Area
#
	.csect	_fm2[RW],3
_fm2: 
# 12
	.long	0xf2139a5a, 0xc1ee3b99		# f13 f2139a5ac1ee3b99
	.long	0xfff4e164, 0x06ba4684		# f24 fff4e16406ba4684
	.long	0x00002080			# msr 00002080
	.long	0x00000061			# fpscr 00000061
	.long	0xfffce164, 0x06ba4684		# f28 fffce16406ba4684
	.long	0x00002080			# msr 00002080
	.long	0xa1011061			# fpscr a1011061
# 13
	.long	0xf28672b3, 0x4edd1af6		# f6 f28672b34edd1af6
	.long	0x7ff00000, 0x00000000		# f11 7ff0000000000000
	.long	0x00002000			# msr 00002000
	.long	0x00000068			# fpscr 00000068
	.long	0xfff00000, 0x00000000		# f29 fff0000000000000
	.long	0x00002000			# msr 00002000
	.long	0x00009068			# fpscr 00009068/d0012068
# 14
#	.long	0x7ff4d4b4, 0x31a2038e		# f11 7ff4d4b431a2038e
#	.long	0x7ff0c916, 0x41647043		# f17 7ff0c91641647043
#	.long	0x00003000			# msr 00003000
#	.long	0x00000062			# fpscr 00000062
#	.long	0x7ff8c916, 0x41647043		# f0 7ff8c91641647043/7ffcd4b4xx
#	.long	0x00003000			# msr 00003000
#	.long	0xa1011062			# fpscr a1011062
#
#
fpsave:	.long	0xffffffff, 0xffffffff
#
	.toc
	TOCE(.fm2, entry)
	TOCL(_fm2, data)
