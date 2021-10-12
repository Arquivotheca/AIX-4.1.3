static char sccsid[] = "@(#)36  1.11  src/bos/kernext/ient/i_entclose.c, sysxient, bos411, 9438B411a 9/21/94 20:54:04";
/*****************************************************************************/
/*
 *   COMPONENT_NAME: SYSXIENT
 *
 *   FUNCTIONS:
 *              ient_close
 *              ient_shutdown
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

extern int ient_slih();
extern struct cdt *ient_cdt_func();
extern ient_dev_ctl_t *p_dev_ctl;
extern int ient_action();


/*****************************************************************************/
/*
 * NAME:     ient_close
 *
 * FUNCTION:  Closure entry point for the Integrated Ehernet Device Driver
 *
 * EXECUTION ENVIRONMENT: Process thread only.
 *
 * NOTES:
 *
 * CALLED FROM: NS user by using the ndd_close field in the NDD on the
 *              NDD chain.
 *
 * CALLS: delay
 *        ient_openclean  (i_entopen.c)
 *        ient_shutdown
 *        lockl
 *        unlockl
 *        unpincode
 *
 * INPUTS:   p_ndd  - pointer to the ndd.
 *
 * RETURNS:  0 (success) or errno
 */
/*****************************************************************************/

int
ient_close(ndd_t *p_ndd)
{
    int rc, ipri, count;

    TRACE_BOTH(HKWD_IENT_OTHER, "ClsB", (ulong)p_ndd,p_dev_ctl->device_state, 0);


    if (lockl((int *)&WRK.lock_anchor, LOCK_NDELAY) != LOCK_SUCC) {
        TRACE_BOTH(HKWD_IENT_OTHER, "Cls1", EBUSY, 0, 0);
        return(EBUSY);
    }

    if (p_dev_ctl->device_state != CLOSED)
    {
        p_dev_ctl->device_state = CLOSE_PENDING;
        WRK.restart = 0;
        count = 0;

        while ((p_dev_ctl->device_state != DEAD) && ((WRK.action_que_active)
                                                 || (WRK.xmits_buffered)))

        {
            delay(50);
            TRACE_BOTH(HKWD_IENT_OTHER, "Cls2", p_dev_ctl->device_state,
                                                                   count, 0);
            if (++count > 10)
                break;                 /* Something messed up if hit this */
                                       /* so don't hang detach            */
        }


        p_ndd->ndd_flags &= ~(NDD_RUNNING | NDD_UP | NDD_LIMBO | NDD_DEAD);

#ifdef DEBUG
        p_ndd->ndd_flags &= ~(NDD_DEBUG);
#endif

        ient_shutdown();

        ient_openclean();

        unpincode(ient_slih);

        p_dev_ctl->device_state = CLOSED;
    }

    unlockl((int *)&WRK.lock_anchor);

    TRACE_BOTH(HKWD_IENT_OTHER, "ClsE", count, 0, 0);
    return(0);
}


/*****************************************************************************/
/*
 * NAME:     ient_shutdown
 *
 * FUNCTION: Shuts down the Integrated Ethernet Device Driver
 *
 * EXECUTION ENVIRONMENT: Process thread only.
 *
 * NOTES:
 *
 * CALLED FROM:  ient_close
 *
 * CALLS:      delay
 *             dmp_del
 *             io_att
 *             io_det
 *             m_freem
 *             xmfree
 *
 * INPUT:  none.
 *
 * RETURNS:  0 (success) always.
 *
 */
/*****************************************************************************/

int
ient_shutdown()
{
    nad_t  *p_multi, *p_temp;
    int rc;
    ulong ioa, iocc_addr, pos_addr;
    struct mbuf *p_mbuf;
    int count = 0;
    uchar   pos2val;

    TRACE_BOTH(HKWD_IENT_OTHER, "CclB", (ulong)p_dev_ctl, 
                                        p_dev_ctl->device_state, IHS.bus_type);

    ioa = (ulong) io_att((DDS.bus_id | BUSMEM_IO_SELECT), 0);

    WRITE_SHORT(WRK.scb_ptr->command, CMD_CUC_ABORT | CMD_RUC_ABORT);
    CHANNEL_ATTENTION();

    delay(20);

    /* clear the multicast table */
    p_multi = WRK.alt_list;

    while (p_multi)
    {
        p_temp = p_multi->link;
        xmfree(p_multi, pinned_heap);
        p_multi = p_temp;
    }

    if (WRK.xmits_queued)                     /* Flush queued up buffers */
    {
        p_mbuf = WRK.txq_first;
        while (p_mbuf)
        {
            WRK.txq_first = p_mbuf->m_nextpkt;
            m_freem(p_mbuf);
            p_mbuf = WRK.txq_first;
            if (count++ > WRK.xmits_queued)   /* If we hit this something got */
                break;                        /* mangled.                     */
        }
    }

    WRK.multi_count     = 0;
    WRK.alt_list        = NULL;
    WRK.alt_count       = 0;
    WRK.restart_ru      = 0;
    WRK.promiscuous_count = 0;
    WRK.buffer_in       = 0;
    WRK.buffer_out      = 0;
    WRK.recv_buffers    = 0;
    WRK.xmits_queued    = 0;
    WRK.txq_first       = (struct mbuf *) NULL;
    WRK.txq_last        = (struct mbuf *) NULL;


    /*
    ** Now shutdown the adapter card.
    */
    iocc_addr = (ulong) io_att((DDS.bus_id | IOCC_SELECT), 0);
    pos_addr = iocc_addr + (POS_REG_OFFSET + (DDS.slot << 16)) + POS_2;
    BUS_GETCX((caddr_t)pos_addr, &pos2val);
    BUS_PUTCX((caddr_t)pos_addr, pos2val & 0xFE);

    io_det(iocc_addr);
    io_det(ioa);

    dmp_del(ient_cdt_func);

    TRACE_BOTH(HKWD_IENT_OTHER, "CclE", 0, 0, 0);
    return(0);
}
