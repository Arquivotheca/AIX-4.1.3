static char sccsid[] = "@(#)28        1.5  src/bos/kernext/pse/str_select.c, sysxpse, bos411, 9428A410j 3/29/94 04:09:32";
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 *
 * FUNCTIONS:      enqueue_head
 *		   dequeue_head
 *		   enqueue_tail
 *		   dequeue_tail
 *		   select_enqueue
 *                 select_dequeue_all
 *                 select_wakeup
 *		   select_wakeup_on_events
 *
 * ORIGINS: 71, 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */


#include <pse/str_stream.h>
#include <pse/str_select.h>
#include <sys/poll.h>
#include <sys/atomic_op.h>

/*
 *      Insert element at head of queue.
 */

void enqueue_head(que, elt)
        register g_queue_t        que;
        register queue_entry_t  elt;
{
        elt->next = que->next;
        elt->prev = que;
        elt->next->prev = elt;
        que->next = elt;
}

/*
 *      Remove and return element at head of queue.
 */
queue_entry_t dequeue_head(que)
        register g_queue_t        que;
{
        register queue_entry_t  elt;

        if (que->next == que)
                return((queue_entry_t)0);

        elt = que->next;
        elt->next->prev = que;
        que->next = elt->next;
        return(elt);
}

/*
 *      Insert element at tail of queue.
 */
void enqueue_tail(que,elt)
        register g_queue_t        que;
        register queue_entry_t  elt;
{
        elt->next = que;
        elt->prev = que->prev;
        elt->prev->next = elt;
        que->prev = elt;
}

/*
 *      Remove and return element at tail of queue.
 */
queue_entry_t dequeue_tail(que)
        register g_queue_t        que;
{
        register queue_entry_t  elt;

        if (que->prev == que)
                return((queue_entry_t)0);

        elt = que->prev;
        elt->prev->next = que;
        que->prev = elt->prev;
        return(elt);
}


void
select_enqueue(sth, events, chan)
	STHP sth;
	ushort events;
	chan_t chan;
{
	POLLQP  pollq;
	POLLSP  polls;

	if (test_and_set(&(sth->sth_ext_flags), F_STH_POLL_INUSE)) {
		polls = &sth->sth_polls;
	} else {
		for (pollq = queue_first(&sth->sth_pollq);
			!queue_end(pollq, &sth->sth_pollq);
			pollq = queue_next(pollq)) {
			polls = (POLLSP)pollq;
			if ((polls->ps_events == events)
				&& (polls->ps_sth == sth)
				&& (polls->ps_chan == chan)) {
				return;
			}
		}
		NET_MALLOC(polls, POLLSP, sizeof(POLLS), M_STRPOLLS, M_WAITOK);
	}
	polls->ps_sth = sth;
	polls->ps_events = events;
	polls->ps_chan = chan;

	enqueue_tail(&sth->sth_pollq, &polls->ps_link);
}

void
select_dequeue_all(pollq)
	POLLSP	pollq;
{
	register POLLSP	qp;

	while (qp = (POLLSP) dequeue_head(&pollq->ps_link)) {
		if (qp == &qp->ps_sth->sth_polls) {
			fetch_and_and(&(qp->ps_sth->sth_ext_flags),
							~F_STH_POLL_INUSE);
			continue;
		}
		NET_FREE(qp, M_STRPOLLS);
	}
}

void 
select_wakeup(pollq)
	POLLQP  pollq;
{
	register POLLQP qp = queue_first(pollq);

	while (!queue_end(qp, pollq)) {
		POLLSP polls = (POLLSP)qp;
		int revents;

		revents = sth_poll_check(polls->ps_sth,
					polls->ps_events, &revents);
		qp = queue_next(qp);
                if (revents) {
                        selnotify(polls->ps_sth->sth_dev, polls->ps_chan,
								revents);
                        polls = (POLLSP)dequeue_head(polls->ps_link.prev);
			if (polls == &polls->ps_sth->sth_polls) {
				fetch_and_and(&(polls->ps_sth->sth_ext_flags),
							~F_STH_POLL_INUSE);
			} else
				NET_FREE(polls, M_STRPOLLS);
		}
	}
}

void
select_wakeup_on_events(pollq, events)
	POLLQP  pollq;
	int events;
{
	register POLLQP qp = queue_first(pollq);

	while (!queue_end(qp, pollq)) {
	    POLLSP polls = (POLLSP)qp;

	    qp = queue_next(qp);
	    if ((polls->ps_events | POLLHUP | POLLERR) & events) {
		int revents;

		revents = sth_poll_check(polls->ps_sth,
						polls->ps_events, &revents);
		if (revents) {
		    selnotify(polls->ps_sth->sth_dev, polls->ps_chan, revents);
		    polls = (POLLSP)dequeue_head(polls->ps_link.prev);
		    if (polls == &polls->ps_sth->sth_polls) {
			fetch_and_and(&polls->ps_sth->sth_ext_flags,
							~F_STH_POLL_INUSE);
		    } else
			NET_FREE(polls, M_STRPOLLS);
		}
	    }
	}
}
