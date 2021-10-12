# @(#)68        1.6  src/bos/kernel/ml/POWER/lock_def.m4, sysml, bos411, 9428A410j 3/29/94 08:14:46
#*****************************************************************************
#
# COMPONENT_NAME: (SYSPROC) 
#
# FUNCTIONS: assembler defines from sys/lock_def.h
#
# ORIGINS: 27 83
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
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************

	.set	LOCK_AVAIL, -1		# initial lock value

#
#	flags
#
	.set	LOCK_NDELAY, 1		# do not wait, if unavailable

#
#	return codes
#
	.set	LOCK_SUCC, 0		# success !
	.set	LOCK_NEST, 1		# alread locked by this process
	.set	LOCK_FAIL, -1		# lock not available

# lockword value: lock free
	.set	SIMPLE_LOCK_AVAIL,	0

# interlock bit
	.set	INTERLOCK,		0x80000000

# waiting bit
	.set	WAITING,		0x40000000

# lock bit
	.set	LOCKBIT,		0x20000000

# recursion bit
	.set	RECURSION,		0x10000000

# instrumentation bit
	.set	INSTR_ON,		0x08000000

# if lsb is set, lock's owner is a thread
# (thread id is always odd)
	.set	THREAD_BIT,		0x00000001

# mask all state bits
	.set	OWNER_MASK,		0x07ffffff
	.set	LOCKL_OWNER_MASK,	0x3fffffff

# other defines
	.set	LOCK_SWRITE_TRACE,      1
	.set	LOCK_WRITE_TRACE,       2
	.set	LOCK_READ_TRACE,        3
	.set	LOCK_UPGRADE_TRACE,     4
	.set	LOCK_DOWNGRADE_TRACE,   5

# boolean defines
	.set	TRUE,			1
	.set	FALSE,			0

# miscellaneous defines
	.set	WORD_ALIGNED,		0x00000003
	.set	HALF_WORD_ALIGNED,	0x00000001
	.set	ALIGN_TO_WORD,		0xFFFFFFFC

# trace subhooks
	.set	hkwd_LOCK_TAKEN         , 1
	.set	hkwd_LOCK_MISS          , 2
	.set	hkwd_LOCK_RECURSIVE     , 3
	.set	hkwd_LOCK_BUSY		, 4
	.set	hkwd_LOCK_DISABLED	, 8
	
	.set	hkwd_SETRECURSIVE       , 1
	.set	hkwd_CLEARRECURSIVE     , 2

ifdef(`_INSTRUMENTATION',`
		.dsect	lock_data_instrumented
s_lock:		.long	0
		.space	4
acquisitions:	.long	0
misses:		.long	0
sleeps:		.long	0
lockname:	.long	0
		.space	8
ifdef(`DEBUG',`
lock_lr:	.long	0
lock_caller:	.long	0
lock_cpuid:	.long	0
unlock_lr:	.long	0
unlock_caller:	.long	0
dbg_zero:	.space	4	# this word must be zero
unlock_cpuid:	.long	0
dgb_flags:	.long	0
')
')
