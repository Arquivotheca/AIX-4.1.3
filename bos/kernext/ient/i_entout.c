static char sccsid[] = "@(#)45  1.12  src/bos/kernext/ient/i_entout.c, sysxient, bos41B, 412_41B_sync 12/8/94 14:24:37";
/*****************************************************************************/
/*
 *   COMPONENT_NAME: SYSXIENT
 *
 *   FUNCTIONS:
 *              ient_output
 *              ient_xmit
 *              ient_xmit_error
 *              ient_txq_put
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
extern uchar ent_broad_adr[];


/*****************************************************************************/
/*
 * NAME:     ient_output
 *
 * FUNCTION: Ethernet driver output routine.
 *
 * EXECUTION ENVIRONMENT: Both Process and Interrupt Threads.
 *
 * NOTES:  Interrupt thread if ient_restart is called via ient_RU_complete.
 *         TCP/IP ARPs (pings) received will be routed back around via the
 *         demuxer to here.
 *
 * CALLED FROM: NS user by using the ndd_output field in the NDD on the
 *              NDD chain.
 *
 * CALLS TO:  ient_txq_put
 *            ient_xmit
 *            ient_xmit_error
 *            io_att
 *            io_det
 *            i_disable
 *            i_enable
 *            m_freem
 *
 * INPUTS:  nddp      - pointer to the ndd.
 *          mbufp     - pointer to a mbuf (chain) for outgoing packets
 *
 * RETURNS:  0 (success) or errno
 *
 */
/*****************************************************************************/

int
ient_output(ndd_t *nddp, struct mbuf *mbufp)
{
    struct mbuf *CurMbufPtr, *MbufToFree;
    struct mbuf *FreeFirst = (struct mbuf *) NULL;
    struct mbuf *FreeLast = (struct mbuf *) NULL;
    struct mbuf * ient_txq_put(struct mbuf *);
    void   ient_xmit_error();
    int    first = TRUE;
    int    IntsDisabled;
    ulong  OldIntrPri;
    ulong  BusIOAddr;
    int    DDrc = 0;

    /*
    ** We have to use i_disable() to serialize access to this routine and
    ** amongst the other upper layers such as ient_ioctl().  We cannot use
    ** the lockl() service because this transmit function can also be called
    ** indirectly by the ient_RU_complete() routine in the case of TCP/IP
    ** ARPs.  Locks are now allowed to be taken in the interrupt thread.
    */

    OldIntrPri = i_disable(DDS.intr_priority);
    IntsDisabled = TRUE;

    TRACE_SYS(HKWD_IENT_XMIT, "TtxB", (ulong)nddp, (ulong)mbufp, 0);

    /*
    ** Update MIB statistics for Ucast, Mcast and Bcast variables
    ** before returning.  Specs say we collect this info regardless
    ** of device state.
    */

    CurMbufPtr = mbufp;                       /* Save off current pointer */

    while ((ulong) CurMbufPtr)
    {
        if (CurMbufPtr->m_flags & (M_BCAST | M_MCAST))
        {
            if (CurMbufPtr->m_flags & M_BCAST)
            {
                if (nddp->ndd_genstats.ndd_ifOutBcastPkts_lsw == ULONG_MAX)
                {
                    nddp->ndd_genstats.ndd_ifOutBcastPkts_msw++;
                }
                nddp->ndd_genstats.ndd_ifOutBcastPkts_lsw++;
            }
            else
            {
                if (nddp->ndd_genstats.ndd_ifOutMcastPkts_lsw == ULONG_MAX)
                {
                    nddp->ndd_genstats.ndd_ifOutMcastPkts_msw++;
                }
                nddp->ndd_genstats.ndd_ifOutMcastPkts_lsw++;
            }
        }
        else
        {
            if (nddp->ndd_genstats.ndd_ifOutUcastPkts_lsw == ULONG_MAX)
            {
                nddp->ndd_genstats.ndd_ifOutUcastPkts_msw++;
            }
            nddp->ndd_genstats.ndd_ifOutUcastPkts_lsw++;
        }
        CurMbufPtr = CurMbufPtr->m_nextpkt;
    }

    /*
    ** Now check the state of the device and if it is not OPENED, then return
    ** appropriate reason.
    */

    if (p_dev_ctl->device_state != OPENED)
    {
        if (p_dev_ctl->device_state == DEAD)
        {
            TRACE_BOTH(HKWD_IENT_ERR, "Ttx1", ENETDOWN, 0, 0);
            DDrc = ENETDOWN;
        }
        else
        {
            TRACE_BOTH(HKWD_IENT_ERR, "Ttx2", ENETUNREACH, 0, 0);
            DDrc = ENETUNREACH;
        }

        (void) i_enable(OldIntrPri);
        return(DDrc);
    }

    /*
    ** If the the CU is currently handling requests or the circular queue
    ** is full and we are filling the other queue then we need to either
    ** add to the queue or return EAGAIN.
    */

    TRACE_SYS(HKWD_IENT_OTHER, "Ttx3", WRK.xmits_queued, DDS.xmt_que_size,
              OldIntrPri);

    if ((ulong) WRK.txq_first)
    {
        /*
        ** There is one or more on the chain so check to see if the queue is
        ** full.  If not, add it, otherwise return error.
        */

        if (WRK.xmits_queued >= DDS.xmt_que_size)      /* SoftQ is full */
        {
            i_enable(OldIntrPri);

            /*
            **  caller does not want to block, return EAGAIN
            **  call xmt_fn when resources are available
            */

            while (mbufp)
            {
                nddp->ndd_genstats.ndd_xmitque_ovf++;
                mbufp = mbufp->m_nextpkt;
            }

            TRACE_SYS(HKWD_IENT_ERR, "Ttx4", EAGAIN, 0, 0);
            return(EAGAIN);
        }
        else
        {
           MbufToFree = ient_txq_put(mbufp);
           i_enable(OldIntrPri);
           
           while (CurMbufPtr = MbufToFree)
           {
               MbufToFree = MbufToFree->m_nextpkt;
               CurMbufPtr->m_nextpkt = NULL;
               m_freem(CurMbufPtr);
               
               nddp->ndd_genstats.ndd_xmitque_ovf++;
               nddp->ndd_genstats.ndd_opackets_drop++;
           }

           TRACE_SYS(HKWD_IENT_XMIT, "Ttx5", DDrc, 0, 0);
           return(DDrc);
        }
    }

    if (WRK.machine != MACH_MEM_BASED)     /* Attach for Stillwell */
        BusIOAddr = (ulong) io_att((DDS.bus_id | BUSMEM_IO_SELECT), 0);

    while (mbufp)
    {
        CurMbufPtr = mbufp;

        /*
        ** If there is TBD available, try to transmit the packet.
        */

        if (WRK.xmits_buffered < XMIT_BUFFERS)
        {
            /*  For netpmon performance tool  */
            TRACE_SYS(HKWD_IENT_XMIT, "WQUE", p_dev_ctl->seq_number,
                      (int)CurMbufPtr, CurMbufPtr->m_pkthdr.len);

            if ((DDrc = ient_xmit(CurMbufPtr, BusIOAddr)) == 0)
            {
                /*
                ** Transmit OK.  To speed things up we will queue up the
                ** mbufs on a to-be-freed chain and then free them all at
                ** the end.
                */

                mbufp = mbufp->m_nextpkt;
                CurMbufPtr->m_nextpkt = NULL;

                if (FreeFirst)
                    FreeLast->m_nextpkt = CurMbufPtr;
                else
                    FreeFirst = CurMbufPtr;

                FreeLast = CurMbufPtr;

                first = FALSE;
            }
            else
            {
                /*
                ** Transmit error. if this is the first packet, return error.
                ** Otherwise, free the rest of the packets and return OK.
                */

                if (WRK.machine != MACH_MEM_BASED) io_det(BusIOAddr);
                (void) i_enable(OldIntrPri);
                IntsDisabled = FALSE;

                if (first)
                {
                    nddp->ndd_genstats.ndd_oerrors++;

                    /*
                    ** Reset the device only if we are not on the
                    ** interrupt thread.
                    */

                    if (getpid() != -1)
                    {
                        ient_xmit_error();
                    }

                    TRACE_SYS(HKWD_IENT_ERR, "Ttx6", ENETDOWN, 0, 0);
                    DDrc = ENETDOWN;
                    break;
                }
                else
                {
                    while (CurMbufPtr = mbufp)
                    {
                        nddp->ndd_genstats.ndd_oerrors++;
                        mbufp = mbufp->m_nextpkt;
                        CurMbufPtr->m_nextpkt = (struct mbuf *) NULL;
                        m_freem(CurMbufPtr);
                    }

                    /*
                    ** Reset the device only if we are not on the
                    ** interrupt thread.
                    */

                    if (getpid() != -1)
                    {
                        ient_xmit_error();
                    }

                    TRACE_SYS(HKWD_IENT_XMIT, "Ttx7", 0, 0, 0);
                    DDrc = 0;
                    break;
                }

            }   /* if ((DDrc = ient_xmit(.... */

        }   /* if (WRK.xmits_buffered.... */
        else
        {
            /*
            ** If all mbufs can not be queued, then the address of the
            ** remaining buffer(s) is returned.
            ** Free and count the mbufs.
            */

            TRACE_DBG(HKWD_IENT_XMIT, "Ttx8", WRK.xmits_buffered, 0, 0);

            MbufToFree = ient_txq_put(CurMbufPtr);

            if (WRK.machine != MACH_MEM_BASED) io_det(BusIOAddr);
            (void) i_enable(OldIntrPri);
            IntsDisabled = FALSE;

            while (CurMbufPtr = MbufToFree)
            {
                MbufToFree = MbufToFree->m_nextpkt;
                CurMbufPtr->m_nextpkt = NULL;
                m_freem(CurMbufPtr);

                nddp->ndd_genstats.ndd_xmitque_ovf++;
                nddp->ndd_genstats.ndd_opackets_drop++;
            }

            DDrc = 0;
            break;
        }
    }    /* while (mbufp) */

    if (IntsDisabled)
    {
        if (WRK.machine != MACH_MEM_BASED) io_det(BusIOAddr);
        (void) i_enable(OldIntrPri);
    }

    /*
    ** Clean up the mbufs setting on the to-be-freed queue.
    */

    while (CurMbufPtr = FreeFirst)
    {
        FreeFirst = FreeFirst->m_nextpkt;
        CurMbufPtr->m_nextpkt = NULL;
        m_freem(CurMbufPtr);
    }

    TRACE_SYS(HKWD_IENT_XMIT,"TtxE", WRK.xmits_buffered,
                                     WRK.xmits_queued, DDrc);

    return(DDrc);

}

/*****************************************************************************/
/*
 * NAME:  ient_xmit
 *
 * FUNCTION:  Setup and start transmit commands for the Intel 82596.
 *
 * EXECUTION ENVIRONMENT: Both Process and Interrupt Threads.
 *
 * NOTES:  Interrupt thread if ient_restart is called via ient_RU_complete.
 *         TCP/IP ARPs (pings) received will be routed back around via the
 *         demuxer to here.
 *
 * CALLED FROM:  ient_output
 *
 * CALLS TO:   d_cflush
 *             ient_logerr  (i_enterr.c)
 *             ient_restart (i_enterr.c)
 *             m_copydata
 *             vm_cflush
 *             w_start
 *
 * INPUTS:   p_mbuf          - pointer to a packet in mbuf
 *           ioa             - handle for accessing I/O bus
 *
 * RETURNS:  0 (success) or errno
 *
 */
/*****************************************************************************/

int
ient_xmit(struct mbuf *p_mbuf, ulong ioa)
{

    ndd_t *p_ndd = (ndd_t *) &(NDD);
    ushort scb_status;
    ulong csc_value;
    int   bytes, save_bytes;
    int   offset;
    int   drc = 0;

    TRACE_SYS(HKWD_IENT_XMIT, "TxmB", (ulong)p_ndd, (ulong)p_mbuf,
              p_mbuf->m_pkthdr.len);

    /*
    ** Call ndd_trace if it is enabled
    */

    if (p_ndd->ndd_trace)
    {
        (*(p_ndd->ndd_trace))(p_ndd, p_mbuf, p_mbuf->m_data,
                                                    p_ndd->ndd_trace_arg);
    }

    /*
    ** Use the first available txd to transmit the packet
    */
    bytes = p_mbuf->m_pkthdr.len;
    save_bytes = bytes;

    /*
    ** copy data into tranmit buffer and do processor cache flush
    */
    m_copydata(p_mbuf, 0, bytes, (caddr_t)&WRK.xmit_buf_ptr[WRK.buffer_in].buf);

    /*
    ** Pad short packet with garbage and fill out the TBD
    */
    if (bytes < ENT_MIN_MTU)
        bytes = ENT_MIN_MTU;

    if (p_mbuf->m_flags & M_BCAST)
    {
        MIB.Generic_mib.ifExtnsEntry.bcast_tx_ok++;
    }
    if (p_mbuf->m_flags & M_MCAST)
    {
        MIB.Generic_mib.ifExtnsEntry.mcast_tx_ok++;
    }

    TRACE_BOTH(HKWD_IENT_OTHER, "Txm1", WRK.buffer_in, bytes, p_mbuf->m_flags);

    if (__power_rs())
        d_cflush(WRK.dma_channel, (caddr_t)&WRK.xmit_buf_ptr[WRK.buffer_in].buf,
                 save_bytes, (caddr_t)&WRK.xmit_buf_addr[WRK.buffer_in].buf);

    bytes = ((bytes & 0x00FF) << 8) | ((bytes & 0xFF00) >> 8);
    WRITE_LONG(WRK.tbd_ptr[WRK.buffer_in].control, bytes << 16 | XMIT_EOF);

    /*
    ** This device driver maintains a circular list of XMIT_BUFFER buffers
    ** for transmitting data.  The end of the list is the one indicated by
    ** "buffer_in".  When "buffer_in" is 0, then we are at the front of the
    ** chain.
    */

    WRITE_LONG(WRK.xcbl_ptr[WRK.buffer_in].xmit.csc,
                         XMIT_CMD | CB_SF | CB_INT | CB_SUS | CB_EL);

    if (WRK.xmits_buffered == 0)
    {
        /* Two reasons for this.  In real world, most of the time there
        ** will be only one buffer that we have to process most of the time.
        ** Two, it allows us to build the chaining and when the chain becomes
        ** full, transmit it as a whole.
        ** Wait for the status in SCB to indicate that the CU is finished
        ** processing commands.
        */

        COMMAND_QUIESCE(drc);
        if (drc)
        {
            TRACE_SYS(HKWD_IENT_ERR, "TtxA", NDD_ADAP_CHECK, 0, 0);
            ient_logerr(ERRID_IENT_FAIL, __LINE__, __FILE__, 0, drc,
                        WRK.dma_channel);
            ient_restart(NDD_ADAP_CHECK);
            return(drc);
        }

        /*
        **  put the bus address of the transmit CBL into the SCB
        */

        READ_SHORT(WRK.scb_ptr->status, &scb_status);

        if ((!(scb_status&STAT_CUS_ACTIVE))&&(WRK.buffer_in == WRK.buffer_out))
        {
            /*
            ** Put the address of the transmit command block into the SCB
            ** CBL word and start the CU by putting the start CU command
            ** into the SCB comamnd word and issuing a CHANNEL ATTENTION.
            */

            WRITE_LONG_REV(WRK.scb_ptr->cbl_addr,&WRK.xcbl_addr[WRK.buffer_in]);

            WRITE_SHORT(WRK.scb_ptr->command, CMD_CUC_START);

            if (__power_rsc())                 /* Just Salmon */
            {
                d_cflush(WRK.dma_channel,(caddr_t) &WRK.xcbl_ptr[WRK.buffer_in],
                         sizeof(struct xcbl),
                         (caddr_t) &WRK.xcbl_addr[WRK.buffer_in]);

                vm_cflush((caddr_t) WRK.scb_ptr, sizeof(struct scb));
            }

            CHANNEL_ATTENTION();

        }
        
        if (WRK.wdt_setter == WDT_INACTIVE)
        {
            WRK.wdt_setter = WDT_XMIT;
            w_start((struct watchdog *)&(WDT));
        }
    }

    WRK.xmits_buffered++;
    WRK.buffer_in = ++WRK.buffer_in % XMIT_BUFFERS;

    /*  For netpmon performance tool  */
    TRACE_SYS(HKWD_IENT_XMIT, "WEND", p_dev_ctl->seq_number, (int)p_mbuf, 0);

    TRACE_SYS(HKWD_IENT_XMIT, "TxmE", WRK.xmits_buffered, WRK.buffer_in,
              WRK.buffer_out);

    return(0);
}

/*****************************************************************************/
/*
 * NAME:    ient_txq_put
 *
 * FUNCTION:  Place incoming mbufs onto the software transmit queue.
 *
 * EXECUTION ENVIRONMENT: Both Process and Interrupt Threads.
 *
 * NOTES:  Interrupt thread if ient_restart is called via ient_RU_complete.
 *         TCP/IP ARPs (pings) received will be routed back around via the
 *         demuxer to here.
 *
 * CALLED FROM:  ient_output
 *
 * CALLS TO:
 *
 * INPUTS:   p_mbuf          - pointer to a mbuf chain
 *
 * RETURNS:  NULL - all mbufs are queued.
 *           mbuf pointer - point to a mbuf chain which contains packets
 *                          that overflows the software transmit queue.
 */
/*****************************************************************************/

struct mbuf *
ient_txq_put(struct mbuf *p_mbuf)
{

    int free_list;
    int pkt_cnt;
    struct mbuf *p_last, *p_over;

    TRACE_SYS(HKWD_IENT_XMIT, "TqpB", (ulong)p_dev_ctl, (ulong)p_mbuf, 0);

    pkt_cnt = 0;

    free_list = DDS.xmt_que_size - WRK.xmits_queued;

    if (free_list > 0)
    {
        p_last = p_mbuf;
        free_list--;
        pkt_cnt++;

        while (free_list && p_last->m_nextpkt)
        {
            /* For netpmon performance tool */
            TRACE_SYS(HKWD_IENT_XMIT, "WQUE", p_dev_ctl->seq_number,
                (int)p_last, p_last->m_pkthdr.len);
            free_list--;
            pkt_cnt++;
            p_last = p_last->m_nextpkt;
        }

        /*  For netpmon performance tool  */
        TRACE_SYS(HKWD_IENT_XMIT, "WQUE", p_dev_ctl->seq_number,
            (int)p_last, p_last->m_pkthdr.len);

        p_over = p_last->m_nextpkt;  /* overflow packets */

        if (WRK.txq_first)
        {
            WRK.txq_last->m_nextpkt = p_mbuf;
            WRK.txq_last = p_last;
        }
        else
        {
            WRK.txq_first = p_mbuf;
            WRK.txq_last = p_last;
        }
        p_last->m_nextpkt = NULL;
        WRK.xmits_queued += pkt_cnt;

        if (NDD.ndd_genstats.ndd_xmitque_max < WRK.xmits_queued)
            NDD.ndd_genstats.ndd_xmitque_max = WRK.xmits_queued;
    }
    else
    {
        p_over = p_mbuf;
    }

    TRACE_SYS(HKWD_IENT_XMIT, "TqpE", (ulong)p_over, free_list, 0);

    return(p_over);
}

/*****************************************************************************/
/*
 * NAME:    ient_xmit_error
 *
 * FUNCTION:  Clean up after a transmit error.
 *
 * EXECUTION ENVIRONMENT: Both Process and Interrupt Threads.
 *
 * NOTES:  Interrupt thread if ient_restart is called via ient_RU_complete.
 *         TCP/IP ARPs (pings) received will be routed back around via the
 *         demuxer to here.
 *
 * CALLED FROM:  ient_output
 *
 * CALLS TO:
 *
 * INPUTS:   None.
 *
 * RETURNS:  void
 */
/*****************************************************************************/

void
ient_xmit_error()
{
    ndd_statblk_t  stat_blk;   /* status block */
    ndd_t *p_ndd = (ndd_t *) &(NDD);
    struct mbuf   *p_mbuf;
    int oldpri;

    TRACE_SYS(HKWD_IENT_XMIT, "TxeB", WRK.xmits_buffered,
                                       WRK.xmits_queued, 0);

    /*
    ** Stop the watchdog timer
    */

    if (WRK.wdt_setter != WDT_INACTIVE)
    {
        w_stop((struct watchdog *)&(WDT));
        WRK.wdt_setter != WDT_INACTIVE;
    }

    oldpri = i_disable(DDS.intr_priority);

    /*
    ** Free up the mbufs sitting in the SoftQ.
    */

    if (WRK.xmits_queued)
    {
        while (p_mbuf = WRK.txq_first)
        {
            WRK.txq_first = p_mbuf->m_nextpkt;
            m_freem(p_mbuf);
            p_ndd->ndd_genstats.ndd_oerrors++;
        }

        WRK.xmits_queued = 0;
        WRK.txq_last = (struct mbuf *) NULL;
    }

    /* all the pending transmit packets will be accounted as errors */
    p_ndd->ndd_genstats.ndd_oerrors += WRK.xmits_buffered;

    (void) ient_restart(NDD_HARD_FAIL);

    (void) i_enable(oldpri);

    TRACE_SYS(HKWD_IENT_XMIT, "TxeE", 0, 0, 0);
    return;
}
