# @(#)18	1.18  src/bos/usr/ccs/lib/libm/POWER/sqrt.s, libm, bos41J, 9508B 2/14/95 07:26:45
#
# COMPONENT_NAME: (LIBM) math library
#
# FUNCTIONS: sqrt
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# NAME: sqrt, _sqrt
#                                                                    
# FUNCTION: COMPUTES THE SQUARE ROOT OF X
#                                                                    
# EXECUTION ENVIRONMENT:
#                                                                   
#                                                                   
# NOTES:
#
#  Uses fixed point ops to generate an initial guess with the help
#  of a table.  Then computes successively better guesses to the
#  square root and the reciprocal, using Newton-Raphson.  The last
#  iteration is so constructed that the last floating point
#  operation correctly sets all the floating point status bits
#  to characterize the results.
#
#  CAUTION:  This routine is dependent on the IEEE representation
#  as well as the fact that the hardware accomplishes
#  multiply-and-add with only one rounding operation.
#
#  For further description of the algorithm used, see "Computation
#  of elementary functions on the IBM RICS System/6000 processor",
#  P. W. Markstein, IBM J. Res. Develop., Vol. 34, No. 1,
#  Jan. 1990, pp.115-116.
#
# RETURNS: The square root of x
#
# If x is negative, then a QNaN is returned and errno is set to EDOM.
#
# ***********************************************************************

ifdef(`_FORTRAN_VERSION',`
	.file "sqrtF.s"
',`
	.file "sqrt.s"
')
	include(fpxcp.m4)
	.extern		ENTRY(fp_raise_xcp)

# errno.h
	.set	EDOM,33

# The stack of the calling routine (i.e. a negative offset from
# r1) is used to store the argument and to construction the first
# guess (G) and 0.5/G (Y).
	
	.set	ARG, -8			# location of argument on stack
	.set	Y, -16			# location of 'Y' (0.5/guess) on stack
	.set	G, -24			# location of 'G' (guess) on stack
	.set	STK_SZ, 64		# size of stack to buy in error case
		
	.toc
	TOCL(SQRT_CONST,data)		# local constants
	TOCE(guesses,data)		# external data table
	TOCE(errno,data)		# external error variable
	
ifdef(`_FORTRAN_VERSION',`
	S_PROLOG(_sqrt)
',`
	S_PROLOG(sqrt)
')

	stfd	fr1, ARG(r1)		# store arg onto stack
	LTOC(r4, SQRT_CONST, data)	# load toc pointer
        mffs	fr7                     # save rounding mode
	LTOC(r5, guesses, data)		# pointer to guesses table
        lfd	fr0, xzero(r4)		# load constant 0.0
        mtfsf	255, fr0		# set round mode nearest
        crandc	28, 28, 28		# set CR bit 28 (cr7 lt) to 0

rejoin:
	
# Calculate an index into the guess table.  Each entry in the
# table contains an initial guess for the square root and a
# a guess for the reciprocal.  The table has 256 entries, 16
# bits per entry.  The high 8 bits contain 8 bits of significand
# for the guess; the low 8 bits contain 8 bits of significand
# for 0.5/guess.
#
# The index for the table is 8 bits; it consists of the
# low order bit of the exponent and the seven highest bits
# of the significand (of the argument).  The low bit of the
# exponent is part of the table because this bit is lost
# in computing the exponent of the guess.
#
# The exponent of the guess is computed by isolating the
# high 32 bits of the argument; adding 0x3ff00000 to normalize
# the exponent, shifting right by 1 to divide by 2, and
# then masking with 0x7ff00000 to isolate the exponent.
# This result is subtracted from 0x7fc00000 to obtain the 
# exponent of .5/guess.
#
# Denormal numbers cannot use the exponent arithmetic described
# above, so they must be scaled.  All numbers less than
# 0x0700000000000000 are scaled -- this number is somewhat
# arbitrary; it is simply in the range between numbers that will
# overflow if scaled and numbers which will fail if not
# not scaled.
#	
# NOTE -- only the top 32 bits of G and Y matter.  Using integer
# instructions we only set the top 32 bits.  When we ready G and
# Y as doubles we pick up whatever happens to be in memory for the
# bottom 32 bits.  There is no need to zero these bits, and
# to do so whould only waste time.
		
	l	r8, ARG(r1)		# load high part of arg from stack
	cau	r6, r0, 0x7fc0		# create constant 0x7fc00000
	rlinm	r0, r8, 20, 0x1fe	# mask & shift arg to form index
	lhzx	r7, r5, r0		# load table entry
	cau	r0, r8, 0x3ff0		# create constant 0x3ff00000
	rlinm	r3, r0, 31, 0x7ff00000	# form exponent of guess
	sf	r0, r3, r6		# form exponent of y
	rlimi	r3, r7, 4, 0xff000	# form guess	
	st	r3, G(r1)		# store guess
	rlimi	r0, r7, 12, 0xff000	# form y
	st	r0, Y(r1)		# store y
	lfd	fr2, G(r1)		# load guess as fp
	fnms	fr3, fr2, fr2, fr1	# d=x-g*g
	lfd	fr5, Y(r1)		# load y as fp
	fma	fr2, fr5, fr3, fr2	# g=g+y*d
	lfd	fr0, almost_half(r4)	# load constant almost_half
	fa	fr6, fr5, fr5		# y2=y+y
	cau	r0, r0, 0x7ff0		# create constant 0x7ff00000
	fnms	fr4, fr5, fr2, fr0	# e=almost_half-y*g
	cmp	cr1, r8, r0		# compare arg with inf/nan
	fnms	fr3, fr2, fr2, fr1	# d=x-g*g
	cau	r0, r0, 0x0700		# create constant 0x07000000
	fma	fr5, fr6, fr4, fr5	# y=y+e*y2
	cmp	cr0, r8, r0		# compare arg with small number
	fma	fr2, fr3, fr5, fr2	# g=g+y*d
	bnl-	cr1, retx		# bail out if nan or inf
	fa	fr6, fr5, fr5		# y2=y+y
	blt-	cr0, smallx		# branch if a small argument
	fnms	fr4, fr5, fr2, fr0	# e=almost_half-y*g
	fnms	fr3, fr2, fr2, fr1	# d=x-g*g
	fma	fr5, fr6, fr4, fr5	# y=y+e*y2
	fma	fr2, fr3, fr5, fr2	# g=g+y*d
	fa	fr6, fr5, fr5		# y2=y+y
	fnms	fr4, fr5, fr2, fr0	# e=almost_half-y*g
	fnms	fr3, fr2, fr2, fr1	# d=x-g*g
	fma	fr5, fr6, fr4, fr5	# y=y+e*y2

# At this point the approximation was good to 64 bits before rounding,
# but may have been rounded incorrectly.  We restore the caller's
# FPSCR, and perform one more interation under the caller's
# rounding mode.  By Markstein's theorem this gets the rounding
# correct in all cases and correctly sets the inexact status
# bit if appropriate.	
	
	mtfsf	255, r7			# restore rounding mode
	fma	fr1, fr3,fr5,fr2	# g=g+y*d
	bnlr+	cr7			# return if not scaled

# The multiply by down-factor should get the same results in
# all rounding modes without a spurious inexact bit, because
# it's simply a power of 2.
	
	lfd	fr2,x2ff(r4)		# load down factor
	fm	fr1,fr1,fr2		# multiply by down factor
	br				# return

# For small numbers we multiply by a scale factor to normalize
# the number, calculate the square root of the scaled number
# and then re-scale that result.
		
smallx:
	lfd	fr2, xzero(r4)		# xzero
	fcmpu	cr1, fr1, fr2		# x ? 0
	beq-	cr1, retx		# if arg == 0 
	bng-	cr1, xnp		# if not positive
	lfd	fr2, x5ff(r4)		# x5ff (scale factor)
	fm	fr1, fr1, fr2		# x1 = arg * scale factor
	stfd	fr1, ARG(r1)		# scaled arg onto stack
	crorc	28, 28, 28		# set bit 28 to 1
	b+	rejoin			# back to main path

# Arrive here for 0.0, NaN and +INF.  The argument is
# multiplied by 1.0 to force signalling NaNs quiet.
# This is done under the user's FPSCR so an invalid
# trap for the SNaN will be taken if appropriate.

retx:
	lfd	fr2, one(r4)		# load constant 1.0
	mtfsf	255, fr7		# restore rounding mode
	fm	fr1, fr1, fr2		# make sure nan is made quiet
	br				# 0,inf,nan return x * 1.0

# Arrive here for negative numbers other than -0.0.
# We call fp_raise_xcp(FP_INV_SQRT) to report the error,
# and return NaNQ.

xnp:
	fcmpu	cr0, fr1, fr1		# test for NaN
	bne-	cr0, retx		# if -NaN, go make quiet and leave
	mtfsf	255, fr7		# restore rounding mode
	LTOC(r3,errno,data)		# Get pointer to errno
	cal	r0, EDOM(r0)		# domain error (33)
	st	r0, 0(r3)		# set errno
	mflr	r3			# get link reister
	st	r3, stklink(r1)		# save link register
	stu	r1, -STK_SZ(r1)		# get stack frame
	cal	r3, FP_INV_SQRT(0)	# load argument for fp_raise
	bl	ENTRY(fp_raise_xcp)	# branch to fp_raise_xcp
	l	r2, stktoc(r1)		# reload toc
	l	r3, stklink+STK_SZ(r1)	# get saved link register
	mtlr	r3			# restore link register
	LTOC(r4, SQRT_CONST, data)	# load toc pointer
	ai	r1, r1, STK_SZ		# pop stack
	lfd	fr1, xnan(r4)		# not (x >= 0) return nan
	S_EPILOG			# return

# read-only constant area

	.csect DATA(SQRT_CONST[RO]),3
SQRT_CONST:
almost_half:	.long	0x3fe00000, 0x00000001	# 0.50000000000000010
x2ff:		.long	0x2ff00000, 0x0		# 8.6361685550944446e-78
x5ff:		.long	0x5ff00000, 0x0		# 1.3407807929942597e+154
xzero:		.long	0x0,        0x0		# 0.0
xnan:		.long	0x7ff80000, 0x0		# NaNQ
one:		.long	0x3ff00000, 0x0		# 1.0

ifdef(`_FORTRAN_VERSION',`
	FCNDES(_sqrt)			# function descriptor
',`
	FCNDES(sqrt)			# function descriptor
')
include(sys/comlink.m4)
