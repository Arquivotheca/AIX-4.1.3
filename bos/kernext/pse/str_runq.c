static char sccsid[] = "@(#)26        1.17  src/bos/kernext/pse/str_runq.c, sysxpse, bos412, 9447B 11/18/94 07:15:28";
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 * 
 * FUNCTIONS:      runq_init
 *                 runq_term
 *                 runq_run 
 *                 sq_wrapper
 *                 qenable
 *                 runq_sq_init 
 *                 runq_remove
 *                 flip_and_run
 *                 scheduled_run
 *                 scheduled_remove
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

#include <sys/param.h>

#include <pse/str_stream.h>
#include <pse/str_proto.h>
#include <sys/stropts.h>
#include <sys/systemcfg.h>
#include <sys/intr.h>
#include <net/netisr.h>
#include <pse/str_debug.h>

/*
 * Streams Run Queue Management
 */

SQH	streams_runq;
SQH	scheduled_runq;
struct intr runq_intr;                 /* runq offlevel interrupt */


void
runq_init()
{
	int		n;

	ENTER_FUNC(runq_init, 0, 0, 0, 0, 0, 0);

	sqh_init(&streams_runq);
	sqh_init(&scheduled_runq);
        /* initialize for off-level processing */
        INIT_OFFL3(&runq_intr, (int(*) ())flip_and_run, 0);
        /* initialize for scheduled processing */
        if (netisr_add(NETISR_STREAMS, scheduled_run,
                        (struct ifqueue *)0, (struct domain *)0))
                panic("runq_init");
	LEAVE_FUNC(runq_init, 0);

}
void flip_and_run()
{
    extern void new_stack(), rest_stack();
    extern char *pse_stack[];

    new_stack(pse_stack[mycpu()], INTR_STACK);
    runq_run();
    rest_stack();
}

void
runq_term()
{

	ENTER_FUNC(runq_term, 0, 0, 0, 0, 0, 0);
        if (netisr_del(NETISR_STREAMS))
                panic("runq_term");

	return;
}
/*
 * scheduled_run - STREAMS scheduled routine.
 */
void scheduled_run()
{
reg     SQP     sq;
reg     queue_t *q;
reg     SQHP    sqh = &scheduled_runq;
        DISABLE_LOCK_DECL

        ENTER_FUNC(scheduled_run, 0, 0, 0, 0, 0, 0);

        LOCK_QUEUE(sqh);
        for ( ;; ) {
                sq = sqh->sqh_next;
                if ( sq == (SQP)sqh )
                        break;
                remque(sq);
                UNLOCK_QUEUE(sqh);
                if (q = sq->sq_queue) {
#if     MACH_ASSERT
                        DISABLE_LOCK(&q->q_qlock);
                        q->q_runq_sq->sq_flags &= ~SQ_QUEUED;
                        DISABLE_UNLOCK(&q->q_qlock);

#endif
                        if (q->q_flag & QUSE)
                                csq_lateral(&q->q_sqh, sq, 0);
                        else {
                                DISABLE_LOCK(&q->q_qlock);
                                sq->sq_flags &= ~(SQ_QUEUED|SQ_INUSE);
                                DISABLE_UNLOCK(&q->q_qlock);
                        }
                } else {
                        void *arg1 = sq->sq_arg1;
                        sq->sq_arg1 = 0;
                        assert(((int) sq->sq_entry) > 0);
                        (*sq->sq_entry)(sq->sq_arg0, arg1);
                }
                LOCK_QUEUE(sqh);
 	}
        UNLOCK_QUEUE(sqh);
}
/*
 * runq_run - STREAMS interrupt routine.
 */
void
runq_run ()
{
reg	SQP	sq;
reg	queue_t	*q;
reg	SQHP	sqh = &streams_runq;
	DISABLE_LOCK_DECL

	ENTER_FUNC(runq_run, 0, 0, 0, 0, 0, 0);

	LOCK_QUEUE(sqh);
	for ( ;; ) {
		sq = sqh->sqh_next;
		if ( sq == (SQP)sqh )
			break;
		remque(sq);
		UNLOCK_QUEUE(sqh);
		if (q = sq->sq_queue) {
#if	MACH_ASSERT
			DISABLE_LOCK(&q->q_qlock);
			q->q_runq_sq->sq_flags &= ~SQ_QUEUED;
			DISABLE_UNLOCK(&q->q_qlock);
			
#endif
			if (q->q_flag & QUSE)
				csq_lateral(&q->q_sqh, sq, 0);
			else {
				DISABLE_LOCK(&q->q_qlock);
				sq->sq_flags &= ~(SQ_QUEUED|SQ_INUSE);
				DISABLE_UNLOCK(&q->q_qlock);
			}
		} else {
			void *arg1 = sq->sq_arg1;
			sq->sq_arg1 = 0;
			assert(((int) sq->sq_entry) > 0);
			(*sq->sq_entry)(sq->sq_arg0, arg1);
		}
		LOCK_QUEUE(sqh);
	}
	UNLOCK_QUEUE(sqh);
}

/*
 * sq_wrapper - wrapper function for the q's service procedure.
 */

void sq_wrapper (q)
	queue_t	* q;
{
	DISABLE_LOCK_DECL


	DISABLE_LOCK(&q->q_qlock);	
	q->q_flag &= ~QWANTR;
#if	MACH_ASSERT
	if ((q->q_runq_sq->sq_flags & SQ_INUSE) == 0 || !q->q_qinfo->qi_srvp)
		panic("sq_wrapper");
#endif
	q->q_runq_sq->sq_flags &= ~(SQ_INUSE|SQ_QUEUED);
	DISABLE_UNLOCK(&q->q_qlock);
	if (q->q_flag & QUSE)
		(void) (*q->q_qinfo->qi_srvp)(q);
}

/*
 * qenable - Enable a queue
 */
int
qenable (q)
reg	queue_t	* q;
{

	ENTER_FUNC(qenable, q, 0, 0, 0, 0, 0);
	if (q->q_qinfo->qi_srvp && (q->q_flag & QUSE)) {
		SQP	sq = q->q_runq_sq;
		int	gotit;
		DISABLE_LOCK_DECL

		DISABLE_LOCK(&q->q_qlock);
		if (gotit = (!(sq->sq_flags & SQ_INUSE)
			&& (q->q_flag & QUSE))) {
#if	MACH_ASSERT
			if (sq->sq_flags & SQ_QUEUED)
				panic("qenable");
#endif
			sq->sq_flags |= (SQ_INUSE|SQ_QUEUED);
			{
				int savpri;
				if (!(q->q_flag & QNOTTOSPEC)) {
					savpri = disable_lock(INTMAX, &streams_runq.sqh_lock);
					insque(sq, streams_runq.sqh_prev);
					unlock_enable(savpri, &streams_runq.sqh_lock);
				}
				else {
					savpri = disable_lock(INTMAX, &scheduled_runq.sqh_lock);
					insque(sq, scheduled_runq.sqh_prev);
					unlock_enable(savpri, &scheduled_runq.sqh_lock);
				}
			}
		}
		DISABLE_UNLOCK(&q->q_qlock);
		if (gotit) {	/* Wake up a STREAMS thread */
			if (!(q->q_flag & QNOTTOSPEC))
				i_sched(&runq_intr);
			else
				schednetisr(NETISR_STREAMS);
		}
	}
	LEAVE_FUNC(qenable, 0);
	return 0;
}

/*
 * runq_sq_init - initialize a run queue element
 *
 * Called by q_alloc to initialize the run queue element which is
 * contained in every queue.
 */

void
runq_sq_init (q)
	queue_t	* q;
{
	ENTER_FUNC(runq_sq_init, q, 0, 0, 0, 0, 0);

	sq_init(q->q_runq_sq);
	q->q_runq_sq->sq_entry = (sq_entry_t)sq_wrapper;
	q->q_runq_sq->sq_queue = q;
	q->q_runq_sq->sq_arg0  = q;
	q->q_runq_sq->sq_arg1  = 0;

	LEAVE_FUNC(runq_sq_init, 0);
}

/*
 * runq_remove - remove a streams queue from the run queue.
 *
 * The specified queue is about to be deallocated.  If, somehow, it has
 * recently gotten itself scheduled, we need to get it off the run queue. 
 * This routine only needs to take action in a multiprocessing or preemptive
 * environment.
 */

void
runq_remove (q)
	queue_t	* q;
{
	SQP	sq = q->q_runq_sq;
	SQHP	sqh = &streams_runq;
reg	SQHP	psqh;
	DISABLE_LOCK_DECL

	ENTER_FUNC(runq_remove, q, 0, 0, 0, 0, 0);

	DISABLE_LOCK(&q->q_qlock);
	if (sq->sq_flags & SQ_INUSE) {
		SIMPLE_LOCK(&sqh->sqh_lock);
		for (psqh = (SQHP)sqh->sqh_next; psqh != (SQHP)sqh; psqh = (SQHP)psqh->sqh_next) {
			if (psqh == (SQHP)sq) {
				remque(sq);
				sq->sq_flags &= ~(SQ_INUSE|SQ_QUEUED);
				break;
			}
		}
		SIMPLE_UNLOCK(&sqh->sqh_lock);
	}
	DISABLE_UNLOCK(&q->q_qlock);

	LEAVE_FUNC(runq_remove, 0);
}

/*
 * scheduled_remove - remove a streams queue from the run queue.
 *
 * The specified queue is about to be deallocated.  If, somehow, it has
 * recently gotten itself scheduled, we need to get it off the run queue. 
 * This routine only needs to take action in a multiprocessing or preemptive
 * environment.
 */

void
scheduled_remove (q)
	queue_t	* q;
{
	SQP	sq = q->q_runq_sq;
	SQHP	sqh = &scheduled_runq;
reg	SQHP	psqh;
	DISABLE_LOCK_DECL

	ENTER_FUNC(scheduled_remove, q, 0, 0, 0, 0, 0);

	DISABLE_LOCK(&q->q_qlock);
	if (sq->sq_flags & SQ_INUSE) {
		SIMPLE_LOCK(&sqh->sqh_lock);
		for (psqh = (SQHP)sqh->sqh_next; psqh != (SQHP)sqh; psqh = (SQHP)psqh->sqh_next) {
			if (psqh == (SQHP)sq) {
				remque(sq);
				sq->sq_flags &= ~(SQ_INUSE|SQ_QUEUED);
				break;
			}
		}
		SIMPLE_UNLOCK(&sqh->sqh_lock);
	}
	DISABLE_UNLOCK(&q->q_qlock);

	LEAVE_FUNC(scheduled_remove, 0);
}
