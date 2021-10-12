# @(#)66        1.4  src/bos/kernel/ml/POWER/zpage.m4, sysml, bos41J, 9522A_all 5/30/95 18:20:07
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: v_zpage
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#*****************************************************************************
		.file	"zpage.m4"

ifdef(`_POWER_PC',`
	ifdef(`_POWER_603_14',`
		define(SYNC_603,`sync')
	',`
		define(SYNC_603)
	')
')

#*****************************************************************************
#
#  NAME: v_zpage
#
#  FUNCTION: zero page
#
#       v_zpage(sid,pno)                rc = none
#
#  INPUT STATE:
#     r3 = sid
#     r4 = pno                  page number
#
#  OUTPUT STATE:
#       The designated page contains zeroes.
#
#  EXECUTION ENVIRONMENT:
#       Supervisor state  : Yes
#
#*****************************************************************************

ifdef(`_POWER_RS',`
	S_PROLOG(v_zpage_pwr)
')
ifdef(`_POWER_PC',`
ifdef(`_POWER_603_14',`
	S_PROLOG(v_zpage_603)
',`
	S_PROLOG(v_zpage_ppc)
')
')
        LTOC(r7,_system_configuration,data)

        mfsr    r10,TEMPSR      # r10 = current contents of sreg TEMPSR
ifdef(`_POWER_RS',`
	l	r5,scfg_dcline(r7) # r5 = dcache line size
',)
ifdef(`_POWER_PC',`
        l       r5,scfg_dcb(r7) # r5 = dcache block size,
',)
        mtsr    TEMPSR,r3       # use key=0, special bit = 0
                                #   -- thus r3 (sid) = sregval
                                #   sreg TEMPSR = (r3)
ifdef(`_POWER_PC',`
	isync			# synchronize the context
',)
        cal     r6,PSIZE/4(0)   # get PSIZE divided by loop factor
                                #  --NOTE--this routine assumes a cache
                                #  --line size of 1 KB or less

	cntlz	r8,r5
        rlinm   r4,r4,L2PSIZE,4,31-L2PSIZE      # page number to byte address
	sfi	r9,r8,31	# log2(dcache line/block size)
        oriu    r4,r4,TEMPSR<12 # r4 = eaddr derived by or-ing TEMPSR into 
				#      top 4 bits
	sr	r6,r6,r9	# r6 = number of lines/blocks per page/4
                                #   (assume all numbers are powers of 2)
	
        sf      r4,r5,r4        # offset by cache line/blk size to start loop
#
#       zero each line/blk in the page
#
        mtspr   CTR,r6          # initialize loop counter
ifdef(`_POWER_603_14',`
	mfmsr	r12		# disable interrupts to avoid resume rfi
	rlinm	r11, r12, 0, ~MSR_EE
	mtmsr	r11
')
zloop:
ifdef(`_POWER_RS',`
        dclz    r4,r5		# zero one cache line and go to the next blk
        dclz    r4,r5		# ditto
        dclz    r4,r5		# ditto 
        dclz    r4,r5		# ditto
',)
ifdef(`_POWER_PC',`
	SYNC_603		# 603 1.4 work around
	dcbz	r4,r5		# zero one cache blk
	a	r4,r4,r5	# go to the next blk
	SYNC_603
	dcbz	r4,r5
	a	r4,r4,r5
	SYNC_603
	dcbz	r4,r5
	a	r4,r4,r5
	SYNC_603
	dcbz	r4,r5
	a	r4,r4,r5

',)
        bdn     zloop           # branch if count nonzero

ifdef(`_POWER_603_14',`
	mtmsr	r12
')

        mtsr    TEMPSR,r10      # restore sreg TEMPSR

ifdef(`_POWER_PC',`
	ifdef(`_POWER_MP',`
	sync			# make sure every processor sees a zeroed page
	',)			
	isync			# synchronize the context
',)
        S_EPILOG

ifdef(`_POWER_PC',`
ifdef(`_POWER_603_14',`
	FCNDES(v_zpage_603)
',`
	FCNDES(v_zpage_ppc)
')
',)

ifdef(`_POWER_RS',`
	FCNDES(v_zpage_pwr)
',)

        .toc
        TOCE(_system_configuration,data)

	include(vmdefs.m4)
	include(scrs.m4)
	include(systemcfg.m4)
	include(machine.m4)
