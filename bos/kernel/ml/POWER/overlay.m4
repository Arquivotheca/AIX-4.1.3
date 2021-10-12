# @(#)04        1.18  src/bos/kernel/ml/POWER/overlay.m4, sysml, bos412, 9445C412a 10/25/94 13:52:36
#
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) machine language routines
#
# FUNCTIONS: overlay addresses and sizes
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1992,1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# NOTE:
#	Do not change without updating overlay.h
#
#	See warnings in "overlay.h"
#
#****************************************************************************
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************

	.set	mulh_addr,	0x3100
	.set	mulh_size,	0x0080

	.set	mull_addr,	0x3180
	.set	mull_size,	0x0080

	.set	divss_addr,	0x3200
	.set	divss_size,	0x0080

	.set	divus_addr,	0x3280
	.set	divus_size,	0x0080

	.set	quoss_addr,	0x3300
	.set	quoss_size,	0x0080

	.set	quous_addr,	0x3380
	.set	quous_size,	0x0080

#
# addresses of atomic lock primitives required by PPC ABI.
# Do not change.
#

	.set	_clear_lock_addr,	0x3400
	.set	_clear_lock_size,	0x0020

	.set	_check_lock_addr,	0x3420
	.set	_check_lock_size,	0x0040

	.set	cs_addr,	0x3460
	.set	cs_size,	0x0040

	.set	nonpriv_reserv_addr,	0x8000
	.set	nonpriv_reserv_size,	0x1000

# i_disable/disable_lock go together now--do not modify one without the other!
	.set	i_disable_addr,		0x9000
	.set	i_disable_size,		0x0004
	.set	disable_lock_addr,	0x9004
	.set	disable_lock_size,	0x02FC

# i_enable/unlock_enable go together now--do not modify one without the other!
	.set	i_enable_addr,		0x9300
	.set	i_enable_size,		0x0004
	.set	unlock_enable_addr,	0x9304
	.set	unlock_enable_size,	0x01FC

	.set	simple_lock_addr,	0x9500
	.set	simple_lock_size,	0x0400

	.set	simple_unlock_addr,	0x9900
	.set	simple_unlock_size,	0x0200

	.set	fetch_and_add_addr,	0x9B00
	.set	fetch_and_add_size,	0x0060

	.set	busgetc_addr,	0xA000
	.set	busgetc_size,	0x0060

	.set	busgets_addr,	0xA060
	.set	busgets_size,	0x0060

	.set	busgetsr_addr,	0xA0c0
	.set	busgetsr_size,	0x0060

	.set	busgetl_addr,	0xA120
	.set	busgetl_size,	0x0060

	.set	busgetlr_addr,	0xA180
	.set	busgetlr_size,	0x0060

	.set	busputc_addr,	0xA1e0
	.set	busputc_size,	0x0060

	.set	busputs_addr,	0xA240
	.set	busputs_size,	0x0060

	.set	busputl_addr,	0xA2a0
	.set	busputl_size,	0x0060

	.set	busputsr_addr,	0xA300
	.set	busputsr_size,	0x0060

	.set	busputlr_addr,	0xA360
	.set	busputlr_size,	0x0060

	.set	buscpy_addr,	0xA3c0
	.set	buscpy_size,	0x00a0
#
#	Keep all read/write segment registers in the same cache
#	line (64 bytes).  These sizes are exact.  Do not change
#	these address without updating inline.h.
#

	.set	chgsr_addr,	0xA480
	.set	chgsr_size,	0x0018

	.set	mtsr_addr,	0xA498
	.set	mtsr_size,	0x0014

	.set	mfsr_addr,	0xA4AC
	.set	mfsr_size,	0x000c

	.set	mfsri_addr,	0xA4B8
	.set	mfsri_size,	0x0008

	.set	compare_and_swap_addr,	0xA4C0
	.set	compare_and_swap_size,	0x0100

	.set	test_and_set_addr,	0xA5C0
	.set	test_and_set_size,	0x0100

	.set	simple_lock_try_addr,	0xA6C0
	.set	simple_lock_try_size,	0x0200

	.set	rsimple_lock_addr,	0xA8C0
	.set	rsimple_lock_size,	0x0180

	.set	rsimple_lock_try_addr,	0xAA40
	.set	rsimple_lock_try_size,	0x0180

	.set	rsimple_unlock_addr,	0xABC0
	.set	rsimple_unlock_size,	0x0100

	.set	lockl_addr,		0xACC0
	.set	lockl_size,		0x0200

	.set	fetch_and_or_addr,	0xAEC0
	.set	fetch_and_or_size,	0x0060

	.set	fetch_and_nop_addr,	0xAF20
	.set	fetch_and_nop_size,	0x0060

	.set	fetch_and_and_addr,	0xAF80
	.set	fetch_and_and_size,	0x0060

	.set	fetch_and_add_h_addr,	0xB000
	.set	fetch_and_add_h_size,	0x0100

	.set	unlockl_addr,		0xB100
	.set	unlockl_size,		0x0200

ifdef(`_POWER_MP',`
# ppda/csa/curthread management
	.set	my_ppda_addr,		0xB400
	.set	my_ppda_size,		0x0008
	
	.set	my_csa_addr,		0xB408
	.set	my_csa_size,		0x0020
	
	.set	get_curthread_addr,	0xB428
	.set	get_curthread_size,	0x0020
	
	.set	set_csa_addr,		0xB448
	.set	set_csa_size,		0x0018
	
	.set	set_curthread_addr,	0xB460
	.set	set_curthread_size,	0x0010
')
