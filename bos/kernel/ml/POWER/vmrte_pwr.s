# @(#)51        1.1.1.4  src/bos/kernel/ml/POWER/vmrte_pwr.s, sysml, bos411, 9434B411a 8/17/94 13:29:52
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
# (C) COPYRIGHT International Business Machines Corp. 1989, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

	.file	"vmrte_pwr.s"
	.machine "pwr"

ifdef(`_POWER_RS',`
#
#       cache line size selectors
#

        .set    DCLSZ,0xd       # data cache line size selector
                                # selected by "01101"=0xd
        .set    MCLSZ,0xe       # minimum cache line size selector
                                # selected by "01110"=0xe

#**********************************************************************
#
#  NAME: clcs
#
#  FUNCTION: cache line compute size
#
#       clcs()                  rc = minimum cache line size
#
#  INPUT STATE:
#
#  OUTPUT STATE:
#       r3 = the minimum cache line size (data, instruction) for the machine
#            is returned.
#
#**********************************************************************

        S_PROLOG(clcs)
        clcs    r3,MCLSZ        # fetch the minimum cache line size,
        S_EPILOG

#**********************************************************************
#
#  NAME: ldsdr0
#
#  FUNCTION: load sdr0
#
#       ldsdr0(value)           rc = none
#
#  INPUT STATE:
#     r3 = value
#
#  OUTPUT STATE:
#       sdr0 contains the new value.
#
#  EXECUTION ENVIRONMENT:
#       Supervisor state  : Yes
#
#**********************************************************************

        S_PROLOG(ldsdr0)
        mtspr   SDR0,r3         # load sdr0 with (r3)
	S_EPILOG

#**********************************************************************
#
#  NAME: invtlb_pwr
#
#  FUNCTION: invalidate TLB
#
#       invtlb(sid,pno)         rc = none
#
#  INPUT STATE:
#     r3 = sid
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

        S_PROLOG(invtlb_pwr)
        mfsr    r5,TEMPSR       # r5 = current contents of sreg TEMPSR
        mtsr    TEMPSR,r3       # use key=0, special bit = 0
                                #   -- thus r3 (sid) = sregval
                                #   sreg TEMPSR = (r3)
        rlinm   r4,r4,L2PSIZE,4,31-L2PSIZE      # page number to byte address
        oriu    r4,r4,TEMPSR<12 # get eaddr by or-ing TEMPSR into top 4 bits
	.machine "any"
        tlbi    r0,r4           # invalidate the tlb
	.machine "com"
        mtsr    TEMPSR,r5       # restore sreg TEMPSR
	S_EPILOG

#****************************************************************************
#
#  NAME:  fetch_and_limit
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

	S_PROLOG(fetch_and_limit_pwr)

	mfmsr	r6			# save the MSR value in r6
	mr	r8,r3			# copy address to scratch
	cal	r7,DISABLED_MSR(0)	# calculate a disabled MSR
	mtmsr	r7			# disable interrupts
	l	r3,0(r8)		# load atomic location value
	andil.	r7,r8,WORD_ALIGNED	# check word alignment
	sf	r7,r4,r3		# subtract delta
	cmp	cr1,r7,r5		# compare to limit
	lil	r3,1			# assume success
	ble	cr1,fail		# branch if less than or equal
	st	r7,0(r8)		# store new value
	mtmsr	r6			# enable interrupts
	beqr	cr0 	    	    	# return if aligned
    	TRAP	    	    	    	# trap, if not aligned

fail:
	lil	r3,0			# return failure
	S_EPILOG

',)

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

