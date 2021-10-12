# @(#)97	1.4  src/bos/usr/ccs/lib/libc/POWER/mini64.s, libccnv, bos411, 9428A410j 2/21/94 14:00:23
#
#   COMPONENT_NAME: LIBCCNV
#
#   FUNCTIONS: __mini64
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
# long long	__mini64( long long, long long )
#
# (r3:r4) = Larger Integer Of (r3:r4) | (r5:r6)
#
# Recall that in a (signed long long int), the MSW is Signed, the LSW unsigned.
#

	.file "mini64.s"
	.machine "com"
	S_PROLOG(__mini64)

	cmp	cr6,  r5,  r3			# Signed compare, MSW
	cmpl	cr7,  r6,  r4			# Unsigned compare, LSW
	bgtr	cr6				# Return if MSW r3 <  MSW r5
	bne	cr6, mn1			#
	bger	cr7				# Return if LSW r4 <= LSW r6
mn1:						#
	oril	 r3,  r5,   0			#
	oril	 r4,  r6,   0			# Return p2

	S_EPILOG(__mini64)
