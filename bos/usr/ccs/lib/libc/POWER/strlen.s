# @(#)49	1.6  src/bos/usr/ccs/lib/libc/POWER/strlen.s, libcasm, bos41J, 9514B 4/7/95 16:21:52
#
# COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
#
# FUNCTIONS: strlen
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1990, 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# Function Name: strlen
#
# Description: returns length of a null-terminated string
#
# Input: r3 has address of string
#
# Output: r3 has byte count of string. The null byte is not counted.
#
#	 e.g.		strlen("")  = 0
#			strlen("a") = 1
#

	.set SYS_CONFIG,r11
	.set SCFG_IMP,r12

	.file	"strlen.s"

	S_PROLOG(strlen)


# a real time check is performed to determine which path of the code should
# be used. It consists of load, load, andil., and branch. Other instructions
# may appear in between for scheduling purposes.

# load _system_configuration struct 

	LTOC(SYS_CONFIG, _system_configuration, data)

# next instruction is here for scheduling purposes

	cal     r0,36(0)	# number of bytes to move, and set byte to
				# compare with to be 0

# load scfg_impl

	l	SCFG_IMP,scfg_impl(SYS_CONFIG)

# next instruction is here for scheduling purposes

	mtxer	r0		# set up xer for transfer

# test for POWER_RS_ALL or POWER_601

	andil.	SCFG_IMP,SCFG_IMP,POWER_RS_ALL|POWER_601

	beq	cr0,cannot_use_lscbx
	.machine "pwr"

can_use_lscbx:

	lscbx.  r4,0,r3		# check fist 36 bytes
	beq     sum		# branch if a NULL was found

str:  
	lscbx.  r4,r3,r0	# check next 36 bytes
	ai      r0,r0,36	# increment address for next load
	beq     sum    

	lscbx.  r4,r3,r0
	ai      r0,r0,36 
	beq     sum    

	lscbx.  r4,r3,r0
	ai      r0,r0,36 
	beq     sum    

	lscbx.  r4,r3,r0
	ai      r0,r0,36 
	beq     sum    

	b       str 

sum:
#	r0 - offset to next 36 byte chunk
#	xer - number of bytes loaded (including 0 byte)

	mfxer 	r3		# get the number of bytes loaded befor NULL
	a     	r3,r3,r0	# add number of bytes loaded, to offset to
				# next chunk to load
	ai     	r3,r3,-37	# subtrace chunk size and the null character
				# for return value
	br

cannot_use_lscbx:
	.machine "com"

# Solution without lscbx instruction:
#
#  return (address of null byte - address of string)
#
#  Check for null in byte-by-byte steps until a word boundary is hit.
#  Then, full words may be loaded without causing an exception.
#  The inner loop checks for null in word-by-word steps.
#
#  How to find if a word (b0123) contains a null ?
#
#  1- x0123 = b0123 & 0x7f7f7f7f          clear 8th bit of each byte
#
#  2- x0123 = x0123 + 0x7f7f7f7f          for each byte if there are any 1's in 
#					  low 7 bits, a 1 propagates to the 8th
#					  bit
#
#  3- y0123 = x0123 nor 0x7f7f7f7f        if y0123 is 0, then no null is found
#					  in low 7 bits of each byte and 
#					  next word may be processed. Else, at
#					  least one of the bytes has 7 0's in 
#					  low 7 bits so the 8th bits need to be
#					  tested to confirm a null byte 
#
#  If y0123 = 0, branch to process the next word; else complete steps 4 and 5 
#
#  4- x0123 = x0123 nor b0123             nor back in original word in order to
#					  test original 8th bit of any byte with
#					  0's in low 7 bits
#
#  5- x0123 = x0123 andc 0x7f7f7f7f test  8th bit of each byte. if 
#					  x0123 = 0, then no null is found
#					  and next word may be processed
#

	.set TMP,r0
	.set Y0123,r0
	.set LENGTH,r3
	.set BASE_ADDR,r3
	.set BYTE,r9
	.set B0123,r9
	.set ANY_REG,r10
	.set X0123,r10
	.set MASK,r11
	.set RUNNING_ADDR,r12
	.set NULL,0


	rlinm.	TMP,BASE_ADDR,0,3              # check for word boundary
					       # (address mod 4)

	cmpi	cr1,TMP,2	               # is string address 2 mod 4 ?
	cmpi	cr6,TMP,3	               # is string address 3 mod 4 ?
	mr	RUNNING_ADDR,BASE_ADDR 	       # save address of string
	beq	cr0,Word_aligned               # string is aligned (0 mod 4),
					       # branch to loop

	ai      RUNNING_ADDR,BASE_ADDR,-1      # backup in order to use load 
					       # update

	beq	cr1,Res2	               # branch to load up to the next
					       # two bytes

	beq	cr6,Res3	               # branch to load next byte

Res1:				               # string address is 1 mod 4
	lbzu    BYTE,1(RUNNING_ADDR)           # load a byte
	cmpi    cr0,BYTE,NULL	               # test for null
	beq     cr0,Found_NULL	               # branch if null is found

Res2:				               # string address is 2 mod 4
	lbzu    BYTE,1(RUNNING_ADDR)	       # load a byte
	cmpi    cr0,BYTE,NULL	               # test for null
	beq     cr0,Found_NULL	               # branch if null is found

Res3:				               # string address is 3 mod 4
	lbzu    BYTE,1(RUNNING_ADDR)	       # load a byte
	cmpi    cr0,BYTE,NULL	               # test for null
	beq     cr0,Found_NULL	               # branch if null is found

	ai      RUNNING_ADDR,RUNNING_ADDR,1    # fix

Word_aligned:			               # address is on word boundary

	andc.	TMP,ANY_REG,ANY_REG	       # zero cr0

	cau	MASK,0,0x7f7f	               # create upper byte of
					       # 0x7f7f7f7f mask

	ai      RUNNING_ADDR,RUNNING_ADDR,-4   # backup in order to use load
					       # update

	ai	MASK,MASK,0x7f7f	       # create lower byte of 
					       # 0x7f7f7f7f mask

#
# the conditional branch is on top for performance
#

loop:
	bne	cr0, maybe_null                # branch if any bytes are all 0's
					       # in low 7 bits

	lu	B0123,4(RUNNING_ADDR)          # load next word
	and	X0123,B0123,MASK	       # step 1
	a	X0123,X0123,MASK	       # step 2
	nor.	Y0123,X0123,MASK	       # step 3

	b	loop		

maybe_null:		
	nor	X0123,X0123,B0123              # step 4
	andc.	X0123,X0123,MASK               # step 5
	beq	cr0, loop	               # branch back if null is not 
					       # found

null_found:			
	cntlz	TMP,X0123		       # count leading zeros bits
	sri	TMP,TMP,3		       # div by 8 to get bytes count
	a	RUNNING_ADDR,RUNNING_ADDR,TMP  # add bytes count to the running
					       # address
Found_NULL:
	sf	LENGTH,BASE_ADDR,RUNNING_ADDR  # subtract null address form
					       # base address

	S_EPILOG(strlen)		       # return
	FCNDES(strlen)

	.toc
	TOCE(_system_configuration, data)
	
include(systemcfg.m4)
