# @(#)96	1.7  src/bos/kernel/ml/POWER/systemcfg.m4, sysml, bos411, 9428A410j 4/28/94 07:20:20
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) machine language routines
#
# FUNCTIONS:
#	Defines from systemcfg.h
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

#
#	Values for architecture field
#
			.set	POWER_RS, 1
			.set	POWER_PC, 2

#
#	Values for implementation field
#
			.set	POWER_RS1, 0x0001
			.set	POWER_RSC, 0x0002
			.set	POWER_RS2, 0x0004
			.set	POWER_601, 0x0008
			.set	POWER_603, 0x0020
			.set	POWER_604, 0x0010
			.set	POWER_620, 0x0040
			.set	POWER_PC_ALL, 0x0078
			.set	POWER_RS_ALL, 0x0007

#
#	Values for model architecture field
#
			.set	RS6K, 1
			.set	RSPC, 2

#
#	Values for model implementation field
#
			.set	RS6K_UP_MCA, 1
			.set	RS6K_SMP_MCA, 2
			.set	RS6K_UP_PCI, 3

#
#	Values for RTC type field
#
			.set	RTC_POWER, 1
			.set	RTC_POWER_PC, 2

			.dsect systemcfg
scfg_arch:		.long	0	# processor architecture 
scfg_impl:		.long	0	# processor implementation
scfg_version:		.long	0	# processor version
scfg_width:		.long	0	# width (32 || 64)
scfg_ncpus:		.long	0	# 1 = UP, n = n-way MP
scfg_cattrib:		.long	0	# bit 31: 0=no cache, 1 = cache present
					# bit 30: 0=separate I/D, 1 = combined
scfg_icsize:		.long	0	# size of L1 instruction cache
scfg_dcsize:		.long	0	# size of L1 data cache
scfg_icasc:		.long	0	# L1 instruction cache associativity
scfg_dcasc:		.long	0	# L1 data cache associativity
scfg_icb:		.long	0	# L1 instruction cache block size
scfg_dcb:		.long	0	# L1 data cache block size
scfg_icline:		.long	0	# L1 instruction cache line size
scfg_dcline:		.long	0	# L1 data cache line size
scfg_L2csize:		.long	0	# size of L2 cache, 0 = No L2 cache
scfg_L2casc:		.long	0	# L2 cache associativity
scfg_tlbattrib:		.long	0	# bit 31: 0=no tlb, 1 = tlb present
					# bit 30: 0=separate I/D, 1 = combined
scfg_itsize:		.long	0	# entries in instruction TLB
scfg_dtsize:		.long	0	# entries in data TLB
scfg_itasc:		.long	0	# instruction tlb associativity
scfg_dtasc:		.long	0	# data tlb associativity
scfg_ressize:		.long	0	# size of reservation
scfg_priv_lcnt:		.long	0	# spin lock count in supervisor mode
scfg_prob_lcnt:		.long	0	# spin lock count in problem state
scfg_rtctype:		.long	0	# RTC type 
scfg_virtalias:		.long	0	# 1 if hardware aliasing is supported
scfg_cachcong:		.long	0	# number of page bits for cache synonm
scfg_modarch:		.long	0	# model architecture
scfg_modimpl:		.long	0	# model implementation
scfg_Xint:		.long	0	# time base conversion
scfg_Xfrac:		.long	0	# time base conversion




