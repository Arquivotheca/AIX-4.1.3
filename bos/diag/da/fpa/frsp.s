# @(#)29	1.3  src/bos/diag/da/fpa/frsp.s, dafpa, bos411, 9428A410j 3/23/94 06:26:30

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
#		will be compared with expected results. If any test frspils,
#		a particular return code will be returned to the caller.
#
	.csect	.frsp[PR]
	.globl	.frsp[PR]
	.align	2
.frsp:
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
	LTOC(4, _frsp, data)		# Loads g4 with the address of _frsp
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
 	.using	_frsp[RW],g4		# g4 is the base register to the data
#	 				     section _frsp[rw]. Eastablishing 
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
	ai	g9,g9,12
#
fstart:
	lfd	rf6,0x0008(g5)		# Tricky! Does not load from MSR
	mtfsf	0xff,rf6		# load fpscr (input)
#
	lfd	rf1,0x0000(g5)
#
#  Instructions used for testing
# 
	frsp	r0,r1
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
	l	g7,0x001c(g5)
	cmp	0,g6,g7
	bne	0,FINI	
#
	ai	g3,g3,1			
	stfd	rf0,fpsave
	l	g6,fpsave
	l	g7,0x0010(g5)
	cmp	0,g6,g7
	bne	0,FINI
	ai	g3,g3,1			
	l	g6,fpsave+4
	l	g7,0x0014(g5)
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g5,g5,0x0020		# g5 now points to data of new
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
	.csect	_frsp[RW],3
_frsp: 
	.long	0xb6afffff, 0xffff6337		# f0 b6afffffffff6337
	.long	0x0000e000			# msr 0000e000
	.long	0x000000d1			# fpscr 000000d1
	.long	0xb6a00000, 0x00000000		# f8 b6a0000000000000
	.long	0x0000e000			# msr 0000e000
	.long	0x8a0380d1			# fpscr 8a0380d1
#
	.long	0xc7dffe76, 0x7b85c0ab		# f18 c7dffe767b85c0ab
	.long	0x0000a000			# msr 0000a000
	.long	0x00020003			# fpscr 00020003
	.long	0xc7dffe76, 0x80000000		# f30 c7dffe7680000000
	.long	0x0000a000			# msr 0000a000
	.long	0x82068003			# fpscr 82068003
#
	.long	0xfffe33eb, 0x381f3757		# f31 fffe33eb381f3757
	.long	0x00002000			# msr 00002000
	.long	0x00000073			# fpscr 00000073
	.long	0xfffe33eb, 0x20000000		# f3 fffe33eb20000000
	.long	0x00002000			# msr 00002000
	.long	0x00011073			# fpscr 00011073
#
	.long	0x7ff034c7, 0xa28c5c36		# f29 7ff034c7a28c5c36
	.long	0x0000a000			# msr 0000a000
	.long	0x00020021			# fpscr 00020021
	.long	0x7ff834c7, 0xa0000000		# f26 7ff834c7a0000000
	.long	0x0000a000			# msr 0000a000
	.long	0xa1011021			# fpscr a1011021
#
	.long	0xb69f1e4e, 0x7e12d48b		# f27 b69f1e4e7e12d48b
	.long	0x00002000			# msr 00002000
	.long	0x00060082			# fpscr 00060082
	.long	0x80000000, 0x00000000		# f18 8000000000000000
	.long	0x00002000			# msr 00002000
	.long	0x8a032082			# fpscr 8a032082
#
	.long	0xb69fff8c, 0xc6d6c4ef		# f14 b69fff8cc6d6c4ef
	.long	0x0000a000			# msr 0000a000
	.long	0x00040023			# fpscr 00040023
	.long	0xc29fff8c, 0xe0000000		# f19 c29fff8ce0000000
	.long	0x0000a000			# msr 0000a000
	.long	0xca068023			# fpscr ca068023
#
	.long	0xffde1a61, 0x73f2c712		# f1 ffde1a6173f2c712
	.long	0x0000a000			# msr 0000a000
	.long	0x00040091			# fpscr 00040091
	.long	0xc7efffff, 0xe0000000		# f1 c7efffffe0000000
	.long	0x0000a000			# msr 0000a000
	.long	0x92028091			# fpscr 92028091
#
	.long	0xffdfe679, 0x16ac90fe		# f7 ffdfe67916ac90fe
	.long	0x00002000			# msr 00002000
	.long	0x00000018			# fpscr 00000018
	.long	0xfff00000, 0x00000000		# f6 fff0000000000000
	.long	0x00002000			# msr 00002000
	.long	0xd2069018			# fpscr d2069018
#
	.long	0x7feffffc, 0x9ff74fe3		# f18 7feffffc9ff74fe3
	.long	0x0000a000			# msr 0000a000
	.long	0x00020032			# fpscr 00020032
	.long	0x7ff00000, 0x00000000		# f20 7ff0000000000000
	.long	0x0000a000			# msr 0000a000
	.long	0x92065032			# fpscr 92065032
#
	.long	0xffd00000, 0x00000000		# f27 ffd0000000000000
	.long	0x00002000			# msr 00002000
	.long	0x00060081			# fpscr 00060081
	.long	0xc7efffff, 0xe0000000		# f8 c7efffffe0000000
	.long	0x00002000			# msr 00002000
	.long	0x92028081			# fpscr 92028081
#
	.long	0xb6afffff, 0xffefacc2		# f15 b6afffffffefacc2
	.long	0x00002000			# msr 00002000
	.long	0x0000008a			# fpscr 0000008a
	.long	0xb6a00000, 0x00000000		# f11 b6a0000000000000
	.long	0x00002000			# msr 00002000
	.long	0xca03808a			# fpscr ca03808a
#
	.long	0x373ffff6, 0x09ffcd78		# f9 373ffff609ffcd78
	.long	0x00002000			# msr 00002000
	.long	0x00000083			# fpscr 00000083
	.long	0x373ff800, 0x00000000		# f13 373ff80000000000
	.long	0x00002000			# msr 00002000
	.long	0x8a034083			# fpscr 8a034083
#
fpsave:	.long	0xffffffff, 0xffffffff
#
	.toc
	TOCE(.frsp, entry)
	TOCL(_frsp, data)
