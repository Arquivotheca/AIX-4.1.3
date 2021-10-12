static char sccsid[] = "@(#)57	1.77  src/bos/kernel/proc/sched.c, sysproc, bos41J, 9521A_all 5/23/95 11:28:17";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: fmult
 *		loadav
 *		post_for_suspension
 *		process_proc_table
 *		sched
 *		sched_swapin
 *		sched_swapout
 *		sched_timer_post
 *		sched_zombies
 *		schedcpu
 *		schedexit
 *		schedfork
 *		schedsig
 *		swapin_process
 *		systhrash
 *		
 *   ORIGINS: 27, 83
 *
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#include <sys/types.h>
#include <sys/systm.h>
#include <sys/sysinfo.h>
#include <sys/sysconfig.h>
#include <sys/proc.h>
#include <sys/pri.h>
#include <sys/prio_calc.h>
#include <sys/sleep.h>
#include <sys/intr.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/sched.h>
#include <sys/lockl.h>
#include <sys/timer.h>
#include <sys/seg.h>
#include <sys/syspest.h>
#include <sys/vmker.h>
#include <sys/vminfo.h>
#include <sys/trchkid.h>
#include <sys/systemcfg.h>
#include <sys/atomic_op.h>
#include "swapper_data.h"

/* EXTERNAL VARIABLES */
 
extern void disown_fp();            	/* release floating point hw  */
extern void prio_requeue();		/* recalc priority on run q   */

extern struct proch *proch_anchor;   /* table of registered resource handlers */

lock_t kernel_lock=LOCK_AVAIL;
Simple_lock proc_int_lock;

struct proc *suspending_q = NULL;	/* FIFO q of swapping out proc */

/* part of the system thrash criteria, when are we thrashing */
int v_repage_hi=6;

/* part of the per process criteria, who do we suspend? */
int v_repage_proc=4;

/* process is exempt from being resuspended for x seconds after being resumed */
int v_exempt_secs=2;

/* seconds to wait before adding suspended processes back into the mix */
int v_sec_wait=1;	

/* don't go below x number of active jobs, minimum multiprocess environment */ 
int v_min_process=2;

int timeslice = 1;			/* global timeslice value */

ulong avenrun[3];		/* average number runnable    */

int sched_R = 16;
int sched_D = 16;


/* LOCAL VARIABLES */

#define SCHED_PID     		0		/* process id for scheduler   */
#define INIT_PID     		1		/* process id for init        */
#define	INIT_SYS_REPAGE_LOW	0
#define INIT_PROC_REPAGE_LOW	0
#define INIT_CPU_HI		5
#define INIT_CPU_LOW		0

static int thrashing;
#define THRASHING   (thrashing == -v_sec_wait)

#define ZSTEP (PAGESIZE / sizeof(struct proc))
static struct proc *zstart;		/* first zombie 		*/
static struct proc *zfinal;		/* last zombie			*/
static unsigned long srvals[ZSTEP];	/* pool of zombie p_adspace  	*/

/* 
 * For multiplication of fractions that are stored as integers. 
 * Not allowed to do loating point arithmetic in the kernel.
 */
#define SBITS		16

/* 
 * Fraction for digital decay to forget 90% of cpu usage in 5 secs 
 */
#define CCPU 		62339
#define CCPU1 		(FLT_MODULO - CCPU)/hz

/*
 * Constants for averages over 1, 5, and 15 minutes
 * when sampling at 5 second intervals.  Constants
 * should be divided by FLT_MODULO to reveal fraction.
 */
static ulong cexp[3] = {
	60296,	/* exp(-1/12) */
	64453,	/* exp(-1/60) */
	65173,	/* exp(-1/180) */
};

static void systhrash();		/* thrashing?                 */
static void sched_zombies();		/* zombie harvesting          */
static void process_proc_table();	/* recalc priority, zombies,  */
static void post_for_suspension();  	/* add to to_be_suspended_q   */
static int  swapin_process();		/* swapin one process	      */
static ulong fmult();			/* multiplies fractions	      */
static void sched_swapin();		/* swap in a given process    */

static struct trb *sched_trb;    	/* 1 second timer for sched() */
static int pagesteals;			/* last seconds page steals   */
static int pagespaceouts;		/* last seconds page space out*/
static int n_active;			/* number nrun + waiting io   */


/*
 * NAME:     schedcpu
 *
 * FUNCTION: Increment cpu usage for current running thread,
 *           recalculate its priority, and force a call to dispatch().
 *
 *           This function is called from 'sys_timer' every clock tick.
 *
 * EXECUTION ENVIRONMENT: 
 *           runs on timer interrupt level and cannot page fault
 *
 * NOTES:  
 *           The bounds for cpu usage are 0<= cpu usage <= T_CPU_MAX.
 *
 *           The constant T_CPU_MAX has been defined in 'thread.h'
 *           to be the the upper bound for 't_cpu' (i.e. cpu usage).
 *
 * RECOVERY OPERATION: 
 *           none
 *
 * DATA STRUCTURES:
 *           _ficd     - forces a call to dispatch() which effectively
 *                       causes roundrobin scheduling.                       
 *
 * RETURNS:  NONE
 *
 */
   
void
schedcpu()
{
	register struct ppda *myppda = PPDA;
	register struct thread *t;
	int	oldpri;

	t = myppda->_curthread;

	/* 
	 * If current thread is less than maximum allowed and 
	 * is not init process, increment time slice (t_cpu).     
	 * For all threads, recalculate priority and force call  
	 * to dispatcher, when interrupts are next enabled.
	 */
	oldpri = disable_lock(INTMAX, &proc_int_lock);
	if ((t->t_cpu < T_CPU_MAX) && (t->t_procp->p_pid != 1))
		t->t_cpu++;
#ifdef _POWER_MP
	if ((t->t_boosted) && (t->t_lockcount == 1))
		t->t_boosted = 0;
#else
	if ((t->t_boosted) && (t->t_lockcount == 0))
		t->t_boosted = 0;
#endif
	prio_requeue(t, prio_calc(t));
      	if (!t->t_boosted)
		t->t_sav_pri = t->t_pri;
	unlock_enable(oldpri, &proc_int_lock);

	/*
	 * The number of clock ticks of cpu time the current thread 
	 * has received since the last time the scheduler ran.
	 */
	t->t_procp->p_cpticks++;
	t->t_ticks++;
	if ((t->t_policy == SCHED_OTHER) ||
	    (t->t_pri == PIDLE)          ||
	    (t->t_policy == SCHED_RR && t->t_ticks > timeslice)) {
			myppda->_ficd = 1;
	}
}


/*
 * NAME:     schedfork
 *
 * FUNCTION: suspends new processes (ie. child) when thrashing. 
 *
 * EXECUTION ENVIRONMENT: 
 *	     Cannot Page Fault.  Interrupts disabled by caller.
 *
 * RECOVERY OPERATION: 
 *           none
 *
 * RETURNS:  NONE
 */

void
schedfork(struct proc *child)
{
	if (THRASHING)
		child->p_int |= SGETOUT;
}

Simple_lock proc_tbl_lock;

/*
 * NAME:     schedexit
 *
 * FUNCTION: removes current job from the to be suspended q
 *	     and adds a new job back into the mix if the 
 *	     current one is exiting.  This code is called
 *	     from exit.c.
 *
 * EXECUTION ENVIRONMENT: 
 *	     Cannot Page Fault.  Interrupts disabled by caller.
 *
 * RECOVERY OPERATION: 
 *           none
 *
 * RETURNS:  NONE
 */

void
schedexit(struct proc *p)
{
	p->p_int &= ~SGETOUT;
}


/*
 * NAME:     schedsig
 *
 * FUNCTION: Swappin the designated process.  It has received
 *           a signal and needs to be restarted.
 *
 * EXECUTION ENVIRONMENT:
 *           Caller of this routine has disabled to INTMAX. 
 *           Cannot page fault.  
 *
 * NOTES:
 *           This function is called from pidsig, tidsig, stop, exit.
 *
 * RECOVERY OPERATION:
 *           none
 *
 * RETURNS:  NONE
 *
 */

void
schedsig(struct proc *p)
{
	/*
	 * In order to avoid race conditions between delivering a signal and
	 * swapping out when a process is multithreaded, we cancel any pending
	 * request to swap.
	 */
	p->p_int &= ~SGETOUT;

	/*
	 * We need to swap the process back in only if at least one of its
	 * threads had already been swapped out.
	 */
	if (p->p_sched_pri != PIDLE)
		sched_swapin(p);
}

Simple_lock uprintf_lock;

/*
 * NAME:     sched_timer_post
 *
 * FUNCTION: post process 0 (swapper process) after 1 second has elapsed,
 *           and restart the 1 second timer.
 *
 * EXECUTION ENVIRONMENT: 
 *           runs on timer interrupt level 
 *
 * NOTES:  
 *           The function sched() starts a 1 second timer and goes to sleep.
 *           When 1 second has elapsed, sched_timer_post() receives control and
 *           posts sched(). 
 *           
 *
 * RECOVERY OPERATION: 
 *           none
 *
 * RETURNS:  NONE
 *
 *
 */
   
static void 
sched_timer_post(struct trb *trb)
{
    	/* 
     	 * reset some trb fields back to zero
     	 */
    	sched_trb->flags = 0;
    	sched_trb->timeout.it_value.tv_sec = 0;
    	sched_trb->timeout.it_value.tv_nsec = 0;
    	/* 
     	 * now set up trb for sched()
     	 */
    	sched_trb->flags = T_INCINTERVAL;
    	sched_trb->timeout.it_value.tv_sec = 1;
    	/*
     	 * restart timer
     	 */
    	tstart(sched_trb);

    	et_post(sched_event_timer,sched_trb->id);
}


/*
 * NAME:     sched
 *
 * FUNCTION: Decay cpu usage for all active threads, re-calculate
 *           their priority, perform process suspension when thrashing, 
 *           and maintain some sysinfo statistics.
 *
 *           This function runs every second.
 *
 * NOTES:
 *           1. Code assumes that maxproc never decreases
 *
 * EXECUTION ENVIRONMENT:
 *           cannot page fault after disabling interrupts.
 *
 * DATA STRUCTURES:
 *           process table (proc[0] thru max_proc - 1).
 *           runrun - forces a call to to dispatch().
 *
 */

void
sched()
{
	struct timestruc_t ct;
        struct proc *p;       
        struct user *uptr;     
	int ipri, i, imax;

	/* 
	 * Turn off load control if we've got 128M or more to work with.
	 * See systhrash() below.
	 */
	if (vmker.nrpages - vmker.badpages >= 128*1024*1024/PAGESIZE) {
		v_repage_hi = 0;
	}

        /* Update the start times for the early processes */
	simple_lock(&proc_tbl_lock);
        for (p = &proc[0]; p < max_proc; p++) {
                if ((p->p_stat != SZOMB && p->p_stat != SNONE) &&
                    (p->p_adspace != NULLSEGVAL))
                {
                        uptr = (struct user *)vm_att(p->p_adspace, &U);
                        uptr->U_start = time;
                        vm_det(uptr);
                }
        }
	simple_unlock(&proc_tbl_lock);

   	/* set up a one second timer */
   	sched_trb = talloc();                       /* get storage for a trb*/

   	sched_trb->flags = T_INCINTERVAL;

   	/* sched_timer_post() will get control when the 1 second is up */ 
   	sched_trb->func = (void (*)(void *)) sched_timer_post; 
	sched_trb->id = curthread->t_tid; 
   	sched_trb->eventlist = EVENT_NULL; 
   	sched_trb->func_data = '\0';

   	/* call sched_timer_post() at a timer interrupt level */
   	sched_trb->ipri = INTTIMER;          
   	sched_trb->timeout.it_value.tv_sec = 1;     /* 1 second interval    */
   	sched_trb->timeout.it_value.tv_nsec = 0;    /* zero nanoseconds     */

   	tstart(sched_trb);                          	/* start timer */

   	/* from here on we execute this loop forever. */
   	for(;;) { 

		/* wait on timer */
		et_wait( sched_event_timer, sched_event_timer, EVENT_SIGRET);

		/* Perform special work if process 1 dies */ 
		if( proc[1].p_stat == SZOMB ) {
			assert (kwaitpid(NULL,1,0,NULL) == 1);
			newroot();
		}


		/* 
		 * Sets thrashing variable, indicating if we are currently
		 * thrashing, or the number of seconds since we last were.
		 */ 
		systhrash();	

		ipri = disable_lock(INTMAX, &proc_base_lock);
#ifdef _POWER_MP
		simple_lock(&proc_int_lock);
#endif

		/* 
	 	 * Recalculates the priority of runnable processes, 
		 * rebuilds the to_be_suspended_q, determines the 
		 * current multiprogramming level, and identifies
		 * the first zombie process.  
		 */
		process_proc_table();

		if (thrashing > 0)	/* wait period over after thrashing */
		{
			/*
			 * Add jobs back into the mix in a progressively
			 * more aggressive fashion.  Adds them to the 
			 * run queue.
		 	 */
			imax = MAX((thrashing+4)/5, n_active/20);
			for (i = 0; i < imax; i++)
			{
				if (!swapin_process())
					break;
			}
		}
		else
		{
			/*
			 * Maintain minimum multiprogramming level.
			 */
			 for (i=n_active+1; i<=v_min_process; i++)
			 {
				if (!swapin_process())
					break;
			 }

			/*
			 * This reference to the tod can be unsafe, 
			 * since it is a read reference and we don't 
			 * care about accuracy.  This logic is essentially 
			 * a release valve to prevent starvation. Swap in 
			 * a process every ten seconds even when the system 
			 * is thrashing. 
			 */
			if (tod.tv_sec % 10 == 0)
				(void)swapin_process();
		}

#ifdef _POWER_MP
		simple_unlock(&proc_int_lock);
#endif

		TRCHKL5T(HKWD_KERN_SCHED, thrashing, v_repage_hi, 
			vmker.sysrepage, vmminfo.pgexct, vmminfo.pgsteals );

		TRCHKL4T(HKWD_KERN_SCHED_STAT1, vmminfo.pageins, 
			vmminfo.pageouts, vmminfo.pgspgins, vmminfo.pgspgouts );

		vmker.sysrepage = 0;

		unlock_enable(ipri, &proc_base_lock);

		/* 
		 * Do this last since this processing cannot be done with 
		 * interrupts disabled. 
		 */
		if (zstart != NULL)
			sched_zombies();

    	}
}

Simple_lock tod_lock;

/*
 * NAME:     systhrash
 *
 * FUNCTION: Determines if we are thrashing.
 *	     	
 * EXECUTION ENVIRONMENT: 
 *           Can page fault.
 *	     	
 * DATA STRUCTURES:
 *	Sets the variable thrashing to
 *           -1  if thrashing 
 *	      0  just ended thrashing.
 *	     +N  seconds since we ended thrashing.
 *	     	
 * RETURNS:
 *	none.
 */

static
void
systhrash()
{									
	pagesteals = vmminfo.pgsteals - pagesteals;
	pagespaceouts = vmminfo.pgspgouts - pagespaceouts;

	/*
	 * This criteria is very sensitive.  Pagespaceouts/pagesteals 
	 * rapidly goes from zero to fifty percent and vice versa. For 
	 * example, thrashing begins with one pageout and 2 steals
	 * assuming v_repage_hi = 3, however it also ends quickly.
	 */
	if ((pagesteals) && (pagespaceouts * v_repage_hi > pagesteals))
		thrashing = -v_sec_wait;
	else
		if ( thrashing < 100 )
			thrashing++; 

	pagesteals = vmminfo.pgsteals;
	pagespaceouts = vmminfo.pgspgouts;

} 


/*
 * NAME:     swapin_process
 *
 * FUNCTION: Add a job back into the mix.  Put on ready to run q.
 *
 * EXECUTION ENVIRONMENT:
 *           Cannot page fault.  Interrupts are disabled by caller.
 * RETURNS:
 *           0  No jobs were suspended.
 *           1  Added a job back into the mix.
 */

static
swapin_process()
{
	struct proc *p, *q;

	if ((p = suspending_q) == NULL)
		return (0);

	for(q = p->p_sched_next; q != suspending_q; q = q->p_sched_next)
		if (q->p_sched_pri < p->p_sched_pri)
			p = q;

	sched_swapin(p);

	return(1);
}


/*
 * NAME:     process_proc_table
 *
 * FUNCTION: Traverses the proc table:
 *		traverses the thread list:
 *		1) swapped	-> priority is recalculated,
 *				   p_sched_pri is updated accordingly.
 *		2) ready to run -> priority is recalculated, 
 *				   system variable sysinfo.runque is incr.
 *		3) sleeping     -> priority is recalculated.
 *		4) stopped      -> priority is recalculated.
 *		5) a zombie     -> returns flag true.
 *           Requests jobs to be suspended.
 *	     	
 * EXECUTION ENVIRONMENT: 
 *           Can page fault (but doesn't).
 *
 * DATA STRUCTURES:
 *           the proc and thread tables
 *           sysinfo data structure
 */

static 
void
process_proc_table()
{
   	register struct proc *p;        /* process block being examined     */
	register struct thread *t;	/* thread block being examined	    */
	int th_event=0;		  	/* number of threads waiting event  */
	int th_suspended=0;		/* number of suspended threads	    */
	int th_runnable=0;		/* number of runnable threads	    */
	int th_pagein=0;		/* number of thrds waiting for page */
	int n_starting=0;		/* number of proc being created     */
	int n_really_active=0;		/* number of proc awaiting mem/cpu  */
	int really_active;
	int ncpus=_system_configuration.ncpus;
	int i;

	zstart = NULL;			/* first zombie process */
	zfinal = NULL;			/* last zombie process */

	for (p = &proc[1], i = 0; p < max_proc; p++) {

		/* skip over the wait processes */
		if ((p<&proc[2 + ncpus]) && (p>=&proc[2]))
			continue;

		/* skip if process is being created */
		if (p->p_stat == SIDL) {
			n_starting++;
			continue;
		}

		/* skip also if process is a zombie */
		if (p->p_stat == SZOMB) {
			if (zstart == NULL)
				zstart = p;
			zfinal = p;
			continue;
		}

      		/* skip if slot is unused too */
      		if (p->p_stat == SNONE)
			continue;

		i++;

		/* 
		 * p_sched_count indicates the number of seconds that a 
		 * process is exempt from being resuspended.
		 */
		if (p->p_sched_count)
			p->p_sched_count--;

		/*
		 * SJUSTBACKIN indicates that the process is a recently
		 * resumed suspended process.  It will not be resuspended
		 * if SJUSTBACKIN is set.  Each process is given a grace 
		 * period in the interests of fairness, because if the  
		 * system starts to thrash again it will be almost certainly 
		 * be resuspended since it would have an artificially 
		 * high repaging rate.
		 */
		if (p->p_int & SJUSTBACKIN) 
		{
			if (p->p_sched_count == 0)
				p->p_int &= ~SJUSTBACKIN;
		}

		if (THRASHING) {
			TRCHKL5T(HKWD_KERN_SCHED_POST, p->p_pid, p->p_repage, 
	 		 	v_repage_proc, p->p_minflt, p->p_majflt);
			post_for_suspension(p);
		}
                else if (thrashing > 0) /* wait period over after thrashing */
                        p->p_int &= ~SGETOUT;

		p->p_repage = 0;
		p->p_majfltsec = p->p_majflt;   

		/* 
		 * Calc percentage cpu time.
		 * pctcpu = CCPU * p_pctcpu + CCPU1 * p_cpticks 
		 */
		p->p_pctcpu  = fmult(p->p_pctcpu, CCPU);
		p->p_pctcpu += fmult(p->p_cpticks << SBITS, CCPU1);
		p->p_cpticks = 0;

		t = p->p_threadlist;
		do {
			really_active = FALSE;

			t->t_cpu = (t->t_cpu * sched_D)>>5;
			if (t->t_time != 255)	/* seconds active */
				t->t_time++;

			switch (t->t_state) {

			case TSSWAP:

				/*
				 * The pri cannot increase, except if nice is
				 * changed, because the thread is not running.
				 * Therefore, we only have to check for a
				 * p_sched_pri decreasing, if we take care of
				 * it when nice is changed.
				 */
				if ((t->t_pri = prio_calc(t)) < p->p_sched_pri)
					p->p_sched_pri = t->t_pri;
			        if (!t->t_boosted)
					t->t_sav_pri = t->t_pri;

				th_suspended++;
				break;

			case TSRUN:
	       
				prio_requeue(t, prio_calc(t)); 
			        if (!t->t_boosted)
					t->t_sav_pri = t->t_pri;
				th_runnable++;
				really_active = TRUE;
				break;

			case TSSLEEP:

				if (t->t_wtype == TWPGIN) {
					th_pagein++;
					really_active = TRUE;
				}
				/* falls through */

			case TSSTOP:

				if (t->t_wtype == TWEVENT) 
					th_event++;
       		       		t->t_pri = prio_calc(t);
			        if (!t->t_boosted)
					t->t_sav_pri = t->t_pri;

		        	break;

			default: 

				break;
        		}

			t = t->t_nextthread;

		} while (t != p->p_threadlist);

		if (really_active)
			n_really_active++;

                /*
                 * Need to unlock periodically to avoid device overruns.
                 * The time needed to receive 2 characters from a tty line
                 * is 2 * ((8+1)/38400 baud) = ~1/2 ms.  At 7 usecs per
                 * pass of the loop (trace enabled), we get 70 loops.
                 *
                 * Note this change is also good for MP scalability, since
                 * the proc_int_lock is used for dispatching.
                 */
                if (i >= 70) {
#ifdef _POWER_MP
			simple_unlock(&proc_int_lock);
#endif
			unlock_enable(INTBASE, &proc_base_lock);
			(void) disable_lock(INTMAX, &proc_base_lock);
#ifdef _POWER_MP
			simple_lock(&proc_int_lock);
#endif
			i = 0;
		}
	}							  /* end for */

     	/*
      	 * finished scanning process table.  Unsafe read OK, because the
	 * statistics are based on chance anyway.  The number runnable.
      	 */

	if (tod.tv_sec % 5 == 0){		   /* calc average runnable   */
		loadav(avenrun, th_runnable+th_pagein+n_starting);
	}

     	if (th_runnable){  
		sysinfo.runque += th_runnable;
         	sysinfo.runocc++;             
     	}

     	if (th_pagein + th_suspended){ 
	 	sysinfo.swpque += th_pagein + th_suspended;
         	sysinfo.swpocc++;             
     	}

	n_active = n_really_active + n_starting;

	TRCHKL5T(HKWD_KERN_SCHED_STAT, th_runnable,
			th_suspended, th_pagein, th_event, n_starting );
}

Simple_lock ptrace_lock;

/*
 * NAME:     sched_zombies
 *
 * FUNCTION:
 *	     Harvest resources for zombies.
 *
 * EXECUTION ENVIRONMENT: 
 *	     Cannot page fault
 *
 */
static
void
sched_zombies()
{
	register struct proc *p;		/* proc block examined	  */
	register struct proc *zsegend;		/* end of batch		  */
	int ipri;				/* old interrupt level	  */
	int i;					/* index into srvals	  */

	/*
	 * Harvest zombies from zstart to zfinal - first and last zombies
	 * recorded by process_proc_table.  Process this segment in ZSTEP
	 * size chunks, reacquiring the proc_tbl_lock each time.  This 
	 * minimizes the number of times that the scheduler will sleep.
	 * It is paramount that the scheduler perform its other duties
	 * in a timely fashion.  ie. decaying priorities, ... 
	 */ 
	while (zstart <= zfinal) {

	    	zsegend = zstart + ZSTEP;
		i = 0;

	    	/* The scheduler should not sleep */
	    	if (!simple_lock_try(&proc_tbl_lock))
			return;

            	for (; zstart < zsegend && zstart <= zfinal; zstart++) {

			p = zstart;

               		switch (p->p_stat) {
	  		case SZOMB:

				/*
			 	 * Remember process private segment.
			 	 */
				srvals[i++] = p->p_adspace;
                                p->p_adspace = NULLSEGVAL;

				/*
			 	 * !SLOAD signifies that the parent is not going
			 	 * to perform a wait().  It is reset in exit()
			 	 * after removing the sibling links.  This code
			 	 * cannot be placed in exit(), because we are
				 * running on the stack in the process private
			 	 * segment.
			 	*/
				if (!(p->p_flag & SLOAD) && !(p->p_flag & STRC))
				{
					/*
				 	 * Remove the proc slot of the exited
				 	 * pgrp leader.
				 	 * Remove the proc slot of p.
				 	 */
					if (p->p_pgrp)
						update_proc_slot(p);
					freeproc(p);
				}
			}

	    	}

	    	simple_unlock(&proc_tbl_lock);

		/* Free segment after releasing lock */
		for (i--; i >= 0; i--)
                	if (srvals[i] != NULLSEGVAL)
                      		vms_delete(SRTOSID(srvals[i]));
	}
}


/*
 * NAME:     post_for_suspension
 *
 * FUNCTION: 
 *	     This function checks whether a process can be suspended and
 *	     if so set the SGETOUT bit.
 *
 * EXECUTION ENVIRONMENT: 
 *	     Caller disables interrupts.
 *           Cannot page fault.
 *
 */

static
void
post_for_suspension(struct proc *p)
{
	if (  (p->p_stat != SZOMB)				 &&
	      (p->p_stat != SNONE)				 &&
	      (p->p_stat != SIDL)				 &&
	     !(p->p_flag & (SEXIT | SKPROC | SNOSWAP)) 		 &&
	      (p->p_pid != SCHED_PID)			 	 &&
	      (p->p_pid != INIT_PID)			 	 &&
	      (p->p_stat != SSWAP)				 &&
	     !(p->p_int & (SGETOUT | SJUSTBACKIN))		 &&
	      (p->p_repage > 0)					 &&
	      (p->p_majflt > p->p_majfltsec)			 && 
	      (p->p_repage*v_repage_proc >= p->p_majflt-p->p_majfltsec)
	    )
		p->p_int |= SGETOUT;
}

Simple_lock proc_base_lock;

/*
 * NAME:     sched_swapout
 *
 * FUNCTION: suspends the current thread (curthread). Add the process to the
 *		FIFO suspending_q if required.
 *	     	
 * 	     This routine is called from resume() and svcret() (via
 *	     sig_slih()).  It is assumed that the curthread is in USERMODE. 
 *	     	
 * EXECUTION ENVIRONMENT: 
 *           Cannot page fault.  Interrupts are disabled by caller. 
 *
 * DATA STRUCTURES:
 *	     suspending_q	
 * RETURNS:
 *           1 (with proc_base_lock held) if the swapout occurred
 *           0 (without proc_base_lock) otherwise
 */

int
sched_swapout()
{
	struct thread	*t;		/* thread to swap out	*/
	struct proc	*p;		/* owning process	*/

	t = curthread;
	p = t->t_procp;

	/* 
	 * We are disabled via the msr but the thread
	 * being resumed is enabled to INTBASE, since it is
	 * returning to user mode.
	 */
	ASSERT(t->t_uthreadp->ut_save.intpri == INTBASE );

#ifdef _POWER_MP
	simple_lock(&proc_base_lock);
	simple_lock(&proc_int_lock);
#endif

	/* Make sure the suspension request is still valid */
	if (!(p->p_int & SGETOUT)) {
#ifdef _POWER_MP
		simple_unlock(&proc_int_lock);
		simple_unlock(&proc_base_lock);
#endif
		return 0;
	}

	/* always leave two active process excluding the wait process */ 
	if (n_active <= v_min_process)
	{
		/*
		 * We prevent the swapout only if the process has not already
		 * begun to swap its threads.
		 */
		if (p->p_sched_pri == PIDLE) {
			p->p_int &= ~SGETOUT;
#ifdef _POWER_MP
			simple_unlock(&proc_int_lock);
			simple_unlock(&proc_base_lock);
#endif
			return 0;
		}
	}

	/* release floating point hw  */
	disown_fp(t->t_tid);                    

	/* set state flags */
	t->t_cpu	  = 0;
	t->t_time	  = 0;
	t->t_wtype	  = TWMEM;
	t->t_state	  = TSSWAP;
	p->p_suspended++;
	TRCHKL5T(HKWD_KERN_SCHED_SWAPOUT, p->p_pid, t->t_tid, p->p_suspended,
		 n_active, v_min_process);
	ASSERT(p->p_suspended <= p->p_active);

	/* No wakeup because swapping is handled transparently */
	if (p->p_suspended == p->p_active) {
		p->p_stat = SSWAP;
		n_active--;
	}

	/* add to the suspending q */
	if (p->p_sched_pri == PIDLE) {
		p->p_sched_pri = t->t_pri;
		if (suspending_q == NULL)
			suspending_q = p->p_sched_back = p->p_sched_next = p;
		else {
			p->p_sched_back = suspending_q->p_sched_back;
			p->p_sched_next = suspending_q;
			p->p_sched_back->p_sched_next = p;
			p->p_sched_next->p_sched_back = p;
		}
	}
	else if (t->t_pri < p->p_sched_pri)
		p->p_sched_pri = t->t_pri;

#ifdef _POWER_MP
	simple_unlock(&proc_int_lock);
	/* Do not release the proc_base_lock */
#endif
	return 1;
}


/*
 * NAME:     sched_swapin
 *
 * FUNCTION: swaps in a process.
 *
 *           This routine is called from schedsig() and swapin_process().
 *
 * EXECUTION ENVIRONMENT:
 *           Cannot page fault.  Interrupts are disabled by caller.
 *
 * DATA STRUCTURES:
 *           suspending_q
 */

void
sched_swapin(struct proc *p)
{
	register struct thread *t;

	ASSERT(p->p_sched_pri != PIDLE);

	TRCHKL2T(HKWD_KERN_SCHED_SWAPIN, p->p_pid, thrashing);

	if (p->p_sched_next == p)
		suspending_q = NULL;
	else {
		if (suspending_q == p)
			suspending_q = p->p_sched_next;
		p->p_sched_back->p_sched_next = p->p_sched_next;
		p->p_sched_next->p_sched_back = p->p_sched_back;
	}

	p->p_int &= ~SGETOUT;
	p->p_int |= SJUSTBACKIN; 

	p->p_sched_pri = PIDLE;
	p->p_sched_count = v_exempt_secs+1;

	t = p->p_threadlist;
	do {
		if (t->t_state == TSSWAP) {
			t->t_time = 0;
			t->t_sav_pri = t->t_pri = prio_calc(t);
			setrq(t, E_WKX_PREEMPT, RQTAIL);
			p->p_suspended--;
		}
		t = t->t_nextthread;
	} while (t != p->p_threadlist);

	ASSERT(p->p_suspended >= 0);
}

Simple_lock watchdog_lock;

/*
 * NAME: fmult()
 *
 * FUNCTION: Multiply two fractions
 *
 * EXECUTION ENVIRONMENT:
 *	Will not page fault.
 *
 * NOTE: 
 *
 *	x = y * z, 
 *
 * 	where z, y, and z are hexidecimal numbers with the upper half 
 *      word reserved for the whole number and lower half for the
 *      fraction.  Consequently, z can be translated to the proper 
 * 	fraction by the statement:
 *
 *	      double f;
 *            f = (double) x / FLT_MODUL0;
 */

static
ulong
fmult( ulong y, ulong z ) 	
{	
	ulong sum;					

	/* 
	 * Multiply fractional part of y with fraction part of z.
	 * Round up, if necessary 
	 */
	sum  = (y << SBITS >> SBITS) * (z << SBITS >> SBITS) / FLT_MODULO; 
	if ((y<<SBITS>>SBITS) * (z<<SBITS>>SBITS) % FLT_MODULO > FLT_MODULO/2) 
		sum++;

	/* Multiply fractional part of y with whole number part of z */  
	sum += ((y << SBITS >> SBITS) * (z >> SBITS)); 

	/* Multiply whole number part of y times fraction part of z */
	sum += ((y >> SBITS) * (z << SBITS >> SBITS));

	/* Multiply whole number part of y with whole number part of z */ 
	sum += (((y >> SBITS) * (z >> SBITS)) << SBITS);

	return(sum);
}
Complex_lock core_lock;


/*
 * Compute a tenex style load average of a quantity on
 * 1, 5 and 15 minute intervals.
 * 
 * Called every 5 seconds.
 * 
 */
loadav(avg, n)
	register ulong avg[];
	register int n;
{
	register int i;

	/*
	 * for (i = 0; i < 3; i++)
	 *	avg[i] = cexp[i] * avg[i] + n * (1.0 - cexp[i]);
	 */
	
    	for (i = 0; i < 3; i++)
    	{
	  	avg[i]  = fmult(cexp[i], avg[i]); 
		avg[i] += fmult(n << SBITS, FLT_MODULO - cexp[i]);
     	}

}
