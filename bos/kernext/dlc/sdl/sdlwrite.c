static char sccsid[] = "@(#)84  1.28.1.9  src/bos/kernext/dlc/sdl/sdlwrite.c, sysxdlcs, bos41J, 9520B_all 5/18/95 18:30:13";

/*
 * COMPONENT_NAME: (SYSXDLCS) SDLC Data Link Control
 *
 * FUNCTIONS: 
 *	pr_write()
 *	write_xid()
 *	write_i_frame()
 *	sdl_xmit()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1987, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/************************************************************************/

/*
**      File Name      : 84
**
**      Version Number : 1.28.1.9
**      Date Created   : 95/05/18
**      Time Created   : 18:30:13
*/


#include "sdlc.h"
#include <sys/errno.h>
#include <sys/uio.h>
#include <sys/lockl.h>


/* defect 122577 */
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
/* end defect 122577 */


/************************************************************************/
/*									*/
/*	sdlc write functions						*/
/*									*/
/************************************************************************/
				/* functions declared in this module	*/

int	pr_write();		/* called by the head code		*/
int	write_xid();		/* send XID data to the device handler	*/
int	write_i_frame();	/* send an i frame to the device hndler	*/
int	sdl_xmit();		/* execute the fp_rwuio command		*/
/************************************************************************/



/************************************************************************/
/*                                                                      */
/* Name:	pr_write                                                */
/*                                                                      */
/* Function:	process a user write command                            */
/*                                                                      */
/* Notes:	if the write command is for an xid, then send the       */
/*		buffer immediatly.  If the user wants to send an      	*/
/*		information frame, then add it to the transmit queue	*/
/*                                                                      */
/* Data									*/
/* Structures:	sdlc port control block                                 */
/*		sdlc station control block				*/
/*		transmit queue (inside the station control block	*/
/*                                                                      */
/* Returns:	DLC_OK 	- if there is room on the transmit queue        */
/*		E_AGAIN	- if the transmit queue is full and NDELAY	*/
/*			  mode was requested				*/
/*                                                                      */
/************************************************************************/

int	pr_write(uiop, mpx, m, ext)

struct  uio             *uiop;          /* Defect 115926 */
struct	dlc_chan	*mpx;		/* pointer to channel ctrl blk	*/
struct	mbuf		*m;		/* pointer to user data		*/
struct	dlc_io_ext	*ext;		/* pointer to write extension	*/

{
	register	PORT_CB	*cb;
	struct	mbuf	*data_m;
	uchar		*data_frame;
	LINK_STATION	*station;
	int		rc;
/* <<< THREADS >>> */
	tid_t           tid;
/* <<< end THREADS >>> */

	cb = (PORT_CB *) mpx->cb;

	/* defect 170450, moved lock down to thread-self check */
	
	/* preset valid return code */
	rc = NORMAL;

	/* if the write routine is called by a process other than the kprocs */
/* <<< THREADS >>> */
	if ((tid = thread_self()) != cb->dlc.kproc_tid)
/* <<< end THREADS >>> */
	{
		/* lock the port cb, defect 170450 */
		simple_lock(&cb->dlc.lock);

		/* check for valid link station correlator, defect 163696 */
		if ((ext->ls_corr < 0) ||
			(ext->ls_corr > MAX_NUM_STATIONS))
		{
			cb->sense = (ulong_t) ext->ls_corr;
			error_log(cb, ERRID_SDL0032, NON_ALERT, 0, FN, LN);
			return(EINVAL);
		}

		/* then use the link station passed in */
#ifdef MULT_PU
		station =
		    (LINK_STATION *) cb->link_station_array[ext->ls_corr];
#endif
	}
	else
	{
		/* the write is running on sdlc's kproc thread, and **
		** the port cb is already locked, defect 170450     */

		/* use the active link station */
		station = cb->active_ls;
	}

	/* check for valid link station correlator */
	if (!exists(cb, station))
	{
		cb->sense = (ulong_t)station;
    		error_log(cb, ERRID_SDL0032, NON_ALERT, 0, FN, LN);
		return(EINVAL);
	}

	if (cb->sdllc_trace)
#ifdef MULT_PU
		sdlmonitor(cb, WRITE_COMMAND, station->ll.sls.gdlc_ls_corr,
			   m, &ext->flags, 4);
#endif


	/* if logical link is opened */
	if ((station->ll_status == OPENED) && (cb->pl_status == OPENED))
	{
		/* if currently in auto response mode */
		if (station->auto_resp)
		{
			cb->wait_end_ar = EVENT_NULL;

			/* then stop auto response */
			cb->rc = fp_ioctl((struct file*)cb->fp, MP_STOP_AR, 0, 0);

			/* wait for async status */
			e_sleepl((int *)&cb->dlc.lock, (int *)&cb->wait_end_ar, 0);
		}

		/* set station mode to active */
		if (station->poll_mode == SLOW)
			--(cb->slow_count);
		else if (station->poll_mode == IDLE)
			--(cb->idle_count);
		station->poll_mode = ACTIVE;
			
		/* then proceed with the write */

		/* get a small mbuf for the data link header	*/
		data_m = (struct mbuf *) m_get(M_WAIT, MT_DATA);

		if (data_m == NULL) 
		{
			/* out of mbufs */
			rc = ENOMEM;
		}

		else	/* fill in the buffer	*/
		{

			/* get a pointer to the data area */
			data_frame = MTOD(data_m, uchar *);

			/* chain the mbuf passed in to this one */
			data_m->m_next = m;

			/* add the DLC header information */
			data_m->m_len = HEADER_LENGTH;

			if (cb->station_type == PRIMARY)
			{
				data_frame[ADDRESS] =
					station->ll.sls.raddr_name[0];
			}
			else
				data_frame[ADDRESS] = station->ll.sdl.secladd;

			/*
			** classify type of write command
			*/

			/* XID buffer */
			if (ext->flags & DLC_XIDD)
			{
				data_frame[CONTROL] = XID;
				station->xid.m = data_m;
				rc = write_xid(cb, station);
			}

			else 
			{
				rc = write_i_frame(cb, station, data_m);

			}
		}
	}

	else	/* logical link not open, log an error */

	{
		/* head code will free the buffer */
		error_log(cb, ERRID_SDL000B, NON_ALERT, 0, FN, LN);
		rc = EINVAL;
	}


	return(rc);

}	/**** end of pr_write *******************************************/



/************************************************************************/
/*                                                                      */
/* Name:	write_xid                                               */
/*                                                                      */
/* Function:	send out an exchange id                                 */
/*                                                                      */
/* Notes:	send an XID buffer to the device handler                */
/*                                                                      */
/* Data									*/
/* Structures:	port control block					*/
/*		station control block					*/
/*                                                                      */
/* Returns:	DLC_OK	(if XID was sent)                               */
/*		EINVAL	(if station is secondary and did not receive	*/
/*			 an XID command from the remote)		*/
/*                                                                      */
/************************************************************************/

int		write_xid(cb, station)

PORT_CB		*cb;			/* ptr to station control block	*/
LINK_STATION	*station;		/* ptr to station control block	*/

{
	int	rc;

	rc = NORMAL;

	if (cb->sdllc_trace)
#ifdef MULT_PU
		sdlmonitor(cb, WRITE_XID, station->ll.sls.gdlc_ls_corr, 0,
			   station->xid.m, 4);
#endif

	if (cb->station_type == PRIMARY)
	{

		/*
		** if there is an unnumbered cmd/response pending
		*/
		if ((station->unnum_rsp) || (station->unnum_cmd))
		{
/* defect 160103 */
			/*
			** free the small mbuf that was for the SDLC headr info
			** and let the head code free the one it passed down
			*/ 
			m_free(station->xid.m);
			station->xid.m = NULL;
			error_log(cb, ERRID_SDL0006, NON_ALERT, 0, FN, LN);
			rc = EINVAL;
/* end defect 160103 */
		}

		else	
		{
			/* if the station is in the quiesce list */
			if (in_list(&cb->quiesce_list, station))
			{
				/* then remove it */
				delete_ls(cb, &cb->quiesce_list, station);
			}

			/* set XID command pending indicator */
			station->unnum_cmd |= XID_PENDING;

			/* if no other link station active */
 			if ((cb->flags.no_active ) && (cb->active_ls == NULL))
			{
				/* then start transmission */
				cb->poll_seq_sw = FALSE;
				cb->active_ls = station;
				cb->flags.no_active = FALSE;
				pri_transmit(cb, station);
			}

			/* 
			** active_ls is a pointer to the station
			** that is currently in a transmit/receive
			** cycle.  If the write command was for 
			** this station, then don't put it on
			** the active list, this will be done when
			** the tx/rx cycle completes
			*/
			else if (cb->active_ls != station)
			{
				/* put on bottom of active list */
				add_ls(cb, &cb->active_list, station);
			}

		}
	}

	else	/* station is secondary */

	{
		/* if station did not receive XID cmd previously */
		if (!(station->unnum_cmd & XID_PENDING))
		{
			/*
			** free the small mbuf that was for the SDLC headr info
			** and let the head code free the one it passed down
			*/ 
			m_free(station->xid.m);
			station->xid.m = NULL;
			error_log(cb, ERRID_SDL0006, NON_ALERT, 0, FN, LN);
			rc = EINVAL;
		}

		else	/* send XID */
                {
                        cb->write.m = station->xid.m;
                        if (station->ll.sls.flags & DLC_TRCO)
                                transmit_trace(cb);

                        /* buffer freed by device handler */
                        sdl_xmit(cb, &station->xid, 0, RESPONSE_FRAME);
                        /* even if sdl_xmit fail, do not report it  */
                        /* to user, we just free the buffer         */
                        /* because some user may free buffer under  */
                        /* error condition.                         */
                        station->xid.m = NULL;

                }

	}

	return(rc);

}	/**** end of write_xid ******************************************/



/************************************************************************/
/*                                                                      */
/* Name:	write_i_frame                                           */
/*                                                                      */
/* Function:	add an information frame to the transmit queue          */
/*                                                                      */
/* Notes:	If the transmit queue is full return EAGAIN             */
/*                                                                      */
/* Data									*/
/* Structures:	port control block                                      */
/*		station control block					*/
/*		transmit queue (inside the station control block)	*/
/*                                                                      */
/* Returns:	DLC_OK (if there was room in the transmit queue)        */
/*		EAGAIN (if the queue is full)				*/
/*                                                                      */
/************************************************************************/

int		write_i_frame(cb, station, m)

PORT_CB		*cb;			/* ptr to port control block	*/
LINK_STATION	*station;		/* ptr to station control block	*/
struct	mbuf	*m;			/* ptr to data for transmission	*/

{
	int	rc;

	rc = NORMAL;

	/* if in normal response mode */
	if (station->mode == NRM)
	{
		/* is there space in the transmit queue */
		if (((station->in + 1) % MOD) == station->ack_nr)
		{
			/*
			** free the small mbuf that was for the SDLC headr info
			** and let the head code free the one it passed down
			*/ 
			m_free(m);
			/* then xmit queue is full */
			rc = EAGAIN;
		}
		else	/* xmit queue has some empty slots */
		{
 	                /* reset transmit queue empty flag */
 			station->xmit_que_empty = FALSE;
			/* place buffer in first available slot */
			station->tx_que[station->in].m = m;
		
			/* increment the tx_que in pointer */
			IMOD(station->in);
		}

	}

	else	/* log the error */

	{
		/*
		** free the small mbuf that was for the SDLC headr info
		** and let the head code free the one it passed down
		*/ 
		m_free(m);
		error_log(cb, ERRID_SDL000A, NON_ALERT, 0, FN, LN);
		rc = EINVAL;
	}

	return(rc);

}	/**** end of write_i_frame **************************************/



/************************************************************************/
/*                                                                      */
/* Name:	sdl_xmit                                                */
/*                                                                      */
/* Function:	send an mbuf to the device handler                      */
/*                                                                      */
/* Notes:	if the physical link is not closing, then send the      */
/*		mbuf or mbuf chain to the device handler		*/
/*                                                                      */
/* Data									*/
/* Structures:	port control block                                      */
/*		one transmit queue element				*/
/*                                                                      */
/* Returns:	DLC_OK	(if device handler accepts the buffer)          */
/*		EAGAIN	(if device handler rejects the buffer)		*/
/*		EIO	(if device handler i/o fails	     )		*/
/*                                                                      */
/************************************************************************/

int		sdl_xmit(cb, tx, dev_flags, frame_type)

PORT_CB		*cb;			/* ptr to port control block	*/
struct	tx_qe	*tx;			/* ptr to transmite queue elem	*/
ulong		dev_flags;		/* common i/o flags		*/
ulong		frame_type;		/* type of sdlc frame this is	*/

{
	struct	mp_write_extension	mpq_w_ext;
	struct	iovec			iovec;
	struct	uio			uio;
	int				rc;


	if (frame_type == INFORMATION_FRAME)
		performance_trace(cb, DLC_TRACE_SNDIF, tx->m->m_len);
	else
		performance_trace(cb, DLC_TRACE_SNOIF, tx->m->m_len);

	/* preset return code */
	rc = NORMAL;

	if (cb->pl_status != CLOSING)
	{
		/*************************/
		/* set up uio parameters */
		/*************************/

		/* set pointer to transmit buffer */
		iovec.iov_base = (caddr_t) tx->m;

		/* set pointer to iovec structure */
		uio.uio_iov = &iovec;

		/* number of buffer chains (always one) */
		uio.uio_iovcnt = 1;

		/* indicate the buffer is from kernel space */
		uio.uio_segflg = UIO_SYSSPACE;

		/* set the fmode to read/write and blocked */
		uio.uio_fmode = O_RDWR;

		/*
		** set up the mpqp write extension
		*/

		{
			/* removed entire switch(frame_type) - 103136 */

			mpq_w_ext.cio_write.flag = dev_flags;

			/* 103136 */
			sdlmonitor(cb, "WRTb",tx->m, tx->m->m_len, "XMIT", 4);
			rc = fp_rwuio((struct file*)cb->fp, UIO_WRITE,
                                       &uio, (int)&mpq_w_ext);
			sdlmonitor(cb, "WRTe", rc, 0, "XMIT", 4);

			/* if an error occured */
			if (rc)
			{
				/* if the errrno is EIO */
				if (rc == EIO)
					error_log(cb, ERRID_SDL8053,
						NON_ALERT, 0, FN, LN);
				else
					error_log(cb, ERRID_SDL8053,
						NON_ALERT, 0, FN, LN);

				/* removed tx->held = 0; 103136 */
				/* removed dev_flag check - 103136 */
				/* free the mbuf since the device driver
                                   failed the write - 103136 */
				m_freem(tx->m);
 			}
#ifdef MULT_PU
			/* if port is operating in multi-PU mode,
			   and station is secondary */
			if ((cb->flags.mpu_enabled == TRUE) &&
			   (cb->station_type == SECONDARY))
			{
			/* set and start the station's
						 inactivity timer */

			SETTIMER(cb->active_ls->inact_timer,
				 cb->active_ls->inact_ticks);
			ENABLE(cb->active_ls->inact_timer);
			}
#endif
			/* defects 118525 and 103136 */
 			if ((cb->station_type == PRIMARY) &&
                            (frame_type       != INFORMATION_FRAME) &&
			    (*(tx->m->m_data+1)  != TEST) &&
			    (*(tx->m->m_data+1)  != XID))
 			{
 				if (cb->repoll_timer.enabled != TRUE)
 					{
 					cb->repoll_timer.ticks =
 					  cb->active_ls->ll.sls.repoll_time;	
 					ENABLE(cb->repoll_timer);
 					}
			}
		}
	}
	else
	{
		rc = EINVAL;
		m_freem(tx->m);
	}

	return(rc);

}	/**** end of sdl_xmit *******************************************/
