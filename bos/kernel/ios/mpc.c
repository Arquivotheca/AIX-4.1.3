static char sccsid[] = "@(#)00  1.6  src/bos/kernel/ios/mpc.c, sysios, bos41J, 9511A_all 3/7/95 13:13:13";
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS: 
 *	mpc_send,	mpc_register
 *	cs_mpc_init,	cs_mpc_offlevel,	cs_mpc_handler,
 *	cs_mpc_issue
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#include <sys/malloc.h>
#include <sys/intr.h>
#include <sys/mstsave.h>
#include <sys/ppda.h>
#include <sys/syspest.h>
#include <sys/low.h>
#include <sys/mpc.h>
#include <sys/lock.h>
#include <sys/systemcfg.h>
#ifdef _POWER_MP
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#endif /* _POWER_MP */

struct mpc_reg	mpc_reg_array[sizeof(int)*NBBY];
					/* registered mpc services array */
struct intr	mpc_hand;		/* mpc interrupt handler  */
uint		mpc_offlvl_mask=0;	/* mpc running at offlvl intpri */
uint            mpc_iodone_mask=0;      /* mpc running at offlvl intpri */
uint            mpc_timer_mask=0;       /* mpc running at INTTIMER */
Simple_lock	mpc_lock;		/* lock on mpc_reg_array update */ 
uint		mpc_max_index=0;	/* mpc service high water mark	*/

/*
 * NAME:  mpc_register
 *
 * FUNCTION:  
 *	register a mpc service 
 *	
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called by either a process or an
 *      interrupt handler.
 *
 *      It can page fault if called from a process on a pageable stack.
 *
 * NOTES:  
 *
 * RETURN VALUE DESCRIPTION:  mpc service number
 *
 * EXTERNAL PROCEDURES CALLED:
 *	disable_lock
 *	unlock_enable
 *	assert
 */
mpc_msg_t
mpc_register(uint priority, void (*function)(void))
{
#ifdef _POWER_MP
	int i, regist, ipri;

	/* check priority value */
	ASSERT(((priority==INTMAX) || 
		(priority==INTTIMER) || (priority==INTIODONE)));

	regist = FALSE;

	ipri = disable_lock( INTMAX, &mpc_lock);

	for (i=0;i<sizeof(int)*NBBY;i++)
		if (mpc_reg_array[i].func == NULL) {
			if (priority!=INTMAX) mpc_offlvl_mask |= (1<<i);
			if (priority==INTTIMER) mpc_timer_mask |= (1<<i);
			if (priority==INTIODONE) mpc_iodone_mask |= (1<<i);
			mpc_max_index++;
			mpc_reg_array[i].func = function;
			mpc_reg_array[i].pri  = priority;
			regist = TRUE;
			break;
		}

	unlock_enable( ipri, &mpc_lock);

	assert(!(regist == FALSE));

	return((mpc_msg_t)i);
#else
	assert(0);
#endif /* _POWER_MP */
}

/*
 * NAME:  mpc_send
 * FUNCTION:  
 *	send a mpc to another processor 
 *	
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called by either a process or an
 *      interrupt handler.
 *
 *      It can page fault if called from a process on a pageable stack.
 *
 * NOTES:  
 *
 * RETURN VALUE DESCRIPTION:  void
 *
 * EXTERNAL PROCEDURES CALLED:
 *	i_disable
 *	i_enable
 *	CPUID
 *	fetch_and_or
 *	i_soft
 *	assert
 */
void
mpc_send(cpu_t cpuid, mpc_msg_t msg)
{
#ifdef _POWER_MP
	int ipri, cpu, mycpu;

	ipri = i_disable(INTMAX);

	mycpu = CPUID;

	/* check if it's a registered mpc service */
	ASSERT(!((msg >= sizeof(int)*NBBY) 
		 || (mpc_reg_array[(uint)msg].func == NULL)));

	/* check if cpuid is a valid logical cpu */
	ASSERT (!(mycpu==cpuid));

	if ( cpuid == MPC_BROADCAST) {
		for (cpu=0;cpu<NCPUS();cpu++) {
			if (cpu!=mycpu) {
			  fetch_and_or(&(GET_PPDA(cpu)->mpc_pend),1<<(uint)msg);
			  __iospace_sync();
			  i_soft(INTMAX, cpu);
			}
		}
	} else {
		fetch_and_or(&(GET_PPDA(cpuid)->mpc_pend),1<<(uint)msg);
		__iospace_sync();
		i_soft(INTMAX, cpuid);
	}

	i_enable(ipri);
#else
	assert(0);
#endif /* _POWER_MP */
}

#ifdef _POWER_MP

/* The following code implements a synchronous MPC where the issuer will
 * wait indefinitely for positive acknowledgement from all other CPUs before
 * returning.  Originally designed for the VMM to invalidate segment registers
 * on other processors, its main purpose is to cause a synchronous (forced)
 * context switch.  When every other processor in the complex returns to the
 * user address space (from servicing the MPC and running this offlevel), the
 * user mode segment registers will be reloaded.
 */

#include <sys/atomic_op.h>		/* fetch_and_???() */

/* The MPC handle for the context synchronizing MPC.  This is written to
 * once during initialization (by MP_MASTER), and only read thereafter.
 */

mpc_msg_t	cs_mpc_id = -1;

/* There is a timing window between when the MPC receiving processor clears
 * ppda.cs_sync and the MFRR interrupt is cleared by the interrupt sub-
 * system.  During this time, another processor may increment cs_sync and
 * post an interrupt again.  This interrupt will be lost since the MFRR can't
 * queue such requests.  As a result, clearing of ppda.cs_sync is done by an
 * offlevel which runs after the MFRR has been cleared, and thus the window
 * is closed.  An offlevel can be scheduled per processor with these intr
 * structures.  (scheduling more than once has no side effects)
 */

struct intr	cs_intr[MAXCPU] = { 0 };


#ifdef DEBUG
/* Statistics on the number of forced synchronous context switches.
 * On early SMP hardware, cache inconsistencies may cause these to drift
 * from their correct values.
 */

volatile int	cs_istat[MAXCPU] = { 0 };	/* MPC issue   */
volatile int	cs_hstat[MAXCPU] = { 0 };	/* MPC handler */
volatile int	cs_ostat[MAXCPU] = { 0 };	/* offlevel    */
#endif

/*
 * NAME:  cs_mpc_init
 *
 * FUNCTION:  Initialize the context synchronizing MPC for SID Shootdown
 *
 * EXECUTION ENVIRONMENT:  This routine is called during system initialization
 *	via a pointer-to-function in an array defined in <sys/init.h>
 *
 * NOTES:  This function will run exactly once on the MP_MASTER processor
 *	when the kernel initializes.
 *
 * DATA STRUCTURES:  Sets global MPC handle cs_mpc_id, used to send MPCs.
 *	Clears ppda.cs_sync for every CPU possible CPU in the complex.
 *
 * RETURN VALUE DESCRIPTION:  This procedure does not return a value.
 *
 * EXTERNAL PROCEDURES CALLED:  my_ppda() via CPUID macro
 *                              mpc_register()
 *                              fetch_and_and()
 *				bzero()
 */ 

void	cs_mpc_init()
{
cpu_t		 cpu_id;
extern void	 cs_mpc_handler(void);
extern int	 cs_mpc_offlevel(void);

	cpu_id = CPUID;
	if( cpu_id == MP_MASTER )
	{
		assert( cs_mpc_id == (mpc_msg_t)-1 );

		bzero( cs_intr, sizeof(struct intr) * MAXCPU );

		/* Clear the MPC status word for each possible processor
		 */
		for( cpu_id = 0; cpu_id < MAXCPU; cpu_id += 1 )
		{
			(void)fetch_and_and( &ppda[cpu_id].cs_sync, 0 );

			cs_intr[cpu_id].handler  = cs_mpc_offlevel;
			cs_intr[cpu_id].priority = INTOFFL3;
#ifdef DEBUG
			(void)fetch_and_and( &cs_istat[cpu_id], 0 );
			(void)fetch_and_and( &cs_hstat[cpu_id], 0 );
			(void)fetch_and_and( &cs_ostat[cpu_id], 0 );
#endif /* DEBUG */
		}

		/* mpc_register never returns a failure: it asserts
		 */
		cs_mpc_id = mpc_register( INTMAX, cs_mpc_handler );
	}
	return;
}

/*
 * NAME:  cs_mpc_offlevel
 *
 * FUNCTION:  Clear the executing CPU's context synchronization word.
 *	You must clear, not decrement, the sync word because this runs
 *	at INTOFFL3 and multiple MPCs per processor or MPCs by multiple
 * 	processors may have been issued.  Also because this may run
 *	when the count is already zero because the offlevel was i_sched'ed
 *	again (from INTMAX) while it was running.
 *
 * EXECUTION ENVIRONMENT:  This routine is scheduled at INTOFFL3 by
 *	the INTMAX MPC handler on every other CPU when a processor in the
 *	complex calls mpc_send with type MPC_BROADCAST.  This routine will
 *	run on the same physical CPU that scheduled it - guaranteed.
 *
 * DATA STRUCTURES:  ppda.cs_sync on the local processor
 *
 * RETURN VALUE DESCRIPTION:  This procedure always returns INTR_SUCC (0).
 *
 * EXTERNAL PROCEDURES CALLED:  my_ppda() via CPUID macro
 *                              fetch_and_and()
 */ 

int	cs_mpc_offlevel()
{
cpu_t	local_cpu = CPUID;

	/* Clear the MPC status word for this processor
	 */
	(void)fetch_and_and( &ppda[local_cpu].cs_sync, 0 );

#ifdef DEBUG
	(void)fetch_and_add( &cs_ostat[local_cpu], 1 );
#endif
	return( INTR_SUCC );
}

/*
 * NAME:  cs_mpc_handler
 *
 * FUNCTION:  Schedules cs_mpc_offlevel for the running processor.
 *	The offlevel will run on this, the issuing CPU, from the back
 *	end of the interrupt subsystem.
 *
 * EXECUTION ENVIRONMENT:  This routine is called by the MPC mechanism on
 *	every other processor when a processor in the complex calls
 *	mpc_send with type MPC_BROADCAST.  Executed at the processor
 *	priority used to register the MPC above.  Cannot page fault.
 *
 * DATA STRUCTURES:  none
 *
 * RETURN VALUE DESCRIPTION:  This procedure does not return a value.
 *
 * EXTERNAL PROCEDURES CALLED:  my_ppda() via CPUID macro
 *                              fetch_and_add()
 *				i_sched()
 */ 

void	cs_mpc_handler()
{
cpu_t	local_cpu = CPUID;

	/* Schedule the offlevel which clears cs_sync after the
	 * MFRR has been cleared from our MPC.
	 */
	i_sched( &cs_intr[local_cpu] );

	/* Clear the MPC status word for this processor.
	 * You must still run the offlevel because of the hardware timing
	 * window, but this can accelerate the poll loop on the issuer.
	 */
	(void)fetch_and_and( &ppda[local_cpu].cs_sync, 0 );

#ifdef DEBUG
	(void)fetch_and_add( &cs_hstat[local_cpu], 1 );
#endif
	return;
}

/*
 * NAME:  cs_mpc_issue
 *
 * FUNCTION:  Issue a context-synchronizing MPC to the processor complex
 *
 * EXECUTION ENVIRONMENT:  This routine is called during from as_det on
 *	multiprocessor systems only for multi-threaded processes.
 *	(munmap() and shmdt())  There is a portion of this function where
 *	you must not change CPUs, i.e. be non-preemptable.  This is accom-
 *	plished by disabling to INTPAGER.  Because the offlevel runs
 *	at INTOFFL3, any two CPUs could deadlock waiting for each other to
 *	acknowledge their MPC.  For this reason, there is a fetch_and_nop/
 *	fetch_and_and function pair for the local CPU.
 *
 * DATA STRUCTURES:  Increments all but the executing CPU's context
 *      synchronization word in the ppda field cs_sync.
 *      References global (number_of_cpus) via NCPUS macro.
 *
 * RETURN VALUE DESCRIPTION:  This procedure does not return a value.
 *
 * EXTERNAL PROCEDURES CALLED:  my_ppda() via CPUID macro
 *                              fetch_and_add()
 *                              fetch_and_and()
 *                              fetch_and_nop()
 *				mpc_send()
 *				i_enable(), i_disable()
 */ 

void	cs_mpc_issue()
{
cpu_t		 local_cpu;
cpu_t		 cpu_id;
unsigned int	 ncpus;
int		 ipri;

	if( ( cs_mpc_id == -1 ) || ( (ncpus = NCPUS()) == 1 ) )
		return;

	ipri = i_disable( INTPAGER );		/* disable preemption */

	local_cpu = CPUID;			/* after disabling... */
#ifdef DEBUG
	(void)fetch_and_add( &cs_istat[local_cpu], 1 );
#endif

	/* Increment the MPC status word for each other processor
	 */
	for( cpu_id = 0; cpu_id < ncpus; cpu_id += 1 )
	    if( cpu_id != local_cpu )
		(void)fetch_and_add( &ppda[cpu_id].cs_sync, 1 );

	/* Notify all other processors in the complex
	 */
	mpc_send( MPC_BROADCAST, cs_mpc_id );

	/* Wait until each other processor acknowledges the MPC
	 */
	for( cpu_id = 0; cpu_id < ncpus; cpu_id += 1 )
	    if( cpu_id != local_cpu )
		while( fetch_and_nop( &ppda[cpu_id].cs_sync ) != 0 )
		    if( fetch_and_nop( &ppda[local_cpu].cs_sync ) != 0 )
			(void)fetch_and_and( &ppda[local_cpu].cs_sync, 0 );

	(void)i_enable( ipri );
	return;
}
#endif /* _POWER_MP */
