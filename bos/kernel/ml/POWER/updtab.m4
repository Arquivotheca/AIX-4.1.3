# @(#)71        1.3  src/bos/kernel/ml/POWER/updtab.m4, sysml, bos411, 9428A410j 3/12/93 18:35:59
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

#############################################################################
#
#  Table used by the Alignment Flih.  Indexed by the Address Register (RA).
#  Moves the Effective Address of an instruction (contents of DAR) to the
#  desired update register.  Note that for RA > 24 we must perform the 
#  update to the appropriate save area copy, since the alignment flih uses
#  GPRs 25-31.
#
#	r27 = effective address to place in updated register
#	r25 = address of save area
#############################################################################
         b     al_ret		# notice for r0, no update is performed
         cror  0,0,0		# this nop keeps table at 8bytes per entry
         or    r1,r27,r27
         b     al_ret
         or    r2,r27,r27
         b     al_ret
         or    r3,r27,r27
         b     al_ret
         or    r4,r27,r27
         b     al_ret
         or    r5,r27,r27
         b     al_ret
         or    r6,r27,r27
         b     al_ret
         or    r7,r27,r27
         b     al_ret
         or    r8,r27,r27
         b     al_ret
         or    r9,r27,r27
         b     al_ret
         or    r10,r27,r27
         b     al_ret
         or    r11,r27,r27
         b     al_ret
         or    r12,r27,r27
         b     al_ret
         or    r13,r27,r27
         b     al_ret
         or    r14,r27,r27
         b     al_ret
         or    r15,r27,r27
         b     al_ret
         or    r16,r27,r27
         b     al_ret
         or    r17,r27,r27
         b     al_ret
         or    r18,r27,r27
         b     al_ret
         or    r19,r27,r27
         b     al_ret
         or    r20,r27,r27
         b     al_ret
         or    r21,r27,r27
         b     al_ret
         or    r22,r27,r27
         b     al_ret
         or    r23,r27,r27
         b     al_ret
         or    r24,r27,r27
         b     al_ret
#
#        for RA >= 25 update register copies in save area
#
         st    r27,SAVE_R25(r25)
         b     al_ret
         st    r27,SAVE_R26(r25)
         b     al_ret
         st    r27,SAVE_R27(r25)
         b     al_ret
         st    r27,SAVE_R28(r25)
         b     al_ret
         st    r27,SAVE_R29(r25)
         b     al_ret
         st    r27,SAVE_R30(r25)
         b     al_ret
         st    r27,SAVE_R31(r25)
         b     al_ret
