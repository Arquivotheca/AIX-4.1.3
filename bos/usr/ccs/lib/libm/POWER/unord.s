# @(#)86	1.9  src/bos/usr/ccs/lib/libm/POWER/unord.s, libm, bos411, 9428A410j 2/21/94 14:03:40
#
# COMPONENT_NAME: (LIBM) math library
#
# FUNCTIONS: unordered
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
# NAME: unordered
#                                                                    
# FUNCTION: Determine if comparing x and y would be unordered
#                                                                    
# EXECUTION ENVIRONMENT:
#                                                                   
#       Unordered expects its arguments to be in FPR's not in two pairs of
#       GPR's. Therefore, a C template should be used so that the arguments
#       are ONLY passed in the FPR's.
#                                                                   
# NOTES:
#
#       IEEE Recommended "unordered" Function
#
#       unordered(x,y)
#
#       Returns true if x is unordered with regard with y.
#       X will be unordered with y if either x or y is a NaN
#
#       This function conforms to the IEEE P754 standard for
#       the recommended unordered function.
#
#       The algorithm for SJ is trivial:
#          compare x with y
#          if (compare result is unordered) then return(true)
#          else return(false)
#
#       Performance estimate = 6 clocks for ordered
#       Performance estimate = 8 clocks for unordered
#
#
#       IEEE Status Bits that might be set are:
#
#          nvsnan       Signaling NaN. x or y or both were an SNaN.
#
#
# RETURNS: Returns a non-zero value if the comparison would be unordered,
#	   otherwise 0.
#
# ***********************************************************************
	.file "unord.s"
#
#
#       Code
#
#

	.align  3
	S_PROLOG(unordered)

# **************************************************************************

	fcmpu   cr0,fr1,fr2     # set/reset unordered bit  (may set VXSNAN)
	lil     r3,0            # Assume false (ordered)
	bbfr    cr0_3           # Return if false
	lil     r3,1            # Set to true
	S_EPILOG

#
# **********************************************************************
#
#       Function Descriptor
#
# **********************************************************************
#
	FCNDES(unordered)
