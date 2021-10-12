# @(#)86        1.18.3.6  src/bos/kernel/ml/proc.m4, sysml, bos411, 9428A410j 2/24/94 07:26:46
#
#   COMPONENT_NAME: SYSML
#
#   FUNCTIONS: image
#
#   ORIGINS: 27, 83
#
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1989, 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************


include(thread.m4)

	.dsect	proc
p_stat:		.byte	0		# process state
		.space	1
p_xstat:	.short	0		# exit status for wait
p_flag:		.long	0		# process flags
p_int:		.long	0		# process flags
p_atomic:	.long	0		# process flags

#	main process link pointers

p_child:	.long	0		# head of list of children
p_siblng:	.long	0		# circular list of siblings
p_uidl:		.long	0		# processes with same pNuid
p_ganchor:	.long	0		# anchor for process group is p_pid

#	thread fields

p_thrdlist:	.long	0		# head of list of threads
p_thrdcount:	.short	0		# number of threads
p_active:	.short	0		# number of active threads
p_suspended:	.short	0		# number of suspended threads
p_terminating:	.short	0		# number of terminating threads
p_local:	.short	0		# number of threads used by "local" pool

#	scheduler fields

p_nice:		.byte	0		# nice for cpu usage
                                        # NOTE: PUSER is added to this
                                        #       for processes that do not 
                                        #       have fixed priority
p_sched_pri:	.byte	0		# pri of our most favored swapped thrd

#	dispatcher fields

p_event:	.long	0		# pending ecbs
p_synch:	.long	0		# event list for this proc suspension

#	identifier fields

p_uid:		.long	0		# real user identifier
p_suid:		.long	0		# set user identifier
p_pid:		.long	0		# process identifier
p_ppid:		.long	0		# parent process id
p_sid:		.long	0		# session id
p_pgrp:		.long	0		# process group leader pid

#	miscellaneous

p_lock:		.long	0		# proc lock
p_kstackseg:	.long	0		# segment for additional kstacks
p_adspace:	.long	0		# process address space
p_grpl:		.long	0		# processes in same process gruop
p_ttyl:		.long	0		# process groups in same session
p_ipc:		.long	0		# ipc when being debugged
p_dblist:	.long	0		# processes being debugged
p_dbnext:	.long	0		# next in p_dblist

#	signal information

p_sig:		.long	0		# pending signals
		.long	0
p_sigignore:	.long	0		# signals being ignored
		.long	0
p_sigcatch:	.long	0		# end, signal area
		.long	0

#	zombie process information

p_p_ru:		.space	18*4

#	process statistics

p_size:		.long	0		# size of image (pages)
p_pctcpu:	.long	0		# cpu percentage
p_auditmask:	.long	0		# auditing stuff
p_minflt:	.long	0		# page fault count - no I/O
p_majflt:	.long	0		# page fault count - I/O needed

# additional scheduler fields

p_repage:	.long	0		# repaging rate
p_sched_count:	.long	0		# watchdog suspension count
p_sched_next:	.long	0		# next process in swap queues
p_sched_back:	.long	0		# prev process in swap queues
p_cpticks:	.short	0		# ticks of cpu time in last sec

p_msgcnt:	.short	0		# uprintf message count
p_majfltsec:	.long	0		# maj flts in the last sec

		.space	1*4

#	end of the proc structure; total size is (p_end-proc)

p_end:		.space	0

#	some process flags for p_flags

	.set	SKPROC,	     0x00000200 # kernel processes
	.set	SSIGSLIH,    0xF0000000 # sigslih to be called
