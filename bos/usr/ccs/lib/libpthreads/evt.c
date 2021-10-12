static char sccsid[] = "@(#)06	1.11  src/bos/usr/ccs/lib/libpthreads/evt.c, libpth, bos41J, 9522A_all 5/30/95 02:52:37";
/*
 * COMPONENT_NAME: libpth
 * 
 * FUNCTIONS:
 *	_event_wait	
 *	_event_waitabs
 *	
 * ORIGINS:  71, 83
 * 
 * LEVEL 1, 5 Years Bull Confidential Information
 *
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.2
 */

/*
 * File: evt.c
 *
 */

#include "internal.h"
#include <sys/m_param.h>

#define MILLISECONDS_TO_TICK(ms)     ((((ms) + ((1000/HZ) - 1)) * HZ) / 1000)
#define SECONDS_TO_MILLISECONDS(s)      ((s) * 1000)
#define NANOSECONDS_TO_MILLISECONDS(n)  (((n) + 999999) / 1000000)
#define	SECONDS_TO_NANOSECONDS(s)	((s) * 1000000000)
#define	MICROSECONDS_TO_NANOSECONDS(m)	((m) * 1000)

#define	USPS	1000000		/* micro seconds per second */
#define FOREVER 0


/*
 * Function:
 *	_event_wait
 *
 * Parameters:
 *	lock - the lock structure of the calling thread.
 *	message - a pointer to where to store the event message
 *	timeout - a pointer to a relative timeout value. This may be NULL.
 *
 * Description:
 *	The action of waiting for an event.
 *	There may be a timeout specified in relative time in the struct 
 *	timespec. If this timeout is zero then the call waits for a minimum 
 *	period of time.
 */
void
_event_wait(spinlock_t *lock, int *message, const struct timespec *timeout)
{
	int	wait;
	int	status;
	long long int wait1;
	long long int wait2;


	/* Convert relative timeout to microsec delay.
	 */
	if (timeout != NO_TIMEOUT) {
                wait1 = SECONDS_TO_MILLISECONDS(timeout->tv_sec) +
                        NANOSECONDS_TO_MILLISECONDS(timeout->tv_nsec);
		wait2 = MILLISECONDS_TO_TICK(wait1);
		wait = wait2;
		if (wait == 0)
			wait = 1;

	} else
		wait = FOREVER;

	/* Wait for the event. When it arrives check for a timeout, if
	 * not the event is the message id.
	 */
	if (lock) {
			/* thread_tsleep done _clear_lock(lock) */
		status = thread_tsleep(wait, (atomic_p)lock, NULL);
	} else
		status = thread_tsleep(wait, NULL, NULL);
	if (status == 0)
           *message = EVT_TIMEOUT; /* = 0 thread sleeps the entire time */
	else if (status > 0)
			 /* > 0 wakeup value = EVT_SIGNAL or EVT_RESUME */
	         *message = status; 
	     else if (status == -1) {
	              if (errno == EINTR) {
			     /* The thread is interrupted by a signal */
	                  *message = EVT_SIGPOST; 
		      } else
	     	          INTERNAL_ERROR("_event_wait");
	     } else
	          INTERNAL_ERROR("_event_wait");
}


/*
 * Function:
 *	_event_waitabs
 *
 * Parameters:
 *	lock - the lock structure of the calling thread.
 *	message - a pointer to where to store the event message
 *	absolute - a pointer to an absolute timeout value.
 *
 * Description:
 *	Convert the absolute timeout time to a relative timeout
 *	and call _event_wait() to do the work.
 */
void
_event_waitabs(spinlock_t *lock, int *message, const struct timespec *absolute)
{
	struct timespec	relative;
	struct timespec	now;

	if (absolute == NO_TIMEOUT) {
		_event_wait(lock, message, NO_TIMEOUT);
		return;
	}

	/* Convert absolute time to relative timeout.
	 */
	if (getclock(TIMEOFDAY, &now))
		INTERNAL_ERROR("_event_waitabs");
	relative.tv_nsec = absolute->tv_nsec - now.tv_nsec;
	relative.tv_sec = absolute->tv_sec - now.tv_sec;
	if (relative.tv_nsec < 0) {
		relative.tv_sec--;
		relative.tv_nsec += SECONDS_TO_NANOSECONDS(1);
	}
	if ((int)relative.tv_sec < 0) {
		*message = EVT_TIMEOUT;
		_spin_unlock(lock);
		return;
	}
	_event_wait(lock, message, &relative);
}

