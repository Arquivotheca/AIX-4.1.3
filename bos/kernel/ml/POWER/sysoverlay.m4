# @(#)55        1.9  src/bos/kernel/ml/POWER/sysoverlay.m4, sysml, bos412, 9445C412a 10/25/94 13:52:53
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) machine language routines
#
# FUNCTIONS: system overlay reserved space
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993,1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#	overlay.m4 will contain the overlay address and length
#
#****************************************************************************
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************

#
#	Reserve a page (4K) for future expansion of millicode
#	type routines
#
	.org	nonpriv_reserv_addr+real0
	.space	nonpriv_reserv_size
#
# Reserve overlay space here
#
	.org	chgsr_addr+real0
ENTRY(chgsr):
	.globl	ENTRY(chgsr)
	.space	chgsr_size

	.org	mtsr_addr+real0
ENTRY(mtsr):
ENTRY(ldsr):
	.globl	ENTRY(mtsr)
	.globl	ENTRY(ldsr)
	.space	mtsr_size

	.org	mfsr_addr+real0
ENTRY(mfsr):
ENTRY(ldfsr):
	.globl	ENTRY(mfsr)
	.globl	ENTRY(ldfsr)
	.space	mfsr_size

	.org	mfsri_addr+real0
ENTRY(mfsri):
ENTRY(vm_geth):
	.globl	ENTRY(mfsri)
	.globl	ENTRY(vm_geth)
	.space	mfsri_size

	.org	simple_lock_addr+real0
ENTRY(simple_lock):
	.globl	ENTRY(simple_lock)
	.space	simple_lock_size

	.org	simple_unlock_addr+real0
ENTRY(simple_unlock):
	.globl	ENTRY(simple_unlock)
	.space	simple_unlock_size

	.org	simple_lock_try_addr+real0
ENTRY(simple_lock_try):
	.globl	ENTRY(simple_lock_try)
	.space	simple_lock_try_size

	.org	fetch_and_add_addr+real0
ENTRY(fetch_and_add):
	.globl	ENTRY(fetch_and_add)
	.space	fetch_and_add_size

	.org	fetch_and_and_addr+real0
ENTRY(fetch_and_and):
	.globl	ENTRY(fetch_and_and)
	.space	fetch_and_and_size

	.org	fetch_and_or_addr+real0
ENTRY(fetch_and_or):
	.globl	ENTRY(fetch_and_or)
	.space	fetch_and_or_size

	.org	fetch_and_nop_addr+real0
ENTRY(fetch_and_nop):
	.globl	ENTRY(fetch_and_nop)
	.space	fetch_and_nop_size

	.org	compare_and_swap_addr+real0
ENTRY(compare_and_swap):
	.globl	ENTRY(compare_and_swap)
	.space	compare_and_swap_size

	.org	test_and_set_addr+real0
ENTRY(test_and_set):
	.globl	ENTRY(test_and_set)
	.space	test_and_set_size

# i_disable/disable_lock go together now--do not modify one without the other!
	.org	i_disable_addr+real0
ENTRY(i_disable):
	.globl	ENTRY(i_disable)
	.space	i_disable_size

	.org	disable_lock_addr+real0
ENTRY(disable_lock):
	.globl	ENTRY(disable_lock)
	.space	disable_lock_size

# i_enable/unlock_enable go together now--do not modify one without the other!
        .org    i_enable_addr+real0
ENTRY(i_enable):
	.globl	ENTRY(i_enable)
	.space  i_enable_size

	.org	unlock_enable_addr+real0
ENTRY(unlock_enable):
	.globl	ENTRY(unlock_enable)
	.space	unlock_enable_size

	.org	fetch_and_add_h_addr+real0
ENTRY(fetch_and_add_h):
	.globl	ENTRY(fetch_and_add_h)
	.space	fetch_and_add_h_size

	.org	rsimple_lock_addr+real0
ENTRY(rsimple_lock):
	.globl	ENTRY(rsimple_lock)
	.space	rsimple_lock_size

	.org	rsimple_lock_try_addr+real0
ENTRY(rsimple_lock_try):
	.globl	ENTRY(rsimple_lock_try)
	.space	rsimple_lock_try_size

	.org	rsimple_unlock_addr+real0
ENTRY(rsimple_unlock):
	.globl	ENTRY(rsimple_unlock)
	.space	rsimple_unlock_size

	.org	lockl_addr+real0
ENTRY(lockl):
	.globl	ENTRY(lockl)
	.space	lockl_size

	.org	unlockl_addr+real0
ENTRY(unlockl):
	.globl	ENTRY(unlockl)
	.space	unlockl_size

ifdef(`_POWER_MP',`
	.org	my_ppda_addr+real0
ENTRY(my_ppda):
	.globl	ENTRY(my_ppda)
	.space	my_ppda_size
	
	.org	my_csa_addr+real0
ENTRY(my_csa):
	.globl	ENTRY(my_csa)
	.space	my_csa_size
	
	.org	get_curthread_addr+real0
ENTRY(get_curthread):
	.globl	ENTRY(get_curthread)
	.space	get_curthread_size
	
	.org	set_csa_addr+real0
ENTRY(set_csa):
	.globl	ENTRY(set_csa)
	.space	set_csa_size
	
	.org	set_curthread_addr+real0
ENTRY(set_curthread):
	.globl	ENTRY(set_curthread)
	.space	set_curthread_size
')

#
# All functions descriptors must be placed here
#
	FCNDES(i_enable, label)
	FCNDES(i_disable, label)
	FCNDES(simple_lock, label)
	FCNDES(simple_unlock, label)
	FCNDES(simple_lock_try, label)
	FCNDES(fetch_and_add, label)
	FCNDES(fetch_and_and, label)
	FCNDES(fetch_and_or, label)
	FCNDES(fetch_and_nop, label)
	FCNDES(compare_and_swap, label)
	FCNDES(test_and_set, label)
	FCNDES(disable_lock, label)
	FCNDES(unlock_enable, label)
	FCNDES(fetch_and_add_h, label)
	FCNDES(rsimple_lock, label)
	FCNDES(rsimple_lock_try, label)
	FCNDES(rsimple_unlock, label)
	FCNDES(lockl, label)
	FCNDES(unlockl, label)
ifdef(`_POWER_MP',`
	FCNDES(my_ppda, label)
	FCNDES(my_csa, label)
	FCNDES(get_curthread, label)
	FCNDES(set_csa, label)
	FCNDES(set_curthread, label)
')


	.csect   ENTRY(low[PR])
