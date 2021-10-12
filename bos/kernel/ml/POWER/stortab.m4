# @(#)70        1.3  src/bos/kernel/ml/POWER/stortab.m4, sysml, bos411, 9428A410j 3/12/93 18:35:45
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
# (C) COPYRIGHT International Business Machines Corp. 1989, 1992
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

#############################################################################
#
#   Table used by the Alignment Flih.  Indexed by Source Register (RS). 
#   Moves original value of RS to r31. Note that for RS > 24 we have to get
#   the data from the save area since the alignment handler uses 25-31.
#
#	r31 = data
#	r25 = address of save area
#############################################################################
         or    r31,r0,r0
         br
         or    r31,r1,r1
         br
         or    r31,r2,r2
         br
         or    r31,r3,r3
         br
         or    r31,r4,r4
         br
         or    r31,r5,r5
         br
         or    r31,r6,r6
         br
         or    r31,r7,r7
         br
         or    r31,r8,r8
         br
         or    r31,r9,r9
         br
         or    r31,r10,r10
         br
         or    r31,r11,r11
         br
         or    r31,r12,r12
         br
         or    r31,r13,r13
         br
         or    r31,r14,r14
         br
         or    r31,r15,r15
         br
         or    r31,r16,r16
         br
         or    r31,r17,r17
         br
         or    r31,r18,r18
         br
         or    r31,r19,r19
         br
         or    r31,r20,r20
         br
         or    r31,r21,r21
         br
         or    r31,r22,r22
         br
         or    r31,r23,r23
         br
         or    r31,r24,r24
         br
#
#        for RS >= 25 move save area copy of RS to r31
#
         l     r31,SAVE_R25(r25)
         br
         l     r31,SAVE_R26(r25)
         br
         l     r31,SAVE_R27(r25)
         br
         l     r31,SAVE_R28(r25)
         br
         l     r31,SAVE_R29(r25)
         br
         l     r31,SAVE_R30(r25)
         br
         l     r31,SAVE_R31(r25)
         br

