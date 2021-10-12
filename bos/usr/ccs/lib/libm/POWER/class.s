# @(#)04	1.7  src/bos/usr/ccs/lib/libm/POWER/class.s, libm, bos411, 9428A410j 2/21/94 14:02:09
#
# COMPONENT_NAME: (LIBM) math library
#
# FUNCTIONS: class
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
# NAME: class
#                                                                    
# FUNCTION: 
#                                                                    
# EXECUTION ENVIRONMENT:
#                                                                   
#       Class expects its argument to be in a pair of GPR's rather than
#       an FPR. Therefore NO C TEMPLATE should be used in defining
#       this routine externally.
#                                                                   
# NOTES:
#
#       IEEE Recommended "class" Function
#
#       class(x)
#
#       This function conforms to the IEEE P754 standard for
#       the class recommended function.
#
#       The algorithm is to split the number up into
#       its components: exponent, sign, fraction hi part, and
#       fraction low part. Then the exponent is tested followed
#       by tests of the sign and fraction if required.
#
#
#       Performance estimate =  7 clocks for x == normalized
#       Performance estimate = 11 clocks for x == zero
#       Performance estimate = 14 clocks for x == denormalized
#       Performance estimate = 13 clocks for x == infinity
#       Performance estimate = 15 clocks for x == SNaN
#       Performance estimate = 15 clocks for x == QNaN
#
#
#       IEEE Status Bits that might be set are:
#
#                none  (as req'd by the IEEE standard)
#
# RETURNS: Returns an index of the class of the argument:
#
#               0  =  + normalized    =   FP_PLUS_NORM    \
#               1  =  - normalized    =   FP_MINUS_NORM    |
#               2  =  + 0             =   FP_PLUS_ZERO     |
#               3  =  - 0             =   FP_MINUS_ZERO    |
#               4  =  + infinity      =   FP_PLUS_INF      > defined in
#               5  =  - infinity      =   FP_MINUS_INF     | float.h
#               6  =  + denormalized  =   FP_PLUS_DENORM   |
#               7  =  - denormalized  =   FP_MINUS_DENORM  |
#               8  =  SNaN            =   FP_SNAN          |
#               9  =  QNaN            =   FP_QNAN         /
#
#
# ***********************************************************************
	.file "class.s"
#
#
#       Code
#
#

	.align  3

	S_PROLOG(class)

	rlinm.  r5,r3,12,0x7ff  # extract the exponent (also sets CR0)
	rlinm   r7,r3,0,0x000fffff  # extract upper fraction bits
	cmpli   cr1,r5,0x7ff    # cr1_eq set if exponent = 0x7ff (max)
	rlinm   r3,r3,1,0x1     # r3 = sign bit: r3 == 0 if plus
				#                r3 == 1 if minus
	cror    cr6_2,cr0_2,cr1_2  # cr6_2 true if x not normalized
	bbfr    cr6_2           # exit if normalized (result = 0 or 1)

#
#       Here when x was not normalized
#
#       Is the exponent zero or 0x7ff ? cr0 eq bit set if exponent was zero.

	bne     cr0,class_inf_nan  # must be an infinity or NaN

#
#       Here when x is zero or denormalized
#

	or.     r8,r7,r4        # or together the fractions bits
				# and set cr0
	ai      r3,r3,2         # assume x is zero
	beqr    cr0             # It is zero: result = 2 or 3
	ai      r3,r3,4         # It is denorm: result = 6 or 7
	br

#
#       Here when x is infinity or NaN
#
class_inf_nan:
	or.     r8,r7,r4        # or together the fractions bits
				# and set cr0
	ai      r3,r3,4         # assume x is infinity
	beqr    cr0             # It is infinity: result = 4 or 5
	rlinm   r3,r7,13,0x1    # NaN: get the QNaN bit
	ai      r3,r3,8         # Add QNaN bit to 8 (SNaN = 8; QNaN = 9)
	S_EPILOG


#
# ************************************************************************
#
#       Function Descriptor
#
# ************************************************************************
#
	FCNDES(class)
