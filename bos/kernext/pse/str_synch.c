static char sccsid[] = "@(#)31        1.44  src/bos/kernext/pse/str_synch.c, sysxpse, bos41J, 9521B_all 5/25/95 15:31:50";
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 * 
 * FUNCTIONS:      osr_run
 *                 osrq_init
 *                 osrq_term
 *                 osrq_insert
 *                 osrq_remove
 *                 osrq_wakeup
 *                 osrq_cancel
 *                 osrq_sig_kill
 *                 osr_set_timeout
 *                 osr_sleep
 *                 osr_bufcall_wakeup
 *                 osr_bufcall
 *                 act_q_init
 *                 act_q_term
 *                 add_tids
 *                 remove_tids
 *                 csq_run
 *                 csq_protect
 *                 csq_which_q
 *                 _csq_acquire
 *                 _csq_release
 *                 csq_turnover
 *                 csq_lateral
 *                 mult_sqh_acquire 
 *                 mult_sqh_release
 *                 sqh_insert
 *                 sqh_remove
 *                 csq_newparent
 *                 csq_cleanup
 * 
 * ORIGINS: 63, 71, 83
 * 
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.1
 */
/** Copyright (c) 1988  Mentat Inc.
 **/


#include <sys/pri.h> /* to solve PZERO, PCATCH etc ... */
#include <pse/str_stream.h>
#include <pse/str_proto.h>
#include <sys/stropts.h>
#include <sys/rtc.h>
#include <pse/str_funnel.h>
/*
 * ROUTINE: osr_run - run an operating system request (OSR)
 *
 * PARAMETERS:
 *	osr	- the operating system request to be executed
 *
 * RETURN VALUE:
 *	error status
 *
 * DESCRIPTION:
 *
 * This routine is responsible for serializing incoming systemcalls on one
 * stream head. For each class on non-conflicting systemcalls, there is a
 * separate OSR queue (OSRQ). The caller has already determined which OSRQ
 * is the right one for this OSR.
 *
 * The OSR gets enqueued into its OSRQ. If necessary, we sleep until the
 * OSR is the first in its OSRQ. This phase might be interrupted, in which
 * case we return EINTR without ever activating the job.
 *
 * We then acquire the minimum resources for the handler: that is the
 * stream head synch queue, and the mult_sqh, if specified in osr_flags.
 * (mult_sqh is acquired on this level, since it must be the first sqh
 * allocated - this is a rule to prevent deadlocks.)
 *
 * For one-at-a-time OSR's, the OSR is guaranteed to stay the first element
 * on its OSRQ. This prevents further OSRs from getting activated, and
 * provides a handle for asynchronous events to find the relevant OSR.
 * Otherwise, OSR's operate in parallel under protection of the various
 * synch queues, and all OSR's are awakened on OSRQ events.
 * 
 * When the OSR is finished (or when we got interrupted while still
 * waiting), the OSR is removed from the OSRQ, and the next in line
 * (if one-at-a-time and any) gets activated.
 *
 * CALLERS of this routine are those systemcalls which need synchronization
 * with other systemcalls, namely pse_read, pse_write and pse_ioctl. 
 * pse_open calls here in the case of a re-open of a stream head which 
 * already exists.
 */

int
osr_run (osr)
	OSRP	osr;
{
	OSRQP		osrq = osr->osr_osrq;
	STHP		sth = osr->osr_sth;
	int		error;
	int		* wake;
	DISABLE_LOCK_DECL

	ENTER_FUNC(osr_run, osr, 0, 0, 0, 0, 0);

	/*
	 * We could check here whether the stream head shows
	 * one of the osr_closeout conditions. But (1) we
	 * don't have control of the stream head here, and (2)
	 * we would have to repeat that test anyway, when we
	 * start the handler. Requests which can't run will go
	 * away quickly, so the only risk we are taking is waiting
	 * as an illegal request behind a legal one.
	 */

	DISABLE_LOCK(&osrq->osrq_lock);
	if ( osrq->osrq_first == nil(OSRP) )
		osrq->osrq_first = osrq->osrq_last = osr;
	else {
		osrq->osrq_last = (osrq->osrq_last->osr_next = osr);
		if ( osrq == &sth->sth_ioctl_osrq ) do {
			e_assert_wait(&osr->osr_sleeper, INTERRUPTIBLE);
			DISABLE_UNLOCK(&osrq->osrq_lock);
			switch(error = e_block_thread()) {
			case THREAD_AWAKENED :
				error = 0;
				break;
			default:
				error = EWOULDBLOCK;
				goto done;
			}
			DISABLE_LOCK(&osrq->osrq_lock);
		} while ( osrq->osrq_first != osr );
	}
	DISABLE_UNLOCK(&osrq->osrq_lock);

	if (osr->osr_flags & F_OSR_NEED_MULT_SQH )
		mult_sqh_acquire(osr);

	csq_acquire(&sth->sth_rq->q_sqh, &osr->osr_sq);

	/*
	 * Now we have done everything we can do on this common level.
	 * One last item is to check whether any osr_closeout condition
	 * exists AT THIS TIME.
	 */
loop:
	if (osr->osr_closeout & sth->sth_flags) {
		error = 0;
		if (osr->osr_closeout & F_STH_WRITE_ERROR)  {
		/*
		 * In case of ttys, EIO error must be avoided, an ioctl
		 * processed.
		 *
		 */
			if ((sth->sth_flags & F_STH_ISATTY)
			    && (osr->osr_flags & F_OSR_TTY_IOCTL)
			    && (sth->sth_write_error == EIO))
				goto tty_stuff;
			error = sth->sth_write_error;
		}
		if (error == 0 && (osr->osr_closeout & F_STH_READ_ERROR))
			error = sth->sth_read_error;
		if (error == 0)
			error = ENXIO;
	} else {
tty_stuff:
		if ( (sth->sth_flags & F_STH_CTTY) &&
		     (osr->osr_flags & F_OSR_TTYBITS) ) {
			if ( (error = osr_bgcheck(osr)) == 0 )
				error = (*osr->osr_handler)(osr);
			else { 
				if (error == -1) goto loop;
				if (error == -2) error = 0;
			}
		} else
			error = (*osr->osr_handler)(osr);
	}

	if ( osr->osr_flags & F_OSR_HAVE_MULT_SQH )
		mult_sqh_release(osr);

	if (sth->sth_rq->q_sqh.sqh_parent->sqh_owner == &osr->osr_sq)
		csq_release(&sth->sth_rq->q_sqh);

done:
	DISABLE_LOCK(&osrq->osrq_lock);
	if ( osrq->osrq_first == osr ) {
		if (osrq->osrq_first = osr->osr_next)
			wake = &(osrq->osrq_first->osr_sleeper);
		else wake = 0;
		if ( osrq != &sth->sth_ioctl_osrq )
			wake = 0;
	} else {
		OSRP osr1 = osrq->osrq_first;
		while ( osr1 && osr1->osr_next != osr )
			osr1 = osr1->osr_next;
		if ( osr1 && (osr1->osr_next = osr->osr_next) == NULL )
			osrq->osrq_last = osr1;
		wake = 0;
	}
	DISABLE_UNLOCK(&osrq->osrq_lock);
	if ( wake )
		e_wakeup(wake);

	LEAVE_FUNC(osr_run, error);

	return error;
}

/*
 * ROUTINE: osrq_init - initialize an OSR queue.
 *
 * PARAMETERS:
 *	osrq	- &OSRQ to be initialized
 *
 * RETURN VALUE:
 *	(none)
 *
 * DESCRIPTION:
 *	Let it look like an empty, unlocked queue.
 */

void
osrq_init(osrq)
	OSRQP	osrq;
{
	static short	osrq_lock_count = 0;
	ENTER_FUNC(osrq_init, osrq, 0, 0, 0, 0, 0);

	lock_alloc((&osrq->osrq_lock), LOCK_ALLOC_PIN, PSE_OSRQ_LOCK, 
			osrq_lock_count++);
	simple_lock_init(&osrq->osrq_lock);
	osrq->osrq_first = nil(OSRP);
	/* N.B.:  osrq_last is undefined when osrq_first is nil. */
	osrq->osrq_sleeper = EVENT_NULL;

	LEAVE_FUNC(osrq_init, 0);
}

void
osrq_term(osrq)
	OSRQP	osrq;
{
	lock_free(&osrq->osrq_lock);
}

/*
 * ROUTINE: osrq_insert - insert an OSR into an OSRQ.
 *
 * PARAMETERS:
 *	osrq	- the queue into which to insert the OSR
 *	osr	- the &OSR to be inserted
 *
 * RETURN VALUE:
 *	none.
 *
 * DESCRIPTION:
 *	Trivial insertion under lock protection.
 *	Note that this routine is called only in exceptional cases,
 *	the mainline code (see osr_run) does this inline.
 */

void
osrq_insert(osrq, osr)
	OSRQP	osrq;
	OSRP	osr;
{
	DISABLE_LOCK_DECL

	ENTER_FUNC(osrq_insert, osrq, osr, 0, 0, 0, 0);

	osr->osr_next = nil(OSRP);
	DISABLE_LOCK(&osrq->osrq_lock);
	if ( osrq->osrq_first == nil(OSRP) )
		osrq->osrq_first = osrq->osrq_last = osr;
	else
		osrq->osrq_last = (osrq->osrq_last->osr_next = osr);
	DISABLE_UNLOCK(&osrq->osrq_lock);

	LEAVE_FUNC(osrq_insert, 0);
}

/*
 * ROUTINE: osrq_remove - remove the first OSR from an OSRQ.
 *
 * PARAMETERS:
 *	osrq	- the OSRQ from which to remove an OSR
 *
 * RETURN VALUE:
 *	the OSR which has been removed (or nil).
 *
 * DESCRIPTION:
 *	Trivial removal under lock protection.
 *	Note also that this is done inline in osr_run.
 */

OSRP
osrq_remove(osrq)
	OSRQP	osrq;
{
	OSRP	osr;
	DISABLE_LOCK_DECL

	ENTER_FUNC(osrq_remove, osrq, 0, 0, 0, 0, 0);

	DISABLE_LOCK(&osrq->osrq_lock);
	if ( osr = osrq->osrq_first ) {
		osrq->osrq_first = osr->osr_next;
		osr->osr_next = nil(OSRP);
	}
	DISABLE_UNLOCK(&osrq->osrq_lock);

	LEAVE_FUNC(osrq_remove, osr);

	return osr;
}

/*
 * ROUTINE: osrq_wakeup - wakeup a sleeping OSR
 *
 * PARAMETERS:
 *	osrq	- the OSRQ on which the first OSR should be awakened
 *
 * RETURN VALUE:
 *	(none)
 *
 * DESCRIPTION:
 * This routine is used by all instances which have events to report.
 * It is the task of the caller to determine which event he has to
 * report. This routine is called with the OSRQ as argument.
 *
 * Note that this routine is potentially called from interrupt contexts
 * such as the stream head rput when a driver calls it. It must therefore
 * execute without context, and it can't acquire any synch locks.
 */

void
osrq_wakeup (osrq)
	OSRQP	osrq;
{
	OSRP	osr;
	int	* wake = NULL;
	DISABLE_LOCK_DECL

	ENTER_FUNC(osrq_wakeup, osrq, 0, 0, 0, 0, 0);

	DISABLE_LOCK(&osrq->osrq_lock);
	if (osr = osrq->osrq_first) {
		++osr->osr_awakened;
		if (osrq == &osr->osr_sth->sth_ioctl_osrq)
			wake = &osr->osr_sleeper;
		else {
			wake = &osrq->osrq_sleeper;
			while (osr = osr->osr_next)
				++osr->osr_awakened;
		}
	}
	DISABLE_UNLOCK(&osrq->osrq_lock);
	if (wake) {
		if (_ssavpri != INTBASE)
			e_wakeup(wake);
		else
			e_wakeupx(wake, E_WKX_NO_PREEMPT);
	}

	LEAVE_FUNC(osrq_wakeup, 0);
}

/*
 * ROUTINE: osr_set_timeout - Set a timer
 *
 * PARAMETERS:
 *      trb     - address of the trb structure pointer to be allacated.
 *      t_val   - number of ticks to wait.
 *
 * RETURN VALUE:
 *      (none)
 *
 * DESCRIPTION:
 *      Must be called between e_assert_wait() and e_block_thread().
 *      Must be called in thread context and with priority sets to
 *      INTBASE.
 */

static void
osr_sleep_timeout(struct trb *trb)
{
	e_clear_wait((tid_t)trb->func_data, THREAD_TIMED_OUT);
}

void
osr_set_timeout(trb, timeout_val)
	struct trb **trb;
	int	timeout_val;
{
	register struct trb *ptrb;

	if (ptrb = talloc()) {
		/*
		 * Convert the number of ticks specified to a time
		 * value that can be provided to the timer services.
		 */
		TICKS_TO_TIME(ptrb->timeout.it_value, timeout_val);

		/*
		 * Ensure that this is treated as an incremental timer,
		 * not an absolute one.
		 */
		ptrb->flags	= T_INCINTERVAL;
		ptrb->func	= osr_sleep_timeout;
		ptrb->eventlist	= EVENT_NULL;
		ptrb->func_data	= thread_self();
		ptrb->ipri	= INTTIMER;

		tstart(ptrb);
		*trb = ptrb;
	}
}

/*
 * ROUTINE: osrq_sig_kill - Send a kill signal
 *
 * PARAMETERS:
 *      osrq    - the OSRQ on which OSRs should be killed
 *
 * RETURN VALUE:
 *      (none)
 *
 * DESCRIPTION:
 *      Wakes all active requests that are waiting for messages.
 */

void
osrq_sigkill(osrq)
        OSRQP   osrq;
{
	struct trb *trb;
	DISABLE_LOCK_DECL

	DISABLE_LOCK(&osrq->osrq_lock);
	while (osrq->osrq_first) {
		e_wakeup_w_sig((int *)&(osrq->osrq_first->osr_sleeper),
								SIGKILL);
		e_wakeup_w_sig((int *)&(osrq->osrq_sleeper), SIGKILL);
		e_assert_wait(&osrq->osrq_sleeper, 0);
		DISABLE_UNLOCK(&osrq->osrq_lock);
		osr_set_timeout(&trb, hz/4);
		(void)e_block_thread();
		if (trb) {
			while (tstop(trb));
			tfree(trb);
		}
		DISABLE_LOCK(&osrq->osrq_lock);
		osrq->osrq_first = osrq->osrq_first->osr_next;	
	}
	DISABLE_UNLOCK(&osrq->osrq_lock);
}

/*
 * osrq_cancel - cancel requests on an OSRQ
 *
 * The stream is being closed asynchronously, such as when a session
 * leader exits. We must cancel active requests and wait for them to
 * leave the queues. The stream head F_STH_CLOS* flags have been set
 * to ensure this will happen...
 */

int
osrq_cancel (osrq)
	OSRQP		osrq;
{
	struct	trb *trb;	/* trb to submit timeout request */
	DISABLE_LOCK_DECL

	ENTER_FUNC(osrq_cancel, osrq, 0, 0, 0, 0, 0);

	DISABLE_LOCK(&osrq->osrq_lock);
	while (osrq->osrq_first != nil(OSRQP)) {
		e_wakeup((int *)&(osrq->osrq_first->osr_sleeper));
		e_wakeup((int *)&(osrq->osrq_sleeper));
		e_assert_wait(&osrq->osrq_sleeper, 0);
		DISABLE_UNLOCK(&osrq->osrq_lock);
		osr_set_timeout(&trb, hz/4);
		(void)e_block_thread();
		if (trb) {
			while (tstop(trb));
			tfree(trb);
		}
		DISABLE_LOCK(&osrq->osrq_lock);
		osrq->osrq_first = osrq->osrq_first->osr_next;
	}
	DISABLE_UNLOCK(&osrq->osrq_lock);

	LEAVE_FUNC(osrq_cancel, 0);
	return 0;
}

/*
 * ROUTINE: osr_sleep - the execution of an OSR has to wait for something
 *
 * PARAMETERS:
 *	osr	- the current OSR
 *	interruptible - are we accepting interrupts during the sleep?
 *	timeout_val - should there be a timeout to the sleep (0 = No.)
 *
 * RETURN VALUE:
 *	error condition
 *
 * DESCRIPTION:
 *
 * Put the OSR to sleep until some event happens.
 *
 * Due to the rule that we have to execute any pending "anonyomous
 * jobs" when releasing a stream lock (see call to csq_turnover
 * in csq_release), we might actually cause the event to happen.
 *
 * Any entity which reports an event for a particular OSRQ, calls
 * osrq_wakeup. It posts the event in the osr_awakened field.
 *
 * We will recognize which event happened by looking at "our"
 * flag, and the wait_result field in our thread. Given this
 * information, we can distinguish the following cases:
 *
 * case			recognized by checking...	action
 * ------------------------------------------------------------------------
 * nonblocking		osr->osr_flags			return EAGAIN
 * interrupt		tsleep return			return EINTR
 * timeout		tsleep return			return ETIME
 * general error	sth->sth_flags			return read/write error
 * real wakeup		osr->osr_flags			return 0
 * stray wakeup		(none of the above)		go back to sleep
 */		

int
osr_sleep (osr, interruptible, timeout_val)
	OSRP	osr;			/* our OSR */
	int	interruptible;		/* do we accept interrupts? */
	int	timeout_val;		/* max delay in ticks (0 = forever) */
{
	STHP	sth = osr->osr_sth;
	int	closeout_check;
	int	error;
	DISABLE_LOCK_DECL

	ENTER_FUNC(osr_sleep, osr, interruptible, timeout_val, 0, 0, 0);

	if ( osr->osr_flags & F_OSR_HAVE_MULT_SQH )
		mult_sqh_release(osr);
	csq_release(&sth->sth_rq->q_sqh);

	do {

                struct trb *trb;   /* trb to submit timeout request */

		if ( closeout_check = (osr->osr_closeout & sth->sth_flags) )
			break;

		DISABLE_LOCK(&osr->osr_osrq->osrq_lock);
		if (osr->osr_awakened) {
			osr->osr_awakened = 0;
			DISABLE_UNLOCK(&osr->osr_osrq->osrq_lock);
			error = 0;
			break;
		}
		/* Ignore nonblock when sleeping on no buffers, per spec. */
		if ((osr->osr_flags & F_OSR_NBIO) && osr->osr_bufcall_id == 0) {
			DISABLE_UNLOCK(&osr->osr_osrq->osrq_lock);
			/* e_block_thread();  Be sure to yield briefly */
			error = EAGAIN;
			break;
		}

                e_assert_wait((osr->osr_osrq == &sth->sth_ioctl_osrq) ?
                        &osr->osr_sleeper : &osr->osr_osrq->osrq_sleeper,
                        (interruptible ? INTERRUPTIBLE : 0));

		DISABLE_UNLOCK(&osr->osr_osrq->osrq_lock);
		if (timeout_val) {
			osr_set_timeout(&trb, timeout_val);
                        error = e_block_thread();
			if (trb) {
				while (tstop(trb));
				tfree(trb);
			}
                }
                else 
			error = e_block_thread();

		switch(error) {
		case THREAD_TIMED_OUT:
			error = ETIME;
			break;
		case THREAD_INTERRUPTED:
			error = EINTR;
			break;
		case THREAD_AWAKENED:
		case THREAD_OTHER:
			error = EVENT_SUCC;
		}

	} while (error == EVENT_SUCC);

	if (osr->osr_flags & F_OSR_NEED_MULT_SQH )
		mult_sqh_acquire(osr);
	csq_acquire(&sth->sth_rq->q_sqh, &osr->osr_sq);

	if ( closeout_check ) {
		error = 0;
		if (osr->osr_closeout & F_STH_WRITE_ERROR)
			error = sth->sth_write_error;
		if (error == 0 && (osr->osr_closeout & F_STH_READ_ERROR))
			error = sth->sth_read_error;
		if (error == 0)
			error = ENXIO;
	}

	LEAVE_FUNC(osr_sleep, error);
	return error;
}

/*
 * ROUTINE: osr_bufcall - the execution of an OSR has to wait for memory
 *
 * PARAMETERS:
 *	osr	- the current OSR
 *	interruptible, timeout_val - as for osr_sleep
 *	len, pri - as for bufcall
 *
 * RETURN VALUE:
 *	error condition
 *
 * DESCRIPTION:
 *
 * Set a bufcall, if it fails, try a timeout. Return osr_sleep value
 * passing interruptible and timeout_val. osr_bufcall_wakeup() does
 * acknowledgement of actual callback.
 */		

void
osr_bufcall_wakeup (osr)
	OSRP	osr;
{
	OSRQP	osrq = osr->osr_osrq;
	int	* wake;
	DISABLE_LOCK_DECL

	DISABLE_LOCK(&osrq->osrq_lock);
	osr->osr_bufcall_id = 0;
	++osr->osr_awakened;
	if (osrq == &osr->osr_sth->sth_ioctl_osrq)
		wake = &osrq->osrq_first->osr_sleeper;
	else
		wake = &osrq->osrq_sleeper;
	DISABLE_UNLOCK(&osrq->osrq_lock);
	e_wakeup(wake);
}

int
osr_bufcall (osr, interruptible, timeout_val, len, pri)
	OSRP	osr;
	int	interruptible;
	int	timeout_val;
	int	len;
	int	pri;
{
	int	bufcall_id, timeout_id;
	int	error = 0;

#if	MACH_ASSERT
	if (osr->osr_bufcall_id)
		panic("osr_bufcall");
#endif
	if (!(osr->osr_bufcall_id = bufcall_id =
			bufcall(len, pri, osr_bufcall_wakeup, osr))
	&&  !(osr->osr_bufcall_id = timeout_id =
			timeout(osr_bufcall_wakeup, osr, hz)))
		return EAGAIN;

	if (osr->osr_awakened && osr->osr_bufcall_id) {
		osr->osr_awakened = 0;
		osr->osr_bufcall_id = 0;
		return error;
	}

	do {
		error = osr_sleep(osr, interruptible, timeout_val);
	} while (error == 0 && osr->osr_bufcall_id);

clean:
	if (osr->osr_bufcall_id) {
		osr->osr_bufcall_id = 0;
		if (bufcall_id)
			unbufcall(bufcall_id);
		else
			untimeout(timeout_id);
	}
	return error;
}

/*
 * Synchronization Queue Handling.
 */

/*
 * Protection of "arbitrary" callback functions
 *
 * bufcall and timeout are allowed to specify arbitrary callback functions
 * as arguments. While it is relatively reasonable to assume that these
 * functions operate within the domain of the same queue where they got
 * scheduled (e.g. call qenable only), we can't guarantee this by adhering
 * to the published spec. The full generality is provided under the
 * QSAFE queue option, set by module config.
 *
 * In order to be able (on the timeout / bufcall level) to find out
 * which queue is the current one, we have to register each queue
 * activation, together with the thread ID. We can then later look up
 * the current thread and find which queue it is handling.
 *
 * The list of active queues is a DLL list, connected by the q_act_next
 * and q_act_prev fields. The active_queues record serves as the static
 * header. The list is protected by the active_qlock (a simple lock).
 *
 * The queue is inserted prior to activating it (but after having control
 * over the queue - the q_act_next / q_act_prev pointers are governed by
 * the same lock!), and removed after the routine returns. Note that the
 * same code is used in-line in csq_run (which handles the "orderly" entries
 * qi_srvp and qi_putp, and csq_protect (which handles the callback functions).
 * 
 * The overall strategy is to make the frequent operations (bad enough
 * that we have to do this at all...) as fast as possible, and to take
 * some hits in the (hopefully) rare case of the lookup. So, a DLL list
 * allows easy insert and delete operations, linear search is acceptable,
 * and we don't distinguish read vs. write access.
 *
 * Note that - as a special optimization - the lookup is omitted if the
 * callback function is "qenable".
 */

struct queue_entry active_queues;
simple_lock_data_t active_qlock = {SIMPLE_LOCK_AVAIL};

void
act_q_init()
{
	ENTER_FUNC(act_q_init, 0, 0, 0, 0, 0, 0);

	lock_alloc((&active_qlock), LOCK_ALLOC_PIN, PSE_ACTIVE_LOCK, -1);
	simple_lock_init(&active_qlock);
	active_queues.next = active_queues.prev = &active_queues;

	LEAVE_FUNC(act_q_init, 0);
}

void
act_q_term()
{
        ENTER_FUNC(act_q_term, 0, 0, 0, 0, 0, 0);
        if ((active_queues.next != &active_queues)
           || (active_queues.prev != &active_queues))
                panic("act_q_term");
	lock_free(&active_qlock);
        LEAVE_FUNC(act_q_term, 0);
}


void
add_tids(q)
	queue_t *q;
{
	tidlist *tids;
	tid_t	thread = thread_self();

	if (!q->q_tids.t_id) {
		q->q_tids.t_id = thread;
		return;
	}
	NET_MALLOC(tids, tidlist *, sizeof(tidlist), M_STREAMS, M_NOWAIT); 
	if (tids == NULL)
		return;
	tids->t_id = thread;
	tids->next = 0;
	if (!q->q_tids.next) 
		q->q_tids.next = tids;
	else {
		tidlist *t;
		for (t = q->q_tids.next; t->next; t = t->next)
			;
		t->next = tids;
	}
	return;
}

void
remove_tids(q)
	queue_t *q;
{
	tidlist *tids, *prev;
	tid_t	thread = thread_self();

	if (q->q_tids.t_id == thread) {
		q->q_tids.t_id = 0;
		return;
	}
	prev = &q->q_tids;
	tids = q->q_tids.next;
	while (tids) {
		if (tids->t_id == thread) {
			prev->next = tids->next;
			NET_FREE(tids, M_STREAMS);
			break;
		}
		prev = tids;
		tids = tids->next;
	}
}

/*
 *	csq_run - run an "anonymous" job
 *
 *	This is a put or a service procedure, run via csq_lateral
 *	(the normal path) or csq_acquire / csq_turnover (in the
 *	case of lock conflicts), and which needs QSAFEty. The
 *	"normal" case is handled by the macro below.
 */

#define	CSQ_RUN(sq)	do {						\
	int	savepri;						\
	queue_t	*q, *enable_q = (queue_t *)NULL;			\
	if ((q = (sq)->sq_queue) && q->q_int_count && (sq)->sq_count) {	\
		savepri = disable_lock(INTMAX, &q->q_qlock);		\
		q->q_int_count -= (sq)->sq_count;			\
		(sq)->sq_count = 0;					\
		if ((q->q_flag & QFULL) && ((q->q_int_count +		\
					q->q_count) <= q->q_lowat)) {	\
			q->q_flag &= ~QFULL;				\
			if (q->q_flag & QWANTW) {			\
				q->q_flag &= ~QWANTW;			\
				enable_q = q->q_bfcp;			\
			}						\
		}							\
		unlock_enable(savepri, &q->q_qlock);			\
	}								\
	if (!q || !(q->q_flag & QSAFE)) {				\
		void *arg1 = sq->sq_arg1;				\
		sq->sq_arg1 = 0;					\
		(*sq->sq_entry)(sq->sq_arg0, arg1);			\
	} else								\
		csq_run(sq);						\
	if (enable_q && (enable_q->q_flag & QUSE))			\
		qenable(enable_q);					\
} while (0)

void
csq_run (sq)
	SQP	sq;
{
	queue_t *	q1 = sq->sq_queue;
	void *		arg1 = sq->sq_arg1;
	DISABLE_LOCK_DECL
	int	found = 0;

	ENTER_FUNC(csq_run, sq, 0, 0, 0, 0, 0);

	sq->sq_arg1 = 0;
	if (q1 && (q1->q_flag & QSAFE)) {
		DISABLE_LOCK(&active_qlock);
		if (!q1->q_tids.next && !q1->q_tids.t_id)
			insque(&q1->q_act, &active_queues);
		add_tids(q1);
		DISABLE_UNLOCK(&active_qlock);

		(*sq->sq_entry)(sq->sq_arg0, arg1);

		DISABLE_LOCK(&active_qlock);
		remove_tids(q1);
		if (!q1->q_tids.next && !q1->q_tids.t_id)
			remque(&q1->q_act);
		DISABLE_UNLOCK(&active_qlock);
	} else
		(*sq->sq_entry)(sq->sq_arg0, arg1);

	LEAVE_FUNC(csq_run, 0);
}

/*
 *	csq_protect - call spec'd function with arg,
 *	while protecting it with the locks of q1 and q2.
 *
 *	Through this wrapping function, we call the module's qi_qopen
 *	and qi_qclose routines, as well as the callback functions
 *	for bufcall and timeout requests. The caller provides the queues
 *	which has to be locked (zero q pointers mean that the caller
 *	regards the callback function as safe without protection - qenable
 *	is such an example).
 *
 *	In addition to the locking, the queue is put on the active queue
 *	list - this enables lower level routines to identify the queue
 *	again, if they have to. This is done based on the thread-id,
 *	and could as well be achieved via thread-global memory. Note
 *	the queue is _always_ placed on the active_queues list regardless
 *	of QSAFE. This is needed by the sleep interface in str_env.c
 *	and acceptable because csq_protect is not used in the normal cases.
 */

int
csq_protect (q1, q2, func, arg, sq, which)
	queue_t *	q1;
	queue_t *	q2;
	csq_protect_fcn_t	func;
	csq_protect_arg_t	arg;
	SQP		sq;
	int		which;
{
	int	retval;
	DISABLE_LOCK_DECL
	int	found = 0;
	int	funnel = 0;

	ENTER_FUNC(csq_protect, q1, q2, func, arg, sq, which);

	if (q1) {
		csq_acquire(&q1->q_sqh, sq);
		if (q2) {
#if	MACH_ASSERT
			if (mult_sqh.sqh_owner != sq)
				panic("csq_protect");
#endif
			csq_acquire(&q2->q_sqh, sq);
		}
		DISABLE_LOCK(&active_qlock);
		if (!q1->q_tids.next && !q1->q_tids.t_id)
			insque(&q1->q_act, &active_queues);
		add_tids(q1);
		DISABLE_UNLOCK(&active_qlock);
	}
 	if (q1 && (q1->q_flag & QFUN))
		funnel = 1;
 	if (q2 && (q2->q_flag & QFUN))
		funnel = 1;
  	if (sq->sq_flags & SQ_IS_FUNNEL)
		funnel = 1;
	if (funnel) 
		(void) switch_cpu(MP_MASTER, SET_PROCESSOR_ID);
	retval = (*func)(arg);
	if (funnel)
		(void) switch_cpu(PROCESSOR_CLASS_ANY, RESET_PROCESSOR_ID);
	if (q1) {
		DISABLE_LOCK(&active_qlock);
		remove_tids(q1);
		if (!q1->q_tids.next && !q1->q_tids.t_id)
			remque(&q1->q_act);
		DISABLE_UNLOCK(&active_qlock);
		/*
		 * Unlock only if "which" is TRUE.
		 */
		if (which) {
			if (q2)
				csq_release(&q2->q_sqh);
			csq_release(&q1->q_sqh);
		}
	}

	LEAVE_FUNC(csq_protect, retval);
	return retval;
}

/*
 * Scan the active_queues list for this thread.
 * Called only from sleep(), timeout(), and bufcall().
 */
queue_t *
csq_which_q ()
{
	struct queue_entry *	q1;
	queue_t *	q;
	tid_t		thread = thread_self();
	DISABLE_LOCK_DECL
	tidlist *tids;

	ENTER_FUNC(csq_which_q, 0, 0, 0, 0, 0, 0);

	DISABLE_LOCK(&active_qlock);
	q1 = active_queues.next;
	while (q1 != &active_queues) {
		q = (queue_t *)((caddr_t)q1 - (int)&((queue_t *)0)->q_act);
		tids = &q->q_tids;
		while (tids) {
			if (tids->t_id == thread)
				goto out;
			tids = tids->next;
		}
		q1 = q1->next;
	}
	q = nil(queue_t *);
out:
	DISABLE_UNLOCK(&active_qlock);
	LEAVE_FUNC(csq_which_q, q);
	return q;
}

/*
 * csq_acquire - acquire a synch queue.
 *
 * This routine is called by process threads which want to get control over
 * queue. The caller is willing to block.
 *
 * A macro version in str_stream.h tries the "easy" case, where the lock
 * is won, and comes here only when blocking is required.
 *
 * Locking different queues need not mean locking different locks.
 * The synchronization is done on the PARENT of the requested synch
 * queue, in order to implement the various, more or less global locking
 * schemes. In order to hide this from the calling level, we implemented
 * a reference count in the synch queue. When the lock is first granted,
 * the synch queue element is recorded as the "owner". If further
 * requests come in, using the same SQ, then the reference count is
 * simply incremented. On a release, the reference count is decremented,
 * and only when it drops to zero, the lock is actually released.
 * The calling level must still make sure that lock and unlock operations
 * match up.
 */

void
_csq_acquire (sqh, sq)
	SQHP	sqh;
	SQP	sq;
{
	SQHP	psqh = sqh->sqh_parent;
	SQP	tsq;
#if defined(MACH_ASSERT) && defined(STR_VERIF)
	int c1;
#endif
	DISABLE_LOCK_DECL

	ENTER_FUNC(_csq_acquire, sqh, sq, 0, 0, 0, 0);

#if	MACH_ASSERT
	if (sq->sq_flags & SQ_QUEUED)
		panic("csq_acquire");
#endif
	LOCK_QUEUE(psqh);
	if ( psqh->sqh_flags & SQ_INUSE ) {
		if ( psqh->sqh_owner != sq ) {
			sq->sq_flags |= (SQ_QUEUED|SQ_HOLD);
			sq->sq_target = sqh;
#if defined(MACH_ASSERT) && defined(STR_VERIF)
			c1 = verif_queue(psqh);
			insque(sq, psqh->sqh_prev);
			ASSERT( (c1 + 1) == verif_queue(psqh));
#else
			insque(sq, psqh->sqh_prev);
#endif
			for (;;) {
				e_assert_wait(&(sq->sq_sleeper), 0);
				UNLOCK_QUEUE(psqh);
			wait_for_turnover:
				(void)e_block_thread();
				psqh = sqh->sqh_parent;
				LOCK_QUEUE(psqh);
				if ( psqh->sqh_owner == sq )
					break;
			}
		}
	} else if ( psqh->sqh_next != (SQP)psqh ) {
		psqh->sqh_flags |= SQ_INUSE;
		sq->sq_flags |= (SQ_QUEUED|SQ_HOLD);
		sq->sq_target = sqh;
#if defined(MACH_ASSERT) && defined(STR_VERIF)
		c1 = verif_queue(psqh);
		insque(sq, psqh->sqh_prev);
		ASSERT( (c1 + 1) == verif_queue(psqh));
#else
		insque(sq, psqh->sqh_prev);
#endif
		/* Run the queue until we find ourself or another SQ_HOLD */
		for (;;) {
			tsq = psqh->sqh_next;
#if defined(MACH_ASSERT) && defined(STR_VERIF)
			c1 = verif_queue(psqh);
			remque(tsq);
			ASSERT( (c1 - 1) == verif_queue(psqh));
#else
			remque(tsq);
#endif
			tsq->sq_flags &= ~SQ_QUEUED;
			if ( tsq->sq_flags & SQ_HOLD ) {
				if ( tsq == sq )
					break;
				psqh->sqh_owner = tsq;
				e_assert_wait(&(sq->sq_sleeper), 0);
				UNLOCK_QUEUE(psqh);
				e_wakeup(&(tsq->sq_sleeper));
				goto wait_for_turnover;
			}
			UNLOCK_QUEUE(psqh);
			CSQ_RUN(tsq);
			LOCK_QUEUE(psqh);
		}
	}
	psqh->sqh_flags |= SQ_INUSE;
	psqh->sqh_owner = sq;
	psqh->sqh_refcnt++;
	UNLOCK_QUEUE(psqh);
	sq->sq_flags &= ~SQ_HOLD;
	LEAVE_FUNC(_csq_acquire, 0);
}

#if	MACH_ASSERT
const char csqrel1[] = "csq_release 1", csqrel2[] = "csq_release 2";
#endif

void
_csq_release (sqh)
	SQHP	sqh;
{
	csq_release(sqh);
}

/*
 * csq_turnover - try to give up responsibility for the SQH
 *
 * If requests got queued while we were holding the queue, we must make
 * sure that they get run now. As long as we find "anonymous jobs", we
 * have to run them ourselves. If we find someone who is blocked in
 * csq_acquire, we are lucky: we hand the queue over to him, and are
 * out of here.
 *
 * We are of course also done when the queue goes empty.
 *
 * At the turnover time, check for q_int_count. If it is non-null, decrement
 * by the SQ data count and enable the backward flow control, if it goes
 * below the low-water mark. If there is no backward queue available, enable
 * our own queue and that might enable the other one.
 */

int
csq_turnover (psqh)
	SQHP	psqh;
{
reg	SQP	sq;
#if defined(MACH_ASSERT) && defined(STR_VERIF)
	int	c1;
#endif
	DISABLE_LOCK_DECL

	ENTER_FUNC(csq_turnover, psqh, 0, 0, 0, 0, 0);

	LOCK_QUEUE(psqh);
	if (!(psqh->sqh_flags & SQ_INUSE)) {
		psqh->sqh_flags |= SQ_INUSE;
		while ((sq = psqh->sqh_next) != (SQP)psqh) {
#if defined(MACH_ASSERT) && defined(STR_VERIF)
			c1 = verif_queue(psqh);
			remque(sq);
			ASSERT( (c1 - 1) == verif_queue(psqh));
#else
			remque(sq);
#endif
			sq->sq_flags &= ~SQ_QUEUED;
			if ( sq->sq_flags & SQ_HOLD ) {
				psqh->sqh_owner = sq;
				UNLOCK_QUEUE(psqh);
				e_wakeup(&sq->sq_sleeper);
				return 1;
			}
			UNLOCK_QUEUE(psqh);
			CSQ_RUN(sq);
			LOCK_QUEUE(psqh);
		}
		psqh->sqh_flags &= ~SQ_INUSE;
	}
	UNLOCK_QUEUE(psqh);

	LEAVE_FUNC(csq_turnover, 0);
	return 0;
}

/*
 * csq_lateral - run an "anonymous job"
 *
 * The anonymous job is either a put procedure, or a service
 * procedure, which has to be run while having control of its
 * queue.
 *
 * We check the queue before attempting to run our job in order
 * to guarantee ordering of our own requests. No such guarantee
 * is made for multiple threads. If requests are present or we
 * fail to lock the queue, we queue our job and leave via
 * csq_turnover. If we are able to lock the queue, then we can
 * just run the job.
 *
 * In order to fix timing windows, we have to check again,
 * after queueing or running our job, if the queue went idle
 * meanwhile. This is done via csq_turnover.
 *
 * If the resource is busy, then this will insert the SQ in the SQH and also
 * some internal flow control is needed. In the case of busy, the caller of 
 * putnext puts a message some where, not on a queue and returns without 
 * checking any flow controls. In that case, there are some chances that the 
 * caller can put data above the high water mark. To avoid this, the 
 * q_int_count will keep count of all the synch Q's data count, which are 
 * currently waiting on the queue and checks against the high water mark and 
 * marks QFULL, if it is higher. At the turnover time, in the csq_turnover() 
 * the q_int_count gets decremented and checks against the low water mark and 
 * enables the backward flow control, if the count goes to below low water mark.
 */

int
csq_lateral (sqh, sq, flow)
	SQHP	sqh;
	SQP	sq;
	int	flow;
{
reg	SQHP	psqh = sqh->sqh_parent;
#if defined(MACH_ASSERT) && defined(STR_VERIF)
	int	c1;
#endif
  	extern int throttle, allocated;
	DISABLE_LOCK_DECL

	ENTER_FUNC(csq_lateral, sqh, sq, 0, 0, 0, 0);

	LOCK_QUEUE(psqh);
	if ( psqh->sqh_next == (SQP)psqh && !(psqh->sqh_flags & SQ_INUSE) ) {
		psqh->sqh_flags |= SQ_INUSE;
		UNLOCK_QUEUE(psqh);
		CSQ_RUN(sq);
		LOCK_QUEUE(psqh);
		psqh->sqh_flags &= ~SQ_INUSE;
	} else {
#if	MACH_ASSERT
		if (sq->sq_flags & SQ_QUEUED)
			panic("csq_lateral");
		if (sq->sq_flags & SQ_HOLD)
			panic("csq_lateral: SQ_HOLD");
#endif
		sq->sq_flags |= SQ_QUEUED;
		sq->sq_target = sqh;
#if defined(MACH_ASSERT) && defined(STR_VERIF)
		c1 = verif_queue(psqh);
		insque(sq, psqh->sqh_prev);
		ASSERT( (c1 + 1) == verif_queue(psqh));
#else
		insque(sq, psqh->sqh_prev);
		/* flow is true only from putnext() */
		if (flow) {
			int count = 0, savepri, shift;
			queue_t * q;
			MBLKP    mp;

			q = sq->sq_arg0;
			mp = sq->sq_arg1;
			do {
				count += mp->b_wptr - mp->b_rptr;
			} while (mp = mp->b_cont);

			savepri = disable_lock(INTMAX, &q->q_qlock);
			q->q_int_count += count;
			sq->sq_count = count;
                        if ((q->q_int_count + q->q_count) >= q->q_hiwat )
				q->q_flag |= QFULL;
			unlock_enable(savepri, &q->q_qlock);
		}
#endif
	}
	UNLOCK_QUEUE(psqh);
	if ( psqh->sqh_next != (SQP)psqh ) 
		csq_turnover(psqh);
	LEAVE_FUNC(csq_lateral, 0);
	return 0;
}

/*
 * Routines to manage mult_sqh for osr's.
 */

SQH	mult_sqh;

void
mult_sqh_acquire (osr)
	OSRP	osr;
{
	ENTER_FUNC(mult_sqh_acquire, osr, 0, 0, 0, 0, 0);

	if ( !(osr->osr_flags & F_OSR_HAVE_MULT_SQH) ) {
		csq_acquire(&mult_sqh, &osr->osr_sq);
		osr->osr_flags |= F_OSR_HAVE_MULT_SQH;
#if	MACH_ASSERT
		/* No recursion allowed on mult_sqh */
		if (mult_sqh.sqh_refcnt != 1)
			panic("mult_sqh_acquire");
#endif
	}

	LEAVE_FUNC(mult_sqh_acquire, 0);
}

void
mult_sqh_release (osr)
	OSRP	osr;
{
	ENTER_FUNC(mult_sqh_release, osr, 0, 0, 0, 0, 0);

	if ( osr->osr_flags & F_OSR_HAVE_MULT_SQH ) {
#if	MACH_ASSERT
		if (mult_sqh.sqh_refcnt != 1
		||  mult_sqh.sqh_owner != &osr->osr_sq)
			panic("mult_sqh_release");
#endif
		csq_release(&mult_sqh);
		osr->osr_flags &= ~ F_OSR_HAVE_MULT_SQH;
	}

	LEAVE_FUNC(mult_sqh_release, 0);
}

/*
 * ROUTINE: sqh_insert - insert a new element into a synch queue
 *
 * PARAMETERS:
 *	sqh	- the sqh where to queue the request (the parent!)
 *	sq	- the request to queue
 *
 * RETURN VALUE:
 *	(none)
 *
 * DESCRIPTION:
 *	The request gets queued at sqh's parent synch queue. We register
 *	the original sqh in the sq, in order to be able to de-multiplex
 *	things in the case of re-parenting or cancellation.
 *
 *	This routine is also used for the STREAMS runq.
 */
void
sqh_insert (sqh, sq)
reg	SQHP	sqh;
reg	SQP	sq;
{
#if defined(MACH_ASSERT) && defined(STR_VERIF)
	int c1;
#endif
	DISABLE_LOCK_DECL

	ENTER_FUNC(sqh_insert, sqh, sq, 0, 0, 0, 0);

	LOCK_QUEUE(sqh);
#if	MACH_ASSERT
	if (sq->sq_flags & SQ_QUEUED)
		panic("sqh_insert");
#endif
	sq->sq_flags |= SQ_QUEUED;
#if defined(MACH_ASSERT) && defined(STR_VERIF)
	c1 = verif_queue(sqh);
	insque(sq, sqh->sqh_prev);
	ASSERT( (c1 + 1) == verif_queue(sqh));
#else
	insque(sq, sqh->sqh_prev);
#endif
	UNLOCK_QUEUE(sqh);

	LEAVE_FUNC(sqh_insert, 0);
}

/*
 * ROUTINE: sqh_remove - remove an element from a synch queue
 *
 * PARAMETERS:
 *	prev_sq	- the synch queue element whose successor should be removed
 *		  from the synch queue
 *
 * RETURN VALUE:
 *	the element with has been dequeued (or nil)
 *
 * DESCRIPTION:
 *	Given a pointer to a synch queue element, remove its successor from
 *	the synch queue (if any). The pointer may or may not be the synch
 *	queue head, so any implementation which needs to lock the queue
 *	has to search *backwards* to find the synch queue head. The head
 *	element is identified by the SQ_IS_HEAD bit, and never gets removed
 *	from the queue.
 *
 *	The synchronization requirement (caller of sqh_remove must own the
 *	queue) makes it safe to go *back* from each element on the queue to
 *	the queue head.
 *
 * SPECIAL NOTE:
 *	In order to remove an element from a synchronization queue,
 *	the caller must be in control of the queue (csq_acquire).
 *	This condition is not checked at this level.
 */
SQP
sqh_remove (prev_sq)
	SQP	prev_sq;
{
#if defined(MACH_ASSERT) && defined(STR_VERIF)
	int	c1;
#endif
	SQHP	sqh;
	SQP	sq = nil(SQP);
	DISABLE_LOCK_DECL

	ENTER_FUNC(sqh_remove, prev_sq, 0, 0, 0, 0, 0);

	sqh = (SQHP)prev_sq;
	while ( !(sqh->sqh_flags & SQ_IS_HEAD) )
		sqh = (SQHP)sqh->sqh_prev;

	LOCK_QUEUE(sqh);
	if ( !(prev_sq->sq_next->sq_flags & SQ_IS_HEAD) ) {
		sq = prev_sq->sq_next;
#if defined(MACH_ASSERT) && defined(STR_VERIF)
		c1 = verif_queue(sqh);
		remque(sq);
		ASSERT( (c1 - 1) == verif_queue(sqh));
#else
		remque(sq);
#endif
		sq->sq_flags &= ~SQ_QUEUED;
	}
	UNLOCK_QUEUE(sqh);

	LEAVE_FUNC(sqh_remove, sq);
	return sq;
}

/*
 * ROUTINE: csq_newparent - change parents of a stream head queue
 *
 * PARAMETERS:
 *	osr	- current OSR (used for acquiring locks)
 *	q	- the queue whose parents should change
 *	str	- streamtab pointer of (new) controlling stream
 *
 * RETURN VALUE:
 *	(none)
 *
 * DESCRIPTION:
 * During link and unlink, the synch queue parents of the lower stream
 * might change, depending on the synchronization level of the multiplexing
 * driver. This routine does the necessary changes, including the business
 * of locking and unlocking the various synch queues, and transferring
 * pending requests.
 *
 * The new parent is figured out based on the str parameter. Note that
 * this works also for switching back to stream head synchronization.
 * The queue has to be passed locked, and it will be returned locked
 * (under the new parent).
 */

void
csq_newparent(osr, q, str)
	OSRP			osr;
	queue_t *		q;
	struct streamtab *	str;
{
	SQHP	old_sqhp;
	SQHP	new_sqhp;

	ENTER_FUNC(csq_newparent, osr, q, str, 0, 0, 0);

	/*
	 * Save the old parent's sqh for later release.
	 * Figure out the new parent. If it is the same
	 * as the old one, then there is nothing more to do.
	 */
	old_sqhp = q->q_sqh.sqh_parent;
	new_sqhp = sqh_set_parent(q, str);
	if ( old_sqhp != new_sqhp ) {
		SQH	fakeout_sqh;
		SQP	sq, prev_sq;
		SQP	end_sq;

		/*
		 * If the parent has changed, we have to
		 * lock the new parent, transfer pending requests,
		 * and unlock the old parent...
		 *
		 * The end_sq is a little trick to avoid collision
		 * with insertions into the synch queue while we are
		 * scanning for "our" elements. (Nobody will queue
		 * something for us, since the parenting has already
		 * been updated.
		 *
		 * There is a race condition, if we lock the new parent first:
		 * During unlinking, if some message is present on lsth->rq,
		 * then it will try to run during the tranfer request but due
		 * to the parent lock, it will not run. This sq is queued on
		 * lsth->rq->q_sqh and at the release time, this will run but
		 * there is no read service routine for sth and so machine
		 * panics. To avoid this, first run the pending requests with
		 * the lock of old parent, lock the new parent and unlock the
		 * old parent. Changes are also made in osr_unlink_subr().
		 *
		 */

		NET_MALLOC(end_sq, SQP, sizeof (SQ), M_STRSQ, M_WAITOK);
		sq_init(end_sq);
		sqh_insert(old_sqhp, end_sq);
		for (
			prev_sq = (SQP)old_sqhp, sq = prev_sq->sq_next;
			sq != end_sq;
			prev_sq = sq, sq = prev_sq->sq_next ) {
			if ( sq->sq_target == &q->q_sqh ) {
				(void) sqh_remove(prev_sq);
				csq_lateral(&q->q_sqh, sq, 0);
				sq = prev_sq;
			}
		}
		(void) sqh_remove(end_sq->sq_prev);
		NET_FREE(end_sq, M_STRSQ);

		csq_acquire(&q->q_sqh, &osr->osr_sq);

		fakeout_sqh.sqh_parent = old_sqhp;
		csq_release(&fakeout_sqh);
	}
	LEAVE_FUNC(csq_newparent, 0);
}


/*
 * csq_cleanup - discard SQ elements which have been queued for a SQH
 *
 * This routine is used during shutdown of a queue, prior to deallocation
 * of the queue. While "anonymous jobs" (put and service procedures) may
 * sneak in harmlessly, we choose to panic if others are found. Examples
 * are timeouts and bufcalls, or other processes acquiring the SQH. The
 * problem lies elsewhere, but this gives us a chance to detect it now,
 * instead of hanging a thread or allowing it to dereference freed memory.
 * Of course, this may still happen! An uncancelled timeout might still be
 * lurking, for example.
 *
 * The caller must be in control of the SQH, and must somehow be sure that
 * nothing gets queued for this SQH anymore. We cannot assure that on this
 * level.
 *
 * If we should be disassembling the parent queue itself, we give it a
 * wildcard target, so that in this case, we actually remove all SQ's.
 * However, since there really SHOULDN'T be other requests queued, we
 * insert a consistency check here, and send a warning message in that
 * case. The problem would not be here, but at some other place.
 */

void
csq_cleanup (sqh)
	SQHP	sqh;
{
reg	SQHP	psqh;
reg	SQHP	target_sqh;
reg	SQP	sq;
	SQP	prev_sq;
	MBLKP	mp;
	SQP	end_sq;
	DISABLE_LOCK_DECL

	ENTER_FUNC(csq_cleanup, sqh, 0, 0, 0, 0, 0);

	if ( (psqh = sqh->sqh_parent) == nilp(SQH) ) {
		LEAVE_FUNC(csq_cleanup, 0);
		return;
	}

	if ( sqh == psqh )
		target_sqh = nil(SQHP);
	else
		target_sqh = sqh;

	NET_MALLOC(end_sq, SQP, sizeof (SQ), M_STRSQ, M_WAITOK);
	sq_init(end_sq);
	/*
	 * we are the owner of the sqh
	 */
	LOCK_QUEUE(psqh);
	insque(end_sq, psqh->sqh_prev);
	UNLOCK_QUEUE(psqh);
	for (
		prev_sq = (SQP)psqh, sq = prev_sq->sq_next;
		sq != end_sq;
		prev_sq = sq, sq = prev_sq->sq_next ) {
		if ( sq->sq_target == target_sqh || target_sqh == nil(SQHP) ) {
#if	MACH_ASSERT
			if ( sq->sq_target != sqh ) {
			/*
			 * ... then we fell into this if via the second part
			 * of the above condition. That means that we are
			 * cleaning out a parent queue itself. In that case,
			 * we should only find such elements which were
			 * actually queued for this queue, otherwise we have
			 * a problem somewhere in the code (not here). Better
			 * print a warning message in that case. 
			 */
			printf("STREAMS warning: csq_cleanup found alien SQ\n");
			}
#endif
			/*
			 * Get rid of the SQ and associated messages:
			 * - message-related SQ's are contained in the
			 *     message header, to which their sq_arg1 points.
			 * - others are unexpected and cause a panic.
			 */
			
			if (sq->sq_flags & (SQ_IS_HEAD|SQ_HOLD|SQ_IS_TIMEOUT))
				panic("csq_cleanup");
			LOCK_QUEUE(psqh);
			remque(sq);
			sq->sq_flags &= ~(SQ_QUEUED|SQ_INUSE);
			UNLOCK_QUEUE(psqh);
			if ( mp = (MBLKP)sq->sq_arg1 ) {
				sq->sq_arg1 = 0;
				freemsg(mp);
			}
			sq = prev_sq;
		}
	}
	LOCK_QUEUE(psqh);
	remque(end_sq);
	UNLOCK_QUEUE(psqh);
	NET_FREE(end_sq, M_STRSQ);

	LEAVE_FUNC(csq_cleanup, 0);
}

#if defined(MACH_ASSERT) && defined(STR_VERIF)
int
verif_queue(psqh)
	SQHP	psqh;
{
	SQP	sq;
	int	count = 0;
	extern void sq_wrapper();
	
	sq = psqh->sqh_next;
	while (sq != (SQP)psqh) {
		if (sq->sq_arg0 != NULL ) {
		    if ((sq->sq_arg1 == NULL) && (sq->sq_entry != sq_wrapper)) {
			printf("verif_queue: sq = %x\n", sq);
			panic(sq->sq_arg1 == NULL);
		    }
		}
		else
			printf("verif_queue: sq_arg0 = 0 sq = %x\n", sq);
		count ++;
		sq = sq->sq_next;
	}
	return count;
}
#endif
