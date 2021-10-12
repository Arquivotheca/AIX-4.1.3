# @(#)83	1.5  src/bos/usr/ccs/lib/libc/POWER/shr_itrunc.s, libccnv, bos411, 9428A410j 2/21/94 14:00:49
#
# COMPONENT_NAME: LIBCCNV shr_itrunc
#
# FUNCTIONS: shr_itrunc
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
# NAME: shr_itrunc
#                                                                    
# FUNCTION: converts a double to a signed integer
#           shared version for use in libc.a shr.o
#                                                                    
# NOTES:
#
# This is (supposed to be) identical to itrunc.s !
# Any changes made to one should be made to both.
#
#       int shr_itrunc( double x );
#
#       Double to Integer Conversion -- Truncate (Round toward zero)
#
#       itrunc is the function performed when a FORTRAN or C double
#       is converted to a signed integer.
#
#       Converts a double precision value in fr0 to a signed
#       integer in r3 as if the rounding mode were round toward zero.
#
#       itrunc expects its argument to be an FPR not a GPR; Therefore
#       a C prototype should be used so that the argument is only
#       passed in the FPR's.
#
#
#       The algorithm is:
#
#          Save caller's rounding mode
#
#          if (-2^31 < nbr < 2^31)
#
#              if ( nbr > 0 )
#
#                  Set rounding mode to "Round toward minus infinity"
#                  which is equivalent to RZ for positive numbers.
#
#                  Add 2^52+2^44 to nbr. This will right justify the integer
#                  part of the nbr in the lower word of the fp double and
#                  will correctly round it.
#
#                  Store this double number to memory and load only
#                  the lower word into an integer register.
#
#              else
#
#                  Set rounding mode to "Round toward plus infinity"
#                  which is equivalent to RZ for negative numbers.
#
#                  Add 2^52+2^44 to the number. This will right justify the
#                  integer part of the nbr in the lower word of the fp
#                  double and will correctly round it.
#
#                  Store this double number to memory and load only
#                  the lower word into an integer register.
#
#
#              Restore the callers rounding mode
#
#           else  /* number is too big or is funny */
#
#              Restore the callers rounding mode
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
#       Note 1: The actual code below does the add of 2^52+2^44
#               before branching on a funny or big number. (This is done
#               to avoid doing the branch for as long as possible which
#               makes code faster for the normal case.) If the branch
#               is taken the add result is ignored.
#
#
#       Performance estimate = 12 clocks for a positive double argument
#       Performance estimate = 13 clocks for a negative double argument
#
#
#       IEEE Status Bits that might be set are:
#
#          nvcvi        Invalid Conversion. The double number was
#                       unrepresentable in integer (NaN, too big, infinity).
#          vx           Invalid summary bit
#          nvsnan       Signaling NaN. The double number was an SNaN.
#          inexact      Inexact. The double number was rounded.
#
# RETURNS: 
#	A signed integer
#
# ***********************************************************************
	.file "shr_itrunc.s"
#
#       Define a TOC entry that points to our constant data
#
	.toc
	TOCL(shr_itrunc_const,data)
z0x0000.S:  .tc  _0x00000000[tc],0     # put single 0 in toc

#
#
#       Code
#
#

	.align  2
	S_PROLOG(shr_itrunc)

	lfs     fr0,z0x0000.S(r2)      # in case fr0 is not already 0.0

# **************************************************************************


	fcmpu   cr6,fr1,fr0     # Is x > 0 (may set VXSNAN)
	LTOC(r6,shr_itrunc_const,data) # Load pointer to constants
	fabs    fr5,fr1         # abs(x)
	lfd     fr2,8(r6)       # Load 2^31
	mffs    fr4             # Save caller's rounding mode (save FPSCR)
	fcmpu   cr0,fr5,fr2     # Is abs(x) >= 2^31? (may set VXSNAN)
	lfd     fr3,0(r6)       # Load the magic number (2^52 + 2^40)
	blt     cr6,itrunc_neg  # Branch if x is negative
	mtfsb1  30              # Set rounding mode to RM for negative nbr.
	mtfsb1  31
#
#       The next instruction causes the integer part of the number
#       to be rounded (and biased) and put in the lower word of the double.
#
	fa      fr6,fr1,fr3     # Add the magic number (may set XX)
	stfd    fr6,-8(r1)      # Store the double on the stack
	mtfsf   1,fr4           # Restore low 4 bits of FPSCR (round mode)
	bnl     cr0,itrunc_spec  # Go process NaN's, Infinities, and too big
	l       r3,-4(r1)       # Load the lower part (now the integer)
	br

itrunc_neg:
	mtfsb1  30              # Set rounding mode to RP for negative nbr.
	mtfsb0  31
	fa      fr6,fr1,fr3     # Add the magic number (may set XX)
	stfd    fr6,-8(r1)      # Store the double on the stack
	mtfsf   1,fr4           # Restore low 4 bits of FPSCR (round mode)
	bgt     cr0,itrunc_spec  # Go process too big
	l       r3,-4(r1)       # Load the lower part (now the integer)
	br

#
#       Here to take care of a funny or too big double value
#
#               We will have to call a routine to set the
#               invalid compare bit in the IEEE Status so we now
#               need to do the standard subroutine linkage stuff below.
#
	.extern  ENTRY(fp_raise_xcp)  # Routine to set exception bits

itrunc_spec:
        mtfsf   0xFF,fr4        # Restore caller's FPSCR (original flags) 
	mflr    r0              # Get the current link register
	st      r31,-4(r1)      # Save r31 on the stack
	st      r0,8(r1)        # Save the link register for return
	stu     r1,-60(r1)      # Create stack frame and backtrace
#
#       Here to generate a result
#
	cau     r31,r0,0x8000   # make default result = 0x80000000
	bbt     3,d2ic          # (bun) Result = default if unordered (NaN)
#       Here when number is too big (including infinities)
	blt     cr6,d2ic        # result = 0x80000000 if -toobig
	sfi     r31,r31,-1      # +toobig; make result = 0x7fffffff
#
#       Here to do the call to set the invalid conversion
#       bit in the IEEE Status and raise the invalid conversion
# 	exception if it's enabled.
#
d2ic:
	l       r3,16(r6)       # load invalid conversion + inexact mask
	bl      ENTRY(fp_raise_xcp)  # Do the call to set nvcvi bit in stat
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
	br
	_DF(0x3,0x1,0x8,0x0,0x0,0x2,0xC0000000) # Traceback Table info.

#
# ************************************************************************
#
#       Constant Area
#
# ************************************************************************
#

	   .csect  DATA(shr_itrunc_const[RO]),3
shr_itrunc_const:
	   .long   0x43300800,0x00000000   # 2^52 + 2^44
	   .long   0x41e00000,0x00000000   # 2^31
	   .long   fpxnvcvi     # Invalid Conversion Bit

#
# ************************************************************************
#
#       Function Descriptor
#
# ************************************************************************
#
	FCNDES(shr_itrunc)

include(sys/comlink.m4)
