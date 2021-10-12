# @(#)83	1.17  src/bos/usr/ccs/lib/libm/POWER/ilogb.s, libm, bos411, 9428A410j 2/21/94 14:02:44
#
# COMPONENT_NAME: (LIBM) math library
#
# FUNCTIONS: ilogb
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
# NAME: ilogb
#                                                                    
# FUNCTION: Return an integer that is equal to the unbiased exponent of x
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
#       int ilogb(double x)
#
#       This function extracts the exponent of x and converts it
#       to an integer. It is equivalent to calling the ieee
#       recommended logb function and then converting the result to
#       an integer. However, it is faster than doing the two functions
#       separately.
#
#
#       The algorithm:
#
#           Extract the biased exponent from GPR copy of x
#
#           if ((exp != 0) && (exp != 0x7FF))  /* Normalized numbers */
#
#               return(exp-1023)
#
#           else /* zero, denorm, infinity or NaN */
#
#               if x == + infinity, result = 0x7fffffff and set VXCVI
#               if x == - infinity, result = 0x7fffffff and set VXCVI
#               if x == QNaN, result = 0x80000000 and set VXVCI
#               if x == SNaN, result = 0x80000000 and set VXSNAN & VXCVI
#               if x == + or - 0, result = 0x80000000 and set ZX & VXCVI
#               if x == Denormalized: Calculate the true exponent of x
#                       by normalizing the fraction of x and adjusting
#                       the exponent downward.
#           end
#
#
#       Performance estimate = 5 clocks for normalized numbers
#
#
#       IEEE Status Bits that might be set are:
#
#          nvcvi        Unable to correctly convert logb to an integer.
#                       Set when x is infinity, NaN, or 0.
#          vx           Invalid summary bit
#          nvsnan       Signaling NaN. The double number was an SNaN.
#          zx           Divide by Zero. Set if x == 0.
#
# RETURNS: Integer that is equal to the unbiased exponent of x
#
#
# ***********************************************************************
	.file "ilogb.s"
#
#
#       Define a TOC entry that points to our constant data
#
	.toc
	TOCL(ilogb_const,data)
z0x0000.S:  .tc  _0x00000000[tc],0     # put single 0 in toc

#
#       Code
#

	.align  3

	S_PROLOG(ilogb)

	lfs     fr0,z0x0000.S(r2)      # in case fr0 is not already 0.0

# **************************************************************************

	rlinm.  r7,r3,12,0x7ff  # Right justify exponent of x and set cr0
	cmpli   cr1,r7,0x7ff        # Check for infinity or NaN
	beq     cr0,ilogb_zero_den  # Branch if zero or denorm nbr.
	ai      r3,r7,-1023         # Subtract the bias
	bner    cr1                 # Return if not infinity or NaN
	b       ilogb_inf_nan       # Branch if infinity or NaN

#
#       Here when x is zero or denormalized
#
#
ilogb_zero_den:
	LTOC(r6,ilogb_const,data)  # Get pointer to constants
	rlinm.  r3,r3,0,0x000fffff    # Are upper fraction bits zero?
	cmpli   cr1,r4,0         # Are lower fraction bits zero?
	bne     cr0,ilogb_den1   # Denorm if bit set in MS word
	bne     cr1,ilogb_den2   # Denorm if bit set in LS word
#
#       Here when x is + or - zero
#
ilogb_zero:
	l       r4,4(r6)        # Return min (0x80000000)
	l       r3,12(r6)       # Set VXCVI & ZX (divide by zero)
	b       il_except       # Go set exceptions and return result
#
#       Here when x is a denorm & the ms bit is in the upper word
#
ilogb_den1:
	cntlz   r5,r3           # count leading zeros including sign & exp
	lil     r3,-1011        # bias (-1023) + bits for sign & exp (12)
	sf      r3,r5,r3        # -count of leading zeroes.
	br                      # return

#
#       Here when x is a denorm & the ms bit is in the lower word
#
ilogb_den2:
	cntlz   r5,r4           # count leading zeros including sign & exp
	lil     r3,-1043        # bias (-1023) - 20 bits in the upper word
	sf      r3,r5,r3        # -count of leading zeroes.
	br                      # return


#
#       Here when x is infinity or NaN
#
ilogb_inf_nan:
	fcmpu   cr0,fr1,fr1     # if (fr0 != fr0) then NaN (may set VXSNAN)
	LTOC(r6,ilogb_const,data)  # Get pointer to constants
	l       r3,8(r6)        # load VXCVI bit
	bne     ilogb_nan       #
#
#       Here when x is infinity
#
ilogb_inf:
	l       r4,0(r6)        # Return max (0x7fffffff)
	b       il_except       # Go set VXCVI and return result
#
#       Here when x is a QNaN or SNaN
#
ilogb_nan:
	l       r4,4(r6)        # Return min (0x80000000)
	b       il_except       # Go set VXCVI and return result
	br


# *************************************************************************
#
#
#       Here to set the appropriate exception bits in the IEEE status
#
#               r3 contains the exception mask flag
#               r4 contains the result to be returned
#
#

	.extern  ENTRY(fp_set_flag)  # Routine to set exception bits

il_except:
	mflr    r0              # Get the current link register
	st      r4,-4(r1)       # Save result on the stack
	st      r0,8(r1)        # Save the link register for return
	stu     r1,-60(r1)      # Create stack frame and backtrace
#
#       Here to do the call to set the status bits in the IEEE Status
#
	bl      ENTRY(fp_set_flag)
	l	r2, stktoc(r1)	# restore toc pointer
#
#       Restore the result and do the epilogue code
#
	l       r0,68(r1)       # Get saved link register
	ai      r1,r1,60        # Remove the stack frame
	mtlr    r0              # Restore the link register
	l       r3,-4(r1)       # Restore the result
#
	br
	_DF(0x0,0x1,0x8,0x0,0x0,0x0,0x0)  # Traceback Table info.


#
# ************************************************************************
#
#       Constant Area
#
# ************************************************************************
#

	   .csect  DATA(ilogb_const[RO]),3
ilogb_const:
	.long   0x7fffffff        # Max integer,           offset = 0
	.long   0x80000000        # Min integer,           offset = 4
	.long   fpxinvalid+fpxnvcvi          # Inv. Conversion,       offset = 8
	.long   fpxinvalid+fpxnvcvi+fpxzdiv  # Inv. Conversion & DZ,  offset = 12

#
# ***********************************************************************
#
#       Function Descriptor
#
# ***********************************************************************
#
	FCNDES(ilogb)

include(sys/comlink.m4)
