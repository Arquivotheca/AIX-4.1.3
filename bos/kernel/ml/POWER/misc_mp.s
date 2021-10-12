# @(#)02	1.3  src/bos/kernel/ml/POWER/misc_mp.s, sysml, bos41J, 9517A_all 4/25/95 17:47:45
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: my_phys_id
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************

	.file "misc_mp.s"
	.using  low, r0

        S_PROLOG(my_phys_id)
ifdef(`_POWER_PC',`
	.machine "ppc"
ifdef(`_POWER_603',`
	lwz	 r3, syscfg_impl	#
	cmpi	cr0, r3, POWER_603	# Is this a 603?
	bne	cr0, not603		# ..no, there is probably a PID register
	xor	 r3,  r3,  r3		# Forge physical ID = 0
	br				#
not603: ') #endif _POWER_603
	mfspr	 r3, PID		# Processor Identification Register
	andil.	 r3,  r3, PID_MASK	# Only 4 significant bits
	.machine "com"
',` #else _POWER_PC
	lil	r3, 0
') #endif _POWER_PC
	br
	FCNDES(my_phys_id)

include(scrs.m4)
include(low_dsect.m4)
include(systemcfg.m4)
