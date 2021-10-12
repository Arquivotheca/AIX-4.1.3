static char sccsid[] = "@(#)58  1.38  src/bos/kernext/cat/cat_util.c, sysxcat, bos411, 9428A410j 2/22/94 16:54:03";
/*
/*
 * COMPONENT_NAME: (SYSXCAT) - Channel Attach device handler
 *
 * FUNCTIONS: clean_sc_queues(), dma_request()
 * async_status(), reserve_pca_xmit(),
 * cat_wait_ack(), notify_all_opens(), cat_read_buf()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#define FNUM 10

#include <net/spl.h>
#include <sys/adspace.h>
#include <sys/errno.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>
#include <sys/comio.h>
#include <sys/device.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/sleep.h>
#include <sys/dma.h>
#include <sys/malloc.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/except.h>
#include <sys/errids.h>

#include "catdd.h"

/*****************************************************************************
 *
 * NAME:        dma_request()
 *
 * FUNCTION:    Process the DMA request at the head of the DMA queue.
 *
 * EXECUTION ENVIRONMENT:       process and interrupt
 *
 * NOTES:
 *              There are two types of dma requests supported: XMIT and RECV.
 *              For XMIT requests, we will check the current element to see
 *              if it is the last in a chain of transmit buffers to be
 *              transferred to the PCA.  If it is the last of a chain, then
 *              we will write a command to the PCA to transmit the list of
 *              buffers that have been previously DMA'd to the PCA.
 *
 *              For RECV requests, we will check the current element to see
 *              if it is the last in a chain of buffers from the PCA. If it
 *              is the last of the chain, then it will wakeup any processes
 *              sleeping on a read.  For a KERNEL process, it will copy the
 *              completed buffer into mbufs and call the kernel process's
 *              receive function
 *
 * Input:       ca - structure for this adapter
 *
 * Returns:     0 ------ queued successfully
 *
 ****************************************************************************/
int
dma_request(
        struct ca *ca,
        dma_req_t *new)
{
        open_t *openp;
        recv_elem_t *recvp;
        xmit_elem_t *xmitp;
        dma_req_t *dmap;
        dma_req_t *new_last;
        ulong saddr;
        ulong bus;
        int reg;
        int rc;
        int spl;
        int i;
        cio_read_ext_t rd_ext;
        struct mbuf *m_ptr;

        DISABLE_INTERRUPTS(spl);

        /*
        ** Add the element(s) into the DMA request queue
        **/
        if (new) {
                if (ca->dma_act == NULL)
                        ca->dma_act = new;
                else {
                        new_last = new->dm_last;
                        new->dm_last = ca->dma_act->dm_last;
                        new->dm_last->dm_next = new;
                        ca->dma_act->dm_last = new_last;
                }
        }

start_dma:
        if (ca->flags & CAT_PAUSED) {
                ENABLE_INTERRUPTS(spl);
                return 0;
        }

        if ((dmap = ca->dma_act) == NULL) {     /* Always use first element */
                /*
                ** This should only happen when a pseudo xmit or
                ** recv was last on the queue and hits the goto...
                */
                ENABLE_INTERRUPTS(spl);
                return 0;
        }

        switch (dmap->dm_state) {


        case DM_STARTED:
        case DM_ABORTED:
                break;

        case DM_READY: {                /* start a DMA */
                switch (dmap->dm_req_type) {
                case DM_XMIT:
                case DM_RECV: {
                        /*
                        ** Set up the DMA channel for the transfer
                        ** d_slave is a void function so we are working
                        ** on faith here.
                        */
                        if (dmap->dm_length & 3)
                                dmap->dm_dmalen = (dmap->dm_length & ~3) + 4;
                        else
                                dmap->dm_dmalen = dmap->dm_length;
                        d_slave(ca->dma_channel, dmap->dm_flags,
                                dmap->dm_buffer,dmap->dm_dmalen, dmap->dm_xmem);
                        saddr = ((int)(dmap->dm_pca_buffer)) >> 2;
                        letni32(&saddr);

                        /*
                        ** Poke the adapter to start the DMA
                        ** and set the next DMA state.
                        */
                        bus = CAT_MEM_ATT;
                        ca->piorc = BUS_PUTLX((bus + (DMA_MAGIC_SPOT & 0x0fffffff)), saddr);
                        if (ca->piorc)
                                ca->piorc =
                                        BUS_PUTLX((bus + (DMA_MAGIC_SPOT & 0x0fffffff)), saddr);
                        if (ca->piorc)
                                ca->piorc =
                                        BUS_PUTLX((bus + (DMA_MAGIC_SPOT & 0x0fffffff)), saddr);
                        if (ca->piorc)
                                cat_logerr(ca, ERRID_CAT_ERR8);
                        BUSMEM_DET(bus);        /* release access to MCI bus */
                        /*
                        ** if CAT_WRITE() failed the adapter has been shutdown,
                        ** just return.
                        */
                        if (ca->piorc) {
                                free_dma_request(ca, dmap);
                                ENABLE_INTERRUPTS(spl);
                                return EIO;
                        }
                        dmap->dm_state = DM_STARTED;
                }       break;

                case DM_PSEUDO_XMIT: {
                        /*
                        ** This is a pseudo-DMA request used to kick
                        ** off the adapter transmit command.
                        */
                        xmitp = dmap->p.xmit_ptr;
                        free_dma_request(ca, dmap);
                        rc = send_elem(ca, xmitp);
                        ASSERT(rc == 0);
                        goto start_dma;
                } break;

                case DM_PSEUDO_RECV: {
                        /*
                        ** Get the receive element and open ptr.
                        */
                        recvp = dmap->p.recv_ptr;
                        ASSERT(recvp);
                        openp = recvp->rc_open;
                        ASSERT(openp);


                        /*
                        ** now we have a valid receive element
                        */
                        if (openp->op_mode & DKERNEL) {
                                recvp->rc_state = RC_COMPLETE;
                                /*
                                ** Fill in the read extension fields.
                                */
                                rd_ext.status = CIO_OK;
                                rd_ext.netid = recvp->rc_scid;
                                rd_ext.sessid = recvp->rc_linkid;

                                /*
                                ** Notify the kernel user of data received.
                                */
                                m_ptr = recvp->rc_mbuf_head;

                                /*
                                ** Set the packet length in the header
                                ** (created in pscaxbuf()) so TCP/IP
                                ** doesn't have to calculate it.
                                */
                                m_ptr->m_pkthdr.len = recvp->rc_resid;
                                recvp->rc_mbuf_head = NULL;
                                (*(openp->op_rcv_fn))(openp->op_open_id,
                                        &(rd_ext), m_ptr);

                                /*
                                ** Notify the SYNC_MODE-mode user via the status queue.
                                */
                                if (openp->op_flags & OP_SYNC_MODE) {
                                        async_status(ca, openp, CIO_ASYNC_STATUS,
                                                CIO_OK, recvp->rc_scid, recvp->rc_ccw, 0);
                                }

                                free_dma_request(ca, dmap);
                                free_recv_element(ca, recvp);
                        } else {
                                recvp->rc_state = RC_COMPLETE;
                                free_dma_request(ca, dmap);

                                /*
                                ** User-mode process:
                                ** If user is blocked on read, do a wakeup.
                                */
                                if (openp->op_rcv_event != EVENT_NULL)
                                        e_wakeup(&openp->op_rcv_event);

                                /*
                                ** Notify the SYNC_MODE-mode user via the status queue.
                                */
                                if (openp->op_flags & OP_SYNC_MODE) {
                                        async_status(ca, openp, CIO_ASYNC_STATUS,
                                                CIO_OK, recvp->rc_scid, recvp->rc_ccw, 0);
                                }

                                /*
                                ** Notify the user vi poll/select mechanism.
                                */
                                if (openp->op_select & POLLIN) {
                                        selnotify((int)ca->dev, openp->op_chan, POLLIN);
                                }

                                /*
                                ** Free the receive element in catread()...
                                */
                        }
                        goto start_dma;
                } break;

                default:
                        panic("invalid req_type in dma_request()");
                        /* logerr() */
                        break;

                } /* end switch */
        } break;

        default:
                panic("invalid state in dma_request()");
                /* logerr() */
                break;

        } /* END SWITCH */
        ENABLE_INTERRUPTS(spl);
        return 0;
} /* dma_request() */


/*****************************************************************************
 *
 * NAME:        dma_alloc
 *
 * FUNCTION:    Allocate, zero, and return the requested number of DMA elements.
 *
 * EXECUTION ENVIRONMENT:       both process and interrupt
 *
 * NOTES:
 *
 * RETURNS:     NULL: couldn't allocate requested elements.
 *              ptr to chain of requested elementes
 *
 ****************************************************************************/
dma_req_t *
dma_alloc(
        struct ca *ca,
        caddr_t owner,
        int nelem)
{
        dma_req_t *dmap;
        dma_req_t *tp;
        int i;
        int spl;

CATDEBUG(("in dma_alloc()\n"));

        DISABLE_INTERRUPTS(spl);
        if (ca->dma_free == NULL || ca->dma_nfree < nelem) {
                cat_logerr(ca, ERRID_CAT_ERR7);
                ENABLE_INTERRUPTS(spl);
                return NULL;
        }
        ca->dma_nfree -= nelem;
        tp = ca->dma_free;
        dmap = ca->dma_free;

        /*
        ** Zero out the free elements
        */
        for (i=1; i<nelem; i++) {
                bzero(dmap, (sizeof(dma_req_t)-(2*sizeof(long))));
                dmap->p.owner_ptr = owner;
                dmap = dmap->dm_next;
        }
        bzero(dmap, (sizeof(dma_req_t)-(2*sizeof(long))));
        dmap->p.owner_ptr = owner;

        /*
        ** Unlink the elements from the free list
        */
        if ((ca->dma_free = dmap->dm_next) != NULL)
                ca->dma_free->dm_last = tp->dm_last;
        tp->dm_last = dmap;
        dmap->dm_next = NULL;

        ENABLE_INTERRUPTS(spl);

        return tp;
} /* dma_alloc() */



/*****************************************************************************
 *
 * NAME:        free_dma_request
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:
 *
 ****************************************************************************/
dma_req_t * free_dma_request(
        struct ca *ca,
        dma_req_t *dmap)
{
        int spl;
        dma_req_t *active_dmap;

CATDEBUG(("free_dma_request()\n"));
        ASSERT(dmap);

        DISABLE_INTERRUPTS(spl);

        /* d50453 make sure this dma element is really on the
        ** active list
        */
        active_dmap = ca->dma_act;
        while (active_dmap) {
               if(dmap==active_dmap) break;
               active_dmap = active_dmap->dm_next;
        }
	   if (dmap != active_dmap ) {
	        ENABLE_INTERRUPTS(spl);
     	   return;
	   }
/* d50453 addition end */


        /* d50453
           To avoid returning the same receive buffer twice
           check fro dm_rfb == 0 and after the buffer has
           been returned set to 0.
     */
        if (dmap->dm_rfb != 0 ){
           cat_put_rfb(ca,&(dmap->dm_rfb));
           dmap->dm_rfb = 0;
     }

        /*
        ** Take it off the active list
        */
        if (ca->dma_act == dmap)
                ca->dma_act = dmap->dm_next;
        else
                dmap->dm_last->dm_next = dmap->dm_next;
        if (dmap->dm_next != NULL)
                dmap->dm_next->dm_last = dmap->dm_last;
        else if (ca->dma_act != NULL)
                ca->dma_act->dm_last = dmap->dm_last;
        dmap->dm_next = NULL;
        dmap->dm_last = dmap;

        /*
        ** Put it on the free list
        */
        if (ca->dma_free == NULL)
                ca->dma_free = dmap;
        else {
                dmap->dm_last = ca->dma_free->dm_last;
                dmap->dm_last->dm_next = dmap;
                ca->dma_free->dm_last = dmap;
        }
        ca->dma_nfree++;

        if (ca->dmabuf_event != EVENT_NULL) {
                CATDEBUG(("free_dma_request: waking dmabuf_event\n"));
                e_wakeup(&ca->dmabuf_event);
        }

        ENABLE_INTERRUPTS(spl);
        return;
} /* free_dma_request() */


/*****************************************************************************
 *
 * NAME:        free_recv_element
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:
 *
 ****************************************************************************/
void
free_recv_element(
        struct ca *ca,
        recv_elem_t *recvp)
{
        recv_elem_t *tp;
        recv_elem_t *active_recv;    /* d50453 */
        open_t *openp;
        dma_req_t *dmap;
        dma_req_t *tmp_dmap;
        int already_locked;
        int spl;
        int i;
        int dma_aborted = 0;

        ASSERT(recvp);

        /*
        ** Free or abort all DMA requests associated with this element.
        **
        ** This is done with interrupts disabled so that an
        ** interrupt handler can not change the state of an
        ** element while we are handling it.
        */
        openp = recvp->rc_open;
        DISABLE_INTERRUPTS(spl);  /*d50453 moved from below */

        /* d50453 make sure this element is really on the
        ** active list
        */
        active_recv = openp->recv_act;
        while (active_recv) {
               if(recvp==active_recv) break;
               active_recv = active_recv->rc_next;
        }

        if (recvp != active_recv) {
          ENABLE_INTERRUPTS(spl);
          return;
        } /* endif */

/* code added for d50453 ends here */

        dmap = ca->dma_act;
        while (dmap) {
                if (dmap->p.recv_ptr == recvp) {
                        if (dmap == ca->dma_act &&
                                (dmap->dm_state == DM_STARTED
						  || dmap->dm_state == DM_ABORTED)) { /* d50453 */
                                dmap->dm_state = DM_ABORTED;
                                dma_aborted = 1;
                                dmap = dmap->dm_next;
                        } else {
                                tmp_dmap = dmap;
                                dmap = dmap->dm_next;
                                free_dma_request(ca, tmp_dmap);
                        }
                } else {
                        dmap = dmap->dm_next;
                }
        }

        /*
        ** If we had to abort a DMA that was already in progress,
        ** leave the resources intact and have pscadmac() call this
        ** routine to release the resources (mbuf chain, recv elem)
        ** at that time.
        */
        if (dma_aborted) {
                ENABLE_INTERRUPTS(spl);
                return;
        }

        /*
        ** Free any mbufs associated with this element
        */
        if (recvp->rc_mbuf_head && (openp->op_mode & DKERNEL) == 0) {
/*d50453*/ ca->mbuf_num-=num_mbufs(recvp->rc_mbuf_head);
                m_freem(recvp->rc_mbuf_head);
                recvp->rc_mbuf_head = NULL;
                if (ca->mbuf_event != EVENT_NULL)
                        e_wakeup(&ca->mbuf_event);
        }

        /*
        ** Take the receive element off the active list.
        */
        tp = recvp->rc_next;
        if (openp->recv_act == recvp)
                openp->recv_act = tp;
        else
                recvp->rc_last->rc_next = recvp->rc_next;
        if( recvp->rc_next != NULL )
                recvp->rc_next->rc_last = recvp->rc_last;
        else if( openp->recv_act != NULL )
                openp->recv_act->rc_last = recvp->rc_last;
        recvp->rc_state = RC_FREE;
        recvp->rc_next = NULL;
        recvp->rc_last = recvp;

        /*
        ** Put it on the free list
        */
        if (ca->recv_free == NULL)
                ca->recv_free = recvp;
        else {
                recvp->rc_last = ca->recv_free->rc_last;
                recvp->rc_last->rc_next = recvp;
                ca->recv_free->rc_last = recvp;
        }

        if (openp->op_rcv_event != EVENT_NULL)
                e_wakeup(&openp->op_rcv_event);
        ENABLE_INTERRUPTS(spl);
        return;
} /* free_recv_element() */


/*****************************************************************************
**
** NAME:                clean_sc_queues
**
** FUNCTION:    cleans out transmit elements for a specified subchannel
**
** EXECUTION
** ENVIRONMENT: process and interrupt threads
**
** NOTES:
**
** Input:               pointer to dds structure
**                              pointer to open_structure
**                              subchannel id
**       NOTE: if the subchannel is -1, then all of the subchannels will be cleared
**
** Returns:             Void
**
** Called By:   clean_queue, reset_sub
**
** Calls To:    DISABLE_INTERRUPTS(), ENABLE_INTERRUPTS(),
**
**
*****************************************************************************/
void
clean_sc_queues(
        struct ca *ca,
        ulong subchannel,
        int linkid)
{
        struct xmit_elem *xmitp;
        struct xmit_elem *next;
        int spl;

        /*
        ** scan through the transmit queue for any transmits in progress
        ** and release the transmit elements associated with this subchannel
        ** or all if subchannel == -1.
        */
        DISABLE_INTERRUPTS(spl);
        for (xmitp=ca->xmit_act; xmitp!=NULL;) {
                next = xmitp->xm_next;

                if ((subchannel == -1) ||
                    ((xmitp->xm_scid == subchannel) &&
                     ((linkid == -1) || (xmitp->xm_linkid == linkid)))) {
                  free_xmit_element(ca,xmitp);
                }
                xmitp = next;
        }
        ENABLE_INTERRUPTS(spl);
        return;
} /* clean_sc_queues() */


/*****************************************************************************
**
** NAME:                async_status
**
** FUNCTION:    report status to the specified open structure
**
** EXECUTION
** ENVIRONMENT: interrupt thread
**
** NOTES:
**
** Input:               pointer to channel adapter structure
**                              pointer to open structure
**                              status code
**                              status
**                              subchannel number
**
** Returns:             0 ------ Success
**                              ENOMEM - Could not allocate a status queue element
**
** Called By:
**
** Calls To:    cat_report_status
**
*****************************************************************************/
int
async_status(
        struct ca *ca,
        open_t *openp,
        int code,
        int status,
        int netid,
        int linkid,
        int extra_code)
{
        cio_stat_blk_t kstat_blk;
        stat_elem_t *statp;
        stat_elem_t *tp;
        int spl;

   /*     ASSERT(openp->op_flags & OP_OPENED);    */
        /* It is possible for user to be stopping a subchannel and
        ** the open flag not to be set so do not assert
        **   d50453 */
        if( !(openp->op_flags & OP_OPENED) ) return;  /*d50453*/


        if (openp->op_mode & DKERNEL) {
                bzero(&kstat_blk, sizeof(cio_stat_blk_t));
                kstat_blk.code          = (ulong)code;
                kstat_blk.option[0]     = (ulong)status;
                kstat_blk.option[1]     = (ulong)netid;
                kstat_blk.option[2]     = (ulong)linkid;
                kstat_blk.option[3]     = (ulong)extra_code;
                (*(openp->op_stat_fn)) (openp->op_open_id, &kstat_blk);
                return 0;
        }

        DISABLE_INTERRUPTS(spl);

        /*
        ** Get a free status element.  The empty
        ** queue condition won't happen if the
        ** user has switched to SYNC_MODE mode via
        ** the ioctl() call.
        */
        if ((statp = openp->stat_free) == NULL) {
                ca->stats.ds.sta_que_overflow++;
                openp->op_flags |= LOST_STATUS;
                ENABLE_INTERRUPTS(spl);   /* d52196 */
                return ENOMEM;
        }

        if (statp->stat_next != NULL) {
                statp->stat_next->stat_last = statp->stat_last;
        }
        openp->stat_free = statp->stat_next;
        bzero(statp, sizeof(struct stat_elem));
        statp->stat_last = statp;

        statp->stat_blk.code            = (ulong)code;
        statp->stat_blk.option[0]       = (ulong)status;
        statp->stat_blk.option[1]       = (ulong)netid;
        statp->stat_blk.option[2]       = (ulong)linkid;
        statp->stat_blk.option[3]       = (ulong)extra_code;

        /*
        ** Add the status element to the active list
        */
        if (openp->stat_act == NULL) {
                openp->stat_act = statp;
        } else {
                statp->stat_last = openp->stat_act->stat_last;
                openp->stat_act->stat_last->stat_next = statp;
                openp->stat_act->stat_last = statp;
        }

        /*
        ** If we used the last status element and we
        ** are in SYNC_MODE mode (can't lose status), pause
        ** the adapter---don't accept any more notifications
        ** until the application reads a status element.
        */
        if (openp->stat_free == NULL
        && openp->op_flags & OP_SYNC_MODE) {
                ca->flags |= CAT_PAUSED;
        }

        /*
        ** notify any waiting user process there is status available
        */
        if (openp->op_select & POLLPRI) {
                openp->op_select &= ~POLLPRI;
                selnotify((int)ca->dev, openp->op_chan, POLLPRI);
        }

        ENABLE_INTERRUPTS(spl);

        return 0;
} /* async_status() */


/*****************************************************************************
**
** NAME:                reserve_pca_xmit
**
** FUNCTION:    reserves a specified amount of pca transmit buffers
**
** EXECUTION
** ENVIRONMENT: process and interrupt
**
** NOTES:
**
** Input:               pointer to channel adapter structure
**                              pointer to transmit element
**                              number of pca buffers to allocate
**
** Returns:             0 -- Success, all buffers allocated
**                              -1 - one or more buffers could not be allocated
**
** Called By:   transmit_mgr
**
** Calls To:    cat_get_sfb
**
*****************************************************************************/
int
reserve_pca_xmit(
        struct ca       *ca,
        xmit_elem_t     *xmitp,
        int                     index,                  /* starting list index */
        int                     num_requested)  /* number of buffers requested */
{
        int i, j;

        /*
        ** Make sure request is valid
        */
        if (num_requested > ca->caddp.config_params.xmitno)
                return -1;

        for (i = 0; i < num_requested; i++) {
           for(j=0; j<150; j++) {
             if (cat_get_sfb(ca, &(xmitp->xm_pca_lst[index].buffer)) == 0)
                break;
           }
           if(j==150) {
              for(index=0; index < i; index++)
                cat_ret_buf(ca, xmitp->xm_pca_lst[index].buffer, SFB_TYPE);
              return(-1);
           }
           ++index;
        }

        return 0;
} /* reserve_pca_xmit() */


/*****************************************************************************
**
** NAME:                cat_wait_ack
**
** FUNCTION:    wait until the adapter acknowledges the command
**
** EXECUTION
** ENVIRONMENT: process thread only
**
** NOTES:
**
** Input:               pointer to channel adapter structure
**                              correlator ID
**                              timeout value in ticks
**
** Returns:             0 ----- success, command acknowledged
**                              EINTR - command interrupted by a signal
**
** Called From:
**
** Calls To:    i_disable, i_enable, e_sleep
**
*****************************************************************************/
int
cat_wait_ack(
        struct ca *ca,
        cmd_t *cmdp,
        int tmout)
{
        int spl;
        void cmdack_timeout();

        /*
        ** start a timeout
        */
        cmdp->timer.t_func = cmdack_timeout;
        cmdp->timer.t_arg = (char *)cmdp;
        cmdp->timer.t_val = tmout * HZ;
        push_timeout(ca, &cmdp->timer);

        DISABLE_INTERRUPTS(spl);
        while (ca->ca_cmd.command != PSCAERR && ca->ca_cmd.command != PSCAACK) {
                ca->ca_cmd.cmd_event= EVENT_NULL;
                if (e_sleep(&ca->ca_cmd.cmd_event, EVENT_SIGRET) == EVENT_SIG) {
                        ENABLE_INTERRUPTS(spl);
                        pop_timeout(ca, &cmdp->timer);
                        return EINTR;
                }
                /*
                ** Check to see if a timeout occurred
                */
                if( ca->ca_cmd.command == 0xff && ca->ca_cmd.retcode == 0xff ) {
                        ENABLE_INTERRUPTS(spl);
                        return ETIMEDOUT;
                }
        }
        ENABLE_INTERRUPTS(spl);
        pop_timeout(ca, &cmdp->timer);
        return 0;
} /* cmd_wait_ack() */


/*****************************************************************************
**
** NAME:                notify_all_opens
**
** FUNCTION:    notify all open subchannels of an asynchronous event
**
** EXECUTION
** ENVIRONMENT: process and interrupt threads
**
** NOTES:
**
** Input:               pointer to the adapter structure
**                              status to return
**
** Returns:             Void
**
** Called By:   clean_queue(), cat_shutdown()
**
** Calls To:    async_status()
**
*****************************************************************************/
void
notify_all_opens(struct ca *ca, int status)
{
        int i;

        for (i=0; i<CAT_MAX_OPENS; i++) {
                if (ca->open_lst[i].op_flags&OP_OPENED) {
                        async_status(ca, &ca->open_lst[i], CIO_ASYNC_STATUS, status, 0, 0, 0);
                }
        }

        return;
} /* notify_all_opens() */


/*****************************************************************************
**
** NAME:                cat_read_buf
**
** FUNCTION:    Read a data buffer from the card
**
** EXECUTION
** ENVIRONMENT:
**
** NOTES:
**
** Input:               pointer to channel adapter structure
**                              adapter shared memory offset
**                              buffer address
**                              transfer length
**
** Returns:             0 --- successful completion
**                              EIO - PIO failure
**
** Called By:
**
** Calls To:    CAT_READ, CAT_MEM_ATT, BUSMEM_DET
**
*****************************************************************************/
int
cat_read_buf(
        struct ca *ca,
        ulong saddr,
        uchar *buf,
        int len)
{
        int rc;
        int spl;
        ulong bus_addr;

        DISABLE_INTERRUPTS(spl);
        bus_addr = CAT_MEM_ATT;
        /* CAT_READ sets ca->piorc */
        CAT_READ(bus_addr, saddr, buf, len);
        BUSMEM_DET(bus_addr);           /* release access to MCI bus */
        ENABLE_INTERRUPTS(spl);
        if (ca->piorc) {
                cat_shutdown( ca );
                ca->flags |= CATDEAD;
        }
        return( ca->piorc );
} /* cat_read_buf() */


/*****************************************************************************
 *
 * NAME:        push_timeout
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:
 *
 ****************************************************************************/
void
push_timeout(
        struct ca       *ca,
        timeout_t               *tp)
{
        timeout_t       *cp,
                                *cpp = NULL;
        int                     spl, timeval = 0;

        DISABLE_INTERRUPTS( spl );
        tp->t_seq = ca->timer_seqno++;
        if( (cp = ca->callout) != NULL ) {
                tp->t_val += lbolt;
                while( tp->t_val >= cp->t_val ) {
                        cpp = cp;
                        if( (cp = cp->t_next) == NULL )
                                break;
                }
                if( cpp == NULL ) {
                        w_stop(&ca->watch);
                        ca->callout = tp;
                        tp->t_next = cp;
                        if( ca->callout->t_val < (lbolt+HZ) ) {
                                ca->flags |= CATWDTACT;
                                i_sched(&ca->ofl);
                        } else {
                                ca->watch.restart = (ca->callout->t_val - lbolt) / HZ;
                                if( (ca->flags & CATWDTACT) == 0 )
                                        w_start(&ca->watch);
                        }
                } else {
                        cpp->t_next = tp;
                        tp->t_next = cp;
                }
        } else {
                ca->callout = tp;
                tp->t_next = NULL;
                ca->watch.restart = tp->t_val / HZ;
                tp->t_val += lbolt;
                w_start(&ca->watch);
        }
        ENABLE_INTERRUPTS( spl );
} /* push_timeout() */


/*****************************************************************************
 *
 * NAME:        pop_timeout
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:
 *
 ****************************************************************************/
void
pop_timeout(
        struct ca       *ca,
        timeout_t               *tp)
{
        timeout_t       *cp,
                                *cpp = NULL;
        int                     spl;

        DISABLE_INTERRUPTS( spl );
        if( (cp = ca->callout) != NULL ) {
                while( cp->t_seq != tp->t_seq ) {
                        cpp = cp;
                        if( (cp = cp->t_next) == NULL )
                                break;
                }
                if( cp == NULL ) {
                        ENABLE_INTERRUPTS( spl );
                        return;
                }
                if( cpp == NULL ) {
                        w_stop(&ca->watch);
                        if( (ca->callout = tp->t_next) != NULL ) {
                                if( ca->callout->t_val < (lbolt+HZ) ) {
                                        ca->flags |= CATWDTACT;
                                        i_sched(&ca->ofl);
                                } else {
                                        ca->watch.restart = (ca->callout->t_val - lbolt) / HZ;
                                        if( (ca->flags & CATWDTACT) == 0 )
                                                w_start(&ca->watch);
                                }
                        }
                }
                else
                        cpp->t_next = tp->t_next;
                tp->t_func = NULL;
        }
        ENABLE_INTERRUPTS( spl );
} /* pop_timeout() */


/*****************************************************************************
 *
 * NAME:
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:
 *
 ****************************************************************************/
/*
** A PSCA command acknowledgement did
** not occur in the anticipated time.
** Wakeup the process waiting on the
** acknowledgement.
*/
void
cmdack_timeout(
        struct ca       *ca,
        cmd_t           *cmdp)
{
        cmdp->origcmd = cmdp->command;
        cmdp->command = -1;
        cmdp->retcode = -1;
        if( cmdp->cmd_event != EVENT_NULL )
                e_wakeup(&cmdp->cmd_event);
}

/*****************************************************************************/
/*                                                                           */
/* NAME: cat_logerr                                                          */
/*                                                                           */
/* FUNCTION: Collect information for making of error log entry.              */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs only under the process thread.                     */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: DDS pointer - tells which adapter                               */
/*           Error id    - tells which error is being logged                 */
/*                                                                           */
/*    Output: Error log entry made via errsave.                              */
/*                                                                           */
/*                                                                           */
/* RETURN:  N/A                                                              */
/*                                                                           */
/*****************************************************************************/
void cat_logerr(
   struct ca *ca,            /* DDS pointer - tells which adapter */
   ulong  errid)              /* Error id    - tells which error */
{
   struct  error_log_def   log;
   int                     i;             /* Loop counter */

   /* Initialize the log entry to zero */
   bzero(&log,sizeof(struct error_log_def));

   /* Store the error id in the log entry */
   log.errhead.error_id = errid;

   /* Load the device driver name into the log entry */
   log.errhead.resource_name[0] = '3';
   log.errhead.resource_name[1] = '7';
   log.errhead.resource_name[2] = '0';
   log.errhead.resource_name[3] = 'P';
   log.errhead.resource_name[4] = 'C';
   log.errhead.resource_name[5] = 'A';
   log.errhead.resource_name[6] = ' ';
   log.errhead.resource_name[7] = ' ';

   /* Start filling in the table with data */

   /* Load the adapter command & status registers if applicable */
   log.adap_flags = ca->flags;
   log.total_sc = ca->tot_subchan;

        log.devnum = ca->dev;
        log.pio_retries= ca->retry;
        log.int_not_handled = ca->stats.ds.intr_not_handled;
        log.nombufs = ca->stats.ds.rec_no_mbuf;
        log.rcv_ovflows = ca->stats.ds.rec_que_overflow;


   /* log the error here */
   errsave (&log,sizeof(struct error_log_def));

   return;
} /* cat_logerr() */
/***************************************************************************
 *                                                                         *
 * NAME: drop_link               d51658 added entire function            *
 *                                                                         *
 *                                                                         *
 * FUNCTION: Removes the passed link_t pointer from the sc->links          *
 *           array passes in. It also moves any other remaining            *
 *           pointers to fill in the gap left so that the next empty       *
 *           slot is always after the last valid pointer in the array      *
 *           also frees the specifed link sturcture memory.                *
 *                                                                         *
 * EXECUTION ENVIRONMENT:                                                  *
 *                                                                         *
 *      This routine runs under the process or interrupt thread.           *
 *                                                                         *
 * NOTES:                                                                  *
 *                                                                         *
 *    Input: sc  pointer - tells which subchannel the links are on         *
 *           link        - tells which link is to be removed from links[]  *
 *                                                                         *
 *    Output: none.                                                        *
 *                                                                         *
 *                                                                         *
 * RETURN:  N/A                                                            *
 *                                                                         *
 ***************************************************************************/
void drop_link(
   subchannel_t *sc,
   link_t *link)
{
   int                     i,j;             /* Loop counters */
   int                     found_it=0;      /* mark link found */

   ASSERT(sc);
   ASSERT(link);
   for (i=0; i<MAX_LINKS; i++) {
      if (sc->links[i] == link || found_it) {    /* found the right link */
          /* remove this link by moving all remaining links back one slot */
          j = i + 1;
          if(j < MAX_LINKS && sc->links[j]) {
              sc->links[i]=sc->links[j];
              /* reset the link correlator if its a valid link */
              if (sc->links[i] ) sc->links[i]->lk_correl = i;
          }
          else {
             sc->links[i] = (link_t *)0;
             break;
          }
          found_it=1;
      }
   }

   if (link->lk_appl_id) sc->num_links--;
   KFREE(link);       /* free up the memory */
   return;
} /* drop_link() */
/***************************************************************************
 *                                                                         *
 * NAME: free_sc_links              d51658 added entire function            *
 *                                                                         *
 *                                                                         *
 * FUNCTION: This frees memeory and releases pointers to the               *
 *           subchannel_t struct pointed to by input parm. It also         *
 *           frees link structs and pointers.                              *
 *                                                                         *
 * EXECUTION ENVIRONMENT:                                                  *
 *                                                                         *
 *      This routine runs under the process or interrupt thread.           *
 *                                                                         *
 * NOTES:                                                                  *
 *                                                                         *
 *    Input: sc  pointer - tells which subchannel to free and where to     *
 *                         find the links.                                 *
 *                                                                         *
 *    Output: none.                                                        *
 *                                                                         *
 *                                                                         *
 * RETURN:  N/A                                                            *
 *                                                                         *
 ***************************************************************************/
void free_sc_links(
         struct ca *ca,
         subchannel_t *sc)
{
   int                     i,j;             /* Loop counters */
   uchar                   sub;
   int                     lastsc;
   open_t                  *openp;
   link_t                  *link;
   int			   spl;

   /* make sure sc is not null */
   if (sc == NULL) return;

   sub = sc->sc_id;    /* set the subchannel for this sub struct */

   if (sc->specmode & CAT_CLAW_MOD) {
/* 	DISABLE_INTERRUPTS(spl); */
		 LOCK(ca->lcklvl);
         /*
         ** Free link and SC memory for each subchannel in the group.
         **                     CLAW MODE
         */
         for (j=0; j<MAX_LINKS; j++) {
                 if (sc->links[j] != NULL) {
                         KFREE(sc->links[j]);
                 }
         }
         /* free the array of pointers to link structs  */
         if (sc->links) KFREE(sc->links);
         ca->sc[sub]= NULL;      /* rm read sc ptr */
         ca->sc[sub+1]= NULL;    /* rm write sc ptr */
         /*
         **  In claw mode, only one sc struct is aquired
         **  and both read and write subchannel pointers
         **  point to this one sc struct.
         */
            KFREE(sc);  /* free sc struct for this claw pair */
		UNLOCK(ca->lcklvl);
/*	ENABLE_INTERRUPTS(spl); */
    }
    else {
         /*
         ** Free link and SC memory for each subchannel in the group.
         ** and the op_default variable in open struct for NON CLAW MODE
         */
/*	 DISABLE_INTERRUPTS(spl);	 */
		LOCK(ca->lcklvl);
            if( (link = sc->link[0]) && (openp = link->lk_open) ) {
                  openp->op_default = NULL;
            }
            lastsc = sub + sc->sc_subset;
            for (i=sub; i < lastsc; i++) {
                    if ( (sc = ca->sc[i]) ) {
                               /* free non claw sc's one link */
                            if( sc->link[0] ) KFREE(sc->link[0]);
                               /* free non claw ptr array to its one link */
                            if (sc->links) KFREE(sc->links);
                            KFREE(sc);              /* free the sc struct */
                            ca->sc[i] = NULL;       /* remove ptr to sc struct */
                    }
            }
			UNLOCK(ca->lcklvl);
/*	ENABLE_INTERRUPTS(spl); */
    }
    return;
} /* free_sc_links() */

/***************************************************************************
 *                                                                         *
 * NAME: free_link_recvs         d50453    added entire function         *
 *                                                                         *
 *                                                                         *
 * FUNCTION: Removes and frees all the receive elements for the            *
 *           specified open and link.                                      *
 *           This function is used to clean up                             *
 *           after a memory over error occurs.                             *
 *                                                                         *
 * EXECUTION ENVIRONMENT:                                                  *
 *                                                                         *
 *      This routine runs under the process or interrupt thread.           *
 *                                                                         *
 * NOTES:                                                                  *
 *                                                                         *
 *    Input: ca  pointer - tells which adapter the open is connected with  *
 *           openp       - tells which open to free                        *
 *           lk_id       - specifies which links receives to free          *
 *                                                                         *
 *    Output: none.                                                        *
 *                                                                         *
 *                                                                         *
 * RETURN:  N/A                                                            *
 *                                                                         *
 ***************************************************************************/
void free_link_recvs(
        struct ca *ca,
        open_t *openp,
        uchar  lk_id)
{
      recv_elem_t *recvp;
	 recv_elem_t *next_recvp;

	recvp = openp->recv_act; /*d50453*/
	while (recvp)  {       /*d50453*/
		next_recvp= recvp->rc_next;
/* only remove the not completed receive elements */
/* that are associated with a receive.   */

          if ( (recvp->rc_state != RC_COMPLETE) &&
               (recvp->rc_linkid==lk_id)){
              cat_gen_trace("COMP",0,0,0);
              free_recv_element(ca,recvp);
          }

		recvp = next_recvp;         /*d50453*/
	}
return;

} /* free_link_recvs() */


/*
 * NAME:     cat_gen_trace
 *
 * FUNCTION: enter trace data in the trace table and call AIX trace
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:    Do not call directly -- use macros defined in cioddhi.h so that
 *           tracing can be converted to a zero-run-time-cost option by
 *           simply re-defining the macros.
 *
 * RETURNS:  nothing
 *
 */
void cat_gen_trace (str1, arg2, arg3, arg4)
register char   str1[];  /* string of 0 to 4 characters (best to use 4) */
ulong  arg2;    /* optional data word                          */
ulong  arg3;    /* optional data word                          */
ulong  arg4;    /* optional data word                          */
{
	register char  *s;
	register int    ndx;
	ulong  trcdata[1];

	/* first argument is supposed to be a character string */
	s = (char *)trcdata;

	for (ndx=0; ndx<4; ndx++,s++)
		*s = str1[ndx];

	/* for AIX trace, make the call */

DDHKWD5(HKWD_DD_CATDD, PSCA_GEN,0,0,trcdata[0],arg2,arg3,arg4);



	return;
} 

/*
 * NAME:     event_timeout
 *
 * FUNCTION: send wakeup signal to process blocking on a given event
 *           used to prevent deadlock on resources
 *
 * EXECUTION ENVIRONMENT:
 * 			Executed from the offlevel
 *
 * RETURNS:  nothing
 *
 */


void
event_timeout(
        struct ca       *ca,
        int           *eventp)
{
        if( eventp != EVENT_NULL ) {
                e_wakeup(eventp);
        }
}

