# @(#)96	1.4  src/bos/usr/ccs/lib/libc/POWER/maxi64.s, libccnv, bos411, 9428A410j 2/21/94 14:00:15
#
#   COMPONENT_NAME: LIBCCNV
#
#   FUNCTIONS: __maxi64
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#******************************************************************************
#
# long long	__maxi64( long long, long long )
#
# (r3:r4) = Larger Integer Of (r3:r4) | (r5:r6)
#
# Recall that in a (signed long long int), the MSW is Signed, the LSW unsigned.
#

	.file "maxi64.s"
	.machine "com"
	S_PROLOG(__maxi64)

	cmp	cr6,  r5,  r3			# Signed compare, MSW
	cmpl	cr7,  r6,  r4			# Unsigned compare, LSW
	bltr	cr6				# Return if MSW r3 >  MSW r5
	bne	cr6, mx1			#
	bler	cr7				# Return if LSW r4 >= LSW r6
mx1:						#
	oril	 r3,  r5,   0			#
	oril	 r4,  r6,   0			# Return p2

	S_EPILOG(__maxi64)
