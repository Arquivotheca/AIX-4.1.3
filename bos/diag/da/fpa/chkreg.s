# @(#)26	1.3  src/bos/diag/da/fpa/chkreg.s, dafpa, bos411, 9428A410j 3/23/94 06:20:25

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
#	Testing floating point unit. The return code, which is saved in
#		general purpose register g3, is the error code. Return
#		code = 0 means pass, 1 means fail.
#
#	This program will test all floating point registers to see if any
#		of the registers has a stuck bit of 1 or 0.
#
#	Also, the FPSCR (Floating Point Status and Condition Register) will
#		be tested for SA1 and SA0 error.
# 
# 	Defining symbols for general and floating point registers
#	g0 - g31	:	32 general purpose registers
#	rf0 - rf31	:	32 floating point registers
#
#
	.csect	.chkreg[PR]
	.globl	.chkreg[PR]
	.align	2
.chkreg:
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
#			Defining different bit patterns for testing stuck bits
#
	.set	ALLZEROS,0x0000			# '00000000'
	.set	ALT0,0x5555			# '01010101'
	.set	ALT1,0x3333			# '00110011'
#
#
	LTOC(4, _chkreg, data)		# Loads g4 with the address of _chkreg
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
 	.using	_chkreg[RW],g4		# g4 is the base register to the data
#	 				     section _chkreg[rw]. Eastablishing 
#					     @ability
#
#
	xor	g9,g9,g9		# Initializes g9 to 0, bit pattern count
	ai	g9,g9,4			# 4 different bit patterns to test
	xor	g10,g10,g10		# Initializes g10 to 0. Stores different
#					     bit patterns 
 	xor	g3,g3,g3		# g3 = 0
	ai	g3,g3,1			# Return code = 1; fail; pessimistic
#
#				Loading all floating point registers with bit
#				pattern stored in OOR1
#
CLOOP:  lfd	rf0,OOR1 
  	lfd	rf1,OOR1 
  	lfd	rf2,OOR1 
  	lfd	rf3,OOR1 
  	lfd	rf4,OOR1 
  	lfd	rf5,OOR1 
  	lfd	rf6,OOR1 
  	lfd	rf7,OOR1 
  	lfd	rf8,OOR1 
  	lfd	rf9,OOR1 
  	lfd	rf10,OOR1 
  	lfd	rf11,OOR1 
  	lfd	rf12,OOR1 
  	lfd	rf13,OOR1 
  	lfd	rf14,OOR1 
  	lfd	rf15,OOR1 
  	lfd	rf16,OOR1 
  	lfd	rf17,OOR1 
  	lfd	rf18,OOR1 
  	lfd	rf19,OOR1 
  	lfd	rf20,OOR1 
  	lfd	rf21,OOR1 
  	lfd	rf22,OOR1 
  	lfd	rf23,OOR1 
  	lfd	rf24,OOR1 
  	lfd	rf25,OOR1 
  	lfd	rf26,OOR1 
  	lfd	rf27,OOR1 
  	lfd	rf28,OOR1 
  	lfd	rf29,OOR1 
  	lfd	rf30,OOR1 
  	lfd	rf31,OOR1 
#
#				Store contents of each floating point register
#				starting with rf0 at SAREA, rf1 at SAREA+8 and
#				so on.
#
	stfd	rf0,SAREA
	stfd	rf1,SAREA+8
	stfd	rf2,SAREA+16
	stfd	rf3,SAREA+24
	stfd	rf4,SAREA+32
	stfd	rf5,SAREA+40
	stfd	rf6,SAREA+48
	stfd	rf7,SAREA+56
	stfd	rf8,SAREA+64
	stfd	rf9,SAREA+72
	stfd	rf10,SAREA+80
	stfd	rf11,SAREA+88
	stfd	rf12,SAREA+96
	stfd	rf13,SAREA+104
	stfd	rf14,SAREA+112
	stfd	rf15,SAREA+120
	stfd	rf16,SAREA+128
	stfd	rf17,SAREA+136
	stfd	rf18,SAREA+144
	stfd	rf19,SAREA+152
	stfd	rf20,SAREA+160
	stfd	rf21,SAREA+168
	stfd	rf22,SAREA+176
	stfd	rf23,SAREA+184
	stfd	rf24,SAREA+192
	stfd	rf25,SAREA+200
	stfd	rf26,SAREA+208
	stfd	rf27,SAREA+216
	stfd	rf28,SAREA+224
	stfd	rf29,SAREA+232
	stfd	rf30,SAREA+240
	stfd	rf31,SAREA+248
#
	xor	g5,g5,g5		# g5 = 0
	ai 	g5,g5,32		# g5 = 32 (loop counter, no. of regs)
	xor	g8,g8,g8		# g8 = 0
	a	g8,g8,g4         	# g8 = g4 (g8 points to @_chkreg[RW])
#
	l	g6,OOR1			# g6 gets upper 4 bytes of OOR1
	l	g7,OOR1+4		# g7 gets lower 4 bytes of OOR1
# 
TLOOP:	l	g11,SAREA(g8)		# Load upper 4 bytes for comparison
	cmp	0,g6,g11		# Comparing values in SAREA with
#					     original value used in loading the
#					     registers
	bne	FINI			# Exit if not equal
	l	g11,SAREA+4(g8)		# Load next 4 bytes
	cmp	0,g7,g11		# Compare next 4 bytes
	bne	FINI	
	ai	g8,g8,8			# Set up next number to be compared
	si	g5,g5,1			# g5 -= 1 (1 less register to test)
	bnz	TLOOP			# Continue until all regs are tested
#
	xor	g10,g10,g10		# Clears g10 to store new bit pattern
	si	g9,g9,1			# g9 -= 1 (1 less bit pattern to test)
	cmpi	0,g9,3			# Has the second pattern been tested ?
	bne	NEXT1   		# If yes, check third pattern at NEXT1
	ai	g10,g10,ALLZEROS	# Load second pattern 0x00000000
	st	g10,OOR1		# Store pattern in @OOR1
	st	g10,OOR1+4
	b	CLOOP			# Branch to Control loop
#
NEXT1:	cmpi	0,g9,2			# Has the third pattern been tested ?
	bne	NEXT2			# If yes, check fourth pattern at NEXT2
	ai 	g10,g10,ALT0		# Load third pattern 0x01010101 
	st	g10,OOR1		# Store pattern in @OOR1
	st	g10,OOR1+4
	b	CLOOP			# Branch to Control loop
#
NEXT2:	cmpi	0,g9,1			# Has the fourth pattern been tested ?
	bne	EXIT			# If yes, exit
	ai	g10,g10,ALT1		# Load fourth pattern, 0x10101010
	st	g10,OOR1		# Store pattern in @OOR1
	st	g10,OOR1+4
	b	CLOOP			# Branch to Control loop
#
EXIT:	xor	g3,g3,g3		# Return code = 0, passes all tests
# 
# 
FINI:	br				# Return to caller. Return code in g3
#
#				# Data area
	.csect _chkreg[RW]
_chkreg:
OOR1:	.long	0xFFFFFFFF		# One's or zero's. Bit pattern storage 
	.long	0xFFFFFFFF
SAREA:	.space	256			# Storage area for contents of all
#					  floating point registers (32*8) bytes
#
	.toc
	TOCE(.chkreg, entry)
	TOCL(_chkreg, data)

