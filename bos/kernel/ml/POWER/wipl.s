# @(#)99	1.9  src/bos/kernel/ml/POWER/wipl.s, sysml, bos411, 9432B411a 8/10/94 10:43:03
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: wipl
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

        .file "wipl.s"
        .machine "com"

#
#	common include files	
#
##print(off);
	include(iplcb.m4)
##print(on);

#       absolute low storage addressability

	.using  low, 0
##page;

#**********************************************************************
#
#  NAME:  warm ipl
#
#  FUNCTION:  Setup for and call ROS warm ipl code.
#
#
#  INPUT STATE:
#	Called from reboot() and sr_slih() with translation enabled
#	R3 contains ROS Warm IPL Entry Address
#	R4 contains value to put into SR15
#
#  OUTPUT STATE:
#	Translation disabled, interrupt prefix enabled
#	Upon exit to ROS warm ipl, R3 will contain
#	the address of the iplcb.
#
#  EXECUTION ENVIRONMENT:
# 	Supervisor state  : Yes
#
#**********************************************************************

#*************************************************************************
#	Turn translation off and interrupt prefix on.  Get the pointer to
#	ipl control block info section.
#*************************************************************************
	S_PROLOG(wipl)
        cal     r8,(MSR_ME + MSR_IP)(0)	# set ME + IP flag
        mtmsr   r8              	# set it in the msr
        mtsr    15,r4                	# load sr15 with seg reg value
	isync				# make sure DR and SR are noticed

# NOTE: we have to do the above 'mtsr' to ZERO SR15 prior to the load from
#       ROS address space.  This is due to the fact that on POWER boxes, the 
#	T bit is still interpreted even in real mode....with the ROS addresses 
#	having a 0xF in the high nibble, it tries to use SR15, which usually 
#	has the T bit set due to previous I/O

	l	r8,0(r3)		# load ros entry point
	cau	r8,r8,0xfff0		# complete address for ROS
        LTOC(r3,ipl_cb,data)		# address of IPL CB pointer
	l       r3,0(r3)		# address of IPL CB
	mtspr	lr,r8			# load up the lr with warm ipl addr
	br				# branch to ROS ipl


	S_EPILOG

##page;
##print(off);
include(low_dsect.m4)
include(machine.m4)
include(systemcfg.m4)
include(scrs.m4)
##print(on);
                .toc
                TOCE(ipl_cb,data)


