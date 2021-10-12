# @(#)36	1.3  src/bos/diag/da/fpa/frspp.s, dafpa, bos411, 9428A410j 3/23/94 06:26:41

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
	.csect	.frspp[PR]
	.globl	.frspp[PR]
	.align	2
.frspp:
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
	LTOC(4, _frspp, data)		# Loads g4 with the address of _frspp
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
 	.using	_frspp[RW],g4		# g4 is the base register to the data
#	 				     section _frspp[rw]. Eastablishing 
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
	ai	g9,g9,10
#
fstart:
	l	g8,0x000c(g5)
	mtcrf	0xff,g8
#
	lfd	rf6,0x000c(g5)		# Tricky! Does not load from CR
	mtfsf	0xff,rf6		# load fpscr (input)
#
	lfd	rf1,0x0000(g5)
#
#  Instructions used for testing
# 
	frsp.	r0,r1
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
#
	ai	g3,g3,1
	l	g6,0x0020(g5)
	mfcr	g7
	cmp	0,g6,g7
	bne	0,FINI
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
	l	g7,0x0014(g5)
	cmp	0,g6,g7
	bne	0,FINI
	ai	g3,g3,1			
	l	g6,fpsave+4
	l	g7,0x0018(g5)
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
	.csect	_frspp[RW],3
_frspp: 
	.long	0x7fefffff, 0xffffec27		# f4 7fefffffffffec27
	.long	0x0000a000			# msr 0000a000
	.long	0xed3243e9			# cr ed3243e9
	.long	0x000200f3			# fpscr 000200f3
	.long	0x73efffff, 0xe0000000		# f7 73efffffe0000000
	.long	0x0000a000			# msr 0000a000
	.long	0xed3243e9			# cr ed3243e9
	.long	0xd20240f3			# fpscr d20240f3
#
	.long	0xfff00000, 0x00000000		# f16 fff0000000000000
	.long	0x00002000			# msr 00002000
	.long	0x0bb92793			# cr 0bb92793
	.long	0x0006005a			# fpscr 0006005a
	.long	0xfff00000, 0x00000000		# f16 fff0000000000000
	.long	0x00002000			# msr 00002000
	.long	0x00b92793			# cr 00b92793
	.long	0x0000905a			# fpscr 0000905a
#
	.long	0x630fffff, 0xffffc247		# f14 630fffffffffc247
	.long	0x00002000			# msr 00002000
	.long	0xb5edccc5			# cr b5edccc5
	.long	0x00060010			# fpscr 00060010
	.long	0x7ff00000, 0x00000000		# f5 7ff0000000000000
	.long	0x00002000			# msr 00002000
	.long	0xb9edccc5			# cr b9edccc5
	.long	0x92065010			# fpscr 92065010
#
	.long	0x801ffff9, 0x54232063		# f21 801ffff954232063
	.long	0x00002000			# msr 00002000
	.long	0x8568bc8f			# cr 8568bc8f
	.long	0x0004002b			# fpscr 0004002b
	.long	0x8c1ffff9, 0x60000000		# f19 8c1ffff960000000
	.long	0x00002000			# msr 00002000
	.long	0x8c68bc8f			# cr 8c68bc8f
	.long	0xca06802b			# fpscr ca06802b
#
	.long	0x3801b1ec, 0xe48e3d50		# f13 3801b1ece48e3d50
	.long	0x00002000			# msr 00002000
	.long	0x0c21b404			# cr 0c21b404
	.long	0x00020050			# fpscr 00020050
	.long	0x3801b1ed, 0x00000000		# f28 3801b1ed00000000
	.long	0x00002000			# msr 00002000
	.long	0x0821b404			# cr 0821b404
	.long	0x8a074050			# fpscr 8a074050
#
	.long	0xc7ffffff, 0xd09d0293		# f31 c7ffffffd09d0293
	.long	0x0000a000			# msr 0000a000
	.long	0xee41aa18			# cr ee41aa18
	.long	0x0004003b			# fpscr 0004003b
	.long	0xfff00000, 0x00000000		# f11 fff0000000000000
	.long	0x0000a000			# msr 0000a000
	.long	0xed41aa18			# cr ed41aa18
	.long	0xd206903b			# fpscr d206903b
#
	.long	0xb96ff219, 0x0b93a878		# f27 b96ff2190b93a878
	.long	0x0000a000			# msr 0000a000
	.long	0xa5c2a774			# cr a5c2a774
	.long	0x00020050			# fpscr 00020050
	.long	0xb96ff219, 0x00000000		# f22 b96ff21900000000
	.long	0x0000a000			# msr 0000a000
	.long	0xa8c2a774			# cr a8c2a774
	.long	0x82028050			# fpscr 82028050
#
	.long	0xffdfffff, 0xffd70c8d		# f1 ffdfffffffd70c8d
	.long	0x0000a000			# msr 0000a000
	.long	0x15093612			# cr 15093612
	.long	0x0002007a			# fpscr 0002007a
	.long	0xf3dfffff, 0xe0000000		# f22 f3dfffffe0000000
	.long	0x0000a000			# msr 0000a000
	.long	0x1d093612			# cr 1d093612
	.long	0xd202807a			# fpscr d202807a
#
	.long	0xffe00000, 0x00000000		# f23 ffe0000000000000
	.long	0x00002000			# msr 00002000
	.long	0x921268b6			# cr 921268b6
	.long	0x0000002b			# fpscr 0000002b
	.long	0xfff00000, 0x00000000		# f31 fff0000000000000
	.long	0x00002000			# msr 00002000
	.long	0x9d1268b6			# cr 9d1268b6
	.long	0xd202902b			# fpscr d202902b
#
	.long	0xb80fffff, 0xffe59d25		# f16 b80fffffffe59d25
	.long	0x0000b000			# msr 0000b000
	.long	0x9861bfdc			# cr 9861bfdc
	.long	0x00000001			# fpscr 00000001
	.long	0xb80fffff, 0xc0000000		# f30 b80fffffc0000000
	.long	0x0000b000			# msr 0000b000
	.long	0x9861bfdc			# cr 9861bfdc
	.long	0x8a038001			# fpscr 8a038001
#
#
fpsave:	.long	0xffffffff, 0xffffffff
#
	.toc
	TOCE(.frspp, entry)
	TOCL(_frspp, data)
