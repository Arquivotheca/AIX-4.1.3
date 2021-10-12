# @(#)42        1.1  src/bos/kernel/ml/POWER/optab2.m4, sysml, bos411, 9428A410j 3/29/94 22:24:21
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
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************


# Table used for stfiwx instruction.  Indexed by bits 22-26 of the DSISR
# (the fp register number) multiplied by 8.
# r25 = address of save area

         stfd  f0,SAVE_WORK0(r25)                 # store double (fpr = 0,1,..31)
         b     f_stfiwx_ret
         stfd  f1,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f2,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f3,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f4,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f5,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f6,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f7,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f8,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f9,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f10,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f11,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f12,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f13,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f14,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f15,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f16,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f17,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f18,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f19,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f20,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f21,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f22,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f23,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f24,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f25,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f26,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f27,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f28,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f29,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f30,SAVE_WORK0(r25)
         b     f_stfiwx_ret
         stfd  f31,SAVE_WORK0(r25)
         b     f_stfiwx_ret
