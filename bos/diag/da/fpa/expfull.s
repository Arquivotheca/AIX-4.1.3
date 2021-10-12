# @(#)18	1.3  src/bos/diag/da/fpa/expfull.s, dafpa, bos411, 9428A410j 3/23/94 06:22:58

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
#	First module to test the Exponent subunit.
#		Appropriate registers are loaded with input values. Then
#		instructions for this module will be executed and results
#		will be compared with expected results. If any test fails,
#		a particular return code will be returned to the caller.
#
	.csect	.expfull[PR]
	.globl	.expfull[PR]
	.align	2
.expfull:
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
	LTOC(4, _expfull, data)		# Loads g4 with the address of _expfull
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
 	.using	_expfull[RW],g4		# g4 is the base register to the data
#	 				     section _expfull[rw]. Eastablishing 
#					     @ability
#
#	
	xor	g3,g3,g3		# g3 = 0. Initialize return code
	ai	g3,g3,1			# Preset return code = 1; fail
#
#				# Loading floating pt registers for computation
#
	lfd	rf3,e1if3
	lfd	rf4,e1if4
	lfd	rf5,e1if5
	lfd	rf6,e1if6
	lfd	rf10,e1if10
	lfd	rf12,e1if12
	lfd	rf17,e1if17
	lfd	rf18,e1if18
	lfd	rf19,e1if19
	lfd	rf20,e1if20
	lfd	rf23,e1if23
	lfd	rf27,e1if27
	lfd	rf29,e1if29
	lfd	rf30,e1if30
#
	lfd	rf1,e1fps
	mtfsf	0xff,rf1
	l	g6,e1icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
	fd.      rf21,rf12,rf23
	fnma.   rf2,rf21,rf6,rf21
	fm      rf22,rf4,rf2
	fnms.   rf16,rf12,rf22,rf17
	fs.      rf11,rf5,rf22
	fnma    rf13,rf18,rf11,rf18
	fs       rf17,rf11,rf30
	fnms    rf25,rf13,rf17,rf18
	fnms.   rf3,rf17,rf25,rf3
	fnma.   rf3,rf30,rf25,rf29
	fs       rf3,rf27,rf3
	fma.    rf3,rf20,rf30,rf22
	fs.      rf15,rf3,rf10
	fd.      rf21,rf25,rf15
	fm.     rf14,rf21,rf12
	fd       rf30,rf19,rf21
	fd       rf21,rf14,rf14
	fnma    rf22,rf30,rf21,rf10
	fs.      rf8,rf6,rf21
	fnms.   rf6,rf8,rf22,rf6
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	lfd	rf1,e1of2		# Load expected result of rf2 in rf1
	fcmpu	3,rf1,rf2		# Compare results
	bne	3,FINI			# Exit if not equal
#
	ai	g3,g3,1			# RC = 2
	lfd	rf1,e1of3		# Check rf3
	fcmpu	3,rf1,rf3
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 3
	stfd	rf6,here		# Check rf6
	l	g7,here
	l	g8,e1of6
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e1of6+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 4
	stfd	rf8,here		# Check rf8
	l	g7,here
	l	g8,e1of8
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e1of8+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 5
	lfd	rf1,e1of11		# Check rf11
	fcmpu	3,rf1,rf11
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 6
	lfd	rf1,e1of13		# Check rf13
	fcmpu	3,rf1,rf13
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 7
	lfd	rf1,e1of14		# Check rf14
	fcmpu	3,rf1,rf14
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 8
	lfd	rf1,e1of15		# Check rf15
	fcmpu	3,rf1,rf15
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 9
	lfd	rf1,e1of16		# Check rf16
	fcmpu	3,rf1,rf16
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 10
	lfd	rf1,e1of17		# Check rf17
	fcmpu	3,rf1,rf17
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 11
	stfd	rf21,here		# Check rf21
	l	g7,here
	l	g8,e1of21
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e1of21+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 12
	stfd	rf22,here		# Check rf22
	l	g7,here
	l	g8,e1of22
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e1of22+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 13
	lfd	rf1,e1of25		# Check rf25
	fcmpu	3,rf1,rf25
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 14
	stfd	rf30,here		# Check rf30
	l	g7,here
	l	g8,e1of30
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e1of30+4
	cmp	0,g7,g8
	bne	0,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf0,e2if0
	lfd	rf1,e2if1
	lfd	rf3,e2if3
	lfd	rf5,e2if5
	lfd	rf8,e2if8
	lfd	rf9,e2if9
	lfd	rf10,e2if10
	lfd	rf13,e2if13
	lfd	rf14,e2if14
	lfd	rf15,e2if15
	lfd	rf16,e2if16
	lfd	rf17,e2if17
	lfd	rf21,e2if21
	lfd	rf31,e2if31
#
	lfd	rf2,e2fps
	mtfsf	0xff,rf2
	l	g6,e2icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
	fnms.   rf2,rf16,rf14,rf1    
	fa.      rf19,rf31,rf2     
	fs       rf31,rf2,rf2     
	fm.     rf18,rf19,rf15   
	fa       rf6,rf17,rf18  
	fnms    rf17,rf14,rf18,rf6                
	fms     rf22,rf8,rf13,rf17                
	fnma    rf16,rf22,rf22,rf22               
	fnma    rf25,rf9,rf16,rf1                 
	fnma.   rf24,rf25,rf16,rf25               
	fma     rf20,rf25,rf15,rf13               
	fnms.   rf16,rf20,rf8,rf10               
	fm      rf25,rf10,rf20                 
	fm.     rf25,rf10,rf17                
	fa       rf26,rf25,rf25              
	fms.    rf18,rf25,rf21,rf5           
	fm      rf19,rf26,rf3              
	fm.     rf18,rf21,rf19            
	fm      rf19,rf14,rf10           
	fs.      rf29,rf18,rf0          
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1			# RC = 15 
	lfd	rf1,e2of2		# Load expected result of rf2 in rf1
	fcmpu	3,rf1,rf2		# Compare results
	bne	3,FINI			# Exit if not equal
#
	ai	g3,g3,1			# RC = 16
	lfd	rf1,e2of6		# Check rf6
	fcmpu	3,rf1,rf6
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 17
	lfd	rf1,e2of16		# Check rf16
	fcmpu	3,rf1,rf16
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 18
	lfd	rf1,e2of17		# Check rf17
	fcmpu	3,rf1,rf17
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 19
	lfd	rf1,e2of18		# Check rf18
	fcmpu	3,rf1,rf18
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 20
	lfd	rf1,e2of19		# Check rf19
	fcmpu	3,rf1,rf19
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 21
	lfd	rf1,e2of20		# Check rf20
	fcmpu	3,rf1,rf20
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 22
	lfd	rf1,e2of22		# Check rf22
	fcmpu	3,rf1,rf22
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 23
	lfd	rf1,e2of24		# Check rf24
	fcmpu	3,rf1,rf24
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 24
	lfd	rf1,e2of25		# Check rf25
	fcmpu	3,rf1,rf25
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 25
	lfd	rf1,e2of26		# Check rf26
	fcmpu	3,rf1,rf26
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 26
	lfd	rf1,e2of29		# Check rf29
	fcmpu	3,rf1,rf29
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 27
	lfd	rf1,e2of31		# Check rf31
	fcmpu	3,rf1,rf31
	bne	3,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf0,e3if0
	lfd	rf2,e3if2
	lfd	rf4,e3if4
	lfd	rf7,e3if7
	lfd	rf8,e3if8
	lfd	rf9,e3if9
	lfd	rf12,e3if12
	lfd	rf14,e3if14
	lfd	rf15,e3if15
	lfd	rf20,e3if20
	lfd	rf23,e3if23
	lfd	rf24,e3if24
	lfd	rf25,e3if25
	lfd	rf27,e3if27
	lfd	rf30,e3if30
#
	lfd	rf1,e3fps
	mtfsf	0xff,rf1
	l	g6,e3icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
	fnms    rf3,rf14,rf8,rf8  
	fma.    rf6,rf12,rf24,rf3                              
	fnms.   rf1,rf25,rf6,rf3                               
	fnma    rf11,rf1,rf0,rf0                               
	fma     rf24,rf1,rf11,rf23                             
	frsp.   rf31,rf24                                    
	fnms.   rf17,rf9,rf24,rf1                              
	fd       rf31,rf9,rf17                                
	fnma    rf17,rf25,rf31,rf30                            
	fnma    rf9,rf2,rf31,rf20                              
	fnms    rf31,rf27,rf12,rf17                            
	fnma    rf24,rf0,rf31,rf4                              
	fms     rf24,rf30,rf3,rf25                             
	fnma    rf4,rf15,rf6,rf24                              
	fms     rf20,rf15,rf4,rf9                              
	fnma.   rf21,rf4,rf20,rf31                             
	frsp    rf16,rf21                                    
	fs       rf1,rf7,rf21                                 
	fs       rf27,rf12,rf1                                
	fd.      rf28,rf20,rf27                               
#
# 
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1			# RC = 28 
	lfd	rf2,e3of1		# Load expected result of rf2 in rf1
	fcmpu	3,rf2,rf1		# Compare results
	bne	3,FINI			# Exit if not equal
#
	ai	g3,g3,1			# RC = 29
	lfd	rf1,e3of3		# Check rf3
	fcmpu	3,rf1,rf3
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 30
	lfd	rf1,e3of4		# Check rf4
	fcmpu	3,rf1,rf4
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 31
	lfd	rf1,e3of6		# Check rf6
	fcmpu	3,rf1,rf6
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 32
	lfd	rf1,e3of9		# Check rf9
	fcmpu	3,rf1,rf9
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 33
	lfd	rf1,e3of11		# Check rf11
	fcmpu	3,rf1,rf11
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 34
	lfd	rf1,e3of16		# Check rf16
	fcmpu	3,rf1,rf16
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 35
	lfd	rf1,e3of17		# Check rf17
	fcmpu	3,rf1,rf17
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 36
	lfd	rf1,e3of20		# Check rf20
	fcmpu	3,rf1,rf20
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 37
	lfd	rf1,e3of21		# Check rf21
	fcmpu	3,rf1,rf21
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 38
	lfd	rf1,e3of24		# Check rf24
	fcmpu	3,rf1,rf24
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 39
	lfd	rf1,e3of27		# Check rf27
	fcmpu	3,rf1,rf27
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 40
	lfd	rf1,e3of28		# Check rf28
	fcmpu	3,rf1,rf28
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 41
	lfd	rf1,e3of31		# Check rf31
	fcmpu	3,rf1,rf31
	bne	3,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf1,e4if1
	lfd	rf2,e4if2
	lfd	rf8,e4if8
	lfd	rf9,e4if9
	lfd	rf11,e4if11
	lfd	rf15,e4if15
	lfd	rf16,e4if16
	lfd	rf17,e4if17
	lfd	rf18,e4if18
	lfd	rf23,e4if23
	lfd	rf25,e4if25
	lfd	rf26,e4if26
#
	lfd	rf3,e4fps
	mtfsf	0xff,rf3
	l	g6,e4icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
	fs       rf5,rf15,rf8                                 
	fms     rf19,rf2,rf16,rf5                              
	fnma    rf6,rf19,rf5,rf5                               
	fa.      rf20,rf1,rf19                                
	fma.    rf14,rf20,rf20,rf20                            
	fs       rf4,rf18,rf20                                
	frsp.   rf4,rf14                                     
	fd       rf18,rf4,rf4                                 
	fa       rf7,rf23,rf18                                
	fa.      rf13,rf11,rf18                               
	fma.    rf31,rf1,rf13,rf13                             
	fma     rf29,rf7,rf8,rf31                              
	fs       rf18,rf26,rf29                               
	fs.      rf12,rf18,rf18                               
	fma     rf4,rf9,rf18,rf2                               
	fm.     rf12,rf25,rf18                                
	fm      rf12,rf1,rf7                                  
	fd       rf27,rf14,rf12                               
	frsp.   rf9,rf27                                     
	fma.    rf27,rf9,rf17,rf9                              
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1			# RC = 42 
	stfd	rf4,here		# Check rf4
	l	g7,here
	l	g8,e4of4
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e4of4+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 43
	lfd	rf1,e4of5		# Check rf5
	fcmpu	3,rf1,rf5
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 44
	stfd	rf6,here		# Check rf6
	l	g7,here
	l	g8,e4of6
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e4of6+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 45
	stfd	rf7,here		# Check rf7
	l	g7,here
	l	g8,e4of7
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e4of7+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 46
	stfd	rf9,here		# Check rf9
	l	g7,here
	l	g8,e4of9
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e4of9+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 47
	stfd	rf12,here		# Check rf12
	l	g7,here
	l	g8,e4of12
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e4of12+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 48
	stfd	rf13,here		# Check rf13
	l	g7,here
	l	g8,e4of13
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e4of13+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 49
	stfd	rf14,here		# Check rf14
	l	g7,here
	l	g8,e4of14
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e4of14+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 50
	stfd	rf18,here		# Check rf18
	l	g7,here
	l	g8,e4of18
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e4of18+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 51
	lfd	rf1,e4of19		# Check rf19
	fcmpu	3,rf1,rf19
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 52
	lfd	rf1,e4of20		# Check rf20
	fcmpu	3,rf1,rf20
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 53 
	stfd	rf27,here		# Check rf27
	l	g7,here
	l	g8,e4of27
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e4of27+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 54
	stfd	rf29,here		# Check rf29
	l	g7,here
	l	g8,e4of29
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e4of29+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 55
	stfd	rf31,here		# Check rf31
	l	g7,here
	l	g8,e4of31
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e4of31+4
	cmp	0,g7,g8
	bne	0,FINI
#
#	** E5 **		# Loading floating pt registers for computation
#
	lfd	rf0,e5if0
	lfd	rf5,e5if5
	lfd	rf8,e5if8
	lfd	rf9,e5if9
	lfd	rf10,e5if10
	lfd	rf12,e5if12
	lfd	rf14,e5if14
	lfd	rf17,e5if17
	lfd	rf19,e5if19
	lfd	rf20,e5if20
	lfd	rf21,e5if21
	lfd	rf23,e5if23
	lfd	rf24,e5if24
	lfd	rf26,e5if26
	lfd	rf28,e5if28
	lfd	rf31,e5if31
#
	lfd	rf1,e5fps
	mtfsf	0xff,rf1
	l	g6,e5icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
	fma     rf7,rf31,rf5,rf23                              
	fa.      rf11,rf7,rf9                                 
	fma     rf13,rf11,rf23,rf10                            
	fs       rf25,rf17,rf11                               
	fm      rf11,rf25,rf14                                
	frsp    rf18,rf25                                    
	fm.     rf31,rf18,rf18                                
	fnms.   rf18,rf19,rf18,rf25                            
	fnms.   rf31,rf18,rf31,rf21                            
	fnma.   rf2,rf0,rf31,rf31                              
	fm      rf0,rf31,rf28                                 
	fma.    rf17,rf2,rf2,rf24                              
	fnma    rf21,rf0,rf2,rf0                               
	fnms.   rf3,rf17,rf17,rf25                             
	fms     rf5,rf21,rf19,rf8                              
	fma.    rf30,rf3,rf26,rf2                              
	fma.    rf22,rf13,rf12,rf5                             
	fnma.   rf30,rf23,rf20,rf20                            
	fnma.   rf9,rf30,rf0,rf26                              
	fa.      rf19,rf2,rf30                                
#
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1			# RC = 56 
	stfd	rf0,here		# Check rf0
	l	g7,here
	l	g8,e5of0
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e5of0+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 57
	stfd	rf2,here		# Check rf2
	l	g7,here
	l	g8,e5of2
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e5of2+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 58
	stfd	rf3,here		# Check rf3
	l	g7,here
	l	g8,e5of3
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e5of3+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 59
	stfd	rf5,here		# Check rf5
	l	g7,here
	l	g8,e5of5
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e5of5+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 60
	lfd	rf1,e5of7		# Check rf7
	fcmpu	3,rf1,rf7
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 61
	stfd	rf9,here		# Check rf9
	l	g7,here
	l	g8,e5of9
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e5of9+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 62
	stfd	rf11,here		# Check rf11
	l	g7,here
	l	g8,e5of11
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e5of11+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 63
	stfd	rf13,here		# Check rf13
	l	g7,here
	l	g8,e5of13
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e5of13+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 64
	stfd	rf17,here		# Check rf17
	l	g7,here
	l	g8,e5of17
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e5of17+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 65
	stfd	rf18,here		# Check rf18
	l	g7,here
	l	g8,e5of18
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e5of18+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 66
	stfd	rf19,here		# Check rf19
	l	g7,here
	l	g8,e5of19
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e5of19+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 67
	stfd	rf21,here		# Check rf21
	l	g7,here
	l	g8,e5of21
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e5of21+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 68
	stfd	rf22,here		# Check rf22
	l	g7,here
	l	g8,e5of22
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e5of22+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 69
	stfd	rf25,here		# Check rf25
	l	g7,here
	l	g8,e5of25
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e5of25+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 70
	stfd	rf30,here		# Check rf30
	l	g7,here
	l	g8,e5of30
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e5of30+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 71
	stfd	rf31,here		# Check rf31
	l	g7,here
	l	g8,e5of31
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e5of31+4
	cmp	0,g7,g8
	bne	0,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf2,e6if2
	lfd	rf3,e6if3
	lfd	rf4,e6if4
	lfd	rf7,e6if7
	lfd	rf8,e6if8
	lfd	rf11,e6if11
	lfd	rf12,e6if12
	lfd	rf13,e6if13
	lfd	rf17,e6if17
	lfd	rf19,e6if19
	lfd	rf20,e6if20
	lfd	rf21,e6if21
	lfd	rf26,e6if26
#
	lfd	rf1,e6fps
	mtfsf	0xff,rf1
	l	g6,e6icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
	fs       rf6,r8,r26                                 
	fms.    rf15,r6,r6,r2                               
	fms     rf20,r20,r6,r20                             
	fnms    rf23,r17,r15,r20                            
	fnms.   rf24,r3,r13,r20                             
	frsp.   r8,r23                                     
	fm.     rf8,r23,r21                                 
	fs.      rf30,r8,r2                                 
	fd       rf18,r8,r12                                
	fd.      rf12,r30,r21                               
	fm.     rf20,r12,r17                                
	fnma    rf10,r12,r18,r12                            
	fd       rf20,r4,r7                                 
	fm      rf20,r11,r30                                
	fnma.   rf16,r21,r20,r19                            
	fnma    rf19,r16,r26,r20                            
	fma     rf30,r16,r3,r24                             
	fa       rf16,r13,r19                               
	fm.     rf18,r30,r30                                
	fd       rf22,r18,r16                               
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1			# RC = 72 
	stfd	rf6,here		# Check rf6
	l	g7,here
	l	g8,e6of6
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e6of6+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 73
	stfd	rf8,here		# Check rf8
	l	g7,here
	l	g8,e6of8
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e6of8+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 74
	stfd	rf10,here		# Check rf10
	l	g7,here
	l	g8,e6of10
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e6of10+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 75
	stfd	rf12,here		# Check rf12
	l	g7,here
	l	g8,e6of12
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e6of12+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 76
	stfd	rf15,here		# Check rf15
	l	g7,here
	l	g8,e6of15
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e6of15+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 77
	stfd	rf16,here		# Check rf16
	l	g7,here
	l	g8,e6of16
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e6of16+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 78
	stfd	rf18,here		# Check rf18
	l	g7,here
	l	g8,e6of18
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e6of18+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 79
	stfd	rf19,here		# Check rf19
	l	g7,here
	l	g8,e6of19
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e6of19+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 80
	stfd	rf20,here		# Check rf20
	l	g7,here
	l	g8,e6of20
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e6of20+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 81
	stfd	rf22,here		# Check rf22
	l	g7,here
	l	g8,e6of22
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e6of22+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 82
	stfd	rf23,here		# Check rf23
	l	g7,here
	l	g8,e6of23
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e6of23+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 83
	stfd	rf24,here		# Check rf24
	l	g7,here
	l	g8,e6of24
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e6of24+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 84
	stfd	rf30,here		# Check rf30
	l	g7,here
	l	g8,e6of30
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,e6of30+4
	cmp	0,g7,g8
	bne	0,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf0,e7if0
	lfd	rf1,e7if1
	lfd	rf3,e7if3
	lfd	rf5,e7if5
	lfd	rf6,e7if6
	lfd	rf10,e7if10
	lfd	rf12,e7if12
	lfd	rf15,e7if15
	lfd	rf19,e7if19
	lfd	rf21,e7if21
	lfd	rf24,e7if24
	lfd	rf27,e7if27
	lfd	rf28,e7if28
	lfd	rf29,e7if29
	lfd	rf31,e7if31
#
	lfd	rf2,e7fps
	mtfsf	0xff,rf2
	l	g6,e7icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
	fm.     rf22,rf29,rf3                                  
	fd.      rf26,rf5,rf22                                 
	fs.      rf17,rf10,rf22                                
	fms.    rf13,rf17,rf26,rf26                             
	fma     rf17,rf3,rf28,rf17                              
	fms.    rf6,rf27,rf6,rf17                               
	fd.      rf25,rf31,rf6                                 
	fnma    rf22,rf6,rf6,rf25                               
	fa       rf25,rf26,rf10                                
	fd       rf10,rf25,rf25                                
	fnms.   rf9,rf25,rf10,rf1                               
	fnms    rf1,rf15,rf9,rf0                                
	fnma.   rf26,rf19,rf9,rf21                              
	fs       rf1,rf24,rf6                                  
	fms     rf11,rf26,rf24,rf13                             
	fnma.   rf21,rf11,rf26,rf1                              
	fm.     rf6,rf11,rf11                                  
	frsp    rf18,rf21                                     
	fnms.   rf18,rf12,rf1,rf27                              
	fnma.   rf21,rf18,rf18,rf18                             
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1			# RC = 56 + 29 
	lfd	rf2,e7of1		# Load expected result of rf1 in rf2
	fcmpu	3,rf2,rf1		# Compare results
	bne	3,FINI			# Exit if not equal
#
	ai	g3,g3,1			# RC = 57
	lfd	rf1,e7of6		# Check rf6
	fcmpu	3,rf1,rf6
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 58
	lfd	rf1,e7of9		# Check rf9
	fcmpu	3,rf1,rf9
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 59
	lfd	rf1,e7of10		# Check rf10
	fcmpu	3,rf1,rf10
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 60
	lfd	rf1,e7of11		# Check rf11
	fcmpu	3,rf1,rf11
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 61
	lfd	rf1,e7of13		# Check rf13
	fcmpu	3,rf1,rf13
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 62
	lfd	rf1,e7of17		# Check rf17
	fcmpu	3,rf1,rf17
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 63
	lfd	rf1,e7of18		# Check rf18
	fcmpu	3,rf1,rf18
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 64
	lfd	rf1,e7of21		# Check rf21
	fcmpu	3,rf1,rf21
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 65
	lfd	rf1,e7of22		# Check rf22
	fcmpu	3,rf1,rf22
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 66
	lfd	rf1,e7of25		# Check rf25
	fcmpu	3,rf1,rf25
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 67
	lfd	rf1,e7of26		# Check rf26
	fcmpu	3,rf1,rf26
	bne	3,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf3,e8if3
	lfd	rf4,e8if4
	lfd	rf5,e8if5
	lfd	rf7,e8if7
	lfd	rf8,e8if8
	lfd	rf9,e8if9
	lfd	rf10,e8if10
	lfd	rf11,e8if11
	lfd	rf17,e8if17
	lfd	rf18,e8if18
	lfd	rf20,e8if20
	lfd	rf21,e8if21
	lfd	rf26,e8if26
	lfd	rf29,e8if29
	lfd	rf30,e8if30
#
	lfd	rf1,e8fps
	mtfsf	0xff,rf1
	l	g6,e8icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
#
	fs.      rf22,rf3,rf21                                
	fs       rf15,rf22,rf22                               
	fnma    rf15,rf9,rf11,rf20                             
	fnms    rf1,rf26,rf15,rf7                              
	fa       rf25,rf1,rf8                                 
	fma.    rf28,rf25,rf25,rf5                             
	fm.     rf28,rf30,rf25                                
	fd       rf15,rf28,rf28                               
	fm      rf7,rf28,rf11                                 
	fms.    rf8,rf15,rf7,rf18                              
	fm.     rf14,rf7,rf17                                 
	fs.      rf9,rf8,rf8                                  
	frsp    rf17,rf9                                     
	fnms    rf31,rf9,rf9,rf17                              
	fa       rf22,rf31,rf21                               
	fd.      rf23,rf10,rf22                               
	fd       rf20,rf22,rf11                               
	fma     rf1,rf23,rf29,rf23                             
	fnma.   rf20,rf18,rf26,rf4                             
	frsp.   rf15,rf20                                    
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1			# RC = 68 
	lfd	rf2,e8of1		# Load expected result of rf1 in rf2
	fcmpu	3,rf2,rf1		# Compare results
	bne	3,FINI			# Exit if not equal
#
	ai	g3,g3,1			# RC = 69
	lfd	rf1,e8of7		# Check rf7
	fcmpu	3,rf1,rf7
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 70
	lfd	rf1,e8of8		# Check rf8
	fcmpu	3,rf1,rf8
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 71
	lfd	rf1,e8of9		# Check rf9
	fcmpu	3,rf1,rf9
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 72
	lfd	rf1,e8of14		# Check rf14
	fcmpu	3,rf1,rf14
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 73
	lfd	rf1,e8of15		# Check rf15
	fcmpu	3,rf1,rf15
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 74
	lfd	rf1,e8of17		# Check rf17
	fcmpu	3,rf1,rf17
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 75
	lfd	rf1,e8of20		# Check rf20
	fcmpu	3,rf1,rf20
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 76
	lfd	rf1,e8of22		# Check rf22
	fcmpu	3,rf1,rf22
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 77
	lfd	rf1,e8of23		# Check rf23
	fcmpu	3,rf1,rf23
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 78
	lfd	rf1,e8of25		# Check rf25
	fcmpu	3,rf1,rf25
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 79
	lfd	rf1,e8of28		# Check rf28
	fcmpu	3,rf1,rf28
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 80
	lfd	rf1,e8of31		# Check rf31
	fcmpu	3,rf1,rf31
	bne	3,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf0,e9if0
	lfd	rf1,e9if1
	lfd	rf2,e9if2
	lfd	rf3,e9if3
	lfd	rf6,e9if6
	lfd	rf7,e9if7
	lfd	rf8,e9if8
	lfd	rf10,e9if10
	lfd	rf11,e9if11
	lfd	rf12,e9if12
	lfd	rf15,e9if15
	lfd	rf16,e9if16
	lfd	rf17,e9if17
	lfd	rf19,e9if19
	lfd	rf20,e9if20
	lfd	rf21,e9if21
	lfd	rf22,e9if22
	lfd	rf27,e9if27
	lfd	rf28,e9if28
	lfd	rf30,e9if30
	lfd	rf31,e9if31
# 
	lfd	rf4,e9fps
	mtfsf	0xff,rf4
	l	g6,e9icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
#
	fm      rf2,rf2,rf7                                   
	fma.    rf14,rf15,rf11,rf11                            
	fa.      rf9,rf28,rf17                                
	fm      rf11,rf21,rf28                                
	fms     rf28,rf19,rf12,rf10                            
	fma.    rf24,rf1,rf2,rf1                               
	fs       rf26,rf0,rf8                                 
	fnms.   rf29,rf8,rf22,rf1                              
	fa.      rf25,rf19,rf30                               
	fs       rf12,rf6,rf30                                
	fms.    rf13,rf20,rf12,rf20                            
	fd       rf5,rf9,rf16                                 
	fa.      rf10,rf21,rf19                               
	fd       rf16,rf7,rf30                                
	fs.      rf20,rf8,rf26                                
	frsp    rf22,rf30                                    
	fnma    rf9,rf28,rf31,rf25                             
	fd       rf15,rf8,rf27                                
	fm      rf4,rf11,rf3                                  
	fa       rf23,rf19,rf19                               
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1			# RC = 81
	lfd	rf1,e9of2		# Load expected result of rf2 in rf1
	fcmpu	3,rf1,rf2		# Compare results
	bne	3,FINI			# Exit if not equal
#
	ai	g3,g3,1			# RC = 82
	lfd	rf1,e9of4		# Check rf4
	fcmpu	3,rf1,rf4
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 83
	lfd	rf1,e9of5		# Check rf5
	fcmpu	3,rf1,rf5
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 84
	lfd	rf1,e9of9		# Check rf9
	fcmpu	3,rf1,rf9
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 85
	lfd	rf1,e9of10		# Check rf10
	fcmpu	3,rf1,rf10
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 86
	lfd	rf1,e9of11		# Check rf11
	fcmpu	3,rf1,rf11
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 87
	lfd	rf1,e9of12		# Check rf12
	fcmpu	3,rf1,rf12
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 88
	lfd	rf1,e9of13		# Check rf13
	fcmpu	3,rf1,rf13
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 89
	lfd	rf1,e9of14		# Check rf14
	fcmpu	3,rf1,rf14
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 90
	lfd	rf1,e9of15		# Check rf15
	fcmpu	3,rf1,rf15
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 91
	lfd	rf1,e9of16		# Check rf16
	fcmpu	3,rf1,rf16
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 92
	lfd	rf1,e9of20		# Check rf20
	fcmpu	3,rf1,rf20
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 93
	lfd	rf1,e9of22		# Check rf22
	fcmpu	3,rf1,rf22
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 94
	lfd	rf1,e9of23		# Check rf23
	fcmpu	3,rf1,rf23
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 95
	lfd	rf1,e9of24		# Check rf24
	fcmpu	3,rf1,rf24
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 96
	lfd	rf1,e9of25		# Check rf25
	fcmpu	3,rf1,rf25
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 97
	lfd	rf1,e9of26		# Check rf26
	fcmpu	3,rf1,rf26
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 98
	lfd	rf1,e9of28		# Check rf28
	fcmpu	3,rf1,rf28
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 99
	lfd	rf1,e9of29		# Check rf29
	fcmpu	3,rf1,rf29
	bne	3,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf1,e10if1
	lfd	rf4,e10if4
	lfd	rf6,e10if6
	lfd	rf7,e10if7
	lfd	rf8,e10if8
	lfd	rf9,e10if9
	lfd	rf11,e10if11
	lfd	rf15,e10if15
	lfd	rf17,e10if17
	lfd	rf18,e10if18
	lfd	rf19,e10if19
	lfd	rf20,e10if20
	lfd	rf22,e10if22
	lfd	rf23,e10if23
	lfd	rf25,e10if25
	lfd	rf26,e10if26
	lfd	rf27,e10if27
	lfd	rf29,e10if29
	lfd	rf31,e10if31
#
	lfd	rf2,e10fps
	mtfsf	0xff,rf2
	l	g6,e10icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
	fnma    rf22,rf22,rf6,rf22                             
	fs       rf28,rf4,rf29                                
	fa.      rf0,rf23,rf17                                
	fnma    rf10,rf15,rf1,rf28                             
	fa       rf24,rf9,rf28                                
	fm.     rf14,rf24,rf23                                
	fm.     rf9,rf15,rf18                                 
	fnms.   rf2,rf7,rf25,rf26                              
	fm      rf16,rf10,rf31                                
	fd.      rf28,rf17,rf4                                
	fms.    rf13,rf25,rf20,rf26                            
	fnma.   rf25,rf24,rf26,rf7                             
	fm      rf3,rf8,rf27                                  
	fma.    rf27,rf19,rf16,rf3                             
	fnma.   rf1,rf8,rf11,rf3                               
	fs.      rf26,rf27,rf26                               
	frsp    rf15,rf3                                     
	fms     rf18,rf28,rf1,rf17                             
	fnma    rf21,rf24,rf13,rf1                             
	frsp.   rf20,rf24                                    
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1			# RC = 100 
	lfd	rf4,e10of0		# Load expected result of rf0 in rf4
	fcmpu	3,rf4,rf0		# Compare results
	bne	3,FINI			# Exit if not equal
#
	ai	g3,g3,1			# RC = 101
	lfd	rf4,e10of1		# Check rf1
	fcmpu	3,rf4,rf1
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 102
	lfd	rf1,e10of2		# Check rf2
	fcmpu	3,rf1,rf2
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 103
	lfd	rf1,e10of3		# Check rf3
	fcmpu	3,rf1,rf3
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 104
	lfd	rf1,e10of9		# Check rf9
	fcmpu	3,rf1,rf9
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 105
	lfd	rf1,e10of10		# Check rf10
	fcmpu	3,rf1,rf10
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 106
	lfd	rf1,e10of13		# Check rf13
	fcmpu	3,rf1,rf13
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 107
	lfd	rf1,e10of14		# Check rf14
	fcmpu	3,rf1,rf14
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 108
	lfd	rf1,e10of15		# Check rf15
	fcmpu	3,rf1,rf15
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 109
	lfd	rf1,e10of16		# Check rf16
	fcmpu	3,rf1,rf16
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 110
	lfd	rf1,e10of18		# Check rf18
	fcmpu	3,rf1,rf18
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 111
	lfd	rf1,e10of20		# Check rf20
	fcmpu	3,rf1,rf20
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 112
	lfd	rf1,e10of21		# Check rf21
	fcmpu	3,rf1,rf21
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 113
	lfd	rf1,e10of22		# Check rf22
	fcmpu	3,rf1,rf22
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 114
	lfd	rf1,e10of24		# Check rf24
	fcmpu	3,rf1,rf24
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 115
	lfd	rf1,e10of25		# Check rf25
	fcmpu	3,rf1,rf25
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 116
	lfd	rf1,e10of26		# Check rf26
	fcmpu	3,rf1,rf26
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 117
	lfd	rf1,e10of27		# Check rf27
	fcmpu	3,rf1,rf27
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 118
	lfd	rf1,e10of28		# Check rf28
	fcmpu	3,rf1,rf28
	bne	3,FINI
#
	xor	g3,g3,g3		# Return code = 0; pass
FINI:	br
#
	.csect	_expfull[RW],3
_expfull: 
here:	.long	0xffffffff
	.long	0xffffffff
e1if3:	.long	0x0000045a
	.long	0x12a30e98
e1if4:	.long	0x8329e082
	.long	0xfac575e6
e1if5:	.long	0x800fff80
	.long	0x00000000
e1if6:	.long	0x1c26884d
	.long	0x6449d907
e1if10:	.long	0x825fffff
	.long	0xffff8000
e1if12:	.long	0x0000008d
	.long	0x01406415
e1if17:	.long	0x80000000
	.long	0x0019d649
e1if18:	.long	0x00000000
	.long	0x00000000
e1if19:	.long	0x800887c7
	.long	0xf35adddf
e1if20:	.long	0x9c49661c
	.long	0x2ee53eb7
e1if23:	.long	0x3d3b76d5
	.long	0x066f5b5f
e1if27:	.long	0x000ffffe
	.long	0x00000000
e1if29:	.long	0x80000000
	.long	0x00ab5427
e1if30:	.long	0x000c0000
	.long	0x00000000
e1fps:	.long	0xffffffff
	.long	0x00000000
e1icr:	.long	0x716b3d00
#
#results          
#
e1of2:	.long	0x81f4895b
	.long	0x10f2aae4
e1of3:	.long	0x80000000
	.long	0x00000000
e1of6:	.long	0x7ff80000
	.long	0x00000000
e1of8:	.long	0x7ff80000
	.long	0x00000000
e1of11:	.long	0x800fff80
	.long	0x00000000
e1of13:	.long	0x80000000
	.long	0x00000000
e1of14:	.long	0x80000000
	.long	0x00000000
e1of15:	.long	0x025fffff
	.long	0xffff8000
e1of16:	.long	0x80000000
	.long	0x0019d649
e1of17:	.long	0x801bff80
	.long	0x00000000
e1of21:	.long	0x7ff80000
	.long	0x00000000
e1of22:	.long	0x7ff80000
	.long	0x00000000
e1of25:	.long	0x80000000
	.long	0x00000000
e1of30:	.long	0x7ff00000
	.long	0x00000000
# 							
e2if0:	.long	0x800fffff
	.long	0xfff00000           
e2if1:	.long	0x80000000
	.long	0x006f7ab6          
e2if3:	.long	0x12ab79b8
	.long	0xdf8803fb         
e2if5:	.long	0x00000000
	.long	0x00000146        
e2if8:	.long	0x9c2a9573
	.long	0x534b57f5       
e2if9:	.long	0x9c44c48d
	.long	0x7fc0467b      
e2if10:	.long	0x80000000
	.long	0x00000000     
e2if13:	.long	0x9c45f0a0
	.long	0xbfdce717    
e2if14:	.long	0x9c39f5ac
	.long	0x82fa8f67   
e2if15:	.long	0x9d604c64
	.long	0x33fbcd2c  
e2if16:	.long	0x1c4ce1d0
	.long	0x165b8469 
e2if17:	.long	0x006fffff
	.long	0xfffffc00  
e2if21:	.long	0x1c39ebfb
	.long	0x94ee74a6 
e2if31:	.long	0x00cfffff
	.long	0xe0000000
e2fps:	.long	0xffffffff	
	.long	0x00000000    
e2icr:	.long	0x3dda901f                    
#
#results
#
e2of2:	.long	0x80000000
	.long	0x006f7ab6
e2of6:	.long	0x006fffff
	.long	0xfffffc00
e2of16:	.long	0x80000000
	.long	0x00000000
e2of17:	.long	0x006fffff
	.long	0xfffffc00
e2of18:	.long	0x80000000
	.long	0x00000000
e2of19:	.long	0x00000000
	.long	0x00000000
e2of20:	.long	0x9c45f0a0
	.long	0xbfdce717
e2of22:	.long	0x806fffff
	.long	0xfffffc00
e2of24:	.long	0x80000000
	.long	0x006f7ab6
e2of25:	.long	0x80000000
	.long	0x00000000
e2of26:	.long	0x80000000
	.long	0x00000000
e2of29:	.long	0x000fffff
	.long	0xfff00000
e2of31:	.long	0x00000000
	.long	0x00000000
#
e3if0:	.long	0x000004ff
	.long	0x74998385                                    
e3if2:	.long	0x9c3997d2
	.long	0x32e0b049                                    
e3if4:	.long	0x00000000
	.long	0x00135f83                                    
e3if7:	.long	0x006fffff
	.long	0xf0000000                                    
e3if8:	.long	0x80000000
	.long	0x00000000                                    
e3if9:	.long	0x9c255853
	.long	0x6a99b23e                                    
e3if12:	.long	0x9c581530
	.long	0xf97f81a5                                    
e3if14:	.long	0x1c6daea1
	.long	0xe31bf684                                    
e3if15:	.long	0x1c3bb491
	.long	0xefec2166                                    
e3if20:	.long	0x00000622
	.long	0x40eeffde                                    
e3if23:	.long	0x80000000
	.long	0x00000007                                    
e3if24:	.long	0x9c2ae93a
	.long	0xce39f1b7                                    
e3if25:	.long	0x9c5e397c
	.long	0xd3aec90c                                    
e3if27:	.long	0x9c48fac9
	.long	0xed1bcf65                                   
e3if30:	.long	0x80000001
	.long	0x62a8c21a                                    
e3fps:	.long	0xffffffff	
	.long	0x00000020                                                    
e3icr:	.long	0xb6756cea
#
#results                                                                
#
e3of1:	.long	0x0620a346
	.long	0x68b5b10e                                    
e3of3:	.long	0x80000000
	.long	0x00000000                                    
e3of4:	.long	0xb4e188e9
	.long	0x26f3bb82                                    
e3of6:	.long	0x589440be
	.long	0x58ed0ce4                                    
e3of9:	.long	0x835c8ebd
	.long	0x57da8c4a                                    
e3of11:	.long	0xdf73fdd2
	.long	0x660e1400                                    
e3of16:	.long	0x9220a346
	.long	0x60000000                                    
e3of17:	.long	0x8380dcdf
	.long	0x735e7cd5                                    
e3of20:	.long	0x912e5ced
	.long	0xa0625629                                    
e3of21:	.long	0x8620a346
	.long	0x68b5b10e                                    
e3of24:	.long	0x1c5e397c
	.long	0xd3aec90c                                    
e3of27:	.long	0x9c581530
	.long	0xf97f81a5                                    
e3of28:	.long	0x34c42c23
	.long	0xf6cc8d30                                    
e3of31:	.long	0x8380dcdf
	.long	0x735e7cd5                                    
#
e4if1:	.long	0x013fffff
	.long	0xfffffffe                                    
e4if2:	.long	0x1c482270
	.long	0x9d9407d0                                    
e4if8:	.long	0x00affc00
	.long	0x00000000                                    
e4if9:	.long	0x1c2dad3c
	.long	0x3b0f57ee                                    
e4if11:	.long	0x00dfffc0
	.long	0x00000000                                    
e4if15:	.long	0x00afffff
	.long	0x00000000                                    
e4if16:	.long	0x1c253ec6
	.long	0xd51558ad                                    
e4if17:	.long	0x9c2733af
	.long	0x1833fece                                    
e4if18:	.long	0x800fffff
	.long	0xff800000                                    
e4if23:	.long	0x800fffff
	.long	0xfffe0000                                    
e4if25:	.long	0x84eb1c08
	.long	0xf866334c                                    
e4if26:	.long	0x800fff80
	.long	0x00000000                                    
e4fps:	.long	0xffffffff	
	.long	0x00000020                                                    
e4icr:	.long	0xea4d1ea0
#
#results:	
#
e4of4:	.long	0x7ff80000
	.long	0x00000000                                    
e4of5:	.long	0x5ffff800
	.long	0x00000000                                    
e4of6:	.long	0x7ff00000
	.long	0x00000000                                    
e4of7:	.long	0x7ff80000
	.long	0x00000000                                    
e4of9:	.long	0x7ff80000
	.long	0x00000000                                    
e4of12:	.long	0x7ff80000
	.long	0x00000000                                    
e4of13:	.long	0x7ff80000
	.long	0x00000000                                    
e4of14:	.long	0x7ff00000
	.long	0x00000000                                    
e4of18:	.long	0x7ff80000
	.long	0x00000000                                    
e4of19:	.long	0xdffff800
	.long	0x00000000                                    
e4of20:	.long	0xdffff800
	.long	0x00000000                                    
e4of27:	.long	0x7ff80000
	.long	0x00000000                                    
e4of29:	.long	0x7ff80000
	.long	0x00000000                                    
e4of31:	.long	0x7ff80000
	.long	0x00000000                                    
#							
e5if0:	.long	0xffe0001f
	.long	0xffffffff                                    
e5if5:	.long	0x3ff0ffff
	.long	0xffffffff                                    
e5if8:	.long	0x7fefffe0
	.long	0x00000000                                    
e5if9:	.long	0xffefffff
	.long	0xffffffff                                    
e5if10:	.long	0xffefffff
	.long	0xffffffff                                    
e5if12:	.long	0xc0000000
	.long	0x0001ffff                                    
e5if14:	.long	0x7ca797cb
	.long	0xe54311a0                                    
e5if17:	.long	0xfca00000
	.long	0x00000000                                    
e5if19:	.long	0x7fe00000
	.long	0x00007fff                                    
e5if20:	.long	0x7fefffff
	.long	0xffffffff                                    
e5if21:	.long	0x7fefffff
	.long	0xffffffff                                    
e5if23:	.long	0xffefc000
	.long	0x00000000                                    
e5if24:	.long	0xffefffff
	.long	0xf0000000                                    
e5if26:	.long	0xc0100000
	.long	0x7fffffff                                    
e5if28:	.long	0x755cffc0
	.long	0xc7344275                                    
e5if31:	.long	0x7fe00000
	.long	0x000001ff                                    
e5fps:	.long	0xffffffff	
	.long	0x00000000                                                    
e5icr:	.long	0xf598689f
#
#results:
#
e5of0:	.long	0x7ff80000
	.long	0x00000000                                    
e5of2:	.long	0x7ff80000
	.long	0x00000000                                    
e5of3:	.long	0x7ff80000
	.long	0x00000000                                    
e5of5:	.long	0x7ff80000
	.long	0x00000000                                    
e5of7:	.long	0xffdd7fff
	.long	0xfffffbc4                                    
e5of9:	.long	0x7ff80000
	.long	0x00000000                                    
e5of11:	.long	0x7ff00000
	.long	0x00000000                                    
e5of13:	.long	0x7ff00000
	.long	0x00000000                                    
e5of17:	.long	0x7ff80000
	.long	0x00000000                                    
e5of18:	.long	0x7ff80000
	.long	0x00000000                                    
e5of19:	.long	0x7ff80000
	.long	0x00000000                                    
e5of21:	.long	0x7ff80000
	.long	0x00000000                                    
e5of22:	.long	0x7ff80000
	.long	0x00000000                                    
e5of25:	.long	0x7ff00000
	.long	0x00000000                                    
e5of30:	.long	0x7ff00000
	.long	0x00000000                                    
e5of31:	.long	0x7ff80000
	.long	0x00000000                                    
#
e6if2:	.long	0x7fefffff
	.long	0xfffffff8                                    
e6if3:	.long	0x7fe00000
	.long	0x0000ffff                                    
e6if4:	.long	0x3d3b3e08
	.long	0x2f31696f                                    
e6if7:	.long	0x80044007
	.long	0xa685d533                                    
e6if8:	.long	0xffefffff
	.long	0xffffffff                                    
e6if11:	.long	0xeea5a85f
	.long	0x7d8b321a                                    
e6if12:	.long	0x80000000
	.long	0xf0ba13b5                                    
e6if13:	.long	0x40100000
	.long	0x0000ffff                                    
e6if17:	.long	0xffd00000
	.long	0x007fffff                                    
e6if19:	.long	0x7feffc00
	.long	0x00000000                                    
e6if20:	.long	0x7fefffff
	.long	0xffffff80                                    
e6if21:	.long	0xf455e69d
	.long	0x757d1024                                    
e6if26:	.long	0xfc840000
	.long	0x00000000                                    
e6fps:	.long	0xffffffff	
	.long	0x00000000                                                    
e6icr:	.long	0x730c14ec
#
#results                                                                
#
e6of6:	.long	0xffefffff
	.long	0xffffffff                                    
e6of8:	.long	0x7ff80000
	.long	0x00000000                                    
e6of10:	.long	0x7ff80000
	.long	0x00000000                                    
e6of12:	.long	0x7ff80000
	.long	0x00000000                                    
e6of15:	.long	0x7ff00000
	.long	0x00000000                                    
e6of16:	.long	0x7ff80000
	.long	0x00000000                                    
e6of18:	.long	0x7ff80000
	.long	0x00000000                                    
e6of19:	.long	0x7ff80000
	.long	0x00000000                                    
e6of20:	.long	0x7ff80000
	.long	0x00000000                                    
e6of22:	.long	0x7ff80000
	.long	0x00000000                                    
e6of23:	.long	0x7ff80000
	.long	0x00000000                                    
e6of24:	.long	0xfff00000
	.long	0x00000000                                    
e6of30:	.long	0x7ff80000
	.long	0x00000000                                    
#
e7if0:	.long	0xffefffc0
	.long	0x00000000                                     
e7if1:	.long	0xffefffff
	.long	0xfff80000                                     
e7if3:	.long	0x753abdcf
	.long	0x3eafcacc                                     
e7if5:	.long	0xbf326898
	.long	0x4b000188                                     
e7if6:	.long	0xc010007f
	.long	0xffffffff                                     
e7if10:	.long	0x7c840000
	.long	0x00000000                                     
e7if12:	.long	0x7fe00000
	.long	0x03ffffff                                     
e7if15:	.long	0xffd0003f
	.long	0xffffffff                                     
e7if19:	.long	0x7fd00000
	.long	0x0000007f                                     
e7if21:	.long	0x7fefffff
	.long	0xffffffff                                     
e7if24:	.long	0x7c8c0000
	.long	0x00000000                                     
e7if27:	.long	0xffe00000
	.long	0x0fffffff                                     
e7if28:	.long	0x4010ffff
	.long	0xffffffff                                     
e7if29:	.long	0x73faf9c1
	.long	0x53ef5290                                     
e7if31:	.long	0x3e2e3d38
	.long	0x28e68895                                     
e7fps:	.long	0xffffffff
	.long	0x00000060
e7icr:	.long	0x124d7cf0                                                     
#
#results                                                                 
#
e7of1:	.long	0x7c8c0000
	.long	0x00000000                                     
e7of6:	.long	0x38e87fff
	.long	0xfff3c185                                     
e7of9:	.long	0xffefffff
	.long	0xfff80000                                     
e7of10:	.long	0x3ff00000
	.long	0x00000000                                     
e7of11:	.long	0x3c6bffff
	.long	0xfff900de                                     
e7of13:	.long	0xf2705509
	.long	0x7fb99ba5                                     
e7of17:	.long	0x7c840000
	.long	0x00000000                                     
e7of18:	.long	0xdc7c0000
	.long	0x06fffffe                                     
e7of21:	.long	0xf9088000
	.long	0x0c3ffffe                                     
e7of22:	.long	0xde1e3c46
	.long	0x28780cce                                     
e7of25:	.long	0x7c840000
	.long	0x00000000                                     
e7of26:	.long	0x5fcfffff
	.long	0xfff800fe                                     
#
e8if3:	.long	0x7ca00000
	.long	0x00000000                                    
e8if4:	.long	0xffefffff
	.long	0xffffffc0                                    
e8if5:	.long	0x7fefffff
	.long	0xffffff00                                    
e8if7:	.long	0x7fefffff
	.long	0xff800000                                    
e8if8:	.long	0x7ca00000
	.long	0x00000000                                    
e8if9:	.long	0x7fd0007f
	.long	0xffffffff                                    
e8if10:	.long	0xbc86f0d5
	.long	0x8e16a2e1                                    
e8if11:	.long	0x40100000
	.long	0x00007fff                                    
e8if17:	.long	0x72fb37a1
	.long	0xf0af04f1                                    
e8if18:	.long	0x7fefffff
	.long	0xfffffffe                                    
e8if20:	.long	0xffefff00
	.long	0x00000000                                    
e8if21:	.long	0x7fefffff
	.long	0xffffffff                                    
e8if26:	.long	0x7fe07fff
	.long	0xffffffff                                    
e8if29:	.long	0xc0100007
	.long	0xffffffff                                    
e8if30:	.long	0x6c61c17d
	.long	0xc244e308                                    
e8fps:	.long	0xffffffff	
	.long	0x00000060                                                    
e8icr:	.long	0x7b0213d3                                                    
#
#results                                                                
#
e8of1:	.long	0x5ca134ab
	.long	0xa2fbc133                                    
e8of7:	.long	0x4931c17d
	.long	0xc2457113                                    
e8of8:	.long	0xffefffff
	.long	0xfffffffe                                    
e8of9:	.long	0x00000000
	.long	0x00000000                                    
e8of14:	.long	0x7c3e3441
	.long	0x0e3b088d                                    
e8of15:	.long	0xd3e08000
	.long	0x00000000                                    
e8of17:	.long	0x00000000
	.long	0x00000000                                    
e8of20:	.long	0xdfe07fff
	.long	0xfffffffe                                    
e8of22:	.long	0x7fefffff
	.long	0xffffffff                                    
e8of23:	.long	0xdc86f0d5
	.long	0x8e16a2e2                                    
e8of25:	.long	0x7ca00000
	.long	0x00000000                                    
e8of28:	.long	0x4911c17d
	.long	0xc244e308                                    
e8of31:	.long	0x80000000
	.long	0x00000000                                    
#
e9if0:	.long	0x000fffff
	.long	0xfffc0000                                    
e9if1:	.long	0x00000000
	.long	0xe27e0a4e                                    
e9if2:	.long	0x3ff00000
	.long	0x00000001                                    
e9if3:	.long	0x000fffff
	.long	0xffffffff                                    
e9if6:	.long	0x800fffff
	.long	0xffffff00                                    
e9if7:	.long	0x000fffc0
	.long	0x00000000                                    
e9if8:	.long	0x80000003
	.long	0xffffffff                                    
e9if10:	.long	0x00005ef9
	.long	0xaf981ebb                                    
e9if11:	.long	0x800003a8
	.long	0x6a3ff883                                    
e9if12:	.long	0x8000000f
	.long	0xf110570c                                    
e9if15:	.long	0x80000000
	.long	0x00002bd7                                    
e9if16:	.long	0x3c4e3911
	.long	0x54f9e5b6                                    
e9if17:	.long	0x8003ffff
	.long	0xffffffff                                    
e9if19:	.long	0x8000067f
	.long	0xb8a385b1                                    
e9if20:	.long	0x00000192
	.long	0xd88dd981                                    
e9if21:	.long	0xbff00000
	.long	0x00000001                                    
e9if22:	.long	0x00000000
	.long	0x00e69f61                                    
e9if27:	.long	0xbdda3adf
	.long	0xc07c46e2                                    
e9if28:	.long	0x800fffff
	.long	0xfffffffc                                    
e9if30:	.long	0x00000000
	.long	0x007fffff                                    
e9if31:	.long	0x80000000
	.long	0x02aaf5a4                                    
e9fps:	.long	0xffffffff
	.long	0x00000000
e9icr:	.long	0x7cd7f3d6                                                    
#results:
e9of2:	.long	0x000fffc0
	.long	0x00000001                                    
e9of4:	.long	0x00000000
	.long	0x00000000                                    
e9of5:	.long	0x83b52d0d
	.long	0x1a20ed53                                    
e9of9:	.long	0x0000067f
	.long	0xb82385b2                                    
e9of10:	.long	0xbff00000
	.long	0x00000001                                    
e9of11:	.long	0x000fffff
	.long	0xfffffffd                                    
e9of12:	.long	0x80100000
	.long	0x007ffeff                                    
e9of13:	.long	0x80000192
	.long	0xd88dd981                                    
e9of14:	.long	0x800003a8
	.long	0x6a3ff883                                    
e9of15:	.long	0x01038507
	.long	0xdf1242f8                                    
e9of16:	.long	0x41bfff80
	.long	0x3fff0080                                    
e9of20:	.long	0x80100007
	.long	0xfffbfffe                                    
e9of22:	.long	0x00000000
	.long	0x00000000                                    
e9of23:	.long	0x80000cff
	.long	0x71470b62                                    
e9of24:	.long	0x00000000
	.long	0xe27e0a4e                                    
e9of25:	.long	0x8000067f
	.long	0xb82385b2                                    
e9of26:	.long	0x00100003
	.long	0xfffbffff                                    
e9of28:	.long	0x80005ef9
	.long	0xaf981ebb                                    
e9of29:	.long	0x00000000
	.long	0xe27e0a4e                                    
#
e10if1:		.long	0x00000000
		.long	0x00000c7b                                    
e10if4:		.long	0x800fffff
		.long	0xfffffffc                                    
e10if6:		.long	0x80000000
		.long	0x00000349                                    
e10if7:		.long	0x80000000
		.long	0x000ad144                                    
e10if8:		.long	0x3ff00000
		.long	0x00000000                                    
e10if9:		.long	0x800fffff
		.long	0xffff8000                                    
e10if11:	.long	0x80000298
		.long	0xa4737f16                                    
e10if15:	.long	0x00000000
		.long	0x00000005                                    
e10if17:	.long	0x00000000
		.long	0x00ffffff                                    
e10if18:	.long	0x800fffff
		.long	0xffffffff                                    
e10if19:	.long	0x80000000
		.long	0x0000abd4                                    
e10if20:	.long	0x00000000
		.long	0x000019a5                                    
e10if22:	.long	0x800087cc
		.long	0xbad2ef8c                                    
e10if23:	.long	0x000fffff
		.long	0xf0000000                                    
e10if25:	.long	0x00000000
		.long	0x00000097                                    
e10if26:	.long	0x800000fc
		.long	0x174e41c5                                    
e10if27:	.long	0x000fffff
		.long	0xffffffff                                    
e10if29:	.long	0x0003ffff
		.long	0xffffffff                                    
e10if31:	.long	0x000fffff
		.long	0xffffffff                                    
e10fps:		.long	0xffffffff
		.long	0x00000020
e10icr:		.long	0x8517a802
#
#results:	
#
e10of0:		.long	0x600fffff
		.long	0xe1fffffe                                    
e10of1:		.long	0xe00fffff
		.long	0xfffffffe                                    
e10of2:		.long	0xdf4f82e9
		.long	0xc838a000                                    
e10of3:		.long	0x600fffff
		.long	0xfffffffe                                    
e10of9:		.long	0x9d13ffff
		.long	0xffffffff                                    
e10of10:	.long	0x0013ffff
		.long	0xfffffffb                                    
e10of13:	.long	0x5f4f82e9
		.long	0xc838a000                                    
e10of14:	.long	0xa041ffff
		.long	0xedffbffe                                    
e10of15:	.long	0x7ff00000
		.long	0x00000000                                    
e10of16:	.long	0x2033ffff
		.long	0xfffffffa                                    
e10of18:	.long	0x5e4fffff
		.long	0xe0000006                                    
e10of20:	.long	0x8c220000
		.long	0x00000000                                    
e10of21:	.long	0x600fffff
		.long	0xfffffffe                                    
e10of22:	.long	0x5fc0f997
		.long	0x5a5df180                                    
e10of24:	.long	0x8021ffff
		.long	0xffffbffe                                    
e10of25:	.long	0x5e05a288
		.long	0x00000000                                    
e10of26:	.long	0x600fffff
		.long	0xfffffffe                                    
e10of27:	.long	0x600fffff
		.long	0xfffffffe                                    
e10of28:	.long	0xbe2fffff
		.long	0xe0000008                                    
#
	.toc
	TOCE(.expfull, entry)
	TOCL(_expfull, data)

