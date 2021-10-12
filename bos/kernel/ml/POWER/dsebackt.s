# @(#)98	1.5  src/bos/kernel/ml/POWER/dsebackt.s, sysml, bos411, 9428A410j 5/23/94 19:10:53
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: dse_backt, dse_ex_handler
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
undefine(`_POWER_RS1')
undefine(`_POWER_RSC')
undefine(`_POWER_RS2')
undefine(`_POWER_RS')

		.file	 "dsebackt.s"
		.machine "ppc"

include(macros.m4)

#****************************************************************************
#
# NAME:	dsebackt
#
# FUNCTION:
#	This is the backtrack routine for 601 DSE handling scheme.
#	The process level mstsave is set up in undo_601_dse() before fetching
#	the failing instruction such that this routine will be entered
#	in case a page fault occurs in fetching the instruction.
# 	This routine is entered in the process level with the faulting 
#	thread's context except 
# 	(1) interrupt disabled, privileged mode, I/D translation on
# 	(2) R/W access to kernel segment
#
#	dsebackt()
#       {
#		register struct ppda    *ppdp;
#		register struct mstsave *proc_mstp, *new_mstp;
#		register struct thread  *threadp;
#		register struct uthread *uthreadp;
#		register int	        sr2val, segno, instr;
#		register int	        oldiar, oldmsr, olddar;
#
#		ppdp = my_ppda();               /* use r15 */
#				
#		/* get kernel addressability
#		 */
#		mtsr(14, g_kxsrval);
#		sr2val = mfsr(2) & !(Ks|Kp);
#		mtsr(2,	sr2val);		/* clears the K bits */
#
#		/* get a new mst save area
#		 */
#               proc_mstp = ppdp->ppda_csa;     /* use r14 */
#		new_mstp = ppdp->ppda_mstack;   /* use r13 */
#		ppdp->ppda_csa = new_mstp;
#		ppdp->ppda_mstack = new_mstp - framesize;
#		initialize the new mstsave;
#
#		/* set up TOC and stack pointer
#		 */
#		r1 = (char *)new_mstp - stkmin;
#		r2 = g_toc;
#
#		/* increment dse_backt_count
#		 */
#		dse_backt_count++;
#
#		/* get address of current thread structure and
#		 * its uthread structure
#		 */
#		threadp = curthread;
#		uthreadp = threadp->t_uaddress.uthreadp;
#
#	        oldiar = uthreadp->ut_scsave[0];
#		oldmsr = uthreadp->ut_scsave[1];
#		olddar = uthreadp->ut_scsave[5];
#		
#		/************************************************************
#		 * Handle the cases of idempotent I/O exceptions (graphics)
#		 ************************************************************
#		 */
#		if (uthreadp->ut_scsave[2] == PIO_EXCP_INV_OP ||
#		    uthreadp->ut_scsave[2] == PIO_EXCP_PROT)
#		{
#	            /* set up interrupt priority INTMAX + backtrack flag
#		     * in the new mstsave
#		     */
#		    new_mstp->mstintpri = (INTMAX << 8 | backtrack flag);
#
#	            /* set up an exception handler in the process level mst
#		     */
#		    proc_mstp->excbranch = dse_ex_handler;
#
#		    /* make the failing instruction addressible using the 
#		     * current segment content in u.u_adspace
#                    */
#		    segno  = oldiar >> SEGSHIFT;
#		    mtsrin(U.U_adspace.srval[segno], oldiar);
#	
#		    /* fetch the failing instruction (may page fault again)
#                    */
#		    instr = ld(oldiar);
#
#		    /* make sure we have R/W access to process private segment
#		     */
#		    if (segno == 2)
#		    {		
#		        mtsr(2, sr2val);
#		    }
#
#		    /* clear the exception handler
#		     */
#		    proc_mstp->excbranch = 0;
#
#		    /* call backout_safe()
#		     */
#		    call backout_safe(proc_mstp, olddar, instr);
#
#		    /* free the lst interrupt level mst
#                    */
#		    ppdp->ppda_csa = proc_mstp;
#		    ppdp->ppda_mstack = new_mstp;
#		 
#		    /* restore iar, msr. and sr0 in the process level mst
#		     */
#		    proc_mstp->iar = oldiar;
#		    proc_mstp->msr = oldmsr;
#		    proc_mstp->sr[0] = U.U_adspace.srval[0];
#
#		    /* decrement signal suspend count of the current thread
#                    */
#		    (threadp->t_suspend)--;
#
#		    /* resume the failing instruction
#		     */
#		    branch to resume_pc();
#	        }
#
#		/************************************************************
#		 * Handle the cases of non-idempotent I/O exceptions
#		 ************************************************************
#		 */
#	        /* set up interrupt priority INTMAX in the new mstsave
#		 */
#		new_mstp->mstintpri = INTMAX << 8;
#
#		/* restore the original except[] saved by io_exception()
#		 * after DSE interrupt occurs (it is bashed in dsis_flih()
#		 * in case of a backtrack data storage interrupt)
#		 */
#	 	proc_mstp->except[0] = uthreadp->ut_scsave[2]; /* dsier    */
#		proc_mstp->except[1] = uthreadp->ut_scsave[3]; /* dsisr    */
#	 	proc_mstp->except[2] = uthreadp->ut_scsave[4]; /* old sreg */  
#	 	proc_mstp->except[3] = olddar;
#	 	proc_mstp->except[4] = 0;
# 
#		/* restore iar, msr, and sr0 in the process level mst
#		 */
#		proc_mstp->iar = oldiar;
#		proc_mstp->msr = oldmsr;
#		proc_mstp->sr[0] = U.U_adspace.srval[0];
#		    
#		/* decrement signal suspend count of the current thread
#                */
#		(threadp->t_suspend)--;
#
#		/* clear the exception handler in process level mstsave
#		 * It was set up in undo_601_dse().
#		 */
#               proc_mstp->excbranch = 0;
#
#		/* enable interrupt to INTPAGER
#		 */
#		call i_enable(INTPAGER);
#
#		call p_slih(proc_mstp, olddar, EXCEPT_IO, threadp);
# 
#		/* NEW_MST(r13), INTR_MST(r14), PPDA_ADDR(r15) are set up
#		 * correctly.
#		 * finish_interrupt() will disable interrupts. 
#		 */	
#		branch to finish_interrupt();
#       }    	
#
#****************************************************************************

	.csect  DATA(dse_backt[PR])	
	.globl  DATA(dse_backt[PR])

	.using	low, r0
        .set    PROC_MST, r14           # process level mstsave
	#       .set    NEW_MST, r13  
	#       .set    INTR_MST, r14
        #	.set    PPDA_ADDR, r15       

	GET_PPDA(unused, PPDA_ADDR)	# load address of my ppda

	# set up R/W access to kernel extention seg and process private seg

	mfsr    r16, sr2                # get process private seg
        l       r3, DATA(g_kxsrval)     # Kernel extension segment reg value
        l       PROC_MST, ppda_csa(PPDA_ADDR)  # process level mst 
        l       NEW_MST, ppda_mstack(PPDA_ADDR) # new mst
	rlinm   r16, r16, 0, ~SR_KsKp   # r16 = process private seg with 
					#       "k" bit(s) cleared 
        mtsr    sr14, r3                # load kernel extension segment value
	mtsr    sr2, r16                # load new process private seg value
	
	# 601 does not need an isync before/after mtsr

	# get a new mst save area
        lil     r0, 0
        cal     r3, -framesize(NEW_MST) 
        st      NEW_MST, ppda_csa(PPDA_ADDR) # current mstsave pointer
        st      r3, ppda_mstack(PPDA_ADDR)   # next available mstsave pointer
        liu     r4, NEW_ADSPACE         # get new address space allocation mask
        st      r0, mstkjmpbuf(NEW_MST) # clear jump buffer pointer
        st      r0, excbranch(NEW_MST)  # clear jump buffer pointer
        st      r4, mstsralloc(NEW_MST) # set allocation vector
	CSA_UPDATED(unused, unused, NEW_MST)

	# set up stack pointer and TOC

        cal     r1, -stkmin(NEW_MST)    # point to a real stack frame
        l       r2, DATA(g_toc)         # load kernel toc

	# increment dse_backt_count
	
	LTOC(r3,dse_backt_count,data)
	l	r4, 0(r3)
	ai	r4, r4, 0x01
	st	r4, 0(r3)
	
        l       r17, ppda_curthread(PPDA_ADDR) # r17 = current thread pointer
	l	r18,t_uthreadp(r17) 	# r18 = pointer to uthread
	l       r19, t_userp(r17)       # r19 = user structure pointer 
	cal	r20, u_adspace_sr(r19)  # r20 = pointer to u.u_adspace

        # r26 = saved iar, r27 = saved msr, r28 - r30 = saved except[0-2]
        # r28 = saved dsier, r29 = saved dsisr, r30 = saved sreg,
	# r31 = saved DAR
	lm	r26,ut_scsave(r18)       
			                 
	sri	r25,r28,28
	cmpli	cr0,r25,PIO_EXCP_INV_OP>28
	cmpli	cr1,r25,PIO_EXCP_PROT>28
	cror	cr0*4+eq,cr0*4+eq,cr1*4+eq
	bne	cr0,non_idem_io_except
	
	# set backtrack flag + INTMAX interrupt priority in the new mstsave

	lil     r5, INTMAX*256+BKTPFAULT # INTMAX + backtrack flag
	LTOC(r3,dse_ex_handler,data)
        sth     r5, mstintpri(NEW_MST) 

	# set up an exception handler in the process level mstsave

	st	r3,excbranch(PROC_MST)

	# load sr[iar >> 28] with the current segment content from u.u_adspace

	rlinm	r6, r26, 4, 0x0000000F  # r6 = segment number of iar
	sli	r7, r6, 2               # r7 = segment number of iar * 4
	mr	r3, PROC_MST	        # r3 = process level mstsave ptr
	lx	r8, r20, r7             # r8 = current segment content
	cmpli	cr0, r6, 0x2            # check if instr in segment 2
	mtsrin	r8, r26                 # load the segment register      
	mr	r4,r31		        # r4 = saved DAR         

	# fetch the instruction. It may page fault again

	l	r5, 0(r26)              # r5 = fetched instruction     

	bne	cr0, exclr
	mtsr    sr2, r16                # load kernel process private seg	

exclr:	st	r0, excbranch(PROC_MST) # clear exception handler

	# backout changes in addressing registers caused by the DSE interrupt

	.extern ENTRY(backout_safe)      
	bl	ENTRY(backout_safe)

	# free the new mstsave

        st      NEW_MST, ppda_mstack(PPDA_ADDR)  # next available mstsave ptr
        st      PROC_MST, ppda_csa(PPDA_ADDR)    # current mstsave ptr
	CSA_UPDATED(unused, unused, PROC_MST)

	lha	r3, t_suspend(r17)
	l	r4,0(r20)
	st	r26, mstiar(PROC_MST)   # restore iar
	st	r27, mstmsr(PROC_MST)   # restore msr
	ai	r3, r3, -1
	st	r4,mstsr(PROC_MST)      # restore sr0       

	# decrement the signal suspend count (incremented in undo_601_dse())

	sth	r3, t_suspend(r17)

	# resume the failing instruction

	.extern ENTRY(resume_pc)
	b	ENTRY(resume_pc)

non_idem_io_except:
        # set up interrupt priority INTMAX in the new mstsave

	lil     r5, INTMAX*256 	        
	lil	r0,0x0
        sth     r5, mstintpri(NEW_MST) 

	# restore except[] in process level mstsave 

	stm	r28,except(PROC_MST)
	st	r0,except+16(PROC_MST)

	# restore iar, msr, and sr0 in process level mstsave

	lha	r3, t_suspend(r17)
	l	r4,0(r20)
	st	r26, mstiar(PROC_MST)   # restore iar
	st	r27, mstmsr(PROC_MST)   # restore msr
	ai	r3, r3, -1
	st	r4,mstsr(PROC_MST)      # restore sr0       

	# decrement the signal suspend count (incremented in undo_601_dse())

	sth	r3, t_suspend(r17)

	# clear the exception handler in process level mstsave

	st	r0, excbranch(PROC_MST)

	# enable interrupt priority to INTPAGER

	lil	r3, INTPAGER
	.extern ENTRY(i_enable)
	bl	ENTRY(i_enable)	

	# call p_slih()

	mr	r3, PROC_MST
	mr	r4, r31
	lil	r5, EXCEPT_IO	
	mr	r6, r17
	.extern ENTRY(p_slih)
	bl	ENTRY(p_slih)	
	
	# branch to finish_interrupt()

	.extern ENTRY(finish_interrupt)
	b	ENTRY(finish_interrupt)

#****************************************************************************
#
# NAME:	dse_ex_handler
#
# FUNCTION:
#	This is the exception handler for 601 DSE handling scheme.
#	The process level mstsave is set up in undo_601_dse() before fetching
#	the failing instruction such that this routine will be entered
#	in case a page fault occurs in fetching the instruction and
#	the page fault results in an exception (e.g. I/O exception).
# 	This routine is entered in the process level with the faulting 
#	thread's context except 
# 	(1) interrupt disabled, privileged mode, I/D translation on
# 	(2) R/W access to kernel segment
#
#	
#	dse_ex_handler()
#	{
#		register struct ppda    *ppdp;
#		register int	        sr2val;
#		register struct mstsave *proc_mstp, *new_mstp;
#		register struct thread  *threadp;
#		register struct uthread *uthreadp;
#
#		ppdp = my_ppda();
#		
#		/* get kernel addressability
#		 */
#		mtsr(14, g_kxsrval);
#		sr2val = mfsr(2) & !(Ks|Kp);
#		mtsr(2,	sr2val);		/* clears the K bits */
#
#		/* get a new mst save area
#		 */
#               proc_mstp = ppdp->ppda_csa;
#		new_mstp = ppdp->ppda_mstack;
#		ppdp->ppda_csa = new_mstp;
#		ppdp->ppda_mstack = new_mstp - framesize;
#		initialize the new mstsave;
#
#               /* set up interrupt priority INTMAX in the new mstsave
#		 */
#		new_mstp->mstintpri = INTMAX << 8;
#
#		/* set up TOC and stack pointer
#		 */
#		r1 = (char *)new_mstp - stkmin;
#		r2 = g_toc;
#		
#		/* increment dse_ex_count
#		 */
#		dse_ex_count++;
#
#		threadp = curthread;
#		uthreadp = threadp->t_uaddress.uthreadp;
#
#		/* send a SIGSEGV signal to the current thread
#		 */
#		call kthread_kill(-1, SIGSEGV);
#
#		/* free the lst interrupt level mst
#                */
#		ppdp->ppda_csa = proc_mstp;
#		ppdp->ppda_mstack = new_mstp;
#		 
#		/* restore iar, msr, and sr0 in the process level mst
#		 */
#		proc_mstp->iar = uthreadp->ut_scsave[0];
#		proc_mstp->msr = uthreadp->ut_scsave[1];
#		proc_mstp->sr[0] = U.U_adspace.srval[0];
#
#		/* decrement signal suspend count of the current thread
#                */
#		(threadp->t_suspend)--;
#
#		branch to call_dispatch();
#	}
#
#****************************************************************************

DATA(dse_ex_handler):
	.globl	DATA(dse_ex_handler)

	GET_PPDA(unused, PPDA_ADDR)	# load address of my ppda

	# set up R/W access to kernel extention seg and process private seg

	mfsr    r16, sr2                # get process private seg
        l       r3, DATA(g_kxsrval)     # Kernel extension segment reg value
        l       PROC_MST, ppda_csa(PPDA_ADDR)  # process level mst 
        l       NEW_MST, ppda_mstack(PPDA_ADDR) # new mst
	rlinm   r16, r16, 0, ~SR_KsKp   # r16 = process private seg with 
					#       "k" bit(s) cleared 
        mtsr    sr14, r3                # load kernel extension segment value
	mtsr    sr2, r16                # load new process private seg value
	
	# 601 do not need an isync before/after mtsr

	# get a new mst save area
        lil     r0, 0
        cal     r3, -framesize(NEW_MST) # store address of next available
        st      NEW_MST, ppda_csa(PPDA_ADDR) # Store new current save area
        st      r3, ppda_mstack(PPDA_ADDR)   # for next interrupt
        liu     r4, NEW_ADSPACE         # get new addr space allocation mask
	lil     r5, INTMAX*256
        st      r0, mstkjmpbuf(NEW_MST) # clear jump buffer pointer
        st      r0, excbranch(NEW_MST)  # clear jump buffer pointer
        st      r4, mstsralloc(NEW_MST) # set allocation vector
	CSA_UPDATED(unused, unused, NEW_MST)

	# set interrupt priority INTMAX in the new mstsave
        sth     r5, mstintpri(NEW_MST) 

	# set up stack pointer and TOC

        cal     r1, -stkmin(NEW_MST)    # point to a real stack frame
        l       r2, DATA(g_toc)         # load kernel toc

	# increment dse_ex_count

	LTOC(r3,dse_ex_count,data)
	l	r4, 0(r3)
	ai	r4, r4, 0x01
	st	r4, 0(r3)
	
        l       r17, ppda_curthread(PPDA_ADDR) # r17 = current thread pointer

	# call thread_kill() to send a SIGSEGV signal to the current thread

	lil	r3, -1
	lil	r4, SIGSEGV
	.extern ENTRY(kthread_kill)
	bl	ENTRY(kthread_kill)	

	l	r18,t_uthreadp(r17) 	# r18 = pointer to uthread
	lha	r3, t_suspend(r17)      # get current signal suspend count

	# free the new mstsave

        st      NEW_MST, ppda_mstack(PPDA_ADDR)  # next available mstsave ptr
        st      PROC_MST, ppda_csa(PPDA_ADDR)    # current mstsave ptr
	CSA_UPDATED(unused, unused, PROC_MST)

	# restore iar and msr

	l	r26,ut_scsave(r18)
	l	r27,ut_scsave+4(r18)
	ai	r3, r3, -1
	l       r19, t_userp(r17)       # r19 = user structure pointer 
	st	r26, mstiar(PROC_MST)   # restore iar
	st	r27, mstmsr(PROC_MST)   # restore msr

	l	r4, u_adspace_sr(r19)

	# decrement the signal suspend count (incremented in undo_601_dse())

	sth	r3, t_suspend(r17)

	st	r4, mstsr(PROC_MST)     # restore sr0  

	# call the dispatcher

	.extern ENTRY(call_dispatch)
	b	ENTRY(call_dispatch)

DATA(dse_backt_count):
	.long	0x0
DATA(dse_ex_count):
	.long	0x0

	.toc
	TOCL(dse_backt_count,data)
	TOCL(dse_ex_count,data)
	TOCL(dse_ex_handler,data)
	.long	TOC[t0]

include(scrs.m4)
include(flihs.m4)
include(mstsave.m4)
include(user.m4)
include(proc.m4)
include(seg.m4)
include(i_machine.m4)
include(except.m4)
include(signal.m4)
include(low_dsect.m4)


