# @(#)04	1.3  src/bos/diag/da/fpa/div.s, dafpa, bos411, 9428A410j 3/23/94 06:22:12

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
# header   exercising data dependencies and                                     
# comment  new floating pt biasing flpdata 0 = 2 full random                    
#
#		Appropriate registers are loaded with input values. Then
#		instructions for this module will be executed and results
#		will be compared with expected results. If any test fails,
#		a particular return code will be returned to the caller.
#
	.csect	.div_fpp[PR]
	.globl	.div_fpp[PR]
	.align	2
.div_fpp:
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
	LTOC(4, _div_fpp, data)		# Loads g4 with the address of _div
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
 	.using	_div_fpp[RW],g4		# g4 is the base register to the data
#	 				     section _div[rw]. Eastablishing 
#					     @ability
#
#	
	xor	g3,g3,g3		# g3 = 0. Initialize return code
	ai	g3,g3,1			# Preset return code = 1; fail
#
#				# Loading floating pt registers for computation
#
	lfd	rf2,d1if2
	lfd	rf3,d1if3
	lfd	rf4,d1if4
	lfd	rf5,d1if5
	lfd	rf10,d1if10
	lfd	rf11,d1if11
	lfd	rf21,d1if21
	lfd	rf26,d1if26
	lfd	rf30,d1if30
	lfd	rf31,d1if31
#
	lfd	rf1,d1fps
	mtfsf	0xff,rf1
	l	g6,d1icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
##  D1 *********
#
	fma     r9,r21,r5,r2                               
	fd.     r28,r30,r9                                 
	fd.     r28,r11,r31                                
	fd      r28,r28,r28                                
	fms     r19,r28,r28,r4                             
	fd.     r31,r19,r19                                
	fd      r1,r26,r31                                 
	fd      r11,r1,r1                                  
	fnma.   r11,r11,r3,r10                             
	fd.     r11,r3,r28                                 
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	lfd	rf0,d1of1
	fcmpu	3,rf0,rf1
	bne	3,FINI	
#
	ai	g3,g3,1
	lfd	rf1,d1of9
	fcmpu	3,rf1,rf9
	bne	3,FINI
#
	ai	g3,g3,1		
	lfd	rf1,d1of11		
	fcmpu	3,rf1,rf11
	bne	3,FINI
#
	ai	g3,g3,1		
	lfd	rf1,d1of19
	fcmpu	3,rf1,rf19
	bne	3,FINI
#
	ai	g3,g3,1	
	lfd	rf1,d1of28
	fcmpu	3,rf1,rf28
	bne	3,FINI
#
	ai	g3,g3,1	
	lfd	rf1,d1of31	
	fcmpu	3,rf1,rf31
	bne	3,FINI
#
#
##  D2 *********
#
#				# Loading floating pt registers for computation
#
	lfd	rf0,d2if0
	lfd	rf1,d2if1
	lfd	rf4,d2if4
	lfd	rf9,d2if9
	lfd	rf11,d2if11
	lfd	rf12,d2if12
	lfd	rf16,d2if16
	lfd	rf20,d2if20
	lfd	rf24,d2if24
	lfd	rf26,d2if26
	lfd	rf28,d2if28
	lfd	rf31,d2if31
#
 	lfd	rf2,d2fps
	mtfsf	0xff,rf2
	l	g6,d2icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
	fma     r6,r28,r31,r11                             
	fd.     r6,r12,r4                                  
	fd.     r6,r24,r31                                 
	fd.     r8,r6,r6                                   
	fms     r8,r8,r8,r26                               
	fd      r8,r1,r8                                   
	fd.     r8,r11,r9                                  
	fd      r2,r8,r0                                   
	fnma.   r29,r16,r2,r2                              
	fd      r20,r29,r29                                
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1		
	lfd	rf1,d2of2
	fcmpu	3,rf1,rf2
	bne	3,FINI	
#
	ai	g3,g3,1
	lfd	rf1,d2of6
	fcmpu	3,rf1,rf6
	bne	3,FINI
#
	ai	g3,g3,1		
	lfd	rf1,d2of8		
	fcmpu	3,rf1,rf8
	bne	3,FINI
#
	ai	g3,g3,1		
	lfd	rf1,d2of20
	fcmpu	3,rf1,rf20
	bne	3,FINI
#
	ai	g3,g3,1	
	lfd	rf1,d2of29
	fcmpu	3,rf1,rf29
	bne	3,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf4,d3if4
	lfd	rf9,d3if9
	lfd	rf10,d3if10
	lfd	rf11,d3if11
	lfd	rf17,d3if17
	lfd	rf18,d3if18
	lfd	rf23,d3if23
	lfd	rf24,d3if24
	lfd	rf29,d3if29
	lfd	rf31,d3if31
#
 	lfd	rf1,d3fps
	mtfsf	0xff,rf1
	l	g6,d3icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
#
##  D3 *********
#
	fma     r12,r10,r31,r18                            
	fd      r24,r9,r12                                 
	fd      r24,r24,r23                                
	fd.     r24,r24,r24                                
	fms     r24,r4,r11,r29                             
	fd.     r18,r24,r24                                
	fd      r8,r18,r29                                 
	fd      r8,r8,r8                                   
	fnma    r8,r8,r17,r8                               
	fd      r11,r8,r8                                  
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1		
	lfd	rf1,d3of8
	fcmpu	3,rf1,rf8
	bne	3,FINI	
#
	ai	g3,g3,1
	lfd	rf1,d3of11
	fcmpu	3,rf1,rf11
	bne	3,FINI
#
	ai	g3,g3,1		
	lfd	rf1,d3of12		
	fcmpu	3,rf1,rf12
	bne	3,FINI
#
	ai	g3,g3,1		
	lfd	rf1,d3of18
	fcmpu	3,rf1,rf18
	bne	3,FINI
#
	ai	g3,g3,1	
	lfd	rf1,d3of24
	fcmpu	3,rf1,rf24
	bne	3,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf1,d4if1
	lfd	rf7,d4if7
	lfd	rf8,d4if8
	lfd	rf12,d4if12
	lfd	rf14,d4if14
	lfd	rf23,d4if23
	lfd	rf24,d4if24
	lfd	rf28,d4if28
	lfd	rf30,d4if30
	lfd	rf31,d4if31
#
 	lfd	rf0,d4fps
	mtfsf	0xff,rf0
	l	g6,d4icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
#
##  D4 *********
#
	fma.    r4,r7,r28,r14                              
	fd      r25,r8,r4                                  
	fd      r19,r12,r25                                
	fd      r19,r19,r1                                 
	fms     r28,r19,r19,r19                            
	fd      r28,r23,r28                                
	fd      r20,r28,r28                                
	fd.     r8,r30,r20                                 
	fnma.   r8,r24,r31,r20                             
	fd.     r8,r24,r7                                  
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1		
	lfd	rf1,d4of4
	fcmpu	3,rf1,rf4
	bne	3,FINI	
#
	ai	g3,g3,1
	lfd	rf1,d4of8
	fcmpu	3,rf1,rf8
	bne	3,FINI
#
	ai	g3,g3,1		
	lfd	rf1,d4of19		
	fcmpu	3,rf1,rf19
	bne	3,FINI
#
	ai	g3,g3,1		
	stfd	rf20,here		# Check rf20
	l	g7,here
	l	g8,d4of20
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d4of20+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1	
	lfd	rf1,d4of25
	fcmpu	3,rf1,rf25
	bne	3,FINI
#
	ai	g3,g3,1	
	stfd	rf28,here
	l	g7,here
	l	g8,d4of28
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d4of28+4
	cmp	0,g7,g8
	bne	0,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf1,d5if1
	lfd	rf3,d5if3
	lfd	rf6,d5if6
	lfd	rf12,d5if12
	lfd	rf21,d5if21
	lfd	rf22,d5if22
	lfd	rf25,d5if25
	lfd	rf26,d5if26
	lfd	rf27,d5if27
	lfd	rf29,d5if29
#
 	lfd	rf2,d5fps
	mtfsf	0xff,rf2
	l	g6,d5icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
#
##  D5 *********
#
	fma     r15,r3,r12,r21                             
	fd      r6,r15,r15                                 
	fd      r30,r6,r6                                  
	fd      r4,r30,r29                                 
	fms.    r20,r22,r4,r4                              
	fd      r9,r20,r1                                  
	fd.     r9,r9,r9                                   
	fd.     r8,r9,r27                                  
	fnma    r8,r26,r10,r29                             
	fd.     r8,r8,r25                                  
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1		
	lfd	rf1,d5of4
	fcmpu	3,rf1,rf4
	bne	3,FINI	
#
	ai	g3,g3,1
	lfd	rf1,d5of6
	fcmpu	3,rf1,rf6
	bne	3,FINI
#
	ai	g3,g3,1		
	lfd	rf1,d5of8		
	fcmpu	3,rf1,rf8
	bne	3,FINI
#
	ai	g3,g3,1		
	lfd	rf1,d5of9
	fcmpu	3,rf1,rf9
	bne	3,FINI
#
	ai	g3,g3,1	
	lfd	rf1,d5of15
	fcmpu	3,rf1,rf15
	bne	3,FINI
#
	ai	g3,g3,1	
	lfd	rf1,d5of20	
	fcmpu	3,rf1,rf20
	bne	3,FINI
#
	ai	g3,g3,1	
	lfd	rf1,d5of30
	fcmpu	3,rf1,rf30
	bne	3,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf0,d6if0
	lfd	rf3,d6if3
	lfd	rf4,d6if4
	lfd	rf5,d6if5
	lfd	rf8,d6if8
	lfd	rf13,d6if13
	lfd	rf15,d6if15
	lfd	rf16,d6if16
	lfd	rf22,d6if22
	lfd	rf23,d6if23
	lfd	rf28,d6if28
#
	lfd	rf1,d6fps
	mtfsf	0xff,rf1
	l	g6,d6icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
#
##  D6 *********
#
	fma     r9,r28,r23,r8                              
	fd.     r25,r9,r3                                  
	fd      r10,r25,r25                                
	fd.     r10,r28,r9                                 
	fms.    r10,r15,r10,r10                            
	fd.     r23,r10,r10                                
	fd      r2,r23,r22                                 
	fd.     r2,r0,r5                                   
	fnma.   r11,r13,r2,r2                              
	fd.     r11,r16,r4                                 
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1		
	lfd	rf1,d6of2
	fcmpu	3,rf1,rf2
	bne	3,FINI	
#
	ai	g3,g3,1
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
	ai	g3,g3,1		
	stfd	rf10,here		# Check rf10
	l	g7,here
	l	g8,d6of10
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d6of10+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1		
	lfd	rf1,d6of11
	fcmpu	3,rf1,rf11
	bne	3,FINI
#
	ai	g3,g3,1	
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
	ai	g3,g3,1	
	stfd	rf25,here		# Check rf25
	l	g7,here
	l	g8,d6of25
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d6of25+4
	cmp	0,g7,g8
	bne	0,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf0,d7if0
	lfd	rf3,d7if3
	lfd	rf8,d7if8
	lfd	rf15,d7if15
	lfd	rf21,d7if21
	lfd	rf22,d7if22
#
	lfd	rf1,d7fps
	mtfsf	0xff,rf1
	l	g6,d7icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
#
##  D7 *********
#
	fma.    r24,r3,r15,r21                             
	fd.     r7,r0,r24                                  
	fd      r24,r7,r7                                  
	fd      r24,r24,r24                                
	fms     r24,r24,r24,r24                            
	fd.     r19,r24,r8                                 
	fd.     r5,r19,r19                                 
	fd.     r5,r5,r22                                  
	fnma    r22,r5,r5,r8                               
	fd.     r22,r22,r22                                
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1		
	stfd	rf5,here		# Check rf5
	l	g7,here
	l	g8,d7of5
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d7of5+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1
	stfd	rf7,here		# Check rf7
	l	g7,here
	l	g8,d7of7
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d7of7+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1		
	stfd	rf19,here		# Check rf19
	l	g7,here
	l	g8,d7of19
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d7of19+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1		
	stfd	rf22,here		# Check rf22
	l	g7,here
	l	g8,d7of22
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d7of22+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1	
	stfd	rf24,here		# Check rf24
	l	g7,here
	l	g8,d7of24
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d7of24+4
	cmp	0,g7,g8
	bne	0,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf1,d8if1
	lfd	rf7,d8if7
	lfd	rf9,d8if9
	lfd	rf10,d8if10
	lfd	rf19,d8if19
	lfd	rf28,d8if28
	lfd	rf29,d8if29
#
 	lfd	rf0,d8fps
	mtfsf	0xff,rf0
	l	g6,d8icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
##  D8 *********
#
	fma     r23,r1,r19,r10                             
	fd      r10,r28,r23                                
	fd.     r0,r10,r7                                  
	fd      r21,r0,r0                                  
	fms     r30,r21,r21,r9                             
	fd.     r25,r30,r29                                
	fd      r15,r25,r25                                
	fd      r25,r15,r28                                
	fnma.   r8,r25,r30,r25                             
	fd      r24,r8,r8                                  
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1		
	lfd	rf1,d8of0
	fcmpu	3,rf1,rf0
	bne	3,FINI	
#
	ai	g3,g3,1
	stfd	rf8,here		# Check rf8
	l	g7,here
	l	g8,d8of8
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d8of8+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1		
	lfd	rf1,d8of10		
	fcmpu	3,rf1,rf10
	bne	3,FINI
#
	ai	g3,g3,1		
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
	ai	g3,g3,1	
	stfd	rf21,here		# Check rf21
	l	g7,here
	l	g8,d8of21
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d8of21+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1	
	lfd	rf1,d8of23	
	fcmpu	3,rf1,rf23
	bne	3,FINI
#
	ai	g3,g3,1	
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
	ai	g3,g3,1	
	stfd	rf25,here		# Check rf25
	l	g7,here
	l	g8,d8of25
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d8of25+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1	
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
	lfd	rf8,d9if8
	lfd	rf12,d9if12
	lfd	rf13,d9if13
	lfd	rf15,d9if15
	lfd	rf21,d9if21
	lfd	rf23,d9if23
	lfd	rf25,d9if25
	lfd	rf27,d9if27
#
 	lfd	rf1,d9fps
	mtfsf	0xff,rf1
	l	g6,d9icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
#
##  D9 *********
#
	fma.    r11,r15,r27,r8                             
	fd      r22,r11,r25                                
	fd.     r14,r23,r22                                
	fd.     r10,r14,r14                                
	fms.    r10,r12,r21,r13                            
	fd      r10,r21,r10                                
	fd      r21,r10,r10                                
	fd      r11,r21,r21                                
	fnma    r11,r11,r23,r11                            
	fd      r11,r11,r11                                
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1		
	lfd	rf1,d9of10
	fcmpu	3,rf1,rf10
	bne	3,FINI	
#
	ai	g3,g3,1
	lfd	rf1,d9of11
	fcmpu	3,rf1,rf11
	bne	3,FINI
#
	ai	g3,g3,1		
	stfd	rf14,here		# Check rf14
	l	g7,here
	l	g8,d9of14
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d9of14+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1
	lfd	rf1,d9of21
	fcmpu	3,rf1,rf21
	bne	3,FINI
#
	ai	g3,g3,1		
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
#				# Loading floating pt registers for computation
#
	lfd	rf0,d10if0
	lfd	rf2,d10if2
	lfd	rf10,d10if10
	lfd	rf15,d10if15
	lfd	rf21,d10if21
	lfd	rf22,d10if22
	lfd	rf23,d10if23
	lfd	rf27,d10if27
	lfd	rf30,d10if30
#
 	lfd	rf1,d10fps
	mtfsf	0xff,rf1
	l	g6,d10icr
	mtcrf	0xff,g6
#
#  Instructions used for testing
#
##  D10 *********
#
	fma     r6,r22,r21,r27                             
	fd.     r19,r6,r10                                 
	fd.     r26,r19,r30                                
	fd      r18,r26,r26                                
	fms.    r29,r18,r0,r18                             
	fd.     r29,r18,r29                                
	fd.     r15,r29,r23                                
	fd      r30,r15,r10                                
	fnma.   r11,r30,r30,r26                            
	fd      r25,r2,r11                                 
#
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1		
	lfd	rf1,d10of6
	fcmpu	3,rf1,rf6
	bne	3,FINI	
#
	ai	g3,g3,1
	lfd	rf1,d10of11
	fcmpu	3,rf1,rf11
	bne	3,FINI
#
	ai	g3,g3,1		
	lfd	rf1,d10of15		
	fcmpu	3,rf1,rf15
	bne	3,FINI
#
	ai	g3,g3,1		
	lfd	rf1,d10of18
	fcmpu	3,rf1,rf18
	bne	3,FINI
#
	ai	g3,g3,1	
	lfd	rf1,d10of19
	fcmpu	3,rf1,rf19
	bne	3,FINI
#
	ai	g3,g3,1	
	stfd	rf25,here		# Check rf25
	l	g7,here
	l	g8,d10of25
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,d10of25+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1	
	lfd	rf1,d10of26
	fcmpu	3,rf1,rf26
	bne	3,FINI
#
	ai	g3,g3,1	
	lfd	rf1,d10of29
	fcmpu	3,rf1,rf29
	bne	3,FINI
#
	ai	g3,g3,1	
	lfd	rf1,d10of30
	fcmpu	3,rf1,rf30
	bne	3,FINI
#
#
	xor	g3,g3,g3		# Return code = 0; pass
FINI:	br
#
# Data Area
#
	.csect	_div_fpp[RW],3
_div_fpp: 
here:	.long	0xffffffff
	.long	0xffffffff
d1if2:	.long	0x010db4f0
	.long	0x3060c9f1                                    
d1if3:	.long	0x69bb9aca
	.long	0x5b81e2f3                                    
d1if4:	.long	0xffefffff
	.long	0xffffff80                                    
d1if5:	.long	0x98d4519f
	.long	0x668d85d8                                    
d1if10:	.long	0xfdb71a4a
	.long	0x2d447f0d                                    
d1if11:	.long	0x80ce57de
	.long	0xb4c3ba04                                    
d1if21:	.long	0x1c1debe0
	.long	0xf590620d                                    
d1if26:	.long	0xbcd89abc
	.long	0x4b4abe3a                                    
d1if30:	.long	0x7de182bf
	.long	0x05435dd8                                    
d1if31:	.long	0x47b787f0
	.long	0xf44df3c2                                    
d1icr:	.long	0xd44af070                                                    
d1fps:	.long	0xffffffff
	.long	0xf4e390d2                                                    
#results                                                               
d1of1:	.long	0xbcd89abc
	.long	0x4b4abe3a                                    
d1of9:	.long	0x010db4f0
	.long	0x3060c9f1                                    
d1of11:	.long	0x7db71a4a
	.long	0x2d447f0c                                    
d1of19:	.long	0x7fefffff
	.long	0xffffff80                                    
d1of28:	.long	0x80000000
	.long	0x00000000                                    
d1of31:	.long	0x3ff00000
	.long	0x00000000                                    
#
d2if0:	.long	0x1fdd5c89
	.long	0x17fdbe8d                                    
d2if1:	.long	0x57cd6da6
	.long	0x8e419fbc                                    
d2if4:	.long	0x3e0dba07
	.long	0x8c2084c3                                    
d2if9:	.long	0xfff00000
	.long	0x00000000                                    
d2if11:	.long	0x7fefffff
	.long	0xff800000                                    
d2if12:	.long	0x00002863
	.long	0x5ce9cb98                                    
d2if16:	.long	0x9ff00000
	.long	0xffffffff                                    
d2if20:	.long	0xb07b97e7
	.long	0xbec54055                                    
d2if24:	.long	0xc971eab4
	.long	0x3de0af65                                    
d2if26:	.long	0x00001fff
	.long	0xffffffff                                    
d2if28:	.long	0xffd00fff
	.long	0xffffffff                                    
d2if31:	.long	0x3ff00000
	.long	0x0000ffff                                    
d2icr:	.long	0x3878b568                                                    
d2fps:	.long	0xffffffff
	.long	0x000000f9                                                    
#results                                                               
d2of2:	.long	0x80000000
	.long	0x00000000                                    
d2of6:	.long	0xc971eab4
	.long	0x3ddf90ba                                    
d2of8:	.long	0x80000000
	.long	0x00000000                                    
d2of20:	.long	0xb07b97e7
	.long	0xbec54055                                    
d2of29:	.long	0x80000000
	.long	0x00000000                                    
#
d3if4:	.long	0xef852ff7
	.long	0xbf220b1f                                    
d3if9:	.long	0x5e4e5ace
	.long	0xd2bafd09                                    
d3if10:	.long	0x1c47326d
	.long	0x48b87511                                    
d3if11:	.long	0xfb47fc9f
	.long	0xe8b44054                                    
d3if17:	.long	0x9c68e7b6
	.long	0x2cabbdd8                                    
d3if18:	.long	0x00000000
	.long	0x00000000                                    
d3if23:	.long	0x00000000
	.long	0x00000000                                    
d3if24:	.long	0x9f47533e
	.long	0x0922ac1c                                    
d3if29:	.long	0xa3febdd4
	.long	0xd3f7eb8d                                    
d3if31:	.long	0x9c61b143
	.long	0x55170b8c                                    
d3icr:	.long	0xcea42ca7                                                    
d3fps:	.long	0xffffffff
	.long	0x708cb0da                                                    
#results                                                               
d3of8:	.long	0xbff00000
	.long	0x00000000                                    
d3of11:	.long	0x3ff00000
	.long	0x00000000                                    
d3of12:	.long	0x80000000
	.long	0x00000000                                    
d3of18:	.long	0x3ff00000
	.long	0x00000000                                    
d3of24:	.long	0x4adfc37b
	.long	0x61977cb4                                    
#
d4if1:	.long	0x00000000
	.long	0x00000000                                    
d4if7:	.long	0x7fe00001
	.long	0xffffffff                                    
d4if8:	.long	0xd4fb9314
	.long	0x5ad2921c                                    
d4if12:	.long	0x3c33a1a5
	.long	0xd22e103f                                    
d4if14:	.long	0x7fec0000
	.long	0x00000000                                    
d4if23:	.long	0xfff00000
	.long	0x00000000                                    
d4if24:	.long	0x1ff001ff
	.long	0xffffffff                                    
d4if28:	.long	0xbff00000
	.long	0x000003ff                                    
d4if30:	.long	0x7a8e0b74
	.long	0x6619ac1a                                    
d4if31:	.long	0xa0000000
	.long	0x0001ffff                                    
d4icr:	.long	0xfcc5cbb4                                                    
d4fps:	.long	0xffffffff
	.long	0x63398059                                                    
#results                                                               
d4of4:	.long	0x7fd7fffb
	.long	0xfffff803                                    
d4of8:	.long	0x00000000
	.long	0x00000000                                    
d4of19:	.long	0xe7111627
	.long	0x6f542442                                    
d4of20:	.long	0x7ff80000
	.long	0x00000000                                    
d4of25:	.long	0x95126210
	.long	0xa239d7e5                                    
d4of28:	.long	0xfff00000
	.long	0x00000000                                    
#
d5if1:	.long	0x3616a549
	.long	0x0c3d456e                                    
d5if3:	.long	0x1cca77b4
	.long	0xae1e8fbd                                    
d5if6:	.long	0x3286fcea
	.long	0xdb32fb31                                    
d5if10:	.long	0x94665480
	.long	0x0e429393                                    
d5if12:	.long	0x9cc92f36
	.long	0x057d207c                                    
d5if21:	.long	0x00000000
	.long	0x00000001                                    
d5if22:	.long	0xeedef784
	.long	0x4a772f17                                    
d5if25:	.long	0xf9b56be4
	.long	0xd85ea4e1                                    
d5if26:	.long	0x195c3580
	.long	0xe03df403                                    
d5if27:	.long	0x00000000
	.long	0x00022a27                                    
d5if29:	.long	0x86048a59
	.long	0xc76bc6cc                                    
d5icr:	.long	0xdb4bad55                                                    
d5fps:	.long	0xffffffff
	.long	0x000400d1                                                    
#results                                                               
d5of4:	.long	0xf9d8ed2b
	.long	0xa1660642                                    
d5of6:	.long	0x3286fcea
	.long	0xdb32fb31                                    
d5of8:	.long	0x80000000
	.long	0x00000000                                    
d5of9:	.long	0x3ff00000
	.long	0x00000000                                    
d5of15:	.long	0x00000000
	.long	0x00000000                                    
d5of20:	.long	0x48c81f26
	.long	0x9c6ab6b0                                    
d5of30:	.long	0x3ff00000
	.long	0x00000000                                    
#
d6if0:	.long	0x00000000
	.long	0x031f454b                                    
d6if3:	.long	0x00000000
	.long	0x00000000                                    
d6if4:	.long	0x80000000
	.long	0x00000000                                    
d6if5:	.long	0xbd04d6e1
	.long	0xe26b6b5c                                    
d6if8:	.long	0xfff00000
	.long	0x00000000                                    
d6if13:	.long	0x9c608e37
	.long	0x44927912                                    
d6if15:	.long	0xffd00000
	.long	0x003fffff                                    
d6if16:	.long	0x5693a920
	.long	0xb2d28e7b                                    
d6if22:	.long	0xfff00000
	.long	0x00000000                                    
d6if23:	.long	0xfffddea3
	.long	0xe4e44c96                                    
d6if28:	.long	0xc9f227c5
	.long	0x4a97333e                                    
d6icr:	.long	0x3ad62af8                                                    
d6fps:	.long	0xffffffff
	.long	0x638060b2                                                    
#results                                                               
d6of2:	.long	0x81532d4b
	.long	0x0cb7fd4e                                    
d6of9:	.long	0xfffddea3
	.long	0xe4e44c96                                    
d6of10:	.long	0xfffddea3
	.long	0xe4e44c96                                    
d6of11:	.long	0x01532d4b
	.long	0x0cb7fd4d                                    
d6of23:	.long	0xfffddea3
	.long	0xe4e44c96                                    
d6of25:	.long	0xfffddea3
	.long	0xe4e44c96                                    
#
d7if0:	.long	0xfff00000
	.long	0x00000000                                    
d7if3:	.long	0x7fd00000
	.long	0x0001ffff                                    
d7if8:	.long	0xe128de7a
	.long	0x8eb9ac0e                                    
d7if15:	.long	0xc0100000
	.long	0x00000fff                                    
d7if21:	.long	0x7fefffff
	.long	0xffffffff                                    
d7if22:	.long	0x9dbd9b4d
	.long	0xedc1bd4e                                    
d7icr:	.long	0x5585d6d8                                                    
d7fps:	.long	0xffffffff
	.long	0x7c468011                                                    
#results                                                               
d7of5:	.long	0x7ff80000
	.long	0x00000000                                    
d7of7:	.long	0x7ff00000
	.long	0x00000000                                    
d7of19:	.long	0x7ff80000
	.long	0x00000000                                    
d7of22:	.long	0x7ff80000
	.long	0x00000000                                    
d7of24:	.long	0x7ff80000
	.long	0x00000000                                    
#
d8if1:	.long	0x88100000
	.long	0x00000007                                    
d8if7:	.long	0xfff00000
	.long	0x00000000                                    
d8if9:	.long	0x62c73f38
	.long	0xc89b8666                                    
d8if10:	.long	0x80000000
	.long	0x0fffffff                                    
d8if19:	.long	0x80000000
	.long	0x0000007f                                    
d8if28:	.long	0xb6a284ee
	.long	0x8bf054d8                                    
d8if29:	.long	0x80034ccc
	.long	0x770eda8c                                    
d8icr:	.long	0x02ffbce4                                                    
d8fps:	.long	0xffffffff
	.long	0x67776018                                                    
#results                                                               
d8of0:	.long	0x80000000
	.long	0x00000000                                    
d8of8:	.long	0x7ff80000
	.long	0x00000000                                    
d8of10:	.long	0x780284ee
	.long	0x8d18a3c1                                    
d8of15:	.long	0x7ff80000
	.long	0x00000000                                    
d8of21:	.long	0x7ff80000
	.long	0x00000000                                    
d8of23:	.long	0x80000000
	.long	0x0fffffff                                    
d8of24:	.long	0x7ff80000
	.long	0x00000000                                    
d8of25:	.long	0x7ff80000
	.long	0x00000000                                    
d8of30:	.long	0x7ff80000
	.long	0x00000000                                    
#
d9if8:	.long	0xfffb1af4
	.long	0x60e12453                                    
d9if12:	.long	0xc0a286d1
	.long	0x02113ec0                                    
d9if13:	.long	0x39ebf6e1
	.long	0x5f8feddd                                    
d9if15:	.long	0xc0122682
	.long	0x8ba3beb0                                    
d9if21:	.long	0x3b658848
	.long	0x374ccc88                                    
d9if23:	.long	0xfa612132
	.long	0x617bb769                                    
d9if25:	.long	0xecac33de
	.long	0x3cbd41e3                                    
d9if27:	.long	0xfffc5e57
	.long	0x18e3a6a3                                    
d9icr:	.long	0x7b47284e                                                    
d9fps:	.long	0xffffffff
	.long	0x630bb0f8                                                    
#results                                                               
d9of10:	.long	0xbf3ba2ca
	.long	0x54d54948                                    
d9of11:	.long	0x3ff00000
	.long	0x00000000                                    
d9of14:	.long	0xfffb1af4
	.long	0x60e12453                                    
d9of21:	.long	0x3ff00000
	.long	0x00000000                                    
d9of22:	.long	0xfffb1af4
	.long	0x60e12453                                    
#
d10if0:		.long	0x1a39beb3
		.long	0xd5cdbc7a                                    
d10if2:		.long	0x7ff00000
		.long	0x00000000                                    
d10if10:	.long	0x00000453
		.long	0xbdefc11a                                    
d10if15:	.long	0x8ed3751e
		.long	0xf417e8ba                                    
d10if21:	.long	0xe62dde10
		.long	0xea9231df                                    
d10if22:	.long	0x43a3c412
		.long	0x74b938d0                                    
d10if23:	.long	0x80000000
		.long	0x00000000                                    
d10if27:	.long	0xaa9e03e8
		.long	0x7b70cba9                                    
d10if30:	.long	0xbc689dbe
		.long	0x1c04479d                                    
d10icr:		.long	0x86a818de
d10fps:		.long	0xffffffff
		.long	0xe7906019
#results                                                               
d10of6:		.long	0xe9e272db
		.long	0x6cbeef2a                                    
d10of11:	.long	0xffefffff
		.long	0xffffffff                                    
d10of15:	.long	0x8ed3751e
		.long	0xf417e8ba                                    
d10of18:	.long	0x3ff00000
		.long	0x00000000                                    
d10of19:	.long	0xffefffff
		.long	0xffffffff                                    
d10of25:	.long	0xfff00000
		.long	0x00000000                                    
d10of26:	.long	0x7fefffff
		.long	0xffffffff                                    
d10of29:	.long	0xbff00000
		.long	0x00000000                                    
d10of30:	.long	0xcf51fc90
		.long	0x30766793                                    
#
	.toc
	TOCE(.div_fpp, entry)
	TOCL(_div_fpp, data)
