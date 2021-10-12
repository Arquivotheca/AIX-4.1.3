# @(#)31        1.15  src/bos/kernel/ml/POWER/ppda.m4, sysml, bos411, 9438C411a 9/23/94 10:53:22
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Defines
#
# FUNCTIONS:
#
# ORIGINS: 27 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1992, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# LEVEL 1,  5 Years Bull Confidential Information
#
# See comments in ppda.h
#
#****************************************************************************

ifdef(`_POWER_MP',`
	.set	MAXCPU,			8	# for this binary
',`
	.set	MAXCPU,			1	# for this binary
') #endif _POWER_MP
	.set	DCACHE_LSIZE,		64	# in bytes

	.dsect	ppdarea

#	Alignment Flih Save Area Definitions

SAVE_WORK0:
SAVE_MATCH:	.byte	0		# Alias for where match byte is stored
SAVE_RT:	.byte	0		# Alias for where copy of RT is stored
SAVE_RA:	.byte	0		# Alias for where copy of RA is stored
SAVE_RB:	.byte	0		# Alias for where copy of RB is stored
SAVE_WORK1:	.long	0		# Save Area Work space 1
SAVE_WORK2:  	.long   0		# Save Area Work space 2
SAVE_WORK3:	.long	0    		# Save Area Work space 3
SAVE_R25:    	.long	0		# Save Area copy of r25
SAVE_R26:    	.long	0		# Save Area copy of r26
SAVE_R27:    	.long	0		# Save Area copy of r27
SAVE_R28:    	.long	0		# Save Area copy of r28
SAVE_R29:    	.long	0		# Save Area copy of r29
SAVE_R30:    	.long	0		# Save Area copy of r30
SAVE_R31:    	.long	0		# Save Area copy of r31
SAVE_SRR0:    	.long	0		# Save Area copy of SRR0
SAVE_SRR1:    	.long	0		# Save Area copy of SRR1
SAVE_LR:	.long	0   		# Save Area copy of LR
SAVE_CR:	.long	0   		# Save Area copy of CR
SAVE_XER:    	.long	0		# Save Area copy of XER

ppda_csa:	.long	0		# current save area
ppda_mstack:	.long	0		# next available mst
ppda_fpowner:	.long	0
ifdef(`_THREADS',`
ppda_curthread:	.long	0
',`
ppda_curproc:	.long	0
')

ppda_syscall:	.long	0		# count of sys calls/processor

ppda_save0:	.long	0		# flih scratch save area
ppda_save1:	.long	0
ppda_save2:	.long	0
ppda_save3:	.long	0
ppda_save4:	.long	0
ppda_save5:	.long	0
ppda_save6:	.long	0
ppda_save7:	.long	0
ppda_intr:	.long	0		# PPC interrupt register address
#
# ppda_softid & ppda_softpri must stay in the same word
#
ppda_softis:	.short	0		# Hardware assist
ppda_softpri:	.short	0		# Software managed priorities
ppda_prilvl0:	.long	0		# Array of priority to levels
ppda_prilvl1:	.long	0		# When MAX_LVL_PER_PRI goes beyond
ppda_prilvl2:	.long	0		# 32 then these will have to
ppda_prilvl3:	.long	0		# change also.
ppda_prilvl4:	.long	0
ppda_dar:	.long	0		# save dar here
ppda_dsisr:	.long	0		# save dsisr her
ppda_dsi_flag:	.long	0		# flag for dsi_flih
ppda_dssave0:	.long	0		# save area for dsi_flih
ppda_dssave1:	.long	0
ppda_dssave2:	.long	0
ppda_dssave3:	.long	0
ppda_dssave4:	.long	0
ppda_dssave5:	.long	0
ppda_dssave6:	.long	0
ppda_dssave7:	.long	0

# Off-level scheduler anchors reserve 4 words for them (INTOFFL3-INTOFFL0+1)
OFFLVL_SCHEDS:	.space	4 * 4

ppda_cpuid:	.short	0		# cpu logical number
ppda_stackfix:	.byte	0
ppda_lru:	.byte	0
ppda_vmflags:				# word of VMM flags:
ppda_sio:		.byte 0		#	flag for starting i/o
ppda_reservation:	.byte 0		#	frame reservation
ppda_hint:		.byte 0		#	hint in the scoreboard
ppda_vmrsvd:		.byte 0		#	reserved
ppda_no_vwait:		.long 0		# flag for v_wait
ppda_scoreboard0:	.long 0		# scoreboard containing lock addresses
ppda_scoreboard1:	.long 0		# to be released on backtrack
ppda_scoreboard2:	.long 0
ppda_scoreboard3:	.long 0
ppda_scoreboard4:	.long 0		# scoreboard containing lock addresses
ppda_scoreboard5:	.long 0		# to be released on backtrack
ppda_scoreboard6:	.long 0
ppda_scoreboard7:	.long 0
ppda_mfrr_pend:	.long 	0
ppda_mfrr_lock: .long	0
ppda_mpc_pend:	.long	0

ppda_iodonelist:	.long	0	# per-cpu iodone list

#
# Timer handling data. Struct ppda_timer, see ppda.h
#
ppda_timer_t_free:           .long 0	
ppda_timer_t_active:         .long 0	
ppda_timer_t_freecnt:        .long 0	
ppda_timer_t_called:         .long 0	
ppda_timer_trblock:          .long 0	
ppda_timer_systimer:         .long 0
ppda_timer_ticks_it:         .long 0
ppda_timer_ref_time:         .long 0
			     .long 0
ppda_timer_time_delta:       .long 0
ppda_timer_time_adjusted:    .long 0
ppda_timer_wtimer:           .long 0
			     .long 0
			     .long 0
			     .long 0
			     .long 0
ppda_timer_w_called:         .long 0

ppda_affinity:               .long 0

kdb_ppda_csa: 	 .long   0		# address of current kdb save area
kdb_ppda_mstack: .long   0		# next kdb area in mstsave pool
kdb_ppda_r0:	 .long	 0		# FLIH save area for gpr 0
kdb_ppda_r1:	 .long	 0		# FLIH save area for gpr 1
kdb_ppda_r2:	 .long	 0		# FLIH save area for gpr 2
kdb_ppda_r15:	 .long	 0		# FLIH save area for gpr 15
kdb_ppda_cr:	 .long	 0		# FLIH save area for cr
kdb_ppda_sr0:	 .long   0              # save area for seg reg 0
kdb_ppda_sr2:	 .long   0              # save area for seg reg 2
kdb_ppda_iar:    .long   0
kdb_ppda_msr:    .long   0

ppda_TB_ref_u:	 .long	0	 	# ref. of Time Base value: upper
ppda_TB_ref_l:	 .long	0	 	# ref. of Time Base value: lower
ppda_sec_ref:	 .long	0	 	# ref. of Time Base value: seconds
ppda_nsec_ref:	 .long	0	 	# ref. of Time Base value: nanos

ppda_ficd:	 .byte   0		# finish interrupt call dispatch
ppda_pad1:	.space  3		# pad to word boundary

ppda_compress:	.long	0		# filesystem compression buffer
ppda_qio:	.long	0		# VMM queued I/O pages

ppda_cs_sync:	.long	0		# Synchronous MPC indicator
ppda_pmapstk:	.long	0		# Address of V=R pmap stack

ppdfiller:	.space	32

ppdend:		.space	0
