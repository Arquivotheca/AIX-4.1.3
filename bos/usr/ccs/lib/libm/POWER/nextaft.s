# @(#)46	1.17  src/bos/usr/ccs/lib/libm/POWER/nextaft.s, libm, bos411, 9428A410j 2/21/94 14:03:11
#
# COMPONENT_NAME: (LIBM) math library
#
# FUNCTIONS: nextafter
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
# NAME: nextaft
#                                                                    
# FUNCTION: Return the next representable neighbor of x is the direction
#	    towards y.
#                                                                    
# EXECUTION ENVIRONMENT:
#                                                                   
#      Nextafter expects its arguments to be in BOTH an FPR and a pair of
#      GPR's. Therefore NO C template should be provided for this
#      function! This is done because it is more efficient for the
#      compiler to move the arguments to pairs of GPR's (or to the
#      FPR's) before the call than to do it inside the routine. The
#      reason it's more efficient is that the compiler may be able
#      to code motion the load(s) upstream from the call.
#                                                                   
# NOTES:
#
#      IEEE Recommended Nextafter Function
#
#      double nextafter(double x, double y);
#
#      Next representable number after x in the direction of y.
#
#      This function conforms to the IEEE P754 standard recommended
#      nextafter function.
#
#      The algorithm is:
#
#      if (x == y) return(x);
#      if ( x or y is a NaN ) return(x*y);  /* filter out NaNs */
#      if ( ( ((x > 0) && (x < y)) || ((x < 0) && (x > y)) ) || (x == 0) )
#           then ADD1 else SUB1
#      return(x)
#
#      Where ADD1 adds 1 ulp to the sign magnitude fraction of x and checks
#      for overflow and SUB1 subtracts 1 ulp from the fraction of x and
#      checks for underflow. If x == 0 then ADD1 is called but the sign
#      of the ADD1 needs to be the same as the sign of y.
#
#
#       Performance estimate = 13 - 16 clocks for normal cases
#
#
#       IEEE Status Bits that might be set are:
#
#          ox           Overflow. Set if x + 1ulp overflows to infinity.
#          ux           Underflow. Set if x -1 ulp of x + 1 ulp is denorm
#                       or zero .
#          xx           Inexact. Set if either of the above are set.
#          vxsnan       SNaN. Set if x or y was an SNaN
#
#
# RETURNS: Next representable number after x in the direction of y.
#
# ***********************************************************************
	.file "nextaft.s"
#
#
#       Define constants that contain the upper half of the exception
#       status bits that need to be set on overflow or underflow
#
	.set    OVX,0x1200      # overflow
	.set    UNX,0x0a00      # underflow
# errno.h
	.set ERANGE,34

	.toc
	TOCE(errno,data)               # external error variable
z0x0000.S:  .tc  _0x00000000[tc],0     # put single 0 in toc
#
#
#       Code
#
#

	.align  2
	S_PROLOG(nextafter)

	lfs     fr0,z0x0000.S(r2)      # in case fr0 is not already 0.0

# **************************************************************************
#
#       On entry:
#           r3  = high x
#           r4  = low x
#           r5  = high y
#           r6  = low y
#
#       The code to do ADD1 and SUB1 has been scheduled up in the
#       code to reduce compare condition result delays. Both x + 1 ulp
#       and x - 1 ulp are pre-calculated in registers. Once it is known
#       whether we want + 1 ulp or -1 ulp then the correct value is
#       loaded into the result (fr1).
#
#
	fcmpu   cr0,fr1,fr2     # cr0: cmp x to y
	fcmpu   cr1,fr1,fr0     # cr1: cmp x to 0.0
	ai      r6,r4,1         # ADD1: add 1 to lsb of x
	aze     r5,r3           # ADD1: if carry then add 1 to msb
	ai      r8,r4,-1        # SUB1: sub 1 from lsb of x
	ame     r7,r3           # SUB1: dec msb if borrow
	beqr    cr0             # if (x == y) return(x)
#
#       r5 =    high part of x + 1 ulp
#       r6 =    low  part of x + 1 ulp
#       r7 =    high part of x - 1 ulp
#       r8 =    low  part of x - 1 ulp
#
#       cr0     x fcmpu y
#       cr1     x fcmpu 0.0
#
#       if ( ( ((x > 0) && (x < y)) || ((x < 0) && (x > y)) ) || (x == 0) )
#           then ADD1 else SUB1
#       Note: NaNs also go to sub1
#
	crand   cr7_3,cr1_1,cr0_0  # cr7_0 = ((x > 0) && (x < y))
	crand   cr6_3,cr1_0,cr0_1  # cr6_0 = ((x < 0) && (x > y))
	cror    cr1_3,cr6_3,cr7_3  # || (or) of the above
	cror    cr6_3,cr1_3,cr1_2  # || (x == 0)
	bbf     cr6_3,na_sub1      # if false then SUB1
#
#       ADD1: Here to add 1 ulp to x's fraction
#
na_add1:
	st      r6,-12(r1)      # store x + 1 ulp to memory
	st      r5,-16(r1)
	lfd     fr1,-16(r1)     # load x into result
	beq     cr1,na_zero     # if (x==0) goto na_zero
	rlinm.  r9,r5,12,0x000007ff # extract exponent of x, set cr0
	cmpli   cr7,r9,0x07ff   # if (x == inf) then overflow
	liu     r3,UNX          # load up underflow + inexact constant
	beq     cr0,na_except   # if x is still denorm then underflow
	bner    cr7             # return if no overflow
	liu     r3,OVX          # load up overflow + inexact constant
# errno = ERANGE
	LTOC(r7,errno,data)     # Get pointer to errno
	cal     r5,ERANGE(0)	# range error code to r5
	st      r5,0(r7)        # errno = ERANGE
	b       na_except       # go process overflows
#
#       SUB1: Here to sub 1 ulp from x's fraction
#
na_sub1:                        # else SUB1
	st      r8,-12(r1)      # store x - 1 ulp to memory
	st      r7,-16(r1)
	bbt     cr0_3,na_nan    # if ( x or y is NaN ) goto na_nan
	lfd     fr1,-16(r1)     # load x into result
	rlinm.  r9,r7,12,0x000007ff # extract exponent of x, set cr0
	liu     r3,UNX          # load up underflow + inexact constant
	bner    cr0             # exit if no underflow
	b       na_except       # go process underflows

#
#       Here when x was zero
#
#       The result of x + 1 ulp is already loaded into fr1. However,
#       the sign of the result is dependent on whether y was negative
#       or positive. ( The sign of the result = the sign of y). The
#       code below gets the sign right and then goes to process the
#       underflow exception since the result is always denormalized.
#
na_zero:
	liu     r3,UNX          # load underflow + inexact mask
	bbt     cr0_3,na_nan    # if nextafter(0.0,NaN) case goto na_nan
	fabs    fr1,fr1         # assume sign of y was + (so set x +)
	blt     cr0,na_except   # if x == 0 and x < y then y > 0
	fnabs   fr1,fr1         # set sign if y < 0
	b       na_except       # go process underflow + inexact exception

#
#       Here to process a NaN
#
na_nan:
	fm      fr1,fr1,fr2     # return a QNaN as the result (May set
	br                      # VXSNAN); return(x*y)

#
#
# *************************************************************************
#
#
#       Here to report an overflow or underflow and an inexact
#
#               r3 contains the exception mask flag
#

	.extern  ENTRY(fp_set_flag)   # Routine to set exception bits

na_except:
	mflr    r0              # Get the current link register
	stfd    fr1,-8(r1)      # Save result on the stack
	st      r0,8(r1)        # Save the link register for return
	stu     r1,-64(r1)      # Create stack frame and backtrace
#
#       Here to do the call to set the status bits in the IEEE Status
#
	bl      ENTRY(fp_set_flag)
	l	r2, stktoc(r1)	# restore toc pointer
#
#       Restore the result and do the epilogue code
#
	l       r0,72(r1)       # Get saved link register
	ai      r1,r1,64        # Remove the stack frame
	mtlr    r0              # Restore the link register
	lfd     fr1,-8(r1)      # Restore the result
#
#
	br  			
	_DF(0x0,0x1,0x8,0x0,0x0,0x0,0x0)  # Traceback Table info.


#
# **********************************************************************
#
#       Function Descriptor
#
# **********************************************************************
#
	FCNDES(nextafter)

include(sys/comlink.m4)
