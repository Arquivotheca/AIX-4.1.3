static char sccsid[] = "@(#)62	1.41  src/bos/kernel/proc/tinit.c, sysproc, bos41J, 9515A_all 4/10/95 10:42:53";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: tinit
 *		watchdog_init
 *
 *
 *   ORIGINS: 27,83
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#include <sys/adspace.h>        
#include <sys/rtc.h>		/* for the real time clock macros	*/
#include <sys/intr.h>		/* to define the handler structure	*/
#include <sys/syspest.h>	/* define assert macro			*/
#include <sys/param.h>		/* define HZ (ticks per second)		*/
#include <sys/types.h>		/* always helpful			*/
#include <sys/time.h>		/* structures within the trb structure	*/
#include <sys/timer.h>		/* for the timer request block		*/
#include <sys/systemcfg.h>	/* machine dependencies			*/
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#include <sys/user.h>

extern int clock();			/* the timer interrupt handler	*/
extern void sys_timer(void *);		/* the system timer routine	*/
extern struct intr clock_intr;		/* timer interrupt struct	*/
extern struct timestruc_t ref_const;	/* 10mS constant timestruc_t    */
extern int brkpoint();
extern void watchdog_init();

#ifdef _POWER_MP
extern int ksettimer_mpc(void);
extern mpc_msg_t mpct;
#endif

#ifdef _POWER_PC
extern	int	dec_vec_hold;		/* decrementer vector contents	*/
extern	void	writedec_vec();
#endif /* _POWER_PC */

/*
 * NAME:  tinit
 *                                                                    
 * FUNCTION:  Initialize the array of timer request structures.
 *                                                                    
 * EXECUTION ENVIRONMENT:  This routine is called during system initialization
 *			   via a pointer-to-function in an array defined in
 *			   <sys/init.h>
 *                                                                   
 * NOTES:  Initialize the clock interrupt handler and the data structures
 *		which will maintain the timer request block array.
 *
 * DATA STRUCTURES:  t_maint (contains all the data about the array)
 *	ticks_its (contains a count of number of ticks in the current second)
 *
 * RETURN VALUE DESCRIPTION:  This procedure does not return a value.
 *
 * EXTERNAL PROCEDURES CALLED:  talloc
 *				tstart
 *				i_init
 *				init_clock
 */  
void
tinit()
{
	register int rv;		/* return value 		*/
	int ipri;			/* interrupt priority		*/
	int cpu; 			/* processor loop index         */
        int i;
	struct timestruc_t t;
	unsigned int ns;
	cpu_t my_cpu=CPUID;
	int maxcpu = _system_configuration.ncpus;

	if (my_cpu == MP_MASTER){

#ifdef _POWER_MP
	  	mpct = mpc_register(INTTIMER, (void (*)())ksettimer_mpc);
#endif

#ifdef _POWER_RS
	    	/*  Set up the clock interrupt handler structure.  */
	  	if( __power_rs() ) {
	    		clock_intr.next = (struct intr *)NULL;	
	    		clock_intr.handler = (int (*)())clock;	
	    		clock_intr.bus_type = BUS_PLANAR;	
	    		clock_intr.flags = INTR_NOT_SHARED | INTR_MPSAFE;	
	    		clock_intr.level = INT_TIMLVL;		
	    		clock_intr.priority = INTTIMER;	
	    		clock_intr.bid = 0x80000000;
	    
	    		/*  the interrupt handler initialization routine. */
	    		rv = i_init(&clock_intr);
	    		assert(rv == INTR_SUCC);
	  	}
#else 
        	/*
	 	 * Power PC does not require an interrupt handler, because the 
 	 	 * decrementer interrupt comes in at INTMAX on its own interrupt
	 	 * vector and is scheduled for off level.
         	 */
#endif /* _POWER_RS */
	  	lbolt = 0;
	  	ref_const.tv_sec = 0;
	  	ref_const.tv_nsec = NS_PER_SEC/HZ;
	  	__iospace_sync(); 		/* Make them globaly visible */
	  	init_clock();
		for (cpu = 0; cpu < maxcpu; cpu += 1) {
			ppda[cpu].ppda_timer.systimer = talloc();
		}
	}

	/*
	 * initiate watchdog structures
	 */
	watchdog_init();
	/*
	 * Initiate trb parts of ppda
	 */
	TIMER(my_cpu)->trb_called = NULL;
	TIMER(my_cpu)->t_free = NULL;
	TIMER(my_cpu)->t_active = NULL;
	TIMER(my_cpu)->t_freecnt = 0;
	/*
	 * Initialize the time, including the tod chip, processor clock
	 * and all memory mapped variables.
	 */
	TIMER(my_cpu)->time_delta = 0;
	TIMER(my_cpu)->time_adjusted = 0;
	lock_alloc(&(TIMER(my_cpu)->trb_lock), LOCK_ALLOC_PIN, 
			TRB_LOCK_CLASS, (short)CPUID);
	simple_lock_init(&(TIMER(my_cpu)->trb_lock));

	/*
	 * Get a systimer running
	 * Timer allocation has be done by boot master for boot slaves.
	 */
	assert(TIMER(my_cpu)->systimer != NULL);
	TIMER(my_cpu)->systimer->flags &= ~(T_ABSOLUTE);
	TIMER(my_cpu)->systimer->timeout.it_value.tv_sec = 0;
	TIMER(my_cpu)->systimer->timeout.it_value.tv_nsec = NS_PER_SEC / HZ;
	TIMER(my_cpu)->systimer->func = sys_timer;
	TIMER(my_cpu)->systimer->func_data = (long)(TIMER(my_cpu)->systimer);
	TIMER(my_cpu)->systimer->ipri = INTTIMER;

	/*
	 *  ticks_its is used to indicate when watchdog() and other
	 *  time specific routines should be called (usually once per
	 *  second).  It is initialized to 0 and incremented each time
	 *  the system wide timer's timeout routine runs.  When it
	 *  reaches HZ, it is reset to 0.
	 */
	TIMER(my_cpu)->ticks_its = 0;
	/*
         * Wait until the second to start this systimer
	 */
	curtime(&t);          /* Get real time registers low */
	ns = t.tv_nsec;
	curtime(&t);
	while (ns <= t.tv_nsec){
	  	ns = t.tv_nsec;
	  	curtime(&t);
	}
	/* It wrapt, so we got a new second, stagger systimer start per proc. */
	while (t.tv_nsec < ((NS_PER_SEC/HZ)/maxcpu)*my_cpu){
		curtime(&t);
	}

	curtime(&TIMER(my_cpu)->ref_time);
        ntimeradd(TIMER(my_cpu)->ref_time, ref_const, TIMER(my_cpu)->ref_time); 
	INIT_DEC();
	ipri = i_disable(INTMAX);
	tstart(TIMER(my_cpu)->systimer);
#ifdef _POWER_PC
	if( __power_pc() ) {
		/*
		 * Now decrementer interrupts will work so restore
		 * the vector 
		 */
		writedec_vec( dec_vec_hold );
	}
#endif /* _POWER_PC */
	i_enable( ipri );
}

/*
 * NAME: watchdog_init
 *
 * FUNCTION:
 *      This code will initialize the watchdog queues, and the watchdog lock.
 *
 * EXECUTING ENVIRONMENT:
 *      It should be called on every cpu, BUT on mp master cpu first.
 *
 * RETURN VALUE DESCRIPTION: None
 *
 */
void
watchdog_init(void)
{
  	struct watchdog *w;
	cpu_t my_cpu=CPUID;

  	TIMER(my_cpu)->w_called = NULL;
  	w = &(TIMER(my_cpu)->wtimer);
  	w->next = w;
  	w->prev = w;
  	w->func = (void (*)())brkpoint;
  	w->count = w->restart = 0;
}
