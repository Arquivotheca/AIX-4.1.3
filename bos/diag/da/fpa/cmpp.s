# @(#)33	1.3  src/bos/diag/da/fpa/cmpp.s, dafpa, bos411, 9428A410j 3/23/94 06:20:42

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
#  testing foiunnormalized numbers
#  producing unnormalized numbers, then moving them            
#
#		Appropriate registers are loaded with input values. Then
#		instructions for this module will be executed and results
#		will be compared with expected results. If any test fails,
#		return code is 1, otherwise 0 will be returned to the caller.
#
	.csect	.cmpp[PR]
	.globl	.cmpp[PR]
	.align	2
.cmpp:
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
	LTOC(4, _cmpp, data)		# Loads g4 with the address of _cmpp
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
 	.using	_cmpp[RW],g4		# g4 is the base register to the data
#	 				     section _cmpp[rw]. Eastablishing 
#					     @ability
#
#	
	xor	g3,g3,g3		# g3 = 0. Initialize return code
	ai	g3,g3,1			# Preset return code = 1; fail
#
#				# Loading floating pt registers for computation
#
	lfd	rf0,if0
	lfd	rf1,if1
#
	lfd	rf2,c1fps
	mtfsf	0xff,rf2
	l	g6,c1cr
	mtcrf	0xff,g6
#
##  C1
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
	lfd	rf1,of0
	fcmpu	3,rf1,rf0
	bne	3,FINI
#
	ai	g3,g3,1
	lfd	rf1,of2
	fcmpu	3,rf1,rf2
	bne	3,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf3,if3
	lfd	rf4,if4
#
	lfd	rf1,c2fps
	mtfsf	0xff,rf1
	l	g6,c2cr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
##  C2
	fma.    r3,r3,r3,r4                               
	fmr.    r5,r3                                     
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
	ai	g3,g3,1
	lfd	rf1,of3
	fcmpu	3,rf1,rf3
	bne	3,FINI
#
	ai	g3,g3,1
	lfd	rf1,of5
	fcmpu	3,rf1,rf5
	bne	3,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf6,if6
	lfd	rf7,if7
#
	lfd	rf1,c3fps
	mtfsf	0xff,rf1
	l	g6,c3cr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
#
##  C3
	fma.    r6,r6,r6,r7                               
	fmr.    r8,r6                                     
# 
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1
	lfd	rf1,of6
	fcmpu	3,rf1,rf6
	bne	3,FINI
#
	ai	g3,g3,1
	lfd	rf1,of8
	fcmpu	3,rf1,rf8
	bne	3,FINI
#
#
#				# Loading floating pt registers for computation
#
	lfd	rf9,if9
	lfd	rf10,if10
#
	lfd	rf1,c4fps
	mtfsf	0xff,rf1
	l	g6,c4cr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
##  C4
	fma.    r9,r9,r9,r10                              
	fmr.    r11,r9                                    
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1
	lfd	rf1,of9
	fcmpu	3,rf1,rf9
	bne	3,FINI
#
	ai	g3,g3,1
	lfd	rf1,of11
	fcmpu	3,rf1,rf11
	bne	3,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf12,if12
	lfd	rf13,if13
#
	lfd	rf1,c5fps
	mtfsf	0xff,rf1
	l	g6,c5cr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
##  C5
	fma.    r12,r12,r12,r13                           
	fmr.    r14,r12                                   
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1
	lfd	rf1,of12
	fcmpu	3,rf1,rf12
	bne	3,FINI
#
	ai	g3,g3,1
	lfd	rf1,of14
	fcmpu	3,rf1,rf14
	bne	3,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf15,if15
	lfd	rf16,if16
#
	lfd	rf1,c6fps
	mtfsf	0xff,rf1
	l	g6,c6cr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
#
##  C6
	fma.    r15,r15,r15,r16                           
	fmr.    r17,r15                                   
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1
	lfd	rf1,of15
	fcmpu	3,rf1,rf15
	bne	3,FINI
#
	ai	g3,g3,1
	lfd	rf1,of17
	fcmpu	3,rf1,rf17
	bne	3,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf18,if18
	lfd	rf19,if19
#
	lfd	rf1,c7fps
	mtfsf	0xff,rf1
	l	g6,c7cr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
##  C7
	fma.    r18,r18,r18,r19                           
	fmr.    r20,r18                                   
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1
	lfd	rf1,of18
	fcmpu	3,rf1,rf18
	bne	3,FINI
#
	ai	g3,g3,1
	lfd	rf1,of20
	fcmpu	3,rf1,rf20
	bne	3,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf21,if21
	lfd	rf22,if22
#
	lfd	rf1,c8fps
	mtfsf	0xff,rf1
	l	g6,c8cr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
##  C8
	fma.    r21,r21,r21,r22                           
	fmr.    r23,r21                                   
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1
	lfd	rf1,of21
	fcmpu	3,rf1,rf21
	bne	3,FINI
#
	ai	g3,g3,1
	lfd	rf1,of23
	fcmpu	3,rf1,rf23
	bne	3,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf24,if24
	lfd	rf25,if25
#
	lfd	rf1,c8fps
	mtfsf	0xff,rf1
	l	g6,c8cr
	mtcrf	0xff,g6
#
#  Instructions used for testing
#
##  C9
	fma.    r24,r24,r24,r25                           
	fmr.    r26,r24                                   
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1
	lfd	rf1,of24
	fcmpu	3,rf1,rf24
	bne	3,FINI
#
	ai	g3,g3,1
	lfd	rf1,of26
	fcmpu	3,rf1,rf26
	bne	3,FINI
#
#
#				# Loading floating pt registers for computation
#
	lfd	rf27,if27
	lfd	rf28,if28
#
	lfd	rf1,c10fps
	mtfsf	0xff,rf1
	l	g6,c10cr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
#
##  C10
	fma.    r27,r27,r27,r28                           
	fmr.    r29,r27                                   
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1
	lfd	rf1,of27
	fcmpu	3,rf1,rf27
	bne	3,FINI
#
	ai	g3,g3,1
	lfd	rf1,of29
	fcmpu	3,rf1,rf29
	bne	3,FINI
#
#				# Loading floating pt registers for computation
#
	lfd	rf30,if30
	lfd	rf31,if31
#
	lfd	rf2,c11fps
	mtfsf	0xff,rf2
	l	g6,c11cr
	mtcrf	0xff,g6
#
#  Instructions used for testing
#
##  C11
	fma.    r30,r30,r30,r31                           
	fmr.    r1,r30                                    
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1
	lfd	rf2,of30
	fcmpu	3,rf2,rf30
	bne	3,FINI
#
	ai	g3,g3,1
	lfd	rf2,of1
	fcmpu	3,rf2,rf1
	bne	3,FINI
#
#
	xor	g3,g3,g3		# Return code = 0; pass
FINI:	br
#
	.csect	_cmpp[RW],3
_cmpp: 
here:	.long	0xffffffff
	.long	0xffffffff
if0:	.long	0x3fffffff
	.long	0xffc00000                                   
if1:	.long	0xc00fffff
	.long	0xff800000                                   
if2:	.long	0xbe50655b
	.long	0x428166da                                   
if3:	.long	0x3fffffff
	.long	0xffe00000                                   
if4:	.long	0xc00fffff
	.long	0xffc00000                                   
if5:	.long	0xe7f53cc0
	.long	0xb05cac26                                   
if6:	.long	0x3fffffff
	.long	0xfff00000                                   
if7:	.long	0xc00fffff
	.long	0xffe00000                                   
if8:	.long	0xda61a6ec
	.long	0x0d67346b                                   
if9:	.long	0x3fffffff
	.long	0xfff80000                                   
if10:	.long	0xc00fffff
	.long	0xfff00000                                   
if11:	.long	0x8301a9c7
	.long	0xdadc4672                                   
if12:	.long	0x3fffffff
	.long	0xfffc0000                                   
if13:	.long	0xc00fffff
	.long	0xfff80000                                   
if14:	.long	0x52532229
	.long	0xc6ac0dbd                                   
if15:	.long	0x3fffffff
	.long	0xfffe0000                                   
if16:	.long	0xc00fffff
	.long	0xfffc0000                                   
if17:	.long	0xb19b113e
	.long	0x2cf3007b                                   
if18:	.long	0x3fffffff
	.long	0xffff0000                                   
if19:	.long	0xc00fffff
	.long	0xfffe0000                                   
if20:	.long	0x3c25f4f5
	.long	0x5ec547aa                                   
if21:	.long	0x3fffffff
	.long	0xffff8000                                   
if22:	.long	0xc00fffff
	.long	0xffff0000                                   
if23:	.long	0x17564276
	.long	0x52b7d60b                                   
if24:	.long	0x3fffffff
	.long	0xffffc000                                   
if25:	.long	0xc00fffff
	.long	0xffff8000                                   
if26:	.long	0xc15570a0
	.long	0xe7286f9f                                   
if27:	.long	0x3fffffff
	.long	0xffffe000                                   
if28:	.long	0xc00fffff
	.long	0xffffc000                                   
if29:	.long	0xdd884eac
	.long	0xf41acf5b                                   
if30:	.long	0x3fffffff
	.long	0xfffff000                                   
if31:	.long	0xc00fffff
	.long	0xffffe000                                   
#
c1cr:	.long	0x73a6ebe0                                                   
c1fps:	.long	0xffffffff
	.long	0xf7979080                                                   
#
c2cr:	.long	0xc9b5ad26                                                   
c2fps:	.long	0xffffffff
	.long	0x6f1980d1                                                   
#
c3cr:	.long	0x17768022                                                   
c3fps:	.long	0xffffffff
	.long	0x62b0f082                                                   
#
c4cr:	.long	0x8df51ac9                                                   
c4fps:	.long	0xffffffff
	.long	0xe646306b                                                   
#
c5cr:	.long	0x191a0452                                                   
c5fps:	.long	0xffffffff
	.long	0x00040071                                                   
#
c6cr:	.long	0x61e70284                                                   
c6fps:	.long	0xffffffff
	.long	0x25a1806a                                                   
#
c7cr:	.long	0xa88a5484                                                   
c7fps:	.long	0xffffffff
	.long	0x00040088                                                   
#
c8cr:	.long	0x1fe49348                                                   
c8fps:	.long	0xffffffff
	.long	0x00040048                                                   
#
c9cr:	.long	0x4fb58f36                                                   
c9fps:	.long	0xffffffff
	.long	0x000400a0                                                   
#
c10cr:	.long	0x0f4fad3d                                                   
c10fps:	.long	0xffffffff
	.long	0x7440f013                                                   
#
c11cr:	.long	0x9a62ded6                                                   
c11fps:	.long	0xffffffff
	.long	0x000200bb                                                   
#
#results                                                              
#
of0:	.long	0x3c300000
	.long	0x00000000                                   
of2:	.long	0x3c300000
	.long	0x00000000                                   
#                                                              
of3:	.long	0x3c100000
	.long	0x00000000                                   
of5:	.long	0x3c100000
	.long	0x00000000                                   
#                                                              
of6:	.long	0x3bf00000
	.long	0x00000000                                   
of8:	.long	0x3bf00000
	.long	0x00000000                                   
#                                                              
of9:	.long	0x3bd00000
	.long	0x00000000                                   
of11:	.long	0x3bd00000
	.long	0x00000000                                   
#                                                              
of12:	.long	0x3bb00000
	.long	0x00000000                                   
of14:	.long	0x3bb00000
	.long	0x00000000                                   
#                                                              
of15:	.long	0x3b900000
	.long	0x00000000                                   
of17:	.long	0x3b900000
	.long	0x00000000                                   
#                                                              
of18:	.long	0x3b700000
	.long	0x00000000                                   
of20:	.long	0x3b700000
	.long	0x00000000                                   
#                                                              
of21:	.long	0x3b500000
	.long	0x00000000                                   
of23:	.long	0x3b500000
	.long	0x00000000                                   
#                                                              
of24:	.long	0x3b300000
	.long	0x00000000                                   
of26:	.long	0x3b300000
	.long	0x00000000                                   
#                                                              
of27:	.long	0x3b100000
	.long	0x00000000                                   
of29:	.long	0x3b100000
	.long	0x00000000                                   
#                                                              
of1:	.long	0x3af00000
	.long	0x00000000                                   
of30:	.long	0x3af00000
	.long	0x00000000                                   
# 
	.toc
	TOCE(.cmpp, entry)
	TOCL(_cmpp, data)
