# @(#)23	1.7  src/bos/usr/ccs/lib/libc/POWER/strncpy.s, libcasm, bos41J, 9514B 4/7/95 16:21:56
#
# COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
#
# FUNCTIONS: strncpy
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989,1994
# All Rights Reserved
# Licensed Materials - Property of IBM
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# Function Name: strncpy
#
# Description: The strncpy subroutine copies NUMBER of bytes from the string
# pointed to by String2 parameter to a character array pointed to by 
# String1 parameter.  If String2 is less than NUMBER characters long, then 
# strncpy pads String1 with trailing null characters to fill NUMBER bytes. If 
# String2 is NUMBER or more characters long, then only the first NUMBER 
# characters are copied and the result is not terminated with a null byte. The
# strncpy subroutine returns the value of the String1 parameter.
#
#
# Input: r3 has	address	of STRING1
#        r4 has address of STRING2
#        r5 has NUMBER of bytes to copy
#
# Output: r3 has address of STRING1
#
#


# a real time check is performed to determine which path of the code should
# be used. It consists of load, load, andil., and branch. Other instructions
# may appear in between for scheduling purposes.

	.set SYS_CONFIG,r11
	.set SCFG_IMP,r12

	.file	"strncpy.s"

	S_PROLOG(strncpy)

# load _system_configuration struct 

	LTOC(SYS_CONFIG,_system_configuration, data)

# next instruction is here for scheduling purposes

	cal     r6,28(0)                # set up for 28 bytes

# load scfg_impl

	l	SCFG_IMP,scfg_impl(SYS_CONFIG)

# next instruction is here for scheduling purposes

	mtspr   xer,r6                  # prime xer for 28 bytes

# test for POWER_RS_ALL or POWER_601

	andil.	SCFG_IMP,SCFG_IMP,POWER_RS_ALL|POWER_601

# next instruction is here for scheduling purposes

	cal     r0,-28(0)               # prime offset

	beq	cr0,cannot_use_lscbx
	.machine "pwr"

can_use_lscbx:
	

ncpy10: ai      r0,r0,28                # bump s1 pointer offset
	ai.     r5,r5,-28               # check if at least 28 bytes remain
	ble     ncpy50                  # branch if less than 28 bytes remain
#
	lscbx.  r6,r4,r0                # get next 28 bytes from s2
	stsx    r6,r3,r0                # copy to s1
	bne     cr0,ncpy10              # branch if NULL not found
#
#       If here, NULL found.  Must append NULLS to s1 until number specified
#       exhausted.
#
	ai      r5,r5,28                # reset count
ncpy15: mfspr   r6,xer                  # get number bytes copied
	a       r0,r6,r0                # next address in s1
	sf      r5,r6,r5                # r5 - r6 -> r5
#
#       Now clear two registers in order to store up to 8 bytes per loop
#
	cal     r7,8(0)                 # set up xer to store 8 bytes
	mtspr   xer,r7
	cal     r7,0(0)
	cal     r8,0(0)
#
ncpy20: ai.     r5,r5,-8                # check if at least 8 bytes remain
	ble     ncpy30                  # branch if less than 8 bytes remain
#
	stsx    r7,r3,r0                # copy to s1
	ai      r0,r0,8                 # bump s1 pointer offset
	b       ncpy20                  # branch if NULL not found
#
#       If here, 8 or less NULLs must be appended to s1
#
ncpy30: ai      r5,r5,8                 # reset count
	mtspr   xer,r5
	stsx    r7,r3,r0                # append remainder of NULLS
	br                              # if here, done - exit
#
#       If here, less than 28 bytes remain and no NULL has been encountered.
#
ncpy50: ai      r5,r5,28                # restore number bytes remaining
	mtspr   xer,r5                  # set up to copy remaining bytes
	lscbx.  r6,r4,r0                # get remainder
	stsx    r6,r3,r0                # to s1
	beq     cr0,ncpy15              # branch to padd NULLs to s1
#
#       If here, no NULL found.  s1 is not a NULL terminated string
#
	
	br

cannot_use_lscbx:
	.machine "com"


# Solution without lscbx instruction:
#
#  Check for null and copy in byte-by-byte steps until a word boundary is hit.
#  Then, full words may be loaded without causing an exception
#  the inner loop checks for null and copies in byte-by-byte steps while 
#  decrementing the NUMBER of bytes.
#
	
	.set TMP,r0
	.set STRING1,r3
	.set STRING2,r4
	.set NUMBER,r5
	.set RUNNING_ADDR,r5
	.set BYTE0,r6
	.set BYTE1,r7
	.set BYTE2,r8
	.set BYTE3,r9
	.set NULL,0

	cmpi	cr7,NUMBER,0			# check for zero-byte copy

	rlinm.	TMP,STRING2,0,3			# check for word boundary
						# (address mod 4)

	cmpi	cr1,TMP,2			# is STRING2 address 2 mod 4
	cmpi	cr6,TMP,3			# is STRING2 address 3 mod 4

	mtctr	NUMBER				# move NUMBER to CTR register

	beqr	cr7				# return if NUMBER is zero

	ai 	STRING2,STRING2,-1		# backup in order to use load
						# update

	ai 	RUNNING_ADDR,STRING1,-1		# backup in order to use store
						# update and save address of
						# STRING1 for return value

	beq	cr0,Wd_aligned			# STRING2 is aligned (0 mod 4)
						# branch to loop

	beq	cr1,Res2			# branch to load up to the next
						# two bytes

	beq	cr6,Res3			# branch to load next byte

Res1:						# string address is 1 mod 4
	lbzu 	BYTE1,1(STRING2)		# load a byte
	cmpi 	cr0,BYTE1,NULL			# test BYTE1 for NULL
	stbu 	BYTE1,1(RUNNING_ADDR)		# store a byte

	bdzr					# decrement CTR register and 
						# return if CTR == 0

	beq  	cr0,Found_NULL1			# branch if NULL is found

Res2:						# string address is 2 mod 4
	lbzu 	BYTE2,1(STRING2)		# load a byte
	cmpi 	cr0,BYTE2,NULL
	stbu 	BYTE2,1(RUNNING_ADDR)		# store a byte
	
	bdzr					# decrement CTR register and 
						# return if CTR == 0

	beq  	cr0,Found_NULL2			# branch if null is found

Res3:						# string address is 3 mod 4
	lbzu 	BYTE3,1(STRING2)		# load a byte
	cmpi 	cr0,BYTE3,NULL			# test BYTE3 for NULL
	stbu 	BYTE3,1(RUNNING_ADDR)		# store a byte
	
	bdzr					# decrement CTR register and 
						# return if CTR == 0

	beq  	cr0,Found_NULL3			# branch if null is found


Wd_aligned:
	lbzu 	BYTE0,1(STRING2)		# load a byte
	lbzu 	BYTE1,1(STRING2)		# load a byte
	lbzu 	BYTE2,1(STRING2)		# load a byte
	lbzu 	BYTE3,1(STRING2)		# load a byte
	cmpi 	cr0,BYTE0,NULL			# test BYTE0 for NULL
	cmpi 	cr1,BYTE1,NULL			# test BYTE1 for NULL
	cmpi 	cr6,BYTE2,NULL			# test BYTE2 for NULL
	cmpi 	cr7,BYTE3,NULL			# test BYTE3 for NULL

No_Null:
	stbu 	BYTE0,1(RUNNING_ADDR)		# store a byte
	
	bdzr					# decrement CTR register and 
						# return if CTR == 0

	beq  	cr0,Found_NULL0
	stbu 	BYTE1,1(RUNNING_ADDR)		# store a byte
	
	bdzr					# decrement CTR register and 
						# return if CTR == 0

	beq  	cr1,Found_NULL1
	stbu 	BYTE2,1(RUNNING_ADDR)		# store a byte
	
	bdzr					# decrement CTR register and 
						# return if CTR == 0

	beq  	cr6,Found_NULL2
	stbu 	BYTE3,1(RUNNING_ADDR)		# store a byte
	
	bdzr					# decrement CTR register and 
						# return if CTR == 0

	beq  	cr7,Found_NULL3

	lbzu 	BYTE0,1(STRING2)		# load a byte
	lbzu 	BYTE1,1(STRING2)		# load a byte
	lbzu 	BYTE2,1(STRING2)		# load a byte
	lbzu 	BYTE3,1(STRING2)		# load a byte
	cmpi 	cr0,BYTE0,NULL			# test BYTE0 for NULL
	cmpi 	cr1,BYTE1,NULL			# test BYTE1 for NULL
	cmpi 	cr6,BYTE2,NULL			# test BYTE2 for NULL
	cmpi 	cr7,BYTE3,NULL			# test BYTE3 for NULL

	b       No_Null

Found_NULL0:
	stbu	BYTE0,1(RUNNING_ADDR)		# store a NULL
	bdn	Found_NULL0			# decrement CTR register and
						# branch out if CTR == 0
	br

Found_NULL1:
	stbu	BYTE1,1(RUNNING_ADDR)		# store a NULL
	bdn	Found_NULL1			# decrement CTR register and
						# branch out if CTR == 0
	br

Found_NULL2:
	stbu	BYTE2,1(RUNNING_ADDR)		# store a NULL
	bdn	Found_NULL2			# decrement CTR register and
						# branch out if CTR == 0
	br

Found_NULL3:
	stbu	BYTE3,1(RUNNING_ADDR)		# store a NULL
	bdn	Found_NULL3			# decrement CTR register and
						# branch out if CTR == 0

	S_EPILOG(strncpy)
	FCNDES(strncpy)


	.toc
	TOCE(_system_configuration, data)
	
include(systemcfg.m4)
