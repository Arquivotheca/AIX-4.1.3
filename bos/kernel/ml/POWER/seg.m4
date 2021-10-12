# @(#)39	1.6  src/bos/kernel/ml/POWER/seg.m4, sysml, bos411, 9428A410j 6/15/90 18:14:07
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Defines
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1990
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

#
#	Segment Registers
#
        .set r_segr0,0		# segment register 0
	.set r_kernel,0		# kernel segment
        .set r_segr1,1		# segment register 1
        .set r_segr2,2		# segment register 2
        .set r_user,2		# user data segment (aka seg reg 2)
        .set r_segr3,3		# segment register 3
        .set r_segr4,4		# segment register 4
        .set r_segr5,5		# segment register 5
        .set r_segr6,6		# segment register 6
        .set r_segr7,7		# segment register 7
        .set r_segr8,8		# segment register 8
        .set r_segr9,9		# segment register 9
        .set r_segr10,10	# segment register 10
        .set r_segr11,11	# segment register 11
        .set r_segr12,12	# segment register 12
        .set r_segr13,13	# segment register 13
        .set r_segr14,14	# segment register 14
	.set r_kdata,14		# kernel data segment
        .set r_segr15,15	# segment register 15
#
#  bits in segment register
#
	.set r_iobit,0		# io bit number
	.set r_key,1		# access key bit number
	.set r_sbit,2		# special bit number
	.set srio,0x8000	# io bit
	.set srkey,0x4000	# srkey bit
	.set srspec,0x2000	# special bit
#
#  miscellaneous useful values from <sys/seg.h>
#
   	.set NULLSEGID,0x7FFFFF   # generic invalid sid
	.set u.NULLSEGID, 0x007F
	.set l.NULLSEGID, 0xFFFF

	.set NULLSEGVAL,0x7FFFFF  # null segreg value
	.set u.NULLSEGVAL, 0x007F
	.set l.NULLSEGVAL, 0xFFFF

	.set KERNELSEGID,0        # kernel segment id
        .set KERNELSEGVAL,0       # kernel segreg value




