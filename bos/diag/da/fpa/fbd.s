# @(#)41	1.3  src/bos/diag/da/fpa/fbd.s, dafpa, bos411, 9428A410j 3/23/94 06:24:18

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
# comment  fd --  denormalize : normalize = underflow                           
#
#		Appropriate registers are loaded with input values. Then
#		instructions for this module will be executed and results
#		will be compared with expected results. If any test fails,
#		return code is 1, otherwise 0 will be returned to the caller.
#
	.csect	.fbd[PR]
	.globl	.fbd[PR]
	.align	2
.fbd:
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
	LTOC(4, _fbd, data)		# Loads g4 with the address of _fbd
#					#    LTOC is the macro to load g4 from
#					#    the address of the TOC which is in
#					#    g2 (linkage convention).
 	.using	_fbd[RW],g4		# g4 is the base register to the data
#	 				     section _fbd[rw]. Eastablishing 
#					     @ability
#
#	
	xor	g3,g3,g3		# g3 = 0. Initialize return code
	ai	g3,g3,1			# Preset return code = 1; fail
#
#  First test
#
#				# Loading floating pt registers for computation
#
	lfd	rf0,i1f0
	lfd	rf4,i1f4
	lfd	rf9,i1f9
	lfd	rf13,i1f13
#
	lfd	rf1,i1fps
	mtfsf	0xff,rf1
	l	g6,i1cr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
	fd.     r0,r0,r13                                 
	fd.     r7,r0,r4                                  
	fd.     r20,r7,r4                                 
	fd.     r25,r20,r9                                
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	lfd	rf1,o1f0
	fcmpu	3,rf1,rf0
	bne	3,FINI
#
	ai	g3,g3,1
	lfd	rf1,o1f7
	fcmpu	3,rf1,rf7
	bne	3,FINI
#
	ai	g3,g3,1
	lfd	rf1,o1f20
	fcmpu	3,rf1,rf20
	bne	3,FINI
#
	ai	g3,g3,1
	lfd	rf1,o1f25
	fcmpu	3,rf1,rf25
	bne	3,FINI
#
#
#  Second
#
#				# Loading floating pt registers for computation
#
	lfd	rf1,i2f1
	lfd	rf7,i2f7
	lfd	rf18,i2f18
	lfd	rf21,i2f21
	lfd	rf26,i2f26
	lfd	rf28,i2f28
#
	lfd	rf0,i2fps
	mtfsf	0xff,rf0
	l	g6,i2cr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
	fd.     r8,r18,r7                                 
	fd.     r18,r8,r21                                
	fd.     r18,r28,r1                                
	fd.     r18,r26,r7                                
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1
	lfd	rf1,o2f8
	fcmpu	3,rf1,rf8
	bne	3,FINI
#
	ai	g3,g3,1
	lfd	rf1,o2f18
	fcmpu	3,rf1,rf18
	bne	3,FINI
#
#
#  Third
#
#				# Loading floating pt registers for computation
#
	lfd	rf3,i3f3
	lfd	rf4,i3f4
	lfd	rf17,i3f17
	lfd	rf22,i3f22
	lfd	rf24,i3f24
	lfd	rf25,i3f25
	lfd	rf27,i3f27
#
	lfd	rf1,i3fps
	mtfsf	0xff,rf1
	l	g6,i3cr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
#
	fd.     r6,r22,r24                                
	fd.     r6,r6,r4                                  
	fd.     r6,r17,r27                                
	fd.     r6,r25,r3                                 
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1
	lfd	rf1,o3f6
	fcmpu	3,rf1,rf6
	bne	3,FINI
#
#
#
#  Fourth
#
#				# Loading floating pt registers for computation
#
	lfd	rf3,i4f3
	lfd	rf8,i4f8
	lfd	rf14,i4f14
	lfd	rf20,i4f20
	lfd	rf21,i4f21
#
	lfd	rf1,i4fps
	mtfsf	0xff,rf1
	l	g6,i4cr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
#
	fd.     r13,r3,r8                                 
	fd.     r13,r21,r14                               
	fd.     r13,r13,r13                               
	fd.     r24,r20,r13                               
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1
	stfd	rf13,here
	l	g7,here
	l	g8,o4f13
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,o4f13+4
	cmp	0,g7,g8
	bne	0,FINI
#
	ai	g3,g3,1
	stfd	rf24,here
	l	g7,here
	l	g8,o4f24
	cmp	0,g7,g8
	bne	0,FINI
	l	g7,here+4
	l	g8,o4f24+4
	cmp	0,g7,g8
	bne	0,FINI
#
#
#  Fifth
#
#				# Loading floating pt registers for computation
#
	lfd	rf8,i5f8
	lfd	rf10,i5f10
	lfd	rf12,i5f12
	lfd	rf19,i5f19
#
	lfd	rf1,i5fps
	mtfsf	0xff,rf1
	l	g6,i5cr
	mtcrf	0xff,g6
#
#  Instructions used for testing
# 
	fd.     r14,r12,r10                               
	fd.     r28,r19,r14                               
	fd.     r20,r28,r28                               
	fd.     r20,r19,r8                                
#
#				# Checking floating point registers
#
# 					# Loading pre-determined results into
#					     floating point register 1 as temp.
#					     storage to compare results.
#
	ai	g3,g3,1
	lfd	rf1,o5f14
	fcmpu	3,rf1,rf14
	bne	3,FINI
#
	ai	g3,g3,1
	lfd	rf1,o5f20
	fcmpu	3,rf1,rf20
	bne	3,FINI
#
	ai	g3,g3,1
	lfd	rf1,o5f28
	fcmpu	3,rf1,rf28
	bne	3,FINI
#
	xor	g3,g3,g3		# Return code = 0 ; pass
#
FINI:	br
#
#
# Data Area
#
	.csect	_fbd[RW],3
_fbd:
here:	.long	0xffffffff
	.long	0xffffffff
i1f0:	.long	0x00000000
	.long	0x00000068                                   
i1f4:	.long	0xbf264991
	.long	0x3d270180                                   
i1f9:	.long	0xbdc81fcc
	.long	0x35665a5c                                   
i1f13:	.long	0x3ceb2941
	.long	0x1134a001                                   
i1cr:	.long	0xed435031                                                   
i1fps:	.long	0xffffffff
	.long	0xb17a7012                                                   
#results                                                              
o1f0:	.long	0x003ea1c9
	.long	0xfb62bd07                                   
o1f7:	.long	0x8105fd95
	.long	0x5fcb5408                                   
o1f20:	.long	0x01cf92e7
	.long	0x320482b7                                   
o1f25:	.long	0x83f4f0db
	.long	0x82989e0a                                   
o1cr:	.long	0xeb435031                                                   
o1fps:	.long	0xffffffff
	.long	0xb37a8012                                                   
#
i2f1:	.long	0x3c80f60d
	.long	0xe45f789c                                   
i2f7:	.long	0xbe5f62fa
	.long	0x7b9df21b                                   
i2f18:	.long	0x80000003
	.long	0xe0ba8060                                   
i2f21:	.long	0x3f13ff16
	.long	0x0d785f51                                   
i2f26:	.long	0x80000000
	.long	0x00000000                                   
i2f28:	.long	0x00000000
	.long	0x00000005                                   
i2cr:	.long	0x9dad6956                                                   
i2fps:	.long	0xffffffff
	.long	0xe3e4b0d8                                                   
#results                                                              
o2f8:	.long	0x007fa107
	.long	0x83e1603e                                   
o2f18:	.long	0x00000000
	.long	0x00000000                                   
o2cr:	.long	0x9ead6956                                                   
o2fps:	.long	0xffffffff
	.long	0xe3e020d8                                                   
#
i3f3:	.long	0xbda2dc13
	.long	0x3bf73725                                   
i3f4:	.long	0xbd202af8
	.long	0x05d679f2                                   
i3f17:	.long	0x80000000
	.long	0xbbb94368                                   
i3f22:	.long	0x00002778
	.long	0x56816a85                                   
i3f24:	.long	0x3e340190
	.long	0x23499e98                                   
i3f25:	.long	0x00000000
	.long	0x003f9b56                                   
i3f27:	.long	0x3e32f791
	.long	0xe41f51f9                                   
i3cr:	.long	0xa34385fa                                                   
i3fps:	.long	0xffffffff
	.long	0x627c30c8                                                   
#results                                                              
o3f6:	.long	0x807afb28
	.long	0x242aae2e                                   
o3cr:	.long	0xa64385fa                                                   
o3fps:	.long	0xffffffff
	.long	0x627e80c8                                                   
#
i4f3:	.long	0x00000000
	.long	0x001d1d41                                   
i4f8:	.long	0xbe2c53a3
	.long	0x7861d0ea                                   
i4f14:	.long	0xbc0019be
	.long	0x42c6c5e0                                   
i4f20:	.long	0x80000000
	.long	0x00000060                                   
i4f21:	.long	0x00000000
	.long	0x00000000                                   
i4cr:	.long	0x8968ad20                                                   
i4fps:	.long	0xffffffff
	.long	0x0004007a                                                   
#results                                                              
o4f13:	.long	0x7ff80000
	.long	0x00000000                                   
o4f24:	.long	0x7ff80000
	.long	0x00000000                                   
o4cr:	.long	0x8e68ad20                                                   
o4fps:	.long	0xffffffff
	.long	0xea21107a                                                   
#
i5f8:	.long	0xbdfce22a
	.long	0x8f74309c                                   
i5f10:	.long	0x3e6b20b1
	.long	0x7526a668                                   
i5f12:	.long	0x8000001b
	.long	0x5438f235                                   
i5f19:	.long	0x00000003
	.long	0xcfb328e3                                   
i5cr:	.long	0x4d3073b3                                                   
i5fps:	.long	0xffffffff
	.long	0xfd64b0a1                                                   
#results                                                              
o5f14:	.long	0x80a01e64
	.long	0x60b8db88                                   
o5f20:	.long	0x80e0e3e9
	.long	0xc208094a                                   
o5f28:	.long	0xbe3e441b
	.long	0xb285d499                                   
o5cr:	.long	0x4f3073b3                                                   
o5fps:	.long	0xffffffff
	.long	0xff6280a1                                                   
#
	.toc
	TOCE(.fbd, entry)
	TOCL(_fbd, data)
