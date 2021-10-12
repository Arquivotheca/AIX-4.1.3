# @(#)72        1.2  src/bos/usr/ccs/lib/libc/POWER/sync_cache.s, sysml, bos411, 9428A410j 2/18/94 11:27:38
#****************************************************************************
#
# COMPONENT_NAME: LIBCSYS
#
# FUNCTIONS: _sync_cache_range
#	     sync_cache_range_pwr
#	     sync_cache_range_ppc
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# Libc/sys platform specific modules
#****************************************************************************
		.file	"sync_cache.s"
#****************************************************************************
#
# INCLUDED FUNCTIONS:
#	_sync_cache_range
#****************************************************************************

#       cache line size selectors
        .set    MCLSZ,0xe       # minimum cache line size selector
                                # selected by "01110"=0xe
#****************************************************************************
#
#  NAME: _sync_cache_range
#
#  FUNCTION: synchronize I cache with D cache for a memory range
#
#       _sync_cache_range(eaddr, bcount)	rc = none
#
#  INPUT STATE:
#     r3 = starting effective address to sync
#     r4 = byte count
#
#  OUTPUT STATE:
#     Caches with data in the specified range are synchronized
#
#  EXECUTION ENVIRONMENT: libc.a
#
#  NOTE: Programs performing code modification in SMP systems have to do it 
#	 as follows:
#
#	 For the process/thread that do the code modification:
#        	call _sync_cache_range() 
#
#	 For any other process/thread that executes the updated code:
#		execute isync first
#
#        Additional isync is required because the isync in _sync_cache_range()
#	 can not flush the other processors' buffer for prefetched instructions.
#
#*******************************************************************************

	.machine "com"
        S_PROLOG(_sync_cache_range)

#	r5 = address of _system_configuration structure
	LTOC(r5,_system_configuration,data)  
        cmpi    cr0,r4,0         # see if nbytes is le 0
	l	r9,scfg_arch(r5) # architecture field 
	bler    		 # return if nbytes <= 0
	cmpi	cr1,r9,POWER_RS	 # check the architecture

	# if power platform, branch to sync_cache_range_pwr() 
	# otherwise, fall through sync_cache_range_ppc() 
	beq	cr1,ENTRY(sync_cache_range_pwr) 

#****************************************************************************
#
#  NAME: sync_cache_range_ppc
#
#  FUNCTION: synchronize I cache with D cache for a memory range
#
#       sync_cache_range_ppc(eaddr, bcount) 	rc= none
#
#  INPUT STATE:
#     r3 = starting effective address to sync
#     r4 = byte count
#     r5 = address of _system_configuration structure
#
#  OUTPUT STATE:
#     Caches for the specified range are synchronized
#
#  EXECUTION ENVIRONMENT: called only from _sync_cache_range()
#
#****************************************************************************

        .machine        "ppc"

ENTRY(sync_cache_range_ppc):
	.globl	ENTRY(sync_cache_range_ppc)

#       check cache attribute 
	l	r9,scfg_cattrib(r5)	 # get cache attribute
	l       r6,scfg_dcb(r5) 	 # r6 = dcache_blk_size
	andil.	r9,r9,0x0002		 # cr0 equal bit = 1 if split cache
        l       r7,scfg_icb(r5) 	 # r7 = icache_blk_size,	
	beq	cr0,sync_range_ppc_split # branch if split I/D cache systems

	# combined cache
        ai      r8,r6,-1        # r8 =get mask of log2(cache_blk_size) length
				# --this assumes cache_blk_size is a power of
				#   two.
	cntlz	r9,r6

#       num_cache_blks = 
#       (offset_into_cache_blk + nbytes + cache_blk_size - 1)/cache_blk_size

        and     r10,r8,r3       # r10 = mask off offset in cache blk
	sfi	r11,r9,31	# r11 = log2(cache_blk_size)
        a       r10,r10,r4      # offset + nbytes
        sf      r5,r6,r3        # subtract cache_blk_size to start loop
        a       r9,r10,r8       # + cache_blk_size - 1
	sr	r12,r9,r11	# / cache_blk_size

        sri     r8,r12,2        # r8 = bigloop count
        andil.  r9,r12,0x3      # r9 = # blks to flush in ppc_remainloop
        cmpi    cr1,r8,0        # check if big loop count is 0
        beq     ppc_bigloop     # if nothing left for remainloop

        mtctr   r9              # setup loop count
ppc_remainloop:
        dcbst   r5,r6
	a	r5,r5,r6
        bdn     ppc_remainloop        # dec ctr and branch if non-zero

ppc_bigloop: 
        beq     cr1,ppc_sync

        mtctr   r8              # setup loop count
ppc_bigloop1:
        dcbst   r5,r6		# store one cache block
	a	r5,r5,r6	# go to the next blk       
        dcbst   r5,r6
	a	r5,r5,r6
        dcbst   r5,r6
	a	r5,r5,r6
        dcbst   r5,r6
	a	r5,r5,r6
        bdn     ppc_bigloop1    # dec ctr and branch if non-zero
ppc_sync:
	sync                    # wait until memory updated
	isync                   # discard prefetched instructions
	br

sync_range_ppc_split:
        ai      r8,r6,-1        # r8 =get mask of log2(dcache_blk_size) length
				# --this assumes dcache_blk_size is a power of
				#   two.
	cntlz	r9,r6

#       num_dcache_blks = 
#       (offset_into_dcache_blk + nbytes + dcache_blk_size - 1)/dcache_blk_size

        and     r10,r8,r3       # r10 = mask off offset in dcache blk
	sfi	r11,r9,31	# r11 = log2(dcache_blk_size)
        a       r10,r10,r4      # offset + nbytes
        sf      r5,r6,r3        # subtract dcache_blk_size to start loop
        a       r9,r10,r8       # + dcache_blk_size - 1
	sr	r12,r9,r11	# / dcache_blk_size

        sri     r8,r12,2        # r8 = bigloop count
        andil.  r9,r12,0x3      # r9 = # blks to flush in ppc_remainloop
        cmpi    cr1,r8,0        # check if big loop count is 0
        beq     ppc_dbigloop    # if nothing left for remainloop

        mtctr   r9              # setup loop count
ppc_dremainloop:
        dcbst   r5,r6
	a	r5,r5,r6
        bdn     ppc_dremainloop        # dec ctr and branch if non-zero

ppc_dbigloop: 
        beq     cr1,ppc_dsync

        mtctr   r8              # setup loop count
ppc_dbigloop1:
        dcbst   r5,r6		# store one dcache block
	a	r5,r5,r6	# go to the next blk       
        dcbst   r5,r6
	a	r5,r5,r6
        dcbst   r5,r6
	a	r5,r5,r6
        dcbst   r5,r6
	a	r5,r5,r6
        bdn     ppc_dbigloop1   # dec ctr and branch if non-zero
ppc_dsync:

	# start icache invalidation
        ai      r8,r7,-1        # r8 =get mask of log2(icache_blk_size) length
				# --this assumes icache_blk_size is a power of
				#   two.
	cntlz	r9,r7

#       num_cache_blks =
#       (offset_into_icache_blk + nbytes + icache_blk_size - 1)/icache_blk_size

        and     r10,r8,r3       # r10 = mask off offset in icache blk
	sfi	r11,r9,31	# r11 = log2(icache_blk_size)
        a       r10,r10,r4      # offset + nbytes
        sf      r5,r7,r3        # subtract icache_blk_size to start loop
        a       r9,r10,r8       # + icache_blk_size - 1
	sr	r12,r9,r11	# / icache_blk_size
        sri     r8,r12,2        # r8 = bigloop count
        andil.  r9,r12,0x3      # r9 = # blks to invalidate in ppc_iremainloop
        cmpi    cr1,r8,0        # check if big loop count is 0
	sync			# wait until memory updated
        beq     ppc_ibigloop    # if nothing left for remainloop

        mtctr   r9              # setup loop count
ppc_iremainloop:
        icbi    r5,r7
	a	r5,r5,r7
        bdn     ppc_iremainloop        # dec ctr and branch if non-zero

ppc_ibigloop: 
        beq     cr1,ppc_isync

        mtctr   r8              # setup loop count
ppc_ibigloop1:
        icbi    r5,r7		# invalidate one icache blk
	a	r5,r5,r7	# go to the next blk
        icbi    r5,r7
	a	r5,r5,r7
        icbi    r5,r7
	a	r5,r5,r7
        icbi    r5,r7
	a	r5,r5,r7
        bdn     ppc_ibigloop1   # dec ctr and branch if non-zero

ppc_isync:
	sync			# wait until icache blks are globally invalidated
	isync			# wait until icbi completes and discard
				# prefetched instructions
	br

#****************************************************************************
#
#  NAME: sync_cache_range_pwr
#
#  FUNCTION: synchronize I cache with D cache for a memory range
#
#       sync_cache_range(eaddr, bcount)		rc = none
#
#  INPUT STATE:
#     r3 = starting effective address to sync
#     r4 = byte count
#
#  OUTPUT STATE:
#     Caches with data in the specified range are synchronized
#
#  EXECUTION ENVIRONMENT: 
#	called only from _sync_cache_range()
#
#****************************************************************************

        .machine        "pwr"

ENTRY(sync_cache_range_pwr):
	.globl  ENTRY(sync_cache_range_pwr)

        clcs    r6,MCLSZ        # r6 = minimum cache line size,
        ai      r7,r6,-1        # get mask of log2(clsize) length
                                # --this assumes cache line size is
                                #   a power of two.
	cntlz	r11,r6

#       num_cache_lines =
#       (offset_into_cache_line + nbytes + cache_line - 1) / cache_line_size

        and     r8,r7,r3        # mask off offset in cache line
	sfi	r12,r11,31	# r12 = log2(cache line size)

	# r10 = (offset + nbytes + cache line size - 1) / cache line size
        a       r8,r8,r4        # offset + nbytes
        sf      r5,r6,r3        # subtract line size to start loop
        a       r9,r8,r7        # + cache line size - 1 
	sr	r10,r9,r12	# / cache line size
        sri     r8,r10,2        # r8 = bigloop count
        andil.  r9,r10,0x3      # r9 = # lines to flush in power_remainloop
        cmpi    cr1,r8,0        # check if big loop count is 0
        beq     power_bigloop   # branch if nothing left for remainloop

        mtctr   r9              # setup loop count
power_remainloop:
        clf     r5,r6
        bdn     power_remainloop        # dec ctr and branch if non-zero

power_bigloop: 
        beq     cr1,power_sync

        mtctr   r8              # setup loop count
power_bigloop1:
        clf     r5,r6
        clf     r5,r6
        clf     r5,r6
        clf     r5,r6
        bdn     power_bigloop1  # dec ctr and branch if non-zero

power_sync:
        dcs                     # wait for memory updates to complete
        ics                     # discard prefretched instructions

	S_EPILOG

	FCNDES(_sync_cache_range)

	.toc
        TOCE(_system_configuration,data)

        include(INCLML/errno.m4)
	include(INCLML/systemcfg.m4)







