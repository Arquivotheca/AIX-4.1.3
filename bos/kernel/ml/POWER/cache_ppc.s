# @(#)58        1.1.1.3  src/bos/kernel/ml/POWER/cache_ppc.s, sysml, bos411, 9428A410j 5/31/94 10:25:24
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: sync_cache_page_ppc, vm_cflush_ppc_comb, 
#	     vm_cflush_ppc_splt, inval_icache_page_ppc
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
#***************************************************************************
		.file	"cache_ppc.s"
#***************************************************************************
#
# INCLUDED FUNCTIONS:
#	sync_cache_page_ppc   
#	vm_cflush_ppc_comb
#	vm_cflush_ppc_splt
#	inval_icache_page_ppc
#
# N.B:  sync_cache_range_ppc is in lib/c/sys/sync_cache.s
#
#***************************************************************************

#***************************************************************************
#
#  NAME: sync_cache_page_ppc
#
#  FUNCTION: synchronize I cache with D cache for a page
#
#       sync_cache_page_ppc(raddr)	rc = none
#
#  INPUT STATE:
#     r3 = real starting address of the page
#
#  OUTPUT STATE:
#     Caches for the specified page are synchronized
#
#  EXECUTION ENVIRONMENT:
#     called with I/D translation off
#
#  Note: This implementation assumes that I/D cache line size is 
#        a power of two and is less than or equal to 1/4 of the page size.
#	 It handles/optimizes for combined cache case.
#***************************************************************************

        .machine        "ppc"

        S_PROLOG(sync_cache_page_ppc)

        LTOC(r5,_system_configuration,data)
#       check parameters and cache attribute 
        cal     r9,PSIZE/4(0)           # r9 = number of bytes to flush/4
	l	r4,scfg_cattrib(r5)	# get cache attribute
	l       r6,scfg_dcb(r5) 	# r6 = dcache_blk_size
	andil.	r4,r4,0x0002		# cr0 equal bit = 1 if split cache
        l       r7,scfg_icb(r5) 	# r7 = icache_blk_size,	
	cntlz	r8,r6
	beq	cr0,sync_page_ppc_spl	# branch if split I/D cache

	# combined I/D cache
	sfi	r8,r8,31		# r8 = log2(cache_blk_size)
	sr	r10,r9,r8		# r10 = loop count
        sf      r5,r6,r3                # subtract blk size to start loop
	mtctr	r10			# set up loop counter

sync_loop:
        dcbst   r5,r6			# store one cache blk
	a	r5,r5,r6		# go to the next blk
	dcbst   r5,r6
	a	r5,r5,r6
        dcbst   r5,r6
	a	r5,r5,r6
        dcbst   r5,r6
	a	r5,r5,r6
        bdn     sync_loop   		# dec ctr and branch if non-zero
	sync				# wait until memory updated

	isync
	br

	# split I/D cache
sync_page_ppc_spl:
 	sfi	r8,r8,31		# r8 = log2(dcache_blk_size)
	cntlz	r4,r7
	sr	r10,r9,r8		# r10 = loop count
 	sfi	r4,r4,31		# r4 = log2(icache_blk_size)
        sf      r5,r6,r3                # subtract blk size to start loop
	mtctr	r10			# set up loop counter

sync_dloop:
        dcbst   r5,r6			# store one dcache blk
	a	r5,r5,r6		# go to the next blk
	dcbst   r5,r6
	a	r5,r5,r6
        dcbst   r5,r6
	a	r5,r5,r6
        dcbst   r5,r6
	a	r5,r5,r6
        bdn     sync_dloop   		# dec ctr and branch if non-zero

	# start icache invalidation
	sr	r10,r9,r4		# r10 = loop count
        sf      r5,r7,r3                # subtract blk size to start loop
	mtctr	r10			# set up loop counter
	sync				# wait until memory updated

sync_iloop:
        icbi    r5,r7			# invalidate one icache blk
	a	r5,r5,r7		# go to the next blk
        icbi    r5,r7
	a	r5,r5,r7
        icbi    r5,r7
	a	r5,r5,r7
        icbi    r5,r7
	a	r5,r5,r7
        bdn     sync_iloop   		# dec ctr and branch if non-zero
	ifdef(`_POWER_MP',`
	sync			        # wait until icbis are globally performed
	',)
	isync				# wait until icbi completes and discard
					# prefetched instructions
	S_EPILOG


#***************************************************************************
#
#  NAME: vm_cflush_ppc_comb
#
#  FUNCTION: flush I cache and D cache for a memory range
#
#       vm_cflush_ppc_comb (eaddr, nbytes)     
#
#  INPUT STATE:
#     	r3 = starting effective address to flush
#     	r4 = byte count
#
#  OUTPUT STATE:
#     	Caches with data in the specified range are flushed
#
#  RETURN VALUE:
#	0 	if success
#	EINVAL  if nbytes < 0
#
#  EXECUTION ENVIRONMENT: 
#	Called through the branch table
#
#***************************************************************************

        .machine        "ppc"

        S_PROLOG(vm_cflush_ppc_comb)

#       r5 = address of _system_configuration structure
#       r6 = value of cache-block-size field in the structure

        LTOC(r5,_system_configuration,data)
        cmpi    cr0,r4,0        # see if nbytes is le 0
        l	r6,scfg_dcb(r5) # r6 = cache block size,
        ble     cr0,spec0_nbytes  # branch if nbytes <= 0
        ai      r7,r6,-1        # get mask of log2(cache block size) length
                                # -- this assumes cache block size is
                                #    a power of two.
	cntlz	r11,r6
	
#       num_cache_blocks =
#       (offset_into_cache_block + nbytes + cache_block_size - 1) /
#	cache_block_size

        and     r8,r7,r3        # mask off offset in cache block
	sfi	r11,r11,31	# r11 = log2(cache block size)
        a       r8,r8,r4        # offset + nbytes
        sf      r5,r6,r3        # subtract block size to start loop
        a       r9,r8,r7        # + cache block size - 1
	sr	r10,r9,r11	# / cache block size

        sri     r8,r10,2        # r8 = bigloop count
        andil.  r9,r10,0x3      # r9 = # blocks to flush in com_remainloop
        cmpi    cr1,r8,0        # chech if big loop count is 0
        beq     ppc_com_bigloop     # if nothing left for remainloop

        mtctr   r9              # setup loop count
ppc_com_remainloop:
        dcbf    r5,r6		# flush a cache block
	a	r5,r5,r6	# increment r5 to the next cache block
        bdn     ppc_com_remainloop  # dec ctr and branch if non-zero

ppc_com_bigloop: 
        beq     cr1,ppc_com_sync

        mtctr   r8              # setup loop count
ppc_com_bigloop1:
        dcbf    r5,r6		# flush a cache block
	a	r5,r5,r6	# increment r5 to the next cache block
        dcbf    r5,r6		
	a	r5,r5,r6	
        dcbf    r5,r6		
	a	r5,r5,r6	
        dcbf    r5,r6		
	a	r5,r5,r6	
        bdn     ppc_com_bigloop1    # dec ctr and branch if non-zero

ppc_com_sync:
	lil	r3,0x0		# success return code
        sync                    # wait for memory updates to complete
        isync                   # discard prefretched instructions
	br

spec0_nbytes:
	lil	r3,0x0		# success return code
	beqr	cr0		# return if nbytes = 0
	lil	r3,EINVAL	# error return code
        S_EPILOG

	FCNDES(vm_cflush_ppc_comb)


#***************************************************************************
#
#  NAME: vm_cflush_ppc_splt
#
#  FUNCTION: flush I cache and D cache for a memory range
#
#       vm_cflush_ppc_splt(eaddr, nbytes)
#
#  INPUT STATE:
#  	r3 = starting effective address to flush
#     	r4 = byte count
#
#  OUTPUT STATE:
#     	Caches with data in the specified range are flushed
#
#  RETURN VALUE:
#	0 	if success
#	EINVAL  if nbytes < 0
#
#  EXECUTION ENVIRONMENT: 
#	Called through the branch table
#
#***************************************************************************

        .machine        "ppc"

        S_PROLOG(vm_cflush_ppc_splt)

#       r5 = address of _system_configuration structure
#       r6 = value of dcache-block-size field in the structure

        LTOC(r5,_system_configuration,data)
        cmpi    cr0,r4,0        # see if nbytes is le 0
        l	r6,scfg_dcb(r5) # r6 = dcache block size,
        ble     cr0,spec1_nbytes # branch if nbytes <= 0
	l	r12,scfg_icb(r5) # r12 = icache block size
        ai      r7,r6,-1        # get mask of log2(dcache block size) length
                                # --this assumes cache block size is
                                #   a power of two.
	cntlz	r11,r6
	
#       num_cache_blocks =
#        (offset_into_cache_block + nbytes + cache_block_size - 1) / 
#	 cache_block_size

        and     r8,r7,r3        # mask off offset in cache block
	sfi	r11,r11,31	# r11 = log2(dcache block size)
        a       r8,r8,r7        # offset + cache block size - 1
        sf      r5,r6,r3        # subtract block size to start loop
        a       r9,r8,r4        # + nbytes
        ai      r7,r12,-1       # get mask of log2(icache block size) length
                                # -- this assumes cache block size is
                                #    a power of two.
	sr	r10,r9,r11	# / cache block size
	cntlz	r11,r12
        sri     r8,r10,2        # r8 = bigloop count
        andil.  r9,r10,0x3      # r9 = # blocks to flush in d_remainloop
        cmpi    cr1,r8,0        # chech if big loop count is 0
        beq     ppc_d_bigloop   # if nothing left for remainloop

        mtctr   r9              # setup loop count
ppc_d_remainloop:
        dcbf    r5,r6		# flush a dcache block
	a	r5,r5,r6	# increment r4 to the next cache block
        bdn     ppc_d_remainloop  # dec ctr and branch if non-zero

ppc_d_bigloop: 
        and     r10,r7,r3       # mask off offset in cache block
	sfi	r11,r11,31	# r11 = log2(icache block size)
        a       r10,r10,r4      # offset + nbytes
        beq     cr1,ppc_d_sync

        mtctr   r8              # setup loop count
ppc_d_bigloop1:
        dcbf    r5,r6		# flush a dcache block
	a	r5,r5,r6	# increment r5 to the next cache block
        dcbf    r5,r6		
	a	r5,r5,r6	
        dcbf    r5,r6		
	a	r5,r5,r6	
        dcbf    r5,r6		
	a	r5,r5,r6	
        bdn     ppc_d_bigloop1  # dec ctr and branch if non-zero

ppc_d_sync:
        a       r9,r10,r7       # + cache block size - 1
        sync                    # wait for memory updates to complete

#       num_cache_blocks =
#       (offset_into_cache_block + nbytes + cache_block_size - 1) / 
#	cache_block_size

	sr	r8,r9,r11	# / cache block size
        sf      r5,r12,r3       # subtract block size to start loop
        sri     r10,r8,2        # r8 = bigloop count
        andil.  r9,r8,0x3       # r9 = # blocks to flush in i_remainloop
        cmpi    cr1,r10,0       # chech if big loop count is 0
        beq     ppc_i_bigloop   # if nothing left for remainloop

        mtctr   r9              # setup loop count
ppc_i_remainloop:
        icbi    r5,r12		# flush a icache block
	a	r5,r5,r12	# increment r5 to the next cache block
        bdn     ppc_i_remainloop  # dec ctr and branch if non-zero

ppc_i_bigloop: 
        beq     cr1,ppc_i_sync

        mtctr   r10              # setup loop count
ppc_i_bigloop1:
        icbi    r5,r12		# flush a icache block
	a	r5,r5,r12	# increment r5 to the next cache block
        icbi    r5,r12		
	a	r5,r5,r12	
        icbi    r5,r12		
	a	r5,r5,r12	
        icbi    r5,r12		
	a	r5,r5,r12	
        bdn     ppc_i_bigloop1  # dec ctr and branch if non-zero

ppc_i_sync:
	lil	r3,0x0		# success return code
        isync                   # discard prefretched instructions
	br

spec1_nbytes:
	lil	r3,0x0		# success return code
	beqr	cr0		# return if nbytes = 0
	lil	r3,EINVAL	# error return code
        S_EPILOG

	FCNDES(vm_cflush_ppc_splt)



#***************************************************************************
#
#  NAME: inval_icache_page_ppc
#
#  FUNCTION: invalidate I cache for a page
#
#       inval_icache_page_ppc(raddr)	rc = none
#
#  INPUT STATE:
#     r3 = real starting address for the page
#
#  OUTPUT STATE:
#     icache for the specified page is invalidated
#
#  Note: this implementation assumes that I cache line size is 
#        a power of two and is less than or equal to 1/4 of the page size.
#***************************************************************************

        .machine        "ppc"
	
	S_PROLOG(inval_icache_page_ppc)

        LTOC(r5,_system_configuration,data)
        cal     r9,PSIZE/4(0)           # r9 = number of bytes to flush/4
	l	r4,scfg_cattrib(r5)	# get cache attribute
	l       r6,scfg_icb(r5) 	# r6 = icache_blk_size
	andil.	r4,r4,0x0002		# cr0 equal bit = 1 if split cache
	cntlz	r7,r6
 	sfi	r8,r7,31		# r8 = log2(icache_blk_size)
	beq	cr0,inval_icache	# branch if split I/D cache
	
	# UP && combined I/D cache
ifdef(`_POWER_MP',`
',`
	isync				
	br
')
	# MP or split I/D cache
inval_icache:
	sr	r10,r9,r8		# r10 = loop count
        sf      r11,r6,r3               # subtract blk size to start loop

	mtctr	r10			# set up loop counter

inv_icache_loop:
	icbi	r11,r6			# invalidate icache blk
	a	r11,r11,r6		# go to the next blk
	icbi	r11,r6
	a	r11,r11,r6
	icbi	r11,r6
	a	r11,r11,r6
	icbi	r11,r6
	a	r11,r11,r6
	bdn	inv_icache_loop		# loop if not done

ifdef(`_POWER_MP',`
	sync				# wait until icache blks are globally 
					# invalidated
',)
	isync				# wait until icbi completes and discard
					# prefetched instructions
	S_EPILOG

	.toc
        TOCE(_system_configuration,data)

        include(vmdefs.m4)
        include(errno.m4)
	include(systemcfg.m4)


	




