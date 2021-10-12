# @(#)97        1.5.1.13  src/bos/kernel/ml/POWER/branch.m4, sysml, bos41J, 9518A_all 5/2/95 08:52:18
#
#   COMPONENT_NAME: SYSML
#
#   FUNCTIONS:
#	low memory branch table
#
#   ORIGINS: 27, 83
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
#
#   LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************

#****************************************************************************
#
#	This is a table of machine dependent services.  These services are
#	reached through an indirect branch.  At system initialization time
#	the word at these locations is patched with a branch absolute
#
#	This table must be in pinned V=R memory

ENTRY(d_slave):		.long	0
ENTRY(d_kmove):		.long	0
ENTRY(d_init):		.long	0
ENTRY(d_clear):		.long	0
ENTRY(d_master):	.long	0
ENTRY(d_complete):	.long	0
ENTRY(d_mask):		.long	0
ENTRY(d_unmask):	.long	0
ENTRY(d_move):		.long	0
ENTRY(d_cflush):	.long	0
ENTRY(d_bflush):	.long	0
ENTRY(xmemdma):		.long	0
ENTRY(state_save):	.long	0
ENTRY(resume):		.long	0
ENTRY(v_copypage):	.long	0
ENTRY(v_zpage):		.long	0
ENTRY(vm_cflush):	.long	0
ENTRY(mfdec):		.long	0
ENTRY(i_reset_int):	.long	0
ENTRY(i_unmask):	.long	0
ENTRY(i_mask):		.long	0
ENTRY(i_enableplvl):	.long	0
ENTRY(i_disableplvl):	.long	0
ENTRY(i_loginterr):	.long	0
ENTRY(i_soft):		.long	0
ENTRY(get_from_list):	.long	0
ENTRY(put_onto_list):	.long	0
ENTRY(fetch_and_limit):	.long	0
ENTRY(i_reset_soft):	.long	0
ENTRY(i_issue_eoi):	.long	0
ENTRY(i_genplvl):	.long	0
ENTRY(slock):			.long	0
ENTRY(sunlock):			.long	0
ENTRY(lock_mine):		.long	0
ENTRY(lock_write):		.long	0
ENTRY(lock_read):		.long	0
ENTRY(lock_done):		.long	0
ENTRY(lock_read_to_write):	.long	0
ENTRY(lock_write_to_read):	.long	0
ENTRY(lock_try_write):		.long	0
ENTRY(lock_try_read):		.long	0
ENTRY(lock_try_read_to_write):	.long	0
ENTRY(vmhwinit):        .long   0
ENTRY(p_enter):         .long   0
ENTRY(p_rename):        .long   0
ENTRY(p_remove):        .long   0
ENTRY(p_remove_all):    .long   0
ENTRY(p_is_modified):   .long   0
ENTRY(p_is_referenced): .long   0
ENTRY(p_clear_modify):  .long   0
ENTRY(p_protect):       .long   0
ENTRY(p_page_protect):  .long   0
ENTRY(p_lookup):  	.long   0
ENTRY(iomem_att):	.long	0
ENTRY(iomem_det):	.long	0
ENTRY(io_att):		.long	0
ENTRY(io_det):		.long	0
ENTRY(xmemccpy):  	.long   0
ENTRY(invtlb):  	.long   0
ENTRY(curtime):  	.long   0
ENTRY(update_decrementer):  	.long   0
ENTRY(update_system_time):  	.long   0
ENTRY(copyin):		.long 	0
ENTRY(copyout):		.long 	0
ENTRY(uiocopyin):	.long 	0
ENTRY(uiocopyout):	.long 	0
ENTRY(uiocopyin_chksum):	.long 	0
ENTRY(uiocopyout_chksum):	.long 	0
ENTRY(exbcopy):		.long	0

ifdef(`_KDB',`
ENTRY(kdb_state_save):			.long	0
ENTRY(kdb_state_restore):		.long	0
ENTRY(kdb_state_restore_to_pr_flih):	.long	0
') #endif _KDB

		.globl	ENTRY(d_slave)
		.globl	ENTRY(d_kmove)
		.globl	ENTRY(d_init)
		.globl	ENTRY(d_clear)
		.globl	ENTRY(d_master)
		.globl	ENTRY(d_complete)
		.globl	ENTRY(d_mask)
		.globl	ENTRY(d_unmask)
		.globl	ENTRY(d_move)
		.globl	ENTRY(d_cflush)
		.globl	ENTRY(d_bflush)
		.globl	ENTRY(xmemdma)
		.globl	ENTRY(state_save)
		.globl	ENTRY(resume)
		.globl	ENTRY(v_copypage)
		.globl	ENTRY(v_zpage)
		.globl	ENTRY(vm_cflush)
		.globl	ENTRY(mfdec)
		.globl	ENTRY(i_reset_int)
		.globl	ENTRY(i_unmask)
		.globl	ENTRY(i_mask)
		.globl	ENTRY(i_enableplvl)
		.globl	ENTRY(i_disableplvl)
		.globl	ENTRY(i_loginterr)
		.globl	ENTRY(i_soft)
		.globl	ENTRY(get_from_list)
		.globl	ENTRY(put_onto_list)
		.globl	ENTRY(fetch_and_limit)
		.globl	ENTRY(i_reset_soft)
		.globl	ENTRY(i_issue_eoi)
		.globl	ENTRY(i_genplvl)
		.globl	ENTRY(slock)
		.globl	ENTRY(sunlock)
		.globl	ENTRY(lock_mine)
		.globl	ENTRY(lock_write)
		.globl	ENTRY(lock_read)
		.globl	ENTRY(lock_done)
		.globl	ENTRY(lock_read_to_write)
		.globl	ENTRY(lock_write_to_read)
		.globl	ENTRY(lock_try_write)
		.globl	ENTRY(lock_try_read)
		.globl	ENTRY(lock_try_read_to_write)
                .globl  ENTRY(vmhwinit)
                .globl  ENTRY(p_enter)
                .globl  ENTRY(p_rename)
                .globl  ENTRY(p_remove)
                .globl  ENTRY(p_remove_all)
                .globl  ENTRY(p_is_modified)
                .globl  ENTRY(p_is_referenced)
                .globl  ENTRY(p_clear_modify)
                .globl  ENTRY(p_protect)
                .globl  ENTRY(p_page_protect)
                .globl  ENTRY(p_lookup)
		.globl	ENTRY(iomem_att)
		.globl	ENTRY(iomem_det)
		.globl	ENTRY(io_att)
		.globl	ENTRY(io_det)
                .globl  ENTRY(xmemccpy)
                .globl  ENTRY(invtlb)
                .globl  ENTRY(curtime)
                .globl  ENTRY(update_decrementer)
                .globl  ENTRY(update_system_time)
		.globl	ENTRY(copyin)
		.globl	ENTRY(copyout)
		.globl	ENTRY(uiocopyin)
		.globl	ENTRY(uiocopyout)
		.globl	ENTRY(uiocopyin_chksum)
		.globl	ENTRY(uiocopyout_chksum)
		.globl	ENTRY(exbcopy)

ifdef(`_KDB',`
		.globl	ENTRY(kdb_state_save)
		.globl	ENTRY(kdb_state_restore)
		.globl	ENTRY(kdb_state_restore_to_pr_flih)
') #endif _KDB

FCNDES(d_slave, label)
FCNDES(d_kmove, label)
FCNDES(d_init, label)
FCNDES(d_clear, label)
FCNDES(d_master, label)
FCNDES(d_complete, label)
FCNDES(d_mask, label)
FCNDES(d_unmask, label)
FCNDES(d_move, label)
FCNDES(d_cflush, label)
FCNDES(d_bflush, label)
FCNDES(xmemdma, label)
FCNDES(state_save, label)
FCNDES(resume, label)
FCNDES(v_copypage, label)
FCNDES(v_zpage, label)
FCNDES(vm_cflush, label)
FCNDES(mfdec, label)
FCNDES(i_reset_int, label)
FCNDES(i_unmask, label)
FCNDES(i_mask, label)
FCNDES(i_enableplvl, label)
FCNDES(i_disableplvl, label)
FCNDES(i_loginterr, label)
FCNDES(i_soft, label)
FCNDES(get_from_list, label)
FCNDES(put_onto_list, label)
FCNDES(fetch_and_limit, label)
FCNDES(i_reset_soft, label)
 FCNDES(i_issue_eoi, label)
FCNDES(i_genplvl, label)
FCNDES(slock, label)
FCNDES(sunlock, label)
FCNDES(lock_mine, label)
FCNDES(lock_write, label)
FCNDES(lock_read, label)
FCNDES(lock_done, label)
FCNDES(lock_read_to_write, label)
FCNDES(lock_write_to_read, label)
FCNDES(lock_try_write, label)
FCNDES(lock_try_read, label)
FCNDES(lock_try_read_to_write, label)
FCNDES(vmhwinit, label)
FCNDES(p_enter, label)
FCNDES(p_rename, label)
FCNDES(p_remove, label)
FCNDES(p_remove_all, label)
FCNDES(p_is_modified, label)
FCNDES(p_is_referenced, label)
FCNDES(p_clear_modify, label)
FCNDES(p_protect, label)
FCNDES(p_page_protect, label)
FCNDES(p_lookup, label)
FCNDES(iomem_att, label)
FCNDES(iomem_det, label)
FCNDES(io_att, label)
FCNDES(io_det, label)
FCNDES(xmemccpy, label)
FCNDES(invtlb, label)
FCNDES(curtime, label)
FCNDES(update_decrementer, label)
FCNDES(update_system_time, label)
FCNDES(copyin, label)
FCNDES(copyout, label)
FCNDES(uiocopyin, label)
FCNDES(uiocopyout, label)
FCNDES(uiocopyin_chksum, label)
FCNDES(uiocopyout_chksum, label)
FCNDES(exbcopy, label)

ifdef(`_KDB',`
FCNDES(kdb_state_save, label)
FCNDES(kdb_state_restore, label)
FCNDES(kdb_state_restore_to_pr_flih, label)
') #endif _KDB
		.csect ENTRY(low[PR])

