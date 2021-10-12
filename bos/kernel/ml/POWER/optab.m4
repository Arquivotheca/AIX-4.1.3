# @(#)69        1.3  src/bos/kernel/ml/POWER/optab.m4, sysml, bos411, 9428A410j 3/12/93 18:34:51
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
#  Table used by the Alignment Flih.  Indexed by bits 20-26 of the DSISR
#  multiplied by 8.  Note that the exception is for the Store Quad Op which
#  we have to clear bit 20 and set bit 21 to make the index work.  The
#  appropriate floating point register is loaded/stored with the appropriate
#  size floating point op.
#
# 	r25 - address of save area
############################################################################

#
#        load singles
#
         lfs   f0,SAVE_WORK0(r25)
         b     update
         lfs   f1,SAVE_WORK0(r25)
         b     update
         lfs   f2,SAVE_WORK0(r25)
         b     update
         lfs   f3,SAVE_WORK0(r25)
         b     update
         lfs   f4,SAVE_WORK0(r25)
         b     update
         lfs   f5,SAVE_WORK0(r25)
         b     update
         lfs   f6,SAVE_WORK0(r25)
         b     update
         lfs   f7,SAVE_WORK0(r25)
         b     update
         lfs   f8,SAVE_WORK0(r25)
         b     update
         lfs   f9,SAVE_WORK0(r25)
         b     update
         lfs   f10,SAVE_WORK0(r25)
         b     update
         lfs   f11,SAVE_WORK0(r25)
         b     update
         lfs   f12,SAVE_WORK0(r25)
         b     update
         lfs   f13,SAVE_WORK0(r25)
         b     update
         lfs   f14,SAVE_WORK0(r25)
         b     update
         lfs   f15,SAVE_WORK0(r25)
         b     update
         lfs   f16,SAVE_WORK0(r25)
         b     update
         lfs   f17,SAVE_WORK0(r25)
         b     update
         lfs   f18,SAVE_WORK0(r25)
         b     update
         lfs   f19,SAVE_WORK0(r25)
         b     update
         lfs   f20,SAVE_WORK0(r25)
         b     update
         lfs   f21,SAVE_WORK0(r25)
         b     update
         lfs   f22,SAVE_WORK0(r25)
         b     update
         lfs   f23,SAVE_WORK0(r25)
         b     update
         lfs   f24,SAVE_WORK0(r25)
         b     update
         lfs   f25,SAVE_WORK0(r25)
         b     update
         lfs   f26,SAVE_WORK0(r25)
         b     update
         lfs   f27,SAVE_WORK0(r25)
         b     update
         lfs   f28,SAVE_WORK0(r25)
         b     update
         lfs   f29,SAVE_WORK0(r25)
         b     update
         lfs   f30,SAVE_WORK0(r25)
         b     update
         lfs   f31,SAVE_WORK0(r25)
         b     update
#
#        load doubles
#
         lfd   f0,SAVE_WORK0(r25)
         b     update
         lfd   f1,SAVE_WORK0(r25)
         b     update
         lfd   f2,SAVE_WORK0(r25)
         b     update
         lfd   f3,SAVE_WORK0(r25)
         b     update
         lfd   f4,SAVE_WORK0(r25)
         b     update
         lfd   f5,SAVE_WORK0(r25)
         b     update
         lfd   f6,SAVE_WORK0(r25)
         b     update
         lfd   f7,SAVE_WORK0(r25)
         b     update
         lfd   f8,SAVE_WORK0(r25)
         b     update
         lfd   f9,SAVE_WORK0(r25)
         b     update
         lfd   f10,SAVE_WORK0(r25)
         b     update
         lfd   f11,SAVE_WORK0(r25)
         b     update
         lfd   f12,SAVE_WORK0(r25)
         b     update
         lfd   f13,SAVE_WORK0(r25)
         b     update
         lfd   f14,SAVE_WORK0(r25)
         b     update
         lfd   f15,SAVE_WORK0(r25)
         b     update
         lfd   f16,SAVE_WORK0(r25)
         b     update
         lfd   f17,SAVE_WORK0(r25)
         b     update
         lfd   f18,SAVE_WORK0(r25)
         b     update
         lfd   f19,SAVE_WORK0(r25)
         b     update
         lfd   f20,SAVE_WORK0(r25)
         b     update
         lfd   f21,SAVE_WORK0(r25)
         b     update
         lfd   f22,SAVE_WORK0(r25)
         b     update
         lfd   f23,SAVE_WORK0(r25)
         b     update
         lfd   f24,SAVE_WORK0(r25)
         b     update
         lfd   f25,SAVE_WORK0(r25)
         b     update
         lfd   f26,SAVE_WORK0(r25)
         b     update
         lfd   f27,SAVE_WORK0(r25)
         b     update
         lfd   f28,SAVE_WORK0(r25)
         b     update
         lfd   f29,SAVE_WORK0(r25)
         b     update
         lfd   f30,SAVE_WORK0(r25)
         b     update
         lfd   f31,SAVE_WORK0(r25)
         b     update
#
#        store singles
#
         stfs  f0,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f1,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f2,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f3,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f4,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f5,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f6,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f7,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f8,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f9,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f10,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f11,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f12,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f13,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f14,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f15,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f16,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f17,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f18,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f19,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f20,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f21,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f22,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f23,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f24,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f25,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f26,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f27,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f28,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f29,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f30,SAVE_WORK0(r25)
         b     f_s_ret
         stfs  f31,SAVE_WORK0(r25)
         b     f_s_ret
#
#        store doubles
#
         stfd  f0,SAVE_WORK0(r25)                 # store double (fpr = 0,1,..31)
         b     f_d_ret
         stfd  f1,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f2,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f3,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f4,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f5,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f6,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f7,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f8,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f9,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f10,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f11,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f12,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f13,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f14,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f15,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f16,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f17,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f18,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f19,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f20,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f21,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f22,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f23,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f24,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f25,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f26,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f27,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f28,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f29,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f30,SAVE_WORK0(r25)
         b     f_d_ret
         stfd  f31,SAVE_WORK0(r25)
         b     f_d_ret
#
#        load quads
#
	#.long  0xE0190000
         lfq   f0,SAVE_WORK0(r25)
         b     update
	#.long  0xE0390000
         lfq   f1,SAVE_WORK0(r25)
         b     update
	#.long  0xE0590000
         lfq   f2,SAVE_WORK0(r25)
         b     update
	#.long  0xE0790000
         lfq   f3,SAVE_WORK0(r25)
         b     update
	#.long  0xE0990000
         lfq   f4,SAVE_WORK0(r25)
         b     update
	#.long  0xE0B90000
         lfq   f5,SAVE_WORK0(r25)
         b     update
	#.long  0xE0D90000
         lfq   f6,SAVE_WORK0(r25)
         b     update
	#.long  0xE0F90000
         lfq   f7,SAVE_WORK0(r25)
         b     update
	#.long  0xE1190000
         lfq   f8,SAVE_WORK0(r25)
         b     update
	#.long  0xE1390000
         lfq   f9,SAVE_WORK0(r25)
         b     update
	#.long  0xE1590000
         lfq   f10,SAVE_WORK0(r25)
         b     update
	#.long  0xE1790000
         lfq   f11,SAVE_WORK0(r25)
         b     update
	#.long  0xE1990000
         lfq   f12,SAVE_WORK0(r25)
         b     update
	#.long  0xE1B90000
         lfq   f13,SAVE_WORK0(r25)
         b     update
	#.long  0xE1D90000
         lfq   f14,SAVE_WORK0(r25)
         b     update
	#.long  0xE1F90000
         lfq   f15,SAVE_WORK0(r25)
         b     update
	#.long  0xE2190000
         lfq   f16,SAVE_WORK0(r25)
         b     update
	#.long  0xE2390000
         lfq   f17,SAVE_WORK0(r25)
         b     update
	#.long  0xE2590000
         lfq   f18,SAVE_WORK0(r25)
         b     update
	#.long  0xE2790000
         lfq   f19,SAVE_WORK0(r25)
         b     update
	#.long  0xE2990000
         lfq   f20,SAVE_WORK0(r25)
         b     update
	#.long  0xE2B90000
         lfq   f21,SAVE_WORK0(r25)
         b     update
	#.long  0xE2D90000
         lfq   f22,SAVE_WORK0(r25)
         b     update
	#.long  0xE2F90000
         lfq   f23,SAVE_WORK0(r25)
         b     update
	#.long  0xE3190000
         lfq   f24,SAVE_WORK0(r25)
         b     update
	#.long  0xE3390000
         lfq   f25,SAVE_WORK0(r25)
         b     update
	#.long  0xE3590000
         lfq   f26,SAVE_WORK0(r25)
         b     update
	#.long  0xE3790000
         lfq   f27,SAVE_WORK0(r25)
         b     update
	#.long  0xE3990000
         lfq   f28,SAVE_WORK0(r25)
         b     update
	#.long  0xE3B90000
         lfq   f29,SAVE_WORK0(r25)
         b     update
	#.long  0xE3D90000
         lfq   f30,SAVE_WORK0(r25)
         b     update
	#.long  0xE3F90000
         lfq   f31,SAVE_WORK0(r25)
         b     update
#
#        store quads
#
	#.long  0xF0190000
         stfq  f0,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF0390000
         stfq  f1,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF0590000
         stfq  f2,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF0790000
         stfq  f3,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF0990000
         stfq  f4,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF0B90000
         stfq  f5,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF0D90000
         stfq  f6,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF0F90000
         stfq  f7,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF1190000
         stfq  f8,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF1390000
         stfq  f9,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF1590000
         stfq  f10,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF1790000
         stfq  f11,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF1990000
         stfq  f12,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF1B90000
         stfq  f13,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF1D90000
         stfq  f14,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF1F90000
         stfq  f15,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF2190000
         stfq  f16,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF2390000
         stfq  f17,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF2590000
         stfq  f18,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF2790000
         stfq  f19,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF2990000
         stfq  f20,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF2B90000
         stfq  f21,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF2D90000
         stfq  f22,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF2F90000
         stfq  f23,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF3190000
         stfq  f24,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF3390000
         stfq  f25,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF3590000
         stfq  f26,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF3790000
         stfq  f27,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF3990000
         stfq  f28,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF3B90000
         stfq  f29,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF3D90000
         stfq  f30,SAVE_WORK0(r25)
         b     f_q_ret
	#.long  0xF3F90000
         stfq  f31,SAVE_WORK0(r25)
         b     f_q_ret
