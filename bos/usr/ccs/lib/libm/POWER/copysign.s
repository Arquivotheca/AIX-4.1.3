# @(#)28	1.11  src/bos/usr/ccs/lib/libm/POWER/copysign.s, libm, bos411, 9428A410j 2/21/94 14:02:17
#
# COMPONENT_NAME: (LIBM) math library
#
# FUNCTIONS: copysign
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
# NAME: copysign
#                                                                    
# FUNCTION: COPIES THE SIGN OF Y INTO THE SIGN OF X
#                                                                    
# EXECUTION ENVIRONMENT:
#                                                                   
#       Copysign expects its arguments to be in FPR's not GPR's;
#       Therefore, a C template should be used so that the arguments
#       are ONLY passed in the FPR's.
#                                                                   
# NOTES:
#
#       IEEE Recommended Copysign Function
#
#       Copysign(x,y)
#
#       Copies the sign of y into the sign of x. QNaN's and SNaN's are
#       treated like any other number (and don't except).
#
#       This function conforms to the IEEE P754 standard recommended
#       copysign function.
#
#       The algorithm is:
#
#             Store y in memory and extract it's sign
#             if (y > 0)
#                 return( abs(x) )
#             if (y < 0)
#                 return( -abs(x) )
#             else  /* y is zero or NaN */
#                 put y in memory
#                 set sign of x to sign of y
#
#       Performance estimate = appx. 7 clocks for positive non zero y
#       Performance estimate = appx. 9 clocks for negative non zero y
#
#
#       IEEE Status Bits that might be set are: None
#
# RETURNS: x with the sign of y
#
# ***********************************************************************
	.file "copysign.s"
#
	.toc
z0x0000.S:  .tc  _0x00000000[tc],0     # put single 0 in toc
#
#       Code
#
#

	.align  2

	S_PROLOG(copysign)

	lfs     fr0,z0x0000.S(r2)      # in case fr0 is not already 0.0

# **************************************************************************

	mffs    fr3             # save status in case vxsnan is set
	fcmpu   cr0,fr2,fr0     # compare y to 0.0 . May set VXSNAN
	fabs    fr1,fr1         # assume y is > 0.0, set x positive
	bgtr                    # return if y positive
	fnabs   fr1,fr1         # now assume y is < 0.0, set y negative
	bltr                    # return if y is negative
#
#       Here when y is zero or a NaN
#
	mtfsf   0xff,fr3        # restore caller's status (may erase vxsnan)
	stfd    fr2,-8(r1)      # put y on the stack
	fabs    fr1,fr1         # assume y is positive
	l       r3,-8(r1)       # get ms word of y
	andiu.  r3,r3,0x8000    # if y is positive then cr0 is EQ
	beqr    cr0             # if (y>0) return( abs(x) )
	fnabs   fr1,fr1         # else return( -abs(x) )
	S_EPILOG


#
# ************************************************************************
#
#       Function Descriptor
#
# ************************************************************************
#
	FCNDES(copysign)
