static char sccsid[] = "@(#)60  1.46  src/bos/kernext/cat/cat_xmit.c, sysxcat, bos41J, 9522A_all 5/26/95 14:01:29";
/*
/*
 * COMPONENT_NAME: (SYSXCAT) - Channel Attach device handler
 *
 * FUNCTIONS:   catwrite, write_block, xmit_alloc, free_xmit_element,
 *              send_elem, cat_write_buf, write_get_user, cat_fastwrt
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

#define FNUM 11
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
#include <sys/except.h>
#include <sys/dma.h>

#include "catdd.h"
extern void event_timeout();

/**************************************************************************
**
** NAME:                catwrite
**
** FUNCTION:    entry point from kernel for writes
**
** EXECUTION
** ENVIRONMENT: process thread
**
** Input:               major and minor number
**                              pointer to uio structure
**                              channel number
**                              optional pointer to write extension structure
**
** Returns:     0 or errno
**              ENXIO           Invalid device or channel specified.
**              EBADF           The specified device is not open.
**              EACCES          The specified device is not open for writing.
**              ENOCONNECT      There have been no subchannels started for
**                              this device.
**              EAGAIN          The resources required to perform this request
**                              are unavailable and the requestor does not
**                              wish to wait until the resources are available.
**              EINTR           This call was interrupted by a signal.
**              EINVAL          The length argument is too large.
**              ENOMEM          Enough memory could not be allocated to service
**                              this request.
**              EFAULT          The supplied mbuf chain was NULL.
**
** Called By:   the kernel's write system call
**
** Calls To:    DISABLE_INTERRUPTS, ENABLE_INTERRUPTS, COPYIN, COPYOUT,
**              e_sleep, xmit_user_dma write_get_user, m_freem
**
*****************************************************************************/
/*  We must put the reserved dma elements on the
**  active q so that they can be removed by
**  the call to free_xmit_element.
*/
#define FREE_DMA_LIST(ca, dmap) \
{\
        dma_req_t *dmap_last; \
        if (ca->dma_free == NULL) \
                ca->dma_free = dmap; \
        else { \
                dmap_last = dmap->dm_last; \
                dmap->dm_last = ca->dma_free->dm_last; \
                dmap->dm_last->dm_next = dmap; \
                ca->dma_free->dm_last = dmap_last; \
        }\
}
int
catwrite(
        dev_t devno,
        struct uio *uiop,
        chan_t chan,
        cat_write_ext_t *extptr)
{
        struct ca *ca;
        open_t *openp;
        xmit_elem_t *xmitp;
        dma_req_t *dmap;
        dma_req_t *tmp_dmap;
        struct mbuf *mbufp;
        struct mbuf *tp;
        cat_write_ext_t wr_ext;
        int rc = 0;
        int temp;
        int bcnt;
        int npca;
        int ndma;
        int resid;
        int xfer_len;
        int xfer_off;
        int pca_off;
        int num = 0;
        int linkid;
        int mode;
        int spl;
        caddr_t xfer_addr;
        caddr_t pca_addr;
        subchannel_t *sc;
        int i;
	   int even_word;  /* ix30181 Dont allow DMA if address is not on word boundary*/

        DDHKWD4(HKWD_DD_CATDD, DD_ENTRY_WRITE, 0, devno, chan,
                uiop->uio_iov->iov_base, uiop->uio_resid);

        /*
        ** get the channel adapter structure
        */
        if ((ca = catget(minor(devno))) == NULL)    /* Validate device */
                rc = ENXIO;
        else if( ca->flags & CATDEAD )
                rc = ESHUTDOWN;
        else if (( chan < 0)  || ( chan > CAT_MAX_OPENS))
                rc =  ENXIO;
        else if (((openp = &ca->open_lst[chan])->op_flags & OP_OPENED) == 0)
                rc =  EBADF;
        else if ((openp->op_mode&DWRITE) == 0)
                rc = EACCES;

        if (rc) {
                DDHKWD1(HKWD_DD_CATDD, DD_EXIT_WRITE, rc, devno);
                return rc;
        }

        /* if it is a kernel process uio_resid is 0 */
        if (((openp->op_mode & DKERNEL) == 0) &&  (uiop->uio_resid == 0)) {
                DDHKWD1(HKWD_DD_CATDD, DD_EXIT_WRITE, 0, devno);
                return 0;
        }

        /*
        ** handle the extension parameter
        */
        if (extptr == NULL) {
                /*
                ** build our own with defaults
                */
                wr_ext.cio_ext.flag = 0;                /* don't acknowledge, do free mbuf */
                wr_ext.cio_ext.write_id = 0;    /* doesn't matter */
                wr_ext.cio_ext.netid = -1;              /* default netid for this write */
                wr_ext.attn_int = 0;                    /* don't send attention interrupt */
                wr_ext.use_ccw = 0;                             /* Don't do ccw pass-through */
                wr_ext.ccw = 0;                                 /* Don't do ccw pass-through */

                if (openp->op_default != NULL)
                        wr_ext.cio_ext.netid = openp->op_default->sc_id;
        } else {
                /*
                ** get the caller-provided extension
                */
                rc = COPYIN(openp->op_mode , extptr, &wr_ext, sizeof(wr_ext));
                if (rc) {
                        DDHKWD1(HKWD_DD_CATDD, DD_EXIT_WRITE, EFAULT, devno);
                        return EFAULT;
                }
                if (!(openp->op_mode & DKERNEL)) {
                        /* always free user mode mbuf */
                        wr_ext.cio_ext.flag &= ~CIO_NOFREE_MBUF;
                }
        }

        /*
        ** if there are no subchannels opened
        */
        if ((wr_ext.cio_ext.netid == -1) ||
            ((sc= ca->sc[wr_ext.cio_ext.netid]) == NULL)) {
                wr_ext.cio_ext.status = (ulong)CIO_NOT_STARTED;
                if (extptr != NULL) {
                        (void)COPYOUT(openp->op_mode, &wr_ext,
                                extptr, sizeof(wr_ext));
                }
                DDHKWD1(HKWD_DD_CATDD, DD_EXIT_WRITE, ENOCONNECT, devno);
                return ENOCONNECT;
        }

/* ix29363  Validate the parameters passed in the extension for CLAW mode */
	  if (sc->specmode & CAT_CLAW_MOD) {
		if ( wr_ext.attn_int ) {
			wr_ext.cio_ext.status = (ulong) (CIO_EXCEPT_USER | 2);
			if (extptr != NULL){
			   (void)COPYOUT(openp->op_mode, &wr_ext, extptr, sizeof(wr_ext));
			}
			DDHKWD1(HKWD_DD_CATDD, DD_EXIT_WRITE, EINVAL, devno);
			return EINVAL;

		}
	  }

        /*
        ** Validate the subchannel for this open
        ** also the link if it is claw mode
        */

        if ((mode = sc->specmode) & CAT_CLAW_MOD) {
                for (i=0; i<MAX_LINKS; i++) {
                    if (sc->links[i]->lk_appl_id == wr_ext.cio_ext.write_id) {
                            linkid = sc->links[i]->lk_actual_id;
                            break;
                    }
                }

			 { /* additional traces */
			  unsigned int *trcptr;
			  trcptr = (unsigned int *)(sc->link[linkid]);
			  if ( trcptr != NULL )
				DDHKWD2(HKWD_DD_CATDD, PSCA_LINK,0,devno,*(trcptr+1));
			 }

                if (i == MAX_LINKS
                || (sc->link[linkid] == NULL)
                || (sc->link[linkid]->lk_state != LK_FIRM)) {
                        wr_ext.cio_ext.status = (ulong)(CIO_EXCEPT_USER | 1);
                        if (extptr != NULL) {
                                (void)COPYOUT(openp->op_mode, &wr_ext,
                                        extptr, sizeof(wr_ext));
                        }
                        DDHKWD1(HKWD_DD_CATDD, DD_EXIT_WRITE, EINVAL, devno);
                        return EINVAL;
                }
        } else {
                linkid = 0;
                { /* additional traces */
                 unsigned int *trcptr;
                 trcptr = (unsigned int *)(sc->link[linkid]);
                 if ( trcptr != NULL )
                    DDHKWD2(HKWD_DD_CATDD, PSCA_LINK,0,devno,*(trcptr+1));
                }

        }

/* Trace the subchannel control block  */
          { /* additional traces */
           unsigned int *trcptr;
           trcptr = (unsigned int *)sc;
           if ( trcptr != NULL )
			 DDHKWD4(HKWD_DD_CATDD, PSCA_SUBC,0,devno,*trcptr,*(trcptr+1),sc->num_links);
          }

        if (sc->link[linkid]->lk_open != openp
                || sc->sc_state != SC_OPEN) {
                CATDEBUG(("cat_xmit: lk_open != openp\n"));
                DDHKWD1(HKWD_DD_CATDD, DD_EXIT_WRITE, EINVAL, devno);
                return EINVAL;
        }

        /*
        ** First, get the data into mbufs
        */
        if (openp->op_mode&DKERNEL) {
                /*
                ** this is a kernel process,
                ** the uio pointer is an mbuf pointer
                */
                mbufp = (struct mbuf *)(uiop->uio_iov->iov_base);
/*d50453*/      ca->mbuf_num+=num_mbufs(mbufp);
        } else {
                /*
                ** the caller is a user process
                ** get the data into an mbuf chain
                */
                mbufp = NULL;
                while ((rc = write_get_user(ca, uiop, &mbufp)) == ENOMEM) {
                        /* Couldn't get an mbuf or cluster... */
                        if (rc = write_block(ca, openp, uiop, &ca->mbuf_event)) {
                                if (mbufp) {
                                        DISABLE_INTERRUPTS(spl);
    /*d50453*/                          ca->mbuf_num-=num_mbufs(mbufp);
                                        m_freem(mbufp);
                                        mbufp = NULL;
                                        if( ca->mbuf_event != EVENT_NULL )
                                                e_wakeup(&ca->mbuf_event);
                                        ENABLE_INTERRUPTS(spl);
                                }
                                return rc;
                        }
                }
                if (rc) {
                        /*
                        ** uiomove() failed when called from
                        ** write_get_user(), so return
                        */
                        return rc;
                }
        }

        /*
        ** determine the total byte count and number of
        ** DMA elements required.
        */
        pca_off = 0;
        tp = mbufp;
        bcnt = 0;
        ndma = 0;
        while (tp) {
                if (tp->m_len <= 0) {
                        tp = tp->m_next;
                        break;
                }

			 /* ix30181 */
			 even_word = (unsigned int)(MTOD(tp,char *))% 4; /* if evenly divisible by 4 then DMA*/ 

                bcnt += tp->m_len;
                resid = tp->m_len;
                xfer_len = 0;
                xfer_off = 0;

                while (resid > 0) {
                        /*
                        ** Calculate transfer parameters
                        */
                        xfer_len = (resid >
                                (ca->caddp.config_params.xmitsz-pca_off))
                                ? (ca->caddp.config_params.xmitsz-pca_off)
                                : resid;
                        xfer_off += xfer_len;
                        pca_off += xfer_len;
                        resid -= xfer_len;
                        /*
                        ** Now, check to see whether we need to use dma or pio.
                        ** Some mbufs may be smaller than the pio threshold, so
                        ** check the chain of mbufs to see how many mbufs are
                        ** large enough to need DMA.
                        */
					/* ix30181 if not on a word boundary PIO the data 
 					 * to the adapter
					 */
				/* ix32459 Offset into pca buffer has to be double word aligned */
                        if (xfer_len <= PIO_THRESHOLD || even_word  || (pca_off - xfer_len) % 8 || xfer_len % 4 ) {  
                                if (resid == 0) {
                                        while ((tp = tp->m_next) !=
                                                NULL && tp->m_len <= 0)
                                                ;
                                }
                                /*
                                ** If this was the last mbuf, cause the PSCA
                                ** send command to be issued.
                                */
                                if (tp == NULL) {
                                        /* Need one DMA element for pseudo-DMA */
                                        ndma++;
                                }
                        } else {
                                ndma++;
                                if (resid == 0) {
                                        while ((tp = tp->m_next) != NULL
                                                && tp->m_len <= 0)
                                                ;
                                }
                        }

                        /*
                        ** If the PSCA buffer is full, index a new
                        ** PSCA buffer.
                        */
                        if (pca_off >= ca->caddp.config_params.xmitsz) {
                                pca_off = 0;
                        }
                }
        }

        /*
        ** calculate the necessary number of PSCA transmit buffers
        */
        npca = (bcnt+ca->caddp.config_params.xmitsz-1)
                / ca->caddp.config_params.xmitsz;
        /*
        ** Make sure the request length is valid
        */
        if (bcnt > MAX_PCA_XMIT_LEN || npca > ca->caddp.config_params.xmitno) {
                if ((openp->op_mode & DKERNEL) == 0 && mbufp) {
                        DISABLE_INTERRUPTS(spl);
     /*d50453     */    ca->mbuf_num-=num_mbufs(mbufp);
                        m_freem(mbufp);
                        mbufp = NULL;
                        if (ca->mbuf_event != EVENT_NULL)
                                e_wakeup(&ca->mbuf_event);
                        ENABLE_INTERRUPTS(spl);
                }
                DDHKWD1(HKWD_DD_CATDD, DD_EXIT_WRITE, EINVAL, devno);
                CATDEBUG(("cat_xmit: xfer too big!\n"));
                return EINVAL;
        }
/* d52514
   Interrupt disablement and enablement in the nested stated
   caused the return to the caller to assert due to the
   original interrupt level not being restored properly.
   Changed to disable and enable withing the conditional
   and disable after the xmit buffer has been allocated.
 */
        if ((xmitp = xmit_alloc(ca)) == NULL) {
                DISABLE_INTERRUPTS(spl);
                if ((openp->op_mode&DKERNEL) == 0 && mbufp) {
  /*d50453*/       ca->mbuf_num-=num_mbufs(mbufp);
                        m_freem(mbufp);
                        mbufp = NULL;
                        if (ca->mbuf_event != EVENT_NULL)
                                e_wakeup(&ca->mbuf_event);
                }
                wr_ext.cio_ext.status = (ulong)CIO_TX_FULL;
                if (extptr != NULL)
                        (void)COPYOUT(openp->op_mode, &wr_ext, extptr,
                                sizeof(wr_ext));
                DDHKWD1(HKWD_DD_CATDD, DD_EXIT_WRITE, ENOMEM, devno);
                ENABLE_INTERRUPTS(spl);
                return ENOMEM;
        }




        /*
        ** Tie the mbuf chain to the transmit pointer.
        */
        xmitp->xm_mbuf = mbufp;
	   xmitp->xm_open = openp; /* need to set XMITP open in */
						  /* case of error so mbuf's are */
						  /* handled properly		   */

        /*
        ** allocate the necessary number of PSCA transmit buffers
        ** leave irpts enabled so other processes can free up the
        ** send buffers I am waiting on.
        */
        while (reserve_pca_xmit(ca, xmitp, 0, npca)) {
                if (rc = write_block(ca, openp, uiop, &ca->pcabuf_event)) {
                        free_xmit_element(ca, xmitp);
                        wr_ext.cio_ext.status = (ulong)CIO_TX_FULL;
                        if (extptr != NULL)
                                (void)COPYOUT(openp->op_mode, &wr_ext, extptr,
                                         sizeof(wr_ext));
                        DDHKWD1(HKWD_DD_CATDD, DD_EXIT_WRITE, rc, devno);
                /*      ENABLE_INTERRUPTS(spl); d50453 */
                        return rc;
                }
        }
        DISABLE_INTERRUPTS(spl);

        /*
        ** Reserve the PSCA control buffer
        */
        while (cat_get_cfb(ca, &xmitp->xm_pca_ctrlbuf)) {
                if (rc = write_block(ca, openp, uiop, &ca->pcabuf_event)) {
                        free_xmit_element(ca, xmitp);
                        wr_ext.cio_ext.status = (ulong)CIO_TX_FULL;
                        if (extptr != NULL)
                                (void)COPYOUT(openp->op_mode, &wr_ext,
                                        extptr, sizeof(wr_ext));
                        DDHKWD1(HKWD_DD_CATDD, DD_EXIT_WRITE, rc, devno);
                        ENABLE_INTERRUPTS(spl);
                        return rc;
                }
        }

        /*
        ** Now, reserve the DMA elements
        */
        CATDEBUG(("calling dma_alloc() from catwrite()\n"));
        while ((dmap = dma_alloc(ca, xmitp, ndma)) == NULL) {
                if (rc = write_block(ca, openp, uiop, &ca->dmabuf_event)) {
                        free_xmit_element(ca,xmitp);
                        wr_ext.cio_ext.status = (ulong)CIO_TX_FULL;
                        if (extptr != NULL)
                                (void)COPYOUT(openp->op_mode, &wr_ext, extptr,
                                        sizeof(wr_ext));
                        DDHKWD1(HKWD_DD_CATDD, DD_EXIT_WRITE, rc, devno);
                        ENABLE_INTERRUPTS(spl);
                        return rc;
                }
        }

        /*
        ** We have a valid transmit element, the necessary number of DMA
        ** elements and adapter buffers.  Now, we initialize the transmit
        ** element, and begin setting up the DMA requests.
        */
        xmitp->xm_open = openp;
        xmitp->xm_length = bcnt;
        xmitp->xm_num_bufs = npca;
        xmitp->xm_scid = wr_ext.cio_ext.netid;
        xmitp->xm_linkid = linkid;
        xmitp->xm_ack = wr_ext.cio_ext.flag;
        xmitp->xm_ack |= wr_ext.attn_int ? CAT_SEND_ATTN : 0;
        xmitp->xm_writeid = wr_ext.cio_ext.write_id;
        xmitp->xm_cmd.ccw = wr_ext.use_ccw ? wr_ext.ccw : 0;
        xmitp->xm_cmd.correl = wr_ext.cio_ext.write_id;

        /*
        ** Process each mbuf in the chain
        ** the transfer process will update the mbuf pointer
        ** after an mbuf has been processed.
        */
        pca_off = 0;
        while (mbufp) {
                resid = mbufp->m_len;
                xfer_len = xfer_off = 0;
                /* ix30181 */
                even_word = (unsigned int)(MTOD(mbufp,char *))% 4; /* if evenly divisible by 4 then DMA*/

                while (resid > 0) {
                        /*
                        ** Determine transfer parameters
                        */
                        xfer_len = (resid >
                                (ca->caddp.config_params.xmitsz-pca_off))
                                ? (ca->caddp.config_params.xmitsz-pca_off)
                                : resid;
                        xfer_addr = MTOD(mbufp,caddr_t) + xfer_off;
                        pca_addr = xmitp->xm_pca_lst[num].buffer + pca_off;
                        xfer_off += xfer_len;
                        pca_off += xfer_len;
                        xmitp->xm_pca_lst[num].length += xfer_len;
                        resid -= xfer_len;
                        /*
                        ** Now, check to see whether we need to use dma or pio.
                        ** Some mbufs may be smaller than the pio threshold, so
                        ** check the chain of mbufs to see how many mbufs are
                        ** large enough to need DMA.
                        */
				    /* ix30181 check for word boundary  */ 
				/* ix32459 Offset into pca buffer has to be double word aligned */
                        if (xfer_len <= PIO_THRESHOLD || even_word || (pca_off - xfer_len) % 8 || xfer_len % 4 ) {
                                /*
                                ** pio stuf here
                                ** cat_write_buf will only fail if a PIO fault
                                ** occurs.
                                */
                                if (cat_write_buf(ca, pca_addr, xfer_addr,
                                         xfer_len)) {
                                        FREE_DMA_LIST(ca, dmap);
                                        free_xmit_element(ca, xmitp);
                                        ENABLE_INTERRUPTS(spl);
                                        return EIO;
                                }
                                if (resid == 0) {
                                        while ((mbufp = mbufp->m_next) !=
                                                NULL && mbufp->m_len <= 0)
                                                ;
                                }
                                /*
                                ** If this was the last mbuf, cause the PSCA
                                ** send command to be issued.
                                */
                                if (mbufp == NULL) {
                                        dmap->dm_open = openp;
                                        dmap->p.xmit_ptr = xmitp;
                                        dmap->dm_scid = xmitp->xm_scid;
                                        dmap->dm_linkid = xmitp->xm_linkid;
                                        dmap->dm_length = 0;
                                        dmap->dm_req_type = DM_PSEUDO_XMIT;
                                        dmap->dm_state = DM_READY;
                                        if (rc = dma_request(ca, dmap)) {
                                                FREE_DMA_LIST(ca, dmap);
                                                free_xmit_element(ca, xmitp);
                                                ENABLE_INTERRUPTS(spl);
                                                return EIO;
                                        }
                                }
                        } else {
                                /*
                                ** dma stuff
                                */
                                dmap->dm_open = openp;
                                dmap->p.xmit_ptr = xmitp;
                                dmap->dm_scid = xmitp->xm_scid;
                                dmap->dm_linkid = xmitp->xm_linkid;
                                dmap->dm_buffer = xfer_addr;
                                dmap->dm_pca_buffer = pca_addr;
                                dmap->dm_length = xfer_len;
                                dmap->dm_xmem = M_XMEMD(mbufp);
                                if (resid == 0) {
                                        while ((mbufp = mbufp->m_next) != NULL
                                                && mbufp->m_len <= 0)
                                                ;
                                }
                                /*
                                ** If this was the last mbuf, cause the PSCA
                                ** send command to be issued.
                                */
                                if (mbufp == NULL) {
                                        dmap->dm_end_of_list = 1;
                                }
                                dmap->dm_req_type = DM_XMIT;
                                dmap->dm_state = DM_READY;
                                dmap->dm_flags = DMA_NOHIDE;
                                tmp_dmap = dmap;
                                if (mbufp) {
                                        dmap = dmap->dm_next;
                                        dmap->dm_last = tmp_dmap->dm_last;
                                        tmp_dmap->dm_last = tmp_dmap;
                                        tmp_dmap->dm_next = NULL;
                                }
                                if (rc = dma_request(ca, tmp_dmap)) {
                                        FREE_DMA_LIST(ca, dmap);
                                        free_xmit_element(ca, xmitp);
                                        ENABLE_INTERRUPTS(spl);
                                        return EIO;
                                }
                        }

                        /*
                        ** If the PSCA buffer is full, index a new
                        ** PSCA buffer.
                        */
                        if (pca_off >= ca->caddp.config_params.xmitsz) {
                                pca_off = 0;
                                num++;
                        }
                }
        }
        ENABLE_INTERRUPTS(spl);
        wr_ext.cio_ext.status = (ulong)CIO_OK;
        if (extptr != NULL) {
                (void)COPYOUT(openp->op_mode, &wr_ext, extptr, sizeof(wr_ext));
        }
        DDHKWD1(HKWD_DD_CATDD, DD_EXIT_WRITE, 0, devno);
        return 0;
} /* catwrite() */


/**************************************************************************
**
** NAME:        write_block
**
** FUNCTION:
**
** EXECUTION ENVIRONMENT:
**
** NOTES:
**
** RETURNS:
**
*****************************************************************************/
int
write_block(
        struct ca *ca,
        open_t *openp,
        struct uio *uiop,
        int *eventp)
{
        int spl;

        /*
        ** action will depend on whether DNDELAY is set
        */
        if (uiop->uio_fmode & DNDELAY) {
                if (openp->op_mode  & DKERNEL) {
                        /*
                        ** set up flags so xmt_fn gets called
                        ** when conditions change
                        */
                        DISABLE_INTERRUPTS(spl);
                        ca->flags |= CATXMITOWED;
                        openp->op_flags |= XMIT_OWED;
                        ENABLE_INTERRUPTS(spl);
                }
                return EAGAIN;
        }
        openp->etimer.t_func = event_timeout;
        openp->etimer.t_arg = eventp;
        openp->etimer.t_val = 4 * HZ;
        push_timeout(ca, &openp->etimer);

        /*
        ** block by sleeping
        */

        if (SLEEP(eventp) != EVENT_SUCC) {

                pop_timeout ( ca, &openp->etimer);
                return EINTR;
        }

        pop_timeout(ca, &openp->etimer );

        return 0;
} /* write_block() */



/**************************************************************************
**
** NAME:        xmit_alloc
**
** FUNCTION:
**
** EXECUTION ENVIRONMENT:
**
** NOTES:
**
** RETURNS:
**
*****************************************************************************/
xmit_elem_t *
xmit_alloc( struct ca *ca )
{
        xmit_elem_t *xmitp;
        int spl;
        int rc;

        DISABLE_INTERRUPTS(spl);
        /*
        ** We do this with interrupts disabled so that
        ** an interrupt handler doesn't have to worry
        ** about the locking protocol when freeing
        ** elements.
        **
        ** update the free list
        */
        if ((xmitp = ca->xmit_free) != NULL) {
                if (xmitp->xm_next != NULL)
                        xmitp->xm_next->xm_last = xmitp->xm_last;
                ca->xmit_free = xmitp->xm_next;
                bzero(xmitp, sizeof(xmit_elem_t));
                xmitp->xm_last = xmitp;

                /*
                ** update the active list
                */
                if (ca->xmit_act == NULL)
                        ca->xmit_act = xmitp;
                else {
                        xmitp->xm_last = ca->xmit_act->xm_last;
                        xmitp->xm_last->xm_next = xmitp;
                        ca->xmit_act->xm_last = xmitp;
                }
        }

        ENABLE_INTERRUPTS(spl);
        return xmitp;
} /* xmit_alloc() */


/**************************************************************************
**
** NAME:        free_xmit_element
**
** FUNCTION:
**
** EXECUTION ENVIRONMENT:
**
** NOTES:
**
** RETURNS:
**
*****************************************************************************/
void
free_xmit_element(
        struct ca *ca,
        xmit_elem_t *xmitp)
{
        xmit_elem_t *active_xmit;   /*d50453*/
        dma_req_t *dmap;
        dma_req_t *tmp_dmap;
        open_t *openp;
        int i;
        int spl;
        int dma_aborted = 0;

        if (xmitp == NULL) {
                return;
        }

        /*
        ** Free or abort all DMA requests associated with this element.
        **
        ** This is done with interrupts disabled so that an
        ** interrupt handler can not change the state of an
        ** element while we are handling it.
        */
        DISABLE_INTERRUPTS(spl);
        /* d50453 make sure this element is really on the
        ** active list
        */
        active_xmit = ca->xmit_act;
        while (active_xmit) {
               if(xmitp==active_xmit) break;
               active_xmit = active_xmit->xm_next;
        }
        if (xmitp != active_xmit) {
           ENABLE_INTERRUPTS(spl);
           return;
        }
/* code added for d50453 ends here */

        dmap = ca->dma_act;
        while (dmap) {
                if (dmap->p.xmit_ptr == xmitp) {
                        if (dmap == ca->dma_act &&
                                (dmap->dm_state == DM_STARTED ||
						  dmap->dm_state == DM_ABORTED)) {
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
        ** unless the caller was a kernel mode process.
        */
        if (xmitp->xm_mbuf != NULL && (xmitp->xm_open->op_mode&DKERNEL) == 0) {
/*d50453     */ ca->mbuf_num-=num_mbufs(xmitp->xm_mbuf);
                m_freem(xmitp->xm_mbuf);
                xmitp->xm_mbuf = NULL;
                if (ca->mbuf_event != EVENT_NULL) {
                        e_wakeup(&ca->mbuf_event);
                }
        }

        /*
        ** Free any PSCA resources associated with this element.
        */
        if (xmitp->xm_pca_ctrlbuf != NULL) {
                cat_ret_buf(ca, xmitp->xm_pca_ctrlbuf, CFB_TYPE);
        }
        for (i=0; i < xmitp->xm_num_bufs; i++) {
                cat_ret_buf(ca, xmitp->xm_pca_lst[i].buffer, SFB_TYPE);
        }

/*d62665 */
/* 
 * The original code only woke up processes sleeping on the event
 * if there was an error condition where the micrcode did not 
 * return the control bufferst to the free buffer FIFO.
 * Any other time a process is sleeping on the event would
 * cause a hang condition
 */
/*d62665*/
/*
 * The proper way to handle this is to wakeup any sleeping 
 * processes on the pcabuf event at this time.
 */
 if ( ca->pcabuf_event != EVENT_NULL) {
	   e_wakeup(&ca->pcabuf_event);
	}


        /*
        ** Now, free the transmit element
        **
        ** We do this with interrupts disabled so that
        ** an interrupt handler doesn't have to worry
        ** about the locking protocol when freeing
        ** elements.
        */
        if (ca->xmit_act == xmitp) {
                ca->xmit_act = xmitp->xm_next;
        } else {
                xmitp->xm_last->xm_next = xmitp->xm_next;
        }
        if (xmitp->xm_next != NULL) {
                xmitp->xm_next->xm_last = xmitp->xm_last;
        } else if (ca->xmit_act != NULL) {
                ca->xmit_act->xm_last = xmitp->xm_last;
        }
        xmitp->xm_last = xmitp;
        xmitp->xm_next = NULL;

        /*
        ** Put it on the free list
        */
        if (ca->xmit_free == NULL) {
                ca->xmit_free = xmitp;
        } else {
                xmitp->xm_last = ca->xmit_free->xm_last;
                xmitp->xm_last->xm_next = xmitp;
                ca->xmit_free->xm_last = xmitp;
        }

        /*
        ** Wakeup anyone waiting on xmit elements
        */
        if (ca->xmitbuf_event != EVENT_NULL) {
                e_wakeup(&ca->xmitbuf_event);
        }
        if (ca->flags & CATXMITOWED) {
                for (i = 0; i < ca->num_opens; i++) {
                        if ((openp = &ca->open_lst[i])->op_flags&OP_OPENED) {
                                if (openp->op_flags&XMIT_OWED) {
                                        (*(openp->op_xmit_fn))(openp->op_open_id);
                                }
                                openp->op_flags &= ~XMIT_OWED;
                        }
                }
                ca->flags &= ~CATXMITOWED;
        }
        ENABLE_INTERRUPTS(spl);

        return;
} /* free_xmit_element() */


/*****************************************************************************
**
** NAME:        send_elem
**
** FUNCTION:    This executes the actual PSCAXLST command to send the data
**              from the channel adapter to the channel control unit.  After
**              the data has been transferred to the adapters shared memory.
**
** EXECUTION
** ENVIRONMENT: process or interrupt thread
**
** Input:       pointer to the channel adapter structure
**              pointer to a transmit element
**              address of the adapter buffer containing the data to send
**
** Returns:     0 ----- no problems occurred
**              EINTR - a signal interrupted the transmission
**
*****************************************************************************/
int
send_elem(
        struct ca *ca,
        xmit_elem_t *xmitp)
{
        int i;
        int rc;
        int spl;
        xbuf_t buflist[MAX_BUF_LIST];
        union {
                cmd_t n_elem;
                struct {
                        unsigned int d1;
                        unsigned int d2;
                        unsigned int d3;
                        unsigned int d4;
                } t_elem;
        } trcun;
#define trcd1   trcun.t_elem.d1
#define trcd2   trcun.t_elem.d2
#define trcd3   trcun.t_elem.d3
#define trcd4   trcun.t_elem.d4

        CATDEBUG(("send_elem()\n"));
        DISABLE_INTERRUPTS(spl);
        bcopy(xmitp->xm_pca_lst,buflist,(MAX_BUF_LIST * sizeof(xbuf_t)));
        for (i=0; i < xmitp->xm_num_bufs && i < MAX_BUF_LIST; i++) {
                letni16(&buflist[i].length);
                letni32(&buflist[i].buffer);
        }
        if (rc = cat_write_buf(ca, xmitp->xm_pca_ctrlbuf, buflist,
                ca->status.cbuflen)) {
                ENABLE_INTERRUPTS(spl);  /* d52196 */
                return rc;
        }
        /*
        ** Fill in the command structure
        */
        xmitp->xm_cmd.command = PSCAXLST;      /* xmit buffers */
        xmitp->xm_cmd.subchan = xmitp->xm_scid;
        if (ca->sc[xmitp->xm_scid]->specmode & CAT_CLAW_MOD) {
                xmitp->xm_cmd.ccw = xmitp->xm_linkid;
                xmitp->xm_cmd.correl = xmitp->xm_linkid;
        }
        xmitp->xm_cmd.data[0] = xmitp->xm_num_bufs;
        xmitp->xm_cmd.data[1] = 0;
        xmitp->xm_cmd.length = xmitp->xm_length;
        xmitp->xm_cmd.buffer = (ulong)xmitp->xm_pca_ctrlbuf;

        /*
        ** Always set ACKNOWLEDGE.  Clean_queue() will check
        ** the xm_ack flag before sending a status block.
        */
        xmitp->xm_cmd.cmdmod = MODACK;
        if (xmitp->xm_ack & CAT_SEND_ATTN)
                xmitp->xm_cmd.cmdmod |= MODATTN;

        if (rc = cat_put_cmd(ca, &xmitp->xm_cmd)) {/* send cmd to adapter */
                ENABLE_INTERRUPTS(spl);
                return rc;
        }

        /*
        ** Zero out these fields so that free_xmit_element()
        ** will not try to release the PSCA buffers.
        */
        xmitp->xm_pca_ctrlbuf = NULL;
        xmitp->xm_num_bufs = 0;
        ENABLE_INTERRUPTS(spl);
        letni16(&xmitp->xm_cmd.length);
        trcun.n_elem = xmitp->xm_cmd;
        DDHKWD5(HKWD_DD_CATDD,PSCA_SEND,0,ca->dev,trcd1,trcd2,trcd3,trcd4);
#undef trcd1
#undef trcd2
#undef trcd3
#undef trcd4
        ca->stats.ds.xmit_sent++;

        return 0;
} /* send_elem() */


/*****************************************************************************
**
** NAME:        cat_write_buf
**
** FUNCTION:    Write a data buffer to the card
**
** EXECUTION ENVIRONMENT: process or interrupt thread
**
** NOTES:
**
** RETURNS:
**
*****************************************************************************/
int
cat_write_buf(
        struct ca *ca,
        ulong saddr,
        uchar *buf,
        int len)
{
        ulong bus;
        extern int pio_rc;
        int spl;

        DISABLE_INTERRUPTS(spl);
        bus = CAT_MEM_ATT;
        CAT_WRITE(bus, saddr, buf, len);        /* sets ca->piorc */
        BUSMEM_DET(bus);                        /* release access to MCI bus */
        ENABLE_INTERRUPTS(spl);
        if (ca->piorc) {
                cat_shutdown( ca );
                ca->flags |= CATDEAD;
        }
        return ca->piorc;
} /* cat_write_buf() */


/*****************************************************************************
**
** NAME:        write_get_user
**
** FUNCTION:    get the user process's data into an mbuf chain
**
** EXECUTION ENVIRONMENT: process thread only
**
** NOTES:
**
** RETURNS:  0 or errno
**
*****************************************************************************/
static int
write_get_user(
        struct ca *dds_ptr,             /* pointer to dds structure */
        struct uio *uiop,               /* pointer to uio structure */
        struct mbuf **mbufph)           /* addr for returning mbuf chain ptr */
{
        struct mbuf *mbufp;             /* current one */
        struct mbuf *mbufpprev;         /* previous one */
        int bytes_to_move;              /* amount to write */
        int rc;

        *mbufph = NULL;
        while ((bytes_to_move = uiop->uio_resid) > 0) {
                /*
                ** get an mbuf
                */
                mbufp = m_get(M_WAIT, MT_DATA);
                if (mbufp == NULL) {
                        cat_logerr(dds_ptr, ERRID_CAT_ERR3);
                        return (ENOMEM);
                }
                ++dds_ptr->mbuf_num;         /*d50453*/
                if (*mbufph == NULL)            /* this is first time */
                        /* caller gets beginning of chain */
                        *mbufph = mbufp;
                else
                        /* link this one to previous one */
                        mbufpprev->m_next = mbufp;
                mbufpprev = mbufp;      /* save for the next time thru loop */
                if( bytes_to_move > MLEN ) {
                        rc = m_clget(mbufp);
                        if( rc == 0 || mbufp->m_len != CLBYTES ) {
                                cat_logerr(dds_ptr, ERRID_CAT_ERR3);
                                return (ENOMEM);
                        }
                        if (bytes_to_move > CLBYTES)
                                /* we'll need another mbuf in chain */
                                bytes_to_move = CLBYTES;
                }
                /*
                ** move the data into the mbuf (uiop->uio_resid will
                ** be decremented)
                */
                if (uiomove(MTOD(mbufp, uchar *), bytes_to_move,
                         UIO_WRITE, uiop)) {
                        return EFAULT;
                }
                /*
                ** now change the mbuf length to reflect the actual data size
                */
                mbufp->m_len = bytes_to_move;
        }
        return 0;
} /* write_get_user() */

/*****************************************************************************/
/*
 * NAME:        cat_fastwrt
 *
 * FUNCTION:    fast write function for kernel
 *
 * EXECUTION ENVIRONMENT:
 *      Can be called from interrupt or process level
 *
 * RETURNS:
 *      0 if successful
 *      errno value on failure
 */
/*****************************************************************************/
int
cat_fastwrt (
        dev_t devno,                            /* major minor number */
        struct mbuf *mbufp,                     /* mbuf list of data */
        cat_write_ext_t *extptr)        /* write extension block */
{
        struct ca *ca;
        cat_write_ext_t wr_ext;
        int spl;
        xmit_elem_t *xmitp;
        dma_req_t *dmap;
        dma_req_t *tmp_dmap;
        struct mbuf *tp;
        int ndma;
        int npca;
        int bcnt;
        int temp;
        int rc;
        int resid;
        int xfer_len;
        int xfer_off;
        int pca_off;
        int num;
        int mode;
        subchannel_t *scp;
        caddr_t xfer_addr;
        caddr_t pca_addr;
	   int even_word;  /* ix30181 */

        DDHKWD4(HKWD_DD_CATDD,DD_ENTRY_WRITE, 0, devno, 0x53, mbufp, 0);
        if ((ca=catget(minor(devno))) == NULL) {
           DDHKWD1(HKWD_DD_CATDD,DD_EXIT_WRITE,ENODEV,0x54);
           return ENODEV;
        }

        if (extptr == NULL) {
           DDHKWD1(HKWD_DD_CATDD,DD_EXIT_WRITE,ENODEV,0x61);
           return(EINVAL);
        }

        bzero(&wr_ext, sizeof(cat_write_ext_t));

        /*
        ** get the caller-provided extension
        */
        bcopy(extptr, &wr_ext, sizeof(wr_ext));

        DISABLE_INTERRUPTS(spl);

        if ((xmitp = xmit_alloc(ca)) == NULL) {
           ENABLE_INTERRUPTS(spl);
           DDHKWD1(HKWD_DD_CATDD,DD_EXIT_WRITE,EAGAIN,0x55);
           return(EAGAIN);
        }

        xmitp->xm_mbuf = mbufp;
        /* need to set XMITP open in */
        /* case of error so mbuf's are */
        /* handled properly            */
	   xmitp->xm_open = scp->link[wr_ext.cio_ext.write_id]->lk_open;


        /*
        ** determine the total byte count and number of
        ** DMA elements required.
        */
        pca_off = 0;
        tp = mbufp;
        bcnt = 0;
        ndma = 0;
        while (tp) {
                if (tp->m_len <= 0) {
                        tp = tp->m_next;
                        break;
                }
                /* ix30181 */
                even_word = (unsigned int)(MTOD(tp,char *))% 4; /* if evenly divisible by 4 then DMA*/

                bcnt += tp->m_len;
                resid = tp->m_len;
                xfer_len = 0;
                xfer_off = 0;

                while (resid > 0) {
                        /*
                        ** Calculate transfer parameters
                        */
                        xfer_len = (resid >
                                (ca->caddp.config_params.xmitsz-pca_off))
                                ? (ca->caddp.config_params.xmitsz-pca_off)
                                : resid;
                        xfer_off += xfer_len;
                        pca_off += xfer_len;
                        resid -= xfer_len;
                        /*
                        ** Now, check to see whether we need to use dma or pio.
                        ** Some mbufs may be smaller than the pio threshold, so
                        ** check the chain of mbufs to see how many mbufs are
                        ** large enough to need DMA.
                        */
				/* ix32459 Offset into pca buffer has to be double word aligned */
                        if (xfer_len <= PIO_THRESHOLD || even_word || (pca_off - xfer_len) % 8 || xfer_len % 4 ) {
                                if (resid == 0) {
                                        while ((tp = tp->m_next) !=
                                                NULL && tp->m_len <= 0)
                                                ;
                                }
                                /*
                                ** If this was the last mbuf, cause the PSCA
                                ** send command to be issued.
                                */
                                if (tp == NULL) {
                                        /* Need one DMA element for pseudo-DMA */
                                        ndma++;
                                }
                        } else {
                                ndma++;
                                if (resid == 0) {
                                        while ((tp = tp->m_next) != NULL
                                                && tp->m_len <= 0)
                                                ;
                                }
                        }

                        /*
                        ** If the PSCA buffer is full, index a new
                        ** PSCA buffer.
                        */
                        if (pca_off >= ca->caddp.config_params.xmitsz) {
                                pca_off = 0;
                        }
                }
        }

        CATDEBUG(("calling dma_alloc() from cat_fastwrt()\n"));
        if ((dmap = dma_alloc(ca, xmitp, ndma)) == NULL) {
           free_xmit_element(ca, xmitp);
           ENABLE_INTERRUPTS(spl);
           DDHKWD1(HKWD_DD_CATDD,DD_EXIT_WRITE,EAGAIN,0x56);
           return EAGAIN;
        }

        if (cat_get_cfb(ca, &xmitp->xm_pca_ctrlbuf)) {
           free_xmit_element(ca, xmitp);
           ENABLE_INTERRUPTS(spl);
           DDHKWD1(HKWD_DD_CATDD,DD_EXIT_WRITE,EAGAIN,0x57);
           return EAGAIN;
        }

        npca = (bcnt+ca->caddp.config_params.xmitsz-1)
                / ca->caddp.config_params.xmitsz;

        if (reserve_pca_xmit(ca, xmitp, 0, npca)) {
           free_xmit_element(ca, xmitp);
           ENABLE_INTERRUPTS(spl);
           DDHKWD1(HKWD_DD_CATDD,DD_EXIT_WRITE,EAGAIN,0x58);
           return EAGAIN;
        }

        /*
        ** We have a valid transmit element, the necessary number of DMA
        ** elements and adapter buffers.  Now, we initialize the transmit
        ** element, and begin setting up the DMA requests.
        */
        scp = ca->sc[wr_ext.cio_ext.netid];
        mode = scp->specmode;

        xmitp->xm_open = scp->link[wr_ext.cio_ext.write_id]->lk_open;
        xmitp->xm_length = bcnt;
        xmitp->xm_num_bufs = npca;
        xmitp->xm_scid = wr_ext.cio_ext.netid;
        xmitp->xm_linkid= (mode & CAT_CLAW_MOD) ? wr_ext.cio_ext.write_id : 0;
        xmitp->xm_ack = wr_ext.cio_ext.flag;
        xmitp->xm_ack |= wr_ext.attn_int ? CAT_SEND_ATTN : 0;
        xmitp->xm_writeid = wr_ext.cio_ext.write_id;
        xmitp->xm_cmd.ccw = wr_ext.use_ccw ? wr_ext.ccw : 0;
        xmitp->xm_cmd.correl = wr_ext.cio_ext.write_id;

        /*
        ** Process each mbuf in the chain
        ** the transfer process will update the mbuf pointer
        ** after an mbuf has been processed.
        */
        rc = 0;
        pca_off = 0;
        num = 0;
        while (mbufp) {
                resid = mbufp->m_len;
                xfer_len = xfer_off = 0;
                /* ix30181 */
                even_word = (unsigned int)(MTOD(mbufp,char *))% 4; /* if evenly divisible by 4 then DMA*/


                while( resid > 0 ) {
                        xfer_len = (resid >
                                (ca->caddp.config_params.xmitsz-pca_off))
                                ? (ca->caddp.config_params.xmitsz-pca_off)
                                : resid;
                        xfer_addr = MTOD(mbufp,caddr_t) + xfer_off;
                        pca_addr = xmitp->xm_pca_lst[num].buffer + pca_off;
                        xfer_off += xfer_len;
                        pca_off += xfer_len;
                        xmitp->xm_pca_lst[num].length += xfer_len;
                        resid -= xfer_len;

                        /*
                        ** Now, check to see whether we need to use dma or pio.
                        ** Some mbufs may be smaller than the pio threshold, so
                        ** check the chain of mbufs to see how many mbufs are
                        ** large enough to need DMA.
                        */
				/* ix32459 Offset into pca buffer has to be double word aligned */
                        if (xfer_len <= PIO_THRESHOLD || even_word || (pca_off - xfer_len) % 8 || xfer_len % 4 ) {
                                /*
                                ** pio stuf here
                                ** cat_write_buf will only fail if a PIO fault
                                ** occurs.
                                */
                                if (cat_write_buf(ca,pca_addr,xfer_addr,xfer_len)) {
                                        ENABLE_INTERRUPTS(spl);
                                        DDHKWD1(HKWD_DD_CATDD, DD_EXIT_WRITE, EIO,0x59);
                                        return EIO;
                                }
                                if (resid == 0) {
                                        while ((mbufp = mbufp->m_next) != NULL
                                                && mbufp->m_len <= 0)
                                                ;
                                }
                                /*
                                ** If this was the last mbuf, cause the PSCA
                                ** send command to be issued.
                                */
                                if (mbufp == NULL) {
                                        dmap->dm_open = xmitp->xm_open;
                                        dmap->p.xmit_ptr = xmitp;
                                        dmap->dm_scid = xmitp->xm_scid;
                                        dmap->dm_linkid = xmitp->xm_linkid;
                                        dmap->dm_length = 0;
                                        dmap->dm_req_type = DM_PSEUDO_XMIT;
                                        dmap->dm_state = DM_READY;
                                        dma_request(ca, dmap);
                                }

                        } else {
                                /*
                                ** dma stuff
                                */
                                dmap->dm_open = xmitp->xm_open;
                                dmap->p.xmit_ptr = xmitp;
                                dmap->dm_scid = xmitp->xm_scid;
                                dmap->dm_linkid = xmitp->xm_linkid;
                                dmap->dm_buffer = xfer_addr;
                                dmap->dm_pca_buffer = pca_addr;
                                dmap->dm_length = xfer_len;
                                dmap->dm_xmem = M_XMEMD(mbufp);
                                if( resid == 0 ) {
                                        while ((mbufp=mbufp->m_next) != NULL
                                                && mbufp->m_len <= 0 )
                                                ;
                                }
                                /*
                                ** If this was the last mbuf, cause the PSCA
                                ** send command to be issued.
                                */
                                if (mbufp == NULL)
                                        dmap->dm_end_of_list = 1;
                                dmap->dm_req_type = DM_XMIT;
                                dmap->dm_state = DM_READY;
                                dmap->dm_flags = DMA_NOHIDE;
                                tmp_dmap = dmap;
                                if (mbufp) {
                                        dmap = dmap->dm_next;
                                        dmap->dm_last = tmp_dmap->dm_last;
                                        tmp_dmap->dm_last = tmp_dmap;
                                        tmp_dmap->dm_next = NULL;
                                }
                                dma_request(ca, tmp_dmap);
                        }

                        /*
                        ** If the PSCA buffer is full, index a new
                        ** PSCA buffer.
                        */

                        if (pca_off >= ca->caddp.config_params.xmitsz) {
                                pca_off = 0;
                                num++;
                        }
                }
        }
        ENABLE_INTERRUPTS(spl);
        DDHKWD1(HKWD_DD_CATDD,DD_EXIT_WRITE,rc,0x60);
        return rc;
} /* cat_fastwrt() */
