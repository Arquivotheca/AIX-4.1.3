# @(#)20	1.7  src/bos/usr/ccs/lib/libc/POWER/strncat.s, libcasm, bos41J, 9514B 4/7/95 16:21:54
#
# COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
#
# FUNCTIONS: strncat
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989,1994
# All Rights Reserved
# Licensed Materials - Property of IBM
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# Function Name: strncat
#
# Description: The strncat subroutine copies at most NUMBER bytes of String2 to
# the end of the string pointed to by the String1 parameter. Copying stops 
# before NUMBER bytes if a null character is encountered in the String2 string.
# The strncat subroutine returns a pointer to the null-terminated result. The
# strncat subroutine returns String1.
#
#
# Input: r3 has address of string1
#	 r4 has address of string2
#	 r5 has NUMBER of bytes to copy
#
# Output: r3 has address of string1.
#
#

# a real time check is performed to determine which path of the code should
# be used. It consists of load, load, andil., and branch. Other instructions
# may appear in between for scheduling purposes.

	.set SYS_CONFIG,r11
	.set SCFG_IMP,r12

	.file	"strncat.s"

	S_PROLOG(strncat)

# load _system_configuration struct 

	LTOC(SYS_CONFIG,_system_configuration, data)

# next instruction is here for scheduling purposes

	cal     r0,28(0)                # use r6-r12 for search of s1 null

# load scfg_impl

	l	SCFG_IMP,scfg_impl(SYS_CONFIG)

# next instruction is here for scheduling purposes

	mtspr   xer,r0                  # set up XER for 28 bytes and stop
					# on NULL byte

# test for POWER_RS_ALL or POWER_601

	andil.	SCFG_IMP,SCFG_IMP,POWER_RS_ALL|POWER_601

	beq	cr0,cannot_use_lscbx
	.machine "pwr"

can_use_lscbx:
	
#
#       First, must find end of string 1
#
	lscbx.  r6,0,r3                 # get up to next 28 bytes
	beq     cr0,ncat20              # branch if NULL found
#
ncat10: lscbx.  r6,r3,r0                # get next 28 bytes
	ai      r0,r0,28                # increment offset
	bne     cr0,ncat10              # branch if NULL not found
#
#       If here, NULL character found in first string (s1)
#
ncat20: mfspr   r6,xer                  # compute address of NULL bytes
	ai      r6,r6,-29               # backup to NULL
	a       r0,r6,r0                #
#
#       r0 + r3 is the address of the NULL byte in s1.  Now must set up
#       xer to get 28 bytes if at least that many specified in r5.
#
	cal     r6,28(0)                # set up for 28 bytes
	mtspr   xer,r6                  # prime xer for 28 bytes
ncat30: ai.     r5,r5,-28               # check if at least 28 bytes remain
	ble     ncat40                  # branch if less than 28 bytes remain
#
	lscbx.  r6,0,r4                 # get next 28 bytes from s2
	stsx    r6,r3,r0                # append to s1
	cal     r4,28(r4)               # bump s2 pointer
	ai      r0,r0,28                # bump s1 pointer offset
	bne     cr0,ncat30              # branch if NULL not found
	br                              # if here, NULL found - exit
#
#       If here, this is at the end of the s2 string
#
ncat40: ai.     r5,r5,28                # get remainder number of bytes
	beq     ncat50                  # branch if no more bytes in s2
#
	mtspr   xer,r5                  # set up xer for up to 28 bytes
	lscbx.  r6,0,r4                 # get up to 28 bytes
	stsx    r6,r3,r0                # append to s1
	beqr    cr0                     # exit if NULL encountered
#
#       If here, NULL not encountered in s2.  Must write NULL to last byte
#       in s1.  R5 contains number of bytes stored.
#
	a       r5,r5,r0                # compute address of last byte in s1
	cal     r0,0(0)                 # clear r0
	stbx    r0,r3,r5                # NULL to end of new s1
	br                              # exit
#
#       If here, number of bytes in s2 is zero modulo 28
#
ncat50: stbx    r5,r3,r0                # store NULL at end of s1
#
	br

cannot_use_lscbx:
	.machine "com"


# Solution without lscbx instruction:
#
# Perform a strlen on String1 to locate null i.e. end of String1. Then,
# strcpy String2 starting at the end of String1 with the initial character
# of String2 overwriting the null at the end of String1. while copying, NUMBER
# of bytes is decremented and tested for zero.
#

	.set TMP,r0
	.set Y0123,r0
	.set STRING1,r3
	.set STRING2,r4
	.set NUMBER,r5
	.set RUNNING_ADDR,r6
	.set BYTE,r7
	.set BYTE0,r7
	.set A0123,r8
	.set BYTE1,r8
	.set BYTE2,r9
	.set B0123,r9
	.set BYTE3,r10
	.set X0123,r10
	.set ANY_REG,r10
	.set MASK,r11
	.set LZ,r12
	.set NULL,0

#
# First strlen(STRING1)
#

	rlinm.	TMP,STRING1,0,3                # check for word boundary
					       # (address mod 4)

	cmpi	cr1,TMP,2	               # is string address 2 mod 4 ?
	cmpi	cr6,TMP,3	               # is string address 3 mod 4 ?
	mr	RUNNING_ADDR,STRING1 	       # save address of string
	beq	cr0,strlen_Word_aligned        # string is aligned (0 mod 4),
					       # branch to strlen_loop

	ai      RUNNING_ADDR,STRING1,-1        # backup in order to use load 
					       # update

	beq	cr1,strlen_Res2	               # branch to load up to the next
					       # two bytes

	beq	cr6,strlen_Res3	               # branch to load next byte

strlen_Res1:				       # string address is 1 mod 4
	lbzu    BYTE,1(RUNNING_ADDR)           # load a byte
	cmpi    cr0,BYTE,NULL	               # test for null
	beq     cr0,Found_NULL	               # branch if null is found

strlen_Res2:			               # string address is 2 mod 4
	lbzu    BYTE,1(RUNNING_ADDR)	       # load a byte
	cmpi    cr0,BYTE,NULL	               # test for null
	beq     cr0,Found_NULL	               # branch if null is found

strlen_Res3:			               # string address is 3 mod 4
	lbzu    BYTE,1(RUNNING_ADDR)	       # load a byte
	cmpi    cr0,BYTE,NULL	               # test for null
	beq     cr0,Found_NULL	               # branch if null is found

	ai      RUNNING_ADDR,RUNNING_ADDR,1    # fix

strlen_Word_aligned:		               # address is on word boundary

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

strlen_loop:
	bne	cr0, maybe_null                # branch if any bytes are all 0's
					       # in low 7 bits

	lu	B0123,4(RUNNING_ADDR)          # load next word
	and	X0123,B0123,MASK	       # step 1
	a	X0123,X0123,MASK	       # step 2
	nor.	Y0123,X0123,MASK	       # step 3

	b	strlen_loop		

maybe_null:		
	nor	X0123,X0123,B0123              # step 4
	andc.	X0123,X0123,MASK               # step 5
	beq	cr0, strlen_loop               # branch back if null is not 
					       # found

strlen_null_found:			
	cntlz	LZ,X0123		       # count leading zeros bits
	sri	LZ,LZ,3			       # div by 8 to get bytes count
	a	RUNNING_ADDR,RUNNING_ADDR,LZ   # add bytes count to the running
					       # address
Found_NULL:

#
# RUNNING_ADDR points to the null byte (end of STRING1.) 
# Now, strcpy (RUNNING_ADDR, STRING2) while NUMBER is decremented and
# tested for zero.
#
	cmpi	cr7,NUMBER,0			# check for zero-byte copy

	rlinm.	TMP,STRING2,0,3			# check for work boundary
						# (address mod 4)

	cmpi	cr1,TMP,2			# is string address 2 mod 4 ?
	cmpi	cr6,TMP,3			# is string address 3 mod 4 ?
	mtctr	NUMBER				# move NUMBER to CTR register

	beqr	cr7				# return if NUMBER is zero

	ai 	STRING2,STRING2,-1		# backup to use load update
	ai 	RUNNING_ADDR,RUNNING_ADDR,-1	# backup to use store update

	beq	cr0,Wd_aligned			# string is aligned (0 mod 4)
						# branch to loop

	beq	cr1,Res2			# branch to load up to the next
						# two bytes

	beq	cr6,Res3			# branch to load next byte

Res1:						# string address is 1 mod 4

	lbzu 	BYTE1,1(STRING2)		# load BYTE1
	cmpi 	cr0,BYTE1,NULL			# test BYTE1 for NULL
	stbu 	BYTE1,1(RUNNING_ADDR)		# store BYTE1

	bdz	Counter_0			# decrement CTR register and
						# branch out if CTR == 0

	beqr  	cr0				# found a NULL, return

Res2:						# string address is 2 mod 4

	lbzu 	BYTE2,1(STRING2)		# load BYTE2
	cmpi 	cr0,BYTE2,NULL			# test BYTE2 for NULL
	stbu 	BYTE2,1(RUNNING_ADDR)		# store BYTE2

	bdz	Counter_0			# decrement CTR register and
						# branch out if CTR == 0

	beqr  	cr0				# found a NULL, return

Res3:						# string address is 3 mod 4

	lbzu 	BYTE3,1(STRING2)		# load BYTE3
	cmpi 	cr0,BYTE3,NULL			# test BYTE3 for NULL
	stbu 	BYTE3,1(RUNNING_ADDR)		# store BYTE3

	bdz	Counter_0			# decrement CTR register and
						# branch out if CTR == 0

	beqr  	cr0				# found NULL, return

Wd_aligned:
	lbzu 	BYTE0,1(STRING2)		# load BYTE0
	lbzu 	BYTE1,1(STRING2)		# load BYTE1
	lbzu 	BYTE2,1(STRING2)		# load BYTE2
	lbzu 	BYTE3,1(STRING2)		# load BYTE3
	cmpi 	cr0,BYTE0,NULL			# test BYTE0 for NULL
	cmpi 	cr1,BYTE1,NULL			# test BYTE1 for NULL
	cmpi 	cr6,BYTE2,NULL			# test BYTE2 for NULL
	cmpi 	cr7,BYTE3,NULL			# test BYTE3 for NULL

No_Null:
	stbu 	BYTE0,1(RUNNING_ADDR)		# strore BYTE0

	bdz	Counter_0			# decrement CTR register and
						# branch out if CTR == 0

	beqr  	cr0				# found NULL, return

	stbu 	BYTE1,1(RUNNING_ADDR)		# strore BYTE1

	bdz	Counter_0			# decrement CTR register and
						# branch out if CTR == 0

	beqr  	cr1				# found NULL, return

	stbu 	BYTE2,1(RUNNING_ADDR)		# strore BYTE2

	bdz	Counter_0			# decrement CTR register and
						# branch out if CTR == 0

	beqr  	cr6				# found NULL, return

	stbu 	BYTE3,1(RUNNING_ADDR)		# strore BYTE3

	bdz	Counter_0			# decrement CTR register and
						# branch out if CTR == 0

	beqr  	cr7				# found NULL, return

	lbzu 	BYTE0,1(STRING2)		# load BYTE0
	lbzu 	BYTE1,1(STRING2)		# load BYTE1
	lbzu 	BYTE2,1(STRING2)		# load BYTE2
	lbzu 	BYTE3,1(STRING2)		# load BYTE3
	cmpi 	cr0,BYTE0,NULL			# test BYTE0 for NULL
	cmpi 	cr1,BYTE1,NULL			# test BYTE1 for NULL
	cmpi 	cr6,BYTE2,NULL			# test BYTE2 for NULL
	cmpi 	cr7,BYTE3,NULL			# test BYTE3 for NULL

	b       No_Null

Counter_0:
	lil	TMP,NULL			# load TMP with NULL
	stbu	TMP,1(RUNNING_ADDR)		# store NULL to end string
						# and return

	S_EPILOG(strncat)			
	FCNDES(strncat)

	.toc
	TOCE(_system_configuration, data)
	
include(systemcfg.m4)
