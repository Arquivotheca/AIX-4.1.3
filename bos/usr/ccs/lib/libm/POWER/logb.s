# @(#)82	1.13  src/bos/usr/ccs/lib/libm/POWER/logb.s, libm, bos411, 9428A410j 2/21/94 14:03:03
#
# COMPONENT_NAME: (LIBM) math library
#
# FUNCTIONS: logb
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
# NAME: logb
#                                                                    
# FUNCTION: Returns a double that is equal to the unbiased exponent of x
#                                                                    
# EXECUTION ENVIRONMENT:
#                                                                   
#       Logb expects its argument to be in BOTH an FPR and a pair of
#       GPR's. Therefore NO C template should be provided for this
#       function! This is done because it is more efficient for the
#       compiler to move the argument to a pair of GPR's (or to the
#       FPR) before the call than to do it inside the routine. The
#       reason it's more efficient is that the compiler may be able
#       to code motion the load(s) upstream from the call.
#
# NOTES:
#
#       IEEE Recommended Logb Function
#
#       y = logb(x)
#
#       This function extracts the exponent of x and converts it
#       to a floating point result.
#
#       This function conforms to the IEEE P754 standard and is
#       a recommended function.
#
#       The algorithm:
#
#           Save the caller's FPSCR
#           Extract the exponent from GPR copy of x
#           Force rounding mode to RN or RZ (clear bit 30 of FPSCR)
#
#           if ((exp != 0) && (exp != 0x7FF))  /* Normalized numbers */
#
#               Put exponent in low part of double in memory
#               Put exponent of 2^52 in high part of double in mem
#               Move the double in memory to an FPR
#               Subtract 2^52 + 1023 from the FPR
#
#           else /* zero, denorm, infinity or NaN */
#
#               if x == + infinity then result = +infinity
#               if x == - infinity then result = +infinity
#               if x == QNaN then result = QNaN
#               if x == SNaN then result = QNaN and set VXSNAN
#               if x == + or - 0 then result = -infinity and set ZX
#               if x == Denormalized: Scale x by 2^64 then extract its
#                       new biased exponent and convert it to double.
#                       Then remove the bias and the scale to get the
#                       result.
#           restore caller's rounding mode
#           end
#
#       The basic idea of the algorithm for normalized numbers is to
#       first extract the exponent and then convert the exponent from
#       an integer to double using the same basic idea we use for all
#       integer to double conversions. One difference is that we can
#       also remove the exponent bias at the same time we subtract
#       the magic number (hence, sub 2^52 +1023 rather than plain 2^52)
#
#       The rounding mode needs to be saved and then set to anything
#       but RM mode, because RM mode would produce a -0.0 result if
#       x's unbiased exponent is 0. The user's round mode is then
#       restored on exit
#
#       Performance estimate = 15 clocks for normalized numbers
#
#
#       IEEE Status Bits that might be set are:
#
#          nvsnan       Signaling NaN. The double number was an SNaN.
#          zx           Divide by Zero. Set if x == 0.
#
# RETURNS: A double that is equal to the unbiased exponent of x
#
# ***********************************************************************
	.file "logb.s"
#
#
#       Define a TOC entry that points to our constant data
#
# errno.h
	.set EDOM,33

	.toc
	TOCL(logb_const,data)
	TOCE(errno,data)               # external error variable
z0x0000.S:  .tc  _0x00000000[tc],0     # put single 0 in toc

#
#
#       Code
#
#

	.align  3
	S_PROLOG(logb)

	lfs     fr0,z0x0000.S(r2)      # in case fr0 is not already 0.0

# **************************************************************************

	LTOC(r6,logb_const,data)  # Load pointer to constants
	mffs    fr4             # Save Caller's Rounding Mode
	liu     r5,0x4330       # Create the upper part of double
	lfd     fr2,0(r6)       # Load the magic constant for logb
	rlinm.  r7,r3,12,0x7ff  # Right justify exponent of x and set cr0
	st      r5,-8(r1)       # Store upper part of double (2^52)
	st      r7,-4(r1)       # Store the exponent in lower part of dbl
	mtfsb0  30              # Make sure rounding != RM
	cmpli   cr1,r7,0x7ff    # Check for infinity or NaN
	beq     cr0,logb_zero_den  # Branch if zero or denorm nbr.
	beq     cr1,logb_inf_nan   # Branch if infinity or NaN
	lfd     fr3,-8(r1)      # Load biased exponent + 2^52
	fs      fr1,fr3,fr2     # Subtract 2^52 + exp. bias (1023)
	mtfsf   0xff,fr4        # Restore caller's rounding mode
	br                      # Return

#
#       Here when x is zero or denormalized
#
#
logb_zero_den:
	mtfsf   0xff,fr4        # Restore caller's rounding mode
	rlinm.  r7,r3,0,0x000fffff    # Are upper fraction bits zero?
	cmpli   cr1,r4,0        # Are lower fraction bits zero?
	lfd     fr3,8(r6)       # Load 2^64 for use below
	fnabs   fr2,fr0         # Load -0.0 for use below
	bne     cr0,logb_den    # Denorm if any fraction bits set
	bne     cr1,logb_den
#
#       Here when x is + or - zero
#
logb_zero:
	mtfsf   0xff,fr4        # Restore caller's rounding mode
	fd      fr1,fr3,fr2     # Do a divide by minus zero (+2^64/-0.0) to
				# make result = -infinity and to set DX
	LTOC(r7,errno,data)     # Get pointer to errno
	cal     r5,EDOM(0)	# domain error code to r5
	st      r5,0(r7)        # errno = EDOM
	br                      # in the FPSCR
#
#       Here when x is a denorm
#            Scale x by 2^64 then extract its biased, scaled exponent.
#            Then unscale this exponent by 64 while converting it
#            to double.
#
logb_den:
	fm      fr1,fr1,fr3     # Scale up denorm by 2^64 (now it's norm)
	stfd    fr1,-16(r1)     # Put the scaled x on the stack
	lfd     fr2,16(r6)      # Load 2^52 + 1023 + 64 for later
	l       r7,-16(r1)      # Get high part of scaled x
	rlinm   r7,r7,12,0x7ff  # Extract the exponent
	st      r7,-4(r1)       # Put exponent into low part of 2^52
	lfd     fr3,-8(r1)      # Load 2^52 + biased and scaled exponent
	fs      fr1,fr3,fr2     # Remove 2^52 + bias + scale
	br


#
#       Here when x is infinity or NaN
#
logb_inf_nan:
	fcmpu   cr0,fr1,fr1     # if (fr1 != fr1) then must be a NaN
	bne     logb_nan
#
#       Here when x is infinity
#
logb_inf:
	fabs    fr1,fr1         # Make the result +infinity
	br
#
#       Here when x is a QNaN or SNaN
#
logb_nan:
	fa      fr1,fr1,fr1     # Will generate a QNaN result and set
	S_EPILOG                # VXSNAN in FPSCR if fr0 is SNaN

#
# ************************************************************************
#
#       Constant Area
#
# ************************************************************************
#

	   .csect  DATA(logb_const[RO]),3
logb_const:
	   .long      0x43300000,0x000003ff   # 2^52 + 1023    offset = 0
	   .long      0x43f00000,0x00000000   # 2^64           offset = 8
	   .long      0x43300000,0x0000043f   # 2^52 + 1087    offset = 16


#
# ************************************************************************
#
#       Function descriptor
#
# ************************************************************************
#
	FCNDES(logb)
