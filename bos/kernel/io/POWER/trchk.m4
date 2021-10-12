# @(#)50	1.11  src/bos/kernel/io/POWER/trchk.m4, sysio, bos411, 9430C411a 7/18/94 05:33:57

# COMPONENT_NAME: SYSTRACE  /dev/systrace pseudo-device driver
#
# FUNCTIONS: .m4 macros for trchka.s
#
# ORIGINS: 27 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1990, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

#
# put real time clock contents in register $1
#
define(r_rtcu,4)		
define(r_rtcl,5)
define(TIMER,
`# $0
ifdef(`_TRCHOOK',`
        l       $1, syscfg_rtctype(0)     # get RTC type
',`
	LTOC($1,_system_configuration,data)
        l       $1, scfg_rtctype($1)     # get RTC type
')
        cmpi    cr0, $1, RTC_POWER_PC     # check 
        beq     cr0, $+12
        mfspr   $1,r_rtcl         # get time in nanoseconds
	b	$+8
	.machine "ppc"
        mftb   $1	         # get time in tic
')
#
# insert hooktype into hookword (into register $1)
#
define(TYPE_INSERT,
`# $0
        cal     r0,$2(0)          # r0 = hooktype
        rlimi   $1,r0,16,12,15    # insert hooktype into hookword
')

