# @(#)06	1.9  src/bos/usr/ccs/lib/libc/POWER/memccpy.s, libcasm, bos41J, 9514B 4/7/95 16:21:34
#
#*****************************************************************************
#
# COMPONENT_NAME: (LIBCMEM) 
#
# FUNCTIONS: memccpy
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1990,1994
# All Rights Reserved
# Licensed Materials - Property of IBM
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# Function Name: memccpy
#
# Description: The memccpy subroutine copies characters from the memory area
# specified by the Source parameter into the memory area specified by the 
# Target parameter. The memccpy subroutine stops after the first character
# specified by the CHAR parameter is copied, or after NUMBER characters are been
# copied, whichever comes first.
#
# The memccpy subroutine returns a pointer to the character after CHAR is copied
# into Target, or a NULL pointer if CHAR is not found in the first NUMBER 
# characters of Source.
#
# Input: r3 has address of TARGET
#	 r4 has address of SOURCE
#        r5 has CHAR
#	 r6 has NUMBER
#
# Output: r3 has either points to the character after CHAR or NULL
#
#
# a real time check is performed to determine which path of the code should
# be used. It consists of load, load, andil., and branch. Other instructions
# may appear in between for scheduling purposes.
#	
#	NOTE:  memccpy is called by copyinstr() in the kernel and
#	       it assumes that memccpy does not access the stack
#	       or change the stack pointer (r1).  If any changes are
#	       made which require the use of the stack or
#	       manipulation of the stack pointer, the owner of
#	       copyinstr() which is in file copyx.s must be
#	       notified.
#


	.set SYS_CONFIG,r11
	.set SCFG_IMP,r12

	S_PROLOG(memccpy)

	.file	"memccpy.s"

# load _system_configuration struct 

	LTOC(SYS_CONFIG,_system_configuration, data)

# next instruction is here for scheduling purposes

	sli	r8,r5,8		# shift	r5 by 8	bits to r8

# load scfg_impl

	l	SCFG_IMP,scfg_impl(SYS_CONFIG)

# next instruction is here for scheduling purposes

	oril	r7,r8,16 	# or lower r7 with 16

# test for POWER_RS_ALL or POWER_601

	andil.	SCFG_IMP,SCFG_IMP,POWER_RS_ALL|POWER_601

# next instruction is here for scheduling purposes

	mtxer	r7		# now xer has 0,0,r5,16

	beq	cr0,cannot_use_lscbx
	.machine "pwr"

can_use_lscbx:
	
	sri.	r0,r6,4		# r0 = r6 / 16 
	mtctr	r0		# mv r0	to counter reg
	rlinm	r0,r6,0,0xf	# r0 = r6 % 16
	cal	r6,0(0)		# r6 = 0
	beq-	small		# cr is	set by sri. above

loop:	
	lscbx.	r9,r4,r6	# load from r4 destroys r9,
				# r10, r11, and r12
	stsx	r9,r3,r6	# store	in r3, xer has the size
	beq-	exit		# the char was found. return
	ai	r6,r6,16	# r6 inc the src & dist	ptr's
	bdnz+	loop		# decr cnt reg and br if not 0

small:
	or	r7,r8,r0	# or lower r8 with remainder
	mtxer	r7		# now xer has 0,0,r5,remainder
	lscbx.	r7,r4,r6	# load what's left
	stsx	r7,r3,r6	# and store it
	beq+	exit		# char is found
	cal	r3,0(0)		# ret NULL if char not found
	br			# return

exit:	
	mfxer	r7		# number of bytes left
	andil.	r8,r7,0xff	# get rid of the char
	a	r3,r3,r6	# add r6 and r8	for ret	value
	a	r3,r3,r8	# 

	br

cannot_use_lscbx:
	.machine "com"


# Solution without lscbx instruction:
#
#  Check for CHAR in byte-by-byte steps until a word boundary is hit.
#  Then, full words may be loaded without causing an exception.
#  The inner loop checks for CHAR in word-by-word steps while decrementing
#  NUMBER and testing it for zero.
#
#

	.set TMP,r0
	.set TARGET,r3
	.set RETURN_VALUE,r3
	.set SOURCE,r4
	.set CHAR,r5
	.set NUMBER,r6
	.set BYTE0,r7
	.set BYTE1,r8
	.set BYTE2,r9
	.set BYTE3,r10
	.set NULL,0
	

	cmpi	cr7,NUMBER,0			# check for zero-byte copy

	rlinm.	TMP,SOURCE,0,3			# check for word boundary
						# (address mod 4)

	cmpi	cr1,TMP,2			# is string address 2 mod 4 ?
	cmpi	cr6,TMP,3			# is string address 3 mod 4 ?

	mtctr	NUMBER				# move NUMBER to CTR register

	beq	cr7, Counter_0			# set rc=0 and return for
						# zero length count

	ai 	SOURCE,SOURCE,-1		# backup in order to use load
						# update

	ai 	TARGET,TARGET,-1		# backup in order to use store
						# update

	beq	cr0,Wd_aligned			# string is aligned (0 mod 4),
					        # branch to loop

	beq	cr1,Res2			# branch to load up to the next
						# two bytes

	beq	cr6,Res3			# branch to load next byte

Res1:                                           # string address is 1 mod 4

	lbzu 	BYTE1,1(SOURCE)			# load BYTE1
	cmpl 	cr0,BYTE1,CHAR			# test if BYTE1 == CHAR
	stbu 	BYTE1,1(TARGET)			# store BYTE1
	beq  	cr0,Found_char			# branch if CHAR found
	bdz	Counter_0			# decrement CTR register and
						# branch if CTR == 0

Res2:                                           # string address is 2 mod 4

	lbzu 	BYTE2,1(SOURCE)			# load BYTE2
	cmpl 	cr0,BYTE2,CHAR			# test if BYTE2 == CHAR
	stbu 	BYTE2,1(TARGET)			# store BYTE2
	beq  	cr0,Found_char			# branch if CHAR found
	bdz	Counter_0			# decrement CTR register and
						# branch if CTR == 0

Res3:                                           # string address is 3 mod 4

	lbzu 	BYTE3,1(SOURCE)			# load BYTE3
	cmpl 	cr0,BYTE3,CHAR			# test if BYTE3 == CHAR
	stbu 	BYTE3,1(TARGET)			# store BYTE3
	beq  	cr0,Found_char			# branch if CHAR found
	bdz	Counter_0			# decrement CTR register and
						# branch if CTR == 0

Wd_aligned:

	lbzu 	BYTE0,1(SOURCE)			# load BYTE0
	lbzu 	BYTE1,1(SOURCE)			# load BYTE1
	lbzu 	BYTE2,1(SOURCE)			# load BYTE2
	lbzu 	BYTE3,1(SOURCE)			# load BYTE3
	cmpl 	cr0,BYTE0,CHAR			# test if BYTE0 == CHAR
	cmpl 	cr1,BYTE1,CHAR			# test if BYTE1 == CHAR
	cmpl 	cr6,BYTE2,CHAR			# test if BYTE2 == CHAR
	cmpl 	cr7,BYTE3,CHAR			# test if BYTE3 == CHAR

No_char:
	stbu 	BYTE0,1(TARGET)			# store BYTE0
	beq  	cr0,Found_char			# branch if CHAR found
	bdz	Counter_0			# decrement CTR register and
						# branch if CTR == 0

	stbu 	BYTE1,1(TARGET)			# store BYTE1
	beq  	cr1,Found_char			# branch if CHAR found
	bdz	Counter_0			# decrement CTR register and
						# branch if CTR == 0

	stbu 	BYTE2,1(TARGET)			# store BYTE2
	beq  	cr6,Found_char			# branch if CHAR found
	bdz	Counter_0			# decrement CTR register and
						# branch if CTR == 0

	stbu 	BYTE3,1(TARGET)			# store BYTE3
	beq  	cr7,Found_char			# branch if CHAR found
	bdz	Counter_0			# decrement CTR register and
						# branch if CTR == 0

	lbzu 	BYTE0,1(SOURCE)			# load BYTE0
	lbzu 	BYTE1,1(SOURCE)			# load BYTE1
	lbzu 	BYTE2,1(SOURCE)			# load BYTE2
	lbzu 	BYTE3,1(SOURCE)			# load BYTE3
	cmpl 	cr0,BYTE0,CHAR			# test if BYTE0 == CHAR
	cmpl 	cr1,BYTE1,CHAR			# test if BYTE1 == CHAR
	cmpl 	cr6,BYTE2,CHAR			# test if BYTE2 == CHAR
	cmpl 	cr7,BYTE3,CHAR			# test if BYTE3 == CHAR

	b       No_char

Found_char:
	ai	RETURN_VALUE,TARGET,1		# if CHAR found return address
						# of next character

	br					# return

Counter_0:
	lil 	RETURN_VALUE,NULL		# if CHAR not found, NUMBER == 0
						# then return NULL

	S_EPILOG(memccpy)
	FCNDES(memccpy)

	.toc
	TOCE(_system_configuration, data)
	
include(systemcfg.m4)
