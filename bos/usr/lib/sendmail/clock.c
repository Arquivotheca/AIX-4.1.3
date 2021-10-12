static char sccsid[] = "@(#)23	1.8  src/bos/usr/lib/sendmail/clock.c, cmdsend, bos411, 9428A410j 12/20/93 10:33:23";
/* 
 * COMPONENT_NAME: CMDSEND clock.c
 * 
 * FUNCTIONS: clrallevents, clrevent, endsleep, setevent, sigmask, 
 *            sleep, startevents, stopevents, tick 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
**  Sendmail
**  Copyright (c) 1983  Eric P. Allman
**  Berkeley, California
**
**  Copyright (c) 1983 Regents of the University of California.
**  All rights reserved.  The Berkeley software License Agreement
**  specifies the terms and conditions for redistribution.
**
*/

# include <stdio.h>

# include <sys/types.h>
# include <netinet/in.h>
# include "conf.h"
# include "useful.h"

# include "sendmail.h"
# include <signal.h>

void free ();
unsigned int  alarm ();
void  tick (int);
char  *xalloc ();
long	time();
#define cur_time	time ((long *) 0)

#define sigmask(m)      (1 << ((m)-1))

/*
**  SETEVENT -- set an event to happen at a specific time.
**	        Can be called from function called from event queue.
**
**	Events are stored in a sorted list for fast processing.
**	An event only applies to the process that set it.
**
**	Parameters:
**		intvl -- intvl until next event occurs.
**		func -- function to call on event.
**		arg -- argument to func on event.
**
**	Returns:
**		none.
**
**	Side Effects:
**		none.
*/

EVENT *
setevent(intvl, func, arg)
	long intvl;
	int (*func)(int);
	int arg;
{
	register EVENT **evp;
	register EVENT *ev;
	long  then;

# ifdef DEBUG
	if (intvl <= 0)
	{
		syserr("setevent: intvl=%ld\n", intvl);
		return (NULL);
	}
# endif DEBUG

	/*
	 *  Prevent an alarm interrupt from messing with queue.
	 */
	(void) alarm (0);

	/*
	 *  Enter new queue entry at proper point in time.
	 */
	then = cur_time + intvl;	/* time for the event		*/

	for (evp = &EventQueue; (ev = *evp) != NULL; evp = &ev->ev_link)
	{
	    if (ev->ev_time >= then)	/* same or later event time?	 */
		break;
	}

	ev = (EVENT *) xalloc(sizeof (*ev));
	ev->ev_time = then;
	ev->ev_func = func;
	ev->ev_arg  = arg;
	ev->ev_pid  = getpid();
	ev->ev_link = *evp;
	*evp        = ev;

# ifdef DEBUG
	if (tTd(5, 5))
	    (void) printf ("setevent: intvl=%ld then=%ld func=0x%x arg=%d ev=0x%x\n",
			                         intvl, then, func, arg, ev);
# endif DEBUG

	/*
	 *  (Re)start the alarm period for what's on top of the event queue.
	 *  It is also possible that something is now ready whose pending alarm
	 *  got cancelled when this function was entered.  The new entry may
	 *  even be ready.
	 */
	tick (0);

	return (ev);
}
/*
**  CLREVENT -- remove an event from the event queue.
**	        Can be called from function called from event queue.
**
**	Parameters:
**		ev -- pointer to event to remove.
**
**	Returns:
**		none.
**
**	Side Effects:
**		arranges for event ev to not happen.
*/

clrevent(ev)
	register EVENT *ev;
{
	register EVENT **evp;

# ifdef DEBUG
	if (tTd(5, 5))
		(void) printf("clrevent: ev=%x\n", ev);
# endif DEBUG
	if (ev == NULL)
		return;

	/*
	 *  Prevent alarm interrupt from messing with queue.
	 */
	(void) alarm (0);

	/*
	 *  Find the event and remove it
	 */
	for (evp = &EventQueue; *evp != NULL; evp = &(*evp)->ev_link)
	{
		if (*evp == ev)
			break;
	}

	if (*evp != NULL)
	{
		*evp = ev->ev_link;
		free((char *) ev);
	}

	/*
	 *  (Re)start the alarm period for what's on top of the event queue.
	 *  It is also possible that something is now ready whose pending alarm
	 *  got cancelled when this function was entered.
	 */
	tick (0);
}
/*
**  CLRALLEVENTS -- remove all events from the event queue.
**	            Can be called from function called from event queue.
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		removes all events from the queue.
*/

clrallevents ()
{
	register EVENT *evp, *evt;

# ifdef DEBUG
	if (tTd(5, 5))
		(void) printf("clrallevent\n");
# endif DEBUG

	/*
	 *  Prevent alarm interrupt from messing with queue.
	 */
	(void) alarm (0);

	/*
	 *  Remove all events on the queue.
	 */
	evp = EventQueue;
	while (evp != NULL)
	{
	    evt = evp;
	    evp = evt->ev_link;
	    free ((char *) evt);
	}
}
/*
**  STOPEVENTS -- stop events on the queue from occuring.
**	          Not effective if called from event worker function.
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		shuts off alarm subsystem.  It can be restarted.
*/

stopevents ()
{
    (void) alarm (0);
}
/*
**  STARTEVENTS -- start the event queue.
**	           Not effective (or to be called) from event worker function.
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		starts event queue processing.
*/

startevents ()
{
    tick (0);
}
/*
**  TICK -- run the event queue.  Set up next an alarm for the first thing not 
**	    runnable immediately.  Cannot be entered recursively from signal
**	    handling, since alarm signal is not enabled until just before exit.
**	    Also, cannot be entered recursive from event worker functions since
**	    this is prevented by flag.
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		calls the next function in EventQueue.
*/

void 
tick(int s)
{
	register long now;
	register EVENT *ev;
	int  mypid;
	struct sigvec sigin;
	static int  stackl = 0;		/* tick nesting controller	*/

	/*
	 *  NOTE:  Event queueing has been slightly simplified over previous
	 *  versions of sendmail.
	 *
	 *  If tick is called from set/clrevent, then alarm (0) has already
	 *  cleared out any possibility of an alarm.
	 *
	 *  If tick is called from signal delivery, then another alarm can't
	 *  occur then, either, until enabled below.
	 *
	 *  We should not have interrupted the processing of a function called 
	 *  from a queue entry below, because an alarm is not enabled in that
	 *  case.  Even if future developments allow this, its OK, since the 
	 *  alarm handler instances are stacked.  What's running when an alarm
	 *  signal occurs would continue after the new instance of tick ceased 
	 *  to exist.
	 */

	/*
	 *  Don't enter tick recursively.  This should only occur when
	 *  an event worker function calls one of the other entry points
	 *  in this file.  The lowest level instantiation of tick is able
	 *  to process every queue entry.
	 */
	if (stackl)			/* don't enter recursively	*/
	    return;

	stackl = 1;			/* set entry indicator		*/

	mypid = getpid();
	now = cur_time;

# ifdef DEBUG
	if (tTd(5, 4))
		(void) printf("tick: enter: now = %ld, pid = %d\n", now, mypid);
# endif DEBUG

	/*
	 *  Scan ordered queue from top down doing the things that are ready
	 *  and purging the things that are not ours.  Queue entries may exist
	 *  from parents after forking.
	 *
	 *  The exact form of the conditional clause below interacts optimally
	 *  with the test for setting the alarm after the end of the while loop.
	 */
	while ((ev = EventQueue) != NULL &&
	       (ev->ev_time <= now || ev->ev_pid != mypid))
	{
		int (*f)(int);
		int arg;
		int pid;

		/*
		 *  Pluck top event from event queue.
		 */
		ev = EventQueue;
		EventQueue = EventQueue->ev_link;
# ifdef DEBUG
		if (tTd(5, 6))
		    (void) printf("tick: event: ev=%x, func=%x, arg=%d, pid=%d",
				       ev, ev->ev_func, ev->ev_arg, ev->ev_pid);
# endif DEBUG

		/*
		 *  Get info from queue entry and free memory.
		 */
		f = ev->ev_func;
		arg = ev->ev_arg;
		pid = ev->ev_pid;
		free((char *) ev);

		/*
		 *  If this is not for us (remember forking may leave
		 *  active queue entries that are for one of our parents)...
		 */
		if (pid != mypid)
		{
# ifdef DEBUG
		    if (tTd (5, 6))
			(void) printf ("  (purge)\n");
# endif DEBUG
		    continue;
		}

# ifdef DEBUG
		if (tTd (5, 6))
		    (void) printf ("  (dispatch)\n");
# endif DEBUG

		/*
		 *  Go do the function.  Some of these event functions used
		 *  to perform a longjmp, but these have been changed to
		 *  not do this any more.  All event functions should return.
		 *  The alarm is not enabled during the execution of an event
		 *  function.
		 *
		 *  An event function should do its thing in a well structured
		 *  manner and return.  No messing with alarms, except via
		 *  the event queue entry points described elsewhere in this
		 *  file!
		 */
		(*f) (arg);

		/*
		 *  Update current time.
		 */
		now = cur_time;
# ifdef DEBUG
		if (tTd (5, 6))
		    (void) printf ("tick: (return)\n");
# endif DEBUG
	}

	/*
	 *  Set flag indicating we're not in tick.
	 */
	stackl = 0;

# ifdef DEBUG
	if (tTd (5, 6))
	    (void) printf ("tick: exit\n");
# endif DEBUG

	/*
	 *  Now that we are out of ready entries, set up alarm signal for
	 *  time till next queue entry, if any.  This is the only place
	 *  that the alarm is enabled.
	 *
	 *  In the test below, the interval ev_time - now must be > 0
	 *  due to the way the loop control tests work above.
	 */
	if (EventQueue != NULL)
	{
	    sigin.sv_handler = tick;
	    sigin.sv_mask = 0;
	    sigin.sv_flags = SV_INTERRUPT;
	    if(sigvec(SIGALRM, &sigin, (struct sigvec *) 0))
		perror("tick: sigvec failure");
# ifdef DEBUG
	    if (tTd (5, 6))
		(void) printf ("tick: alarm (%d)\n", EventQueue->ev_time - now);
# endif DEBUG
	    (void) alarm ((unsigned) (EventQueue->ev_time - now));
	}
	/*
	 *  Entry to "tick" via signal handler from this point on to exit
	 *  doesn't affect anything.  Tick still remains logically nonrecursive.
	 */
}
/*
**  SLEEP -- a version of sleep that works with this stuff
**
**	Because sleep uses the alarm facility, I must reimplement
**	it here.
**
**	Parameters:
**		intvl -- time to sleep.
**
**	Returns:
**		none.
**
**	Side Effects:
**		waits for intvl time.  However, other events can
**		be run during that interval.
*/

static int  sleepdone;
static int  endsleep ();

sleep(intvl)
	unsigned int intvl;
{
	int  mask;

	if (intvl == 0)
		return;

	/*
	 *  Clear done flag before registering for "interrupt".
	 */
	sleepdone = FALSE;

	/*
	 *  Register for alarm signal which will set sleepdone.
	 */
	(void) setevent((long) intvl, endsleep, 0);

	/*
	 *  Loop waiting for sleepdone to be true.
	 *
	 *  This is traditionally done with the statements:
	 *
	 *     while (!sleepdone)  pause ();
	 *
	 *  However, this leaves a window between the testing of sleepdone
	 *  and the pause system call.  The flag might get set in the window
	 *  and then the pause () would not return until the next (possibly
	 *  unrelated) signal, if any.  Therefore, alarms must be blocked
	 *  while the flag is checked, up until a special sigpause call.
	 *  Sigpause sets a new mask (to allow alarms in our case) and
	 *  performs the pause as a UNITARY operation.  Also, when exiting, 
	 *  sigpause restores the mask as it was before the sigpause call
	 *  (again disabling alarm signals in our case).
	 */
	mask = sigblock (sigmask (SIGALRM)); /* save prev and disable alrm */

	while (1)
	{
	    /*
	     *  One system design problem here is that the sigpause to
	     *  restore the mask may not restore a mask that has been
	     *  asynchronously changed elsewhere in the program (another
	     *  signal handler).  It just turns out that this isn't done in 
	     *  sendmail.
	     */
	    if (sleepdone)
		break;
# ifdef DEBUG
	    if (tTd (5, 6))
	        (void) printf ("sleep: sigpause (0x%x)\n", mask);
# endif DEBUG
	    (void) sigpause (mask);
	}

	/*
	 *  Always leave mask cleared to allow alarms.
	 *
	 *  The same system design problem mentioned above in relation
	 *  to sigpause applies here.
	 */
# ifdef DEBUG
	if (tTd (5, 6))
	    (void) printf ("sleep: sigsetmask (0x%x)\n", mask);
# endif DEBUG
	(void) sigsetmask (mask);
}

static
endsleep()
{
	sleepdone = TRUE;
}
