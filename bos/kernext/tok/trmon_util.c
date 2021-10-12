static char sccsid[] = "@(#)10	1.13  src/bos/kernext/tok/trmon_util.c, sysxtok, bos411, 9428A410j 5/28/94 15:07:59";
/*
 *   COMPONENT_NAME: SYSXTOK
 *
 *   FUNCTIONS: add_cdt
 *		del_cdt
 *		logerr
 *		save_trace
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

extern dd_ctrl_t mon_dd_ctrl;

/*
 *
 * ROUTINE NAME: logerr
 *
 * DESCRIPTIVE NAME: Token-Ring error logging routine.
 *
 * FUNCTION: Collect information for making of error log entry.
 *
 */

void logerr( dds_t *p_dds,
	    ulong  errid,
	    int    line,	/* line number */
	    char   *p_fname)	/* file name */
{
    tok_error_log_data_t    log;
    register int            i;             /* Loop counter */
    char		    errbuf[300];
    
    TRACE_BOTH(MON_OTHER, "logB", p_dds, errid, line);
    
    /* Initialize the log entry to zero */
    bzero(&log,sizeof(tok_error_log_data_t));
    
    /* Store the error id in the log entry */
    log.errhead.error_id = errid;
    
    /* Load the device driver name into the log entry */
    strncpy(log.errhead.resource_name, DDI.lname,
	    (size_t)sizeof(log.errhead.resource_name));

    /* plug in the line number and filename */
    sprintf(errbuf, "line: %d file: %s", line, p_fname);

    strncpy(log.file, errbuf, (size_t)sizeof(log.file));

    /*
     *   Start filling in the log with data
     */
    
    log.rr_entry = WRK.rr_entry;
    log.limcycle = WRK.limcycle;
    log.adap_state = WRK.adap_state;
    log.limbo = WRK.limbo;
    log.footprint = WRK.footprint;
    log.slot = DDI.slot;
    log.afoot = WRK.afoot;
    log.mcerr = WRK.mcerr;
    log.iox = WRK.piox;
    log.pio_rc = WRK.pio_rc;
    log.pio_addr = WRK.pio_addr;
    log.ac_blk = WRK.ac_blk;
    log.ring_status = WRK.ring_status;
    log.wdt_opackets = WRK.xmit_wdt_opackets;
    log.opackets_lsw = NDD.ndd_genstats.ndd_opackets_lsw;
    log.ipackets_lsw = NDD.ndd_genstats.ndd_ipackets_lsw;
    
    /* Load POS data in the table */
    for (i=0; i<8; i++)
    {
	log.cfg_pos[i] = WRK.cfg_pos[i];
    }

    /* Load Network address in use value into the table */
    COPY_NADR(WRK.adap_open_opts.node_addr, log.tok_addr);
    
    /* log the error here */
    errsave (&log,sizeof(tok_error_log_data_t) );
    
    return;
    
}  /* end logerr */

/*****************************************************************************/
/*
 * NAME:     save_trace
 *
 * FUNCTION: enter trace data in the trace table and call AIX trace
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:    Do not call directly -- use macros
 *
 * RETURNS:  nothing
 *
 */
/*****************************************************************************/
void save_trace(
	ulong	hook,	/* trace hook */
	char	*tag,	/* trace ID */
	ulong	arg1,	/* first argument */
	ulong	arg2,	/* second argument */
	ulong	arg3)	/* third argument */
{
	int	i, tracepri;

	tracepri = disable_lock(PL_IMP, &TRACE_LOCK);

	/*
	 * Copy the trace point into the internal trace table
	 */
	i = mon_dd_ctrl.trace.next_entry;
	mon_dd_ctrl.trace.table[i] = *(ulong *)tag;
	mon_dd_ctrl.trace.table[i+1] = arg1;
	mon_dd_ctrl.trace.table[i+2] = arg2;
	mon_dd_ctrl.trace.table[i+3] = arg3;
	
	if ((i += 4) < TRACE_TABLE_SIZE) {
		mon_dd_ctrl.trace.table[i] = MON_TRACE_NEXT;
		mon_dd_ctrl.trace.next_entry = i;
	}
	else {
		mon_dd_ctrl.trace.table[0] = MON_TRACE_NEXT;
		mon_dd_ctrl.trace.next_entry = 0;
	}
 
	unlock_enable(tracepri, &TRACE_LOCK);

	/*
	 * Call the external trace routine
	 */
	TRCHKGT(hook, *(ulong *)tag, arg1, arg2, arg3, 0);

} /* end save_trace */

/*****************************************************************************/
/*
 * NAME:     add_cdt
 *
 * FUNCTION: add an entry to the component dump table
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  nothing
 *
 */
/*****************************************************************************/
void
add_cdt (
    register char *name,  /* label string for area dumped */
    register char *ptr,   /* area to be dumped */
    register int   len)   /* amount of data to be dumped */
{
    struct cdt_entry temp_entry;
    int              num_elems;
    
    TRACE_DBG(MON_OTHER, "ACDb", (ulong)name, (ulong)ptr,(ulong)len);
    
    strncpy (temp_entry.d_name, name, sizeof(temp_entry.d_name));
    temp_entry.d_len = len;
    temp_entry.d_ptr = ptr;
    temp_entry.d_xmemdp = NULL;

    num_elems =
	(mon_dd_ctrl.cdt.header._cdt_len - sizeof(mon_dd_ctrl.cdt.header)) /
	sizeof(struct cdt_entry);
    if (num_elems < MAX_CDT_ELEMS)
    {
	mon_dd_ctrl.cdt.entry[num_elems] = temp_entry;
	mon_dd_ctrl.cdt.header._cdt_len += sizeof(struct cdt_entry);
    }
    
    TRACE_DBG(MON_OTHER, "ACDe", 0, 0, 0);
    return;
} /* end add_cdt */

/*****************************************************************************/
/*
 * NAME:     del_cdt
 *
 * FUNCTION: delete an entry from the component dump table
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  nothing
 *
 */
/*****************************************************************************/
void
del_cdt (
    register char *name,  /* label string for area dumped */
    register char *ptr,   /* area to be dumped */
    register int   len)   /* amount of data to be dumped */
{
    struct cdt_entry temp_entry;
    int num_elems;
    int ndx;
    
    TRACE_DBG(MON_OTHER, "DCDb", (ulong)name, (ulong)ptr,(ulong)len);
    
    strncpy (temp_entry.d_name, name, sizeof(temp_entry.d_name));
    temp_entry.d_len = len;
    temp_entry.d_ptr = ptr;
    temp_entry.d_xmemdp = NULL;
    
    num_elems =
	(mon_dd_ctrl.cdt.header._cdt_len - sizeof(mon_dd_ctrl.cdt.header)) /
	sizeof(struct cdt_entry);
    
    /* find the element in the array (match only the memory pointer) */
    for (ndx = 0;
	 (ndx < num_elems) &&
	 (temp_entry.d_ptr != mon_dd_ctrl.cdt.entry[ndx].d_ptr);
	 ndx++)
	; /* NULL statement */
    
    /* re-pack the array to remove the element if it is there */
    if (ndx < num_elems)
    {
	for (ndx++ ; ndx < num_elems; ndx++)
	    mon_dd_ctrl.cdt.entry[ndx-1] = mon_dd_ctrl.cdt.entry[ndx];
	bzero (&mon_dd_ctrl.cdt.entry[ndx-1], sizeof(struct cdt_entry));
	mon_dd_ctrl.cdt.header._cdt_len -= sizeof(struct cdt_entry);
    }
    
    TRACE_DBG(MON_OTHER, "DCDe", 0, 0, 0);
    return;
} /* end del_cdt */
