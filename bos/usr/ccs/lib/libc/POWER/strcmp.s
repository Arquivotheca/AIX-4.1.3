# @(#)20        1.10  src/bos/usr/ccs/lib/libc/POWER/strcmp.s, libcasm, bos41J, 9514B 4/7/95 16:21:49
#
# COMPONENT_NAME: LIBCSTR
#
# FUNCTIONS: strcmp
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1988,1994
# All Rights Reserved
# Licensed Materials - Property of IBM
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# a real time check is performed to determine which path of the code should
# be used. It consists of load, load, andil., and branch. Other instructions
# may appear in between for scheduling purposes.


	.set SYS_CONFIG,r11
	.set SCFG_IMP,r12

	.file	"strcmp.s"

         S_PROLOG(strcmp)

# load _system_configuration struct 

	LTOC(SYS_CONFIG,_system_configuration, data)

# next instruction is here for scheduling purposes

	  lil      r0,8              #1  We do 8 bytes each iteration.

# load scfg_impl

	l	SCFG_IMP,scfg_impl(SYS_CONFIG)

# next instruction is here for scheduling purposes

          mtspr    1,r0              #1  Set XER(25:31) = 8 (length) and
				     #   XER(16:23) = 0 (the byte to match).

# test for POWER_RS_ALL or POWER_601

	andil.	SCFG_IMP,SCFG_IMP,POWER_RS_ALL|POWER_601

	beq	cr0,cannot_use_lscbx

can_use_lscbx:

#
#  NAME: strcmp
#
#  FUNCTION: C string compare
#
#  EXECUTION ENVIRONMENT:
#  Standard register usage and linkage convention.
#  Registers used r0,r3-r8
#  Condition registers used: 0,1,6
#  No stack requirements.
#
#
#  Register usage:
#
#    r0    r1    r2    r3      r4      r5       r6       r7       r8
#     8, stack  toc  addr1,  addr2  string1  string1  string2  string2
#   base             return
#
#  NOTES:
#  This routine performs the compare with a loop that loads 8 bytes at a time
#  with "lscbx".  Both strings are read in using "lscbx", to avoid a false
#  memory protection violation.  However, the string end is recognized by
#  using the "record" function only on reading the first string.  If the second
#  string ends earlier that will be recognized by a "not equal" comparison, and
#  thus in no case will a read be done beyond the end of either string (at
#  least that's the intent!).
#
#  Only 8 bytes at a time are loaded to avoid adding too much delay to short
#  strings and early miscompares.  Still, this routine exceeds the performance
#  of a byte-at-a-time loop only for strings which have at least two non-zero
#  equal bytes.  For longer strings, it approaches 5 times the byte-at-a-time
#  loop.
#
#           Execution time:
#        The execution time has been optimized for SJ1.
#        Let n = byte number at which mismatch occurs or the terminating
#        zero occurs in the first string, and let a/b denote "floor
#        division".  Then roughly, for SJ1:
#           t = 15                   if mismatch, n <= 8,
#           t = 19 + 11((n-1)/8)     if mismatch, n > 8,
#           t = 21 + 11((n-1)/8)     if strings are equal.
#        The timing for lscbx on SJ1 is 2 + #words accessed + 2 for
#        each 128-byte boundary crossing, but we ignore the last term.
#        The caller's side (setup and BALI) must be added.
#
#
#  RETURN VALUE DESCRIPTION:
#  -1 if first string is less,
#   0 if strings are equal,
#  +1 if first string is greater.
#
#
#  Calling sequence:
#       R3  Address of first input string
#       R4  Address of second input string
#       R3  Return value
#

          lscbx.   r5,0,r3           #4  Load up to 8 bytes of the first string
          lscbx    r7,0,r4           #4  Load up to 8 bytes of the second string
          beq      cr0,check         #0  If null found then must mask and check
          cmpl     cr1,r5,r7         #1  Look for mismatch in first word.
          cmpl     cr6,r6,r8         #1  Look for mismatch in second word.
          bne      cr1,rs_retne      #2  Return if mismatch.
          beq      cr6,more          #0  Continue if both match.
          mcrf     cr1,cr6           #0  Else, copy condition codes.
          b        rs_retne          #0  Return.
#
more:     lscbx.   r5,r3,r0          #4  Load up to 8 bytes of the first string.
          lscbx    r7,r4,r0          #4  Load up to 8 bytes of the second string
          beq      cr0,check         #0  If null found then must mask and check.
#
rs_loop:  ai       r0,r0,8           #1  Point at next eight bytes to load.
          cmpl     cr1,r5,r7         #1  Look for mismatch in first word.
          cmpl     cr6,r6,r8         #1  Look for mismatch in second word.
          lscbx.   r5,r3,r0          #4  Load up to 8 bytes of the first string.
          bne      cr1,rs_retne      #0  Return if mismatch in first word.
          bne      cr6,copy6         #0  Copy CR6 to CR1 and return if mismatch.
          lscbx    r7,r4,r0          #4  Load up to 8 bytes of the second string
          bne      cr0,rs_loop       #0  Continue if no null.
#
check:    mfspr    r0,1              #1  Copy XER count, r0 = 1,2,3,4,5,6,7,8.

          cmpi     cr0,r0,4          #2  Check if more than one register 					     #	(> 4 bytes).

          sfi      r0,r0,8           #1  R0 = 7, 6, 5, 4, 3, 2, 1, 0.
          rlinm    r0,r0,3,0x18      #1  R0 = 24, 16, 8, 0, 24, 16, 8, 0.
          bgt      cr0,tworegs       #1  Should second reg be right justified?
          sr       r5,r5,r0          #1  Right justify first reg.
          sr       r7,r7,r0          #1  Right justify second reg.
          cmpl     cr1,r5,r7         #1  Put answer in CR1.
          b        rs_ret            #0  Return.
#
tworegs:  cmpl     cr1,r5,r7         #1  Put possible answer in CR1.
          sr       r6,r6,r0          #1  Right justify first reg.
          sr       r8,r8,r0          #1  Right justify second reg.
          cmpl     cr6,r6,r8         #1  Check other word too.
          bne      cr1,rs_retne      #0  Return if first word mismatch.
                                     #   First words are equal, so
copy6:    mcrf     cr1,cr6           #0  move result from CR6 to CR1.
#
rs_ret:   beq      cr1,rs_ret0       #0  Result is now in CR1.
rs_retne: bgt      cr1,rs_ret1       #0  Translate lt, gt, eq to
          lil      r3,-1             #1   -1,  1,  0 resp.
          br                         #0
rs_ret1:  lil      r3,1              #1
          br                         #0
rs_ret0:  lil      r3,0              #1
          br 	                     #0  return.   Generate proc end table

cannot_use_lscbx:

	
#
# Function Name: strcmp
#
# Description: Compares two strings
#
# Input: r3, r4  have addresses of strings
#
# Output: r3 has the return value of 0 if strings are equal, -1 if first string
#         is less, or +1 if first string is greater
#
# Solution without lscbx instruction:
#
# 	1- load a byte of each string 
#	2- check first byte for null
#	3- check for equality
#       
#       branch out if null is found of bytes are not equal
#

	.set STR1,r3
	.set STR2,r4
	.set BYTE1,r5
	.set BYTE2,r6
	.set NULL,0


	addi	STR1, STR1, -1		# lbz loads 1 byte ahead of pointer
	addi	STR2, STR2, -1

	lbz	BYTE1,1(STR1)		# load a byte of first string
	lbz	BYTE2,1(STR2)		# load a byte of second string

	cmpi	cr0,BYTE1,NULL		# check for null
	cmpl	cr1,BYTE1,BYTE2 	# check for equality

pc_loop:

	beq	cr0,Found_null

	lbzu	BYTE1,1(STR1)		# load a byte of first string

	bne	cr1,Found_diff

	lbzu	BYTE2,1(STR2)		# load a byte of second string

	cmpi	cr0,BYTE1,NULL		# check for null
	cmpl	cr1,BYTE1,BYTE2 	# check for equality

	b	pc_loop

Found_null:
	beq	cr1,pc_ret0		# if both null, strings are equal
Found_diff:
	bgt	cr1,pc_ret1
	lil	r3,-1			# str1 is less than str2
	br
pc_ret0:
	lil	r3,0			# strings are equal
	br
pc_ret1:
	lil	r3,1			# str1 is greater than str2

	S_EPILOG(strcmp)                   
	FCNDES(strcmp)            
	

	.toc
	TOCE(_system_configuration, data)
	
include(systemcfg.m4)
