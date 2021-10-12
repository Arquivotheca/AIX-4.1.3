# @(#)47	1.9  src/bos/usr/ccs/lib/libm/POWER/isnan.s, libm, bos411, 9428A410j 2/21/94 14:02:54
#
# COMPONENT_NAME: (LIBM) math library
#
# FUNCTIONS: isnan
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
# NAME: isnan
#                                                                    
# FUNCTION: Determine is x is a NaN
#                                                                    
# EXECUTION ENVIRONMENT:
#                                                                   
#       Isnan expects its argument to be in an FPR not a pair of
#       GPR's. Therefore, a C template should be used so that the argument
#       is ONLY passed in the FPR's.
#                                                                   
# NOTES:
#
#       IEEE Recommended isnan Function
#
#       isnan(x)
#
#       Returns true if x is either a QNaN or an SNaN
#
#       This function conforms to the IEEE P754 standard for
#       the isnan recommended function.
#
#       The algorithm is trivial:
#               if (x != x) then return(true)
#               else return(false);
#
#
#       Performance estimate = 6 clocks for x != NaN
#       Performance estimate = 8 clocks for x == NaN
#
#
#       IEEE Status Bits that might be set are:
#
#          nvsnan       Signaling NaN. The double number was an SNaN.
#
# RETURNS: Returns a non-zero value if x is a NaN; 0 otherwise
#
#
# ***********************************************************************
	.file "isnan.s"
#
#
#       Code
#
#

	.align  3
	S_PROLOG(isnan)

# **************************************************************************

	fcmpu   cr0,fr1,fr1     # not eq if fr1 is a NaN (may set VXSNAN)
	lil     r3,0            # Assume false (not a NaN)
	beqr    cr0             # Return if false
	lil     r3,1            # Set to true
	S_EPILOG


#
# ************************************************************************
#
#       Function Descriptor
#
# ************************************************************************
#
	FCNDES(isnan)
