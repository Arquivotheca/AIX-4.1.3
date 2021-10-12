# @(#)82        1.10  src/bos/kernel/ml/interrupt.m4, sysml, bos41J, 9511A_all 3/7/95 13:12:24
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS:
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************
#
#  FUNCTION: IOS interrupt data structures
#
#       Any changes to this file should also be reflected in:
#               ( ios/interrupt.h )
#
#       Refer to the above files for a more detailed description
#       of the layout & function of these data structures / variables.
#
#********************************************************************

#
# The following definition uses labels defined in intr.m4.
# A file that references this file (interrupt.m4) needs to reference
# intr.m4 also (can't just use it here due to assembler
# not allowing multiple inclusions of same file).
#


#----------------------------------------------------------------------
#
#       This maps i_data - see com/sys/ios/interrupt.h
#
# struct i_data
# {
#	union {
#             struct imask eim[NUM_INTR_PRIORITY];
#             char    cil[NUM_INTR_PRIORITY];
#	} i_pri_map;
#
#       struct i_poll {
#             struct intr     *poll;          /* next interrupt handler */
#       } i_poll[NUM_INTR_SOURCE];
# };
#----------------------------------------------------------------------
ifdef(`_POWER_MP',`
	.set	LOCAL_CPU,	-2
')
        .set    I_DATA_SH, 2   		# log2(sizeof struct i_poll)

        .dsect  i_data_dsect            # interrupt handler data
i_deim:
        .space  NUM_INTR_PRIORITY*4*2   # eim per priority

i_dpoll:
        .space  NUM_INTR_SOURCE*(1<I_DATA_SH)	# poll array
        .set    i_nextint,0             	# - next interrupt handler

#
# These must stay in sync with ios/interrupt.h
#
	.set	MAX_LVL_PER_PRI,	32	#
#
# This algorithm will not work above a size of 64 unless it is a power of 2
#
	.set	PRILVL_LEN,		(MAX_LVL_PER_PRI / 32) + 1
