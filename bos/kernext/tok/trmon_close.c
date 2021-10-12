static char sccsid[] = "@(#)22	1.8  src/bos/kernext/tok/trmon_close.c, sysxtok, bos411, 9428A410j 5/26/94 16:53:08";
/*
 *   COMPONENT_NAME: SYSXTOK
 *
 *   FUNCTIONS: tokclose
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "tokpro.h"

extern dd_ctrl_t   mon_dd_ctrl;

/*****************************************************************************
 *
 * NAME:     tokclose
 *
 * FUNCTION: close entry point from kernel
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  0 or errno
 *
 *
 *****************************************************************************/
int
tokclose (ndd_t *p_ndd)
{
	dds_t	*p_dds;
	int	i, slihpri, iocc;
	uchar	pos2;
    
	/*
	 *  Get adapter structure
	 */
	p_dds = (dds_t *) (((unsigned long) p_ndd) - offsetof(dds_t, ndd));

	TRACE_BOTH(MON_OTHER, "CLOb", (int)p_dds, WRK.adap_state, WRK.limbo);

	TRACE_DBG(MON_OTHER, "SL_L", (int)p_dds, 0, 0);
	slihpri = disable_lock(PL_IMP, &SLIH_LOCK);

	/*
	 * Check the current state and do what is appropriate to progress
	 * towards shutdown.
	 * There is nothing to do if in the DEAD_STATE.
	 * The NULL_STATE, CLOSED_STATE, and CLOSE_PENDING states are invalid.
	 */
	switch(WRK.adap_state)
	{
	case OPEN_STATE:
		WRK.adap_state = CLOSE_PENDING;
		break;
	
	case OPEN_PENDING:
		/*
	 	 *  in middle of activation, stop it
	 	 */
		w_stop(&BUWDT);

		hwreset(p_dds);

		WRK.adap_state = CLOSED_STATE;
		break;
	
	case LIMBO_STATE:
		/*
	 	 *  we're in some phase of limbo, kill the recovery effort
	 	 */
		unlock_enable(slihpri, &SLIH_LOCK);
		TRACE_DBG(MON_OTHER, "SL_U", (int)p_dds, 0, 0);

		kill_limbo( p_dds );

		TRACE_DBG(MON_OTHER, "SL_L", (int)p_dds, 0, 0);
		slihpri = disable_lock(PL_IMP, &SLIH_LOCK);
		WRK.adap_state = CLOSED_STATE;
		break;
	}

	unlock_enable(slihpri, &SLIH_LOCK);
	TRACE_DBG(MON_OTHER, "SL_U", (int)p_dds, 0, 0);

	/*
	 * ignore any receive data
	 */
        WRK.recv_mode = FALSE;

	/*
	 * wait for transmit and control operations to finish
	 */
	while ((WRK.adap_state == CLOSE_PENDING) &&
	       (WRK.event_wait || WRK.xmits_queued || WRK.xmits_adapter)) {
		TRACE_BOTH(MON_OTHER, "CLOw", (int)p_dds, WRK.event_wait, 0);
		delay(HZ);
	}

	/*
	 * stop the transmit timeout watchdog timer now because it (and the
	 * generic watchdog timer) may be what stop the preceeding loop
	 */
	w_stop(&XMITWDT);

	if (WRK.adap_state == CLOSE_PENDING) {
		close_adap(p_dds);
	}

	/*
	 *  Restore the POS registers to their values at config time.
	 *  Ignore POS 0 and 1 since they are read only.
	 *  The card will be disabled.
	 */
        iocc = (int)IOCC_ATT( (ulong)DDI.bus_id,
			(ulong)(IO_IOCC + (DDI.slot << 16)));
	for (i = 2; i < 8; i++)
		TOK_PUTPOS( iocc + POS_REG_0 + i, WRK.cfg_pos[i]);
	IOCC_DET( iocc );

	TRACE_DBG(MON_OTHER, "DD_L", (int)p_dds, 0, 0);

	simple_lock(&DD_LOCK);
	/*
	 *  last open for any adapter
	 */
	if ((--mon_dd_ctrl.num_opens) == 0) {
	    /*
	     *  unregister for dump
	     */
	    dmp_del(((void (*) ())tok_cdt_func));
	}
	simple_unlock(&DD_LOCK);
	TRACE_DBG(MON_OTHER, "DD_U", (int)p_dds, 0, 0);

	/*
	 *  i_clear, free memory, unpincode, set adap_state & ndd_flags
	 */
	tokopen_cleanup(p_dds);

	TRACE_BOTH(MON_OTHER, "CLOe", 0, 0, 0);

	return (0);

} /* end tokclose */
