static char sccsid[] = "@(#)42  1.23  src/bos/kernext/ient/i_entintr.c, sysxient, bos41J, 9520B_all 5/18/95 12:41:02";
/*****************************************************************************/
/*
 *   COMPONENT_NAME: SYSXIENT
 *
 *   FUNCTIONS:
 *              ient_slih
 *              ient_RU_complete
 *              ient_xmit_done
 *              ient_action_done
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

void ient_xmit_done(register ient_dev_ctl_t *, ushort, ulong);
void ient_RU_complete(register ient_dev_ctl_t *, ushort, ulong, ulong);
void ient_action_done(register ient_dev_ctl_t *, ushort, ulong);

/*
** Global Definitions
*/

static uchar EthBCAddr[ENT_NADR_LENGTH] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};


/*****************************************************************************/
/*
 *
 * NAME:  ient_slih
 *
 * FUNCTION: Interrupt handler for the Integrated Ethernet Device Driver
 *
 * EXECUTION ENVIRONMENT: Interrupt thread only.
 *
 * NOTES:  This routine should always return INTR_SUCC because we are set
 *         up with interrupts not shared by the ODM busresolve() routine.
 *
 * CALLED FROM: extrn_flih()  
 *
 * CALLS TO:   ient_action_done
 *             ient_logerr  (i_enterr.c)
 *             ient_RU_complete
 *             ient_restart  (i_enterr.c)
 *             ient_xmit_done
 *             io_att
 *             io_det
 *             retry_get  (i_entutil.c)
 *             retry_put  (i_entutil.c)
 *
 * INPUTS:   ihsptr    - this is also the beginning of the DDS
 *
 * RETURNS:  INTR_SUCC - if it was our interrupt
 *           INTR_FAIL - if it was not our interrupt
 *
 */
/*****************************************************************************/

int
ient_slih(struct intr *ihsptr)
{
    register ient_dev_ctl_t *p_dev_ctl;
    ulong    status = 0;
    int      rc = 0;
    ulong    save_status, cur_cs_val;
    volatile ushort  SCB_StatVal, SCB_AckVal;
    uchar    value = 0;
    ulong    stat_reg, ioa;
    ulong    i = 0;

    TRACE_SYS(HKWD_IENT_OTHER, "SinB", (ulong) ihsptr, 0, 0);

    p_dev_ctl = (ient_dev_ctl_t *) ihsptr;

    /*************************************************************************
    **                                                                      **
    ** The 82596 interrupt handler will respond to the interrupt as follows:**
    **                                                                      **
    **    1. Check the status bits for interrupt in the status register.    **
    **                                                                      **
    ** In ack_interrupt():                                                  **
    **                                                                      **
    **    2. Check that the SCB command word is all zeros.                  **
    **    3. Set the interrupt acknowledge bits in the command word.        **
    **    4. Issue a Channel Attention to the 82596.                        **
    **    5. Reclaim the Command Block for future use.                      **
    **                                                                      **
    *************************************************************************/

    /* Read the ethernet status register word. */

    ioa = (ulong) io_att((DDS.bus_id | BUSMEM_IO_SELECT), 0);

    if (WRK.machine == MACH_MEM_BASED)          /* Rainbow or Salmon */
    {
        /*
         ====================================================================
          Bit 31 - +Channel Check
          Bit 30 - +Select Feed Back Error
          Bit 29 - +Data Parity Error
          Bit 28 - +Aborted Cycle
                                                 Memory Based Ethernet
          Bit 27 - Reserved                      Status Register
          Bit 26 - +Twisted Pair/-Thick
          Bit 25 - Fuse Good
          Bit 24 - -Ethernet Chip Interrupt

          Bit 0 -23 Reserved
         ====================================================================
        */

        stat_reg = ioa + (DDS.io_port + ENT_MEMBASED_STATUS_REG);

        if (BUS_GETLX((long *)stat_reg, (long *)&status))
            retry_get(stat_reg, 4, &status);

        save_status = status;

        if (status & MICRO_CHAN_ERR)     /* Micro Channel error has occurred */
        {
            /* CHANNEL CHECK, LEON, CHAPS or OLYMPUS, PARITY, SELECT FEEDBACK*/
            /* Make the interrupt bit consistent on all platforms */

            status &= 0xFFFFFFFE;

            if (__power_pc())             /* Is this a Rainbow?? */
            {
                TRACE_BOTH(HKWD_IENT_ERR, "Sin1", status, stat_reg, 0);

                /*
                ** Just check for ABORTED CYCLE.  The other MicroChannel
                ** errors will be handled below.
                */

                if (status & ABORTED_CYCLE)
                {
                    TRACE_BOTH(HKWD_IENT_ERR, "Sin2", status, stat_reg, 0);
                    
                    /* Just acknowledge the intr.. */
                    if (BUS_PUTLX((long *) stat_reg, 0x00000011))
                        retry_put(stat_reg, 4, 0x11);

                    /*
                    ** In 4.1 the flih handles the i_reset() so we no longer
                    ** have to call it.  intr.h #defines it as a NoOp.
                    */
                    
                    io_det(ioa);
                    return(INTR_SUCC);
                }
            }
        }
        WRK.do_ca_on_intr = TRUE;
    }
    else                                         /* This is a Stillwell */
    {
        if (BUS_GETCX((caddr_t)(ioa + ENT_STATUS_IO_BASED),
                      ((char *)(&status) + 3)))
                retry_get(ioa+ENT_STATUS_IO_BASED, 1, ((char *)(&status) + 3));

        save_status = status;

        /*
        ** Make the interrupt bit consistent on all platforms and log any
        ** MicroChannel errors.
        */

        if (status & ANY_MC_CHECK)
        {
            TRACE_BOTH(HKWD_IENT_ERR, "Sin3", save_status, ioa, 0);
            ient_logerr(ERRID_IENT_DMAFAIL, __LINE__, __FILE__, status, 0xdead,
                        0xcafe);
        }

        if (status & CHK_OR_NOT_INTR)
            status &= 0xFFFFFFFE;

        else
            status |= INTERRUPT;

    }

    TRACE_DBG(HKWD_IENT_OTHER, "Sin4", p_dev_ctl->device_state, status,
              stat_reg);

    if (!(status & INTERRUPT))
    {
        /*
        **  the 82596 interrupted, check for errors
        */

        if (WRK.machine == MACH_MEM_BASED)        /* Rainbow or Salmon */
        {
            if (status & MICRO_CHAN_ERR)
            {
                io_det(ioa);
                
                /*
                ** It appears that the only way that the software is able
                ** to handle a Channel Check is by resetting the adapter.
                */
                
                TRACE_BOTH(HKWD_IENT_ERR, "Sin5", status, INTR_SUCC, 0);
                ient_logerr(ERRID_IENT_DMAFAIL, __LINE__, __FILE__,
                            save_status, 0xaaaa, WRK.dma_channel);

                ient_restart(NDD_BUS_ERROR);
                return(INTR_SUCC);
            }
	}

        /*
        ** First we need to reset the interrupt condition in the I/O
        ** register for only Stilwells an Salmons.  HW people made
        ** us have to do a CHANNEL_ATTENTION before we can reset the
        ** interrupt bit.
        */
            
        if (__power_rsc())                            /* Salmon */
        {
            if (BUS_PUTLX((long *)stat_reg, RESET_STATUS_REG))
                retry_put(stat_reg, 4, RESET_STATUS_REG);
        }

        if (WRK.machine == MACH_IO_BASED)             /* Stillwell */
        {
            BUSPUTC(ENT_STATUS_IO_BASED, 0x0);
            BUSGETC(ENT_STATUS_IO_BASED, &value);
            while ((value & 0x03 ) && (i < 10))
            {
                BUSPUTC(ENT_STATUS_IO_BASED, 0x0);
                BUSGETC(ENT_STATUS_IO_BASED, &value);
                i++;
            }
        }
            
        if (p_dev_ctl->device_state == OPENED)
        {
            /*
            ** Check the status word in the SCB for what action to take.
	    ** Then acknowledge immediately via the SCB.
            ** If the RU finished receiving a frame (STAT_FR) and the RU is
            ** ready (STAT_RUS_READY) or the RU is suspended (STAT_RNR), then
            ** call ient_RU_complete().
            */

            READ_SHORT(WRK.scb_ptr->status, &SCB_StatVal);
	    SCB_AckVal = SCB_StatVal & CMD_ACK_MASK;

	    COMMAND_QUIESCE(rc);  /* Wait till SCB command word is all zeros */
	    if (!rc)
	    {
		WRITE_SHORT(WRK.scb_ptr->command, SCB_AckVal);
		CHANNEL_ATTENTION();

		TRACE_DBG(HKWD_IENT_OTHER, "Sin6", SCB_StatVal, SCB_AckVal,
			  WRK.scb_ptr);
    
		if ((SCB_StatVal & STAT_FR) ||
		    (SCB_StatVal & (STAT_RUS_NO_RESOURCE | STAT_RNR)))
		{
		    READ_LONG(WRK.rfd_ptr[WRK.begin_fbl].csc, &cur_cs_val);
		    ient_RU_complete(p_dev_ctl, SCB_StatVal, ioa, cur_cs_val);
		}

                /*
                ** Interrupt was a result of an action command completing with
                ** its I bit set (STAT_CX) or the CU becoming not ready
                ** (STAT_CNA).
                ** Determine type of command processing. If no action command is
                ** pending, then we must have completed a transmit command.
                */
    
		if (SCB_StatVal & (STAT_CX | STAT_CNA))
		{
		    if (WRK.control_pending == TRUE)
                    {
			ient_action_done(p_dev_ctl, SCB_StatVal, ioa);
		    }
		    else
		    { 
			/*
			** There is a possiblity that another control command
			** can be issued from within ient_action_done and could
			** have the execute_now flag set so make sure that one
			** is not pending before we transmit anything.
			*/

			if (!WRK.action_que_active && WRK.xmits_buffered)
			{
			    TRACE_DBG(HKWD_IENT_OTHER, "Sin8", SCB_StatVal, 0, 0);
			    ient_xmit_done(p_dev_ctl, SCB_StatVal, ioa);
			}
			else
			{
			    ient_action_done(p_dev_ctl, SCB_StatVal, ioa);
			}
		    }
		}
	    }
	    else
	    {
		/*
		** The Command Queisce failed so there must be something
		** wrong with the i596, so reset the hardware and bail.
		*/

		TRACE_BOTH(HKWD_IENT_ERR, "Sin9", rc, NDD_ADAP_CHECK, 0);
		ient_logerr(ERRID_IENT_FAIL, __LINE__, __FILE__, 0, 0xaaaa,
			    WRK.dma_channel);
		ient_restart(NDD_ADAP_CHECK);
	    }
        }
        rc = INTR_SUCC;
    }
    else
    {
        TRACE_SYS(HKWD_IENT_ERR, "SinA", status, INTR_SUCC,
                   p_dev_ctl->device_state);

        if (__power_pc()) 
		rc = INTR_SUCC;
        else
		rc = INTR_FAIL;
    }

    /*
    ** This is where we get to reset the interrupt on a Rainbow.
    */
    
    if (__power_pc())
    {
        /*
        ** The following is an ugly kludge that was forced on us by the HW
        ** people. It is only applicable to the Rainbows (2xx's).  We have to
        ** do a CHANNEL_ATTENTION in order to make sure the interrupt reset is
        ** performed correctly.
        */

        if (WRK.do_ca_on_intr)
        {
            WRITE_SHORT(WRK.scb_ptr->command, 0);   /* big NOP */

            CHANNEL_ATTENTION();
        }

        WRK.do_ca_on_intr = FALSE;

        if (BUS_PUTLX((long *) stat_reg, 0x00000001))
            retry_put(stat_reg, 4, 0x01);
    }

    io_det(ioa);
    TRACE_SYS(HKWD_IENT_OTHER, "SinE", rc, 0, 0);
    return(rc);
}

/*****************************************************************************/
/*
 *
 * NAME: ient_RU_complete
 *
 * FUNCTION: Completion routine for received Ethernet packets
 *
 * EXECUTION ENVIRONMENT:  Interrupt thread only.
 *
 * NOTES:  The d_complete must occur AFTER the check of the Complete bit.
 *         If the order were reversed, the C bit could be set after the
 *         d_complete  finished but before we looked at it, leaving a small
 *         portion of valid data in the IOCC buffers.
 *
 *         Never ever access in any way the receive buffers after they have
 *         been invalidated by vm_cflush.  This will corrupt the cache and
 *         return invalid data to the ULCS.
 *
 * CALLED FROM:  ient_slih
 *
 * CALLS TO:    bcopy
 *              d_complete
 *              cache_inval
 *              ient_badframe_processing
 *              ient_get_err_type
 *              ient_broadcast_addr
 *              ient_logerr   (i_enterr.c)
 *              ient_restart  (i_enterr.c)
 *              ient_start_ru (i_entopen.c)
 *              m_getclustm
 *              m_gethdr
 *              reverse_short (i_entutil.c)
 *              vm_cflush
 *
 * INPUT:     p_dev_ctl   -  pointer to device control structure
 *            SCB_CmdStatus - SCB status value
 *            ioa         -  pointer to system's BUS I/O address space.
 *            cur_cs_val  -  current value of receive frame descriptor.
 *
 * RETURNS:  N/A
 *
 */
/*****************************************************************************/

void
ient_RU_complete(register ient_dev_ctl_t *p_dev_ctl, ushort SCB_StatVal,
                 ulong ioa, ulong cur_cs_val)
{
    struct   mbuf       *mbufp;
    register ushort     SCB_ack_val;
    ushort              cnt_value;
    int                 cur_intr_pri;
    volatile int        errval, rc = 0;
    ndd_statblk_t       stat_blk;
    ulong               rbd_addr, buf_addr, work1; 
    uchar               broadcast_pkt, multicast_pkt, *dest_addr;
    ndd_t               *nddp;

    TRACE_SYS(HKWD_IENT_RECV, "SrcB", cur_cs_val, ioa, SCB_StatVal);

    /*
    ** Update statistics regarding receive interrupts.
    */

    nddp = (ndd_t *) &(NDD);
    
    if (NDD.ndd_genstats.ndd_recvintr_lsw == ULONG_MAX)
        NDD.ndd_genstats.ndd_recvintr_msw++;
    NDD.ndd_genstats.ndd_recvintr_lsw++;

    /*
    ** During this interrupt we are going to process as many receives as
    ** possible.  First we will check the status of the receive frame
    ** descriptor.  If it is busy, then we know that the 82596 RU has not
    ** completed receiving data into this buffer.
    **
    ** Never ever access in any way the receive buffers after they have been
    ** invalidated by vm_cflush. This will corrupt the cache and return bogus
    ** data to the ULCS.
    */
    
    while (TRUE)
    {
        TRACE_BOTH(HKWD_IENT_RECV, "Src1", cur_cs_val, WRK.begin_fbl, 0);

        /*
        ** WRK.begin_fbl is our place holder while processing receive
        ** interrupts.  When we leave this routine it will be pointing
        ** the receive frame descriptor that is currently busy. On the
        ** next receive interrupt processing will begin with that buffer.
        */
        
        if ( ! (cur_cs_val & CB_C))
        {
            /*
            ** We are looping until there are no more buffers to process.
            ** We reach this condition when either the BUSY bit is set or
            ** we have filled the receive queue and we had to run through
            ** the entire ring. We should have a RNR condition when we
            ** filled the ring so we need to check the RU status and then
            ** restart it if needed.
            */
            break;
        }

        /*
        ** The d_complete must occur AFTER the check of the status bits.
        ** If the order were reversed, the CB_C bit could be cleared after
        ** the d_complete finished but before we looked at it, leaving a
        ** small portion of valid data in the IOCC buffers.
        */

        if (__power_rs())
        {
            rc = d_complete(WRK.dma_channel, DMA_NOHIDE, (caddr_t) WRK.sysmem,
                            WRK.alloc_size, (struct xmem *)&WRK.xbuf_xd,
                            (caddr_t) WRK.dma_base);
            if (rc)
            {
                TRACE_BOTH(HKWD_IENT_ERR, "Src2", WRK.dma_channel, rc, 0);
                ient_logerr(ERRID_IENT_DMAFAIL, __LINE__, __FILE__, rc,
                            WRK.sysmem, WRK.dma_channel);
            }
        }
        
        /*
        ** Based on the size of the incoming data obtain a mbuf for the
        ** incoming packet and set the flags accordingly.
        **
        ** NOTE: In the 3.2 driver, a check was made to see if this was a
        ** good packet or not and wasted time jumping around handling this
        ** condition.  First, if we are executing here it is because either
        ** the incoming frame was Ok, OR the Intel 82596 has been configured
        ** to save bad frames, period.
        */
        
        READ_LONG_REV(WRK.rfd_ptr[WRK.begin_fbl].rbd, &rbd_addr);

        if (rbd_addr == ALLONES)
        {
            TRACE_DBG(HKWD_IENT_RECV, "Src3", rbd_addr, 0, 0);
            goto reset_rfd;
        }
        
        READ_LONG_REV(WRK.rbd_ptr[WRK.begin_fbl].rb_addr, &buf_addr);
            
        buf_addr = IOADDRTOSM(buf_addr);
        dest_addr = (uchar *) buf_addr;
        
        READ_SHORT_REV(WRK.rbd_ptr[WRK.begin_fbl].count, &cnt_value);
            
        cnt_value = cnt_value & 0x3FFF;      /* Mask off EL and F bits */
        
        TRACE_DBG(HKWD_IENT_RECV, "Src4", rbd_addr, buf_addr, cnt_value);

        if (cnt_value <= MHLEN)
        {
            mbufp = m_gethdr(M_DONTWAIT, MT_HEADER);
        }
        else
        {
            mbufp = m_getclustm(M_DONTWAIT, MT_HEADER, cnt_value);
        }

        /*
        ** Next check to see if this is a good or bad packet.
        ** RFDs with bit 13 (little endian) equal to zero are possible
        ** only if the save bad frames configuration option is selected.
        ** Otherwise, all frames with errors will be discarded, although
        ** statistics will be collected on them.  Instead of checking for
        ** CB_OK, we will apply the error mask.  We will take the "else"
        ** for gathering information about the bad frames.
        */
        
        if ((!(cur_cs_val & RECV_STAT_ERR_MASK)) &&
                                        (mbufp != (struct mbuf *) NULL))
        {
            /*
            ** No error, good packet.
            */

            mbufp->m_flags |= M_PKTHDR;
            mbufp->m_pkthdr.len = cnt_value;
            mbufp->m_len = cnt_value;

            /*  For netpmon performance tool  */
            TRACE_SYS(HKWD_IENT_RECV, "RDAT", p_dev_ctl->seq_number, cnt_value,
                      0);

            if (NDD.ndd_genstats.ndd_ipackets_lsw == ULONG_MAX)
                NDD.ndd_genstats.ndd_ipackets_msw++;

            NDD.ndd_genstats.ndd_ipackets_lsw++;

            if ((ULONG_MAX - cnt_value) < NDD.ndd_genstats.ndd_ibytes_lsw)
                NDD.ndd_genstats.ndd_ibytes_msw++;

            NDD.ndd_genstats.ndd_ibytes_lsw += cnt_value;

            /* Check for broadcasts and multicast addresses. */

            broadcast_pkt = FALSE;
            multicast_pkt = FALSE;

            if ((dest_addr[0] & MULTI_BIT_MASK) == MULTI_BIT_MASK)
            {
                if (SAME_NADR(dest_addr, EthBCAddr))
                {
                    broadcast_pkt = TRUE;
                    MIB.Generic_mib.ifExtnsEntry.bcast_rx_ok++;
                }
                else            /* MultiCast Address */
                {
                    multicast_pkt = TRUE;
                    MIB.Generic_mib.ifExtnsEntry.mcast_rx_ok++;
                }
            }

            TRACE_SYS(HKWD_IENT_OTHER, "Src5", cnt_value, broadcast_pkt,
		      multicast_pkt);

            if (broadcast_pkt)
                mbufp->m_flags |= M_BCAST;

            if (multicast_pkt)
                mbufp->m_flags |= M_MCAST;


            /* move the data into the mbuf */
            bcopy((char *) buf_addr, MTOD(mbufp, char *), cnt_value);

            /*  For netpmon performance tool  */
            TRACE_SYS(HKWD_IENT_RECV, "RNOT", p_dev_ctl->seq_number, mbufp,
                      cnt_value);
            
            /* Deliver the package. */
            (*(nddp->nd_receive))(nddp, mbufp);

            /*  For netpmon performance tool  */
            TRACE_SYS(HKWD_IENT_RECV, "REND", mbufp, 0, 0);

            /*
            ** Required for Stilwells, especially the 390.  If we don't
            ** invalidate the cache then we will send data that
            ** previously sent up stream.  This really caused problems
            ** on FTP puts.  The ACK packets coming back from the remote
            ** are less than a cache line and so instead of getting the
            ** correct data from memory, we were getting stale data from
            ** the cache.
            */

            cache_inval(buf_addr, cnt_value);

        }
        else
        {
            /*
            ** This was either a bad packet or we could not get the mbuf. If
            ** the packet was bad and we got the mbuf then fill out the status
            ** information and pass it up to the demuxer.
            */

            if (mbufp != (struct mbuf *) NULL)       /* Packet was bad */
            {
                TRACE_DBG(HKWD_IENT_ERR, "Src6", mbufp, cnt_value, 0);

                mbufp->m_flags |= M_PKTHDR;
                mbufp->m_pkthdr.len = cnt_value;
                mbufp->m_len = cnt_value;

                nddp->ndd_genstats.ndd_ibadpackets++;    /* Bump bad packets */

                bzero((caddr_t) &stat_blk, sizeof(ndd_statblk_t));

                /* move the data into the mbuf */
                bcopy((char *) buf_addr, MTOD(mbufp, char *), cnt_value);

                if (cnt_value < ENT_MIN_MTU)
                {
                    ENTSTATS.short_frames++;
                    errval = ENT_RCV_SHORT_ERR;
                }

                if (cnt_value > ENT_MAX_MTU)
                {
                    ENTSTATS.long_frames++;
                    errval = ENT_RCV_LONG_ERR;
                }

                if (WRK.badframe_user_count)
                {
                    switch (cur_cs_val & RECV_STAT_ERR_MASK)
                    {
                        case RECV_COLLISION:
                            errval = ENT_RCV_COLL;
                            break;
                        case RECV_SBF_TRUNC:
                            errval = ENT_RCV_SHORT_ERR;
                            break;
                        case RECV_EOP:
                            errval = ENT_RCV_LONG_ERR;
                            break;
                        case RECV_FRAME_SHORT:
                            errval = ENT_RCV_SHORT_ERR;
                            break;
                        case RECV_DMA_OVERRUN:
                            errval = ENT_RCV_OVRUN_ERR;
                            break;
                        case RECV_NO_RESOURCE:
                            errval = ENT_RCV_RSC_ERR;
                            break;
                        case RECV_ALIGN:
                            errval = ENT_RCV_ALIGN_ERR;
                            break;
                        case RECV_CRC:
                            errval = ENT_RCV_CRC_ERR;
                            break;
                        case RECV_LENGTH:
                            errval = ENT_RCV_LONG_ERR;
                            break;
                    }
                }

                stat_blk.code = (ulong) NDD_BAD_PKTS;
                stat_blk.option[0] = errval;
                stat_blk.option[1] = (ulong) mbufp;

                (*(nddp->nd_status)) (nddp, &stat_blk);
                
                /*
                ** This mbuf will not be freed by the status routine so we
                ** must free it here now.
                */

                m_free(mbufp);
                cache_inval(buf_addr, cnt_value);
            }
            else  /* We were unable to get an mbuf above */
            {
                TRACE_DBG(HKWD_IENT_ERR, "Src7", mbufp, cnt_value, 0);
                nddp->ndd_genstats.ndd_nobufs++;
                nddp->ndd_genstats.ndd_ipackets_drop++;
            }
        }

reset_rfd:
        
        /*
        ** Set the new EL before we release the old EL do we do not expose
        ** the RU to an infinite loop.  This is also a very critical path
        ** and if we are preempted by other parts of the system during this
        ** time we could be exposed to the RU so disable interrupts to INTMAX.
        */

        cur_intr_pri = i_disable(INTMAX);
        
        WRITE_LONG_REV(WRK.rbd_ptr[WRK.begin_fbl].size,
                       sizeof(WRK.recv_buf_ptr[WRK.begin_fbl].buf) | RBD_EL);
        WRITE_LONG_REV(WRK.rbd_ptr[WRK.begin_fbl].count, 0x00);

        WRITE_LONG(WRK.rfd_ptr[WRK.begin_fbl].rbd, ALLONES);
        WRITE_SHORT(WRK.rfd_ptr[WRK.begin_fbl].size, 0x00);
        WRITE_SHORT(WRK.rfd_ptr[WRK.begin_fbl].count, 0x00);
        WRITE_LONG(WRK.rfd_ptr[WRK.begin_fbl].csc, CB_EL | CB_SF);
        
        /*
        ** Now clear out old EL at former RFD and RBD end of lists
        ** preserving the csc and size information.
        **
        ** WRK.el_ptr = former RFD End of List index
        ** WRK.rbd_el_ptr = former RBD End of List address
        */

        READ_LONG_REV(WRK.rbd_ptr[WRK.end_fbl].size, &work1);
        work1 = work1 & (~(RBD_EL));
        WRITE_LONG_REV(WRK.rbd_ptr[WRK.end_fbl].size, work1);

        READ_LONG(WRK.rfd_ptr[WRK.el_ptr].csc, &work1);
        work1 = work1 & ((~(CB_EL)));
        WRITE_LONG(WRK.rfd_ptr[WRK.el_ptr].csc, work1);
        
        (void) i_enable(cur_intr_pri);     /* Reestablish interrupts */
        
        /* set up where the beginning and end of RFD is at */
        WRK.el_ptr = WRK.begin_fbl;
        WRK.end_fbl = WRK.begin_fbl;
        WRK.begin_fbl = (WRK.begin_fbl + 1) % ADP_RCV_QSIZE;

        /*
        ** Read status of the next receive frame descriptor and return to the
        ** top of the loop.
        */
        
        READ_LONG(WRK.rfd_ptr[WRK.begin_fbl].csc, &cur_cs_val);
        
    } /* while */
    
    /*
    ** If the RU has been shutdown because it it ran out of resources then
    ** we need to restart the RU.  This is done by copying the beginning of
    ** free buffer chain into the SCB receive frame address location and then
    ** ORing in the CMD_RUC_START with the acknowledgement bits.
    */

    if ( ! (SCB_StatVal & STAT_RUS_READY))
    {
        COMMAND_QUIESCE(rc);    /* Wait till SCB command word is all zeros */
        if (rc)
        {
            TRACE_BOTH(HKWD_IENT_ERR, "Rrc8", rc, NDD_ADAP_CHECK, 0);
            ient_logerr(ERRID_IENT_FAIL, __LINE__, __FILE__, 0, 0xaaaa,
                        WRK.dma_channel);
            ient_restart(NDD_ADAP_CHECK);
        }
        ENTSTATS.start_rx++;
        WRITE_LONG_REV(WRK.rfd_ptr[WRK.begin_fbl].rbd,
                       &WRK.rbd_addr[WRK.begin_fbl]);
        WRITE_LONG_REV(WRK.scb_ptr->rfa_addr, &WRK.rfd_addr[WRK.begin_fbl]);
        WRITE_SHORT(WRK.scb_ptr->command, CMD_RUC_START);
    
        CHANNEL_ATTENTION();
    }

    TRACE_SYS(HKWD_IENT_RECV, "SrcE", 0, 0, 0);
    return;
}


/*****************************************************************************/
/*
 * NAME: ient_xmit_done
 *
 * FUNCTION:  Transmit complete processing. Free resources and acknowledge
 *            transmit done interrupt. Moves transmit elements from the waitq
 *            to the transmit ready queue if necessary.
 *            Links all available xmits into a linked list and starts CU
 *
 * EXECUTION ENVIRONMENT:  Interrupt thread only.
 *
 * NOTES:     This function is to be called on each interrupt when xmits
 *            are pending so that some 82596 errata can be handled and so
 *            that any xmits which were waiting for service can be started.
 *
 *            Any xmits that were placed in buffers by ient_output and not
 *            immediately started because the CU was busy are started here.
 *            Also, any action commands that could not be immediately started
 *            because xmits were in progress are started here.
 *
 *            The condition where the busy bit is set and the CU is
 *            suspended is an invalid condition covered by 82596 errata.
 *            The busy bit indicates the CU is busy processing this CBL,
 *            yet the CU is suspended. Restart the CU.
 *
 *            Move queued transmit elements to the ready queue as buffers
 *            are freed.  This allows us to link all possible xmits into
 *            a list that will execute with a single terminating interrupt.
 *            It also preserves the packet ordering since i_entout cannot
 *            insert packets in freed buffers until the queue is satisfied.
 *
 *            The watchdog timer is affected only if:
 *
 *                1. It is already running for xmits, and
 *                   we actually see a xmit completion; or
 *
 *                2. We start an xmit that was waiting.
 *
 *
 * CALLED FROM: ient_slih
 *              ient_timeout (i_enterr.c)
 *
 * CALLS TO:    d_complete
 *              ient_action  (i_entopen.c)
 *              ient_logerr  (i_enterr.c)
 *              m_copydata
 *              m_freem
 *              vm_cflush
 *              w_start
 *              w_stop
 *
 * INPUTS:   p_dev_ctl    -  pointer to device control structure
 *           SCB_StatVal  -  status field of SCB
 *           ioa          -  attached I/O address
 *
 * RETURNS:  N/A
 *
 */
/*****************************************************************************/

void
ient_xmit_done(register ient_dev_ctl_t *p_dev_ctl, ushort SCB_StatVal, ulong ioa)
{
    ulong   xcbl_status;
    int     bytes, need_a_CA = TRUE;
    int     counter, drc = 0;
    ndd_t   *nddp;
    ulong   nbr_collisions;
    struct  mbuf *p_mbuf;
    ushort  cmd_value;
    ulong   status, xmit_active = 0;

    TRACE_SYS(HKWD_IENT_XMIT, "TxdB", (ulong) p_dev_ctl, SCB_StatVal,
               WRK.xmits_buffered);

    /*
    ** Even though the Intel book says that we can chain together multiple
    ** transmit command blocks, due to errata in the 82596 chip, they can
    ** NOT be handled by just one interrupt. (i.e., turn interrupt bit on
    ** in last command block and then wait for that interrupt to occur.
    ** No, I have to turn the suspend bit on in each command block and then
    ** take an interrupt on each buffer (packet) transmitted.  I can still
    ** chain them together and have the last one specify the EL bit for all
    ** the good it does me.
    */

    TRACE_DBG(HKWD_IENT_XMIT, "Txd1", WRK.buffer_in, WRK.buffer_out, 0);

    /*
    ** Read the xcbl status of the current sequential buffer. If it is marked
    ** complete, then process the buffer. Else, check for more errata.
    */

    READ_LONG(WRK.xcbl_ptr[WRK.buffer_out].xmit.csc, &xcbl_status);
    if (SCB_StatVal & STAT_CUS_TIMEOUT)
    {
        /* If we timed out, force it to completion. */
        xcbl_status |= CB_C;
        xcbl_status &= ~(CB_B);
    }

    TRACE_SYS(HKWD_IENT_XMIT, "Txd2", SCB_StatVal, xcbl_status, status);

    if (xcbl_status & CB_B)
    {
        /*
        ** If the transmit command block's busy bit is set and the CU
        ** is suspended, we hit another one of the 82596 errata.
        ** Resume CU.
        */

        TRACE_SYS(HKWD_IENT_XMIT, "Txd3", SCB_StatVal, xcbl_status, 0);
        
        if (SCB_StatVal & STAT_CUS_SUSPEND)
        {
            COMMAND_QUIESCE(drc);
            WRITE_SHORT(WRK.scb_ptr->command, (ushort)CMD_CUC_RES);
            CHANNEL_ATTENTION();
        }

        return;
    }
    
    if (WRK.wdt_setter != WDT_INACTIVE)
    {
        WRK.wdt_setter = WDT_INACTIVE;
        w_stop((struct watchdog *)&(WDT));
    }

    while (TRUE)
    {
        TRACE_BOTH(HKWD_IENT_XMIT, "Txd4", xcbl_status,counter,WRK.buffer_out);

        if ( ! (xcbl_status & CB_C))
        {
            break;
        }
        
        if (__power_rs())
        {
            drc = d_complete(WRK.dma_channel, DMA_NOHIDE,
                             WRK.xmit_buf_ptr[WRK.buffer_out].buf, PACKET_SIZE,
                             (struct xmem *)&WRK.xbuf_xd, (char *)WRK.dma_base);

            if (drc)
            {
                TRACE_BOTH(HKWD_IENT_ERR, "Txd5", WRK.dma_channel,
                           WRK.xmit_buf_ptr[WRK.buffer_out].buf, drc);
                ient_logerr(ERRID_IENT_DMAFAIL, __LINE__, __FILE__, 0x1149,
                            drc, WRK.dma_channel);
            }
        }
        
        status = xcbl_status & STAT_FIELD_MASK;

        if (WRK.xmits_buffered)
            WRK.xmits_buffered--;

        if (NDD.ndd_genstats.ndd_xmitintr_lsw == ULONG_MAX)
            NDD.ndd_genstats.ndd_xmitintr_msw++;

        NDD.ndd_genstats.ndd_xmitintr_lsw++;

        if (status & XMIT_STAT_MASK)
        {
            TRACE_DBG(HKWD_IENT_XMIT, "Txd6", xcbl_status, 0, 0);

            if (status & XMIT_STAT_NCS)
            {
                /*
                ** Carrier sense signal not detected during transmission.
                ** Probably cause: Cable not attached to port.
                */
                ENTSTATS.carrier_sense++;
                NDD.ndd_genstats.ndd_oerrors++;
                ABORT_CU();
            
            }

            if (status & XMIT_STAT_LC)
            {
                ENTSTATS.late_collisions++;
                NDD.ndd_genstats.ndd_oerrors++;
            }

            if (status & XMIT_STAT_NCS)
            {
                ENTSTATS.carrier_sense++;
                NDD.ndd_genstats.ndd_oerrors++;
            }
    
            if (status & XMIT_STAT_CSS)
            {
                ENTSTATS.cts_lost++;
                NDD.ndd_genstats.ndd_oerrors++;
            }
    
            if (status & XMIT_STAT_DU)
            {
                ENTSTATS.underrun++;
                NDD.ndd_genstats.ndd_oerrors++;
            }
    
            if (status & XMIT_STAT_TD)
            {
                ENTSTATS.defer_tx++;
            }

            if (status & XMIT_STAT_MAXCOLL)
            {
                nbr_collisions = xcbl_status;
                nbr_collisions >>= 24;
                nbr_collisions &= 0x0F;
                DEVSTATS.coll_freq[nbr_collisions - 1]++;
    
                /* Update MIBS statistics. */
                if (nbr_collisions == 1)
                {
                    /* Single collision. */
                    MIB.Ethernet_mib.Dot3StatsEntry.s_coll_frames++;
                }
                else
                {
                    /* Multiple collision. */
                    MIB.Ethernet_mib.Dot3StatsEntry.m_coll_frames++;
                }
            }

            if (status & XMIT_STAT_STOP)
            {
                NDD.ndd_genstats.ndd_oerrors++;
                ENTSTATS.excess_collisions++;
                DEVSTATS.coll_freq[15]++;
            }
        }

        READ_LONG(WRK.tbd_ptr[WRK.buffer_out].control, &bytes);
        bytes &= ~XMIT_EOF;
        bytes >>= 16;
        bytes = ((bytes & 0x00FF) << 8 ) | ((bytes & 0xFF00) >> 8);

        if (NDD.ndd_genstats.ndd_opackets_lsw == ULONG_MAX)
            NDD.ndd_genstats.ndd_opackets_msw++;

        NDD.ndd_genstats.ndd_opackets_lsw++;

        if ((ULONG_MAX - bytes) < NDD.ndd_genstats.ndd_obytes_lsw)
            NDD.ndd_genstats.ndd_obytes_msw++;

        NDD.ndd_genstats.ndd_obytes_lsw += bytes;


        if (WRK.xmits_queued > 0)
        {
            /*
            ** If packets have been queued up on the chain.  Let the first
            ** one on the chain occupy the one we just processed.
            */
            
            TRACE_DBG(HKWD_IENT_XMIT, "Txd8", WRK.txq_first, WRK.buffer_out,
                      WRK.xmits_queued);

            p_mbuf = WRK.txq_first;
            WRK.txq_first = p_mbuf->m_nextpkt;
            p_mbuf->m_nextpkt = NULL;

            nddp = (ndd_t *) &(NDD);

            if (nddp->ndd_trace)
            {
                (*(nddp->ndd_trace))(nddp, p_mbuf, p_mbuf->m_data,
                                     nddp->ndd_trace_arg);
            }

            /*  For netpmon performance tool  */
            TRACE_SYS(HKWD_IENT_XMIT, "WEND", p_dev_ctl->seq_number,
                      (int)p_mbuf, 0);

            /*
            ** Set up to transmit the packet.
            ** Copy data into tranmit buffer and do processor cache flush
            ** for non-memory mapped machines.
            ** Free up the mbufs that we just transferred to a buffer.
            */

            bytes = p_mbuf->m_pkthdr.len;
            m_copydata(p_mbuf, 0, bytes, 
                       (caddr_t) &WRK.xmit_buf_ptr[WRK.buffer_out].buf);

            /* Update MIBS statistics. */
            if (p_mbuf->m_flags & M_BCAST)
            {
                MIB.Generic_mib.ifExtnsEntry.bcast_tx_ok++;
            }
            if (p_mbuf->m_flags & M_MCAST)
            {
                MIB.Generic_mib.ifExtnsEntry.mcast_tx_ok++;
            }

            if (__power_rs())
            {
                vm_cflush((caddr_t) &WRK.xmit_buf_ptr[WRK.buffer_out].buf,
                          bytes);
            }

            m_freem(p_mbuf);

            if (WRK.xmits_queued) WRK.xmits_queued--;

            WRK.xmits_buffered++;

            /* Pad short packet with garbage. */
            if (bytes < ENT_MIN_MTU) bytes = ENT_MIN_MTU;

            bytes = ((bytes & 0x00FF) << 8) | ((bytes & 0xFF00) >> 8);

            WRITE_LONG(WRK.tbd_ptr[WRK.buffer_out].control, 
                       (bytes << 16 | XMIT_EOF));

            WRITE_LONG(WRK.xcbl_ptr[WRK.buffer_out].xmit.csc, 
                       XMIT_CMD | CB_SF | CB_INT | CB_SUS | CB_EL);

            WRK.buffer_in = (WRK.buffer_in + 1) % XMIT_BUFFERS;
        }
        else
        {
            /* Clear the xcbl command to a noop. */
            WRITE_LONG(WRK.xcbl_ptr[WRK.buffer_out].xmit.csc, 
                                                NOP_CMD | CB_SUS | CB_EL);
        }

        WRK.buffer_out = (WRK.buffer_out + 1) % XMIT_BUFFERS;
            
        READ_LONG(WRK.xcbl_ptr[WRK.buffer_out].xmit.csc, &xcbl_status);
        
        /*
        ** If the command has not started let's kick it and also ack the
        ** SCB completion.
        */
        
        if (xcbl_status == (XMIT_CMD | CB_SF | CB_INT | CB_SUS | CB_EL))
        {
            COMMAND_QUIESCE(drc);
            
            WRITE_LONG_REV(WRK.scb_ptr->cbl_addr,
                           &WRK.xcbl_addr[WRK.buffer_out]);

	    /*
	    ** If the CU is not ACTIVE, ie., Idle or Suspended, kick it.
	    */

            if ( ! (SCB_StatVal & STAT_CUS_ACTIVE))
            {
                WRITE_SHORT(WRK.scb_ptr->command, CMD_CUC_START);
            }
            CHANNEL_ATTENTION();
            break;
        }
    } /* End of for (i = 0; i < XMIT_BUFFERS..... */
    
    if (!WRK.xmits_buffered && WRK.action_que_active)
    {
        (void) ient_action(WRK.action_que_active, TRUE);
    }
    
    /*
    ** Restart the timer if we still have transmits buffered.
    */

    if ((WRK.wdt_setter == WDT_INACTIVE) && (WRK.xmits_buffered))
    {
        WRK.wdt_setter = WDT_XMIT;
        w_start((struct watchdog *)&(WDT));
    }

    TRACE_SYS(HKWD_IENT_XMIT, "TxdE", WRK.xmits_buffered, WRK.buffer_in,
               WRK.buffer_out);

    return;
}

/*****************************************************************************/
/*
 *
 * NAME:  ient_action_done
 *
 * FUNCTION:  Completion processing for all miscellanoues action commands
 *
 * EXECUTION ENVIRONMENT:  Interrupt thread only.
 *
 * NOTES:
 *
 * CALLED FROM: ient_slih
 *
 * CALLS TO:   ient_action (i_entopen.c)
 *             ient_logerr (i_enterr.c)
 *             ient_restart (i_enterr.c)
 *             w_stop
 *
 * INPUTS:   p_dev_ctl     - pointer to device control structure.
 *           stat_value    - SCB status value
 *           ioa           - pointer to system's BUS I/O address space.
 *
 * RETURNS:  N/A
 *
 */
/*****************************************************************************/

void
ient_action_done(register ient_dev_ctl_t *p_dev_ctl, ushort stat_value,
                 ulong ioa)
{
    int     rc = 0, drc = 0;
    ulong   value, k;
    ushort  cmd_value;

    TRACE_SYS(HKWD_IENT_OTHER, "AadB", (ulong)p_dev_ctl, stat_value, 0);

    if ((WRK.wdt_setter != WDT_INACTIVE) && (!WRK.xmits_buffered))
    {
        WRK.wdt_setter = WDT_INACTIVE;
        w_stop((struct watchdog *)&(WDT));
    }

    WRK.control_pending = FALSE;

    WRITE_LONG(WRK.acbl_ptr[0].csc, NOP_CMD);

    /*
    ** If there is another action waiting, dispatch it
    */
    
    if (WRK.action_que_active)
    {
        ient_action(WRK.action_que_active, TRUE);
    }

    TRACE_SYS(HKWD_IENT_OTHER, "AadE", 0, 0, 0);

    return;
}
