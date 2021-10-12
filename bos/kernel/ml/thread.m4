# @(#)46	1.14  src/bos/kernel/ml/thread.m4, sysml, bos41J, 9513A_all 3/24/95 15:17:51
#
#   COMPONENT_NAME: SYSML
#
#   FUNCTIONS: none
#
#   ORIGINS: 27, 83
#
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
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************


	.dsect	thread
t_state:	.byte	0		# thread state
t_wtype:	.byte	0		# type of thread wait
t_suspend:	.short	0		# suspend signal nesting level
t_flags:	.long	0		# thread flags
t_atomic:	.long	0		# thread flags
t_stackp:	.long	0		# saved user stack pointer

# 	related data structures

t_procp:	.long	0		# owner process
t_uthreadp:	.long	0		# local data
t_userp:	.long	0		# owner process ublock

#	main thread link pointers

t_prevthread:	.long	0		# previous thread in same process
t_nextthread:	.long	0		# next thread	in same process

#	sleep and lock fields

t_lock:		.long	0		# thread lock
t_wchan:	.long	0		# wait identifier - hashed
t_wchan1:	.long	0		# wait identifier - real
t_wchan1sid:	.long	0		# SID of wchan1
t_wchan1offset:	.long	0		# OFFSET of wchan1
t_wchan2:	.long	0		# VMM wait channel
t_swchan:	.long	0		# channel for simple/complex lock list
t_eventl:	.long	0		# event list
t_result:	.long	0		# wait result
t_polevl:	.long	0		# page out wait level
t_pevent:	.long	0		# pending events
t_wevent:	.long	0		# awaited events
t_slist:	.long	0		# threads waiting for s/c locks
t_lockcount:	.short	0		# number of locks held

#	dispatcher fields

t_ticks:	.short	0		# # of ticks since dispatched
t_prior:	.long	0		# running list
t_next:		.long	0		# running/wait list
t_synch:	.long	0		# threads waiting for me to be suspended
t_dispct:	.long	0		# number of dispatches
t_fpuct:	.long	0		# number of FP unavail ints.

#	scheduler fields

t_cpuid:	.short	0		# processor on which I am bound
t_scpuid:	.short	0		# saved last t_cpuid for funnelling
t_affinity:	.short	0		# processor on which I last ran
t_pri:		.byte	0		# current effective priority
t_policy:	.byte	0		# scheduling policy
t_cpu:		.short	0		# processor usage
                                        # NOTE: the bounds for p_cpu are
                                        #       0<= t_cpu <= 20+HZ
t_lockpri:	.byte	0		# thread priority while holding a lock
t_wakepri:	.byte	0		# wakeup priority for the thread
t_time:		.byte	0		# resident time for scheduling
t_sav_pri:	.byte	0		# original, unboosted priority
		.space	1		# padding to word boundary

#	signal information

t_cursig:	.byte	0		# current/last signal taken
t_sig:		.long	0		# pending signals
		.long	0
t_sigmask:	.long	0		# current signal mask
		.long	0
t_scp:		.long	0		# sigctx location in user space

#	identifier fields

t_tid:		.long	0		# thread identifier

#	miscellaneous fields

t_graphics:	.long	0		# user address space tied to graphics
t_cancel:       .long   0               # thread cancelation
t_lockowner:    .long   0               # thread to be boosted 
t_boosted:      .long   0               # thread boost count
t_tsleep:	.long	0		# thread_tsleep event list
t_userdata:	.long	0		# user owned data

		.space	4*4

#	end of the thread structure; total size is (t_end-thread)

t_end:		.space	0

#	some thread flags for t_flags

	.set	TSIGSLIH,    0x00000007 # sigslih to be called
	.set	TSIGINTR,    0x0000000F # sigslih to be called
	.set	TKTHREAD,    0x00001000 # kernel thread

