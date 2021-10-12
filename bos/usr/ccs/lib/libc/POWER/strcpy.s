# @(#)43	1.6  src/bos/usr/ccs/lib/libc/POWER/strcpy.s, libcasm, bos41J, 9514B 4/7/95 16:21:51
#
# COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
#
# FUNCTIONS: strcpy
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989,1994
# All Rights Reserved
# Licensed Materials - Property of IBM
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# Function Name: strcpy
#
# Description: copies a str into another including the null terminator
#
# Input: r3 has	address	of target string, r4 has address of source string
#
# Output: r3 has address of target string
#
#

# a real time check is performed to determine which path of the code should
# be used. It consists of load, load, andil., and branch. Other instructions
# may appear in between for scheduling purposes.

	.set SYS_CONFIG,r11
	.set SCFG_IMP,r12

	.file	"strcpy.s"

	S_PROLOG(strcpy)

# load _system_configuration struct 

	LTOC(SYS_CONFIG,_system_configuration, data)

# next instruction is here for scheduling purposes

	cal     r0,32(0)                # use r5-r12 for string move ops

# load scfg_impl

	l	SCFG_IMP,scfg_impl(SYS_CONFIG)

# next instruction is here for scheduling purposes

	mtspr   xer,r0                  # set up XER for 32 bytes and stop
					# on NULL byte

# test for POWER_RS_ALL or POWER_601

	andil.	SCFG_IMP,SCFG_IMP,POWER_RS_ALL|POWER_601

	beq	cr0,cannot_use_lscbx
	.machine "pwr"

can_use_lscbx:
	
	lscbx.  r5,0,r4                 # get up to next 32 bytes
	stsx    r5,0,r3                 # XER will contain # bytes to store
	beqr    cr0                     # exit if NULL found
#
#       If here, there are more than 32 bytes in the string
#
str10:  
	lscbx.  r5,r4,r0                # get up to next 32 bytes
	stsx    r5,r3,r0                # XER will contain # bytes to store
	ai      r0,r0,32                # increment destination address
	bne     cr0,str10               # move more bytes

	br
#

cannot_use_lscbx:
	.machine "com"


# Solution without lscbx instruction:
#
#  Check for null and copy in byte-by-byte steps until a word boundary is hit.
#  Then, full words may be loaded without causing an exception
#  the inner loop checks for null and copies in word-by-word steps.
#
#  How to find if a word (b0123) contains a null ?
#
#  1- x0123 = b0123 & 0x7f7f7f7f	  clear	8th bit	of each	byte
#
#  2- x0123 = x0123 + 0x7f7f7f7f	  for each byte	if there are any 1's in	
#					  low 7	bits, a	1 propagates to	the 8th
#					  bit
#
#  3- y0123 = x0123 nor	0x7f7f7f7f	  if y0123 is 0, then no null is found
#					  in low 7 bits	of each	byte and 
#					  next word may	be processed. Else, at
#					  least	one of the bytes has 7 0's in 
#					  low 7	bits so	the 8th	bits need to be
#					  tested to confirm a null byte	
#
#  If y0123 = 0, branch to process the next word; else complete steps 4 and 5 
#
#  4- x0123 = x0123 nor	b0123		  nor back in original word in order to
#					  test original	8th bit	of any byte with
#					  0's in low 7 bits
#
#  5- x0123 = x0123 andc 0x7f7f7f7f test  8th bit of each byte.	if 
#					  x0123	= 0, then no null is found
#					  and next word	may be processed
#
	.set TMP,r0
	.set TARGET,r3
	.set SOURCE,r4
	.set RUNNING_ADDR,r5
	.set BYTE,r6
	.set A0123,r7
	.set B0123,r8
	.set X0123,r9
	.set MASK,r10
	.set LZ,r11
	.set Y0123,r12
	.set NULL,0


	rlinm.    TMP,SOURCE,0,3		# check for word boundary
						# (address mod 4)

	cmpi	  cr1,TMP,2			# is source address 2 mod 4
	cmpi	  cr6,TMP,3			# is source address 3 mod 4

	mr	  RUNNING_ADDR,TARGET		# save address for return value

	beq	  cr0,Word_aligned		# source address is aligned
						# (0 mod 4)

	ai	  SOURCE,SOURCE,-1		# backup to use load update
	ai	  RUNNING_ADDR,TARGET,-1	# backup to use store update

	beq	  cr1,Res2			# branch to load up to the next
						# two bytes

	beq	  cr6,Res3			# branch to load next byte

Res1:						# source address is 1 mod 4
	lbzu	  BYTE,1(SOURCE)	 	# load a byte
	cmpi	  cr7,BYTE,NULL	  		# check for null
	stbu	  BYTE,1(RUNNING_ADDR)		# store the byte
	beqr	  cr7		  		# return if it was null

Res2:						# source address is 2 mod 4
	lbzu	  BYTE,1(SOURCE)	 	# load a byte
	cmpi	  cr0,BYTE,NULL	  		# check for null
	stbu	  BYTE,1(RUNNING_ADDR)		# store the byte
	beqr	  cr0		  		# return if it was null

Res3:						# source address is 3 mod 4
	lbzu	  BYTE,1(SOURCE)	 	# load a byte
	cmpi	  cr6,BYTE,NULL	  		# check for null
	stbu	  BYTE,1(RUNNING_ADDR)		# store the byte
	beqr	  cr6		  		# return if it was null

	ai      RUNNING_ADDR,RUNNING_ADDR,1	# fix
	ai      SOURCE,SOURCE,1			# fix
 
Word_aligned:

	cau	MASK,0,0x7f7f			# create upper byte of
						# 0x7f7f7f7f mask

	ai      SOURCE,SOURCE,-4		# backup in order to use load
						# update

	ai	MASK,MASK,0x7f7f		# create lower byte of 
						# 0x7f7f7f7f mask

#
# for performance need the conditional branch on top 
# and unroll the loop to have 
#
#	load  B 
#	store A 
#	load  A
#	store B
#

	lu	A0123,4(SOURCE)			# load next word

	ai      RUNNING_ADDR,RUNNING_ADDR,-4	# backup in order to use load
						# update

	and	X0123,A0123,MASK		# step 1
	a	X0123,X0123,MASK		# step 2
	nor.	Y0123,X0123,MASK		# step 3

loop:
	bne	cr0, A0123_maybe_null		# branch if any byte are all 0's
						# in low 7 bits

	lu	B0123,4(SOURCE)			# load next word
	stu	A0123,4(RUNNING_ADDR)		# store word
	and	X0123,B0123,MASK		# step 1
	a	X0123,X0123,MASK		# step 2
	nor.	Y0123,X0123,MASK		# step 3

in_loop:
	bne	cr0, B0123_maybe_null		# branch if any byte are all 0's
						# in low 7 bits

	lu	A0123,4(SOURCE)			# load next word
	stu	B0123,4(RUNNING_ADDR)		# store word
	and	X0123,A0123,MASK		# step 1
	a	X0123,X0123,MASK		# step 2
	nor.	Y0123,X0123,MASK		# step 3

	b	loop		
 
A0123_maybe_null:
	nor	X0123,X0123,A0123              # step 4
	andc.	X0123,X0123,MASK               # step 5
	beq	cr0,loop	               # branch back if null not found
	b	null_found
 
B0123_maybe_null:
	nor	X0123,X0123,B0123              # step 4
	andc.	X0123,X0123,MASK               # step 5
	beq	cr0, in_loop	               # branch back if null not found

# here there is	a null somewhere in the	register
# X0123 contains a value which can yield the location
# take advantage of the	fact that the final mask used
# to compute the mask has only a single	bit per	byte,
# so we	don't need to compute tha actual byte number.
	
	mr	A0123,B0123			# move B to A to have a single 
						# copy of code below

null_found:

	cntlz	  LZ,X0123	  		# the first non-zero byte
						# is the first null byte

	cmpi	  cr0,LZ,24	  		# test for full word 
						# (24 leading zero bits)

	cmpi	  cr1,LZ,8	  		# test for 2 bytes 
						# (8 leading zero bits)

	rlinm	  TMP,A0123,8,0,31	  	# rotate left 1 byte
	beq	  cr0,full_last	  		# branch to store 4 bytes

	stb	  TMP,4(RUNNING_ADDR)	  	# store 1st byte
	rlinm	  TMP,TMP,8,0,31	  	# rotate to next byte
	bltr	  cr1		  		# return if just 1 byte

	stb	  TMP,5(RUNNING_ADDR)	  	# store 2nd byte
	beqr	  cr1		  		# return if just 2 bytes

	rlinm	  TMP,TMP,8,0,31	  	# rotate to next byte
	stb	  TMP,6(RUNNING_ADDR)	  	# store 3rd byte
	br			  		# return
 
full_last:
	st	  A0123,4(RUNNING_ADDR)	  	# store last full word
						# and return
 
	S_EPILOG(strcpy)
	FCNDES(strcpy)

	.toc
	TOCE(_system_configuration, data)
	
include(systemcfg.m4)
