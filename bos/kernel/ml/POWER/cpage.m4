# @(#)60        1.4  src/bos/kernel/ml/POWER/cpage.m4, sysml, bos41J, 9522A_all 5/30/95 18:33:25
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: v_copypage
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
#****************************************************************************
		.file	"cpage.m4"
#**********************************************************************
#
# v_copypage
#
#  FUNCTION: copy page
#
#       v_copypage(tsid,tpno,ssid,spno)         rc = none
#
#       copy page (ssid,spno) to (tsid,tpno).
#
#  INPUT STATE:
#     r1 = caller's stack pointer
#     r3 = tsid                 target sid
#     r4 = tpno                 target page number
#     r5 = ssid                 source sid
#     r6 = spno                 source page number
#
#  OUTPUT STATE:
#       The source page has been copied to the target page.
#
#  EXECUTION ENVIRONMENT:
#       Supervisor state  : Yes
#
#  Note: (1) this implementation assumes that D cache line size, D cache blk 
#	     size, and I cache blk size are a power of two and are less than 
#	     or equal to 1/4 of the page size.
#
#	 (2) On Power platforms, I cache has been invalidated for the page 
#	     frame when it was paged out or released.  So we do not need to
#	     invalidate I cache here.
#
#	 (3) In MP configurations, a process performing a fork may be preempted
#            in v_copypage().
#	     The sync instruction in the unlock routine called when the process
#	     changes its state will guarantee that all executed cache 
#	     operations are globally visible.
#	     
#       Maximum disabled time is about xxx  cycles.
#
#**********************************************************************

ifdef(`_POWER_PC',`
	ifdef(`_POWER_601',`
	       define(_POWER_PC_COMB,)	
	',`
	       define(_POWER_PC_SPLT,)
	')

	ifdef(`_POWER_603_14',`
		define(SYNC_603,`sync')
	',`
		define(SYNC_603)
	')
')

ifdef(`_POWER_RS',`
	S_PROLOG(v_copypage_pwr)
',)
ifdef(`_POWER_PC_COMB',`
	S_PROLOG(v_copypage_ppc_comb)
',)
ifdef(`_POWER_PC_SPLT',`
ifdef(`_POWER_603_14',`
	S_PROLOG(v_copypage_603)
',`
	S_PROLOG(v_copypage_ppc_splt)
')
')
        mfsr    r10,TEMPSR      # save current contents of sreg TEMPSR
        mfsr    r11,PTASR       # save current contents of sreg PTASR
        mtsr    TEMPSR,r3       # use key=0, special bit = 0
                                #   -- thus r3 (tsid) = sregval
                                #   sreg TEMPSR = (r3)
        mtsr    PTASR,r5        # use key=0, special bit = 0
                                #   -- thus r5 (ssid) = sregval
                                #   sreg PTASR = (r5)
ifdef(`_POWER_PC',`
	isync			# synchronize the context
',)
        rlinm   r3,r4,L2PSIZE,4,31-L2PSIZE      # page number to byte address
        LTOC(r5,_system_configuration,data)
        rlinm   r4,r6,L2PSIZE,4,31-L2PSIZE      # page number to byte address
        mfmsr   r0              # save the current msr
        cal     r12,PSIZE/4(0)  # get PSIZE divided by loop factor

ifdef(`_POWER_RS',`
        l       r6,scfg_dcline(r5) # r6 = dcache line size
',)
ifdef(`_POWER_PC',`
        l       r6,scfg_dcb(r5) # r6 = dcache block size
	ifdef(`_POWER_PC_COMB',`
	',`
        l       r8,scfg_icb(r5) # r8 = icache block size	
	')
',)

        oriu    r3,r3,TEMPSR<12 # r3 = get target eaddr by or-ing TEMPSR
                                #      into top 4 bits
	cntlz	r7,r6
        oril    r5,r0,MSR_FP    # turn on floating point
	sfi	r7,r7,31	# r7 = log2(dcache line/block size)
        oriu    r4,r4,PTASR<12  # r4 = get source eaddr by or-ing PTASR
                                #      into top 4 bits
	sr	r9,r12,r7	# r9 = loop count
        rlinm   r5,r5,0,~MSR_EE # disable external interrupts
#
#       zero the lines in the target page
#       this saves loading the cache from the target page
#
        sf      r7,r6,r3        # establish starting address for loop
        mtspr   CTR,r9          # set count to number of data cache lines/blks
ifdef(`_POWER_RS',`
	mr	r8,r7		# r8 = target saved for later cache store loop
',)

ifdef(`_POWER_603_14',`
	mtmsr	r5		# turn off interrupts to avoid resume rfi
')

czloop:
ifdef(`_POWER_RS',`
        dclz    r7,r6           # zero cache line
        dclz    r7,r6           # zero cache line
        dclz    r7,r6           # zero cache line
        dclz    r7,r6           # zero cache line
',)
ifdef(`_POWER_PC',`
	SYNC_603		# 603 1.4 workaround
	dcbz	r7,r6		# zero cache block
	a	r7,r7,r6
	SYNC_603
	dcbz	r7,r6		# zero cache block
	a       r7,r7,r6
	SYNC_603
	dcbz	r7,r6		# zero cache block
	a       r7,r7,r6
	SYNC_603
	dcbz	r7,r6		# zero cache block
	a       r7,r7,r6
',)
        bdn     czloop          # dec ctr and branch if non-zero

#
#       copy source page to target page, 32 bytes at a time
#
        mtmsr   r5              # fp available disabled
ifdef(`_POWER_PC',`
	isync			# synchronize the context
',)
        stfd    fr1,-8(r1)      # save fr 1 in caller's fr save area
        stfd    fr2,-16(r1)     # save fr 2 in caller's fr save area
        stfd    fr3,-24(r1)     # save fr 3 in caller's fr save area
        stfd    fr4,-32(r1)     # save fr 4 in caller's fr save area
                                #   --don't need to buy a stack frame
        cal     r12,4(r0)       # r12 = outer loop count
        cal     r7,32(r0)       # r7 = inner loop count
        cal     r3,-8(r3)       # r3 -> 8 bytes before target
        cal     r4,-8(r4)       # r4 -> 8 bytes before source

oloop:  mtspr   CTR,r7          # set count for inner loop
#
#       the inner loop copies 1024 bytes and runs disabled
#
iloop:  lfdu    fr1,8(r4)       # increment r4 and get 8 bytes
        lfdu    fr2,8(r4)       # ditto
        lfdu    fr3,8(r4)       # ditto
        lfdu    fr4,8(r4)       # ditto
        stfdu   fr1,8(r3)       # increment r3 and store 8 bytes
        stfdu   fr2,8(r3)       # ditto
        stfdu   fr3,8(r3)       # ditto
        stfdu   fr4,8(r3)       # ditto
        bdn     iloop           # dec ctr and branch if non-zero
        lfd     fr1,-8(r1)      # restore fr1
        lfd     fr2,-16(r1)     # restore fr2
        lfd     fr3,-24(r1)     # restore fr3
        lfd     fr4,-32(r1)     # restore fr4
        ai.     r12,r12,-1      # decrement outer loop counter
        mtmsr   r0              # restore msr to original value
ifdef(`_POWER_PC',`
	isync			# synchronize the context
',)
        beq     cret            # branch if done
        mtmsr   r5              # disable and make fp available
ifdef(`_POWER_PC',`
	isync			# synchronize the context
',)
        b       oloop           # continue next loop
#
#       Store the cache lines of target page so I-cache will see results of 
#	this operation.  This might later be optimised by interleaving with 
#	the copying operation.
#       Regs 6,8,9 are saved from earlier cache zero operation.
#
cret:
ifdef(`_POWER_RS',`
        mtspr   CTR,r9          # set counter to number of data cache lines
scloop_pwr:
        dclst   r8,r6           # store cache line
        dclst   r8,r6           # store cache line
        dclst   r8,r6           # store cache line
        dclst   r8,r6           # store cache line
        bdn     scloop_pwr      # dec ctr and branch if non-zero
        dcs                     # wait until memory updated
',)

ifdef(`_POWER_PC_SPLT',`
	ai	r7,r3,-(PSIZE-8) # r7 = starting target address
        mtspr   CTR,r9          # set counter to number of data cache blks
	mr	r3,r7		# target saved for later icbi loop
	cntlz	r9,r8
        cal     r5,PSIZE/4(0)   # get PSIZE divided by loop factor
	sfi	r9,r9,31	# r9 = log2(icache block size)
	sf	r7,r6,r7	# subtract dcache blk size to start loop 
	sr	r4,r5,r9	# r4 = (number of icache blocks per page)/4

scloop_ppc:
	dcbst	r7,r6		# store dcache block
	a	r7,r7,r6
	dcbst	r7,r6		# store dcache block
	a	r7,r7,r6
	dcbst	r7,r6		# store dcache block
	a	r7,r7,r6
	dcbst	r7,r6		# store dcache block
	a	r7,r7,r6
        bdn     scloop_ppc      # dec ctr and branch if non-zero
        sync                    # wait until memory updated

	sf	r3,r8,r3	# subtract icache blk size to start loop
	mtspr	CTR,r4
icloop_ppc:
	icbi	r3,r8		# invalidate icache block
	a	r3,r3,r8
	icbi	r3,r8		# invalidate icache block
	a	r3,r3,r8
	icbi	r3,r8		# invalidate icache block
	a	r3,r3,r8
	icbi	r3,r8		# invalidate icache block
	a	r3,r3,r8
        bdn     icloop_ppc      # dec ctr and branch if non-zero
	ifdef(`_POWER_MP',`
	sync			# wait until icbi are globally performed
	',)
',)

ifdef(`_POWER_PC_COMB',`
	# Store dcache blocks in UP systems with cache-inhibited 
	# I-fetch problem (i.e. early versions of 601 processor will 
	# sometimes short-circuit the cache and fetch instructions from 
	# main memory when the page I bit = 0
	#
	# Store dcache blocks in MP configurations (regardless of
	# comb/split cache) because instruction fetches may be originated
	# as noncoherent requests in Power PC architecture regardless of  
	# the page M bit.

	ai	r7,r3,-(PSIZE-8) # r7 = starting target address
        mtspr   CTR,r9          # set counter to number of cache blks
	sf	r7,r6,r7	# subtract cache blk size to start loop 
scloop_ppc:
	dcbst	r7,r6		# store dcache block
	a	r7,r7,r6
	dcbst	r7,r6		# store dcache block
	a	r7,r7,r6
	dcbst	r7,r6		# store dcache block
	a	r7,r7,r6
	dcbst	r7,r6		# store dcache block
	a	r7,r7,r6
        bdn     scloop_ppc      # dec ctr and branch if non-zero
        sync                    # wait until memory updated
',)

        mtsr    TEMPSR,r10      # restore sreg TEMPSR
        mtsr    PTASR,r11       # restore sreg PTASR
        ics                     # synchronize the context and 
				# discard prefetched instructions
        S_EPILOG

ifdef(`_POWER_RS',`
	FCNDES(v_copypage_pwr)
',)
ifdef(`_POWER_PC_COMB',`
	FCNDES(v_copypage_ppc_comb)
',)
ifdef(`_POWER_PC_SPLT',`
ifdef(`_POWER_603_14',`
	FCNDES(v_copypage_603)
',`
	FCNDES(v_copypage_ppc_splt)')
',)

undefine(_POWER_PC_COMB)
undefine(_POWER_PC_SPLT)

        .toc
        TOCE(_system_configuration,data)

	include(vmdefs.m4)
	include(scrs.m4)
	include(errno.m4)
	include(systemcfg.m4)
	include(machine.m4)


