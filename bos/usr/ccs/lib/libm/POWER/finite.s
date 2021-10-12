# @(#)46	1.8  src/bos/usr/ccs/lib/libm/POWER/finite.s, libm, bos411, 9428A410j 2/21/94 14:02:36
#
# COMPONENT_NAME: (LIBM) math library
#
# FUNCTIONS: finite
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
#
# NAME: finite
#                                                                    
# FUNCTION: DETERMINES IS A NUMBER IS FINITE
#                                                                    
# EXECUTION ENVIRONMENT:
#                                                                   
#       Finite expects its argument to be in a pair of GPR's rather than
#       an FPR. Therefore NO C TEMPLATE should be used in defining
#       this routine externally.
#                                                                   
# NOTES:
#
#       IEEE Recommended "finite" Function
#
#       finite(x)
#
#       Returns true if x is finite (not an infinity or a NaN)
#
#       This function conforms to the IEEE P754 standard for
#       the "finite" recommended function.
#
#       The algorithm for SJ is trivial:
#             extract the exponent of x
#             if (exponent == max) return(false)
#             else return(true)
#
#
#       Performance estimate = 5 clocks when x is finite *
#       Performance estimate = 7 clocks when x is not finite *
#
#
#       IEEE Status Bits that might be set are: none
#
#
# RETURNS: Returns a non-zero value if x is finite (not an infinity or a NaN);
#	   0 otherwise.
#
#
# ***********************************************************************
	.file "finite.s"
#
#
#       Code
#
#

	.align  3

	S_PROLOG(finite)

	rlinm   r5,r3,12,0x7ff  # Extract the exponent
	cmpli   cr0,r5,0x7ff    # Is exponent == max?
	lil     r3,1            # Set result true
	bner                    # if (exp != max) return(true)
	lil     r3,0            # if (exp == max) return(false)
	S_EPILOG

	FCNDES(finite)


