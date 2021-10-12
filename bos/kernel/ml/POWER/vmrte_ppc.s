# @(#)54	1.2.1.12  src/bos/kernel/ml/POWER/vmrte_ppc.s, sysml, bos412, 9446B 11/9/94 09:02:57
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS:	invtlb_ppc, inval_all_dbats, fetch_and_limit
#
# ORIGINS: 27
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
#****************************************************************************

		.file	"vmrte_ppc.s"
		.machine "ppc"

ifdef(`_POWER_PC',`

#**********************************************************************
#
#  NAME: invtlb_ppc()
#
#  FUNCTION: invalidate TLB
#
#       invtlb(sid,pno)         rc = none
#
#  INPUT STATE:
#     r3 = sid                  ignored - pwrpc uses only VPI bits 4-19
#     r4 = pno                  page number
#
#  OUTPUT STATE:
#       The TLB for the designated page has been invalidated in the
#       memory management unit.
#
#  EXECUTION ENVIRONMENT:
#       Supervisor state  : Yes
#
#**********************************************************************
	
        S_PROLOG(invtlb_ppc)
        rlinm   r4,r4,L2PSIZE,4,31-L2PSIZE      # page number to byte address
	.machine "any"

ifdef(`_POWER_MP',`
#
#	Acquire TLB lock on MP to ensure only one tlbie at a time
#
tlbi_spin:
	LTOC(r3,tlb_lock,data)
	lwarx	r5,0,r3		# load lock value
	cmpi	cr0,r5,0	# 0 is free
	bne-	cr0,tlbi_spin
	stwcx.	r3,0,r3		# store non zero value in tlbi lock
	bne-	cr0,tlbi_spin
') #endif _POWER_MP

        tlbie   r4              # invalidate the tlb
	.machine "com"

	sync			# tlbie req sync following on 601

ifdef(`_POWER_MP',`
#
#	Release TLB lock
#
	li	r5,0
	st	r5,0(r3)
') #endif _POWER_MP
	S_EPILOG

#**********************************************************************
#
#  NAME: inval_all_dbats
#
#  FUNCTION: invalidate all dbats
#
#  EXECUTION ENVIRONMENT:
#       Supervisor state  : Yes
#
#**********************************************************************

	.machine "ppc"
	S_PROLOG(inval_all_dbats)
	isync
	lil	r3, 0x0
	mtdbatu BAT0,r3
	mtdbatu BAT1,r3
	mtdbatu BAT2,r3
	mtdbatu BAT3,r3
	isync
	.machine "com"
	S_EPILOG		

#****************************************************************************
#
#  NAME:  fetch_and_limit_ppc
#
#  FUNCTION:  atomically decrement a memory location if within specified limit
#
# 	int fetch_and_limit(atomic_p,int,int)
#
#  INPUT STATE:
#	r3 = ptr to the atomic location
#	r4 = delta
#	r5 = limit
#
#  OUTPUT STATE:
#	r3 = 1 and memory location decremented by delta
#	     if (atomic - delta) > limit
#	r3 = 0
#	     if (atomic - delta) <= limit
#
#  EXECUTION ENVIRONMENT:
#	can be called from thread or interrupt.
#	may only page fault when called at INTBASE.
#
#*****************************************************************************

	.machine "ppc"
	S_PROLOG(fetch_and_limit_ppc)

	rlwinm	r7,r3,0,WORD_ALIGNED	# check alignment
	or	r6,r3,r3		# copy address to scratch

retry:
	lwarx	r8,0,r6			# load atomic location value
	twnei	r7,0	    	    	# trap if not aligned
	sub	r9,r8,r4		# subtract delta
	cmp	cr0,r9,r5		# compare to limit
	lil	r3,1			# assume success
	ble	fail			# branch if less than or equal
	stwcx.	r9,0,r6			# store new value
	beqlr+				# predict successful store

	b	retry			# retry

fail:
	lil	r3,0			# return failure
	S_EPILOG


include(vmdefs.m4)
include(scrs.m4)
include(seg.m4)
include(errno.m4)
include(m_types.m4)
include(low_dsect.m4)
include(mstsave.m4)
include(machine.m4)
include(proc.m4)
include(user.m4)
include(param.m4)
include(lock_def.m4)

ifdef(`_POWER_MP',`
	.toc
	TOCE(tlb_lock,data)
') #endif _POWER_MP

',) #endif _POWER_PC
