# @(#)87	1.18  src/bos/usr/ccs/lib/libm/POWER/dtoi.s, libm, bos411, 9428A410j 2/21/94 14:02:27
#
# COMPONENT_NAME: (LIBM) math library
#
# FUNCTIONS: dtoi
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#
# NAME: dtoi
#                                                                    
# FUNCTION: CONVERTS A DOUBLE PRECISION VALUE TO AN INTEGER
#                                                                    
# EXECUTION ENVIRONMENT:
#                                                                   
#       Double to Integer expects its argument to be in an FPR not a GPR;
#       Therefore, a C template should be used so that the argument
#       is ONLY passed in the FPR's.
#                                                                   
# NOTES:
#
#       int  dtoi(double x);
#
#       Double to Integer Conversion
#
#       Converts a double precision value in fr1 to an
#       integer in r3. The rounding mode is the currently defined
#       rounding mode.
#
#       This function conforms to the IEEE P754 standard and is
#       a required function.
#
#       The algorithm is:
#
#          if (-2^31 < nbr < 2^31)
#
#              Add 2^52+2^40 to nbr. This will right justify the integer
#              part of the nbr in the lower word of the fp double and
#              will correctly round it (See note 2 below). The 2^40th bit
#              will give negative numbers a bit to borrow from so that the
#              borrow won't come from the 2^52 bit (which would cause
#              the result to be normalized by shifting it left 1 bit).
#
#              Store this double number to memory and load only
#              the lower word into an integer register.
#
#           else  /* number is too big or is funny */
#
#               set the result to:
#                   if(nbr >= 2^31-1)    return(0x7fffffff)
#                   if(nbr <= -2^31)     return(0x80000000)
#                   if(nbr == +infinity) return(0x7fffffff)
#                   if(nbr == -infinity) return(0x80000000)
#                   if(nbr == NaN)       return(0x80000000)
#
#               Set the invalid conversion bit in the IEEE status.
#
#           end
#
#       Note 1: The actual code below does the add of 2^52+2^40
#               before branching on a funny or big number. (This is done
#               to avoid doing the branch for as long as possible which
#               makes SJ code faster for the normal case.) If the branch
#               is taken the add result is ignored. This could cause a
#               problem in "round to plus infinity" rounding mode since
#               the add could cause an overflow. Therefore, the original
#               status register is saved before the add, then restored
#               if the rounding mode is RP. Then the number is tested
#               for too big before repeating the add again.
#
#
#       Note 2: The above algorithm does not work for negative numbers
#               when the rounding mode is round toward zero (RZ).
#               Therefore, the code below identifies this case and
#               branches to special code when it occurs. The algorithm
#               for the negative RZ case is to convert abs(x) to an
#               integer as described above and then to 2's complement the
#               resulting integer. The code to test for the negative RZ
#               case is scattered thruout the normal code to fill the
#               bubbles that would have occurred in the normal code.
#               Thus, the impact on performance of the normal (Round
#               Nearest) case is minimal.
#
#       The performance impact of the above notes is that this code
#       is highly optimized for the RN case:
#
#       Performance estimate = 13 clocks RN + or - norm
#       Performance estimate = 26 clocks RZ - norm
#       Performance estimate = 25 clocks RZ + norm
#       Performance estimate = 25 clocks RM + or - norm
#       Performance estimate = 25 clocks RP + or - norm
#
#
#       IEEE Status Bits that might be set are:
#
#          nvcvi        Invalid Conversion. The double number was
#                       unrepresentable in integer (NaN, too big, infinity).
#          vx           Invalid summary bit
#          nvsnan       Signaling NaN. The double number was an SNaN.
#          inexact      Inexact. The double number was rounded.
#                       Also true if the number was too big.
#
# RETURNS: an integer representation of x
#
#
# ***********************************************************************
	.file "dtoi.s"
#
#
#       Define a TOC entry that points to our constant data
#
	.toc
	TOCL(dtoi_const,data)
z0x0000.S:  .tc  _0x00000000[tc],0     # put single 0 in toc


#
#
#       Code
#
#

	.align  2

	S_PROLOG(dtoi)

	lfs     fr0,z0x0000.S(r2)      # in case fr0 is not already 0.0

# **************************************************************************


	mcrfs   cr1,7           # Transfer rounding mode from FPSCR to cr1
	LTOC(r6,dtoi_const,data)  # Load pointer to constants
	fabs    fr5,fr1         # Take the absolute value of input
	lfd     fr2,8(r6)       # Load 2^31
	mffs    fr7             # Save copy of current status register
	lfd     fr3,0(r6)       # Load the magic number (2^52 + 2^40)
	fcmpu   cr0,fr5,fr2     # Is abs(x) >= 2^31? (may set VXSNAN)
#
#       The next instruction causes the integer part of the number
#       to be rounded (and biased) and put in the lower word of the double.
#
	fa      fr6,fr1,fr3     # Add the magic number (may set XX or OX)
	stfd    fr6,-8(r1)      # Store the double on the stack
				#    Now:  cr0 = abs(x) cmp to 2^31
				#          cr1 = rounding mode field:
				#                0000 = RN, 0001 = RZ
				#                0010 = RP, 0011 = RM
	cror    cr7_0,cr1_2,cr1_3 # True if round mode != RN
	bbt     cr7_0,dtoi_b     # Go to special code if not RN rounding
	l       r3,-4(r1)       # Load the lower part (now the integer)
	bnl     cr0,dtoi_special # Go process NaN's, Infinities, and too big
	br
#
#
#       Here when rounding mode is RM, RP or RZ.
#
#
dtoi_b:
	mtfsf   0xff,fr7        # Restore original status register to
				#    clear spurious overflows and inexacts
	fcmpu   cr6,fr5,fr1     # if abs(x) == x then x was positive or -0
				#    May set VXSNAN
				#    Now:  cr0 = abs(x) cmp to 2^31
				#          cr1 = rounding mode field:
				#                0000 = RN, 0001 = RZ
				#                0010 = RP, 0011 = RM
				#          cr2 = abs(x) cmp to x
	bnl     cr0,dtoi_special # Go process NaN's, Infinities, and too big
	crnor   cr7_0,cr1_2,cr6_2  # cr7_0 = !((!RZ mode) OR (fr0 == fr4))
	bbt     cr7_0,dtoi_nrz   # Branch if RZ mode and x was negative
	fa      fr6,fr1,fr3     # Add the magic number (may set XX)
	stfd    fr6,-8(r1)      # Store the double on the stack
	l       r3,-4(r1)       # Load the lower part (now the integer)
	br
#
#
#       Here when rounding mode is RZ and the input was negative
#       (Note: control doesn't get here if x was a funny number.)
#
dtoi_nrz:
	fa      fr6,fr5,fr3     # Add the magic number to abs(x)
	stfd    fr6,-8(r1)      # Store the double on the stack
	l       r3,-4(r1)       # Load the lower part (now the integer)
	neg     r3,r3           # Negate the result
	br
#
#       Here to take care of a funny or too big double value
#
#               We will have to call a routine to set the
#               invalid conversion bit in the IEEE Status so we now
#               need to do the standard subroutine linkage stuff below.
#
#

	.extern ENTRY(fp_raise_xcp)
dtoi_special:

	mtfsf   0xff,fr7        # Restore original status register to
				#    clear spurious overflows and inexacts
	fcmpu   cr6,fr5,fr1     # Will set VXSNAN if x was a SNaN
	mflr    r0              # Get the current link register
	st      r31,-4(r1)      # Save r31 on the stack
	st      r0,8(r1)        # Save the link register for return
	stu     r1,-60(r1)      # Create stack frame and backtrace
#
#       Here to generate a result
#
	cau     r31,r0,0x8000   # make default result = 0x80000000
	bbt     3,dtoi_c        # (bun) Result = default if unordered (NaN)
#       Here when number is too big (including infinities)
	fcmpu   cr0,fr1,fr2     # Is number +toobig or -toobig
	blt     dtoi_c           # result = 0x80000000 if -toobig
	sfi     r31,r31,-1      # +toobig; make result = 0x7fffffff
#
#       Here to do the call to set the invalid conversion bit in the
#       software IEEE Status and raise the exception if enabled.
#
dtoi_c:
	l       r3,16(r6)       # load invalid conversion mask bit (nvcvi)
	bl      ENTRY(fp_raise_xcp) # Do the call to set nvcvi bit in status
	l	r2, stktoc(r1)	# restore toc pointer
#
#       Get the result and do the epilogue code
#
	mr      r3,r31          # Get the result into r3
	l       r0,68(r1)       # Get saved link register
	ai      r1,r1,60        # Remove the stack frame
	mtlr    r0              # Restore the link register
	l       r31,-4(r1)      # Restore r31
#
#
	br  			# branch back through the link register
	_DF(0x2,0x1,0x8,0x0,0x0,0x2,0xC0000000) # traceback info.

#
# ************************************************************************
#
#       Constant Area
#
# ************************************************************************
#

	   .csect  DATA(dtoi_const[RW]),3
dtoi_const:
	   .long   0x43300800,0x00000000   # 2^52 + 2^40
	   .long   0x41e00000,0x00000000   # 2^31
	   .long   fpxnvcvi     # Invalid Conversion Bit

#
# ************************************************************************
#
#       Function Descriptor
#
# ************************************************************************
#
	FCNDES(dtoi)

include(sys/comlink.m4)
