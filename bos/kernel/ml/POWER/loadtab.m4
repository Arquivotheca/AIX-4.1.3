# @(#)68        1.3  src/bos/kernel/ml/POWER/loadtab.m4, sysml, bos411, 9428A410j 3/12/93 18:34:34
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
# (C) COPYRIGHT International Business Machines Corp. 1989, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

#****************************************************************************
#
#	Table used by the Alignment Flih.  Indexed by Target Register (RT)
#	multiplied by 8.  Moves data from GPR 31 to desired register. Note
#       for RT > 24, the data is moved to the save area since the alignment
#	flih uses registers 25-31.
#
#	r31 - contains the data
#	r25 - contains the address of the save area
#
#****************************************************************************

         or    r0,r31,r31
         br
         or    r1,r31,r31
         br
         or    r2,r31,r31
         br
         or    r3,r31,r31
         br
         or    r4,r31,r31
         br
         or    r5,r31,r31
         br
         or    r6,r31,r31
         br
         or    r7,r31,r31
         br
         or    r8,r31,r31
         br
         or    r9,r31,r31
         br
         or    r10,r31,r31
         br
         or    r11,r31,r31
         br
         or    r12,r31,r31
         br
         or    r13,r31,r31
         br
         or    r14,r31,r31
         br
         or    r15,r31,r31
         br
         or    r16,r31,r31
         br
         or    r17,r31,r31
         br
         or    r18,r31,r31
         br
         or    r19,r31,r31
         br
         or    r20,r31,r31
         br
         or    r21,r31,r31
         br
         or    r22,r31,r31
         br
         or    r23,r31,r31
         br
         or    r24,r31,r31
         br
#
#        for RT >= 25 update the register copies in the save area
#
         st    r31,SAVE_R25(r25)
         br
         st    r31,SAVE_R26(r25)
         br
         st    r31,SAVE_R27(r25)
         br
         st    r31,SAVE_R28(r25)
         br
         st    r31,SAVE_R29(r25)
         br
         st    r31,SAVE_R30(r25)
         br
         st    r31,SAVE_R31(r25)
         br
