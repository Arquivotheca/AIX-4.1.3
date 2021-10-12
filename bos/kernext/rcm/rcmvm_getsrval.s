#
# COMPONENT_NAME: (rcm) Rendering Context Manager
#
# FUNCTIONS:	rcmvm_getsrval
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
		.globl	.rcmvm_getsrval

.rcmvm_getsrval:mfsri   3, 0, 3
		br
