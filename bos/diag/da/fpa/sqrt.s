# @(#)83	1.2  src/bos/diag/da/fpa/sqrt.s, dafpa, bos411, 9428A410j 8/5/93 14:38:43
#
#   COMPONENT_NAME: DAFPA
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#   Description :
#		Appropriate registers are loaded with input values. Then
#		instructions for this module will be executed and results
#		will be compared with expected results. If any test fails,
#		a particular return code will be returned to the caller.
#

.machine "pwrx"
	.csect	.sqrt[PR]
	.globl	.sqrt[PR]
	.align	2
.sqrt:
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
	LTOC(4, _sqrt, data)		# Loads g4 with the address of _sqrt
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
 	.using	_sqrt[RW],g4		# g4 is the base register to the data
#	 				     section _sqrt[rw]. Eastablishing 
#					     @ability
#
#	
	doz	g3,g3,g3		# g3 = 0. Initialize return code
#
#				# Loading floating pt registers for computation
#
	st	g4,fpsave
	l	g5,fpsave		# g5 gets pointer to TOC
#
	doz	g9,g9,g9		# g9 = 0
	ai	g9,g9,30
#
fst:
	lfd	rf6,0x000c(g5)		# Tricky! Only second 32 bits are copied
	mtfsf	0xff,rf6		# load fpscr (input)
#
	lfd	rf0,0x0008(g5)		# preload result to avoid NaN op.
#
	lfd	rf1,0x0000(g5)
#
#  Instructions used for testing
# 
	fsqrt	r0,r1
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
#
HERE:	ai	g3,g3,1			
	stfd	rf0,fpsave
	l	g6,fpsave
	l	g7,0x0008(g5)
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g3,g3,1			
	l	g6,fpsave+4
	l	g7,0x000c(g5)
	cmp	0,g6,g7
	bne	0,FINI
#
	ai	g5,g5,0x0014		# g5 now points to data of new
	ai	g9,g9,-1		# g9 -= 1
	cmpi	1,g9,0
	bne	1,fst
#
	doz	g3,g3,g3		# Return code = 0; pass
#
FINI:	br
#
# Data Area
#
	.csect	_sqrt[RW],3
_sqrt: 
# 1
	.long 	0x40006000, 0x00000000		# operand
	.long	0x3FF6E41B, 0x9BFB3B75		# good result
# 	.long	0x3FF6E41B, 0x9BFB3B74		# fake result - to test code
	.long	0x82064000			# fpscr
# 2
	.long 	0x4000C000, 0x00000000		# operand
	.long	0x3FF726D4, 0x1832A0BE		# result
	.long	0x82064000			# fpscr
# 3	 
	.long 	0x40018000, 0x00000000		# operand
	.long	0x3FF7AA10, 0xD193C22D		# result
	.long	0x82064000			# fpscr
# 4	 
	.long 	0x4001C000, 0x00000000		# operand
	.long	0x3FF7D52F, 0x244809E9		# result
	.long	0x82064000			# fpscr
# 5	 
	.long 	0x4001E000, 0x00000000		# operand
	.long	0x3FF7EAA1, 0x26F15284		# result
	.long	0x82064000			# fpscr
# 6	 
	.long 	0x40020000, 0x00000000		# operand
	.long	0x3FF80000, 0x00000000		# result
	.long	0x82064000			# fpscr
# 7	 
	.long 	0x40020000, 0x00000000		# operand
	.long	0x3FF80000, 0x00000000		# result
	.long	0x82064000			# fpscr
# 8	 
	.long 	0x40024000, 0x00000000		# operand
	.long	0x3FF82A85, 0x00794E6C		# result
	.long	0x82064000			# fpscr
# 9	 
	.long 	0x40026000, 0x00000000		# operand
	.long	0x3FF83FAB, 0x8B4D4315		# result
	.long	0x82064000			# fpscr
# 10	 
	.long 	0x4002A000, 0x00000000		# operand
	.long	0x3FF869C1, 0xA85CC346		# result
	.long	0x82064000			# fpscr
# 11	 
	.long 	0x4002E000, 0x00000000		# operand
	.long	0x3FF8938F, 0xB37BC9C1		# result
	.long	0x82064000			# fpscr
# 12 
	.long 	0x40032000, 0x00000000		# operand
	.long	0x3FF8BD17, 0x1A07E38A		# result
	.long	0x82064000			# fpscr
# 13	 
	.long 	0x40034000, 0x00000000		# operand
	.long	0x3FF8D1C0, 0xBE7F20AC		# result
	.long	0x82064000			# fpscr
# 14	 
	.long 	0x40038000, 0x00000000		# operand
	.long	0x3FF8FAE0, 0xC15AD38A		# result
	.long	0x82064000			# fpscr
# 15	 
	.long 	0x4003C000, 0x00000000		# operand
	.long	0x3FF923BD, 0x7E25164D		# result
	.long	0x82064000			# fpscr
# 16	 
	.long 	0x4003E000, 0x00000000		# operand
	.long	0x3FF93813, 0x088978C5		# result
	.long	0x82064000			# fpscr
# 17	 
	.long 	0x40003FFF, 0xFFFFFFFF		# operand
	.long	0x3FF6CDB2, 0xBBB212EB		# result
	.long	0x82024000			# fpscr
# 18	 
	.long 	0x40005FFF, 0xFFFFFFFF		# operand
	.long	0x3FF6E41B, 0x9BFB3B74		# result
	.long	0x82024000			# fpscr
# 19	 
	.long 	0x40007FFF, 0xFFFFFFFF		# operand
	.long	0x3FF6FA6E, 0xA162D0EF		# result
	.long	0x82024000			# fpscr
# 20	 
	.long 	0x4000BFFF, 0xFFFFFFFF		# operand
	.long	0x3FF726D4, 0x1832A0BD		# result
	.long	0x82024000			# fpscr
# 21	 
	.long 	0x4000DFFF, 0xFFFFFFFF		# operand
	.long	0x3FF73CE7, 0x04FB7B23		# result
	.long	0x82024000			# fpscr
# 22	 
	.long 	0x40015FFF, 0xFFFFFFF0		# operand
	.long	0x3FF79464, 0x16EBC562		# result
	.long	0x82024000			# fpscr
# 23	 
	.long 	0x40017FFF, 0xFFFFFFF0		# operand
	.long	0x3FF7AA10, 0xD193C222		# result
	.long	0x82024000			# fpscr
# 24	 
	.long 	0x4001FFFF, 0xFFFFFFF0		# operand
	.long	0x3FF7FFFF, 0xFFFFFFF5		# result
	.long	0x82024000			# fpscr
# 25	 
	.long 	0x40021FFF, 0xFFFFFFF0		# operand
	.long	0x3FF8154B, 0xE277351B		# result
	.long	0x82024000			# fpscr
# 26	 
	.long 	0x4002BFFF, 0xFFFFFFF0		# operand
	.long	0x3FF87EB1, 0x990B6970		# result
	.long	0x82024000			# fpscr
# 27	 
	.long 	0x4002DFFF, 0xFFFFFFF0		# operand
	.long	0x3FF8938F, 0xB37BC9B6		# result
	.long	0x82024000			# fpscr
# 28	 
	.long 	0x40031FFF, 0xFFFFFFF0		# operand
	.long	0x3FF8BD17, 0x1A07E380		# result
	.long	0x82024000			# fpscr
# 29	 
	.long 	0x40035FFF, 0xFFFFFFF0		# operand
	.long	0x3FF8E659, 0x3D77B0AE		# result
	.long	0x82024000			# fpscr
# 30	 
	.long 	0x4003BFFF, 0xFFFFFFF0		# operand
	.long	0x3FF923BD, 0x7E251643		# result
	.long	0x82024000			# fpscr
# 31	 
	.long 	0x4003DFFF, 0xFFFFFFF0		# operand
	.long	0x3FF93813, 0x088978BB		# result
	.long	0x82024000			# fpscr
#
#
fpsave:	.long	0xffffffff, 0xffffffff
#
	.toc
	TOCE(.sqrt, entry)
	TOCL(_sqrt, data)
