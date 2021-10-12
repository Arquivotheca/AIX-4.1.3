static char sccsid[] = "@(#)29  1.1  src/bos/kernext/dlpi/timer.c, sysxdlpi, bos41J, 9514A_all 3/31/95 16:20:37";
/*
 *   COMPONENT_NAME: SYSXDLPI
 *
 *   FUNCTIONS: tq_alloc
 *		tq_fire
 *		tq_free
 *		tq_restart
 *		tq_start
 *		tq_stop
 *		tq_valid
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * timer.c - timer functions
 *
 * public routines:
 *	tq_alloc - allocate a timer
 *	tq_free - free a timer
 *	tq_start - start a timer only if not running
 *	tq_restart - (re)start a timer
 *	tq_stop - stop a timer
 *	tq_valid - see if a tired as fired yet
 *	tq_fire - process a timer
 *
 * NB: When a timer fires, it enqueues an M_PCSIG message on the q 
 *     in the timerq_t.  This message should not be freed, as tq_free()
 *     will handle this when the stream is closed.
 *
 * NB: The timeout function is called from the service proc context
 *     *of the queue specified in tq_alloc*, NOT an interrupt context.
 *     Therefore, it must obey and use the driver's locking protocol
 *     for proper mutex.
 */

#include "include.h"

/*
 * tq_alloc - allocate a timer
 */

timerq_t *
tq_alloc(q, tag)
	queue_t *q;
	int tag;
{
	timerq_t *tp;
	mblk_t *mp;

	extern mblk_t *mi_timer_alloc();

	if (mp = mi_timer_alloc(sizeof(*tp))) {
		tp = (timerq_t *)mp->b_rptr;
		bzero(tp, sizeof(timerq_t *));
		tp->tag = tag;
		tp->dlb = 0;
		tp->mp = mp;
		tp->fired = 1;
		return tp;
	}
	return 0;
}

/*
 * tq_free - free a timer
 */

void
tq_free(tp)
	timerq_t *tp;
{
	mi_timer_free(tp->mp);
}

/*
 * tq_start - start a timer only if not running
 *
 * will not alter an already running timer
 */

void
tq_start(tp, tenths)
	timerq_t *tp;
	int tenths;
{
	if (tp->fired)
		tq_restart(tp, tenths);
	else
		TRC(tp->dlb, "tq_start: %s tenths %d", &tp->tag, tenths);
}

/*
 * tq_restart - restart a timer
 *
 * starts or restarts a timer from the beginning
 */

void
tq_restart(tp, tenths)
	timerq_t *tp;
	int tenths;
{
	int ms = tenths * 100;	/* convert tenths of a second to ms */
	tp->fired = 0;
	mi_timer(tp->dlb->dlb_wq, tp->mp, ms);
	TRC(tp->dlb, "tq_restart: %s tenths %d", &tp->tag, tenths);
}

/*
 * tq_stop - stop a timer
 *
 * stopping a timer is idempotent (i.e. you can stop a stopped timer)
 */

void
tq_stop(tp)
	timerq_t *tp;
{
	if (!tp->fired)
		TRC(tp->dlb, "tq_stop: %s", &tp->tag);
	mi_timer(tp->dlb->dlb_wq, tp->mp, -1);
	tp->fired = 1;
}

/*
 * tq_valid - check if a timer is running
 */

int
tq_valid(tp)
	timerq_t *tp;
{
	return !tp->fired;
}

/*
 * tq_fire - process a timer
 */

void
tq_fire(mp)
	mblk_t *mp;
{
	timerq_t *tp = (timerq_t *)mp->b_rptr;

	/*
	 * Between the time the timer fires and this routine
	 * is called (from the service proc), the timer could
	 * have been cancelled or restarted.  Make sure this
	 * timer is still valid.
	 */
	if (!mi_timer_valid(mp)) {
		TRC(tp->dlb, "tq_fire: misfired %s", &tp->tag);
		return;
	}

	tp->fired = 1;
	TRC(tp->dlb, "tq_fire: %s: 0x%x(0x%x)", &tp->tag, tp->func, tp->arg);
	(*tp->func)(tp->arg);
}
