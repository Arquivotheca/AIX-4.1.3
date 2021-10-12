# @(#)34	1.3  src/bos/usr/ccs/lib/libc/POWER/read_real_time.s, libctime, bos411, 9428A410j 4/28/94 07:19:03
#
#   COMPONENT_NAME: libctime
#
#   FUNCTIONS:
#		read_real_time
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

.file	"read_real_time.s"

#  NAME:  read_real_time
#
#  DESCRIPTION:
#  General interface to read the real time.  If running on
#  POWER or 601, t->tb_high and t->tb_low are set to 
#  real time clock high and real time clock low, and
#  t->flag is set to RTC_POWER, and RTC_POWER
#  is returned.  If running on PowerPC, t->tb_high and
#  t->tb_low are set to time base high and time base
#  low, and t->flag is set to RTC_POWER_PC, and
#  RTC_POWER_PC is returned.
#
#  PARAMETERS:
#	r3 = pointer to structure t
#	r4 = size of structure (future use)
#
# PROTOTYPE: 
# int read_real_time(timebasestruct_t *t,
# 		   size_t size_of_timebasestruct_t);
#
# EXECUTION ENVIRONMENT:
# Problem-state library routine.  Standard linkage convention.

# define the elements in timebasestruct_t
.set	flag, 0
.set	tb_high, flag + 4
.set	tb_low, tb_high + 4

	.machine "com"
	S_PROLOG(read_real_time)
	LTOC(r11,_system_configuration, data)

	mr	r4, r3			# save address of struct
	cal	r3, RTC_POWER(0)	# compute POWER return code
	l	r12,scfg_rtctype(r11)	# get type of clock
	st	r3, flag(r4)		# store POWER flag
	cmpli	cr0, r12, RTC_POWER	# do we have processor rtc?
	bne	cr0, TIME_BASE		# if no, go do time base

real_time_try_again:
	.machine "pwr"
	mfspr	r5,MF_RTCU		# RTCU
	mfspr	r6,MF_RTCL		# RTCL
	mfspr	r7,MF_RTCU		# RTCU
	cmpl	cr0, r5, r7		# test for valid read
	st	r5, tb_high(r4)		# store the high part
	st	r6, tb_low(r4)		# store the low part
	beqr+	cr0			# return if valid read
	b+	real_time_try_again

TIME_BASE:
	.machine "ppc"
	cal	r3, RTC_POWER_PC(0)	# compute PowerPC return code
	st	r3, flag(r4)		# and store it to structure

time_base_try_again:

	mftbu	r5			# time base upper
	mftb	r6			# time base lower
	mftbu	r7			# time base upper (for check)
	cmpl	cr0, r5, r7		# test for carry
	st	r5, tb_high(r4)		# store the high part
	st	r6, tb_low(r4)		# store the low part
	beqr+	cr0			# return if valid read
	b+	time_base_try_again

	FCNDES(read_real_time)

	.toc
	TOCE(_system_configuration, data)

	.machine "com"

include(systemcfg.m4)
include(scrs.m4)
