# @(#)54	1.17  src/bos/usr/ccs/lib/libc/POWER/uitrunc.s, libccnv, bos411, 9428A410j 2/21/94 14:00:56
#
# COMPONENT_NAME: LIBCCNV uitrunc
#
# FUNCTIONS: uitrunc
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
# NAME: uitrunc
#                                                                    
# FUNCTION: convert a double to an unsigned integer
#                                                                    
# NOTES:
#
#       unsigned int uitrunc( double x );
#
#       Unsigned Double to Integer Conversion -- Truncate (Round toward 0)
#
#       itrunc is the function performed when a FORTRAN or C double
#       is converted to an unsigned integer.
#
#       Converts a double precision value in fr1 to an unsigned
#       integer in r3 as if the rounding mode were round toward zero.
#
#       uitrunc expects its argument to be an FPR not a GPR; Therefore
#       a C prototype should be used so that the argument is only
#       passed in the FPR's.
#
#
#       The algorithm is:
#
#          Save caller's rounding mode
#
#          Set rounding mode to RZ
#
#          if ( 0 <= nbr < 2^32)
#
#                  Add 2^52+2^44 to nbr. This will right justify the integer
#                  part of the nbr in the lower word of the fp double and
#                  will correctly round it.
#
#                  Store this double number to memory and load only
#                  the lower word into an integer register.
#
#                  Restore the callers rounding mode
#
#           else  /* number is negative, too big or is funny */
#
#               Restore the callers rounding mode
#
#               set the result to:
#                   if(nbr <  0)         return(0x00000000)
#                   if(nbr == NaN)       return(0x00000000)
#                   if(nbr >  2^32)      return(0xffffffff)
#                   if(nbr == +infinity) return(0xffffffff)
#
#               Set the invalid conversion bit in the IEEE status.
#
#           end
#
#       Note 1: The actual code below does the add of 2^52+2^44
#               before branching on a funny or big number. (This is done
#               to avoid doing the branch for as long as possible which
#               makes SJ code faster for the normal case.) If the branch
#               is taken the add result is ignored.
#
#
#       Performance estimate = 12 clocks for in-range number
#
#
#       IEEE Status Bits that might be set are:
#
#          nvcvi        Invalid Conversion. The double number was
#                       unrepresentable as an integer (NaN, negative, too
#                       big, or infinity).
#          vx           Invalid summary bit
#          nvsnan       Signaling NaN. The double number was an SNaN.
#          inexact      Inexact. The double number was rounded.
#
#
# RETURNS: 
#	An unsigned integer
#
# NOTE:  The files uitrunc.s and _uitrunc.s are exactly
#        identical except for lines involving the name.
#        Please keep them that way.
# ***********************************************************************
	.file "uitrunc.s"
#
#
#       Define a TOC entry that points to our constant data
#
	.toc
	TOCL(uitrunc_const,data)
z0x0000.S:  .tc  _0x00000000[tc],0     # put single 0 in toc

#
#
#       Code
#
#
	.align  2
	S_PROLOG(uitrunc)

	lfs     fr0,z0x0000.S(r2)      # in case fr0 is not already 0.0

# **************************************************************************


	fcmpu   cr6,fr1,fr0     # test to see if x >= 0
	LTOC(r6,uitrunc_const,data) # Load pointer to constants
	mffs    fr4             # Save caller's rounding mode (save FPSCR)
	lfd     fr2,8(r6)       # Load 2^32
	mtfsb0  30              # Set rounding mode to RZ
	fcmpu   cr0,fr1,fr2     # Is x >= 2^32 (may set VXSNAN)
	lfd     fr3,0(r6)       # Load the magic number (2^52 + 2^44)
	mtfsb1  31              # Set rounding mode to RZ
	blt     cr6,uitrunc_neg  # if (x<0) then go to spec handling
#
#       The next instruction causes the integer part of the number
#       to be rounded (and biased) and put in the lower word of the double.
#
	fa      fr6,fr1,fr3     # Add the magic number (may set XX)
	stfd    fr6,-8(r1)      # Store the double on the stack
	mtfsf   1,fr4           # Restore low 4 bits of FPSCR (round mode)
	bnl     cr0,uitrunc_spec  # Go process NaN's, Infinities, and too big
	l       r3,-4(r1)       # Load the lower part (now the integer)
	br
#
#       Here when x is negative
#
uitrunc_neg:
	mtfsf   1,fr4           # Restore low 4 bits of FPSCR (round mode)
	lil     r3,0            # All negative numbers return 0
	b       uitrunc_except  # Go do an exception

#
#       Here when x is too big or a NaN
#
uitrunc_spec:
	lil     r3,0
	bbt     3,uitrunc_except   # if unordered, result = 0
	lil     r3,-1              # if too big or NaN, result = MAXINT
	b       uitrunc_except     # go signal an exception
#  
	br

#
#        Call a routine to set the invalid compare bit in the IEEE
#        Status.
#
	.extern  ENTRY(fp_raise_xcp)  # Routine to set exception bit

uitrunc_except:
        mtfsf   0xFF,fr4        # Restore caller's FPSCR (original flags)
#       Do the standard linkage convention
	mflr    r0              # Get the current link register
	st      r31,-4(r1)      # Save r31 on the stack
	st      r0,8(r1)        # Save the link register for return
	stu     r1,-60(r1)      # Create stack frame and backtrace
	mr      r31,r3          # Save the result in r31
#
#       Here to do the call to set the invalid conversion
#       bit in the IEEE Status and raise the exception if enabled
#
	l       r3,16(r6)       # load invalid conversion + inexact mask
	bl      ENTRY(fp_raise_xcp) # Do the call to set nvcvi bit status
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
	_DF(0x3,0x1,0x8,0x0,0x0,0x2,0xC0000000) # traceback table info.
#
# ************************************************************************
#
#       Constant Area
#
# ************************************************************************
#

	   .csect  DATA(uitrunc_const[RO]),3
uitrunc_const:
	   .long   0x43300800,0x00000000   # 2^52 + 2^44
	   .long   0x41f00000,0x00000000   # 2^32
	   .long   fpxnvcvi     # Invalid Conversion Bit

#
# ************************************************************************
#
#       Function Descriptor
#
# ************************************************************************
#
	FCNDES(uitrunc)

include(sys/comlink.m4)
