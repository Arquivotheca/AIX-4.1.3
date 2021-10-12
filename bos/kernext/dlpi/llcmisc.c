static char sccsid[] = "@(#)24  1.1  src/bos/kernext/dlpi/llcmisc.c, sysxdlpi, bos41J, 9514A_all 3/31/95 16:20:32";
/*
 *   COMPONENT_NAME: SYSXDLPI
 *
 *   FUNCTIONS: abort_session
 *		init_session
 *		local_busy
 *		local_okay
 *		setup_timers
 *		stop_timers
 *		term_session
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
 * llcmisc.c - MCS llc misc routines - find a better home for these
 *
 * public routines:
 *	local_busy - mark the local station busy
 *	local_okay - mark the local station as not-busy
 *	stop_timers - stop all the llc timers
 *	setup_timers - initialize the llc timers
 *	init_session - initialize the session variables for a new session
 *	term_session - stop timers, become idle
 *	abort_session - disconnect, flush, stop everything
 */

#include "include.h"

extern void expired_t1(), expired_t2(), expired_ti();

/*
 * local_busy - mark the local station busy
 */

void
local_busy(dlb)
	DLB *dlb;
{
	TRC(dlb, "local_busy");
	dlb->dlb_vb |= FB_LOCAL;
	(void)tx_super(dlb, S_RSP);
}

/*
 * local_okay - mark the local station as not-busy
 */

void
local_okay(dlb)
	DLB *dlb;
{
	if (dlb->dlb_vb & FB_LOCAL) {
		TRC(dlb, "local_okay");
		dlb->dlb_vb &= ~FB_LOCAL;
		(void)tx_super(dlb, S_RSP|S_FINAL);
	}
}

/*
 * stop_timers - stop all the llc timers
 */

void
stop_timers(dlb)
	DLB *dlb;
{
	tq_stop(dlb->dlb_tq1);
	tq_stop(dlb->dlb_tq2);
	tq_stop(dlb->dlb_tqi);
}

/*
 * setup_timers - initialize the llc timers
 */

void
setup_timers(dlb)
	DLB *dlb;
{
	/* prepare timers, but don't start them yet  */
	dlb->dlb_tq1->func = expired_t1;
	dlb->dlb_tq1->arg[0] = (ulong)dlb;

	dlb->dlb_tq2->func = expired_t2;
	dlb->dlb_tq2->arg[0] = (ulong)dlb;

	dlb->dlb_tqi->func = expired_ti;
	dlb->dlb_tqi->arg[0] = (ulong)dlb;
}

/*
 * init_session - initialize the session variables for a new session
 */

void
init_session(dlb)
	DLB *dlb;
{
	/* stop timers (in case this is a reset) */
	stop_timers(dlb);

	/* reset variables */
	dlb->dlb_s_flag		= 0;
	dlb->dlb_retry		= dlb->dlb_n2;
	dlb->dlb_ir_ct		= dlb->dlb_n3;
	dlb->dlb_vb		= 0;
	dlb->dlb_vs		= 0;
	dlb->dlb_vr		= 0;
	dlb->dlb_va		= 0;
	dlb->dlb_poll		= 0;
	dlb->dlb_final		= 0;
	dlb->dlb_state		= DL_DATAXFER;
	dlb->dlb_localreset	= 0;

	setup_timers(dlb);

	/* start the Ti timer now */
	tq_restart(dlb->dlb_tqi, dlb->dlb_ti);
}

/*
 * term_session - stop timers, become idle
 */

void
term_session(dlb)
	DLB *dlb;
{
	stop_timers(dlb);
	if (dlb->dlb_conn) {
		bzero(dlb->dlb_conn->remaddr, sizeof(dlb->dlb_conn->remaddr));
		dlb->dlb_conn->remsap = 0;
	}
	dlb->dlb_state = DL_IDLE;
}

/*
 * abort_session - disconnect, flush, stop everything
 */

void
abort_session(dlb)
	DLB *dlb;
{
	conn_t *c;

	switch (dlb->dlb_state) {
	case DL_DATAXFER:
	case DL_OUTCON_PENDING:
	case DL_USER_RESET_PENDING:
	case DL_PROV_RESET_PENDING:
		dl_flush(dlb, FLUSHRW);
		tx_rsp(dlb->dlb_conn, DM);
		break;
	case DL_INCON_PENDING:
		for (c = dlb->dlb_pend; c < &dlb->dlb_pend[MAXCONIND]; ++c)
			c->dlb = 0;
		dlb->dlb_npend = 0;
		break;
	}
	term_session(dlb);
}
