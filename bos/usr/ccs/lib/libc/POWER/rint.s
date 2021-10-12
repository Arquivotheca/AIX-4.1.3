# @(#)30	1.8  src/bos/usr/ccs/lib/libc/POWER/rint.s, libccnv, bos411, 9428A410j 2/21/94 14:00:41
#
# COMPONENT_NAME: LIBCCNV rint
#
# FUNCTIONS: rint
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
# NAME: rint
#                                                                    
# FUNCTION: round to nearest
#                                                                    
# NOTES:
#
#       double rint(double x);
#
#       IEEE Round to Nearest Integer Function
#
#       This function is also commonly called "Integer Part".
#
#       Converts a double precision value in fr1 to an integral
#       valued double number in fr1. That is, the result is a
#       double number that only contains the integer part of the
#       input double. For example, 3.45 becomes 3.00.
#
#       This function conforms to the IEEE P754 standard.
#
#       Rint expects its argument to be an FPR not a pair of GPR's.
#       Therefore, a C template should be used so that the argument
#       is ONLY passed in the FPR's.
#
#
#       The algorithm is:
#
#       if (x == 0) return(x)
#       if (abs(x) >= 2^52) return(x)
#       if ((rounding mode = RP or RM) && (x < 0))
#           if (rounding mode = RP)
#              switch to RM
#              temp = abs(x) + 2^52
#              result = temp - 2^52
#              switch back to RP
#              return(-result)
#           else   /* RM */
#              switch to RP
#              temp = abs(x) + 2^52
#              result = temp - 2^52
#              switch back to RM
#              return(-result)
#       /* Positive x or Rounding mode = RN or RZ */
#       temp = abs(x) + 2^52
#       result = temp - 2^52
#       if (x < 0) result = -result
#       return(result)
#
#       The basic idea of the algorithm is to add 2^52 to x to cause
#       it to right shift the integer part of the number to the lower
#       right of the fraction so that it will be rounded correctly.
#       Then 2^52 is subtracted to reposition the rounded integer part.
#       Negative x's add 2^52 to abs(x) and then negate the result.
#       The directed rounding modes (RM and RP) cause problems when
#       x is negative. In these cases the add to abs(x) will round in
#       the opposite direction than desired. Therefore for RM and RP
#       the rounding modes are reversed for the add and subtract.
#
#       NOTE: The tests for (x == 0) and (abs(x) > 2^52) are code
#       motioned into the normal code for efficiency reasons.
#       Some extra logic was required in RM mode to insure that
#       the sign of a zero result is correct as per the IEEE Standard.
#
#
#       Performance estimate = 13 clocks for RN or RZ positive norms
#       Performance estimate = 15 clocks for RN or RZ negative norms
#       Performance estimate = 20 clocks for RM positive norms
#       Performance estimate = 22 clocks for RM negative norms
#       Performance estimate = 20 clocks for RP positive norms
#       Performance estimate = 20 clocks for RP negative norms
#
#
#       IEEE Status Bits that might be set are:
#
#          nvsnan       Signaling NaN. The double number was an SNaN.
#          inexact      Inexact. The double number was rounded.
#
#
# RETURNS: 
#	A double rounded to the nearest integer
#
# ***********************************************************************
	.file "rint.s"
#
#
#       Define a TOC entry that points to our constant data
#
	.toc
	TOCL(rint_const,data)
z0x0000.S:  .tc  _0x00000000[tc],0     # put single 0 in toc

#
#
#       Code
#
#

	.align  3

	S_PROLOG(rint)

	lfs     fr0,z0x0000.S(r2)      # in case fr0 is not already 0.0

# **************************************************************************


	fabs    fr2,fr1         # Take the absolute value of input x
	LTOC(r6,rint_const,data)
	mcrfs   cr1,7           # Transfer rounding mode from FPSCR to cr1
	lfd     fr3,0(r6)       # Load 2^52
	fcmpu   cr0,fr2,fr3     # Is abs(x) >= 2^52? (may set VXSNAN)
	fcmpu   cr6,fr1,fr0     # Compare x with zero
	bbt     cr1_2,rnt_rmrp  # Branch if round mode = RM or RP
#
#       The 'fa' causes the integer part of the abs(number) to be right
#       justified in the fraction and rounded. The 'fs' causes the
#       rounded number to be renormalized.
#
	bnl     cr0,rnt_special # if (x !< 2^52) goto special processing
	fa      fr4,fr2,fr3     # temp = abs(nbr) + 2^52 (may set XX)
	beqr    cr6             # if (x == 0) return(x)
	fs      fr1,fr4,fr3     # result = temp - 2^52 (renormalize)
	bgtr    cr6             # Return if input nbr was positive
	fnabs   fr1,fr1         # Negate result if input nbr was negative
	br                      # Return

#
#       Here when in RM or RP mode
#
rnt_rmrp:
	bnl     cr0,rnt_special  # if (x !< 2^52) goto special processing
	bge     cr6,rnt_rmrp_pos   # positive or zero?
	bbt     cr1_3,rnt_rm_neg   # Branch if RM mode and negative
#
#       Here when x is negative and in RP mode
#
	mtfsb1  31              # Change to RM for the calculation
	fa      fr4,fr2,fr3     # temp = abs(nbr) + 2^52 (may set XX)
	fs      fr1,fr4,fr3     # result = temp - 2^52 (renormalize)
	mtfsb0  31              # Change back to RP
	fnabs   fr1,fr1         # Negate result since number was negative
	br                      # Return
#
#       Here when x is negative and in RM mode
#
rnt_rm_neg:
	mtfsb0  31              # Change to RP for the calculation
	fa      fr4,fr2,fr3     # temp = abs(nbr) + 2^52 (may set XX)
	fs      fr1,fr4,fr3     # result = temp - 2^52 (renormalize)
	mtfsb1  31              # Change back to RM
	fnabs   fr1,fr1         # Negate result since nbr was negative
	br                      # Return
#
#       Here when x is positive or zero and in RM or RP mode
#
#           This code is necessary because the addition to and subtraction
#           from a positive number could yield -0.0 but the IEEE standard
#           requires that the result have the same sign as the input (x).
#           Therefore, this code has a fabs that the mainline code doesn't.
#
rnt_rmrp_pos:
	fa      fr4,fr2,fr3     # temp = abs(nbr) + 2^52 (may set XX)
	beqr    cr6             # if (x == 0) return(x)
	fs      fr1,fr4,fr3     # result = temp - 2^52 (renormalize)
	fabs    fr1,fr1         # Make sure result is positive
	br                      # Return


#
#
#       Here to handle NaN's and "too big numbers"
#
#
rnt_special:
	fa      fr1,fr1,fr0     # Dummy add of 0.0 to change SNaN to QNaN
	S_EPILOG


#
# ************************************************************************
#
#       Constant Area
#
# ************************************************************************
#
	   .csect  DATA(rint_const[RO]),3
rint_const:
	 .long   0x43300000,0x00000000   # 2^52

#
# ************************************************************************
#
#       Define function descriptor
#
# ************************************************************************
#
	FCNDES(rint)
