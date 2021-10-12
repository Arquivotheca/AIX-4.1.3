static char sccsid[] = "@(#)75  1.17  src/bos/kernel/lib/libsys/timeout.c, libsys, bos41J, 9515B_all 4/7/95 11:23:35";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: timeout
 *		timeout_end
 *		timeoutcf
 *		untimeout
 *
 *   ORIGINS: 3,27,83
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>	/* always gotta have this one			*/
#include <sys/param.h>	/* to define the number of clock ticks/sec (HZ)	*/
#include <sys/syspest.h>/* to define the assert and ASSERT macros	*/
#include <sys/timer.h>	/* For the timer related defines		*/
#include <sys/intr.h>	/* for the serialization defines		*/
#include <sys/malloc.h>	/* for the parameters to xmalloc()		*/
#include <sys/lock_def.h>

/*
 * IMPORTANT MP PORTING NOTE :
 *
 * These functions have not been properly architected to be used in
 * MP environments.  Whenever possible one should use the base timer
 * kernel primitives : talloc, tstart, tstop, and tfree.  The kernel
 * also provides the following watchdog routines : w_init, w_stop,
 * w_clear, and w_start.
 *
 * The design flaw is that the caller may be holding a lock that is
 * needed by the callout handler when calling these routines.  If the
 * timer pops while one of these routines is in progress a deadlock
 * may occur, because we don't know the address of the callers lock 
 * to release and reacquire.  We will just loop in one of the 
 * following routines.  
 *
 * This possibility did not exist in the original uni-processor code, 
 * because interrupt locks were not used and it is likely to be
 * introduced when porting to MP, because of the general guideline of
 * translating i_disable to disable_lock calls.  Anybody that falls 
 * into this trap should rewrite their code to use the kernel 
 * primitives listed above.
 *
 * IBM code should not use these primitives.
 *
 * In retrospect we should have changed untimeout to return an error
 * much like we did with tstop.
 */

/* 
 * Instrumentation is not possible, because there is no initialization 
 * routine that can be called.
 */
Simple_lock callout_lock = { SIMPLE_LOCK_AVAIL };

struct	tos	{
	struct	tos	*toprev;	/* previous tos in callout table*/
	struct	tos	*tonext;	/* next tos in callout table	*/
	struct	trb	*trb;		/* this timer request block	*/
};

struct	callo	{
	int	ncallo;		/* number of callout table elements	*/
	struct	tos	*head;	/* callout table head element		*/
};

struct	callo callo = {0, NULL};/* callout table anchor			*/

static void	timeout_end(struct trb *);  /* timeout()'s timeout function */
extern int 	tstop(struct trb *);
extern void	tstart(struct trb *);
extern void	i_enable(int);


/*
 * NAME:  timeout
 *
 * FUNCTION:  Arrange that the specified function is called with the
 *	specified argument in the specified number of ticks.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine can be called under either a process or an interrupt level.
 *
 *      It does not page fault except when called on a pageable stack.
 *
 * NOTES:  Callers of this routine are expected to have called timeopen()
 *	to create that caller's callout table.
 *
 * RECOVERY OPERATION:
 *
 * RETURNS:  timeout() has no returned value.
 *
 * EXTERNAL PROCEDURES CALLED:	disable_lock
 *				unlock_enable
 */
void
timeout(
	register void	 (*func)(),		/* function to call at timeout*/
	register caddr_t arg,			/*   It's argument. */
	register int	 ticks)			/* when to set timeout for */
{
	register int	ipri;		/* caller's interrupt priority	*/
	register struct tos *tos;	/* tos to use for timeout	*/
	register struct trb *trb;	/* trb in the tos being used	*/
	struct itimerstruc_t tv;	/* timeout interval		*/

	tv.it_value.tv_sec  =  ticks / HZ;
	tv.it_value.tv_nsec = (ticks % HZ) * (NS_PER_SEC / HZ);

	assert(callo.ncallo != 0);

timeout_retry:

	ipri = disable_lock(INTMAX, &callout_lock);

	/*
	 *  Run the callout table chain to see if there is already a pending
	 *  timeout for the specified function.  If so, that timeout will
	 *  be cancelled and the tos re-used.
	 */
	for(tos = callo.head; tos != NULL; tos = tos->tonext)  {
		if((tos->trb->tof == func) && (tos->trb->func_data == (int)arg))
			break;
	}

	/*
	 *  If a pending timeout for the specified function was NOT found,
	 *  then the callout table chain will have to be run to find an
	 *  unused tos.
	 */
	if(tos == NULL) {
		for(tos = callo.head; tos != NULL; tos = tos->tonext)  {
			if(tos->trb->tof == NULL)  {
				break;
			}
		}

		/*
		 *  If there isn't an available tos, then there is no error
		 *  recovery.  This means that either the caller has not
		 *  correctly registered the number of callout table entries
		 *  that would be needed or is incorrectly using the ones that
		 *  were registered.  Either way, panic is the only recourse.
		 */
		assert(tos != NULL);
	}

	/* 
	 *  A pending timeout for the specified function WAS found.
	 *  If the request is still active, stop it.
	 */
	while (tstop(tos->trb)) {
		unlock_enable(ipri, &callout_lock);
		goto timeout_retry;
	}

	tos->trb->knext		= NULL;
	tos->trb->kprev		= NULL;
	tos->trb->flags		= 0;
	tos->trb->timeout	= tv;
	tos->trb->tof		= func;
	tos->trb->func		= (void (*)()) timeout_end;
	tos->trb->func_data	= (ulong) arg;
	tos->trb->ipri		= INTTIMER;
	tos->trb->id		= -1;

	tstart(tos->trb);

	unlock_enable(ipri, &callout_lock);
}


/*
 * NAME:  untimeout()
 *
 * FUNCTION:  Simulate the old UNIX-style untimeout() kernel routine.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine can be called under either a process or an interrupt level.
 *
 *      It does not page fault except when called on a pageable stack.
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * RETURNS:  untimeout() has no returned value.
 *
 * EXTERNAL PROCEDURES CALLED:	disable_lock
 *				unlock_enable
 */
void
untimeout(
register void	(*func)(),
register ulong	arg)
{
	register int	ipri;		/* caller's interrupt priority	*/
	register struct tos *tos;	/* tos to walk callout table	*/
	register struct trb *trb;	/* trb for this tos		*/

untimeout_retry:

	ipri = disable_lock(INTMAX, &callout_lock);

	/*  Run the callout table chain looking for the timeout.  */
	for(tos = callo.head; tos != NULL; tos = tos->tonext)  {
		if(tos->trb->tof == func && tos->trb->func_data == arg)  {
			break;
		}
	}

	if(tos)  {
		/*
		 *  Found it on the timeout list - stop the pending timeout 
		 *  if it is active.
		 */
		while(tstop(tos->trb)) {
			unlock_enable(ipri, &callout_lock);
			goto untimeout_retry;
		}

		/*  Mark this callout table entry as free.  */
		tos->trb->knext = NULL;
		tos->trb->kprev = NULL;
		tos->trb->tof = NULL;
	}

	unlock_enable(ipri, &callout_lock);
}


/*
 * NAME:  timeout_end
 *
 * FUNCTION:  Call the function associated with the current timeout.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called on the clock interrupt level.
 *	It is static.
 *
 *      It does not page fault.
 *
 * RECOVERY OPERATION:
 *
 * RETURNS:  timeout_end() has no returned value.
 *
 * EXTERNAL PROCEDURES CALLED:
 */
static void
timeout_end(
struct trb *trb)			/* trb of the current timeout	*/
{
	register void	 (*func)();	/* function to call at timeout	*/
	int ipri;

	func = trb->tof;

	ipri = disable_lock(INTMAX, &callout_lock);
	trb->func = NULL;
        trb->tof = NULL;            /* Zero out pointer to user function  */
	unlock_enable(ipri, &callout_lock);

	(* func)(trb->func_data);
                                    /* for compatibility with untimeout() */
}

/*
 * NAME:  timeoutcf
 *
 * FUNCTION:  Configure (increase or decrease) the caller's callout table
 *	for use with timeout().
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine can only be called under a process.
 *
 *      It may page fault.
 *
 * NOTES:  If the caller calls timeoutcf(-x), then there must be at least
 *	x callout table entries WHICH ARE NOT CURRENTLY SET TO TIMEOUT. 
 *	Otherwise, panic will result.
 *
 * RECOVERY OPERATION:
 *
 * RETURNS:  0 upon successful allocation/deallocation of the callout
 *	table element, -1 otherwise.
 *
 * EXTERNAL PROCEDURES CALLED:  talloc
 *				tfree
 */
int
timeoutcf(
	register int	cocnt)		/* # entries to change callout table by	*/
{
	register int	ipri;		/* caller's interrupt priority	*/
	register int	rv;	/* return value to the caller		*/
	register struct tos *tos; /* tos to add to/remove from table	*/
	register struct trb *trb; /* trb in the tos to be added/removed	*/

	rv = 0;

	if(cocnt > 0)  {
		/*
		 *  Callout table is being enlarged - keep working until the
		 *  right number of elements have been added.
		 */
		while(cocnt > 0)  {
			/*  Allocate a timer request block.  */
			trb = (struct trb *) talloc();

			/*
			 *  If the low-level timer service could not provide
			 *  a trb, the callout table can't be expanded any
			 *  more so get out.
			 */
			if(trb == NULL)  {
				rv = -1;
				break;
			}

			/*  Allocate memory for the callout table structure.  */
			tos = (struct tos *)
				xmalloc((uint)sizeof(struct tos), (uint)0, pinned_heap);

			/*
			 *  If memory couldn't be allocated for the tos, the
			 *  callout table can't be expanded any more so get out.
			 */
			if(tos == NULL)  {
				rv = -1;
				break;
			}
			else  {
				bzero(tos, sizeof(struct tos));
			}

			/*  The trb and the tos were both allocated.  */
			tos->trb = trb;
#ifdef DEBUG
			/* 
			 *  Debug code to ensure that the low-level timer 
			 *  service talloc() clears out the pointers.
			 */
			ASSERT(trb->knext == NULL);
			ASSERT(trb->kprev == NULL);
#endif /* DEBUG */

			ipri = disable_lock(INTMAX, &callout_lock);
			if(callo.head == NULL)  {
				/*
				 *  The callout table is currently empty.  This
				 *  is the easy case, just set the head of the
				 *  callout chain to this tos.
				 */
				callo.head = tos;
			}
			else  {
				/*
				 *  The callout table is not empty.  Chain this
				 *  trb to the head of the callout chain.
				 */
				tos->tonext = callo.head;
				callo.head->toprev = tos;
				callo.head = tos;
			}

			/*  Just finished adding a trb to the callout table.  */
			callo.ncallo++;
			cocnt--;
			unlock_enable(ipri, &callout_lock);
		}
	}
	else  {
		/*
		 *  Callout table is being shrunk - keep working until the
		 *  right number of elements have been removed being careful
		 *  only to remove elements which do not belong to timeout
		 *  requests that are currently active.
		 */
		if(cocnt < 0)  {

			/*
			 *  There had better be at least as many tos's in
			 *  the callout table as the size by which the caller 
			 *  wants to decrease the size of the table.
			 */
			assert(callo.ncallo >= -cocnt);

			while(cocnt < 0)  {

timeoutcf_retry:
				/*
				 *  Start from the head of the callout chain,
				 *  making sure that there is a tos at the 
				 *  head (i.e. that there is a callout chain).
				 */
				ipri = disable_lock(INTMAX, &callout_lock);
				tos = callo.head;
				assert(tos != NULL);

				/*
				 *  Keep walking down the callout chain until
				 *  a tos is found which is not currently 
				 *  active.
				 */
				while((tos != NULL) && 
				      (tos->trb->tof != NULL))  {
					tos = tos->tonext;
				}

				/*
				 *  If trb is not NULL, then there was not a
				 *  callout table entry that wasn't set to
				 *  timeout.  Panic.
				 */
				assert(tos != NULL);

                		/*
                 		 *  Stop the pending timeout if it is active.
                 		 */
                		while (tstop(tos->trb)) {
					unlock_enable(ipri, &callout_lock);
					goto timeoutcf_retry;
				}

				/*
				 *  Found a free callout table element, free
				 *  it and remove it from the callout table.
				 */
				tfree(tos->trb);
				if(callo.head == tos)  {
					callo.head = tos->tonext;
					if(callo.head != NULL)  {
						callo.head->toprev = NULL;
					}
				}
				else  {
					assert(tos->toprev != NULL);
					tos->toprev->tonext = tos->tonext;
					if(tos->tonext != NULL)  {
						tos->tonext->toprev =
							tos->toprev;
					}
				}
				/*
				 *  Just finished removing a trb from the
				 *  callout table.
				 */
				callo.ncallo--;
				cocnt++;
				unlock_enable(ipri, &callout_lock);
				xmfree((void *)tos, pinned_heap);

			}
		}
	}

	return(rv);
}
