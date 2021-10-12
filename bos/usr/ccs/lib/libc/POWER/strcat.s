# @(#)19	1.6  src/bos/usr/ccs/lib/libc/POWER/strcat.s, libcasm, bos41J, 9514B 4/7/95 16:21:44
#
# COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
#
# FUNCTIONS: strcat
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989,1994
# All Rights Reserved
# Licensed Materials - Property of IBM
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# Function Name: strcat
#
# Description: The strcat subroutine adds a copy of the string pointed to by 
#              String2 parameter to the end of the string pointed to by 
#              String1 parameter. The strcat subroutine returns a pointer to the
#              null-terminated result.
#
#
# Input: r3 has address of string1, r4 has address of string2
#
# Output: r3 has address of string1.
#
#


# a real time check is performed to determine which path of the code should
# be used. It consists of load, load, andil., and branch. Other instructions
# may appear in between for scheduling purposes.


	.set SYS_CONFIG,r11
	.set SCFG_IMP,r12

	S_PROLOG(strcat)

	.file	"strcat.s"

# load _system_configuration struct 

	LTOC(SYS_CONFIG,_system_configuration, data)

# next instruction is here for scheduling purposes

	cal     r0,32(0)                # use r5-r12 for search of s1 null

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
	
#
#       First, must find end of string 1
#
	lscbx.  r5,0,r3                 # get up to next 32 bytes
	beq     cr0,cat20               # branch if NULL found
#
cat10:  lscbx.  r5,r3,r0                # get next 32 bytes
	ai      r0,r0,32                # increment offset
	bne     cr0,cat10               # branch if NULL not found
#
#       If here, NULL character found in first string (s1)
#
cat20:  mfspr   r6,xer                  # compute address of NULL bytes
	ai      r6,r6,-33               # backup to NULL
	a       r0,r6,r0                #
#
#       r0 + r3 is the address of the NULL byte in s1.  Now must set up
#       xer to get 32 bytes.
#
	cal     r6,32(0)                # set up for 32 bytes
	mtspr   xer,r6                  # prime xer for 32 bytes
#
cat30:  lscbx.  r5,0,r4                 # get next 28 bytes from s2
	stsx    r5,r3,r0                # append to s1
	cal     r4,32(r4)               # bump s2 pointer
	ai      r0,r0,32                # bump s1 pointer offset
	bne       cat30                 # branch if NULL not found
#
	br

cannot_use_lscbx:
	.machine "com"


# Solution without lscbx instruction:
#
# Perform a strlen on String1 to locate null i.e. end of String1. Then,
# strcpy String2 starting at the end of String1 with the initial character
# of String2 overwriting the null at the end of String1.
#

	.set TMP,r0
	.set Y0123,r0
	.set STRING1,r3
	.set STRING2,r4
	.set RUNNING_ADDR,r5
	.set BYTE,r6
	.set A0123,r7
	.set B0123,r8
	.set X0123,r9
	.set ANY_REG,r10
	.set MASK,r10
	.set LZ,r11
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
# Now, strcpy (RUNNING_ADDR, STRING2)
#

	rlinm.    TMP,STRING2,0,3		# check for word boundary
						# (address mod 4)

	cmpi	  cr1,TMP,2			# is source address 2 mod 4
	cmpi	  cr6,TMP,3			# is source address 3 mod 4

	beq	  cr0,strcpy_Word_aligned	# source address is aligned
						# (0 mod 4)

	ai	  STRING2,STRING2,-1		# backup to use load update
	ai	  RUNNING_ADDR,RUNNING_ADDR,-1	# backup to use store update

	beq	  cr1,strcpy_Res2		# branch to load up to the next
						# two bytes

	beq	  cr6,strcpy_Res3		# branch to load next byte

strcpy_Res1:					# source address is 1 mod 4
	lbzu	  BYTE,1(STRING2)	 	# load a byte
	cmpi	  cr7,BYTE,NULL	  		# check for null
	stbu	  BYTE,1(RUNNING_ADDR)		# store the byte
	beqr	  cr7		  		# return if it was null

strcpy_Res2:					# source address is 2 mod 4
	lbzu	  BYTE,1(STRING2)	 	# load a byte
	cmpi	  cr0,BYTE,NULL	  		# check for null
	stbu	  BYTE,1(RUNNING_ADDR)		# store the byte
	beqr	  cr0		  		# return if it was null

strcpy_Res3:					# source address is 3 mod 4
	lbzu	  BYTE,1(STRING2)	 	# load a byte
	cmpi	  cr6,BYTE,NULL	  		# check for null
	stbu	  BYTE,1(RUNNING_ADDR)		# store the byte
	beqr	  cr6		  		# return if it was null

	ai      RUNNING_ADDR,RUNNING_ADDR,1    # fix
	ai      STRING2,STRING2,1    # fix
 
strcpy_Word_aligned:

	cau	MASK,0,0x7f7f			# create upper byte of
						# 0x7f7f7f7f mask

	ai      STRING2,STRING2,-4		# backup in order to use load
						# update

	ai	MASK,MASK,0x7f7f		# create lower byte of 
						# 0x7f7f7f7f mask

#
# for performance need the conditional branch on top 
# and unroll the strcpy_loop to have 
#
#	load  B 
#	store A 
#	load  A
#	store B
#

	lu	A0123,4(STRING2)		# load next word

	ai      RUNNING_ADDR,RUNNING_ADDR,-4	# backup in order to use load
						# update

	and	X0123,A0123,MASK		# step 1
	a	X0123,X0123,MASK		# step 2
	nor.	Y0123,X0123,MASK		# step 3

strcpy_loop:
	bne	cr0, A0123_maybe_null		# branch if any byte are all 0's
						# in low 7 bits

	lu	B0123,4(STRING2)		# load next word
	stu	A0123,4(RUNNING_ADDR)		# store word
	and	X0123,B0123,MASK		# step 1
	a	X0123,X0123,MASK		# step 2
	nor.	Y0123,X0123,MASK		# step 3

in_strcpy_loop:
	bne	cr0, B0123_maybe_null		# branch if any byte are all 0's
						# in low 7 bits

	lu	A0123,4(STRING2)		# load next word
	stu	B0123,4(RUNNING_ADDR)		# store word
	and	X0123,A0123,MASK		# step 1
	a	X0123,X0123,MASK		# step 2
	nor.	Y0123,X0123,MASK		# step 3

	b	strcpy_loop		
 
A0123_maybe_null:
	nor	X0123,X0123,A0123              # step 4
	andc.	X0123,X0123,MASK               # step 5
	beq	cr0,strcpy_loop	               # branch back if null not found
	b	strcpy_null_found
 
B0123_maybe_null:
	nor	X0123,X0123,B0123              # step 4
	andc.	X0123,X0123,MASK               # step 5
	beq	cr0, in_strcpy_loop	       # branch back if null not found

# here there is	a null somewhere in the	register
# X0123 contains a value which can yield the location
# take advantage of the	fact that the final mask used
# to compute the mask has only a single	bit per	byte,
# so we	don't need to compute tha actual byte number.
	
	mr	A0123,B0123			# move B to A to have a single
						# copy of code below

strcpy_null_found:

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
	S_EPILOG(strcat)
	FCNDES(strcat)
	

	.toc
	TOCE(_system_configuration, data)
	
include(systemcfg.m4)
