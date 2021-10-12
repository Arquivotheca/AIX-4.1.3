static char sccsid[] = "@(#)89  1.6  src/bos/kernel/db/POWER/dbppd.c, sysdb, bos411, 9435B411a 8/30/94 15:16:39";

/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS:	disp_ppda
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#include <sys/ppda.h>
#ifndef _H_M_INTR
#include <sys/m_intr.h>
#endif /* _H_M_INTR */

/*
 * NAME: disp_ppda
 *
 * FUNCTION: this routine displays the ppda structure
 *	     indexed by the parameter.
 *	
 *
 * RETURN VALUE DESCRIPTION: None
 */

disp_ppda(target_cpu)
int target_cpu;
{
	int i;
	struct ppda *ppda_p;

	ppda_p = &ppda[target_cpu] ;
	printf("\nPer Processor %3d Data Area [%08x]\n",target_cpu, ppda_p);
	printf("csa......................%08x\n", ppda_p->_csa);
	printf("mstack...................%08x\n", ppda_p->mstack);
	printf("fpowner..................%08x\n", ppda_p->fpowner);
#ifdef _THREADS
	printf("curthread................%08x\n", ppda_p->_curthread);
#else /* _THREADS */
	printf("curproc..................%08x\n", ppda_p->_curproc);
#endif /* _THREADS */
	printf("r0.......................%08x\n", ppda_p->save[0]);
	printf("r1.......................%08x\n", ppda_p->save[1]);
	printf("r2.......................%08x\n", ppda_p->save[2]);
	printf("r15......................%08x\n", ppda_p->save[3]);
	printf("sr0......................%08x\n", ppda_p->save[4]);
	printf("sr2......................%08x\n", ppda_p->save[5]);
	printf("iar......................%08x\n", ppda_p->save[6]);
	printf("msr......................%08x\n", ppda_p->save[7]);
	printf("intr.....................%08x\n", ppda_p->intr);
	printf("i_softis.................%08x\n", ppda_p->i_softis);
	printf("i_softpri................%08x\n", ppda_p->i_softpri);
	for (i=0; i < INTTIMER; i++)
	   printf("i_prilvl[%1d]................%08x\n",i,ppda_p->i_prilvl[i]);
	printf("dar......................%08x\n", ppda_p->dar);
	printf("dsisr....................%08x\n", ppda_p->dsisr);
	printf("dsi_flag.................%08x\n", ppda_p->dsi_flag);
	if (debpg() == FALSE)
		return(0);
#ifdef _POWER_MP
	printf("cpuid....................%08x\n", ppda_p->cpuid);
	printf("mfrr_pend................%08x\n", ppda_p->mfrr_pend);
	printf("mpc_pend.................%08x\n", ppda_p->mpc_pend);
#endif /* _POWER_MP */
	printf("iodonelist...............%08x\n", ppda_p->iodonelist);
	for (i=0; i<8; i++)
		printf("dssave[%1d]................%08x\n", i, ppda_p->dssave[i]);
	for ( i=0 ; i <= (INTOFFL3-INTOFFL0) ; i++ )
		printf("schedtail[%d].............%08x\n", i, ppda_p->schedtail[i]);
	if (ppda_p->alsave[0]) {
		if (debpg() == FALSE)
			return(0);
		printf("match....................%08x\n", (ppda_p->alsave[0] >> 12) & 0xff);
		printf("rt.......................%08x\n", (ppda_p->alsave[0] >> 8) & 0xff);
		printf("ra.......................%08x\n", (ppda_p->alsave[0] >> 4) & 0xff);
		printf("rb..................... .%08x\n", ppda_p->alsave[0] & 0xff);
		printf("work1....................%08x\n", ppda_p->alsave[1]);
		printf("work2....................%08x\n", ppda_p->alsave[2]);
		printf("work3....................%08x\n", ppda_p->alsave[3]);
		printf("r25......................%08x\n", ppda_p->alsave[4]);
		printf("r26......................%08x\n", ppda_p->alsave[5]);
		printf("r27......................%08x\n", ppda_p->alsave[6]);
		printf("r28......................%08x\n", ppda_p->alsave[7]);
		printf("r29......................%08x\n", ppda_p->alsave[8]);
		printf("r30......................%08x\n", ppda_p->alsave[9]);
		printf("r31......................%08x\n", ppda_p->alsave[10]);
		printf("srr0.....................%08x\n", ppda_p->alsave[11]);
		printf("srr1.....................%08x\n", ppda_p->alsave[12]);
		printf("lr.......................%08x\n", ppda_p->alsave[13]);
		printf("cr.......................%08x\n", ppda_p->alsave[14]);
		printf("xer......................%08x\n", ppda_p->alsave[15]);
	}
	if (debpg() == FALSE)
		return(0);
	
	printf("TIMER....................");
	printf("\nt_free...................%08x", ppda_p->ppda_timer.t_free);
	printf("\nt_active.................%08x", ppda_p->ppda_timer.t_active);
	printf("\nt_freecnt................%08x", ppda_p->ppda_timer.t_freecnt);
	printf("\ntrb_called...............%08x", ppda_p->ppda_timer.trb_called);
	printf("\nsystimer.................%08x", ppda_p->ppda_timer.systimer);
	printf("\nticks_its................%08x", ppda_p->ppda_timer.ticks_its);
	printf("\nref_time.tv_sec..........%08x", ppda_p->ppda_timer.ref_time.tv_sec);
	printf("\nref_time.tv_nsec.........%08x", ppda_p->ppda_timer.ref_time.tv_nsec);
	printf("\ntime_delta...............%08x", ppda_p->ppda_timer.time_delta);
	printf("\ntime_adjusted............%08x", ppda_p->ppda_timer.time_adjusted);
	printf("\nwtimer.next..............%08x", ppda_p->ppda_timer.wtimer.next);
	printf("\nwtimer.prev..............%08x", ppda_p->ppda_timer.wtimer.prev);
	printf("\nwtimer.func..............%08x", ppda_p->ppda_timer.wtimer.func);
	printf("\nwtimer.count.............%08x", ppda_p->ppda_timer.wtimer.count);
	printf("\nwtimer.restart...........%08x", ppda_p->ppda_timer.wtimer.restart);
	printf("\nw_called.................%08x\n", ppda_p->ppda_timer.w_called);

	if (debpg() == FALSE)
		return(0);

	/* VMM Stuff */
	printf("\nVMFLAGS");
	printf("\nsio......................%02x",ppda_p->ppda_sio);
 	printf("\nreservation..............%02x",ppda_p->ppda_reservation);
	printf("\nhint.....................%02x",ppda_p->ppda_hint);
	printf("\nstackfix.................%02x",ppda_p->stackfix);
	printf("\nlru......................%02x",ppda_p->lru);
	printf("\nno_vwait.................%08x",ppda_p->no_vwait);
	printf("\nppda_qio.................%08x",ppda_p->ppda_qio);

	printf("\nscoreboard[0-3]..........%08x  %08x  %08x  %08x",
		ppda_p->scoreboard[0], 
		ppda_p->scoreboard[1], 
		ppda_p->scoreboard[2], 
		ppda_p->scoreboard[3]); 
	printf("\nscoreboard[4-7]..........%08x  %08x  %08x  %08x\n",
		ppda_p->scoreboard[4], 
		ppda_p->scoreboard[5], 
		ppda_p->scoreboard[6], 
		ppda_p->scoreboard[7]); 

}
