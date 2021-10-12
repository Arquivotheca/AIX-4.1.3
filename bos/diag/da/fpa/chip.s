# @(#)16	1.3  src/bos/diag/da/fpa/chip.s, dafpa, bos411, 9428A410j 3/23/94 06:20:13

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
	.csect	.chip[PR]
	.globl	.chip[PR]
	.align	2
.chip:
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
	LTOC(4, _chip, data)		# Loads g4 with the address of _chip
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
 	.using	_chip[RW],g4		# g4 is the base register to the data
#	 				     section _chip[rw]. Eastablishing 
#					     @ability
#
#	
	xor	g3,g3,g3		# g3 = 0. Initialize return code
	ai	g3,g3,1			# Preset return code = 1; fail
#
#				# Loading floating pt registers for computation
#
	lfd	rf0,i1f0
	lfd	rf1,i1f1
#
	lfd	rf4,i1fps
	mtfsf	0xff,rf4
	l	g6,i1cr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
	 fma.    r0,r0,r0,r1                      
	 fmr.    r2,r0                            
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	mfcr	g5			# Check condition register
	l	g6,o1cr
	cmp	0,g5,g6
	bne	0,FINI
#
#					# Check fpscr output 
	ai	g3,g3,1			# Preset return code = 1; fail
	mffs	rf7
	stfd	rf7,here
	l	g5,here+4
	l	g6,o1fps
	cmp	0,g5,g6
	bne	0,FINI
#
	ai	g3,g3,1
	lfd	rf3,o1f0
	fcmpu	3,rf0,rf3
	bne	3,FINI	
#
	ai	g3,g3,1			# Preset return code = 1; fail
	lfd	rf3,o1f2
	fcmpu	3,rf2,rf3
	bne	3,FINI	
#					
	lfd	rf3,i2f3
	lfd	rf4,i2f4
#
	lfd	rf7,i2fps
	mtfsf	0xff,rf7
	l	g6,i2cr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
	fma.    r3,r3,r3,r4                      
	fmr.    r5,r3                            
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
#
	ai	g3,g3,1			# Preset return code = 1; fail
	mfcr	g5			# Check condition register
	l	g6,o2cr
	cmp	0,g5,g6
	bne	0,FINI
#					# Check fpscr output 
	ai	g3,g3,1			# Preset return code = 1; fail
	mffs	rf7
	stfd	rf7,here
	l	g5,here+4
	l	g6,o2fps
	cmp	0,g5,g6
	bne	0,FINI
#
	ai	g3,g3,1			# Preset return code = 1; fail
	lfd	rf1,o2f3
	fcmpu	3,rf3,rf1
	bne	3,FINI	
#
	ai	g3,g3,1			# Preset return code = 1; fail
	lfd	rf1,o2f5
	fcmpu	3,rf5,rf1
	bne	3,FINI	
#	
	xor	g3,g3,g3		# Return code = 0; pass
#
FINI:	br	
#
# Data Area
#
	.csect	_chip[RW],3
_chip: 
#
here:	.long	0xffffffff
	.long	0xffffffff
i1f0:	.long	0x3fffffff
	.long	0xffc00000                          
i1f1:	.long	0xc00fffff
	.long	0xff800000                          
i1cr:	.long	0x73a6ebe0                                          
i1fps:	.long	0xffffffff
	.long	0xf7979080                                          
#results                                                     
o1f0:	.long	0x3c300000
	.long	0x00000000                          
o1f2:	.long	0x3c300000
	.long	0x00000000                          
o1cr:	.long	0x7fa6ebe0                                          
o1fps:	.long	0xf7904080                                          
#
i2f3:	.long	0x3fffffff
	.long	0xffe00000                          
i2f4:	.long	0xc00fffff
	.long	0xffc00000                          
i2cr:	.long	0xc9b5ad26                                          
i2fps:	.long	0xffffffff
	.long	0x6f1980d1                                          
#results                                                     
o2f3:	.long	0x3c100000
	.long	0x00000000                          
o2f5:	.long	0x3c100000
	.long	0x00000000                          
o2cr:	.long	0xc6b5ad26                                          
o2fps:	.long	0x6f1840d1                                          
#
	.toc
	TOCE(.chip, entry)
	TOCL(_chip, data)
