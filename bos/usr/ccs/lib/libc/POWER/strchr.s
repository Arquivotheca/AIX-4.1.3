# @(#)14	1.7  src/bos/usr/ccs/lib/libc/POWER/strchr.s, libcasm, bos41J, 9514B 4/7/95 16:21:47
#
# COMPONENT_NAME: (LIBCSTR) 
#
# FUNCTIONS: strchr
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1990,1994
# All Rights Reserved
# Licensed Materials - Property of IBM
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# Function Name: strchr
#
# Description: returns the address of the first occurrence of input char in 
#              input string or NULL if char is not found
#
# Input: r3 has address of string, r4 has the char
#
# Output: r3 has address of first occurrence of char or NULL
#
#


# a real time check is performed to determine which path of the code should
# be used. It consists of load, load, andil., and branch. Other instructions
# may appear in between for scheduling purposes.


	.set SYS_CONFIG,r11
	.set SCFG_IMP,r12

	S_PROLOG(strchr)

	.file	"strchr.s"

# load _system_configuration struct 

	LTOC(SYS_CONFIG, _system_configuration, data)

# next instruction is here for scheduling purposes

	sli	r8,r4,8		# shift	r4 by 8	bits

# load scfg_impl

	l	SCFG_IMP,scfg_impl(SYS_CONFIG)

# next instruction is here for scheduling purposes

	oril	r5,r8,4 	# or lower r8 with 4
				# r5 has 0,0,r4,4


# test for POWER_RS_ALL or POWER_601

	andil.	SCFG_IMP,SCFG_IMP,POWER_RS_ALL|POWER_601

# next instruction is here for scheduling purposes

	cal	r0,0(0)		# r0 has 0 

	beq	cr0,cannot_use_lscbx
	.machine "pwr"

can_use_lscbx:

	cal     r4,4(0)  	# r4 has 0,0,0,4

rs_loop:
	mtxer	r4		# r4 -> xer
	lscbx.  r6,r3,r0	# load 4 bytes and check for NULL
	beq     Null_found    
	mtxer	r5		# r5 -> xer
	lscbx.  r6,r3,r0	# load 4 bytes and chek for char 
	beq     Char_found    
	ai      r0,r0,4 	# inc counter

	mtxer	r4		# do the same thing 8 copies
	lscbx.  r6,r3,r0	# so we have fewer branches
	beq     Null_found    	# next to each other
	mtxer	r5
	lscbx.  r6,r3,r0
	beq     Char_found    
	ai      r0,r0,4

	mtxer	r4
	lscbx.  r6,r3,r0
	beq     Null_found    
	mtxer	r5
	lscbx.  r6,r3,r0
	beq     Char_found    
	ai      r0,r0,4 

	b       rs_loop 

Null_found:

	mfxer	r7
	andil.	r8,r5,0xff00	
	or	r5,r8,r7		
	mtxer	r5		# check if char was there
	lscbx.  r6,r3,r0
	beq     Char_found    
	cal	r3,0(0)		# char not found ret NULL
	br

Char_found:

	mfxer	r7		# number of bytes left
	andil.	r8,r7,0xff	# get rid of char
	a	r3,r3,r0	# add r0 to r3
	a	r3,r3,r8	# add r8 to r3
	ai	r3,r3,-1	# 

	br

cannot_use_lscbx:
	.machine "com"

# Solution without lscbx instruction:
#
#  Check for null/char in byte-by-byte steps until a word boundary is hit.
#  Then, full words may be loaded without causing an exception.
#  The inner loop checks for null/char in word-by-word steps.
#
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
#  5- x0123 = x0123 andc 0x7f7f7f7f       test 8th bit of each byte. if 
#					  x0123 = 0, then no null is found
#					  and next word may be processed
#
#  How to find if a word (b0123) contains a char c ?
#
#  6- z0123 =  b0123 xor cccc             xor the word with a cccc word 
#					  if b0123 has a c in any byte, then
#					  z0123 has a null byte. Test for null
#					  in z0123 using steps 1-5 above.
#

	.set TMP,r0
	.set Y0123,r0
	.set RUNNING_ADDR,r3
	.set RETURN_VALUE,r3
	.set CHAR,r4
	.set CCCC,r5
	.set Z0123,r8
	.set BYTE,r9
	.set B0123,r9
	.set ANY_REG,r10
	.set X0123,r10
	.set MASK,r11
	.set NULL,0

	rlinm.	TMP,RUNNING_ADDR,0,3           # check for word boundary
					       # (address mod 4)

	cmpi	cr1,TMP,2	               # is string address 2 mod 4 ?
	cmpi	cr6,TMP,3	               # is string address 3 mod 4 ?
	beq	cr0,Wd_aligned	               # string is aligned (0 mod 4),
					       # branch to loop

	ai      RUNNING_ADDR,RUNNING_ADDR,-1   # backup in order to use load 
					       # update

	beq	cr1,Res2	               # branch to load up to the next
					       # two bytes

	beq	cr6,Res3	               # branch to load next byte

Res1:				               # string address is 1 mod 4
	lbzu    BYTE,1(RUNNING_ADDR)           # load a byte
	cmpl    cr6,BYTE,CHAR	               # test for char
	cmpi    cr0,BYTE,NULL	               # test for null
	beqr	cr6			       # found char, return
	beq     cr0,Found_NULL	               # branch if null is found

Res2:				               # string address is 2 mod 4
	lbzu    BYTE,1(RUNNING_ADDR)	       # load a byte
	cmpl    cr6,BYTE,CHAR	               # test for char
	cmpi    cr0,BYTE,NULL	               # test for null
	beqr	cr6			       # found char, return
	beq     cr0,Found_NULL	               # branch if null is found

Res3:				               # string address is 3 mod 4
	lbzu    BYTE,1(RUNNING_ADDR)	       # load a byte
	cmpl    cr6,BYTE,CHAR	               # test for char
	cmpi    cr0,BYTE,NULL	               # test for null
	beqr	cr6			       # found char, return
	beq     cr0,Found_NULL	               # branch if null is found

	ai      RUNNING_ADDR,RUNNING_ADDR,1    # fix

Wd_aligned:			               # address is on word boundary

	andc.	TMP,ANY_REG,ANY_REG	       # zero cr0

	sli	CCCC,CHAR,8		       # CHAR = 000c, CCCC = 00c0

	cau	MASK,0,0x7f7f	               # create upper byte of
					       # 0x7f7f7f7f mask

	or	CCCC,CCCC,CHAR                 # CCCC = 00cc

	ai      RUNNING_ADDR,RUNNING_ADDR,-4   # backup in order to use load
					       # update

	ai	MASK,MASK,0x7f7f	       # create lower byte of 
					       # 0x7f7f7f7f mask

	rlimi   CCCC,CCCC,16,0xFFFF0000	       # CCCC = cccc

#
# the five step loop is rotated to allow conditional branch on top
#

pc_loop:

	bne	cr0, maybe_char                # branch if any bytes are all 0's
					       # in low 7 bits
pc_loop_2:

# check for NULL 

	lu	B0123,4(RUNNING_ADDR)          # load next word
	and	X0123,B0123,MASK	       # step 1
	a	X0123,X0123,MASK	       # step 2
	nor.	Y0123,X0123,MASK	       # step 3

	bne	cr0, maybe_null                # branch if any byte are all 0's
					       # in low 7 bits
check_char:

# check for CHAR 

	xor     Z0123,B0123,CCCC               # step 6

	and	X0123,Z0123,MASK	       # step 1
	a	X0123,X0123,MASK	       # step 2
	nor.	Y0123,X0123,MASK	       # step 3

	b	pc_loop		

maybe_null:		
	nor	X0123,X0123,B0123              # step 4
	andc.	X0123,X0123,MASK               # step 5

	beq	cr0, check_char	               # branch back if null is not 
					       # found
	b	null_found

# If here, we know that null is not found is the current word
# (B0123) and we have done "Step 6" to convert the char to
# null in Z0123, and we have tested and found that one
# character has all 0 bits in the low 7 bits.  Here we
# test for zero in the high bits as well.  If so, we have
# found the character.  If not, read next word.
	
maybe_char:
	nor	X0123,X0123,Z0123              # step 4
	andc.	X0123,X0123,MASK               # step 5

	beq	cr0, pc_loop_2	               # branch back if char is not 
					       # found
char_found:			
	cntlz	TMP,X0123		       # count leading zeros bits
	sri	TMP,TMP,3		       # div by 8 to get bytes count
	a	RUNNING_ADDR,RUNNING_ADDR,TMP  # add bytes count to the running
	br

null_found:			

# NULL is found but need to check if any byte before NULL is CHAR
# Each byte is tested for NULL first and then for CHAR
# if NULL found, set RETURN_VALUE to NULL and return
# if CHAR found, update the RUNNING_ADDR and return

	rlinm.	TMP,B0123,8,0xff	       # strip off B0 and set cr0
	cmpl	cr1,TMP,CHAR		       # test for CHAR and set cr1
	beqr	cr1			       # found CHAR, RUNNING_ADDR has 
					       # address of CHAR
	beq	cr0,Found_NULL		       # found NULL

	rlinm.	TMP,B0123,16,0xff              # strip off B1 and set cr0
	cmpl	cr1,TMP,CHAR		       # test for CHAR and set cr1	
	ai	RUNNING_ADDR,RUNNING_ADDR,1    # update RUNNING_ADDR
	beqr	cr1			       # found CHAR, return
	beq	cr0,Found_NULL		       # found NULL

	rlinm.	TMP,B0123,24,0xff	       # strip off B2 and set cr0
	cmpl	cr1,TMP,CHAR		       # test for CHAR and set cr1
	ai	RUNNING_ADDR,RUNNING_ADDR,1    # update RUNNING_ADDR
	beqr	cr1                            # found CHAR, return
	beq	cr0,Found_NULL		       # found NULL
	
	rlinm.	TMP,B0123,0,0xff               # strip off B3 and set cr0
	cmpl	cr1,TMP,CHAR                   # test for CHAR and set cr1
	ai	RUNNING_ADDR,RUNNING_ADDR,1    # update RUNNING_ADDR
	beqr	cr1                            # found CHAR, return
	beq	cr0,Found_NULL                 # found NULL

Found_NULL:
	lil	RETURN_VALUE,NULL	       # set RETURN_VALUE to NULL

	S_EPILOG(strchr)		       # return
	FCNDES(strchr)
	.toc
	TOCE(_system_configuration, data)
	
include(systemcfg.m4)
