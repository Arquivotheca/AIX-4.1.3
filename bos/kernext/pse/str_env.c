static char sccsid[] = "@(#)14        1.28  src/bos/kernext/pse/str_env.c, sysxpse, bos41J, 9521B_all 5/25/95 15:32:12";
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 * 
 * FUNCTIONS:      cmn_err
 *                 str_to
 *                 str_to_init
 *                 str_to_term
 *                 str_timeout
 *                 find_to
 *                 find_sq
 *		   str_timeout_fire
 *		   str_timeout_trb
 *		   str_untimeout_trb
 *		   microtime
 *                 pse_timeout_cf
 *                 pse_untimeout
 *                 pse_timeout
 *                 pse_sleepx
 *                 pse_block_thread
 *                 pse_sleep
 *                 pse_sleepl
 *                 pse_sleep_thread
 *		   mps_sleep
 *		   mps_wakeup
 *                 
 * 
 * ORIGINS: 63, 71, 83
 * 
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/** Copyright (c) 1988  Mentat Inc.
 **/

#include <sys/errno.h>
#include <sys/time.h>

#include <pse/str_stream.h>
#include <pse/str_proto.h>
#include <sys/stropts.h>
#include <sys/pri.h>
#include <sys/systemcfg.h>

#include <net/netisr.h>

/*
 * Some environment routines which are specified by
 * the STREAMS definition, but not available in OSF/1.
 * These come "without warranty", since the documentation
 * in the STREAMS programmer's guide and the System V
 * reference code leave some questions open. The first
 * usage in connection with software which is ported
 * from System V.3 will show whether we guessed right.
 * The open issues are pointed out at each routine.
 */

/*
 * cmn_err - generalized interface to printf and panic
 *
 * The definition of cmn_err() has been derived from the AT&T SVR4 STREAMS
 * Programmer's Guide, as published by Prentice Hall. Refer to that document
 * for further information.
 *
 * The spec says that the output goes (only) to the console, if the first
 * character is '!', (only) to an internal buffer - for crash dump analysis -
 * if it is '^', and to both locations when it is 
 */

void
cmn_err(int level, char *fmt, ...)
{
	int	to_console;
	int	to_buffer;
	int	dont_panic;
	va_list	ap;
	char *	prefix;
	char    buf[256];

	switch ( fmt[0] ) {
	case '!':
		to_console = TRUE;
		to_buffer = FALSE;
		fmt++;
		break;
	case '^':
		to_console = FALSE;
		to_buffer = TRUE;
		fmt++;
		break;
	default:
		to_console = TRUE;
		to_buffer = TRUE;
		break;
	}

	switch ( level ) {
	default: 
	case CE_CONT:
		prefix = NULL;
		dont_panic = TRUE;
		break;
	case CE_NOTE:
		prefix = "NOTE";
		dont_panic = TRUE;
		break;
	case CE_WARN:
		prefix = "WARNING";
		dont_panic = TRUE;
		break;
	case CE_PANIC:
		prefix = "PANIC";
		dont_panic = FALSE;
		break;
	}

	if ( to_buffer && !to_console ) {
		/* TODO: cmn_err - store messages in msgbuf */
		to_console = TRUE;      /* in the meantime */
	}
	if ( to_console ) {
		if ( prefix )
			printf("%s: %s\n", prefix, buf);
		else
			printf("%s", buf);
	}
	if ( !dont_panic )
		panic("cmn_err");
}

/*
 * Resolution of name conflicts which can't be handled by the C preprocessor
 *
 * This passage must be compliant with the definitions in the "conflicts"
 * section of str_config.h!
 */

/*
 *	Timeout handling
 *
 *	Source interface:
 *		id = timeout(func, arg, ticks)
 *		untimeout(id);
 *
 *	The C-Preprocessor maps
 *		timeout -> streams_timeout
 *		untimeout -> streams_untimeout
 *
 *	Intercepting these routines solves two problems
 *		- change of interfaces from V to OSF/1
 *		- correct synchronization in MP case
 *
 *	Interface change:
 *		- System V interface explained above
 *		- OSF/1 uses the (func, arg) pair as identifier,
 *		  i.e. those are the arguments to untimeout, and
 *		  timeout is a void function.
 *
 *	MP-Problem
 *		the callback function must run under the same
 *		protection as the caller who scheduled the callback.
 *
 *		When we get called back initially, we are running in
 *		the softclock context, so we simply move the timeout
 *		to the "todo" queue and schedule a Streams timeout event.
 *		Therefore, we use the SQ structure for storing the
 *		requests.
 *
 *	Data structure
 *		For our "callout" list, we use a chain of SQ's, headed
 *		by str_to_head and protected by the simple_lock str_to_lock.
 *		The SQ's get allocated dynamically, and removed after use.
 */

struct { SQP next, prev; } str_to_head, str_todo_head;
int str_to_id;
static simple_lock_data_t str_to_lock = {SIMPLE_LOCK_AVAIL};
static simple_lock_data_t trb_lock = {SIMPLE_LOCK_AVAIL};

#ifdef STREAMS_DEBUG
void
#else
static void
#endif
str_to(void)
{
	SQP	sq;
	DISABLE_LOCK_DECL

	ENTER_FUNC(str_to, 0, 0, 0, 0, 0, 0);
	
	DISABLE_LOCK(&str_to_lock);

	while ((sq = str_todo_head.next) != (SQP)&str_todo_head) {
		remque(sq);
		DISABLE_UNLOCK(&str_to_lock);
		sq->sq_arg1 = 0;
		(void) csq_protect(sq->sq_queue, (queue_t *)0,
				(csq_protect_fcn_t)sq->sq_entry,
				(csq_protect_arg_t)sq->sq_arg0,
				sq, TRUE);
		NET_FREE(sq, M_STRSQ);
		DISABLE_LOCK(&str_to_lock);
	}

	DISABLE_UNLOCK(&str_to_lock);

	LEAVE_FUNC(str_to, 0);
}

#define PSE_TIMER_ALLOC 20

void
str_to_init(void)
{

	ENTER_FUNC(str_to_init, 0, 0, 0, 0, 0, 0);
	lock_alloc((&str_to_lock), LOCK_ALLOC_PIN, PSE_TO_LOCK, -1);
	simple_lock_init(&str_to_lock);
	str_to_head.next = str_to_head.prev = (SQP)&str_to_head;
	str_todo_head.next = str_todo_head.prev = (SQP)&str_todo_head;
	lock_alloc((&trb_lock), LOCK_ALLOC_PIN, PSE_TRB_LOCK, -1);
	simple_lock_init(&trb_lock);
	(void)pse_timeout_cf(PSE_TIMER_ALLOC);
	if (netisr_add(NETISR_STRTO, str_to,
			(struct ifqueue *)0, (struct domain *)0))
		panic("str_to_init");
	LEAVE_FUNC(str_to_init, 0);
}

void
str_to_term(void)
{

	ENTER_FUNC(str_to_term, 0, 0, 0, 0, 0, 0);
        if (netisr_del(NETISR_STRTO))
                panic("str_to_term");
	(void)pse_timeout_cf(-PSE_TIMER_ALLOC);

#ifdef STREAMS_DEBUG
        if ((str_to_head.next != (SQP)&str_to_head) ||
                (str_to_head.prev != (SQP)&str_to_head))
                        panic("str_to_head");

        if ((str_todo_head.next != (SQP)&str_todo_head) ||
                (str_todo_head.prev != (SQP)&str_todo_head))
                        panic("str_todo_head");
#endif /*STREAMS_DEBUG*/

	lock_free(&trb_lock);
	lock_free(&str_to_lock);

	LEAVE_FUNC(str_to_term, 0);

}

#ifdef STREAMS_DEBUG
void
#else
static void
#endif
str_timeout(sq)
	caddr_t	sq;
{
	DISABLE_LOCK_DECL
	
	ENTER_FUNC(str_timeout, sq, 0, 0, 0, 0, 0);

	DISABLE_LOCK(&str_to_lock);

	remque(sq);
	insque(sq, str_todo_head.prev);

	DISABLE_UNLOCK(&str_to_lock);
	schednetisr(NETISR_STRTO);

	LEAVE_FUNC(str_timeout, 0);
}

#ifdef STREAMS_DEBUG
SQP
#else
static SQP
#endif
find_to(id, remove)
	int id, remove;
{
	SQP	sq;
	DISABLE_LOCK_DECL

	ENTER_FUNC(find_to, id, remove, 0, 0, 0, 0);
	assert(id);

	/* Check for presence on pending and active timeout queues */
	for (sq = str_to_head.next;
	     sq != (SQP)&str_to_head;
	     sq = sq->sq_next)
		if ((int)sq->sq_arg1 == id) {
			if (remove)
				remque(sq);
			LEAVE_FUNC(find_to, sq);
			return sq;
		}
	for (sq = str_todo_head.next;
	     sq != (SQP)&str_todo_head;
	     sq = sq->sq_next)
		if ((int)sq->sq_arg1 == id) {
			if (remove) 
				remque(sq);
			LEAVE_FUNC(find_to, sq);
			return sq;
		}
	LEAVE_FUNC(find_to, 0);
	return 0;
}

int
find_sq(sq)
	SQP     sq;
{
        SQP     psq;

        assert(sq);

        /* Check for presence on pending and active timeout queues */
        for (psq = str_to_head.next;
             psq != (SQP)&str_to_head;
             psq = psq->sq_next)
                if (psq == sq) {
                        remque(psq);
                        return 1;
                }


        for (psq = str_todo_head.next;
             psq != (SQP)&str_todo_head;
             psq = psq->sq_next)
                if (psq == sq) {
                        remque(psq);
                        return 1;
                }

        return 0;
}

#include <sys/time.h>
#include <sys/timer.h>
#include <sys/sleep.h>
#include <sys/intr.h>
#include <pse/str_select.h>

struct st_trb {
	struct st_trb	*st_next;
	struct st_trb	*st_prev;
	struct trb	*st_trb;
};

static queue_head_t trb_active = {&trb_active, &trb_active};
static queue_head_t trb_free = {&trb_free, &trb_free};

/*
 * pse_timeout_cf - preallocate timer blocks
 *
 * Called from pse_init() at PSE startup to allocate a pool of trbs.
 *
 */
int
pse_timeout_cf(int count)
{
	struct st_trb *pst;

	if (count > 0) {
		while (count) {
			/*
			 * Allocate a new str_trb structure initialize it.
			 * Allocate a new trb structure.
			 * Chain st_trb structure to the trb_freed list.
			 */
			NET_MALLOC(pst, struct st_trb *, sizeof *pst,
							M_STREAMS, M_WAITOK);
			if (pst == NULL) return -1;
			pst->st_next = pst;
			pst->st_prev = pst;
			pst->st_trb = talloc();
			if (pst->st_trb == NULL) {
				NET_FREE(pst, M_STREAMS);
				return -1;
			}
			SIMPLE_LOCK(&trb_lock);
			enqueue_head(&trb_free, pst);
			SIMPLE_UNLOCK(&trb_lock);
			count --;
		}
	} else {
		while ( count < 0) {

			/*
			 * Remove a st_trb struvcture from the free list
			 * Free the trb structure
			 * Free the st_trb structure
			 */

			SIMPLE_LOCK(&trb_lock);
			if ((pst = (struct st_trb *)dequeue_head(&trb_free))
								!= NULL) {
				tfree(pst->st_trb);
				NET_FREE(pst, M_STREAMS);
				SIMPLE_UNLOCK(&trb_lock);
			} else {
				SIMPLE_UNLOCK(&trb_lock);
				return -1;
			}
			count ++;
		}
	}
	return 0;
}

static void
str_timeout_fire(
	struct trb *trb)
{
	struct st_trb   *pst;
	DISABLE_LOCK_DECL

	(* (trb->tof))(trb->func_data);
	/*
	 * Find the correspondant st_trb structure.
	 * remove it from the active list and
	 * chain it in the free list
	 */
	DISABLE_LOCK(&trb_lock);
	for (pst = (struct st_trb *)(trb_active.next);
		pst != (struct st_trb *)(&trb_active); pst = pst->st_next) {
		if (pst->st_trb == trb) {
			enqueue_tail(&trb_free, dequeue_head(pst->st_prev));
			DISABLE_UNLOCK(&trb_lock);
			return;
		}
	}
	DISABLE_UNLOCK(&trb_lock);
}

int
str_timeout_trb(
        int		(*func)(),
        caddr_t		arg,
        int		ticks)
{
	struct trb	*pt = NULL;
	struct st_trb   *pst;
	DISABLE_LOCK_DECL
	
	/*
	 * Find a new trb structure in free list
	 */
	DISABLE_LOCK(&trb_lock);
	pst = (struct st_trb *)dequeue_head(&trb_free);
	if (pst) {
		enqueue_head(&trb_active, pst);
		DISABLE_UNLOCK(&trb_lock);
		pt = pst->st_trb;
	} else {
		DISABLE_UNLOCK(&trb_lock);
		return 0;
	}
	
	if (pt) {
		while (tstop(pt));

		pt->func = (void (*)()) str_timeout_fire;
		pt->tof = (void (*)()) func;
		pt->func_data = (unsigned long)arg;
		pt->timeout.it_value.tv_sec = ticks / HZ;
		pt->timeout.it_value.tv_nsec = (ticks % HZ) * (NS_PER_SEC / HZ);
		pt->flags = 0;
		pt->ipri = INTTIMER;
		pt->id = -1;
		pt->knext = pt->kprev = (struct trb *)0;

		tstart(pt);
	}

	return (int)pt;
}

void
str_untimeout_trb(
	struct trb	* id)
{
	struct st_trb	*pst;
	DISABLE_LOCK_DECL


	/*
	 * Find the correspondant st_trb structure.
	 * remove it from the active list and
	 * chain it in the free list
	 */

	DISABLE_LOCK(&trb_lock);
	for (pst = (struct st_trb *)(trb_active.next);
		pst != (struct st_trb *)(&trb_active); pst = pst->st_next) {
		if (pst->st_trb == id) {
			enqueue_head(&trb_free, dequeue_head(pst->st_prev));
			DISABLE_UNLOCK(&trb_lock);
			return;
		}
	}
	DISABLE_UNLOCK(&trb_lock);
}

void
microtime(
	struct timeval *tvp)
{
	tvp->tv_sec = tod.tv_sec;
	tvp->tv_usec = tod.tv_nsec / NS_PER_uS;
}

void
pse_untimeout(id)
	int	id;
{
	SQP	sq;
	DISABLE_LOCK_DECL

	ENTER_FUNC(pse_untimeout, id, 0, 0, 0, 0, 0);

	if (!id) return;
	while (tstop(id));

	DISABLE_LOCK(&str_to_lock);
	if (sq = find_to(id, 0)) {
		/* Stop timeout, then remove entry (if still there) */
		str_untimeout_trb((struct trb*)id);
		if (find_sq(sq)) {
			DISABLE_UNLOCK(&str_to_lock);
			NET_FREE(sq, M_STRSQ);
			return;
		}
	}
	DISABLE_UNLOCK(&str_to_lock);

	LEAVE_FUNC(pse_untimeout, 0);
}

int
pse_timeout(func, arg, ticks)
	timeout_fcn_t	func;
	timeout_arg_t	arg;
	int	ticks;
{
	SQP	sq;
	int	retval;
	DISABLE_LOCK_DECL

	ENTER_FUNC(pse_timeout, func, arg, ticks, 0, 0, 0);
	NET_MALLOC(sq, SQP, sizeof *sq, M_STRSQ, M_NOWAIT);
	if (sq == 0) {
	        LEAVE_FUNC(pse_timeout, 0);
		return 0;
	}
	sq_init(sq);

	sq->sq_entry = (sq_entry_t)func;
	sq->sq_arg0  = arg;
	if (func != (timeout_fcn_t)qenable &&
	    func != (timeout_fcn_t) osr_bufcall_wakeup) {
		sq->sq_queue = csq_which_q();
		if (!sq->sq_queue)
			sq->sq_flags |= SQ_IS_FUNNEL;
	} else
		sq->sq_queue = nil(queue_t *);
	sq->sq_flags |= SQ_IS_TIMEOUT;
	DISABLE_LOCK(&str_to_lock);

	retval = str_timeout_trb((int(*)())str_timeout, (caddr_t)sq, ticks);

	if (!retval) {
		DISABLE_UNLOCK(&str_to_lock);
		NET_FREE(sq, M_STRSQ);
		return retval;
	}

	insque(sq, str_to_head.next);
	sq->sq_arg1 = (void *)retval;
	DISABLE_UNLOCK(&str_to_lock);
	LEAVE_FUNC(pse_timeout, retval);
	return retval;
}

/*
 *	streams_mpsleep - MP safe version of mpsleep
 *
 *	We need this wrapper function for the actual
 *	sleep. It takes care of releasing and re-acquiring the locks under
 *	which the caller is running. There is quite a bit of knowledge about
 *	which locks can possibly be held at this time hard-coded in here,
 *	in order to avoid some expensive registry mechanism at each
 *	acquire and release operation.
 *
 *	The possible locks held are
 *	- this queue itself
 *		The sqh_owner identification which we find there, serves
 *		later to find out whether we own the other locks. So it'd
 *		better be intact!
 *
 *	- the queue above us
 *		This is usually the stream head. The only exception to the
 *		rule is re-open, which takes care of the stream head itself.
 *
 *	- the queue below us (if any)
 *
 *	SINCE THE INTRODUCTION OF SQLVL_QUEUE, we have to take care of
 *	each read and write queue separately. This doubles the number of
 *	potentially held locks...
 *
 *	- the mult_sqh lock
 *		This is a global lock, needed whenever we are working with
 *		more than one lock. A consistency check might be made here.
 *
 *	After having released all those locks, we might also insert another
 *	consistency check in order to assert that we are not holding any
 *	more locks...
 *		
 *	This routine may only be called during qi_qclose and qi_qopen
 *	routines and potentially on non-syncq streams, so we check the
 *	return of csq_which_q().
 */

#define	R	0	/* read */
#define	W	1	/* write */
#define	LR	2	/* lower read */
#define	LW	3	/* lower write */
#define	UR	4	/* upper read */
#define	UW	5	/* upper write */
#define	NQ	6

int
pse_sleepx(chan, pri, flags)
	caddr_t	chan;
	int	pri;
	int	flags;
{
	int		retval;
	SQP		my_sq;
	queue_t *	my_qs[NQ];
	int		i = 0, have_mult_sqh = 0;

        ENTER_FUNC(pse_sleepx, chan, pri, flags, 0, 0, 0);

	if (my_qs[R] = csq_which_q()) {
		if (pri > PZERO)
			pri |= PCATCH;
		my_sq = my_qs[R]->q_sqh.sqh_parent->sqh_owner;
		if (!(my_qs[R]->q_flag & QREADR)) {
			my_qs[W] = my_qs[R];
			my_qs[R] = OTHERQ(my_qs[W]);
		} else
			my_qs[W] = OTHERQ(my_qs[R]);
		my_qs[LR] = my_qs[R]->q_next;
		my_qs[UR] = backq(my_qs[R]);
		my_qs[LW] = backq(my_qs[W]);
		my_qs[UW] = my_qs[W]->q_next;
		if ( have_mult_sqh = (mult_sqh.sqh_owner == my_sq) )
			csq_release(&mult_sqh);
		for ( ; i < NQ; i++)
			if ( my_qs[i]
			&&   my_qs[i]->q_sqh.sqh_parent->sqh_owner == my_sq )
				csq_release(&my_qs[i]->q_sqh);
			else
				my_qs[i] = nilp(queue_t);
	} /* Else thread not in Streams context - no translation necessary */

        retval = sleepx((int)chan, pri, flags);

	if (have_mult_sqh)
		csq_acquire(&mult_sqh, my_sq);
	while (i > 0) {
		i -= 2;
		if (my_qs[i])
			csq_acquire(&my_qs[i]->q_sqh, my_sq);
		if (my_qs[i + 1])
			csq_acquire(&my_qs[i + 1]->q_sqh, my_sq);
	}
        LEAVE_FUNC(pse_sleepx, retval);
	return retval;
}

int
pse_block_thread()
{
	int		retval;
	SQP		my_sq;
	queue_t *	my_qs[NQ];
	int		i = 0, have_mult_sqh = 0;

        ENTER_FUNC(pse_block_thread, 0, 0, 0, 0, 0, 0);

	if (my_qs[R] = csq_which_q()) {
		my_sq = my_qs[R]->q_sqh.sqh_parent->sqh_owner;
		if (!(my_qs[R]->q_flag & QREADR)) {
			my_qs[W] = my_qs[R];
			my_qs[R] = OTHERQ(my_qs[W]);
		} else
			my_qs[W] = OTHERQ(my_qs[R]);
		my_qs[LR] = my_qs[R]->q_next;
		my_qs[UR] = backq(my_qs[R]);
		my_qs[LW] = backq(my_qs[W]);
		my_qs[UW] = my_qs[W]->q_next;
		if ( have_mult_sqh = (mult_sqh.sqh_owner == my_sq) )
			csq_release(&mult_sqh);
		for ( ; i < NQ; i++)
			if ( my_qs[i]
			&&   my_qs[i]->q_sqh.sqh_parent->sqh_owner == my_sq )
				csq_release(&my_qs[i]->q_sqh);
			else
				my_qs[i] = nilp(queue_t);
	} /* Else thread not in Streams context - no translation necessary */

	retval = e_block_thread();

	if (have_mult_sqh)
		csq_acquire(&mult_sqh, my_sq);
	while (i > 0) {
		i -= 2;
		if (my_qs[i])
			csq_acquire(&my_qs[i]->q_sqh, my_sq);
		if (my_qs[i + 1])
			csq_acquire(&my_qs[i + 1]->q_sqh, my_sq);
	}

        LEAVE_FUNC(pse_block_thread, retval);
	return retval;
}

int
pse_sleep(int *event_list, int flags)
{
	int	rc;
	int	interruptible;

	interruptible = ((flags == EVENT_SIGWAKE) || (flags == EVENT_SIGRET));

	e_assert_wait(event_list, interruptible);

	rc = pse_block_thread();

	if (rc == THREAD_INTERRUPTED) {
		if (flags == EVENT_SIGWAKE) {
			longjmpx(EINTR);
		}
		rc = EVENT_SIG;
	} else {
		rc = EVENT_SUCC;
	}

	return rc;
}

int
pse_sleepl(int *lock_word, int *event_list, int flags)
{
	int	rc;
	int	interruptible;

	interruptible = ((flags == EVENT_SIGWAKE) || (flags == EVENT_SIGRET));

	e_assert_wait(event_list, interruptible);

	unlockl(lock_word);

	rc = pse_block_thread();

	(void) lockl(lock_word, LOCK_SHORT);

	switch(rc){
	case THREAD_INTERRUPTED :
		rc = EVENT_SIG;
		break;
	default:
		rc = EVENT_SUCC;
		break;
	}

	return rc;
}

int
pse_sleep_thread(int *event_list, void *lockp, int flags)
{
	int	rc;
	int     _curpri;

        ASSERT((flags&(LOCK_SIMPLE|LOCK_HANDLER)) != (LOCK_SIMPLE|LOCK_HANDLER))

	e_assert_wait(event_list, flags & INTERRUPTIBLE);

	if (lockp != NULL) {
		if (flags & LOCK_SIMPLE)
			simple_unlock(lockp);
		else if (flags & LOCK_HANDLER) {
			_curpri = i_disable(INTBASE);
			unlock_enable(_curpri, lockp);
		} else if (flags & (LOCK_READ|LOCK_WRITE))
			lock_done(lockp);
	}

	rc = pse_block_thread();

        if (lockp) {
		if (flags & LOCK_SIMPLE) 
			simple_lock(lockp);
		else if (flags & LOCK_HANDLER)
			disable_lock(_curpri, lockp);
		else if (flags & LOCK_READ)
			lock_read(lockp);
		else if (flags & LOCK_WRITE)
			lock_write(lockp);
        }

	return rc;
}

/*
 * Defect 130950 - 
 * mps_sleep Streams exported service  must be mapped into pse_sleep
 * and exported in pse.exp.
 *
 * mps_wakeup - too -
 *
 * This is for backward compatibility with AIX 3.2.5 users who expect
 * the service to be named mps_sleep.
 */

int
mps_sleep(event_list, flags)
	int *event_list;
	int flags;
{
	return pse_sleep(event_list, flags);
}

void
mps_wakeup(event_list)
	int *event_list;
{
	e_wakeup(event_list);	
}

/* Defect 141587
 * This is for backward compatibility with AIX.3.2.5
 */

#undef splstr

int
splstr()
{
	return (i_disable(INTMAX));
}
