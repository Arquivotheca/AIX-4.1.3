# @(#)78        1.7  src/bos/kernel/ml/POWER/clock.s, sysml, bos411, 9428A410j 4/29/94 03:35:28
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: Hardware Clock Routines
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
#
# INCLUDED PROCEDURES:
#	curtime_pwr
#
# NOTES:
#       These procedures are pinned and only fault if called on a
#       pageable stack.
#
#       No vm critical section
#       No fixed stack
#       No backtrack
#       Can page fault on caller's stack
#
#	common include files	
#
#       absolute low storage addressability
#
#****************************************************************************

	.file	"clock.s"
	.using  low, 0
	.machine "com"

#****************************************************************************
#
#  NAME:  curtime_pwr
#
#  FUNCTION:  Read the current time from the realtime clock on POWER & 
#		Power_PC 601.
#
# 	curtime_pwr(timerstruc)
#	rc = timerstruc with current time in it
#
#  INPUT STATE:
#	r3 = ptr to the timerstruc into which is placed the current time
#
#  OUTPUT STATE:
#	The timerstruc pointed to by the pointer passed to this procedure
#	contains the current time as maintained by the realtime clock.
#
#  EXECUTION ENVIRONMENT:
# 	Supervisor state  : Yes
#
#*****************************************************************************

	S_PROLOG(curtime_pwr)
curloop:
	mfspr	4, MF_RTCU	# Move realtime clock upper (secs) into r4
	mfspr	5, MF_RTCL	# Move realtime clock lower (ns) into r5
	mfspr	6, MF_RTCU	# Move realtime clock upper (secs) into r6
	cmp	cr0, r4, r6	# See if the both values read for secs match
	.using	timerstruc, r3
	st	r4, tv_sec	# Move the seconds value into the structure
	st	r5, tv_nsec	# Move the ns value into the structure
	beqr			# Repeat until the secs values match
	b	curloop
	_DF(_DF_NOFRAME)

	FCNDES(curtime_pwr)

include(low_dsect.m4)
include(time.m4)
include(scrs.m4)


