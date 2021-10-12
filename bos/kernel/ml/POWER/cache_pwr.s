# @(#)59        1.1  src/bos/kernel/ml/POWER/cache_pwr.s, sysml, bos411, 9428A410j 7/14/93 13:41:02
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: sync_cache_page_pwr, vm_cflush_pwr	 
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
#****************************************************************************
		.file	"cache_pwr.s"
#****************************************************************************
#
# INCLUDED FUNCTIONS:
#	sync_cache_page_pwr   
#	vm_cflush_pwr	      
#
#****************************************************************************

#       cache line size selector
        .set    MCLSZ,0xe       # minimum cache line size selector
                                # selected by "01110"=0xe
#****************************************************************************
#
#  NAME: sync_cache_page_pwr
#
#  FUNCTION: synchronize I cache with D cache for a virtual page
#
#       sync_cache_page_pwr(sid,pno)	rc = none
#
#  INPUT STATE:
#     r3 = segment id
#     r4 = page number in the segment
#
#  OUTPUT STATE:
#     Caches with data in the specified page are synchronized
#
#  EXECUTION ENVIRONMENT:
#	called from VMM machine dependent layer with I/D translation off.
#	MUST turn data translation on before flushing cache due to
#	cache synonym problem.
#
#  Note: (1) This implementation assumes that the minimum cache line size is 
#            a power of two and is less than or equal to 1/4 of the page size.
#	 (2) No other data references should be made after turning xlate on
#	     other than the page to be synchronized.
#
#***************************************************************************

        .machine        "pwr"

        S_PROLOG(sync_cache_page_pwr)

        mfmsr   r10              	# get current msr
#
#       set up virtual address for sid,pno
#
        clcs    r5,MCLSZ                # fetch the minimum cache line size,
        mfsr    r11,TEMPSR              # save value of sr13
        oril    r6,r10,MSR_DR 		# turn data translation on
        sli     r4,r4,L2PSIZE           # calculate segment offset for the page
	cntlz	r7,r5
        mtmsr   r6              	# set it in the msr
        mtsr    TEMPSR,r3               # set segment id
	sfi	r8,r7,31		# r8 = log2(cache line size)
        cal     r9,PSIZE/4(0)           # r9 = number of bytes to flush/4
        oriu    r3,r4,TEMPSR<12         # calculate efective address
	sr	r6,r9,r8		# r6 = loop count
        sf      r3,r5,r3                # subtract for clf

        mtctr   r6                      # set up loop counter
sync_loop:
        clf     r3,r5                   # flush line
        clf     r3,r5
        clf     r3,r5
        clf     r3,r5
        bdn     sync_loop               # dec ctr and branch if non-zero

        mtsr    TEMPSR,r11              # restore segment register
        dcs
        ics
        mtmsr   r10              	# restore original msr
        S_EPILOG


#****************************************************************************
#
#  NAME: vm_cflush_pwr
#
#  FUNCTION: flush I cache and D cache for a memory range
#
#       vm_cflush_pwr(eaddr, nbytes)
#
#  INPUT STATE:
#       r3 = starting effective address to sync
#       r4 = byte count
#
#  OUTPUT STATE:
#       Caches with data in the specified range are flushed
#
#  RETURN VALUE:
#	0 	if success
#	EINVAL  if nbytes < 0
#
#  EXECUTION ENVIRONMENT: 
#	Called through the branch table
#
#****************************************************************************

        .machine        "pwr"

	S_PROLOG(vm_cflush_pwr)

        clcs    r6,MCLSZ        # r6 = minimum cache line size,
        cmpi    cr0,r4,0        # see if nbytes is le 0

        ai      r7,r6,-1        # get mask of log2(clsize) length
                                # --this assumes cache line size is
                                #   a power of two.
	cntlz	r11,r6
        ble     cr0,spec_nbytes # branch if nbytes <= 0

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
	lil	r3,0x0		# success return code
        dcs                     # wait for memory updates to complete
        ics                     # discard prefretched instructions
	br

spec_nbytes:
	lil	r3,0x0		# success return code
	beqr	cr0		# return if nbytes = 0
	lil	r3,EINVAL	# error return code
	S_EPILOG

	FCNDES(vm_cflush_pwr)

        include(vmdefs.m4)
        include(errno.m4)
	include(systemcfg.m4)
	include(machine.m4)






