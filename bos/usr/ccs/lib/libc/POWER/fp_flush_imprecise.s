# @(#)34	1.3  src/bos/usr/ccs/lib/libc/POWER/fp_flush_imprecise.s, libccnv, bos411, 9428A410j 2/21/94 13:59:49
#
# COMPONENT_NAME: LIBCCNV
#
# FUNCTIONS: fp_flush_imprecise
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# NAME: fp_flush_imprecise
#                                                                    
# FUNCTION: Force pending imprecise interrupts to be delivered
#                                                                    
# NOTES:
#
# C prototype:
# 	void fp_flush_imprecise(void);
#
# Kills fr1
#
# RETURNS: 
#	Nothing
#
#
	.file "fp_flush_imprecise.s"

	.align  2
	S_PROLOG(fp_flush_imprecise)
	mffs	fr1			# get floating point unit
	ics				# Rios-2 step
	mffs	fr1			# take care of Rios-2 timing window
	S_EPILOG

	FCNDES(fp_flush_imprecise)
