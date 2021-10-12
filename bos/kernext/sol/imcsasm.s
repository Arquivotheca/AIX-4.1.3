# @(#)64	1.4  src/bos/kernext/sol/imcsasm.s, sysxsol, bos411, 9428A410j 7/29/92 14:52:19
#
# COMPONENT_NAME: (SYSXSOL) Serial Optical Link Device Driver
#
# FUNCTIONS: imcslra, vm_cinval
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1990
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#************************************************************************
		.file	"imcsasm.s"
#************************************************************************
#
#  assembly code used by imcs
#  copied from vmrte.s
#**********************************************************************
#
#       imcslra
#       vm_cinval

#**********************************************************************
#       common include files
#**********************************************************************
##print(off);

include(machine.m4)

##print(on);
#
#       cache line size selectors
#

#        .set    DCLSZ,0xd       # data cache line size selector
                                # selected by "01101"=0xd
        .set    MCLSZ,0xe       # minimum cache line size selector
                                # selected by "01110"=0xe

        .set    L2SSIZE,28      # log base 2 of segment size (copied from vmdefs.m4)

#       absolute low storage addressability

#        .using  low, 0
##page;
#**********************************************************************
#
#  NAME: imcslra
#
#  FUNCTION: load real address
#
#       lra(eaddr)              rc = real address (32 bits)
#
#  INPUT STATE:
#     r3 = effective address    (32 bits)
#
#  OUTPUT STATE:
#     r3 = real address if address exists
#        = -1   otherwise
#
#  EXECUTION ENVIRONMENT:
#       Supervisor state  : Yes
#
#  NOTES:
#     The -1 (0xFFFFFFFF) return value is possible because the last
#     megabyte of real address space is reserved for addressing ROS.
#
#**********************************************************************

        S_PROLOG(imcslra)
        rac.    r3,r0,r3        # real address compute
        beqr                    # if good, return
        ai      r3,r3,-1        # else set r3 to -1
        S_EPILOG
        FCNDES(imcslra)
##page;
#**********************************************************************
#
#  NAME: vm_cinval
#
#  FUNCTION: flush cache lines for an address range
#
#       vm_cinval(eaddr,nbytes)         rc = none
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
#  The input effective address may have any alignment in a page, and
#  the number of bytes may be any number to which the process has
#  access.
#
#  The pages touched are assumed to be in memory.
#
#  The invalidate operation does not change memory.  It requires an
#  instruction cache synchronize because the architecture allows
#  a combined data and instruction cache.
#
#**********************************************************************

        S_PROLOG(vm_cinval)
#
#       check parameters
#
#	cmpi    cr0,r4,0        # see if nbytes is le 0
	clcs    r5,MCLSZ        # fetch the minimum cache line size,
                                # selected by "01110"=0xe

#	bler			# branch if nbytes <= 0


#
#       calculate the number of times to loop
#

        ai      r6,r5,-1        # get mask of log2(clsize) length
                                # --this assumes cache line sizes are
                                #   a power of two.
        and.    r7,r6,r3        # mask off offset in cache line
	a	r9,r4,r3	# r9 = eaddr + nbytes
#
#	check if the starting address is aligned to the start of a cache line
#	if it is not then flush the cache line
#
	beq	cifulline1	# branch if first
	clf	0,r3		# flush the cache line
cifulline1:
        a       r8,r7,r4        # offset + nbytes
	ai	r9,r9,-1	# r9 = eaddr + nbytes - 1
        a       r7,r8,r6        # plus clsize - 1
        divs    r7,r7,r5        # divide by cache line size
	and.	r10,r9,r6	# get offset of last byte in range into line
        sri     r11,r7,2        # divide by loop size
                                #  --NOTE--this routine assumes a cache
                                #  --line size of 1 KB or less
	cmp	cr0,r10,r6	# check if entire last cache line is used

#
#       calculate any remainder
#
        sli     r8,r11,2         # multiply by loop size
#
#	if eaddr + nbytes -1 does not fall on the end of cache line, the cache
#	line must be flushed
#
	beq	cifulline2	# baranch if address range covers last cache
				# line
	clf	0,r9		# flush cache line
cifulline2:
        sf.     r7,r8,r7        # remainder
        sf      r3,r5,r3        # subtract cache line size to start loop
        cmpi    cr1,r11,0        # set up for check of big loop count
        ble     bigloopci       # if no small loop, go check big loop

#
#       do remainder
#

        mtctr   r7              # set count register
ciloop:
        cli     r3,r5
        bdn     ciloop          # dec ctr and branch if non-zero

#
#       big loop -- set the loop counter and loop until satisfied
#

bigloopci:
        beq     cr1,cisync      # if nothing more to do, synchronize
        mtctr   r11             # set count register
cibloop:
        cli     r3,r5
        cli     r3,r5
        cli     r3,r5
        cli     r3,r5
        bdn     cibloop         # dec ctr and branch if non-zero

#
#       synchronize the instruction cache by waiting for cache invalidate
#       to be completed, and invalidating the instruction prefetch buffer
#

cisync:
        ics
	S_EPILOG

        FCNDES(vm_cinval)
##page;
#**********************************************************************
#
#  NAME: cprgs
#
#  FUNCTION: copies via string instuctions up to 88 bytes
#
#       cprgs(source, destination, nbytes)	rc = none
#
#  INPUT STATE:
#     r3 = address of source string
#     r4 = address of destination string
#     r5 = nbytes
#
#  OUTPUT STATE:
#       nbytes of data will be copied from source to destination
#
#
#**********************************************************************

        S_PROLOG(cprgs)
        stm   r13,-76(r1)           #	save callers regs in its dsa
        mtxer r5                    #	load count register
        lsx   r10,0,r3              #   load source string
        stsx  r10,0,r4              #	and store it
	lm    r13,-76(r1)           #	restore regs
	S_EPILOG
        FCNDES(cprgs)
