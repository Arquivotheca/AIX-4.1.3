static char sccsid[] = "@(#)56  1.17  src/bos/kernext/cat/cat_recv.c, sysxcat, bos411, 9432B411a 3/30/94 09:27:56";

/*
 * COMPONENT_NAME: (SYSXCAT) - Channel Attach device handler
 *
 * FUNCTIONS: catread()
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

#define FNUM 8
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
#include <sys/comio.h>
#include <sys/except.h>

#include "catdd.h"

#define HANDLE_EXT(extstatus)   { \
        if (ext_p) { \
                ext.cio_ext.status = extstatus; \
                copyout(&ext, ext_p, sizeof(cat_read_ext_t)); \
        } \
}


/*****************************************************************************
**
** NAME:                cat_read
**
** FUNCTION:    this is the entry point for reads (for user processes only)
**
** EXECUTION
** ENVIRONMENT: process thread only
**
** NOTES:
**
** Input:               dev --- device number
**                              uiop  --- pointer to uio structure
**                              chan  --- channel number
**                              ext_p --- read extension structure (if present)
**
** Output:
**
** Returns:             0 ------ Success
**                              ENXIO -- invalid data from MPX routine
**                              EINVAL - kernel user is calling this routine
**                              EINTR -- system call was interrupted
**                              EMSGSIZE data too big for user rcv buffer
**                              EFAULT - problem copying data to user space
**
** Called By:   kernel when user application calls "read"
**
** Calls To:    e_sleep(), uiomove(), i_disable(), i_enable()
**
*****************************************************************************/
int
catread(
        dev_t                   dev,
        struct uio              *uiop,
        chan_t                  chan,
        cat_read_ext_t  *ext_p)
{
        struct ca               *ca;
        open_t                  *openp;
        recv_elem_t             *recvp;
        struct mbuf             *mbufp;
/* 141333 addition */
		struct mbuf				*tmbufp, *tmbufp2;
		int						bytes_moved;
        recv_elem_t             *rcvp;
/* End 141333 addition */
        cat_read_ext_t  ext;

        int             total_bytes,
                        bytes_to_move,
                        already_locked,
                        rc,
                        ext_rc,
                        found,
                        spl,
                        i;

        DDHKWD4(HKWD_DD_CATDD, DD_ENTRY_READ, 0, dev, chan,
                        uiop->uio_iov->iov_base, uiop->uio_resid);

        rc = 0;
        ext_rc = CIO_OK;

        if ((chan < 0)
             || (chan > CAT_MAX_OPENS)
             || ((ca = catget(minor(dev))) == NULL)
             || (((openp = &ca->open_lst[chan])->op_flags&OP_OPENED) == 0 )) {
                DDHKWD1(HKWD_DD_CATDD, DD_EXIT_READ, ENXIO, dev);
                HANDLE_EXT(CIO_OK);
                return ENXIO;
        }

        if (openp->op_mode & DKERNEL) {
                /*
                ** Kernel users can't use this entry point.
                ** They give us a rcv_function to call
                ** from dma_dequeue() when the data is ready.
                */
                HANDLE_EXT(CIO_OK);
                return EINVAL;
        }

        DISABLE_INTERRUPTS( spl );
        while( ((recvp = openp->recv_act) == NULL)
        || (recvp->rc_state != RC_COMPLETE)) {
                while( recvp ) {
                        if( recvp->rc_state == RC_COMPLETE )
                                goto foundit;
                        recvp = recvp->rc_next;
                }
                /*
                ** no data available
                */
                if (openp->op_mode & DNDELAY) {
                        ENABLE_INTERRUPTS( spl );
                        DDHKWD1(HKWD_DD_CATDD, DD_EXIT_READ, 0, dev);
                        HANDLE_EXT(CIO_OK);
                        return 0;
                }
                /*
                ** sleep waiting for a receive event
                */
                if (SLEEP(&openp->op_rcv_event) != EVENT_SUCC) {
                        ENABLE_INTERRUPTS( spl );
                        DDHKWD1(HKWD_DD_CATDD, DD_EXIT_READ, EINTR, dev);
                        HANDLE_EXT(CIO_OK);
                        return EINTR;
                }
        }

foundit:
        /*
        ** Take an element from the "active" list.
        */
        recvp = openp->recv_act;
        if( recvp->rc_next != NULL )
                recvp->rc_next->rc_last = recvp->rc_last;
        openp->recv_act = recvp->rc_next;
        recvp->rc_next = NULL;
        recvp->rc_last = recvp;
        ENABLE_INTERRUPTS( spl );

        /*
        ** Inform user of source of read data.
        */
        if (ext_p) {
                ext.cio_ext.netid = recvp->rc_scid;
                ext.cio_ext.sessid = recvp->rc_linkid;
                ext.ccw = recvp->rc_ccw;
        }

        /*
        ** return notification in the extension (if supplied)
        ** if the data received is larger than the user's buffer
        */
		bytes_moved = 0;
        if (recvp->rc_count > uiop->uio_resid) {
                ext_rc = CIO_BUF_OVFLW;
                total_bytes = uiop->uio_resid;
        } else {
                total_bytes = recvp->rc_count;
                if (total_bytes < uiop->uio_resid ) {
                        {
                                int mbufcount;
                        for (mbufcount = 0,mbufp=recvp->rc_mbuf_head;mbufp;mbufp=mbufp->m_next)
                                mbufcount++;
                        }
                }
        }


        if( mbufp = recvp->rc_mbuf_head ) {
                while( total_bytes > 0 && mbufp ) {
                        bytes_to_move = mbufp->m_len;
                        if (bytes_to_move > total_bytes)        /* limit data from this mbuf */
                                bytes_to_move = total_bytes;
                        if( rc =
                        uiomove(MTOD(mbufp,caddr_t), bytes_to_move, UIO_READ, uiop) ) {
                                DISABLE_INTERRUPTS( spl );
     /*d50453     */            ca->mbuf_num-=num_mbufs(recvp->rc_mbuf_head);
                                m_freem(recvp->rc_mbuf_head);
                                recvp->rc_mbuf_head = NULL;
                                if( ca->mbuf_event != EVENT_NULL )
                                        e_wakeup(&ca->mbuf_event);
                                ENABLE_INTERRUPTS( spl );
                                DDHKWD1(HKWD_DD_CATDD, DD_EXIT_READ, rc, dev);
                                ext_rc = CIO_LOST_DATA;
                                goto catread_exit;
                        }
                        total_bytes -= bytes_to_move;
/* 141333 addition */
						bytes_moved += bytes_to_move;

						if ( bytes_to_move == mbufp->m_len)
                        	mbufp = mbufp->m_next;
 					    else {
								mbufp->m_data += bytes_to_move;
								mbufp->m_len -= bytes_to_move;
						}

/* End 141333 addition */

                }

/* 141333 addition */
			if( bytes_moved < recvp->rc_count){
				recvp->rc_count -= bytes_moved;    /* decrement the remaining amt in the recv */
				/* Walk the data mbufs and get rid of them */
				tmbufp = recvp->rc_mbuf_head;
				while ( tmbufp != mbufp) {
						tmbufp2 = tmbufp->m_next;
						tmbufp->m_next = NULL;
						m_free(tmbufp);
						tmbufp = tmbufp2;
				}			
				recvp->rc_mbuf_head = mbufp;

				DISABLE_INTERRUPTS(spl);
				/*
				 * Put the recieve element back on the top of the list
				 */

				if ( openp->recv_act ) {
					rcvp = openp->recv_act;
					recvp->rc_last = openp->recv_act->rc_last;
					recvp->rc_next = openp->recv_act;
					openp->recv_act = recvp;
				
				} else {
					openp->recv_act = recvp;
				}


				ENABLE_INTERRUPTS(spl);
				DDHKWD1(HKWD_DD_CATDD, DD_EXIT_READ, rc, dev);
				HANDLE_EXT(ext_rc);
				return rc;
			} else {
/* End 141333 addition */

                DISABLE_INTERRUPTS( spl );
/*d50453     */ ca->mbuf_num-=num_mbufs(recvp->rc_mbuf_head);
                m_freem(recvp->rc_mbuf_head);
                recvp->rc_mbuf_head = NULL;
                if( ca->mbuf_event != EVENT_NULL )
                        e_wakeup(&ca->mbuf_event);
                ENABLE_INTERRUPTS( spl );
			}
        }

        /*
        ** if this subchannel is in the aborted state -- meaning that
        ** there was data lost because of no receive elements, then
        ** we need to signal the 370 that it can begin sending data
        ** again
        */


catread_exit:
        DISABLE_INTERRUPTS( spl );

        /*
        ** Add this element to the "free" list.
        */
        if (ca->recv_free == NULL)
                ca->recv_free = recvp;
        else {
                recvp->rc_last = ca->recv_free->rc_last;
                recvp->rc_last->rc_next = recvp;
                ca->recv_free->rc_last = recvp;
        }
        e_wakeup(&openp->op_rcv_event);

        ENABLE_INTERRUPTS( spl );
        DDHKWD1(HKWD_DD_CATDD, DD_EXIT_READ, rc, dev);
        HANDLE_EXT(ext_rc);
        return rc;
}
