# @(#)33	1.2  src/bos/usr/ccs/lib/libm/POWER/fpxcp.m4, libm, bos411, 9428A410j 1/30/91 08:20:01
#
# COMPONENT_NAME: (LIBM) math library
#
# FUNCTIONS: none.  This is a header file
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#      fp_xcp.h
#
#      Header file to define the floating point exception status
#      bits.
#
#      RIOS specific version
#
#      Note: this header file will be different for different
#            platforms.
#

#  ******* REQUIRED SUMMARY BITS *********** */

	.set     FP_INVALID  ,    0x20000000
	.set     FP_OVERFLOW ,    0x10000000
	.set     FP_UNDERFLOW,    0x08000000
	.set     FP_DIV_BY_ZERO,  0x04000000
	.set     FP_INEXACT    ,  0x02000000

# ******** OPTIONAL INVALID EXCEPTION DETAIL BITS ***********/

#    FP_INV_SNAN       Signalling NaN
#    FP_INV_ISI        Infinity - Infinity
#    FP_INV_IDI        Infinity / Infinity
#    FP_INV_ZDZ        0/0
#    FP_INV_IMZ        Infinity * 0
#    FP_INV_CMP        Unordered Compare
#    FP_INV_REM_Y0     Remainder(x,y) with y=0
#    FP_INV_REM_XI     Remainder(x,y) with x=infinity
#    FP_INV_SQRT       Square Root of negative number
#    FP_INV_CVI        Conversion to integer error


	.set     FP_INV_SNAN,     0x01000000
	.set     FP_INV_ISI ,     0x00800000
	.set     FP_INV_IDI ,     0x00400000
	.set     FP_INV_ZDZ ,     0x00200000
	.set     FP_INV_IMZ ,     0x00100000
	.set     FP_INV_CMP ,     0x00080000
	.set     FP_INV_REM_Y0,   0x00000800
	.set     FP_INV_REM_XI,   0x00000400
	.set     FP_INV_SQRT  ,   0x00000200
	.set     FP_INV_CVI   ,   0x00000100
