# @(#)74        1.3  src/bos/kernel/lib/libsysp/POWER/cacheinval.s, libsysp, bos411, 9428A410j 3/23/94 11:10:46
#**********************************************************************
# COMPONENT_NAME: (LIBSYSP)
#
# FUNCTIONS: cache_inval
#
# ORIGINS: 27 
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989,1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# Libsysp platform specific modules
#**********************************************************************
		.file	"cacheinval.s"	
		.machine "any"
#**********************************************************************
#
#  NAME: cache_inval
#
#  FUNCTION: flush cache lines for an address range
#
#       cache_inval(eaddr,nbytes)         rc = none
#
#  INPUT STATE:
#     r3 = eaddr
#     r4 = nbytes
#
#  OUTPUT STATE:
#       The designated lines in the cache have been invalidated.
#
#  EXECUTION ENVIRONMENT:
#       Supervisor state  : Yes
#
#  NOTES:
#  This routine is a NOOP for Power PC machine.
#  The input effective address may have any alignment in a page, and
#  the address range may be any within the current address space.
#  The kernel will crash if the address range is not covered by the current
#  address space.
#
#  cache_inval() will first flush cache line of either end of the range 
#  if it is not cache aligned.
#
#  cache_inval() is to be used by device drivers to clean up DATA CACHE
#  for a buffer area before starting a DMA-read.
#  To optimize its performance, data cache line size (instead of
#  min cache line size) is used in the computation.
#  This routine WILL NOT WORK if drivers want to DMA-read INSTRUCTIONS
#  into its buffer.
#
#**********************************************************************

	.set    DCLSZ,0xd       # data cache line size selector

        S_PROLOG(cache_inval)

#	r5 = address of _system_configuration structure
	LTOC(r5,_system_configuration,data)  

#       check parameters
        cmpi    cr0,r4,0        # see if nbytes is le 0
	l	r9,scfg_arch(r5) # architecture field 
        bler                    # return if nbytes <= 0
	cmpi	cr1,r9,POWER_RS	# check the architecture
        a       r8,r4,r3        
	bner	cr1		# return if not POWER_RS
        clcs    r6,DCLSZ        # r6 = the data cache line size,
	ai	r8,r8,-1	# r8 = eaddr of the last byte
        ai      r7,r6,-1        # r7 = mask of log2(clsize) length        
                                # --this assumes cache line sizes are
                                #   a power of two.
#       num_cache_lines =
#        (offset_into_cache_line + nbytes + cache_line - 1) / cache_line_size

	cntlz	r12,r6
        and.    r11,r7,r3       # r11 = offset of starting addr in cache line
	sfi	r12,r12,31	# r12 = log2(data cache line size)
        a       r11,r11,r4        
	and	r9,r8,r7	# r9 = offset of last byte in the last line
        a       r4,r11,r7       # r4 = (offset + nbytes + cache line size - 1)
	cmp	cr1,r9,r7	# check if entire last line is covered
        sr      r10,r4,r12      # r10 = covered line count
	mr	r11,r10		# initialize r11 = # cache line
 
#       check if the starting address is aligned on a cache line boundary
#       if it is not then flush the cache line

        beq     cifulline1      # branch if starting address aligned
	ai.	r11,r11,-1	# r11 = # cache line
        clf     0,r3            # flush the cache line
	beq	dsync

#       check if the last cache line is fully covered by the range
#       if it is not then flush the last cache line

cifulline1:
	sf	r3,r6,r3	# substract line size to start loop
	beq	cr1,cifulline2	# branch if the last line is fully covered
	ai.	r11,r11,-1	# r11 = # whole line	
	clf	0,r8		# flush the last line
	beq	dsync

#	invalidate the remaining lines

cifulline2:
	sri	r8,r10,2        # r8 = bigloop count
	cmp	cr1,r10,r11	# check if clf ever executed
	andil.  r9,r10,0x3      # r9 = # lines to flush in remainloop
	cmpi	cr6,r8,0	# check if big loop count is 0
	beq	cr1,noclf
	dcs			# wait for memory to be stored if clf executed

noclf:
	beq	bigloop		# branch if nothing left for remainloop

	mtctr	r9		# setup loop count
remainloop:
	cli	r3,r6		# cache line invalidate
	bdn	remainloop	# dec ctr and branch if non-zero

bigloop:	
	beq	cr6,isync

	mtctr	r8		# setup loop count
bigloop1:
	cli	r3,r6
	cli	r3,r6
	cli	r3,r6
	cli	r3,r6
	bdn	bigloop1	# dec ctr and branch if non-zero

isync:	ics			# wait for cli to be completed and
	br			# refetch any fetched instructions

dsync:	dcs			# wait for memory to be stored if only
				# execute clf's
	S_EPILOG

	.toc
        TOCE(_system_configuration,data)

	include(INCLML/systemcfg.m4)
     

