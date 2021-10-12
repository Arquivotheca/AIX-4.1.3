static char sccsid[] = "@(#)39  1.11  src/bos/kernext/ient/i_enterr.c, sysxient, bos411, 9438B411a 9/21/94 20:54:11";
/*****************************************************************************/
/*
 *   COMPONENT_NAME: SYSXIENT
 *
 *   FUNCTIONS:
 *              ient_timeout
 *              ient_restart
 *              ient_logerr
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************/
#include <stddef.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <sys/intr.h>
#include <sys/timer.h>
#include <sys/watchdog.h>
#include <sys/dma.h>
#include <sys/malloc.h>
#include <sys/intr.h>
#include <sys/adspace.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/sleep.h>
#include <sys/trchkid.h>
#include <sys/err_rec.h>
#include <sys/mbuf.h>
#include <sys/dump.h>


#include <sys/ndd.h>

#include <sys/cdli.h>
#include <sys/generic_mibs.h>
#include <sys/ethernet_mibs.h>
#include <sys/cdli_entuser.h>

#include "i_entdds.h"
#include "i_entmac.h"
#include "i_enthw.h"
#include "i_entsupp.h"
#include "i_ent.h"
#include "i_enterrids.h"

extern ient_dev_ctl_t *p_dev_ctl;

void ient_timeout(struct watchdog *p_wdt);
void ient_logerr(ulong errid, int line, char *p_fname, ulong parm1,
                 ulong parm2, ulong parm3);


/*****************************************************************************/
/*
 * NAME:     ient_timeout
 *
 * FUNCTION: Ethernet driver watchdog timer timeout routine.
 *
 * EXECUTION ENVIRONMENT: Interrupt thread only.
 *
 * NOTES:
 *
 * CALLED FROM: Timer popping
 *
 * CALLS TO:    ient_logerr 
 *              ient_restart
 *              ient_xmit_done
 *              io_att
 *              io_det
 *              i_disable
 *              i_enable
 *
 * INPUTS:   p_wdt   - pointer to the watchdog structure in device control area
 *
 * RETURNS:  None
 *
 */
/*****************************************************************************/

void
ient_timeout(struct watchdog *p_wdt)  /* pointer to watchdog timer structure */
{
    ulong   delta;
    ulong   rc;
    ulong   status;
    ulong   csc_value;
    ulong   ioa;
    ushort  stat_value;
    int		oldpri;

    TRACE_BOTH(HKWD_IENT_ERR, "TtoB", (ulong)p_dev_ctl, 0, 0);

    /*
    ** A watch dog timer interrupt has occurred.  Determine where is was set
    ** and process it.
    */
    switch (WRK.wdt_setter) {

    case WDT_XMIT:
        {

        oldpri = i_disable(DDS.intr_priority);
        ioa = (ulong) io_att((DDS.bus_id | BUSMEM_IO_SELECT), 0);

        /*
        ** watchdog tripped as a result of a transmit not completing in
        ** due time start CU regardless of what state we think its in
        */
        WRK.wdt_setter = WDT_INACTIVE;
        WRK.timeout = TRUE;
        ENTSTATS.tx_timeouts++;
        /*
        ** call xmit done to give CU a nudge
        */
        READ_SHORT(WRK.scb_ptr->status, &stat_value);
        ient_xmit_done(p_dev_ctl, stat_value | STAT_CUS_TIMEOUT, ioa);

        io_det(ioa);

        TRACE_BOTH(HKWD_IENT_ERR, "Tto1", EFAULT, 0, 0);
	i_enable(oldpri);

        break;
        }

    case WDT_ACT:
        {
        /*
        ** An action command has not completed within the allocated time.
        ** Restart the adapter.
        */
        TRACE_BOTH(HKWD_IENT_ERR, "Tto2", EFAULT, 0, 0);
        ient_logerr(ERRID_IENT_TMOUT, __LINE__, __FILE__, EFAULT, 0, 0);
        ient_restart(NDD_ADAP_CHECK);
        break;
        }

    default:
        /* watchdog timer popped for an unknown reason */
        break;
    }
    TRACE_BOTH(HKWD_IENT_ERR, "TtoE", 0, 0, 0);
    return;
}

/*****************************************************************************/
/*
 *
 * NAME:  ient_restart
 *
 * FUNCTION: Restarts the Integrated Ethernet Device Driver on failure.
 *
 * EXECUTION ENVIRONMENT:  Both Process and Interrupt threads.
 *
 * NOTES:
 *
 * CALLED BY:   ient_action      (i_entopen.c)
 *              ient_action_done (i_entopen.c)
 *              ient_RU_complete (i_entintr.c)
 *              ient_slih        (i_entintr.c)
 *              ient_timeout
 *              ient_xmit_done   (i_entintr.c)
 *
 * CALLS TO:    d_complete
 *              ient_getstat     (i_entctl.c)
 *              ient_logerr
 *              ient_restart
 *              ient_start       (i_entopen.c)
 *              io_att
 *              io_det
 *              i_disable
 *              i_enable
 *              m_freem
 *              port             (i_entutil.c)
 *
 * INPUTS:      err_code   -  fault that caused the restart.
 *
 * RETURNS:     0 for a successful restart.  error code, otherwise
 *
 */
/*****************************************************************************/

int
ient_restart(int err_code)
{
    ulong  ioa;
    ulong  rc = 0;
    ndd_t  *p_ndd;
    struct mbuf *p_mbuf, *p_next;
    int    ipri;
    ndd_statblk_t  stat_blk;   /* status block */

    TRACE_BOTH(HKWD_IENT_OTHER, "NrsB", (ulong)p_dev_ctl, err_code, 0);

    ipri = i_disable(DDS.intr_priority);

    p_ndd = (ndd_t *) &(NDD);

    /*
    ** Make sure that we do not awaken Lazarus from
    ** his sleep.
    */
    if (p_dev_ctl->device_state == DEAD)
    {
        i_enable(ipri);
        TRACE_BOTH(HKWD_IENT_OTHER, "Nrs1", EFAULT, p_dev_ctl->device_state, 0);
        return(EFAULT);
    }

    /*
    ** Set the restart flag to prevent us from re-allocating
    ** resources and force us to use wait loops instead of delays
    */
    WRK.restart = TRUE;

    /*
    ** Disable the Ethernet adapter regardless of state
    */
    if (WRK.machine != MACH_MEM_BASED)
        ioa = (ulong) io_att((DDS.bus_id | BUSMEM_IO_SELECT), NULL);

    port(ioa, PORT_RESET);

    if (WRK.machine != MACH_MEM_BASED)
        io_det(ioa);

    /* Disable the device */
    DISABLE_DEVICE();

    /*
    ** if we are already in LIMBO, then kill the adapter.
    **  and report to NDD.
    */
    if (p_dev_ctl->device_state == LIMBO)
    {
        /*
        ** N.B.: This is the only place that DEAD state should
        **        ever be set and reported.  DEAD state can be forced
        **        by setting LIMBO and calling this routine.
        */

        /*
        ** Before entering the dead state, we need to free all the buffers
        ** waiting in the queue.
        */

        if (WRK.xmits_queued)
        {
            p_mbuf = WRK.txq_first;
            while (p_mbuf)
            {
                p_next = p_mbuf->m_nextpkt;
                p_mbuf->m_nextpkt = NULL;
                m_freem(p_mbuf);
                p_mbuf = p_next;
                p_ndd->ndd_genstats.ndd_opackets_drop++;
            }
        }
                
        p_dev_ctl->device_state = DEAD;
        p_ndd->ndd_flags = NDD_DEAD;
        i_enable(ipri);

        stat_blk.code = NDD_HARD_FAIL;
        (*(p_ndd->nd_status))(p_ndd, &stat_blk);

        TRACE_DBG(HKWD_IENT_OTHER, "Nrs2", p_dev_ctl->device_state, 0, 0);
        ient_logerr(ERRID_IENT_FAIL, __LINE__, __FILE__, ENOCONNECT,
                    p_dev_ctl->device_state, 0);
        return(ENOCONNECT);
    }

    p_dev_ctl->device_state = LIMBO;
    ENTSTATS.restart_count++;

    /*
    ** collect statistics from the SCB before it can be bzeroed.
    */
    ient_getstat();

    /*
    ** Report LIMBO state with a status block to demuxer
    */
    stat_blk.code = NDD_LIMBO_ENTER;
    stat_blk.option[0] = err_code;
    stat_blk.option[1] = 0;
    stat_blk.option[2] = 0;
    (*(p_ndd->nd_status))(p_ndd, &stat_blk);

    i_enable(ipri);

    /*
    ** If I/O based, then clear the DMA for the ASIC RAM
    */
    if (WRK.machine != MACH_MEM_BASED)
    {
        if (WRK.channel_allocated == TRUE)
        {
            rc = d_complete(WRK.dma_channel,DMA_NOHIDE, (char *) WRK.sysmem,
                        WRK.alloc_size, (struct xmem *)&WRK.xbuf_xd,
                        (char *)WRK.dma_base);
            if (rc)
            {
                TRACE_BOTH(HKWD_IENT_ERR, "Nrs3", rc, 0, 0);
                ient_logerr(ERRID_IENT_DMAFAIL, __LINE__, __FILE__, rc,
                            WRK.sysmem, WRK.dma_channel);
            }
        }
    }

    if ((rc = ient_start()) != 0)
    {
        TRACE_SYS (HKWD_IENT_ERR, "Nrs4", NDD_HARD_FAIL, 0, 0);
        /*
        ** Recursively call ient_restart.  Since we are
        ** already in LIMBO, this will kill the adapter
        ** and move us to DEAD state.  rc will remain set
        ** to the rc returned from ient_start, above.
        */
        rc = ient_restart(NDD_HARD_FAIL);

    }
    else
    {
        p_dev_ctl->device_state = OPENED;

        WRK.dev_stime = lbolt;        /* Reset the device timestamp */
        
        stat_blk.code = NDD_LIMBO_EXIT; /* pass a status block to demuxer */

        stat_blk.option[0] = 0;

        (*(p_ndd->nd_status))(p_ndd, &stat_blk);

    }

    TRACE_BOTH(HKWD_IENT_OTHER, "NrsE", rc, p_dev_ctl->device_state, 0);
    WRK.restart = FALSE;
    return(rc);
}


/*****************************************************************************/
/*
 *
 * NAME: ient_logerr
 *
 * FUNCTION:  Error logger for the Integrated Ethernet Device Driver
 *
 * EXECUTION ENVIRONMENT:  Both Process and Interrupt threads.
 *
 * NOTES:
 *
 * CALLED FROM: ient_action       (i_entopen.c)
 *              ient_action_done  (i_entopen.c)
 *              ient_restart
 *              ient_RU_complete  (i_entintr.c)
 *              ient_setup        (i_entopen.c)
 *              ient_slih         (i_entintr.c)
 *              ient_start        (i_entopen.c)
 *              ient_start_ru     (i_entopen.c)
 *              ient_timeout
 *              ient_xmit         (i_entout.c)
 *              ient_xmit_done    (i_entintr.c)
 *              retry_get         (i_entutil.c)
 *              retry_put         (i_entutil.c)
 *
 * CALLS TO:    bzero
 *              errsave
 *              sprintf
 *              strncpy
 *
 * INPUTS:      errid       - tells which error is being logged
 *              errnum      - tells which error code is returned
 *              scb         - Adapter Command Register value  (if available)
 *              Status      - Adapter Status Register value  (if available)
 *
 * RETURNS:  N/A
 *
 */
/*****************************************************************************/

void
ient_logerr(ulong errid, int line, char *p_fname, ulong parm1, ulong parm2,
            ulong parm3)
{
    struct  error_log_def   log;
    uchar   lbuf[64];
    int     i;

    TRACE_BOTH(HKWD_IENT_OTHER, "NlgB", (ulong)p_dev_ctl, errid, line);

    bzero(&log, sizeof(struct error_log_def));   /* zero out log entry */

    /*
    **  fill the error id and driver name into log entry
    */
    log.errhead.error_id = errid;                   /* error id */

    strncpy(log.errhead.resource_name, DDS.lname, ERR_NAMESIZE);

    /* put the line number and filename in the table */
    sprintf(lbuf, "line: %d file: %s", line, p_fname);
    strncpy(log.fname, lbuf, sizeof(log.fname));

    log.parm1 = parm1;                            /* error return code */
    log.parm2 = parm2;                            /* SCB  status field */
    log.parm3 = parm3;                            /* ethernet stat reg */

    /*
    ** POS register values
    */

    for (i=0; i<8; i++)
        log.pos_reg[i] = WRK.pos_reg[i];

    /*
    ** network address value
    */

    for (i=0; i<ENT_NADR_LENGTH; i++)
        log.ent_addr[i] = WRK.ent_addr[i];

    errsave(&log,sizeof(struct error_log_def));    /* log the error */

    TRACE_BOTH(HKWD_IENT_OTHER, "NlgE", 0, 0, 0);
    return;
}

