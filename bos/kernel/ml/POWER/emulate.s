# @(#)67        1.2  src/bos/kernel/ml/POWER/emulate.s, sysml, bos411, 9430C411a 7/27/94 02:37:20
#*****************************************************************************
#
# COMPONENT_NAME: (SYSPROC) Kernel Assembler Code
#
# FUNCTIONS: 	emulate_cli_clf
#		emulate_dclst
#		emulate_clcs
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
			.file   "emulate.s"

        .machine        "ppc"

#**********************************************************************
#
#  NAME: emulate_cli_clf
#
#  FUNCTION: emulate cli and clf instruction in POWER PC
#
#       emulate_cli_clf(eaddr)			rc = none
#
#  INPUT STATE:
#       r3 = eaddr within a cache line
#
#  OUTPUT STATE:
#       the dcache line containing eaddr is flushed
#	the icache line containing eaddr is invalidated 
#
#  N.B.: It is semantically correct to emulate cli by flushing dcache
#	 and invalidate icache
#**********************************************************************

        S_PROLOG(emulate_cli_clf)
	
        LTOC(r4,_system_configuration,data)
	l	r7,scfg_cattrib(r4)	
        l       r5,scfg_dcline(r4) 	# r5 = dcache line size,
        l       r6,scfg_dcb(r4) 	# r6 = dcache block size,
        l       r8,scfg_icline(r4) 	# r8 = icache line size,
        l       r9,scfg_icb(r4) 	# r9 = icache block size,
	rlinm	r7,r7,31,0x00000001	# r7 = 1 if combined cache
	cntlz	r4,r6			# assume dcache blk size is power of 2
	neg	r11,r5			# mask for starting dcache line addr 
	cmpi	cr1,r7,1		# check if combined cache
	sfi	r10,r4,31
	and	r12,r3,r11      	# r12 = address for first dcache block
	sr	r4,r5,r10		# r4 = dcache blocks/dcache line

flush_dcache_loop:
	dcbf	r0,r12  		# flush a dcache block
	ai.	r4,r4,-1		# decrement loop count
	a	r12,r12,r6		# address for the next dcache block 
	bne	flush_dcache_loop	# more to flush

	beqr	cr1			# return if combined cache

	# invalidate icache		
	cntlz	r4,r9			# assume icache blk size is power of 2
	neg	r11,r8			# mask for starting icache line addr
	sfi	r10,r4,31
	and	r12,r3,r11		# r12 = address for first icache block
	sr	r4,r8,r10		# r4 = icache blocks/icache line

inv_icache_loop:
	icbi	r0,r12			# invalidate an icache block
	ai.	r4,r4,-1		# decrement loop count
	a	r12,r12,r9		# address for the next icache block
	bne	inv_icache_loop		# more
	S_EPILOG


#**********************************************************************
#
#  NAME: emulate_dclst
#
#  FUNCTION: emulate dclst instruction in POWER PC
#
#       emulate_dclst(eaddr)			rc = none
#
#  INPUT STATE:
#       r3 = eaddr with a cache line
#
#  OUTPUT STATE:
#       the dcache line containing eaddr is stored to main memory
#
#**********************************************************************

        S_PROLOG(emulate_dclst)

        LTOC(r4,_system_configuration,data)
        l       r6,scfg_dcb(r4) 	# r6 = dcache block size,
        l       r5,scfg_dcline(r4) 	# r5 = dcache line size,
	cntlz	r7,r6			# assume dcache blk size is power of 2
	neg	r8,r5			# mask for starting dcache line addr
	sfi	r9,r7,31
	and	r10,r3,r8		# r10 = address for first dcache block
	sr	r11,r5,r9		# r11 = dcache blocks/dcache line

store_dcache_loop:
	dcbst   r0,r10   		# store a dcache block
	ai.	r11,r11,-1		# decrement loop count
	a	r10,r10,r6		# address for the next dcache block 
	bne	store_dcache_loop	# more to flush

	S_EPILOG


#**********************************************************************
#
#  NAME: emulate_clcs
#
#  FUNCTION: emulate clcs instruction in POWER PC
#
#       emulate_clcs(r3)			
#
#  INPUT STATE:
#       r3 = 12		compute instruction cache line size
#	     13		compute data cache line size
#	     14		compute minimum line size
#	     15		compute maximum line size
#
#  OUTPUT STATE:
#	r3 = cache line size selected by input
#
#**********************************************************************
	.set	ICLINE,12
	.set	DCLINE,13
	.set	MINLINE,14
	.set	MAXLINE,15

        S_PROLOG(emulate_clcs)

        LTOC(r4,_system_configuration,data)
	cmpi	cr1,r3,DCLINE		# check if dcache line size
	cmpi	cr0,r3,ICLINE		# check if icache line size
        l       r5,scfg_dcline(r4) 	# r5 = dcache line size
        l       r6,scfg_icline(r4) 	# r6 = icache line size
	bne	cr1,clcs0
	mr	r3,r5			# r3 = dcache line size
	br						
clcs0:
	bne	cr0,clcs1
	mr	r3,r6			# r3 = icache line size
	br						
clcs1:
	sf.	r7,r5,r6		# r7 = r6 - r5
	a	r4,r5,r6		# r4 = r6 + r5
	bge	cr0,clcs2
	neg	r7,r7			# r7 = |r6 - r5|
clcs2:
	cmpi	cr6,r3,MINLINE		# check if minimum line size
	cmpi	cr7,r3,MAXLINE		# check if maximum line size
	a	r8,r7,r4		# r8 = 2 * max cache line size 
	sf	r9,r7,r4		# r9 = 2 * min cache line size
	bne	cr6,clcs3
	sri	r3,r9,1			# r3 = min cache line size
	br
clcs3:
	bne	cr7,clcs4
	sri	r3,r8,1			# r3 = max cache line size
	br
clcs4:
	lil	r3,0			# r3 = 0 for undefined instruction
	S_EPILOG


        .toc
        TOCE(_system_configuration,data)

	include(INCLML/systemcfg.m4)
