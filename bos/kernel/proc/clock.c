static char sccsid[] = "@(#)45	1.104.3.5  src/bos/kernel/proc/clock.c, sysproc, bos41B, 412_41B_sync 12/2/94 17:49:14";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: addupc
 *		adj_trbs
 *		bd_psusage
 *		bd_rusage
 *		clock
 *		delay
 *		delay_end
 *		itimerdecr
 *		resched
 *		reset_decr
 *		sys_timer
 *		talloc
 *		tfork
 *		tfree
 *		tstart
 *		tstop
 *
 *   ORIGINS: 3, 27, 83
 *
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#include <sys/types.h>		/* always gotta have this one		      */
#include <sys/trcmacros.h>	/* to define trace macros		      */
#include <sys/trchkid.h>	/* to define trace hook ids 		      */
#include <sys/sleep.h>		/* to define return values from e_sleep()     */
#include <sys/syspest.h>	/* to define the assert and ASSERT macros     */
#include <sys/time.h>		/* \  					      */
#include <sys/timer.h>		/*  \  For the timer related defines	      */
#include <sys/rtc.h>		/*  /					      */
#include <sys/intr.h>		/* for the serialization defines	      */
#include <sys/mstsave.h>	/* for various asserts (called by process,etc)*/
#include <sys/low.h>		/* to get access to the current save area(csa)*/
#include <sys/proc.h>		/* to access the trb ptr in the proc structure*/
#include <sys/malloc.h>		/* for the parameters to xmalloc()	      */
#include <sys/param.h>		/* to define the HZ label, MIN macro	      */
#include <sys/user.h>		/* per-process timers are maintained in ublock*/
#include <sys/errno.h>		/* errno values to return from bad system call*/
#include <sys/sysinfo.h>	/* sysinfo data updated each clock interrupt  */
#include <sys/var.h>		/* system vars that can be changed by user    */
#include <mon.h>		/* to define the profil structures	      */
#include "ld_data.h"		/* definitions to identify overflow segment   */

#ifndef  _LONG_LONG

/*
 * Simulate 64 bit addition if the (long long) data type is not
 * supported.  This code may be removed when the build environment
 * is updated.
 */

#define ADDL64(ll,a) { \
        unsigned long t; \
        t = ll[1]; ll[1] += a; \
        if (ll[1] < (unsigned long) a || ll[1] < t) ll[0]++; \
}

#endif
/*
 * Only used by mp master or constants
 */
int resched_event = EVENT_NULL;
struct timestruc_t ref_const;  	/* 10mS constant for drift calculations */
struct intr clock_intr;		/* timer interrupt structure		*/
int disks_active;           	/* global indicating disk activity --   */
   				/*     for statistics kept in sys_timer */
/*
 * only updated by mp master, and he do a sync afterwards
 */
time_t lbolt;                  	/* number ticks since last boot         */
time_t time;                   	/* memory mapped time/secs since epock  */

/*
 * protected by tod_lock
 */   				/*     for statistics kept in sys_timer */
struct timestruc_t tod;		/* memory mapped timer structure	*/
struct tms tm;			/* date to secs conversion structure	*/

/*
 * forward declarations
 */
extern void watchdog();		/* process watchdog timers		*/
extern void kthread_kill();	/* send SIGVTALRM, SIGALRM, and SIGPROF */
extern void update_iostats();	/* update i/o statistics routine	*/
extern void scrubclock();	/* memory scrubbing code		*/
extern void schedcpu();		/* scheduler routine			*/

void reset_decr();		/* declare for forward reference	*/
void delay_end();		/* declare for forward reference	*/


/*
 * NAME:  clock
 *
 * FUNCTION:  Timer interrupt handler.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is only called on the timer interrupt level.
 *
 *      It does not page fault.
 *
 * RETURN VALUE DESCRIPTION:  INTR_SUCC
 *
 * EXTERNAL PROCEDURES CALLED:  None
 */
int
clock(struct intr *poll_addr)
{
	struct timestruc_t ct;		/* current time value */
	register struct trb *tmpptr;	/* temp ptr to walk active list	*/
	register int ipri;		/* func()'s int. priority */
	struct ppda_timer *mytimer;	/* my processor's timer struct */
	cpu_t mycpu;			/* my cpu number */

	/*
	 * As the trb queues may be handled by another processors,
	 * we do have to grab the lock to serialize.
	 */
	ASSERT(csa->prev != NULL);
	ASSERT(csa->intpri == INTTIMER);

	mycpu = CPUID;
	mytimer = &ppda[mycpu].ppda_timer;

	(void) disable_lock(INTMAX, &tod_lock);

	/*  
	 * Update the memory mapped time variables and return the
	 * current time.  Only the seconds field is updated in the
	 * global variable tm.  The date is not calculated.
	 */ 
	curtime(&ct);
	if (mycpu == MP_MASTER) {
		time = tm.secs = ct.tv_sec;
		tod = ct;
	}

	/*  Get the timer request at the head of the active list.  */
	/*  Accuire the lock first.                                */

	tmpptr = mytimer->t_active;

	while((tmpptr != NULL)  &&
	      !(ntimercmp(tmpptr->timeout.it_value, ct, >)))  {

	  	/*  
		 * The interrupt value must be set prior to the timer 
		 * being used.                    
		 */
	  	ASSERT(tmpptr->ipri != -1);

		mytimer->trb_called = tmpptr;

	  	/*
	   	 *  The timer request at the front of the active list is
	   	 *  expired (i.e. is less than or equal to the current time),
	   	 *  so it needs to be removed from the active list.
	   	 */
	  	mytimer->t_active = tmpptr->knext;

	  	/*
	   	 *  If the system wide timer were the only trb on the active
	   	 *  list, then the active list will be empty from now until
	   	 *  the system wide timer timeout routine gets called and
	   	 *  reschedules itself.  This condition needs to be checked
	   	 *  for before clearing the previous pointer of the active
	   	 *  trb.
	   	 */
	  	if(mytimer->t_active != NULL)  {
	    		mytimer->t_active->kprev = (struct trb *)NULL;
	  	}
	  
	  	/*  Clear out this trb's chain pointers.  */
	  	tmpptr->kprev = tmpptr->knext = (struct trb *)NULL;
	  
		ASSERT((tmpptr->flags & (T_ACTIVE|T_PENDING)) == 
				(T_ACTIVE|T_PENDING));

	  	/*  Mark the trb as no longer pending.  */
	  	tmpptr->flags &= (~T_PENDING);
		ASSERT(tmpptr->cpunum == mycpu);
	  
	  	/*  
	   	 * Call the expired trb's timeout routine. 
	   	 * Drop the trb lock before to avoid deadlock.
	   	 */
	  	ipri = MIN(INTTIMER, tmpptr->ipri);
	  	unlock_enable(ipri, &tod_lock);

	  	(* tmpptr->func)(tmpptr);
	  
	  	/*
	   	 * disable and accuire the lock again, and check next trb.
	   	 */
	  	(void) disable_lock(INTMAX, &tod_lock);

		ASSERT(tmpptr->cpunum == mycpu);

		/* 
		 * If the trb was not rescheduled, then clear flag.  
		 */
		if (!(tmpptr->flags & T_PENDING))
			tmpptr->flags &= ~T_ACTIVE; 

		mytimer->trb_called = NULL;

	  	/*
	   	 *  The active list must contain at least one trb
	   	 *  (the system wide timer) by this point.
	   	 */
	  	ASSERT(mytimer->t_active != NULL);
	  	tmpptr = mytimer->t_active;
	}

	/* 
	 * Update the memory mapped time variables.
	 */
        if (mycpu == MP_MASTER) {
        	curtime(&tod);
                time = tm.secs = tod.tv_sec;
        }

	/*  Reset the decrementer if the active list is not empty */
	if(mytimer->t_active != NULL)  {
	  	/*  Reset the decrementer.  */
	  	reset_decr();
	}

	/*  Drop the lock and restore to the initial interrupt level */
	unlock_enable(INTTIMER, &tod_lock);

	return(INTR_SUCC);
}


/*
 * NAME:  resched
 *
 * FUNCTION:  
 *	Put a process to sleep to be awakened by the timer for
 * 	the purposes of delivering job control signals.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can only be called under a process.
 *
 *      It may page fault.
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION:  Returns the value returned by e_sleep.
 *
 * EXTERNAL PROCEDURES CALLED:	e_sleep
 *
 */
int resched()
{
    	return e_sleep(&resched_event, EVENT_SIGRET);
}


/*
 * NAME:  talloc
 *
 * FUNCTION:  Get a timer request block from the free list
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can only be called under a process.
 *
 *      It may page fault.
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION:  A pointer to a timer request block on successful
 *	allocation of a trb, NULL upon failure.
 *
 * EXTERNAL PROCEDURES CALLED:	i_disable
 *				i_enable
 */
struct	trb *
talloc()
{
        register int ipri;          		/* caller's int priority  */
        register int i;				/* count of free trbs     */
        register struct trb *tmpptr=NULL;      	/* trb to return          */
        register struct trb *freeptr;      	/* trb to xmfree          */
        register struct trb *nextptr;      	/* next trb to xmfree     */
        struct ppda_timer *mytimer;      	/* processor timer struct */

        /*
         *  If there is a timer request block on the free list, then
         *  release all trb's on the free list.  Otherwise, allocate
         *  memory for one.
         */
	assert((ipri = i_disable(INTMAX)) == INTBASE);

        mytimer = MY_TIMER();

        if (mytimer->t_freecnt) {

                ASSERT(mytimer->t_free != NULL);

		/* recycle the first entry */
                tmpptr = mytimer->t_free;
                mytimer->t_freecnt--;
                mytimer->t_free = tmpptr->knext;

		/* 
		 * The freelist is serialized by disabling.  A lock is
		 * not required since this is a per processor data
		 * structure and it is only referenced by the current
		 * processor.  Use a local variable (freeptr) in case 
		 * we get rescheduled on another processor as a result
		 * of the i_enable or the xmfree. 
		 */
		freeptr = mytimer->t_free;
		mytimer->t_free = 0;
		i = mytimer->t_freecnt;
		mytimer->t_freecnt = 0;

		while (freeptr != NULL) {
                       	i--;
                       	nextptr = freeptr->knext;
                       	i_enable(ipri);
                       	xmfree(freeptr, pinned_heap);
                       	ipri = i_disable(INTMAX);
			freeptr = nextptr;	
                }
		assert(i == 0);
        }

        i_enable(ipri);

        if (tmpptr == NULL)
                tmpptr = xmalloc(sizeof(struct trb), 0, pinned_heap);

        if(tmpptr != NULL)  {
                bzero(tmpptr, sizeof(struct trb));
                tmpptr->eventlist = EVENT_NULL;
                tmpptr->id = -2;
                tmpptr->ipri = -1;
        }

        return(tmpptr);
}


/*
 * NAME:  tfree
 *
 * FUNCTION:  Free up a timer request block
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can be called under either a process or an interrupt level.
 *
 *      It may not page fault.
 *
 * RETURN VALUE DESCRIPTION: tfree does not have a returned value
 *
 * EXTERNAL PROCEDURES CALLED:	i_disable
 *				i_enable
 */
void
tfree(register struct trb *t)
{
	register int ipri;			/* caller's int priority  */
	struct ppda_timer *mytimer;		/* processor timer struct */

	/*
	 *  Verify that the timer request block is not on the active
	 *  list.
	 */
	assert(!(t->flags & T_PENDING));
	assert(!(t->flags & T_ACTIVE));

	/*
	 *  If the caller is a process, the trb can be freed.  Otherwise,
	 *  just put it on the local free list and it will be freed later.
	 */
 	if (csa->intpri == INTBASE)  
	{
		xmfree(t, pinned_heap);
	}
	else  
	{
		ipri = i_disable(INTMAX); 

		mytimer = MY_TIMER();

		/* Put the timer request block at the front of the free list */
                t->ipri = -1;
		t->knext = mytimer->t_free;
		mytimer->t_free = t;
		mytimer->t_freecnt++;

		i_enable(ipri);
	}
}


/*
 * NAME:  tstart
 *
 * FUNCTION:  Start a timer request.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can be called under either a process or an interrupt level.
 *
 *      It may not page fault.
 *
 * RETURN VALUE DESCRIPTION: tstart does not have a returned value
 *
 * EXTERNAL PROCEDURES CALLED:	i_disable
 *				i_enable
 */
void
tstart(register struct trb *t)
{
	register int ipri;			/* caller's int priority  */
	struct   timestruc_t scurtime;		/* current clock value	  */
	register struct	trb *tmpnext;		/* temp timer req ptr to  */
						/* walk active list	  */
	register struct trb *tmpptr;		/* temp prev timer req 	  */
						/* to walk active list	  */
	struct ppda_timer *mytimer;		/* processor timer struct */
	cpu_t mycpu;				/* the current cpu        */

        ipri = disable_lock(INTMAX, &tod_lock);

	/*
	 * Either it wasn't filled out correctly the first time or
	 * you are tstart'ing a freed trb.
	 */
	assert(t->ipri != -1);		  

	/* 
	 * Guard against concurrent tstart calls.
	 */
	assert(!(t->flags & T_PENDING));

        mycpu = CPUID;
        mytimer = &ppda[mycpu].ppda_timer;

	/*
	 * If it's been started, then it can only be restarted at the 
	 * interrrupt level if its your trb.
	 */
	if (t->flags & T_ACTIVE)
		assert(mytimer->trb_called == t);

	/*  Convert an interval timer to an absolute one.  */
	if (!(t->flags & T_ABSOLUTE)){
		curtime(&scurtime);
		ntimeradd(t->timeout.it_value, scurtime, t->timeout.it_value);
		t->flags |= T_INCINTERVAL;
	}
	t->cpunum = mycpu; /* Mark owning processor */

	tmpptr = mytimer->t_active;
	/*
	 *  See where on the system active list the trb is being placed.  If
	 *  it is being placed at the beginning, then it becomes the next
	 *  most immediate timeout and the "time until the next clock 
	 *  interrupt" needs to be recalculated.  Otherwise, it can just
	 *  be placed, in order, on the system active list.
	 */
	if((tmpptr == NULL) || 
	   (ntimercmp(tmpptr->timeout.it_value, t->timeout.it_value, >)))  {
		t->knext = tmpptr;
		t->kprev = (struct trb *)NULL;
		if(tmpptr != NULL)  {
			tmpptr->kprev = t;
		}
		mytimer->t_active = t;
		reset_decr();
	}
	else  {
		tmpnext = tmpptr->knext;
		/*
		 *  The trb needs to be placed somewhere other than the 
		 *  beginning of the list.  Run the list to find out exactly
		 *  where to place it.
		 */
		while(tmpnext != NULL &&
		      (ntimercmp(t->timeout.it_value, tmpnext->timeout.it_value, >))) 
		{
			tmpptr = tmpnext;
			tmpnext = tmpptr->knext;
		}
		/*
		 *  The position in the list has been found.  Now, chain the
		 *  trb into the list and handle the links differently if
		 *  placing the trb at the end because this is a doubly
		 *  linked, null terminated list.
		 */
		if(tmpnext == NULL)  {
			t->kprev = tmpptr;
			t->knext = (struct trb *)NULL;
			tmpptr->knext = t;
		}
		else  {
			t->kprev = tmpptr;
			t->knext = tmpnext;
			tmpptr->knext = t;
			tmpnext->kprev = t;
		}
	}

	/*  Mark this timer request as pending.  */
	t->flags |= (T_ACTIVE|T_PENDING);

	unlock_enable(ipri, &tod_lock);
}


/*
 * NAME:  tstop
 *
 * FUNCTION:  Stop a timer request.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can be called under either a process or an interrupt level.
 *
 *      It may not page fault.
 *
 * RETURN VALUE DESCRIPTION: None
 *
 * EXTERNAL PROCEDURES CALLED:	i_disable
 *				i_enable
 */
int
tstop(register struct trb *t)
{
        register int ipri;                   	/* caller's int priority  */
        struct ppda_timer *timer;               /* processor timer struct */
	int ncpus;				/* number of cpus         */
	int i;					/* ppda index             */
	cpu_t mycpu;

        ASSERT(t != NULL);

	ncpus = NCPUS();

	ipri = disable_lock(INTMAX, &tod_lock);

	mycpu = CPUID;

	/* 
	 * Check if handler is currently being called.  Can't depend on
	 * T_ACTIVE being set, because the handler can modify the flags
	 * field either directly by setting T_INCINTERVAL or indirectly
	 * by calling tstop.  Therefore we must scan an external data
	 * structure, the ppda structure, for state since it is beyond
	 * the reach of the caller. 
	 */
	for (i = 0; i < ncpus; i++) {
		if (ppda[i].ppda_timer.trb_called == t) {
			unlock_enable(ipri, &tod_lock);
			/*
			 * tstop may be called from the handler.   
			 */
			return(t->cpunum == mycpu ? 0 : -1);
		}
	}

	/* If the trb is on the active list. */
	if (t->flags & T_PENDING) {

		/* Get the right timer queue */
        	timer = TIMER(t->cpunum);

        	/*
         	 *  t is the request to be removed from the active list.  Now
         	 *  find out where on the list it is.
         	 */
        	if (t == timer->t_active)  {
                	/*  This request is at the head of the active list.  */
                	if (t->knext == NULL)  {
                        	/*
                         	 * This is the only request on the active 
				 * list.  Mark the active list empty by 
				 * clearing out the pointer to the front 
				 * of the active list.
                         	 */
                        	timer->t_active = (struct trb *)NULL;
                	}
                	else  {
                        	/*
                         	 * This is not the only request on the active 
				 * list so t->knext is a valid trb structure.
                         	 */
                        	timer->t_active = t->knext;
                        	t->knext->kprev = (struct trb *)NULL;
                	}
        	}
        	else  {
                	/*
                 	 * This request is not at the front of the active 
			 * list.  See if it is at the back.
                 	 */
                	if(t->knext == NULL)  {
                        	/*  at the back of the active list.  */
                        	t->kprev->knext = (struct trb *)NULL;
                	}
                	else  {
                        	/*
                         	 * This request is between the front and 
				 * the back elements on the active list so 
				 * t->knext and t->kprev are valid.
                         	 */
                        	t->kprev->knext = t->knext;
                        	t->knext->kprev = t->kprev;
                	}
        	}

        	t->knext = t->kprev = (struct trb *)NULL;

        	/*  Mark this timer request as not pending.  */
        	t->flags &= ~(T_PENDING|T_ACTIVE);
	}

        unlock_enable(ipri, &tod_lock);
	return(0);
}


/*
 * NAME:  tfork
 *
 * FUNCTION:  
 *	Initilializes the appropriate timer fields for a process upon
 *	process creation.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can only be called under a process.
 *
 *      It may page fault.
 *
 * RETURN VALUE DESCRIPTION: None
 *
 * EXTERNAL PROCEDURES CALLED:	xmalloc
 *				bzero
 */
tfork(register struct user *nuser, register struct uthread *nut)
{
	register int tindex;		/* index into user timer array  */
 
	/*  POSIX specifies that timers are not inherited across fork().  */
	if (nuser != NULL) {
		for(tindex = 0; tindex < NTIMERS; tindex++)  {
			nuser->U_timer[tindex] = NULL;
		}
	}
	if (nut != NULL) {
        	for(tindex = 0; tindex < NTIMERS_THREAD; tindex++)  {
                	nut->ut_timer[tindex] = NULL;
        	}
	}
}


/*
 * NAME:  reset_decr
 *
 * FUNCTION:  
 *	Reset the clock decrementer to the appropriate time to arrange
 *	for the next clock interrupt.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can be called under either a process or an interrupt
 *	level.
 *
 *      It does not page fault.
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION: None
 *
 * EXTERNAL PROCEDURES CALLED:	i_disable
 *				i_enable
 */
void
reset_decr()
{
	struct	timestruc_t scurtime;		/* current time		*/
	struct	timestruc_t difftime;		/* time between now and	*/
						/* next timer request	*/
	struct ppda_timer *mytimer;		/* processor timer struct */

	ASSERT(csa->intpri == INTMAX);    	

        mytimer = MY_TIMER();       		

	/*  Get the current system time.  */
	curtime(&scurtime);

	/*
	 *  If the current time is already later than the first timer 
	 *  request, then there should be a clock interrupt right away.
	 */
	if(ntimercmp(mytimer->t_active->timeout.it_value, scurtime, <))  {
		dec_imm();
		return;
	}

	/*
	 *  If the current time is not later than the first timer request,
	 *  then the time to set the next timer interrupt for is MIN(the 
	 *  greatest value that can be specified for the next timer interrupt,
	 *  the difference between the time now and the next timer request).
	 */
	ntimersub(mytimer->t_active->timeout.it_value, scurtime, difftime);
	dec_set(difftime);
}


/*
 * NAME:  sys_timer
 *                                                                    
 * FUNCTION:  
 *	This is the timeout function for the system clock.  This routine
 *	is called via the clock interrupt handler upon every clock interrupt.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine is only called upon the clock interrupt level.
 *                                                                   
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:  sys_timer does not have a returned value.
 *
 * EXTERNAL PROCEDURES CALLED:	i_enable
 *				i_disable
 *				adj_tick
 *				kthread_kill
 *				watchdog
 *				schedcpu
 */  
void
sys_timer(register struct trb *systimer)
{
	struct thread *t;		/* current thread		*/
	struct uthread *ut;		/* current uthread		*/
	struct proc *p;			/* current proc                 */
	register int ipri;		/* caller's interrupt priority	*/
	long ixrss;		 	/* shared memory size		*/
	long idrss;			/* unshared data size		*/
	struct timestruc_t ct;		/* used in timer rescheduling   */
	long clock_ticks;		/* clock ticks			*/
	struct ppda_timer *mytimer;	/* processor timer struct 	*/
	struct ppda *myppda;		/* processor data area		*/
	struct mstsave *mycsaprev;	/* previous csa			*/
	cpu_t mycpuid;			/* current cpu id		*/
	int ncpus, i;
	boolean_t timer_locked;		/* U_timer_lock taken?		*/
	
	myppda = PPDA;

        ASSERT(myppda->_csa->intpri == INTTIMER);

	t = myppda->_curthread;
	p = t->t_procp;
	ut = t->t_uthreadp;

        mytimer = &myppda->ppda_timer;
	mycsaprev = myppda->_csa->prev;

	TRCHKL2T(HKWD_KERN_PROF, mycsaprev->iar, mycsaprev->lr);

        if(t->t_suspend == 0)  {

		/* update resource usage fields */
		idrss = 4 * vms_rusage(U.U_adspace.srval[PRIVSEG]);
		ixrss = 4 * vms_rusage(U.U_adspace.srval[TEXTSEG]);

		/* Check for shared library data segment */
		if (U.U_adspace.alloc & ((unsigned)0x80000000 >> SHDATASEG))
			idrss += 4 * vms_rusage(U.U_adspace.srval[SHDATASEG]);

		/* Check for an overflow segment */
		if (OVFL_EXISTS((struct loader_anchor *)U.U_loader))
			idrss += 4 * vms_rusage(((struct loader_anchor *)
				(U.U_loader))->la_ovfl_srval);

		if (U.U_dsize > SEGSIZE)	/* big data? */
			idrss += 4 * bd_rusage(&U);

 		/* Only update if no error from vms_rusage */
		if ((idrss >= 0) && (ixrss >= 0))
		{
#ifdef _POWER_MP
			while ((idrss + ixrss) > U.U_ru.ru_maxrss &&
				!compare_and_swap(&U.U_ru.ru_maxrss, 
					&U.U_ru.ru_maxrss, idrss + ixrss));
			fetch_and_add(&(U.U_ru.ru_idrss),idrss);
			fetch_and_add(&(U.U_ru.ru_ixrss),ixrss);
#else
			U.U_ru.ru_maxrss = 
				MAX(U.U_ru.ru_maxrss, idrss+ixrss);
			U.U_ru.ru_idrss += idrss;
			U.U_ru.ru_ixrss += ixrss;
#endif
		}
	}

#ifdef _POWER_MP
	if (timer_locked = (p->p_active != 1))
		simple_lock(&U.U_timer_lock);
#endif

        if(t->t_suspend == 0)  {

		/*
		 *  The current thread is in user mode so update the BSD 
		 *  virtual timer if it is set.  Do the same for the AIX
		 *  Virtual Timer.
		 */
		if((U.U_timer[ITIMER_VIRTUAL] != NULL) &&
		   (ntimerisset(&(U.U_timer[ITIMER_VIRTUAL]->timeout.it_value))) &&
		   ((itimerdecr(&(U.U_timer[ITIMER_VIRTUAL]->timeout),
				NS_PER_SEC / HZ)) == 0)) {
			pidsig(p->p_pid, SIGVTALRM);
		}
	
		if((U.U_timer[ITIMER_VIRT] != NULL) &&
		   (ntimerisset(&(U.U_timer[ITIMER_VIRT]->timeout.it_value))) &&
		   ((itimerdecr(&(U.U_timer[ITIMER_VIRT]->timeout),
				NS_PER_SEC / HZ)) == 0)) {
			pidsig(p->p_pid, SIGVIRT);
		}

		/*
 		 *  Charge the current process based on the mode of the cpu.
 		 *  Note that while the ru_utime and ru_stime fields of the
 		 *  u-block are timeval structures and these traditionally
 		 *  contain seconds and MICROseconds, the kernel uses them
 		 *  to represent seconds and NANOseconds.  Conversion to 
 		 *  microseconds is made above the kernel.
 		 */
		ADD_NS_TO_TIMEVAL(U.U_ru.ru_utime, NS_PER_SEC / HZ);

		/*
		 * If we are profiling, update the profile buffer.
		 */
		if (U.U_prof.pr_scale) {
			addupc( (caddr_t)ut->ut_save.iar, 
				(struct profdata *)&U.U_prof, 1);
		}
 		/* Only update if no error from vms_rusage */
		if ((idrss >= 0) && (ixrss >= 0)) {
#ifdef  _LONG_LONG
                        U.U_irss += idrss + ixrss;
#else
                        ADDL64 (U.U_irss, idrss);
                        ADDL64 (U.U_irss, ixrss);
#endif
		}
	}
	else  {
		/*         
		 * must be in kernal mode.  See comment above about updating times. 
		 */
		ADD_NS_TO_TIMEVAL(U.U_ru.ru_stime, NS_PER_SEC / HZ);
	}

	/*  Update the current process's BSD profile timer if it is set.  */
	if((U.U_timer[ITIMER_PROF] != NULL)  &&
	   (ntimerisset(&(U.U_timer[ITIMER_PROF]->timeout.it_value))) &&
	   ((itimerdecr(&(U.U_timer[ITIMER_PROF]->timeout), 
			NS_PER_SEC / HZ)) == 0))  {
		pidsig(p->p_pid, SIGPROF);
	}

	/*  Enforce CPU Time Slice Limits */ 
	if (!(t->t_flags & TKTHREAD)) {
             	if (U.U_ru.ru_utime.tv_sec + U.U_ru.ru_stime.tv_sec + 1 >
                    U.U_rlimit[RLIMIT_CPU].rlim_cur)
		{
			pidsig(p->p_pid, SIGXCPU);
                	if (U.U_rlimit[RLIMIT_CPU].rlim_cur <
                    			U.U_rlimit[RLIMIT_CPU].rlim_max)
                        	U.U_rlimit[RLIMIT_CPU].rlim_cur += 5;
		}
	}

#ifdef _POWER_MP
	if (timer_locked)
        	simple_unlock(&U.U_timer_lock);
#endif

	/*
	 * The lbolt and time variables will only by updated by the master 
	 * processor.  Sync the memory access so the other processors see 
	 * it.  They are exported, so a fetch_and_add won't help us as old 
	 * code don't use it!
	 */
	mycpuid = myppda->cpuid;
	if (mycpuid == MP_MASTER) {
	  	/*  Update tick oriented time variables.  */
	  	lbolt++;
#ifdef _POWER_MP
	  	__iospace_sync();
#endif
	}

	/*
	 *  HZ is the number of clock "ticks" per second, watchdog()
	 *  should be called once per second.  sys_timer runs every 
	 *  clock "tick".  
	 */
	mytimer->ticks_its++;
	if(mytimer->ticks_its == HZ) {
		mytimer->ticks_its = 0;

		if (mycpuid == MP_MASTER) {
			/*
		 	 *  Update the memory mapped variable "time"
			 *  which is the number of seconds since the Epoch.
		 	 */
		  	time++;
#ifdef _POWER_MP
		  	__iospace_sync();
#endif

		  	e_wakeup(&resched_event);

		  	/*
		   	 * Call memory scrubbing code once a second, if enabled.
		   	 */
		  	if (v.v_memscrub)
		    		scrubclock();
		}
		watchdog();
	}

	/*  
	 *  schedcpu() needs to be called every clock tick.  
	 *  update i/o statistics once each clock tick.
	 *  update syscall statistics from ppda.
	 */

	schedcpu();
	cpuinfo[mycpuid].syscall = myppda->syscall;
	ncpus = _system_configuration.ncpus;
	if (mycpuid == MP_MASTER){
		/* MP_MASTER only in order to update only once per clock tick */
	  	update_iostats(); /* waiting for disk, or idle? */
		/* MP_MASTER only to minimize overhead */
		sysinfo.syscall = cpuinfo[0].syscall;
#ifdef _POWER_MP
		for (i = 1; i < ncpus; i++)
			sysinfo.syscall += cpuinfo[i].syscall;
#endif
	}

	/* 
   	 *  Update the sysinfo structures:
   	 *  IMPORTANT: THIS HAS TO HAPPEN AFTER update_iostats()      
   	 *  SINCE THAT IS WHERE THE disks_active FLAG GETS SET
   	 *
   	 *  If executing code on an interrupt level, then charge the
   	 *  time to CPU_KERNEL. If the current process is 'wait' and 
   	 *  there is no pending disk i/o, then we're idle, otherwise
   	 *  we're waiting and should charge cpu[CPU_WAIT]. If the 
	 *  curproc  isn't 'wait', then charge CPU_KERNEL or CPU_USER 
	 *  accordingly.
   	 *
   	 *  The sysinfo structure might be accessed by other processors.
   	 *  Use atomic operations to solve that.
	 *  The cpuinfo structure is per processor and thus safe.
   	 */
  	if(mycsaprev->prev == NULL) {		/* We're not on an int level */
            	if ((t->t_procp > &proc[1 + ncpus]) || (t->t_procp < &proc[2]))
		{				/* We're not a wait process */
		        if(t->t_suspend == 0) {	/* We're in user mode */
#ifdef _POWER_MP
				fetch_and_add(&(sysinfo.cpu[CPU_USER]), 1);
#else
				sysinfo.cpu[CPU_USER]++;
#endif
				cpuinfo[mycpuid].cpu[CPU_USER]++;
			}
	      		else {			/* We're in kernel mode */
#ifdef _POWER_MP
				fetch_and_add(&(sysinfo.cpu[CPU_KERNEL]), 1);
#else
				sysinfo.cpu[CPU_KERNEL]++;
#endif
				cpuinfo[mycpuid].cpu[CPU_KERNEL]++;
			}
	    	}
	    	else {      			/* We're a wait process */
	      		if(disks_active) {	/* We're waiting for disk IO */
#ifdef _POWER_MP
				fetch_and_add(&(sysinfo.cpu[CPU_WAIT]), 1); 
#else
				sysinfo.cpu[CPU_WAIT]++;
#endif
				cpuinfo[mycpuid].cpu[CPU_WAIT]++;
			}
			else {			/* We're idle */
#ifdef _POWER_MP
				fetch_and_add(&(sysinfo.cpu[CPU_IDLE]), 1); 
#else
				sysinfo.cpu[CPU_IDLE]++;
#endif
				cpuinfo[mycpuid].cpu[CPU_IDLE]++;
			}
	 	}
	}
  	else {     	                  	/* We're on an int level */
#ifdef _POWER_MP
	    	fetch_and_add(&(sysinfo.cpu[CPU_KERNEL]), 1);
#else
		sysinfo.cpu[CPU_KERNEL]++;
#endif
		cpuinfo[mycpuid].cpu[CPU_KERNEL]++;
	}	  

	/* 
	 * Perform tick adjustment (resets the processor rtc, decrementer,
	 * trbs, memory mapped time variables), if an adjtime() was done.
	 * adj_tick() assumes that that ref_time is greater than curtime.
	 */
        clock_ticks=0;
	ipri = i_disable(INTMAX);
	curtime(&ct);
	while (!ntimercmp(mytimer->ref_time, ct, >)){
		ntimeradd(mytimer->ref_time, ref_const, mytimer->ref_time); 
		clock_ticks++;
	}
	ntimersub(mytimer->ref_time, ct, ct);
	adj_tick();
	i_enable(ipri);

	/*
	 * Schedule the next timer interrupt.  Moreover, schedule it 
	 * immediately if more than one clock tick has occurred.  This
	 * is necessary for accounting and watchdog timers, etc.  Assume
	 * that only one extra clock tick will occur at a time.
	 */
	systimer->flags = T_INCINTERVAL;
	systimer->timeout.it_value.tv_sec = 0;
	if (clock_ticks > 1)
		systimer->timeout.it_value.tv_nsec = 0;
	else 
		systimer->timeout.it_value.tv_nsec = ct.tv_nsec;

        systimer->ipri = INTTIMER;
	tstart(systimer);
}


/*
 * NAME:  adj_trbs
 *                                                                    
 * FUNCTION:  Adjust the interval oriented timer request blocks on the active
 *	list.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 *	This routine may be called under either a process or an interrupt 
 *	level.
 *
 *	This routine does not page fault except on a pageable stack.
 *
 *	The caller is expected to serialize to INTMAX.
 *                                                                   
 * RETURN VALUE DESCRIPTION:  adj_trbs does not have a returned value
 *
 * EXTERNAL PROCEDURES CALLED:  simple_lock, simple_unlock
 */  
void
adj_trbs(struct timestruc_t nct)
{
	struct timestruc_t ct;		/* current system time		*/
	struct timestruc_t dt;		/* diff between current/new time*/
	register struct trb *t;		/* trb ptr to walk active list	*/
	register struct trb **tp;	/* ptr to ptr to the active list*/
	register struct trb *at;	/* altered trb list		*/
	register struct trb *ath;	/* head of the altered trb list	*/
	register struct trb *tmpptr;	/* ptr to previous trb on active*/
	struct ppda_timer *mytimer;	/* processor timer struct	*/

	ASSERT(csa->intpri == INTMAX);

	mytimer = MY_TIMER();

	/*  Get the current system time to see by how much time is changing.  */
	curtime(&ct);

	/*
	 *  If the new time is the same as the old time, there is no work 
	 *  that needs to be done.
	 */
	if(!(ntimercmp(nct, ct, !=)))  {
		return;
	}

	/*
	 *  Start from the head of the active list.  Each trb on the active
	 *  list will be examined and, if it is an interval request, it will
	 *  be altered by the amount of the time change and put on a temporary
	 *  list.  When all trbs on the active list have been examined, the
	 *  trbs on the temporary list will be put back on the active list.
	 */
	t = tmpptr = at = ath = NULL;
	if(ntimercmp(nct, ct, >))  {
		/*  Calculate by how much to change each trb.  */
		ntimersub(nct, ct, dt);
		for(tp = &(mytimer->t_active); t = *tp; tp = &(t->knext))  {
			/*
			 *  The times for each trb should only be altered if
			 *  the trb is an incremental one.
			 */
			if(t->flags & T_INCINTERVAL)  {
				/*
				 *  If the head of the active list is the one
				 *  that is going to be removed, then the next
				 *  trb will be the new head of the active list
				 *  and its previous pointer must be cleared.
				 */
				if(t == mytimer->t_active)  {
					ASSERT(t->kprev == NULL);
					mytimer->t_active = t->knext;
				}
				/*
				 *  Otherwise, kprev must be valid and can be
				 *  used to update the next pointer of the
				 *  previous trb.
				 */
				else  {
					ASSERT(t->kprev != NULL);
					t->kprev->knext = t->knext;
				}
				/*
				 *  If the trb being removed is not at the end
				 *  of the active list, then the trb following
				 *  this one needs to be updated to point to
				 *  the trb preceeding this one.
				 */
				if(t->knext != NULL)  {
					t->knext->kprev = t->kprev;
				}

				/*
				 *  Update the timeout value before putting the
				 *  trb on the altered list.
				 */
                        	ntimeradd(dt,t->timeout.it_value,t->timeout.it_value);

				/*
				 *  Put the trb on the altered list, keeping
				 *  a pointer (ath) to the head of the list.
				 */
				if(at == NULL)  {
					ath = at = t;
					t->kprev = NULL;
				}
				else  {
					t->kprev = at;
					at->knext = t;
					at = t;
				}
			}
		}
		if(at != NULL)  {
			at->knext = NULL;
		}
	}
	else  {
		/*  Calculate by how much to change each trb.  */
		ntimersub(ct, nct, dt);
		for(tp = &(mytimer->t_active); t = *tp; tp = &(t->knext))  {
			/*
			 *  The times for each trb should only be altered if
			 *  the trb is an incremental one.
			 */
			if(t->flags & T_INCINTERVAL)  {
				/*
				 *  If the head of the active list is the one
				 *  that is going to be removed, then the next
				 *  trb will be the new head of the active list
				 *  and its previous pointer must be cleared.
				 */
				if(t == mytimer->t_active)  {
					ASSERT(t->kprev == NULL);
					mytimer->t_active = t->knext;
				}
				/*
				 *  Otherwise, kprev must be valid and can be
				 *  used to update the next pointer of the
				 *  previous trb.
				 */
				else  {
					ASSERT(t->kprev != NULL);
					t->kprev->knext = t->knext;
				}
				/*
				 *  If the trb being removed is not at the end
				 *  of the active list, then the trb following
				 *  this one needs to be updated to point to
				 *  the trb preceeding this one.
				 */
				if(t->knext != NULL)  {
					t->knext->kprev = t->kprev;
				}

				if(!(ntimercmp(t->timeout.it_value, dt, <)))  {
					/*
					 *  Update the timeout value before 
					 *  putting the trb on the altered list.
					 */
					ntimersub(t->timeout.it_value, dt, t->timeout.it_value);
				}
				else  {
					/*
					 *  If the timeout value is less than
					 *  the "time by which to change each
					 *  trb", then this is an exception
					 *  case.  This means that the timeout
					 *  is already past and would have 
					 *  occurred had interrupts not been 
					 *  disabled.  In this case, it is 
					 *  necessary to ensure that this 
					 *  timeout will occur right away. This
					 *  can be done by setting the timeout 
					 *  field to 0.  This case should only
					 *  occur when the time is being set
					 *  to a very small value (e.g. less 
					 *  than 21600 which is the Epoch) so
					 *  this really should never happen.
					 */
					t->timeout.it_value.tv_sec = 0;
					t->timeout.it_value.tv_nsec = 0;
				}

				/*
				 *  Put the trb on the altered list, keeping
				 *  a pointer (ath) to the head of the list.
				 */
				if(at == NULL)  {
					ath = at = t;
					t->kprev = NULL;
				}
				else  {
					t->kprev = at;
					at->knext = t;
					at = t;
				}
			}
		}
		if(at != NULL)  {
			at->knext = NULL;
		}
      	}
        /*  There is only work to be done if the altered list is not empty.  */
	if(ath != NULL)  {
		/*
		 *  Initialize the adjusted timer ptr to the head of the
		 *  adjusted timer list.
		 */
		at = ath;

		/*
		 *  Start at the front of the active list and keep working
		 *  until either a) the altered list is empty, or b) the end of
		 *  the active list has been reached.
		 */
		for(t = mytimer->t_active; 
		    t != NULL && ath != NULL; )  {
			/*
			 *  The active list needs to remain monotonically
			 *  increasing with respect to timeout value so don't
			 *  stop until a trb is found with a timeout value
			 *  "later than" the trb's.
			 */
			if(ntimercmp(ath->timeout.it_value, t->timeout.it_value, <))  {
				/*
				 *  If placing the trb at the front of the 
				 *  active list, the active list anchor needs
				 *  to be updated and the trb's kprev field 
				 *  must be cleared.
				 */
				if(t == mytimer->t_active)  {
					at = ath;
					ath = ath->knext;
					at->kprev = NULL;
					at->knext = t;
					t->kprev = at;
					mytimer->t_active = at;
				}
				/*
				 *  If the trb is being placed somewhere other
				 *  than the beginning of the active list, then
				 *  the trb's kprev field needs to be updated.
				 */
				else  {
					at = ath;
					ath = ath->knext;
					at->kprev = t->kprev;
					at->knext = t;
					t->kprev->knext = at;
					t->kprev = at;
				}
			}
			else  {
				tmpptr = t;
				t = t->knext;
			}
		}
		/*
		 *  If some of the altered trbs are "later than" all of those
		 *  on the active list, then at this point, the rest of the
		 *  ones on the altered list can just be chained to the end
		 *  of the active list.
		 */
		if(ath != NULL)  {
			ASSERT(t == NULL);
			/*
			 *  If the active list was empty to begin with, then
			 *  the pointer to the head of the active list can
			 *  just be set to the altered list...
			 */
			if(tmpptr == NULL)  {
				mytimer->t_active = ath;
				ath->kprev = NULL;
			}
			/*
			 *  otherwise, the altered list needs to be chained 
			 *  at the pointer to the last trb on the active list
			 *  (tmpptr).
			 */
			else  {
				ath->kprev = tmpptr;
				tmpptr->knext = ath;
			}
		}
	}
}


/*
 * NAME:  delay
 *                                                                    
 * FUNCTION:  Suspend calling process for a specified number of clock ticks.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine can only be called under a process.
 *                                                                   
 * NOTES:  The ticks parameter is the number of clock ticks for which to
 *	delay the calling process.  Traditional UNIX defines the hz or
 *	HZ label (in a system include file) as the number of clock ticks
 *	per second.
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:  This service returns a 0 upon succesful
 *	completion, and it returns a -1 if the delay was interrupted
 *	by a signal.
 *
 * EXTERNAL PROCEDURES CALLED:	e_sleep
 *				e_wakeup
 *				i_disable
 *				i_enable
 */  
int
delay(int ticks)
{
	register int ipri;		/* caller's interrupt priority	*/
	register int rv;		/* return value from e_sleep	*/
	register struct	trb *trb;	/* trb to submit timeout request*/

	assert(csa->prev == NULL);

	if (ticks <= 0)  {
		return(0);
	}

	/*  Allocate the timer request block.  */
	trb = talloc();

	/*
	 *  Convert the number of ticks specified to a time value that
	 *  can be provided to the timer services.
	 */
	TICKS_TO_TIME(trb->timeout.it_value, ticks);

	/*
	 *  Ensure that this is treated as an incremental timer, not an
	 *  absolute one.
	 */
	trb->flags	=  T_INCINTERVAL;
	trb->func	=  (void (*)(void *))delay_end;
	trb->eventlist	=  EVENT_NULL;
	trb->id	        =  curthread->t_tid;
	trb->func_data	=  (uint)(&(trb->eventlist));
        trb->ipri       =  INTTIMER;

	/* Put on event list so that the wakeup will not be lost. */
	e_assert_wait(&trb->eventlist, FALSE);

	tstart(trb);

	(void)e_block_thread();

	/* ensure timeout handler is through with trb */
	while (tstop(trb));

	/* finally, free up trb since we're done */
	tfree(trb);

	return(0);
}


/*
 * NAME:  delay_end
 *                                                                    
 * FUNCTION:  Call e_wakeup() to resume suspended process.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine is called via the timer interrupt handler.
 *                                                                   
 */  
void
delay_end(register struct trb *t)
{
	assert(csa->prev != NULL);

	e_wakeup((int *)t->func_data);
}


/*
 * NAME:  itimerdecr
 *                                                                    
 * FUNCTION:  Decrement an interval timer (i.e. the it_value field of the
 *	itimerstruc_t parameter) by a specified number of nanoseconds.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine is called by the timer interrupt handler.
 *                                                                   
 *	It does not page fault.
 *                                                                   
 * NOTES:  The number of nanoseconds by which to decrement the interval must
 *	be greater than zero and less than one second (i.e. 1000000000).
 *
 *	If the timer expires, then it is reloaded (with the contents of the
 *	it_interval field of the itimerstruc_t parameter).
 *
 *	If the parameter by which to decrement is greater than the value in
 *	the timer and there is no reload value, timer value is made 0 and
 *	0 is returned indicating that the timer has decremented to or past
 *	0.  If the parameter by which to decrement is greater than the 
 *	value in the timer and there IS a reload value, then the reload 
 *	value is loaded into the timer value, any of the decrement that 
 *	was left over is used to decrement the newly reloaded timer value,
 *	and 0 is returned indicating that the timer has decremented to or
 *	past 0.  If the parameter by which to decrement is less than the
 *	value in the timer, then the timer value is simply decremented 
 *	and 1 is returned indicating that the timer has not decremented
 *	to or past 0.
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:  0 if the timer has decremented to or past zero,
 *	1 otherwise.
 */  
int
itimerdecr(register struct itimerstruc_t *tv, register int ns)
{
	/*  Parameters must be valid nanosecond values.  */
	ASSERT((ns < NS_PER_SEC) && (ns > 0));

	/*  See if we're decrementing by more ns than are left in the timer.  */
	if(tv->it_value.tv_nsec < ns)  {
		/*  We are - see if there are any seconds left.  */
		if(tv->it_value.tv_sec == 0)  {
			/*
			 *  There aren't so take away as many ns as there were
			 *  and remember how many we still need to take away.
			 *  We do this because if this timer has a reload 
			 *  value, the rest of the ns will need to be taken
			 *  from that.
			 */
			ns -= tv->it_value.tv_nsec;
		}
		else  {
			/*
			 *  There are seconds left so we can borrow a second's
			 *  worth of ns and do the decrement.
			 */
			tv->it_value.tv_nsec += NS_PER_SEC;
			tv->it_value.tv_sec--;
			tv->it_value.tv_nsec -= ns;
			ns = 0;

			/*
			 *  If there is still time left, return to the caller
			 *  indicating such.
			 */
			if(ntimerisset(&(tv->it_value)))  {
				return(1);
			}
		}
	}
	else  {
		/*
		 *  Simple case - no borrowing needed.  Just decrement the
		 *  ns and if there is still time left, return to the caller
		 *  indicating such.
		 */
		tv->it_value.tv_nsec -= ns;
		ns = 0;
		if(ntimerisset(&(tv->it_value)))  {
			return(1);
		}
	}

	/*  See if the timer has a reload value.  */
	if(ntimerisset(&(tv->it_interval)))  {
		/*
		 *  It does, so copy the reload value into the value structure,
		 *  and take away any ns that were left over from ns being
		 *  greater than what was left in the timer.
		 */
		tv->it_value = tv->it_interval;
		tv->it_value.tv_nsec -= ns;
		if(tv->it_value.tv_nsec < 0)  {
			tv->it_value.tv_nsec += NS_PER_SEC;
			if (tv->it_value.tv_sec)
				tv->it_value.tv_sec--;
			else
				tv->it_value = tv->it_interval;
		}
	}
	else  {
		tv->it_value.tv_nsec = 0;
	}

	/*  Indicate that the timer has decremented to or past 0.  */
	return(0);
}

/*
 * increment profiling buffer entry corresponding to the given pc
 * the use of pr_scale follows PDP-11 tradition, so as to avoid the need to
 * customize prof, etc. across 16-bit and 32-bit architectures.
 */
addupc(caddr_t pc, struct profdata *profp, int inc)
{
	short *base;
	struct prof *pp;
	unsigned short scale;
	unsigned long off;
	unsigned long size;
	unsigned long t;
	short *wp;
	unsigned short hi;
	unsigned short lo;
	unsigned short cnt_tmp;
	int pri;
	vmhandle_t srval;

	t = (unsigned)pc;
	if ((long)(size = profp->pr_size) != -1) {
		off = profp->pr_off;
		scale = profp->pr_scale;
		base = (short *)profp->pr_base;
		if (t < off)
			return(-1);
	} 
	else {
		scale = 0;
		for (pp = (struct prof *)profp->pr_base; pp->p_high; ++pp)
			if (pp->p_low <= (caddr_t)t && (caddr_t)t < pp->p_high){
				base = (short *) pp->p_buff;
				off = (unsigned long)pp->p_low;
				scale = pp->p_scale;
				size = pp->p_bufsize;
				break;
			}
		if (!scale)
			return(-1);
	}
	t -= off;

	hi = (t & (TOP(1) - TEXTORG)) >> 16; 
	lo = t & 0x0000ffff;
	t  = hi * scale;
	t += (( lo * scale ) >> 16);

	if (t >= size)
		return(-1);

	if (t & 0x1)
		t += 1;
	/*
	 * Update the short.
	 */
	wp = (short *)((int)base + (int)t);

        /*
         * The current thread is in user mode, so grab the segment
         * register out of the mst.  An exception handler is not
         * required, because the profiling buffer is pinned.  The
         * segment is protected from disappearing, because the VMM
         * waits for an INTIODONE MPC to complete before deleting
         * the segment and this code runs at a more favored level.
         */
        srval = as_getsrval(&u.u_save.as, wp);
        wp = (short *)vm_att(srval, wp);
        *wp += inc;
        vm_det(wp);

	return(0);
}

/*
* Return number of resident pages in use for a large data program.
*/
bd_rusage(struct user *uptr)
{
	int segno, ru = 0;

	for (segno = BDATASEG; segno <= BDATASEGMAX; segno++)
	{
		/* all done on first non-working segment */
		if (!(uptr->U_segst[segno].segflag & SEG_WORKING))
			break;

		ru += vms_rusage(uptr->U_adspace.srval[segno]);
	}
	return ru;
}

/*
* Return number of page space blocks in use for a large data program.
*/
bd_psusage(struct user *uptr)
{
	int segno, psu = 0;

	for (segno = BDATASEG; segno <= BDATASEGMAX; segno++)
	{
		/* all done on first non-working segment */
		if (!(uptr->U_segst[segno].segflag & SEG_WORKING))
			break;

		psu += vms_psusage(uptr->U_adspace.srval[segno]);
	}
	return psu;
}

