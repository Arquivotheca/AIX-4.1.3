# @(#)40	1.3  src/bos/diag/da/fpa/decfull.s, dafpa, bos411, 9428A410j 3/23/94 06:21:36

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
#	First module to test the "decode floating point" functionality.
#		Appropriate registers are loaded with input values. Then
#		instructions for this module will be executed and results
#		will be compared with expected results. If any test fails,
#		a particular return code will be returned to the caller.
#
	.csect	.decfull[PR]
	.globl	.decfull[PR]
	.align	2
.decfull:
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
	LTOC(4, _decfull, data)		# Loads g4 with the address of _decfull
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
 	.using	_decfull[RW],g4		# g4 is the base register to the data
#	 				     section _decfull[rw]. Eastablishing 
#					     @ability
#
#	
	xor	g3,g3,g3		# g3 = 0. Initialize return code
	ai	g3,g3,1			# Preset return code = 1; fail
#
#				# Loading floating pt registers for computation
#
	lfd	rf1,d1if1
	lfd	rf2,d1if2
	lfd	rf4,d1if4
	lfd	rf5,d1if5
	lfd	rf6,d1if6
	lfd	rf7,d1if7
	lfd	rf8,d1if8
	lfd	rf13,d1if13
	lfd	rf15,d1if15
	lfd	rf18,d1if18
	lfd	rf19,d1if19
	lfd	rf21,d1if21
	lfd	rf22,d1if22
	lfd	rf23,d1if23
	lfd	rf25,d1if25
	lfd	rf26,d1if26
	lfd	rf28,d1if28
	lfd	rf29,d1if29
	lfd	rf30,d1if30
#
	lfd	rf0,d1fps
	mtfsf	0xff,rf0
	l	g6,d1icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
	fd.      rf11,rf4,rf7                                 
	fa.      rf24,rf25,rf11                               
	fcmpo    1,rf22,rf1                                  
	fa.      rf3,rf24,rf30                                
	fa.      rf16,rf3,rf8                                 
	fcmpu    2,rf3,rf24                                  
	mffs    rf16                                        
	mtfsf.  0x87,rf6		# b'1100001110'(first and last bit ign)
	frsp.   rf31,rf16                                    
	mcrfs   3,7                                        
	fma     rf0,rf19,rf21,rf30                             
	frsp    rf0,rf28                                     
	mtfsb0  4                                          
	mtfsb1. 5                                          
	fcmpu    1,rf1,rf2                                   
	mtfsf   0x3c,rf6		# b'1001111001',rf6
	fm      rf12,rf18,rf15                                
	mcrfs   3,1                                        
	fnma.   rf22,rf23,rf13,rf24                            
	fnms.   rf25,rf15,rf22,rf22                            
	fabs.   rf29,rf25                                    
	fneg.   rf24,rf29                                    
	fnma    rf20,rf7,rf24,rf29                             
	mtfsb1. 25                                         
	fcmpu    6,rf5,rf24                                  
	fcmpo    7,rf7,rf26                                  
	fnabs.  rf28,rf31                                    
	fa       rf23,rf28,rf22                               
	fd.      rf7,rf22,rf23                                
	fcmpo    5,rf6,rf7                                   
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	lfd	rf1,d1of0		# Load expected result of rf0 in rf1
	fcmpu	3,rf1,rf0		# Compare results
	bne	3,FINI			# Exit if not equal
#
	ai	g3,g3,1			# RC = 2
	lfd	rf1,d1of3		# Check rf3
	fcmpu	3,rf1,rf3
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 3
	stfd	rf7,here		# Check rf7
	l	g7,here
	l	g8,d1of7
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d1of7+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 4
	lfd	rf1,d1of11		# Check rf11
	fcmpu	3,rf1,rf11
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 5
	lfd	rf1,d1of12		# Check rf12
	fcmpu	3,rf1,rf12
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 6
	stfd	rf16,here		# Check rf16 
	l	g7,here
	l	g8,d1of16
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d1of16+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 7
	lfd	rf1,d1of20		# Check rf20
	fcmpu	3,rf1,rf20
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 8
	lfd	rf1,d1of22		# Check rf22
	fcmpu	3,rf1,rf22
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 9
	stfd	rf23,here		# Check rf23
	l	g7,here
	l	g8,d1of23
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d1of23+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 10
	lfd	rf1,d1of24		# Check rf24
	fcmpu	3,rf1,rf24
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 11
	lfd	rf1,d1of25		# Check rf25
	fcmpu	3,rf1,rf25
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 12
	stfd	rf28,here		# Check rf28
	l	g7,here
	l	g8,d1of28
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d1of28+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 13
	lfd	rf1,d1of29		# Check rf29
	fcmpu	3,rf1,rf29
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 14
	stfd	rf31,here		# Check rf31
	l	g7,here
	l	g8,d1of31
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d1of31+4
	cmp	0,g7,g8
	bne	0,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf0,d2if0
	lfd	rf1,d2if1
	lfd	rf2,d2if2
	lfd	rf6,d2if6
	lfd	rf9,d2if9
	lfd	rf10,d2if10
	lfd	rf12,d2if12
	lfd	rf13,d2if13
	lfd	rf14,d2if14
	lfd	rf17,d2if17
	lfd	rf19,d2if19
	lfd	rf20,d2if20
	lfd	rf22,d2if22
	lfd	rf24,d2if24
	lfd	rf25,d2if25
	lfd	rf27,d2if27
	lfd	rf28,d2if28
	lfd	rf29,d2if29
#
	lfd	rf3,d2fps
	mtfsf	0xff,rf3
	l	g6,d2icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
	fmr.    rf29,rf20                                    
	fcmpo    3,rf25,rf29                                 
	fcmpu    0,rf28,rf29                                 
	fnma    rf27,rf22,rf27,rf27                            
	fnma.   rf11,rf20,rf27,rf24                            
	fcmpu    3,rf11,rf1                                  
	fnabs.  rf11,rf20                                    
	fms.    rf23,rf13,rf11,rf9                             
	mtfsb1. 1                                          
	fneg    rf19,rf11                                    
	fcmpu    1,rf19,rf0                                  
	frsp.   rf4,rf19                                     
	fnma.   rf31,rf12,rf2,rf4                              
	fcmpo    7,rf4,rf6                                   
	mcrfs   0,1                                        
	frsp    rf12,rf31                                    
	fa.      rf13,rf12,rf28                               
	fm      rf12,rf12,rf17                                
	mtfsf.  0xd8,rf12		# b'0110110001',rf12
	fnma    rf6,rf28,rf12,rf12                             
	fm.     rf25,rf6,rf0                                  
	fcmpo    4,rf23,rf25                                 
	fma     rf25,rf6,rf14,rf20                             
	fd       rf21,rf25,rf4                                
	fd       rf29,rf27,rf21                               
	fnma    rf1,rf10,rf21,rf29                             
	fnma.   rf1,rf13,rf0,rf31                              
	mffs    rf7                                         
	mtfsb1. 17                                         
	fabs.   rf19,rf1                                     
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1			# Preset return code = 1; fail
	stfd	rf1,here		# Check rf1
	l	g7,here
	l	g8,d2of1
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d2of1+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 2
	stfd	rf4,here		# Check rf4
	l	g7,here
	l	g8,d2of4
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d2of4+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 3
	stfd	rf6,here		# Check rf6
	l	g7,here
	l	g8,d2of6
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d2of6+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 4
	stfd	rf7,here		# Check rf7
	l	g7,here
	l	g8,d2of7
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d2of7+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 5
	stfd	rf11,here		# Check rf11
	l	g7,here
	l	g8,d2of11
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d2of11+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 6
	stfd	rf12,here		# Check rf12
	l	g7,here
	l	g8,d2of12
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d2of12+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 7
	stfd	rf13,here		# Check rf13
	l	g7,here
	l	g8,d2of13
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d2of13+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 8
	stfd	rf19,here		# Check rf19
	l	g7,here
	l	g8,d2of19
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d2of19+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 9
	stfd	rf21,here		# Check rf21
	l	g7,here
	l	g8,d2of21
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d2of21+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 10
	stfd	rf23,here		# Check rf23
	l	g7,here
	l	g8,d2of23
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d2of23+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 11
	stfd	rf25,here		# Check rf25
	l	g7,here
	l	g8,d2of25
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d2of25+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 12
	stfd	rf27,here		# Check rf27
	l	g7,here
	l	g8,d2of27
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d2of27+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 13
	stfd	rf29,here		# Check rf29
	l	g7,here
	l	g8,d2of29
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d2of29+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 14
	stfd	rf31,here		# Check rf31
	l	g7,here
	l	g8,d2of31
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d2of31+4
	cmp	0,g7,g8
	bne	0,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf0,d3if0
	lfd	rf1,d3if1
	lfd	rf2,d3if2
	lfd	rf4,d3if4
	lfd	rf7,d3if7
	lfd	rf10,d3if10
	lfd	rf12,d3if12
	lfd	rf13,d3if13
	lfd	rf15,d3if15
	lfd	rf16,d3if16
	lfd	rf17,d3if17
	lfd	rf18,d3if18
	lfd	rf19,d3if19
	lfd	rf21,d3if21
	lfd	rf27,d3if27
	lfd	rf28,d3if28
	lfd	rf29,d3if29
	lfd	rf31,d3if31
#
	lfd	rf3,d3fps
	mtfsf	0xff,rf3
	l	g6,d3icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
	fnms.   rf23,rf0,rf1,rf19                              
	fcmpo    1,rf23,rf23                                 
	fcmpo    2,rf16,rf23                                 
	fm.     rf9,rf27,rf17                                 
	fcmpo    3,rf12,rf9                                  
	mcrfs   3,5                                        
	fd.      rf24,rf2,rf9                                 
	fnms.   rf27,rf4,rf24,rf24                             
	mtfsf.  0x56,rf21		# b'0010101101',rf21
	fmr.    rf13,rf27                                    
	mtfsf   0xdb,rf13		# b'1110110110',rf13
	mcrfs   3,2                                        
	mcrfs   3,1                                        
	fma     rf3,rf13,rf16,rf4                              
	fms     rf4,rf29,rf3,rf3                               
	fabs.   rf16,rf4                                     
	mcrfs   0,6                                        
	fd       rf6,rf4,rf23                                 
	fs.      rf26,rf3,rf6                                 
	fms.    rf26,rf19,rf23,rf29                            
	mtfsf.  0xe8,rf26		# b'1111010000',rf26
	frsp.   rf26,rf15                                    
	fmr.    rf28,rf26                                    
	fnma.   rf17,rf26,rf18,rf0                             
	fabs.   rf10,rf17                                    
	mcrfs   1,5                                        
	mtfsf.  0x04,rf18		# b'0000001000',rf18
	fabs.   rf6,rf7                                      
	fm      rf9,rf31,rf6                                  
	fd       rf12,rf6,rf29                                
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1			# Preset return code = 1; fail
	stfd	rf3,here		# Check rf3
	l	g7,here
	l	g8,d3of3
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d3of3+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 2
	stfd	rf4,here		# Check rf4
	l	g7,here
	l	g8,d3of4
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d3of4+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 3
	lfd	rf1,d3of6			# Check rf6
	fcmpu	3,rf1,rf6
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 4
	lfd	rf1,d3of9		# Check rf9
	fcmpu	3,rf1,rf9
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 5
	lfd	rf1,d3of10		# Check rf10
	fcmpu	3,rf1,rf10
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 6
	lfd	rf1,d3of12		# Check rf12
	fcmpu	3,rf1,rf12
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 7
	stfd	rf13,here		# Check rf13
	l	g7,here
	l	g8,d3of13
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d3of13+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 8
	stfd	rf16,here		# Check rf16
	l	g7,here
	l	g8,d3of16
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d3of16+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 9
	lfd	rf1,d3of17		# Check rf17
	fcmpu	3,rf1,rf17
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 10
	lfd	rf1,d3of23		# Check rf23
	fcmpu	3,rf1,rf23
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 11
	lfd	rf1,d3of24		# Check rf24
	fcmpu	3,rf1,rf24
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 12
	lfd	rf1,d3of26		# Check rf26
	fcmpu	3,rf1,rf26
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 13
	stfd	rf27,here		# Check rf27
	l	g7,here
	l	g8,d3of27
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d3of27+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 14 
	lfd	rf1,d3of28		# Check rf28
	fcmpu	3,rf1,rf28
	bne	3,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf2,d4if2
	lfd	rf3,d4if3
	lfd	rf4,d4if4
	lfd	rf5,d4if5
	lfd	rf6,d4if6
	lfd	rf7,d4if7
	lfd	rf8,d4if8
	lfd	rf11,d4if11
	lfd	rf13,d4if13
	lfd	rf14,d4if14
	lfd	rf16,d4if16
	lfd	rf18,d4if18
	lfd	rf20,d4if20
	lfd	rf21,d4if21
	lfd	rf22,d4if22
	lfd	rf27,d4if27
	lfd	rf28,d4if28
	lfd	rf29,d4if29
	lfd	rf31,d4if31
#
	lfd	rf1,d4fps
	mtfsf	0xff,rf1
	l	g6,d4icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
	fmr.    rf6,rf20                                     
	fnms    rf10,rf11,rf21,rf6                             
	fd.      rf10,rf28,rf5                                
	fabs    rf28,rf10                                    
	mcrfs   4,7                                        
	mcrfs   3,7                                        
	fneg    rf10,rf14                                    
	fcmpu    3,rf10,rf16                                 
	fnms.   rf15,rf10,rf21,rf10                            
	fnms.   rf15,rf10,rf29,rf29                            
	fnms.   rf30,rf15,rf28,rf20                            
	fms     rf30,rf22,rf4,rf8                              
	fnabs   rf7,rf30                                     
	fm      rf30,rf14,rf27                                
	fd.      rf17,rf30,rf18                               
	fabs    rf3,rf17                                     
	fs       rf15,rf17,rf13                               
	fs       rf13,rf14,rf3                                
	fma     rf9,rf15,rf31,rf2                              
	fms     rf31,rf9,rf9,rf15                              
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
#
	ai	g3,g3,1			# Preset return code = 1; fail
	stfd	rf3,here		# Check rf3
	l	g7,here
	l	g8,d4of3
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d4of3+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 2
	stfd	rf6,here		# Check rf6
	l	g7,here
	l	g8,d4of6
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d4of6+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 3
	stfd	rf7,here		# Check rf7
	l	g7,here
	l	g8,d4of7
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d4of7+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 4
	stfd	rf9,here		# Check rf9
	l	g7,here
	l	g8,d4of9
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d4of9+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 5
	stfd	rf10,here		# Check rf10
	l	g7,here
	l	g8,d4of10
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d4of10+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 6
	stfd	rf13,here		# Check rf13
	l	g7,here
	l	g8,d4of13
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d4of13+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 7
	stfd	rf15,here		# Check rf15
	l	g7,here
	l	g8,d4of15
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d4of15+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 8
	stfd	rf17,here		# Check rf17
	l	g7,here
	l	g8,d4of17
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d4of17+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 9
	lfd	rf1,d4of28		# Check rf28
	fcmpu	3,rf1,rf28
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 10
	stfd	rf30,here		# Check rf30
	l	g7,here
	l	g8,d4of30
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d4of30+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 11
	stfd	rf31,here		# Check rf31
	l	g7,here
	l	g8,d4of31
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d4of31+4
	cmp	0,g7,g8
	bne	0,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf0,d5if0
	lfd	rf1,d5if1
	lfd	rf2,d5if2
	lfd	rf3,d5if3
	lfd	rf4,d5if4
	lfd	rf6,d5if6
	lfd	rf7,d5if7
	lfd	rf13,d5if13
	lfd	rf14,d5if14
	lfd	rf16,d5if16
	lfd	rf17,d5if17
	lfd	rf22,d5if22
	lfd	rf25,d5if25
	lfd	rf26,d5if26
#
	lfd	rf5,d5fps
	mtfsf	0xff,rf5
	l	g6,d5icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
	fd       rf20,rf16,rf14                               
	frsp    rf15,rf20                                    
	fms.    rf7,rf15,rf7,rf20                              
	mtfsb1  9                                          
	mffs.   rf10                                        
	mcrfs   2,6                                        
	fs       rf30,rf0,rf16                                
	fm      rf30,rf26,rf30                                
	frsp.   rf16,rf30                                    
	mtfsf   0x10,rf2			# b'0000100001',rf2
	fd       rf19,rf16,rf4                                
	fma.    rf4,rf19,rf22,rf19                             
	mffs    rf8                                         
	fneg    rf1,rf8                                      
	fs       rf9,rf1,rf6                                  
	mtfsb0  0                                          
	mffs.   rf31                                        
	frsp    rf11,rf31                                    
	fabs.   rf19,rf11                                    
	fnma    rf19,rf15,rf14,rf3                             
	fms.    rf1,rf17,rf14,rf19                             
	fnabs.  rf1,rf17                                     
	fcmpo    0,rf1,rf25                                  
	fnma    rf24,rf13,rf1,rf26                             
	mtfsb0  11                                         
	fms.    rf5,rf26,rf24,rf25                             
	fd       rf23,rf5,rf9                                 
	frsp.   rf13,rf23                                    
	fma.    rf21,rf17,rf13,rf13                            
	fnma    rf21,rf13,rf21,rf26                            
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1			# Preset return code = 1; fail
	lfd	rf1,d5of1		# Load expected result of rf8 in rf1
	fcmpu	3,rf1,rf1		# Compare results
	bne	3,FINI			# Exit if not equal
#
	ai	g3,g3,1			# RC = 2
	stfd	rf4,here		# Check rf4
	l	g7,here
	l	g8,d5of4
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d5of4+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 3
	stfd	rf5,here		# Check rf5
	l	g7,here
	l	g8,d5of5
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d5of5+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 4
	stfd	rf7,here		# Check rf7
	l	g7,here
	l	g8,d5of7
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d5of7+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 5
	stfd	rf8,here		# Check rf8
	l	g7,here
	l	g8,d5of8
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d5of8+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 6
	stfd	rf9,here		# Check rf9
	l	g7,here
	l	g8,d5of9
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d5of9+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 7
	stfd	rf10,here		# Check rf10
	l	g7,here
	l	g8,d5of10
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d5of10+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 8
	stfd	rf11,here		# Check rf11
	l	g7,here
	l	g8,d5of11
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d5of11+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 9
	stfd	rf13,here		# Check rf13
	l	g7,here
	l	g8,d5of13
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d5of13+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 10
	stfd	rf15,here		# Check rf15
	l	g7,here
	l	g8,d5of15
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d5of15+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 11
	stfd	rf16,here		# Check rf16
	l	g7,here
	l	g8,d5of16
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d5of16+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 12
	stfd	rf19,here		# Check rf19
	l	g7,here
	l	g8,d5of19
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d5of19+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 13
	stfd	rf20,here		# Check rf20
	l	g7,here
	l	g8,d5of20
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d5of20+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 14
	stfd	rf21,here		# Check rf21
	l	g7,here
	l	g8,d5of21
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d5of21+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 15
	stfd	rf23,here		# Check rf23
	l	g7,here
	l	g8,d5of23
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d5of23+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 16
	stfd	rf24,here		# Check rf24
	l	g7,here
	l	g8,d5of24
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d5of24+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 17
	stfd	rf30,here		# Check rf30
	l	g7,here
	l	g8,d5of30
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d5of30+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 18
	stfd	rf31,here		# Check rf31
	l	g7,here
	l	g8,d5of31
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d5of31+4
	cmp	0,g7,g8
	bne	0,FINI
# 
#				# Loading floating pt registers for computation
#
	lfd	rf0,d6if0
	lfd	rf1,d6if1
	lfd	rf3,d6if3
	lfd	rf4,d6if4
	lfd	rf5,d6if5
	lfd	rf7,d6if7
	lfd	rf8,d6if8
	lfd	rf9,d6if9
	lfd	rf10,d6if10
	lfd	rf12,d6if12
	lfd	rf14,d6if14
	lfd	rf16,d6if16
	lfd	rf19,d6if19
	lfd	rf20,d6if20
	lfd	rf21,d6if21
	lfd	rf22,d6if22
	lfd	rf25,d6if25
	lfd	rf28,d6if28
	lfd	rf29,d6if29
	lfd	rf30,d6if30
	lfd	rf31,d6if31
#
	lfd	rf2,d6fps
	mtfsf	0xff,rf2
	l	g6,d6icr
	mtcrf	0xff,g6
#
#
	mtfsb1  28                                         
	fm.     rf26,rf14,rf0                                 
	fnms    rf17,rf31,rf26,rf0                             
	fd       rf17,rf30,rf9                                
	fma     rf30,rf10,rf12,rf17                            
	fnabs.  rf8,rf30                                     
	fneg.   rf29,rf30                                    
	fneg.   rf8,rf10                                     
	mtfsf.  0x30,rf8		# b'0001100000',rf8
	fa.      rf18,rf3,rf8                                 
	fd.      rf8,rf18,rf9                                 
	fneg    rf18,rf8                                     
	fneg    rf9,rf18                                     
	fcmpu    3,rf20,rf18                                 
	mffs.   rf9                                         
	fcmpo    0,rf9,rf9                                   
	fd.      rf23,rf9,rf5                                 
	fcmpu    1,rf7,rf23                                  
	fnabs   rf19,rf23                                    
	fa.      rf11,rf19,rf5                                
	fneg    rf4,rf11                                     
	fs       rf11,rf28,rf30                               
	fm.     rf24,rf18,rf11                                
	mtfsb0. 24                                         
	mffs    rf11                                        
	fabs.   rf16,rf11                                    
	fnma    rf31,rf11,rf1,rf21                             
	fcmpu    6,rf31,rf31                                 
	fnms    rf20,rf25,rf31,rf31                            
	fa.      rf7,rf20,rf22                                
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1			# Preset return code = 1; fail
	stfd	rf4,here		# Check rf4
	l	g7,here
	l	g8,d6of4
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d6of4+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 2
	stfd	rf7,here		# Check rf7
	l	g7,here
	l	g8,d6of7
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d6of7+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 3
	stfd	rf8,here		# Check rf8
	l	g7,here
	l	g8,d6of8
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d6of8+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 4
	stfd	rf9,here		# Check rf9
	l	g7,here
	l	g8,d6of9
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d6of9+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 5
	stfd	rf11,here		# Check rf11
	l	g7,here
	l	g8,d6of11
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d6of11+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 6
	stfd	rf16,here		# Check rf16
	l	g7,here
	l	g8,d6of16
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d6of16+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 7
	stfd	rf17,here		# Check rf17
	l	g7,here
	l	g8,d6of17
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d6of17+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 8
	stfd	rf18,here		# Check rf18
	l	g7,here
	l	g8,d6of18
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d6of18+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 9
	stfd	rf19,here		# Check rf19
	l	g7,here
	l	g8,d6of19
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d6of19+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 10
	stfd	rf20,here		# Check rf20
	l	g7,here
	l	g8,d6of20
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d6of20+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 11
	stfd	rf23,here		# Check rf23
	l	g7,here
	l	g8,d6of23
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d6of23+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 12
	stfd	rf24,here		# Check rf24
	l	g7,here
	l	g8,d6of24
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d6of24+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 13
	stfd	rf26,here		# Check rf26
	l	g7,here
	l	g8,d6of26
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d6of26+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 14
	stfd	rf29,here		# Check rf29
	l	g7,here
	l	g8,d6of29
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d6of29+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 15
	stfd	rf30,here		# Check rf30
	l	g7,here
	l	g8,d6of30
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d6of30+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 16
	stfd	rf31,here		# Check rf31
	l	g7,here
	l	g8,d6of31
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d6of31+4
	cmp	0,g7,g8
	bne	0,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf0,d7if0
	lfd	rf3,d7if3
	lfd	rf5,d7if5
	lfd	rf7,d7if7
	lfd	rf9,d7if9
	lfd	rf10,d7if10
	lfd	rf13,d7if13
	lfd	rf14,d7if14
	lfd	rf15,d7if15
	lfd	rf16,d7if16
	lfd	rf17,d7if17
	lfd	rf18,d7if18
	lfd	rf20,d7if20
	lfd	rf22,d7if22
	lfd	rf25,d7if25
	lfd	rf27,d7if27
	lfd	rf28,d7if28
	lfd	rf29,d7if29
	lfd	rf30,d7if30
#
	lfd	rf1,d7fps
	mtfsf	0xff,rf1
	l	g6,d7icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
	fd       rf31,rf18,rf29                               
	fma.    rf11,rf16,rf31,rf13                            
	fneg    rf28,rf11                                    
	fneg    rf14,rf28                                    
	mtfsf.	0xc1,rf17			# b'0110000011',rf17
	fd       rf2,rf22,rf14                                
	fmr.    rf25,rf2                                     
	fneg.   rf15,rf2                                     
	fs       rf4,rf25,rf10                                
	fd       rf23,rf15,rf4                                
	fma.    rf23,rf30,rf9,rf20                             
	mcrfs   1,0                                        
	fm      rf23,rf20,rf7                                 
	fm      rf19,rf23,rf3                                 
	fms.    rf19,rf18,rf17,rf4                             
	frsp.   rf3,rf19                                     
	fcmpu    0,rf3,rf28                                  
	fmr     rf20,rf3                                     
	fnma.   rf9,rf20,rf0,rf20                              
	fcmpu    2,rf25,rf9                                  
	mtfsf   0x60,rf18			# b'0011000000',rf18
	fms.    rf22,rf2,rf31,rf11                             
	fms.    rf3,rf22,rf27,rf9                              
	fnma    rf9,rf5,rf22,rf20                              
	fmr.    rf18,rf9                                     
	mcrfs   1,1                                        
	fd.      rf17,rf9,rf9                                 
	fm.     rf21,rf17,rf17                                
	fabs.   rf2,rf21                                     
	fmr.    rf2,rf2                                      
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1			# Preset return code = 1; fail
	lfd	rf1,d7of2		# Load expected result of rf2 in rf1
	fcmpu	3,rf1,rf2		# Compare results
	bne	3,FINI			# Exit if not equal
#
	ai	g3,g3,1			# RC = 2
	lfd	rf1,d7of3		# Check rf3
	fcmpu	3,rf1,rf3
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 3
	lfd	rf1,d7of4		# Check rf4
	fcmpu	3,rf1,rf4
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 4
	lfd	rf1,d7of9		# Check rf9
	fcmpu	3,rf1,rf9
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 5
	lfd	rf1,d7of11		# Check rf11
	fcmpu	3,rf1,rf11
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 6
	lfd	rf1,d7of14		# Check rf14
	fcmpu	3,rf1,rf14
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 7
	lfd	rf1,d7of15		# Check rf15
	fcmpu	3,rf1,rf15
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 8
	lfd	rf1,d7of17		# Check rf17
	fcmpu	3,rf1,rf17
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 9
	lfd	rf1,d7of18		# Check rf18
	fcmpu	3,rf1,rf18
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 10
	lfd	rf1,d7of19		# Check rf19
	fcmpu	3,rf1,rf19
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 11
	lfd	rf1,d7of20		# Check rf20
	fcmpu	3,rf1,rf20
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 12
	lfd	rf1,d7of21		# Check rf21
	fcmpu	3,rf1,rf21
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 13
	lfd	rf1,d7of22		# Check rf22
	fcmpu	3,rf1,rf22
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 14
	lfd	rf1,d7of23		# Check rf23
	fcmpu	3,rf1,rf23
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 15
	lfd	rf1,d7of25		# Check rf25
	fcmpu	3,rf1,rf25
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 16
	lfd	rf1,d7of28		# Check rf28
	fcmpu	3,rf1,rf28
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 17
	lfd	rf1,d7of31		# Check rf31
	fcmpu	3,rf1,rf31
	bne	3,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf1,d8if1
	lfd	rf2,d8if2
	lfd	rf3,d8if3
	lfd	rf6,d8if6
	lfd	rf7,d8if7
	lfd	rf8,d8if8
	lfd	rf10,d8if10
	lfd	rf11,d8if11
	lfd	rf12,d8if12
	lfd	rf15,d8if15
	lfd	rf21,d8if21
	lfd	rf23,d8if23
	lfd	rf24,d8if24
	lfd	rf25,d8if25
	lfd	rf26,d8if26
	lfd	rf27,d8if27
	lfd	rf28,d8if28
	lfd	rf30,d8if30
	lfd	rf31,d8if31
#
	lfd	rf0,d8fps
	mtfsf	0xff,rf0
	l	g6,d8icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
	fmr     rf30,rf28                                    
	fnma    rf19,rf30,rf11,rf30                            
	fneg.   rf24,rf30                                    
	fnma    rf17,rf28,rf27,rf24                            
	frsp    rf4,rf24                                     
	fms     rf18,rf25,rf17,rf4                             
	fa.      rf13,rf4,rf18                                
	fmr     rf28,rf18                                    
	fcmpo    1,rf1,rf28                                  
	fm.     rf28,rf8,rf7                                  
	mcrfs   4,0                                        
	fma.    rf11,rf23,rf31,rf6                             
	fa.      rf22,rf10,rf11                               
	fms.    rf9,rf11,rf18,rf22                             
	fcmpo    7,rf22,rf17                                 
	fabs    rf9,rf9                                      
	mtfsf.  0xab,rf9			# b'1101010111',rf9
	fd       rf23,rf27,rf9                                
	fnma    rf5,rf23,rf23,rf31                             
	fnma    rf29,rf6,rf8,rf5                               
	fnabs.  rf15,rf29                                    
	mtfsb1  26                                         
	fnabs.  rf12,rf29                                    
	fs.      rf14,rf2,rf12                                
	fms     rf4,rf12,rf21,rf14                             
	fd.      rf4,rf13,rf1                                 
	fcmpo    0,rf4,rf26                                  
	fnabs   rf14,rf4                                     
	fnms.   rf20,rf3,rf14,rf17                             
	fms     rf14,rf28,rf9,rf25                             
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1			# Preset return code = 1; fail
	stfd	rf4,here		# Check rf4
	l	g7,here
	l	g8,d8of4
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d8of4+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 2
	stfd	rf5,here		# Check rf5
	l	g7,here
	l	g8,d8of5
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d8of5+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 3
	stfd	rf9,here		# Check rf9
	l	g7,here
	l	g8,d8of9
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d8of9+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 4
	stfd	rf11,here		# Check 11
	l	g7,here
	l	g8,d8of11
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d8of11+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 5
	stfd	rf12,here		# Check rf12
	l	g7,here
	l	g8,d8of12
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d8of12+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 6
	stfd	rf13,here		# Check rf13
	l	g7,here
	l	g8,d8of13
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d8of13+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 7
	stfd	rf14,here		# Check rf14
	l	g7,here
	l	g8,d8of14
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d8of14+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 8
	stfd	rf15,here		# Check rf15
	l	g7,here
	l	g8,d8of15
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d8of15+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 9
	stfd	rf17,here		# Check rf17
	l	g7,here
	l	g8,d8of17
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d8of17+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 10
	stfd	rf18,here		# Check rf18
	l	g7,here
	l	g8,d8of18
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d8of18+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 11
	stfd	rf19,here		# Check rf19
	l	g7,here
	l	g8,d8of19
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d8of19+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 12
	stfd	rf20,here		# Check rf20
	l	g7,here
	l	g8,d8of20
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d8of20+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 13
	stfd	rf22,here		# Check rf22
	l	g7,here
	l	g8,d8of22
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d8of22+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 14
	stfd	rf23,here		# Check rf23
	l	g7,here
	l	g8,d8of23
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d8of23+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 15
	stfd	rf24,here		# Check rf24
	l	g7,here
	l	g8,d8of24
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d8of24+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 16
	stfd	rf28,here		# Check rf28
	l	g7,here
	l	g8,d8of28
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d8of28+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 17
	stfd	rf29,here		# Check rf29
	l	g7,here
	l	g8,d8of29
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d8of29+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 18
	stfd	rf30,here		# Check rf30
	l	g7,here
	l	g8,d8of30
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d8of30+4
	cmp	0,g7,g8
	bne	0,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf3,d9if3
	lfd	rf5,d9if5
	lfd	rf6,d9if6
	lfd	rf7,d9if7
	lfd	rf9,d9if9
	lfd	rf13,d9if13
	lfd	rf14,d9if14
	lfd	rf16,d9if16
	lfd	rf17,d9if17
	lfd	rf18,d9if18
	lfd	rf20,d9if20
	lfd	rf21,d9if21
	lfd	rf22,d9if22
	lfd	rf23,d9if23
	lfd	rf24,d9if24
	lfd	rf25,d9if25
	lfd	rf27,d9if27
	lfd	rf30,d9if30
#
	lfd	rf1,d9fps
	mtfsf	0xff,rf1
	l	g6,d9icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
	mtfsb1  31                                         
	mtfsb0  11                                         
	fms.    rf31,rf6,rf14,rf7                              
	fnabs   rf23,rf31                                    
	fcmpo    3,rf23,rf5                                  
	mcrfs   1,4                                        
	mffs.   rf12                                        
	fm.     rf15,rf12,rf22                                
	fabs.   rf12,rf23                                    
	mtfsb0. 13                                         
	fms.    rf19,rf15,rf20,rf15                            
	fm      rf26,rf20,rf19                                
	mtfsb1. 2                                          
	fabs    rf25,rf19                                    
	fms     rf29,rf25,rf3,rf25                             
	fnabs   rf22,rf29                                    
	fnms.   rf7,rf29,rf23,rf22                             
	mtfsb0  0                                          
	fcmpu    3,rf24,rf22                                 
	fcmpo    0,rf3,rf9                                   
	fcmpu    7,rf13,rf23                                 
	fd.      rf0,rf29,rf17                                
	mtfsb0  25                                         
	fnms.   rf17,rf7,rf16,rf30                             
	mtfsf   0x62,rf5			# b'0011000101',rf5
	fnma    rf17,rf18,rf7,rf27                             
	fmr.    rf21,rf17                                    
	fs.      rf29,rf23,rf17                               
	mffs    rf28                                        
	fa       rf15,rf28,rf29                               
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1			# Preset return code = 1; fail
	stfd	rf0,here		# Check rf0
	l	g7,here
	l	g8,d9of0
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d9of0+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 2
	stfd	rf7,here		# Check rf7
	l	g7,here
	l	g8,d9of7
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d9of7+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 3
	lfd	rf1,d9of12		# Check rf12
	fcmpu	3,rf1,rf12
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 4
	stfd	rf15,here		# Check rf15
	l	g7,here
	l	g8,d9of15
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d9of15+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 5
	stfd	rf17,here		# Check rf17
	l	g7,here
	l	g8,d9of17
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d9of17+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 6
	stfd	rf19,here		# Check rf19
	l	g7,here
	l	g8,d9of19
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d9of19+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 7
	stfd	rf21,here		# Check rf21
	l	g7,here
	l	g8,d9of21
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d9of21+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 8
	stfd	rf22,here		# Check rf22
	l	g7,here
	l	g8,d9of22
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d9of22+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 9
	lfd	rf1,d9of23		# Check rf23
	fcmpu	3,rf1,rf23
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 10
	stfd	rf25,here		# Check rf25
	l	g7,here
	l	g8,d9of25
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d9of25+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 11
	stfd	rf26,here		# Check rf26
	l	g7,here
	l	g8,d9of26
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d9of26+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 12
	stfd	rf28,here		# Check rf28
	l	g7,here
	l	g8,d9of28
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d9of28+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 13
	stfd	rf29,here		# Check rf29
	l	g7,here
	l	g8,d9of29
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d9of29+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 14
	lfd	rf1,d9of31		# Check rf31
	fcmpu	3,rf1,rf31
	bne	3,FINI
# 
#				# Loading floating pt registers for computation
#
	lfd	rf0,d10if0
	lfd	rf5,d10if5
	lfd	rf6,d10if6
	lfd	rf7,d10if7
	lfd	rf11,d10if11
	lfd	rf16,d10if16
	lfd	rf18,d10if18
	lfd	rf21,d10if21
	lfd	rf23,d10if23
	lfd	rf25,d10if25
	lfd	rf26,d10if26
	lfd	rf29,d10if29
	lfd	rf30,d10if30
	lfd	rf31,d10if31
#
	lfd	rf1,d10fps
	mtfsf	0xff,rf1
	l	g6,d10icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
	fm      rf10,rf16,rf11                                
	fma     rf15,rf25,rf0,rf10                             
	fm.     rf15,rf7,rf26                                 
	fnma    rf24,rf23,rf15,rf15                            
	frsp.   rf28,rf24                                    
	fcmpo    1,rf24,rf5                                  
	mcrfs   4,5                                        
	fs       rf16,rf11,rf28                               
	fa       rf26,rf0,rf16                                
	fnabs   rf30,rf16                                    
	fnms    rf16,rf18,rf26,rf15                            
	fms.    rf0,rf30,rf30,rf30                             
	mcrfs   7,3                                        
	mtfsb1  31                                         
	fa.      rf27,rf16,rf5                                
	fnma    rf15,rf27,rf21,rf11                            
	mcrfs   7,7                                        
	fnms    rf4,rf27,rf27,rf27                             
	mtfsf   0x82,rf4			# b'1100000100',rf4
	mcrfs   3,7                                        
	mffs.   rf4                                         
	fneg.   rf4,rf5                                      
	fnms.   rf23,rf4,rf4,rf4                               
	fneg    rf29,rf23                                    
	fmr     rf23,rf23                                    
	fnma.   rf13,rf10,rf6,rf29                             
	fmr     rf30,rf13                                    
	fs.      rf9,rf5,rf13                                 
	fm.     rf30,rf31,rf26                                
	fd.      rf30,rf13,rf31                               
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1			# Preset return code = 1; fail
	stfd	rf0,here		# Check rf0
	l	g7,here
	l	g8,d10of0
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d10of0+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 2
	lfd	rf1,d10of4		# Check rf4
	fcmpu	3,rf1,rf4
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 3
	lfd	rf1,d10of9		# Check rf9
	fcmpu	3,rf1,rf9
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 4
	lfd	rf1,d10of10		# Check rf10
	fcmpu	3,rf1,rf10
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 5
	lfd	rf1,d10of13		# Check rf13
	fcmpu	3,rf1,rf13
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 6
	stfd	rf15,here		# Check rf15
	l	g7,here
	l	g8,d10of15
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d10of15+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 7
	stfd	rf16,here		# Check rf16
	l	g7,here
	l	g8,d10of16
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d10of16+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 8
	lfd	rf1,d10of23		# Check rf23
	fcmpu	3,rf1,rf23
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 9
	stfd	rf24,here		# Check rf24
	l	g7,here
	l	g8,d10of24
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d10of24+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 10
	stfd	rf26,here		# Check rf26
	l	g7,here
	l	g8,d10of26
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d10of26+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 11
	stfd	rf27,here		# Check rf27
	l	g7,here
	l	g8,d10of27
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d10of27+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 12
	stfd	rf28,here		# Check rf28
	l	g7,here
	l	g8,d10of28
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d10of28+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1			# RC = 13
	lfd	rf1,d10of29		# Check rf29
	fcmpu	3,rf1,rf29
	bne	3,FINI
#
	ai	g3,g3,1			# RC = 14
	lfd	rf1,d10of30		# Check rf30
	fcmpu	3,rf1,rf30
	bne	3,FINI
# 
	xor	g3,g3,g3		# Return code = 0; pass
FINI:	br
#
	.csect	_decfull[RW],3
_decfull: 
here:	.long	0xffffffff
	.long	0xffffffff
d1if1:	.long	0x0312467f
	.long	0x810a7de7                                    
d1if2:	.long	0x359c7da2
	.long	0x1ac09561                                    
d1if4:	.long	0x8c8df3e4
	.long	0xc1522ad7                                    
d1if5:	.long	0x7ad7cda7
	.long	0x3155b8ed                                    
d1if6:	.long	0xffffffff
	.long	0xfffff0fb                                    
d1if7:	.long	0x268131a2
	.long	0xcd250dba                                    
d1if8:	.long	0x819b7308
	.long	0x92448f82                                    
d1if13:	.long	0x30544678
	.long	0x12350dcd                                    
d1if15:	.long	0xbefe68ca
	.long	0x39f70856                                    
d1if18:	.long	0x60c8f0fd
	.long	0x9b318d7c                                    
d1if19:	.long	0x4008fe70
	.long	0xc1392d9f                                    
d1if21:	.long	0x1b539db6
	.long	0x93e0c109                                    
d1if22:	.long	0x0312467f
	.long	0x010a7de6                                    
d1if23:	.long	0x9af46fa9
	.long	0x170d193d                                    
d1if25:	.long	0x8232dfc4
	.long	0x46b9ea0b                                    
d1if26:	.long	0xc60363d6
	.long	0x46c2ef76                                    
d1if28:	.long	0xc7dffffe
	.long	0x30a97bca                                    
d1if29:	.long	0xe63f4908
	.long	0xea550045                                    
d1if30:	.long	0x80082b1f
	.long	0x5eefe148                                    
d1fps:	.long	0xffffffff
	.long	0x00000000
d1icr:	.long	0x84ea4cf2
#
#results:
#
d1of0:	.long	0xc7dffffe
	.long	0x40000000                                    
d1of3:	.long	0xa5fbdf75
	.long	0x764e0f57                                    
d1of7:	.long	0xffffffff
	.long	0x80000000                                    
d1of11:	.long	0xa5fbdf75
	.long	0x764e0f57                                    
d1of12:	.long	0xdfd7b39a
	.long	0x982587bb                                    
d1of16:	.long	0xffffffff
	.long	0x82022000                                    
d1of20:	.long	0xa5fbdfaa
	.long	0x6fd712d0                                    
d1of22:	.long	0x25fbdf75
	.long	0x764e0f58                                    
d1of23:	.long	0xffffffff
	.long	0x80000000                                    
d1of24:	.long	0xa5fbdfaa
	.long	0x6fd712d1                                    
d1of25:	.long	0x25fbdfaa
	.long	0x6fd712d1                                    
d1of28:	.long	0xffffffff
	.long	0x80000000                                    
d1of29:	.long	0x25fbdfaa
	.long	0x6fd712d1                                    
d1of31:	.long	0xffffffff
	.long	0x80000000                                    
# 
d2if0:	.long	0x7fefffff
	.long	0xffffffff                                    
d2if1:	.long	0xffefffff
	.long	0xffffffff                                    
d2if2:	.long	0xe65ee3d2
	.long	0x8559df93                                    
d2if6:	.long	0xfc840000
	.long	0x00000000                                    
d2if9:	.long	0xffeffffc
	.long	0x00000000                                    
d2if10:	.long	0x7fd00000
	.long	0x0000007f                                    
d2if12:	.long	0x699e5a1d
	.long	0x7e2bbc87                                    
d2if13:	.long	0xc40fffff
	.long	0xffffffff                                    
d2if14:	.long	0xbff00000
	.long	0x07ffffff                                    
d2if17:	.long	0x7cb857dd
	.long	0xd1f3fa57                                    
d2if19:	.long	0x6a164082
	.long	0xc489b16e                                    
d2if20:	.long	0xfff0d6f7
	.long	0x98c6a021                                    
d2if22:	.long	0xffd00000
	.long	0x00000003                                    
d2if24:	.long	0x7fefffff
	.long	0xffffffff                                    
d2if25:	.long	0xfc800000
	.long	0x00000000                                    
d2if27:	.long	0x7feffffe
	.long	0x00000000                                    
d2if28:	.long	0x7fefffff
	.long	0xf8000000                                    
d2if29:	.long	0xe106d519
	.long	0x90b4ca9d                                    
d2fps:	.long	0xffffffff
	.long	0x00000000
d2icr:	.long	0x09bdf4af
#
#results:
#
d2of1:	.long	0x7ff8d6f7
	.long	0x80000000                                    
d2of4:	.long	0x7ff8d6f7
	.long	0x80000000                                    
d2of6:	.long	0x7ff8d6f7
	.long	0x80000000                                    
d2of7:	.long	0xffffffff
	.long	0xa1091000                                    
d2of11:	.long	0xfff0d6f7
	.long	0x98c6a021                                    
d2of12:	.long	0x7ff8d6f7
	.long	0x80000000                                    
d2of13:	.long	0x7ff8d6f7
	.long	0x80000000                                    
d2of19:	.long	0x7ff8d6f7
	.long	0x80000000                                    
d2of21:	.long	0x7ff8d6f7
	.long	0x80000000                                    
d2of23:	.long	0xfff8d6f7
	.long	0x98c6a021                                    
d2of25:	.long	0x7ff8d6f7
	.long	0x80000000                                    
d2of27:	.long	0x7ff00000
	.long	0x00000000                                    
d2of29:	.long	0x7ff8d6f7
	.long	0x80000000                                    
d2of31:	.long	0x7ff8d6f7
	.long	0x80000000                                    
# 
d3if0:	.long	0x800018b9
	.long	0x0180f7ce                                    
d3if1:	.long	0x80000000
	.long	0x0eab3e24                                    
d3if2:	.long	0x80000000
	.long	0x00000bc5                                    
d3if4:	.long	0x1c3c5642
	.long	0xa4efdbdc                                    
d3if7:	.long	0x1881e337
	.long	0x9c813684                                    
d3if10:	.long	0x238828e2
	.long	0xc67d99f6                                    
d3if12:	.long	0x800fffff
	.long	0xffffffc0                                    
d3if13:	.long	0x81646c81
	.long	0x374e3bce                                    
d3if15:	.long	0xb69ffede
	.long	0xa57d32bd                                    
d3if16:	.long	0x551bd917
	.long	0x94e78c6d                                    
d3if17:	.long	0x1ed580c4
	.long	0x3b73fe4d                                    
d3if18:	.long	0xa0000000
	.long	0x0000001f                                    
d3if19:	.long	0x80000000
	.long	0x00000098                                    
d3if21:	.long	0xffffffff
	.long	0x5ca08030                                    
d3if27:	.long	0x9402b57d
	.long	0xda8ad1ff                                    
d3if28:	.long	0x19669129
	.long	0xb0f08eaa                                    
d3if29:	.long	0xa0000000
	.long	0x003fffff                                    
d3if31:	.long	0x906b42d3
	.long	0xc20f2cfa                                    
d3fps:	.long	0xffffffff
	.long	0x00000000
d3icr:	.long	0x622ee4e7
#
#results:
#
d3of3:	.long	0x7ff80000
	.long	0x00000000                                    
d3of4:	.long	0x7ff80000
	.long	0x00000000                                    
d3of6:	.long	0x1881e337
	.long	0x9c813684                                    
d3of9:	.long	0x80000000
	.long	0x00000000                                    
d3of10:	.long	0x16b00000
	.long	0x0000001f                                    
d3of12:	.long	0xb871e337
	.long	0x9c39a9a7                                    
d3of13:	.long	0x7ff80000
	.long	0x00000000                                    
d3of16:	.long	0x7ff80000
	.long	0x00000000                                    
d3of17:	.long	0x96b00000
	.long	0x0000001f                                    
d3of23:	.long	0x80000000
	.long	0x00000098                                    
d3of24:	.long	0x7ff00000
	.long	0x00000000                                    
d3of26:	.long	0xb6a00000
	.long	0x00000000                                    
d3of27:	.long	0x7ff80000
	.long	0x00000000                                    
d3of28:	.long	0xb6a00000
	.long	0x00000000                                    
# 
d4if2:	.long	0x80000000
	.long	0x00000000                                    
d4if3:	.long	0x026cf9fb
	.long	0x6a67acf3                                    
d4if4:	.long	0x7ff48277
	.long	0x0a9764df                                    
d4if5:	.long	0x7ff00000
	.long	0x00000000                                    
d4if6:	.long	0x058f5655
	.long	0x23cbbd12                                    
d4if7:	.long	0xb7718942
	.long	0x982a0139                                    
d4if8:	.long	0xfffc243c
	.long	0xc78ecc53                                    
d4if11:	.long	0x7ff6d554
	.long	0x8e923e14                                    
d4if13:	.long	0x7ea13ed2
	.long	0x09797c65                                    
d4if14:	.long	0x7ff85ef7
	.long	0xee4c6651                                    
d4if16:	.long	0xfff00000
	.long	0x00000000                                    
d4if18:	.long	0x064737c3
	.long	0x8691030e                                    
d4if20:	.long	0xfffcfecc
	.long	0x15576d73                                    
d4if21:	.long	0x7ff00000
	.long	0x00000000                                    
d4if22:	.long	0x890ef173
	.long	0x5af72bd3                                    
d4if27:	.long	0xfff00000
	.long	0x00000000                                    
d4if28:	.long	0x00000000
	.long	0x00000000                                    
d4if29:	.long	0x7ffdf54d
	.long	0x6cca584b                                    
d4if31:	.long	0x7ff00000
	.long	0x00000000                                    
d4fps:	.long	0xffffffff
	.long	0x00000000
d4icr:	.long	0x6389fe87
#
#results:
#
d4of3:	.long	0x7ff85ef7
	.long	0xee4c6651                                    
d4of6:	.long	0xfffcfecc
	.long	0x15576d73                                    
d4of7:	.long	0xfffc243c
	.long	0xc78ecc53                                    
d4of9:	.long	0x7ff85ef7
	.long	0xee4c6651                                    
d4of10:	.long	0xfff85ef7
	.long	0xee4c6651                                    
d4of13:	.long	0x7ff85ef7
	.long	0xee4c6651                                    
d4of15:	.long	0x7ff85ef7
	.long	0xee4c6651                                    
d4of17:	.long	0x7ff85ef7
	.long	0xee4c6651                                    
d4of28:	.long	0x00000000
	.long	0x00000000                                    
d4of30:	.long	0x7ff85ef7
	.long	0xee4c6651                                    
d4of31:	.long	0x7ff85ef7
	.long	0xee4c6651                                    
# 
d5if0:	.long	0x0537e48d
	.long	0x55d158d1                                    
d5if1:	.long	0xd0ed7b93
	.long	0xb9e58954                                    
d5if2:	.long	0x464d87e5
	.long	0x04b260f2                                    
d5if3:	.long	0x7ff00000
	.long	0x00000000                                    
d5if4:	.long	0xfff00000
	.long	0x00000000                                    
d5if6:	.long	0xfff00000
	.long	0x00000000                                    
d5if7:	.long	0x7ff00000
	.long	0x00000000                                    
d5if13:	.long	0xfff00000
	.long	0x00000000                                    
d5if14:	.long	0xa1d53983
	.long	0x4679cf5a                                    
d5if16:	.long	0x7ff00000
	.long	0x00000000                                    
d5if17:	.long	0x589ce7eb
	.long	0xc8416cc4                                    
d5if22:	.long	0x80000000
	.long	0x00000000                                    
d5if25:	.long	0x7fffe6ae
	.long	0xf2632d90                                    
d5if26:	.long	0x7ff00000
	.long	0x00000000                                    
d5fps:	.long	0xffffffff
	.long	0x00000000
d5icr:	.long	0xf4760edf
#
#results:
#
d5of1:	.long	0xd89ce7eb
	.long	0xc8416cc4                                    
d5of4:	.long	0x7ff80000
	.long	0x00000000                                    
d5of5:	.long	0x7fffe6ae
	.long	0xf2632d90                                    
d5of7:	.long	0x7ff80000
	.long	0x00000000                                    
d5of8:	.long	0xffffffff
	.long	0xa0c11000                                    
d5of9:	.long	0x7fffffff
	.long	0xa0c11000                                    
d5of10:	.long	0xffffffff
	.long	0xa0c11000                                    
d5of11:	.long	0xffffffff
	.long	0x20000000                                    
d5of13:	.long	0x7fffe6ae
	.long	0xe0000000                                    
d5of15:	.long	0xfff00000
	.long	0x00000000                                    
d5of16:	.long	0xfff00000
	.long	0x00000000                                    
d5of19:	.long	0xfff00000
	.long	0x00000000                                    
d5of20:	.long	0xfff00000
	.long	0x00000000                                    
d5of21:	.long	0x7fffe6ae
	.long	0xe0000000                                    
d5of23:	.long	0x7fffe6ae
	.long	0xf2632d90                                    
d5of24:	.long	0xfff00000
	.long	0x00000000                                    
d5of30:	.long	0xfff00000
	.long	0x00000000                                    
d5of31:	.long	0xffffffff
	.long	0x20c11000                                    
#
d6if0:	.long	0xf68fd43e
	.long	0x9b56ad68                                    
d6if1:	.long	0x7cfbdfc7
	.long	0x85e32e99                                    
d6if3:	.long	0x7fefffff
	.long	0xffffff00                                    
d6if4:	.long	0xfa59bde6
	.long	0x30c1c578                                    
d6if5:	.long	0x18124ac7
	.long	0x389a3df5                                    
d6if7:	.long	0x001fffff
	.long	0xffffff00                                    
d6if8:	.long	0xb82cbfa2
	.long	0x8e5f8b17                                    
d6if9:	.long	0x2088a164
	.long	0xbed36ae5                                    
d6if10:	.long	0x65800c3b
	.long	0x42a9039c                                    
d6if12:	.long	0xf658c163
	.long	0x4e284f36                                    
d6if14:	.long	0xe3ecce2d
	.long	0x6aab1919                                    
d6if16:	.long	0x21d3a188
	.long	0xfac49027                                    
d6if19:	.long	0xaf5f445d
	.long	0x4dc023fb                                    
d6if20:	.long	0x7c9c0000
	.long	0x00000000                                    
d6if21:	.long	0xe20b3236
	.long	0x8bbc8924                                    
d6if22:	.long	0xff5fffff
	.long	0xffffffff                                    
d6if25:	.long	0xfcd01c9a
	.long	0xf2d27ddb                                    
d6if28:	.long	0x7fefffff
	.long	0xfffc0000                                    
d6if29:	.long	0xd1020a58
	.long	0x79fb7fcb                                    
d6if30:	.long	0xe1079904
	.long	0xfc5e3b45                                    
d6if31:	.long	0xfcfeeb92
	.long	0xead78d03                                    
d6fps:	.long	0xffffffff
	.long	0x00000000
d6icr:	.long	0x1f64f81b
#
#results                                                               
#
d6of4:	.long	0x7fffffff
	.long	0xf2ae4008                                    
d6of7:	.long	0xffffffff
	.long	0xf2a89008                                    
d6of8:	.long	0x7ff00000
	.long	0x00000000                                    
d6of9:	.long	0xffffffff
	.long	0xf2ae4008                                    
d6of11:	.long	0xffffffff
	.long	0xf2a89008                                    
d6of16:	.long	0x7fffffff
	.long	0xf2a89008                                    
d6of17:	.long	0xfff00000
	.long	0x00000000                                    
d6of18:	.long	0xfff00000
	.long	0x00000000                                    
d6of19:	.long	0xffffffff
	.long	0xf2ae4008                                    
d6of20:	.long	0xffffffff
	.long	0xf2a89008                                    
d6of23:	.long	0xffffffff
	.long	0xf2ae4008                                    
d6of24:	.long	0xfff00000
	.long	0x00000000                                    
d6of26:	.long	0x7ff00000
	.long	0x00000000                                    
d6of29:	.long	0x7ff00000
	.long	0x00000000                                    
d6of30:	.long	0xfff00000
	.long	0x00000000                                    
d6of31:	.long	0xffffffff
	.long	0xf2a89008                                    
# 
d7if0:	.long	0x0b662c6c
	.long	0x117f2729                                    
d7if3:	.long	0x83ab062e
	.long	0xd6366e8b                                    
d7if5:	.long	0x1f7b2408
	.long	0xe2b0211c                                    
d7if7:	.long	0x17bba3fe
	.long	0xfc2ac757                                    
d7if9:	.long	0x03c2b0a7
	.long	0x2f6e9ba0                                    
d7if10:	.long	0x000fffff
	.long	0xffff8000                                    
d7if13:	.long	0x1d5a7066
	.long	0x9bd355ef                                    
d7if14:	.long	0x096dcbc7
	.long	0xcc5b6b2b                                    
d7if15:	.long	0xfd7e2a94
	.long	0xfd34637f                                    
d7if16:	.long	0x8db4c9cd
	.long	0x97784c1f                                    
d7if17:	.long	0xbe997531
	.long	0x5e1550b3                                    
d7if18:	.long	0x80e36817
	.long	0xe363485d                                    
d7if20:	.long	0x896572eb
	.long	0xa92474b7                                    
d7if22:	.long	0x22d5fa9a
	.long	0x54ff4dbd                                    
d7if25:	.long	0xc6092227
	.long	0x4196eac3                                    
d7if27:	.long	0x9c3d973f
	.long	0xd1170277                                    
d7if28:	.long	0xa41ebd4a
	.long	0x300377fa                                    
d7if29:	.long	0xdfbee9d7
	.long	0x8c6c9544                                    
d7if30:	.long	0x0021060e
	.long	0x29bc9676                                    
d7fps:	.long	0xffffffff
	.long	0x00000000
d7icr:	.long	0x8283db60
#
#results                                                               
#
d7of2:	.long	0x3ff00000
	.long	0x00000000                                    
d7of3:	.long	0xc56a9a0c
	.long	0x40000001                                    
d7of4:	.long	0x456a9a0c
	.long	0x24d5ee38                                    
d7of9:	.long	0x456a9a0c
	.long	0x40000001                                    
d7of11:	.long	0x1d5a7066
	.long	0x9bd355ef                                    
d7of14:	.long	0x1d5a7066
	.long	0x9bd355ef                                    
d7of15:	.long	0xc56a9a0c
	.long	0x24d5ee39                                    
d7of17:	.long	0x3ff00000
	.long	0x00000000                                    
d7of18:	.long	0x456a9a0c
	.long	0x40000001                                    
d7of19:	.long	0xc56a9a0c
	.long	0x24d5ee38                                    
d7of20:	.long	0xc56a9a0c
	.long	0x40000000                                    
d7of21:	.long	0x3ff00000
	.long	0x00000000                                    
d7of22:	.long	0x9d5a7066
	.long	0x9bd355ef                                    
d7of23:	.long	0x80000000
	.long	0x00000001                                    
d7of25:	.long	0x456a9a0c
	.long	0x24d5ee39                                    
d7of28:	.long	0x9d5a7066
	.long	0x9bd355ef                                    
d7of31:	.long	0x00000000
	.long	0x00000000                                    
# 
d8if1:	.long	0x7fefffff
	.long	0xffffffff                                    
d8if2:	.long	0x7ff00000
	.long	0x00000000                                    
d8if3:	.long	0x7ff00000
	.long	0x00000000                                    
d8if6:	.long	0xfff00000
	.long	0x00000000                                    
d8if7:	.long	0x8c642192
	.long	0xf2b41a01                                    
d8if8:	.long	0x7ff00000
	.long	0x00000000                                    
d8if10:	.long	0xfffb12bd
	.long	0x0c30b874                                    
d8if11:	.long	0xf33b33c4
	.long	0xc51a39e8                                    
d8if12:	.long	0xf1057541
	.long	0x50462704                                    
d8if15:	.long	0x64c492ac
	.long	0xec0894f9                                    
d8if21:	.long	0x94ef9a50
	.long	0x86bc754b                                    
d8if23:	.long	0x7fffb60e
	.long	0xaf0b43e9                                    
d8if24:	.long	0x90362432
	.long	0xc6fde21c                                    
d8if25:	.long	0x7ff00000
	.long	0x00000000                                    
d8if26:	.long	0x7ffebd25
	.long	0x8f2dad0c                                    
d8if27:	.long	0x3f919260
	.long	0x0365aa29                                    
d8if28:	.long	0x7ff7a443
	.long	0x12f15ed1                                    
d8if30:	.long	0xf09499c3
	.long	0x6bf380f8                                    
d8if31:	.long	0x39936b69
	.long	0x5a4b5079                                    
d8fps:	.long	0xffffffff
	.long	0x00000000
d8icr:	.long	0x34f18b88
#
#results                                                                
#
d8of4:	.long	0xffffa443
	.long	0x00000000                                    
d8of5:	.long	0x7fffb60e
	.long	0xaf0b43e9                                    
d8of9:	.long	0x7fffb60e
	.long	0xaf0b43e9                                    
d8of11:	.long	0x7fffb60e
	.long	0xaf0b43e9                                    
d8of12:	.long	0xffffb60e
	.long	0xaf0b43e9                                    
d8of13:	.long	0xffffa443
	.long	0x00000000                                    
d8of14:	.long	0x7fffb60e
	.long	0xaf0b43e9                                    
d8of15:	.long	0xffffb60e
	.long	0xaf0b43e9                                    
d8of17:	.long	0x7fffa443
	.long	0x12f15ed1                                    
d8of18:	.long	0xffffa443
	.long	0x00000000                                    
d8of19:	.long	0x7fffa443
	.long	0x12f15ed1                                    
d8of20:	.long	0x7fffa443
	.long	0x12f15ed1                                    
d8of22:	.long	0xfffb12bd
	.long	0x0c30b874                                    
d8of23:	.long	0x7fffb60e
	.long	0xaf0b43e9                                    
d8of24:	.long	0xfff7a443
	.long	0x12f15ed1                                    
d8of28:	.long	0xfff00000
	.long	0x00000000                                    
d8of29:	.long	0x7fffb60e
	.long	0xaf0b43e9                                    
d8of30:	.long	0x7ff7a443
	.long	0x12f15ed1                                    
# 
d9if3:	.long	0x071dffff
	.long	0xffffffff                                    
d9if5:	.long	0xe7ed092e
	.long	0x22f07ad0                                    
d9if6:	.long	0x1f6fffff
	.long	0xffffffff                                    
d9if7:	.long	0x03bfffff
	.long	0xfffffffe                                    
d9if9:	.long	0x8060fdc9
	.long	0xf0a1486f                                    
d9if13:	.long	0x0207a3ca
	.long	0x6e733c44                                    
d9if14:	.long	0x243fffff
	.long	0xffffffff                                    
d9if16:	.long	0x294899ef
	.long	0x10924b7d                                    
d9if17:	.long	0xfff00000
	.long	0x00000000                                    
d9if18:	.long	0x7fe00000
	.long	0x03ffffff                                    
d9if20:	.long	0xb09fffef
	.long	0xffffffff                                    
d9if21:	.long	0x9e5303c6
	.long	0x0fb7ffc7                                    
d9if22:	.long	0x000fffff
	.long	0xffffffff                                    
d9if23:	.long	0x7c91706a
	.long	0xf5146c1d                                    
d9if24:	.long	0xfff00000
	.long	0x00000000                                    
d9if25:	.long	0x98a0f6cd
	.long	0x20bbdb55                                    
d9if27:	.long	0x7fefffff
	.long	0xffffffff                                    
d9if30:	.long	0xf1b86145
	.long	0xc29cebf2                                    
d9fps:	.long	0xffffffff
	.long	0x00000000
d9icr:	.long	0x2a0ff04d
#
#results                                                               
#
d9of0:	.long	0x7fffffff
	.long	0x00014001                                    
d9of7:	.long	0x7fffffff
	.long	0x00014001                                    
d9of12:	.long	0x00000000
	.long	0x00000020                                    
d9of15:	.long	0xffffffff
	.long	0x62f110d1                                    
d9of17:	.long	0x7fffffff
	.long	0x00014001                                    
d9of19:	.long	0xffffffff
	.long	0x00014001                                    
d9of21:	.long	0x7fffffff
	.long	0x00014001                                    
d9of22:	.long	0xffffffff
	.long	0x00014001                                    
d9of23:	.long	0x80000000
	.long	0x00000020                                    
d9of25:	.long	0x7fffffff
	.long	0x00014001                                    
d9of26:	.long	0xffffffff
	.long	0x00014001                                    
d9of28:	.long	0xffffffff
	.long	0x62f110d1                                    
d9of29:	.long	0x7fffffff
	.long	0x00014001                                    
d9of31:	.long	0x00000000
	.long	0x00000020                                    
# 
d10if0:		.long	0xd9200000
		.long	0x00000001                                    
d10if5:		.long	0x7edfffff
		.long	0xffffffff                                    
d10if6:		.long	0x9c431905
		.long	0x6403ac53                                    
d10if7:		.long	0xfff7bb9a
		.long	0xd010c6a7                                    
d10if11:	.long	0x000fffff
		.long	0xffffffff                                    
d10if16:	.long	0xbff00000
		.long	0x00000000                                    
d10if18:	.long	0x80000000
		.long	0x0026c5dd                                    
d10if21:	.long	0xbff00fff
		.long	0xffffffff                                    
d10if23:	.long	0xffef7deb
		.long	0x565338f9                                    
d10if25:	.long	0xe6bfffff
		.long	0xffffffff                                    
d10if26:	.long	0xfe628165
		.long	0x9e389898                                    
d10if29:	.long	0x1c82e3e7
		.long	0x26fd3793                                    
d10if30:	.long	0xd7c46c92
		.long	0x2e1e1a20                                    
d10if31:	.long	0x1160c224
		.long	0xd264db1b                                    
d10fps:		.long	0xffffffff
		.long	0x00000000
d10icr:		.long	0xfa33edd1
#
#results                                                                
#
d10of0:		.long	0xffffbb9a
		.long	0xc0000000                                    
d10of4:		.long	0xfedfffff
		.long	0xffffffff                                    
d10of9:		.long	0x7fefffff
		.long	0xffffffff                                    
d10of10:	.long	0x800fffff
		.long	0xffffffff                                    
d10of13:	.long	0xffefffff
		.long	0xffffffff                                    
d10of15:	.long	0xffffbb9a
		.long	0xd010c6a7                                    
d10of16:	.long	0xffffbb9a
		.long	0xd010c6a7                                    
d10of23:	.long	0xffefffff
		.long	0xffffffff                                    
d10of24:	.long	0xffffbb9a
		.long	0xd010c6a7                                    
d10of26:	.long	0xffffbb9a
		.long	0xc0000000                                    
d10of27:	.long	0xffffbb9a
		.long	0xd010c6a7                                    
d10of28:	.long	0xffffbb9a
		.long	0xc0000000                                    
d10of29:	.long	0x7fefffff
		.long	0xffffffff                                    
d10of30:	.long	0xffefffff
		.long	0xffffffff                                    
#
	.toc
	TOCE(.decfull, entry)
	TOCL(_decfull, data)
