# @(#)81	1.4  src/bos/kernel/ml/sysinfo.m4, sysml, bos411, 9428A410j 6/16/90 03:44:06
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1990
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************


#  NAME:    sysinfo.m4

#           This is the assembly language version of "<sys/sysinfo.h>",
#           which defines counters and other statistical data.

#	    N.B.:  Any changes to <sys/sysinfo.h> MUST be reflected here
#	           as well!


       .dsect   sysinfo

sysinfo_cpu:			# four CPU times
sysinfo_cpu_idle:	.long	0
sysinfo_cpu_user:	.long	0
sysinfo_cpu_kernel:	.long	0
sysinfo_cpu_wait:	.long	0
			.set	CPU_IDLE, 	0
			.set	CPU_USER, 	1
			.set	CPU_KERNEL,	2
			.set	CPU_WAIT,	3

				# counters
sysinfo_bread:		.long	0
sysinfo_bwrite:		.long	0
sysinfo_lread:		.long	0

sysinfo_lwrite:		.long	0
sysinfo_phread:		.long	0
sysinfo_phwrite:	.long	0
sysinfo_pswitch:	.long	0
sysinfo_syscall:	.long	0
sysinfo_sysread:	.long	0
sysinfo_syswrite:	.long	0
sysinfo_sysfork:	.long	0
sysinfo_sysexec:	.long	0
sysinfo_runque:		.long	0
sysinfo_runocc:		.long	0
sysinfo_swpque:		.long	0
sysinfo_swpocc:		.long	0
sysinfo_iget:		.long	0
sysinfo_namei:		.long	0
sysinfo_dirblk:		.long	0
sysinfo_readch:		.long	0
sysinfo_writech:	.long	0
sysinfo_rcvint:		.long	0
sysinfo_xmtint:		.long	0
sysinfo_mdmint:		.long	0
sysinfo_rawch:		.long	0
sysinfo_canch:		.long	0
sysinfo_outch:		.long	0
sysinfo_msg:		.long	0
sysinfo_sema:		.long	0
sysinfo_ksched:		.long	0	# added for kernel processes 
sysinfo_koverf:		.long	0
sysinfo_kexit:		.long	0
sysinfo_rbread:		.long	0	# remote read requests
sysinfo_rcread:		.long	0	# reads from remote cache
sysinfo_rbwrt:		.long	0	# remote writes
sysinfo_rcwrt:		.long	0	# cached remote writes
sysinfo_devintrs:	.long	0	# device interrupts
sysinfo_softintrs:	.long	0	# software interrupts
sysinfo_traps:		.long	0	# traps


       .dsect   syswait

syswait_iowait:		.short	0
syswait_physio:		.short	0


       .dsect   syserr

syserr_inodeovf:	.long	0
syserr_fileovf:		.long	0
syserr_textovf:		.long	0
syserr_procovf:		.long	0

syserr_sbi:			# five entries:
syserr_sbi_siloc:	.long	0
syserr_sbi_crdrds:	.long	0
syserr_sbi_alert:	.long	0
syserr_sbi_fault:	.long	0
syserr_sbi_timeo:	.long	0
			.set	SBI_SILOC,	0
			.set	SBI_CRDRDS,	1
			.set	SBI_ALERT,	2
			.set	SBI_FAULT,	3
			.set	SBI_TIMEO,	4
