# @(#)54        1.4  src/bos/usr/ccs/lib/libc/POWER/clc.s, libcmisc, bos411, 9428A410j 11/23/93 15:32:30
#  
#  COMPONENT_NAME: (LIBCMISC) lib/c/misc
#
#  FUNCTIONS: _clc
#
#  ORIGINS: 27
#
#  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#  combined with the aggregated modules for this product)
#                   SOURCE MATERIALS
#  (C) COPYRIGHT International Business Machines Corp. 1985, 1993
#  All Rights Reserved
#
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
####################################################################
#	  .file    "clc.s"
#
#  NAME: _clc
#
#  FUNCTION: General character string compare
#
#  EXECUTION ENVIRONMENT:
#  Standard register usage and linkage convention.
#  Registers used r3-r10
#  Condition registers used: 0,6
#  No stack requirements.
#
#  NOTES:
#  String addresses may be even or odd, both operands of arbitrary
#  length (possibly 0), padding may be required, and operands may overlap.
#
#  String lengths must be in the range 0 to 2**31-1.
#
#  Operation of this routine for lengths >= 2**31 is undefined.
#
#  OPS_KILL must show which regs this routine alters (OPS_KILL is
#  set in MACHDEP PLI).
#
#     The semantics are that the shorter string is treated as if
#  it were extended with blanks to a length equal to the longer
#  string, and then the comparison goes straightforwardly.  The
#  result of this routine is a condition code setting in CR0.
#  ">" means first op > second op.
#
#     This routine performs the comparison with a loop that loads
#  four bytes at a time with "lsi", and then goes into a smaller
#  loop to test the last 0, 1, 2, or 3 bytes.  No great effort
#  has been expended to unroll the loops for scheduling purposes.
#  This routine does not refer to storage outside of the bounds
#  of the strings.  In particular, if a length is zero, it does
#  not use the associated address.
#     An interesting statistic: on one trace tape for the S/370,
#  the average length of CLC executed before "^=" resulted, was 4
#  bytes
#     Execution time:
#  Let n = number of bytes compared,
#      w = floor(n/4),
#      r = mod(n, 4).
#  Assuming no blank extension is necessary, and strings are fullword
#  aligned, then approximately:
#      t = 11 + 6w + 6r on SJ0,
#	    9 + 7w + 7r on SJ1.
#  The caller's side (setup and BALI) must be added.
#     String lengths may be as long as 2**31-1.  (However, the
#  routine is not very fast for very long comparisons).
#
#  RETURN VALUE DESCRIPTION: cr0 is set to the result of the compare
#  of string 1 to string 2.  Gt means string 1 > string 2.
#
	   S_PROLOG(DCCS)
	   .rename ENTRY(DCCS[PR]),"ENTRY(_clc)"
#
#  The PL.8 entry description is ENTRY(CHAR(*), CHAR(*)).
#  Calling sequence:
#	R3  Address of first operand string
#	R4  Length of first operand string
#	R5  Address of second operand string
#	R6  Length of second operand string
#
clc:
start:	   #doz	      r7,r4,r6 # R7 = r6 -. r4.

	   sf         r7,r4,r6        #1 get the difference
	   srai       r8,r7,31        #1 extend sign bit to 0xffffffff or 0
	   andc.      r7,r7,r8        #1 AND complement of extended sign with
				      #  the difference to zero it if negative

	   sf	      r7,r7,r6 # R7 = r6 - (r6 -. r4) = min length.
#
	   sri.       r8,r7,2 # R8 = min length/4 and set CR0.
	   mtctr      r8 # Set CTR = min length/4.
	   rlinm      r10,r7,0,30,31 # R10 = number of residual bytes (0-3).
	   beq	      last3 # Branch if < 4 bytes to compare.
#
loop1:	   lsi	      r8,r3,4 # Main loop, compare 4 at a time.
	   lsi	      r9,r5,4
	   ai	      r3,r3,4
	   cmpl       cr0,r8,r9
	   ai	      r5,r5,4
	   bc	      8,eq,loop1 # Decrement CTR and loop if ^= 0 & eq.
	   bner        # Return if we had a ^= compare.
#
last3:	   cmpi       cr0,r10,0
	   mtctr      r10 # Set CTR = residual byte count.
	   beq	      checkx # If no residual 0-3 bytes to check,
#				  branch to check blank ext.
loop2:	   lbz	      r8,0(r3)
	   lbz	      r9,0(r5)
	   ai	      r3,r3,1
	   cmpl       cr0,r8,r9
	   ai	      r5,r5,1
	   bc	      8,eq,loop2 # Decrement CTR and loop if ^= 0 & eq.
	   bner        # Return if we had a ^= compare.
#
#	 Strings are equal up to the min length.  See if one needs to
#	 be blank-extended and if so continue the comparison.  At this
#	 point r3 and r5 point at the next character to examine.
#
checkx:    cmp	      cr0,r4,r6 # If lengths are equal,
	   beqr        # return with CR0 = "eq".
	   crxor      cr6_0,cr6_0,cr6_0 # Set CR bit cr6_0 = 0 (switch).
	   bgt	      firstl # Branch if first is long.
	   mr	      r3,r5 # Second is long.  Put second sting's
	   mr	      r4,r6 # address and length in r3/r4.
	   creqv      cr6_0,cr6_0,cr6_0 # Set CR bit cr6_0 = 1 (switch).
firstl:    cal	      r9,0x2020(0)
	   cau	      r9,r9,0x2020
#
	   sf	      r7,r7,r4 # R7 = num bytes remaining.
	   sri.       r8,r7,2 # R8 = num remaining/4 and set CR0.
	   mtctr      r8 # Set CTR = num remaining/4.
	   rlinm      r10,r7,0,30,31 # R10 = number of residual bytes (0-3).
	   beq	      last3b # Branch if < 4 bytes to compare.
#
loop3:	   lsi	      r8,r3,4 # Main loop for blank-extended part.
	   ai	      r3,r3,4
	   cmpl       cr0,r8,r9
	   bc	      8,eq,loop3 # Decrement CTR and loop if ^= 0 & eq.
	   bne	      retx # Return if we had a ^= compare.
#
last3b:    cmpi       cr0,r10,0
	   mtctr      r10 # Set CTR = residual byte count.
	   beqr        # If no residual 0-3 bytes to check,
#				  return with "eq" set in CR0.
loop4:	   lbz	      r8,0(r3)
	   ai	      r3,r3,1
	   cmpli      cr0,r8,0x20
	   bc	      8,eq,loop4 # Decrement CTR and loop if ^= 0 & eq.
	   beqr        # If eq, return with "eq" result
#				  (string ended in all blanks).
#
#	 Return, but may have to reverse the comparison result.
retx:	   crxor      lt,lt,cr6_0 # Invert result bits in CR if
	   crxor      gt,gt,cr6_0 # second string was the longer.
	   br	       # Return.
#
# The following is a PL8 format procend table
procend:   .long    0 # Marker.
	   .long    0xF0000000		# lreg, save area offset.
	   .long    0xff000000+procend-clc # fltreg, code length.
	   .long    0			# Line number tables.
	   .long    0x01000000		# Table type, ctl auto.
	   .byte    0xDF,0x24,0x04	# Nodsa etc., proc name length.
	   .byte    0x5f,0x63,0x6c,0x63 # Proc name, _clc in ASCII.

# The following will generate an AIX version 3 procend table
#	  S_EPILOG		     #return.	Generate proc end table
	  FCNDES(DCCS)		     #Function Descriptor
	  .rename  DCCS[DS],"_clc"   #Set PL8 name

