static char sccsid[] = "@(#)83  1.40  src/bos/kernext/dlc/sdl/sdlsec.c, sysxdlcs, bos41J, 9520B_all 5/19/95 12:23:05";

/*
 * COMPONENT_NAME: (SYSXDLCS) SDLC Data Link Control
 *
 * FUNCTIONS: 
 *	sec_rx_endframe()
 *	gen_unnum_resp()
 *	gen_frame_reject()
 *	rcvd_i_frame()
 *	nr_validate()
 *	sec_rx_s_frame()
 *	nrm_setup()
 *	start_auto_resp()
 *	xmit_endframe()
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
**      File Name      : 83
**
**      Version Number : 1.40
**      Date Created   : 95/05/19
**      Time Created   : 12:23:05
*/


#include "sdlc.h"
#include <sys/uio.h>

#define	AUTO_RESP_FAILED	0x02


/************************************************************************/
/*									*/
/*	secondary link station receive data functions			*/
/*									*/
/************************************************************************/

				/* functions defined in this module	*/

void	sec_rx_endframe();	/* process secondary rcv data		*/
void	gen_unnum_resp();	/* generate unnumbered response		*/
void	gen_frame_reject();	/* generate frame reject		*/
void	rcvd_i_frame();		/* received an information frame	*/
int	nr_validate();		/* validate the received nr count	*/
void	sec_rx_s_frame();	/* secondary received a supervisory frm	*/
void	nrm_setup();		/* set up for normal response mode	*/
void	start_auto_resp();	/* start auto response mode		*/
void	xmit_endframe();	/* close the logical link              	*/

/**** end of module information *****************************************/



/************************************************************************/
/*									*/
/* Function								*/
/* Name:	sec_rx_endframe						*/
/*									*/
/* Description: secondary receive endframe acknowledgment		*/
/*									*/
/* Function:	secondary station does the following:			*/
/*		    if poll/final bit set, set up for next xmission	*/
/*		    else continue to receive				*/
/*									*/
/* Input:	pointer to the port control block			*/
/*		pointer to a receive_data queue element			*/
/*									*/
/* Output:	set up for transmit/receive				*/
/*									*/
/* Normal Exit: return from routine					*/
/*									*/
/* Error Exit:	none							*/
/*									*/
/* Return Type:	void							*/
/*									*/
/************************************************************************/

void		sec_rx_endframe(cb, rx_qe)

PORT_CB		*cb;
struct	rx_qe	*rx_qe;

{
	struct	dlc_getx_arg	st_block;
	struct	dlc_io_ext	io_ext;
	LINK_STATION		*station;

	ulong			valid;
	int			data_len;

#ifdef MULT_PU
	/* get addressability to the buffer */
	cb->m = (struct mbuf *) rx_qe->m_addr;

	/* get access to the data area */
	cb->frame_ptr = MTOD(cb->m, uchar *);

	if (cb->station_type == PRIMARY)
	{
		station = cb->active_ls;
	}
	else /* secondary */
	{
		/* if operating in multi-pu mode */
		if (cb->flags.mpu_enabled == TRUE)
		{
			/*** begin Billerica design change 11-18-92 ***/
			/*** Allows point-to-point operation on a
			     multi-pu open.  Adds back support for
			     the broadcast address to a single point-
			     to-point secondary station. ***/

			/* if operating point-to-point, and
			   address 0xFF was received */
			/* Defect 83880 begin */
			if ( ((cb->pl.esap.flags & DLC_ESAP_LINK) == 0)
			     && (cb->frame_ptr[ADDRESS] == 0xFF) )
			/* Defect 83880 end */
			{
				/* if any station exists */
				if (cb->mpu_sta_list.head != NULL)
				{
					/* set active link station */
					station =  cb->mpu_sta_list.head;
					cb->active_ls = station;
				}
				else /* no active link station */
				{
					/* null active link station */
					cb->active_ls = NULL;
				}
			}
			else /* not 0xFF address */
			{

				/* get addressability to the station from
				   the received frame's link station
				   address.
				   Note: this may be garbage if network
					 data, or null if no station */

				station = cb->link_station_array[
						   cb->frame_ptr[ADDRESS]];

				/* set the active link station */
				cb->active_ls = station;
			}
			/*** end Billerica design change 11-18-92 ***/
		}
		else /* single station secondary */
		     /* note: this was split out so that there was no
			      impact to the migration single station
			      secondary. (non-mpu)  */
		{
		     station = cb->active_ls;
		}
	}
#endif

	if (cb->m)
		data_len = cb->m->m_len;
	else
		data_len = 0;

	performance_trace(cb, DLC_TRACE_RCVBB, data_len);

	switch (rx_qe->status)
	{
	case	MP_X21_CPS:
	case	MP_X21_DPI:
	case	MP_MODEM_DATA:

		performance_trace(cb, DLC_TRACE_RDIDA, cb->m->m_len);

#ifdef MULT_PU
		/* force station and active_ls to zero (fast asserts) */
		station = NULL;
		cb->active_ls = NULL;

		/* if retry network data is already pending */
		if (cb->n_m != DLC_NULL)
		{
			/* break out to discard the received buffer */
			valid = FALSE;
			break;
		}

		/* set up io extension param */
		io_ext.flags = DLC_NETD;
		io_ext.sap_corr = cb->pl.esap.user_sap_corr;
		io_ext.ls_corr = 0;      /* no station involved */
		io_ext.dlh_len = HEADER_LENGTH;

		/* send data and extension to user */
		(*cb->sap_cid->rcvn_fa)(cb->m, &io_ext, cb->sap_cid);
#endif

		/*
		** if the user function is busy, then all the
		** parameters are stored, and the function is
		** retried every clock tick (200 ms).
		** Each failed call to the function will log an
		** error
		*/

		if (cb->rc == (ulong) DLC_FUNC_RETRY)
		{
#ifdef MULT_PU
			/* save the network data block information */
#endif
			bcopy(&io_ext, &cb->n_block, &io_ext,
					sizeof(struct dlc_io_ext));

			/* save the pointer to the mbuf		*/
			cb->n_m = cb->m;

			/* log a temporary error */
			error_log(cb, ERRID_SDL0063, NON_ALERT, 0, FN, LN);
		}

		/*
		** set valid to FALSE so buffer is not processed
		** further by dlc
		*/
		valid = FALSE;
#ifdef MULT_PU
		/* note: zero out the mbuf pointer so that the mbuf is
		   not returned to the pool at the end of this routine */
		cb->m = NULL;
#endif
		break;

	case	MP_BUF_OVERFLOW:
			
		/*
		** set overflow flag, but allow buffer to be
		** processed
		*/
		io_ext.flags = DLC_OFLO;
		valid = TRUE;
		break;

	case	MP_AR_DATA_RCVD:	/* data rcvd while in auto resp mode */
		/* turn off auto response flag */
		station->auto_resp = FALSE;

		if (cb->sdllc_trace)
			sdlmonitor(cb, END_AUTO_RESP, 0, 0, "DATA", 4);

	case	CIO_OK:			/* data received */

  		if /* there is active station */
  			(cb->active_ls != NULL)
		{
			/* there is no llc closing state for secondary */
			/* if either logical or physical link is not opened */

			if ((cb->pl_status != OPENED)
				|| (station->ll_status != OPENED))
			{
				/*
				** If a halt_ls command has been sent
				** then a DISC command is received
				*/
		    		if ((station->ll_status == CLOSING)
					&& (cb->frame_ptr[CONTROL] == DISC))
						/* then respond to it */
						valid = TRUE;
					else
						valid = FALSE;
			}

			else	/* logical link is opened */

			{
				/* check address mismatch */
				if (cb->frame_ptr[ADDRESS] != station->ll.sdl.secladd
			    	&& cb->frame_ptr[ADDRESS] != BROADCAST)
				{
					/* set flag to discard this buffer */
					valid = FALSE;
				}
				else
					valid = TRUE;
			}
		}
		else valid = FALSE;

		break;

	default:
		cb->sense = rx_qe->status;
		error_log(cb, ERRID_SDL0026, NON_ALERT, 0, FN, LN);
		valid = FALSE;
		break;

	}	/* end of case */

	/* if valid flag is set, then handle this buffer	*/
	if (valid)
	{
#ifdef MULT_PU
		/* if in multi-pu mode */
		if (cb->flags.mpu_enabled == TRUE)
		{
			/* restart the station's receive
					       inactivity timer */
			SETTIMER(station->inact_timer,
					     station->inact_ticks);
			ENABLE(station->inact_timer);
		}
#endif
		/* if link trace enabled, write trace record */
		if (station->ll.sls.flags & DLC_TRCO)
			receive_trace(cb);

		/* if internal trace enabled, write trace record */
		if (cb->sdllc_trace)
			sdlmonitor(cb, SECONDARY_RCV, rx_qe->m_addr,
					rx_qe->status, cb->frame_ptr, 4);

		/*
	    	** once a SNRM is received, any other receive (except 
	    	** another SRNM) will cause us to go into contacted mode
	    	*/
		if (station->conn_pending)
		{

			if ((cb->frame_ptr[CONTROL] | POLL_MASK) == SNRM)
			{
				/* received another SNRM so do nothing */
			}
			else
			{
				/*
				** if this is the first frame for this
				** receive burst
				*/
				if (!station->unnum_rsp)
				{
					/* set this link to normal resp mode */
					station->mode = NRM;

					station->conn_pending = FALSE;
					station->sub_state |= DLC_CONTACTED;

					/* build status block	*/
					st_block.user_sap_corr =
						cb->pl.esap.user_sap_corr;
					st_block.user_ls_corr =
						station->ll.sls.user_ls_corr;
					st_block.result_ind = DLC_CONT_RES;
					st_block.result_code = DLC_SUCCESS;

					/* send status block	*/
#ifdef MULT_PU
					(*station->cid->excp_fa)(&st_block,
					      station->cid);
#endif

				}

				else

				{
					/*
		        		** already received a SNRM before this
					** frame, in the same burst,
					** ignore this frame
		        		*/
				}
			}
		}

		/* 
		** check inactivity_without_termination pending
		*/
		if (station->inact_pending)
		{
			/* reset inactivity without termination indicator */
			station->inact_pending = FALSE;

			/* build status block	*/
			st_block.user_sap_corr = cb->pl.esap.user_sap_corr;
			st_block.user_ls_corr = station->ll.sls.user_ls_corr;
			st_block.result_ind = DLC_IEND_RES;
			st_block.result_code = DLC_SUCCESS;

			/* send status block	*/
#ifdef MULT_PU
			(*station->cid->excp_fa)(&st_block, station->cid);
#endif
		}

		/* get control byte and data length */
		cb->control_byte = cb->frame_ptr[CONTROL];

		/* mask the poll final bit in the control byte */
		cb->control_byte |= POLL_MASK;

		/*
		** secondary modes of operation
		*/

		/*
		** select one of the following
		*/

		/* -> there is an unnumbered response pending */
		if (station->unnum_rsp)
		{

/* defect 154624 */
                        m_freem(cb->m);  /* defect 110865 */
/* end defect 154624 */

			/* if poll/final set */
			if (cb->frame_ptr[CONTROL] & POLL_MASK)
			{

				/* restore the saved buffer */
				cb->m = cb->stored_m;

				cb->frame_ptr = MTOD(cb->m, uchar *);

				cb->control_byte = cb->stored_control_byte;

				gen_unnum_resp(cb, station, rx_qe->status);
			}
		}

		/* there is a frame reject response pending */
		else if (station->frmr_response_pending)
		{
			/*
			** if the control byte is a valid SNRM
			** or DISC command
			*/

			/* if the control byte is a DISC or SNRM */
			if ((cb->control_byte == DISC
				|| cb->control_byte == SNRM)
				/* and it contains no i field */
			    	&& cb->m->m_len == HEADER_LENGTH)
			{
				/*  
				** then respond to the unnumbered command
				** instead of the frame reject
				*/
				if (cb->frame_ptr[CONTROL] & POLL_MASK)
				{
					gen_unnum_resp(cb, station,
							rx_qe->status);
				}

				else
				{
					/* save this buffer */
					cb->stored_m = cb->m;
					cb->stored_control_byte =
						cb->control_byte;
					station->unnum_rsp = TRUE;
				}
				station->frmr_response_pending = FALSE;
			}

			else	/* it is still a frame reject cond */

			{
				if (cb->frame_ptr[CONTROL] & POLL_MASK)
				{
					/* - get a small mbuf for the cmd*/
					cb->write.m = (struct mbuf *)
							m_get(M_WAIT, MT_DATA);

					/* - set pointer to data area	*/
					cb->write_frm =
						MTOD(cb->write.m, uchar *);
					
					gen_frame_reject(cb, station);
				}

/* defect 110865 */
				m_freem(cb->m);
/* end defect 110865 */
			}
		}

		/* -> dlc received unnumbered frame */
		else if ((cb->control_byte & U_MASK) == U_MASK)
		{
			/* check poll/final bit */
			if (cb->frame_ptr[CONTROL] & POLL_MASK)
				gen_unnum_resp(cb, station, rx_qe->status);
			else
			{
				cb->stored_m = cb->m;
				cb->stored_control_byte = cb->control_byte;
				station->unnum_rsp = TRUE;
			}

			/* reset receive_first_frame_pending flag */
			station->rec_first_pending = FALSE;
		}

		/* received frame in normal disconnect mode */
		else if (station->mode == NDM)
		{
			/* check poll/final bit */
			if (cb->frame_ptr[CONTROL] & POLL_MASK)
				gen_unnum_resp(cb, station, rx_qe->status);
			else
			{
				/* free this buffer */
/* defect 110865 */
				m_freem(cb->m);
/* end defect 110865 */
			}
		}

		/* supervisory frame */
		else if (cb->control_byte & S_F_MASK)
			sec_rx_s_frame(cb, station);

		else	/* information frame */
		{
			/* check if path control busy */
			if (station->rnr)
			{
				/* check poll/final bit */
				if (cb->frame_ptr[CONTROL] & POLL_MASK)
						nrm_setup(cb, station);

				/* then just free this buffer */
/* defect 110865 */
				m_freem(cb->m);
/* end defect 110865 */
			}

			/* process the i_frame */
			else
				rcvd_i_frame(cb, station, rx_qe->status);
		}

		/*
		** end of selection
		*/

	}
	else	/* not valid */
	{
		/* if there is a buffer */
		if (cb->m != NULL)
		{
			/* then free it */
/* defect 110865 */
			m_freem(cb->m);
/* end defect 110865 */
		}
	}

	performance_trace(cb, DLC_TRACE_RCVBE, data_len);

}	/**** end of sec_rx_endframe ************************************/


/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	gen_unnum_rsp                                           */
/*                                                                      */
/* Description:	generate an unnumbered response                         */
/*                                                                      */
/* Function:	                                                        */
/*                                                                      */
/* Input:	pointer to the port control block                       */
/*		pointer to the station control block			*/
/*		adapter receive status					*/
/*                                                                      */
/* Output:	approriate response                                     */
/*                                                                      */
/* Normal Exit:	return from call                                        */
/*                                                                      */
/* Error Exit:	none                                                    */
/*                                                                      */
/* Return Type:	void                                                    */
/*                                                                      */
/************************************************************************/

void		gen_unnum_resp(cb, station, mpqp_stat)

PORT_CB		*cb;			/* pointer to port ctrl block	*/
LINK_STATION	*station;		/* pointer to station ctl block	*/
int		mpqp_stat;		/* adapter receive status	*/

{

	struct	dlc_io_ext	xid;
	struct	tx_qe		test;
	int			dev_flag;
	/* removed int tx_ack_flag; 103136 */


	/* reset unnumbered response pending indicator	*/
	station->unnum_rsp = FALSE;

	/*
	** set the control information in the mbuf
	*/

	/* - get a small mbuf for the command	*/
	cb->write.m = (struct mbuf *) m_get(M_WAIT, MT_DATA);

	/* - set pointer to data area in mbuf	*/
	cb->write_frm = MTOD(cb->write.m, uchar *);

	/* - length of the buffer is just the size of the SDLC header	*/
	cb->write.m->m_len = HEADER_LENGTH;

	/* - set the address field in the SDLC header	*/
	cb->write_frm[ADDRESS] = station->ll.sdl.secladd;

	/*
	** check the last buffer received to determine what type
	** of response action is required
	*/

	/* if the last item received was a SNRM with no i-field	*/
	if ((cb->control_byte == SNRM) && (cb->m->m_len == 2))
	{
		m_freem(cb->m);

		/* if in normal disconnected mode */
		if (station->mode == NDM)
		{
			/* then send a UA to the primary station */
			cb->write_frm[CONTROL] = UA;
			dev_flag = 0;

			/* if connection is not pending	*/
			if (station->conn_pending != TRUE)
			{
				/* then set connection pending flag	*/
				station->conn_pending = TRUE;
			}
		}

		else	/* close down --> rcvd SNRM while in NRM	*/

		{
			cb->write_frm[CONTROL] = DM;
			/* setup for link station closure - 103136 */
			cb->flags.xmit_endframe_expected = TRUE;
			/* removed dev_flag = CIO_ACK_TX_DONE; 103136 */
			error_log(cb, ERRID_SDL8003, ALERT, 0, FN, LN);
		}

		if (cb->sdllc_trace)
#ifdef MULT_PU
			sdlmonitor(cb, WRITE_UNNUM_RESP,
					station->ll.sls.gdlc_ls_corr,
					     cb->write.m, cb->write_frm, 2);
#endif

		if (station->ll.sls.flags & DLC_TRCO)
			transmit_trace(cb);

		/* transmit the buffer */
		cb->rc = sdl_xmit(cb, &cb->write, dev_flag, RESPONSE_FRAME);

	}

	/* if the last item received was an XID */
	else if (cb->control_byte == XID)
	{
		/* free the mbuf that we just got for build up */
                /* a transmit frame                            */
		m_freem(cb->write.m);

#ifdef MULT_PU
		if (station->x_m != DLC_NULL)
#endif
		{
			m_freem(cb->m);
		}
		else
		{
			/*
			** send the data to the user
			*/

			/* build the xid data block	*/
			xid.sap_corr = cb->pl.esap.user_sap_corr;
			xid.ls_corr  = station->ll.sls.user_ls_corr;
			xid.flags    = (DLC_XIDD | DLC_RSPP);
			xid.dlh_len  = HEADER_LENGTH;

			/* if the buffer overflowed	*/
			if ((cb->m->m_len - 2 > station->ll.sls.maxif)
				|| (mpqp_stat == MP_BUF_OVERFLOW))
			{
				/* then set the overflow flag	*/
				cb->m->m_len = station->ll.sls.maxif;
				xid.flags |= DLC_OFLO;
				INC_COUNTER(
					station->ct.adapter_det_rx,MAX_COUNT);
			}

			/*
			** increment the m_data pointer and decrement the m_len
			** to exclude the SDLC header information
			*/
			m_adj(cb->m, HEADER_LENGTH);

			if (cb->sdllc_trace)
#ifdef MULT_PU
				sdlmonitor(cb, RECEIVED_XID,
					   station->ll.sls.gdlc_ls_corr,
					   cb->m, MTOD(cb->m, uchar *), 4);
#endif
			station->unnum_cmd |= XID_PENDING;
		
			/* send xid data to the user */
#ifdef MULT_PU
			cb->rc = (*station->cid->rcvx_fa)(cb->m, &xid,
							  station->cid);
#endif

			/*
			** if the user function is busy, then all the
			** parameters are stored, and the function is
			** retried every clock tick (200 ms).
			** Each failed call to the function will log an
			** error
			*/
			if (cb->rc == (ulong) DLC_FUNC_RETRY)
			{
#ifdef MULT_PU
	
				/* save the xid block information	*/
				bcopy(&xid, &station->x_block,
					sizeof(struct dlc_io_ext));
				
				/* add the station to the retry list
				   (may already be there) */
				add_retry (cb,station);

				/* save the pointer to the mbuf		*/
				station->x_m = cb->m;
#endif

				/* log a temporary error	*/
				error_log(cb, ERRID_SDL0061,
						NON_ALERT,0, FN, LN);
			}

			/* user program is responsible for freeing m_buff */

		}
	}

	/* unnumbered command is test cmd	*/
	else if (cb->control_byte == TEST)
	{
		if (cb->sdllc_trace)
#ifdef MULT_PU
			sdlmonitor(cb, RECEIVED_TEST,
					 station->ll.sls.gdlc_ls_corr,
						cb->m, cb->frame_ptr, 4);
#endif
		/* increment RAS counter for test command	*/
		INC_COUNTER(station->ct.test_cmd_sent, MAX_COUNT);

		/*
		** return test data to the sender
		*/

		/* if information field overflowed or p/f not set */
		if ((cb->m->m_len - 2 > station->ll.sls.maxif)
			|| (mpqp_stat == MP_BUF_OVERFLOW)
			|| ((cb->frame_ptr[CONTROL] & POLL_MASK) == 0))
		{
			/* then just return the data link header */
			INC_COUNTER(station->ct.adapter_det_rx, MAX_COUNT);
			/* fill in transmission buffer */
			cb->write_frm[CONTROL] = TEST;
		}
 
                else
 		{
 		        m_clget(cb->write.m) ;
			cb->write.m->m_len = cb->m->m_len;
	 		bcopy ( cb->frame_ptr, mtod (cb->write.m, uchar *), 
                                 cb->m->m_len );
		}



		if (station->ll.sls.flags & DLC_TRCO)
			transmit_trace(cb);

		/* if physical link is not closing */
		if (cb->pl_status != CLOSING)
		{
			/* transmit the test buffer */
			test.m = cb->write.m;
			test.held = FALSE;
			cb->rc = sdl_xmit(cb, &test, 0, RESPONSE_FRAME);
		}

		/* buffer will be freed by the device handler */
		/* free the received TEST frame */
		m_freem ( cb->m);

	}

	/* unnumbered command is DISC and there is no i-frame attached */
	else if ((cb->control_byte == DISC) && (cb->m->m_len == 2))
	{
		if (station->mode == NRM)
		{
			/* return UA with p/f bit on */
			cb->write_frm[CONTROL] = UA;

			/* setup for station closure - 103136 */
			cb->flags.xmit_endframe_expected = TRUE;
			/* removed tx_ack_flag = CIO_ACK_TX_DONE; 103136 */
			cb->flags.enque_disc_pending = TRUE;
		}
		else
		{
			/* return DM with p/f bit on */
			cb->write_frm[CONTROL] = DM;

			/* do not expect transmit endframe - 103136 */
			cb->flags.xmit_endframe_expected = FALSE;
			/* removed tx_ack_flag = 0; 103136 */

		}

		if (cb->sdllc_trace)
#ifdef MULT_PU
			sdlmonitor(cb, WRITE_UNNUM_RESP,
				      station->ll.sls.gdlc_ls_corr,
					    cb->write.m, cb->write_frm, 4);
#endif
		
		if (station->ll.sls.flags & DLC_TRCO)
			transmit_trace(cb);

		if (cb->pl_status != CLOSING)
		{
			cb->rc = sdl_xmit(cb, &cb->write,
				0, RESPONSE_FRAME); /* 103136 */
		}

		m_freem(cb->m);
	}

	/* if in Normal disconnect mode and not connecting	*/
	else if ((station->mode == NDM) && (station->conn_pending == FALSE))
	{
		/* return DM with poll/final bit on */
		cb->write_frm[CONTROL] = DM;

		/*
		** set up the write command
		*/

		if (cb->sdllc_trace)
#ifdef MULT_PU
			sdlmonitor(cb, RECEIVED_IN_NDM,
					station->ll.sls.gdlc_ls_corr,
						cb->m, cb->write_frm, 4);
#endif

		if (station->ll.sls.flags & DLC_TRCO)
			transmit_trace(cb);

		if (cb->pl_status != CLOSING)
		{
			cb->rc = sdl_xmit(cb, &cb->write, 0, RESPONSE_FRAME);
		}

		/* free rx buffer */
		m_freem(cb->m);
	}

	else	/* build an FRMR frame	*/
	{
		/* increment the invalid frame RAS counter */
		INC_COUNTER(station->ct.invalid_i_frame, MAX_COUNT);

		/*
		** save the failed control byte for continual retransmission 
		** of rejected frame until a mode setting command
		** (SNRM or DISC) is received
		*/
		station->failed_control_byte = cb->frame_ptr[CONTROL];

		/* if illegal i field rcvd on SNRM or DISC command */
		if (cb->control_byte == SNRM || cb->control_byte == DISC)
		{
			error_log(cb, ERRID_SDL8005, ALERT, 0, FN, LN);
			station->frmr_reason = INVALID_I_FIELD;
		}
		else	/* invalid command */
		{
			error_log(cb, ERRID_SDL8004, ALERT, 0, FN, LN);
			station->frmr_reason = INVALID_COMMAND;
		}


		/* set frame reject response pending flag */
		station->frmr_response_pending = TRUE;

		/* start abort timer */
		if (station->abort_running == FALSE)
		{
#ifdef MULT_PU
			SETTIMER(station->abort_timer,
						  station->abort_ticks);
#endif
			ENABLE(station->abort_timer);
			station->abort_running = TRUE;
		}

		gen_frame_reject(cb, station);

		m_freem(cb->m);
	}

	if (cb->flags.xmit_endframe_expected)	/* 103136 */
	{
		xmit_endframe(cb, station);
	}

}	/**** end of gen_unnum_resp *************************************/



/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	gen_frame_reject                                        */
/*                                                                      */
/* Description:	generate a frame reject response                        */
/*                                                                      */
/* Function:	Send a FRMR to the primary station                      */
/*                                                                      */
/* Input:	pointer to the port control block                       */
/*		pointer to the station control block			*/
/*                                                                      */
/* Output:	frame reject                                            */
/*                                                                      */
/* Normal Exit:	return from call                                        */
/*                                                                      */
/* Error Exit:	none                                                    */
/*                                                                      */
/* Return Type:	void                                                    */
/*                                                                      */
/************************************************************************/

void		gen_frame_reject(cb, station)

PORT_CB		*cb;
LINK_STATION	*station;

{
	struct	frame_ct_info	fc;

	/* load the reject information into the reject bufer */

	cb->write_frm[LAST_CONTROL] = station->failed_control_byte;
	cb->write_frm[REASON] =  station->frmr_reason;
	cb->write_frm[ADDRESS] = station->ll.sdl.secladd;
	cb->write_frm[CONTROL] = FRMR;

	/* load nr and ns counts into a single byte */
	fc.frame.nr_ns_byte = 0;
	fc.frame.count.nr = station->nr;
	fc.frame.count.ns = station->ns;

	/* now copy the byte into the reject frame */
	cb->write_frm[COUNTS] = fc.frame.nr_ns_byte;

	/* fill in the length field */
	cb->write.m->m_len = FRMR_LENGTH;

	if (cb->sdllc_trace)
#ifdef MULT_PU
		sdlmonitor(cb, SEND_FRAME_REJECT,
				 station->ll.sls.gdlc_ls_corr,
					cb->write.m, cb->write_frm, 4);
#endif

	/* if link trace enabled */
	if (station->ll.sls.flags & DLC_TRCO)
   		transmit_trace(cb);

	if (cb->pl_status != CLOSING)
	{
		sdl_xmit(cb, &cb->write, 0, RESPONSE_FRAME);
	}

}	/**** end of gen_frame_reject ***********************************/


				

/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	rcvd_i_frame                                            */
/*                                                                      */
/* Description:	received an information frame                           */
/*                                                                      */
/* Function:	Reject any invalid i frames by sending an FRMR frame.	*/
/*		For normal data, send the buffer to the user and update	*/
/*		the nr and ns counts					*/
/*                                                                      */
/* Input:	pointer to the port control block                       */
/*		pointer to the station control block			*/
/*		adapter receive status					*/
/*                                                                      */
/* Output:	                                                        */
/*                                                                      */
/* Normal Exit:	return from call                                        */
/*                                                                      */
/* Error Exit:	none                                                    */
/*                                                                      */
/* Return Type:	void                                                    */
/*                                                                      */
/************************************************************************/

void		rcvd_i_frame(cb, station, mpqp_stat)

PORT_CB		*cb;
LINK_STATION	*station;
ulong		mpqp_stat;

{
	struct	frame_ct_info	fct;	/* holds the rcvd nr and ns cts	*/
	int	nr_count_valid;
	struct	dlc_io_ext	io_ext; /* required io_extension 	*/

	bzero(&io_ext, sizeof(struct dlc_io_ext));

	/* increment the i_frames_received counter */
	INC_COUNTER(station->ct.i_frames_rec, MAX_COUNT);

	if (station->poll_mode != ACTIVE)
        {	if (station->poll_mode == SLOW)
 			--(cb->slow_count);
		else    --(cb->idle_count);
		station->poll_mode = ACTIVE;
	}  

	/*
	** check the received nr count and free acknowledged transmit
	** buffers
	*/

	nr_count_valid = nr_validate(cb, station);

	if (nr_count_valid)
	{
		/* then check the received ns count */

		/*
		** if the received ns count equals the internal nr count
		*/

		/* copy the byte into the fct so nr and ns can be extracted */
		fct.frame.control = cb->frame_ptr[CONTROL];

		if (fct.frame.count.ns == station->nr)
		{
			/* set data len for user */
			m_adj(cb->m, HEADER_LENGTH);

			/* then check i_field overflow */
			if (mpqp_stat == MP_BUF_OVERFLOW)
			{
				if (cb->m->m_len > station->ll.sls.maxif)
					cb->m->m_len = station->ll.sls.maxif;

				/* set overflow flag and increment ctr	*/
				io_ext.flags = DLC_OFLO;

				INC_COUNTER(station->ct.adapter_det_rx,
								     MAX_COUNT);

			}
			else if (cb->m->m_len > station->ll.sls.maxif)
			{
				cb->m->m_len = station->ll.sls.maxif;

				/* set overflow flag and increment ctr	*/
				io_ext.flags = DLC_OFLO;
			}


			if (cb->sdllc_trace)
#ifdef MULT_PU
				sdlmonitor(cb, RECEIVED_I_FRAME,
					    station->ll.sls.gdlc_ls_corr,
						cb->m, cb->frame_ptr, 4);
#endif

			/* send i frame to the user */
			io_ext.sap_corr = cb->pl.esap.user_sap_corr;
			io_ext.ls_corr = station->ll.sls.user_ls_corr;
                        io_ext.dlh_len = HEADER_LENGTH;  /* defect 151355 */
			io_ext.flags |= DLC_INFO;

#ifdef MULT_PU
/* defect 96156 ix37135 */
			IMOD(station->nr);
/* defect 96156 ix37135 */

			cb->rc = (*station->cid->rcvi_fa)(cb->m, &io_ext,
							     station->cid);
			/* if the user accepted the i-frame */
			if (cb->rc == (ulong) DLC_FUNC_OK)
			{
				/* increment internal nr count */
				station->ack_pending = TRUE;
				station->poll_only = FALSE;
				station->sub_state &= (~DLC_REMOTE_BUSY);
			}

			/* else if the return code was valid */
			else if ((cb->rc == (ulong) DLC_FUNC_BUSY) ||
				 (cb->rc == (ulong) DLC_FUNC_RETRY))
			{
				if ((cb->rc == (ulong) DLC_FUNC_RETRY))
				{
					/* Will retry the function every
					** 200 ms, and log a temporary
					** error every time the user func
					** fails
					*/

					/* add the station to the retry list
					   (if not already added) */
					add_retry (cb, station);

					/* set "dlc" local busy mode */
					station->rnr |= DLC_SET_LBUSY;
				}
				else /* will wait for DLC_EXIT_LBUSY ioctl
					from the user */
				{
					/* set "user" local busy mode */
					station->rnr |= USER_SET_LBUSY;
				}

				/* save the pointer to the mbuf */
				station->i_m = cb->m;

				/* save the io_ext information  */
				bcopy(&io_ext, &station->i_block,
					sizeof(struct dlc_io_ext));

				/* set local busy substate */
				station->sub_state |= DLC_LOCAL_BUSY;

				/* log a temporary error */
				error_log(cb, ERRID_SDL0062,
						NON_ALERT,0, FN, LN);
			}
			else /* error - bad return from rcvi_fa() */
			{
				/* log a temporary error */
				/* Note - separated so that line number will
					  indicate which error occurred */
				error_log(cb, ERRID_SDL0062,
						NON_ALERT,0, FN, LN);
				/* free the mbuf */
/* defect 110865 */
				m_freem(cb->m);
/* end defect 110865 */
			}

#endif
		}
		else	/* invalid received ns count */
		{
/* defect 110865 */
			m_freem(cb->m);
/* end defect 110865 */
			INC_COUNTER(station->ct.invalid_i_frame, MAX_COUNT);
		}

	}

	/*
	** set up for sending protocol_error response
	*/

	/* if nr count is invalid */
	if (nr_count_valid == FALSE)
	{
		INC_COUNTER(station->ct.invalid_i_frame, MAX_COUNT);

		if (cb->station_type == PRIMARY)
		{
			/* set protocol error */
			station->disc_reason = DLC_PROT_ERR;
			station->unnum_cmd |= DISC_PENDING;
			error_log(cb, ERRID_SDL800A, ALERT, 0, FN, LN);
/* defect 154624 */
                        m_freem(cb->m);
/* end defect 154624 */
				
		}
		else	/* it is a secondary station */
		{
			/* get a small mbuf for the FRMR  */
			cb->write.m =
				(struct mbuf *) m_get(M_WAIT, MT_DATA);
			cb->write_frm = MTOD(cb->write.m, uchar *);

			/* set up frame reject response */
			station->failed_control_byte =  cb->frame_ptr[CONTROL];

			station->frmr_reason = COUNT_INVALID;
			error_log(cb, ERRID_SDL8006, ALERT, 0, FN, LN);

			/* set frame reject response pending */
			station->frmr_response_pending = TRUE;

			/* if poll/final bit set */
			if (cb->frame_ptr[CONTROL] & POLL_MASK)
			{
				/*
				** then call the reject routine
				*/

				/* set the abort timer */
				if (station->abort_running == FALSE)
				{
#ifdef MULT_PU
					SETTIMER(station->abort_timer,
						  station->abort_ticks);
#endif
					ENABLE(station->abort_timer);
					station->abort_running = TRUE;
				}
				gen_frame_reject(cb, station);
			}
			else m_freem(cb->write.m);  /* defect 160103 */
			m_freem(cb->m);
		}
	}
	else	/* not rejected */
	{

		if (cb->station_type == SECONDARY)
		{
			/* if poll / final is set */
			if (fct.frame.control & POLL_MASK)
			{
				/* then generate a response */
				nrm_setup(cb, station);
			}
		}
	}

}	/**** end of rcvd_i_frame ***************************************/




/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	nr_validate                                             */
/*                                                                      */
/* Description:	validate the received nr count                          */
/*                                                                      */
/* Function:	Test the incoming nr count to see if it is valid, then  */
/*		free all the acknowledged transmission buffers in the	*/
/*		transmission queue. The last operation is to update	*/
/*		the internal ns count.					*/
/*                                                                      */
/* Input:	pointer to the port control block                       */
/*		pointer to the link station control block		*/
/*                                                                      */
/* Output:	TRUE if nr count is valid                               */
/*		FALSE if it is not valid				*/
/*                                                                      */
/* Normal Exit:	return from call                                        */
/*                                                                      */
/* Error Exit:	none                                                    */
/*                                                                      */
/* Return Type:	int                                                     */
/*                                                                      */
/************************************************************************/

int		nr_validate(cb, station)

PORT_CB		*cb;
LINK_STATION	*station;

{

	struct	frame_ct_info	rx;
	char	rcvd_nr;
	char	anr, ns;
	int	number_acked;
	int	return_code;
	int	i;


	/*
	** copy the control word into the stuct so
	** the nr count can be accessed
	*/
	rx.frame.control = cb->frame_ptr[CONTROL];

	/* now copy the received buffer's nr count into a holding variable */
	rcvd_nr = rx.frame.count.nr;

	/* if the received nr count has wrapped passed the modulus	*/
	if (rcvd_nr < station->ack_nr)
	{
		/* then add the modulus to the rcvd_nr counter */
		rcvd_nr += MOD;
	}

	/* if the transmitted ns count has wrapped passed the modulus */
	if (station->tx_ns < station->ack_nr)
	{
		/* then add the modulus to the ack_nr counter */
		station->tx_ns += MOD;
	}

	/* if the ns count has wrapped past the modulus	*/
	if (station->ns < station->ack_nr)
	{
		/* then add the modulus to the ns count */
		ns = station->ns + MOD;
	}
	else	/* just use the ns count */
		ns = station->ns;

	/*
	** Check the received nr count and free any ackowledged tx buffers
	*/

	/*
	** if the received nr count is still less than or equal to the
	** transmitted ns count
	*/
	if (rcvd_nr <= station->tx_ns)
	{
		/* then the nr count is valid */
		return_code = TRUE;

		/* calculate the number of buffs that were acknowledged */
		number_acked = rcvd_nr - station->ack_nr;

		/* if there are buffers that have been acknowledged */
		if (number_acked > 0)
		{
			/* save the old ack_nr count */
			anr = station->ack_nr;

			/* then update the acknowledged nr count */
			station->ack_nr = rx.frame.count.nr;
 			if (station->ack_nr == station->in)
 				station->xmit_que_empty = TRUE;

			/* free the transmission buffers */
			for (i = 0; i < number_acked; i++)
			{
				m_freem(station->tx_que[anr % MOD].m);
				station->tx_que[anr++ % MOD].m = NULL;
			}

			/*
			** it is possible that the user process could be
			** asleep waiting for room on the transmit queue,
			** Since a slot(s) in the transmit queue has just
			** become available, issue a wakeup.  If there is
			** no process waiting, nothing will happen
			*/
#ifdef MULT_PU
			if (station->cid->writesleep != EVENT_NULL)
				e_wakeup(&station->cid->writesleep);
#endif

		}

		/*
		** if the number of frames we transmitted is more than
		** the number of frames received by the remote
		*/
		if (ns - rcvd_nr)
		{
			/* the dlc must retransmit unacked frames */
			station->retransmission_flag = TRUE;

			/* back off the number sent counter */
			station->ns = rx.frame.count.nr;

			/*
			** the ns count serves as an index into the
			** transmission queue
			*/
		}
	}

	else	/* nr count is invalid */
		return_code = FALSE;

	/* restore xmit ns count */
	station->tx_ns %= MOD;

	return(return_code);

}	/**** end of nr_validate *****************************************/




/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	sec_rx_s_frame                                          */
/*                                                                      */
/* Description:	Secondary station received a supervisory frame          */
/*                                                                      */
/* Function:	Verify the incoming nr count is valid, set up for       */
/*              normal response is the frame is valid, set up for       */
/*              reject response if the frame is invalid.                */
/*                                                                      */
/* Input:	pointer to the port control block                       */
/*		pointer to the station control block			*/
/*                                                                      */
/* Output:	transmission buffer                                     */
/*                                                                      */
/* Normal Exit:	nr count valid (TRUE)                                   */
/*                                                                      */
/* Error Exit:	nr count invalid (FALSE)                                */
/*                                                                      */
/* Return Type:	void                                                    */
/*                                                                      */
/************************************************************************/

void		sec_rx_s_frame(cb, station)

PORT_CB		*cb;
LINK_STATION	*station;

{
	int	nr_valid;
	int	reject_frame;


	/* preset no error conditon */
	reject_frame = FALSE;

	/* if RR or RNR is indicated in the received control byte */
	if ((cb->frame_ptr[CONTROL] & RR_MASK) < 8)
	{
		/* then if no i field attached */
		if (cb->m->m_len == HEADER_LENGTH)
		{
			/* then check the nr count */
			nr_valid = nr_validate(cb, station);

			if (nr_valid)
			{
				/* then set response mode */

				/* if rcvd control byte is an RR */
				if ((cb->frame_ptr[CONTROL] & RR_MASK)==REC_RR)
				{
					/* reset the poll only indicator */
					station->sub_state
						&= (~DLC_REMOTE_BUSY);
					station->poll_only = FALSE;
				}
				else	/* set the poll only indicator */
				{
					station->poll_only = TRUE;
					station->sub_state |= DLC_REMOTE_BUSY;
				}

				/* if poll final is set */
				if (cb->frame_ptr[CONTROL] & POLL_MASK)
				{
					nrm_setup(cb, station);
				}

				station->last_rcvd = cb->frame_ptr[CONTROL];
			}
			else	/* reject frame for invalid nr count */
			{
				reject_frame = TRUE;
				/* set the frame reject reason */
				station->frmr_reason = COUNT_INVALID;
				error_log(cb, ERRID_SDL8006, ALERT, 0, FN, LN);

			}
		}
		else	/* reject frame for invalid i field */
		{
			reject_frame = TRUE;

			/* set reject reason */
			station->frmr_reason = INVALID_I_FIELD;
			error_log(cb, ERRID_SDL8005, ALERT, 0, FN, LN);
		}
	}
	else	/* reject frame for invalid command */
	{
		reject_frame = TRUE;

		/* set reject reason */
		station->frmr_reason = INVALID_COMMAND;
		error_log(cb, ERRID_SDL8004, ALERT, 0, FN, LN);
	}

	/* if the reject reason is set (not normal) */
	if (reject_frame)
	{
		/*
		** then reject the frame
		*/

/* defect 154624 */
		INC_COUNTER(station->ct.invalid_i_frame, MAX_COUNT);
		station->failed_control_byte = cb->frame_ptr[CONTROL];

		station->frmr_response_pending = TRUE;

		/* if poll/final bit is set */
		if (cb->frame_ptr[CONTROL] & POLL_MASK)
		{
			/* get a small mbuf */
			cb->write.m = (struct mbuf *) m_get(M_WAIT, MT_DATA);

			/* get addressability to the data area */
			cb->write_frm = MTOD(cb->write.m, uchar *);
/* defect 154624 */

			gen_frame_reject(cb, station);

			if (station->abort_running == FALSE)
			{
#ifdef MULT_PU
				SETTIMER(station->abort_timer,
						  station->abort_ticks);
#endif
				ENABLE(station->abort_timer);
				station->abort_running = TRUE;
			}
		}
	}
	m_freem(cb->m);

}	/**** end of sec_rx_s_frame *************************************/




/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	nrm_setup                                               */
/*                                                                      */
/* Description:	set up for normal response                              */
/*                                                                      */
/* Function:	Generate a transmission buffer when in normal response  */
/*              mode.  Send an RR when there is nothing to transmit,    */
/*              send an RNR when there are no buffers available, or     */
/*		send an information frame it there is one.		*/
/*                                                                      */
/* Input:	pointer to the port control block                       */
/*		pointer to the station control block			*/
/*                                                                      */
/* Output:	completed transmission buffer                           */
/*                                                                      */
/* Normal Exit:	return from call                                        */
/*                                                                      */
/* Error Exit:	none                                                    */
/*                                                                      */
/* Return Type:	void                                                    */
/*                                                                      */
/************************************************************************/

void		nrm_setup(cb, station)

PORT_CB		*cb;
LINK_STATION	*station;

{
	int	tx_window;
	int	done;
	int	no_i_frames;		/* true if there are no info	*/
	uchar	old_ns;
					/* frames waiting to be sent	*/
	struct	frame_ct_info	tx;
	struct	dlc_getx_arg	ex_block;
	int     i_frames_resent_updated =0;   /* defect 165982 */

					/* 103136 */
	int	tx_ack_flag;

	/*
	** generate normal response
	*/

	/*
	** NOTE: The ns count serves as the index into the transmit queue
	**       and is backed off when buffers must be retransmitted, the
	**	 tx_ns count serves as the "high water" mark for retrans-
	**       mission purposes
	*/

	/* pre-reset the transmit acknowledgement requirement - 103136 */
	tx_ack_flag = FALSE;

	/* calculate the transmission window */
	tx_window = station->ns - station->ack_nr;

	if (tx_window < 0)
		tx_window += MOD;

	if ((tx_window == station->ll.sls.xmit_wind)
		|| (station->in == station->ack_nr))
	{
		no_i_frames = TRUE;
	}
	else
		no_i_frames = FALSE;

	/* when rnr flag is on and there are no i frames ready to send */
/* defect 96156 ix37135 */
	if (station->rnr )
/* defect 96156 ix37135 */
	{
		/* send an RNR */

		/* get a small mbuf */
		cb->write.m = (struct mbuf *) m_get(M_WAIT, MT_DATA);
		cb->write_frm = MTOD(cb->write.m, uchar *);

		/* build the control byte */
		tx.frame.type = SEND_RNR;
		tx.frame.count.nr = station->nr;

		cb->write_frm[CONTROL] = tx.frame.control;
		cb->write.m->m_len = HEADER_LENGTH;

		if (cb->station_type == PRIMARY)
			cb->write_frm[ADDRESS] = station->ll.sls.raddr_name[0];
		else
			cb->write_frm[ADDRESS] = station->ll.sdl.secladd;
		
		if (cb->sdllc_trace)
#ifdef MULT_PU
			sdlmonitor(cb, WRITE_RNR,
					station->ll.sls.gdlc_ls_corr,
						   0, cb->write_frm, 4);
#endif

		if (station->ll.sls.flags & DLC_TRCO)
			transmit_trace(cb);

		/* save last control byte */
		station->last_sent = cb->write_frm[CONTROL];

		/* transmit the RNR */
		cb->rc = sdl_xmit(cb, &cb->write, 0, SUPERVISORY_FRAME);

	}

	/* when in poll only mode or no info frames to send */
	else if (station->poll_only || no_i_frames)
	{
		/* get a small mbuf */
		cb->write.m = (struct mbuf *) m_get(M_WAIT, MT_DATA);
		cb->write_frm = MTOD(cb->write.m, uchar *);

		/* build the control byte */
		tx.frame.type = SEND_RR;
		tx.frame.count.nr = station->nr;
		cb->write_frm[CONTROL] = tx.frame.control;

		if (cb->station_type == PRIMARY)
		{
			/* if sending same rr again */
			if (station->last_sent == cb->write_frm[CONTROL])
				++(station->s_frame_ct);
			else
				station->s_frame_ct = 0;

			/* if non productive poll max is reached */
 			/* and user wants to turn on slow poll   */
                        /* slow_ticks of 0 means no slow poll    */
			if ((station->s_frame_ct == 3)
				&& ( station->poll_mode != SLOW)
				&& ( cb->slow_ticks != 0))
			{
				station->poll_mode = SLOW;
				++(cb->slow_count);

				if (!cb->slow_timer.enabled)
				{
					SETTIMER(cb->slow_timer,
							cb->slow_ticks);
					ENABLE(cb->slow_timer);
				}
			}
		}
		cb->write.m->m_len = HEADER_LENGTH;

		if (cb->station_type == PRIMARY)
		{
			cb->write_frm[ADDRESS] = station->ll.sls.raddr_name[0];
			station->last_sent = cb->write_frm[CONTROL];
		}
		else
		{
			cb->write_frm[ADDRESS] = station->ll.sdl.secladd;

#ifdef notdef
#ifdef MULT_PU
			/*** Auto-Response is not activated at this time
			   due to difficulties in the MPQP dd.  If ever
			   activated, the code should be as follows ***/

			/* if not in multi-pu mode, and
			   if the same RR was received twice */
			if ((cb->flags.mpu_enabled == FALSE) &&
			    (station->last_rcvd == cb->frame_ptr[CONTROL])
				&& (station->auto_resp != AUTO_RESP_FAILED))
#endif /* MULT_PU */
				start_auto_resp(cb, station);
			else
			{
				station->auto_resp = FALSE;
#endif
				station->last_sent = cb->write_frm[CONTROL];
#ifdef notdef
			}
#endif
		}

		if (cb->sdllc_trace)
#ifdef MULT_PU
			sdlmonitor(cb, WRITE_RR, station->ll.sls.gdlc_ls_corr, 0, cb->write_frm, 4);
#endif

		if (station->ll.sls.flags & DLC_TRCO)
			transmit_trace(cb);


		/* transmit the RR */
		cb->rc = sdl_xmit(cb, &cb->write, 0, SUPERVISORY_FRAME);
	}

	else	/* send information frames */

	{
		station->s_frame_ct = 0;

		/* if this is a retransmission */
		if (station->ns == station->saved_ns)
		{
			/* then increment the i_frames resent counter */
			++(station->ct.i_frames_resent);
			/* defect 165982 */
			i_frames_resent_updated = TRUE;
			/* end defect 165982 */
		}
		else
		{
			station->saved_ns = station->ns;
			station->ct.i_frames_resent = 0;
		}

		/* if the retransmission limit has not been reached */
		if (station->ct.i_frames_resent != station->ll.sdl.retxct)
		{

			/*
			** send out buffers until the ns count equals the   
			** value of the in pointer.
			*/
			if ((station->ns != station->in)
				&& (tx_window < station->ll.sls.xmit_wind))
			{
				done = FALSE;
			}
			else
				done = TRUE;

			while (!done)
			{
				/* get the current buffer in the queue */
				cb->write.m =
					station->tx_que[station->ns].m;

				/* removed ref to cb->write.held - 103136 */

				/* get addressability to the data area */
				cb->write_frm = MTOD(cb->write.m, uchar *);

				/* load the address and counts */
				if (cb->station_type == PRIMARY)
				{
					cb->write_frm[ADDRESS] =
						station->ll.sls.raddr_name[0];
				}
				else
					cb->write_frm[ADDRESS] =
							station->ll.sdl.secladd;

			
				/* build control byte */
				tx.frame.control = 0;
				tx.frame.count.nr = station->nr;
				tx.frame.count.ns = station->ns;
			

				++tx_window;

				/* if this is not a retransmitted frame */
				if (station->tx_ns == station->ns)
				{
					/* then increment transmitted count */
					IMOD(station->tx_ns);

					if (station->ct.i_frames_sent
								< MAX_COUNT)
						++(station->ct.i_frames_sent);
				}

				/* increment internal ns count */
				old_ns = station->ns;
				IMOD(station->ns);

				/* is this the last buffer in the burst */
				if ((station->ns != station->in)
				    && (tx_window < station->ll.sls.xmit_wind))
				{
					/* no it is not last buffer */
					done = FALSE;
				}
				else
				{
					/* yes it is last buffer */
					done = TRUE;
				}

				/* if it is the last buffer in the burst */
				if (done)
				{
					/* then turn on the poll/final bit */
					tx.frame.count.pf_bit = 1;

					/* 103136 */
					if (cb->station_type == PRIMARY)
					{
						/* Need to enable transmit ack
						   so that repoll timeout is not
						   started too early */
						tx_ack_flag = CIO_ACK_TX_DONE;
					} 
				}

				/*
				** copy the control info into the
				** transmit buffer
				*/
				cb->write_frm[CONTROL] = tx.frame.control;

				/*********************************************/
				/* begin 103136                              */
				/* Copy the small mbufs and double link the  */
                                /* cluster so that there is no conflict with */
                                /* the device on m_free and retransmits.     */ 
				/*********************************************/

				cb->write.m = (struct mbuf *) m_copym
                                          (cb->write.m, 0, M_COPYALL, M_WAIT);

				/* if the m_copy was not successful */
				if (cb->write.m == 0)
    				{
                                	/*************************************/
					/* permanent station error -         */
					/* out of station resources          */
                                	/*************************************/
					error_log(cb, ERRID_SDL8046, NON_ALERT,
								0, FN, LN);
					return;
				}
				/* end 103136 */

				if (cb->sdllc_trace)
#ifdef MULT_PU
					sdlmonitor(cb, WRITE_I_FRAME,
					      station->ll.sls.gdlc_ls_corr,
					      cb->write.m, cb->write_frm, 4);
#endif
				/* if link trace is enabled */
				if (station->ll.sls.flags & DLC_TRCO)
					transmit_trace(cb);

				sdl_xmit(cb, &cb->write, tx_ack_flag,
					INFORMATION_FRAME); /* 103136 */

			} /* end of while */

			/* if retransmission flag is on */
			if (station->retransmission_flag)
			{
				/*
				** update the counter that tracks the
				** number of bursts that contain retransmitted
				** frames
				*/
				/* defect 165982 */
				/* i_frames_resent may have already
				** been updated so don't do it again
				*/
				if (i_frames_resent_updated == FALSE)
				/* end defect 165982 */
				update_counter(&station->ct.i_frames_resent,
						&station->burst_rexmitted);
				update_counter(&station->ct.i_frames_sent,
						&station->total_burst_xmitted);
				station->retransmission_flag = FALSE;
			}
			else
				++(station->total_burst_xmitted);

			/*
			** if total bursts transmitted
			** reached the sample limit
			*/
			if (station->total_burst_xmitted == MAX_LIMIT)
			{
				/* then test for retx exceeding threshold */
				if ((station->burst_rexmitted * 2) >=
					station->ll.sdl.retxth)
				{
					error_log(cb, ERRID_SDL0015,
						NON_ALERT, 0, FN, LN);
				}

				/* reset counters */
				station->total_burst_xmitted = 0;
				station->burst_rexmitted = 0;
			}
		
			/* save last control byte */
			station->last_sent = cb->write_frm[CONTROL];

		}

		else	/* retrans limit has been reached */
		{
			error_log(cb, ERRID_SDL801D, NON_ALERT, 0, FN, LN);

			/* if disconnect on inactivity was selected */
			/* defect 165982 */
			if ((station->ll.sls.flags & DLC_SLS_HOLD) != DLC_SLS_HOLD )
			/* end defect 165982 */
			{
				/* if primary */
				if (cb->station_type == PRIMARY)
				{
					/* then send DISC to remote */
					station->disc_reason = DLC_INACT_TO;
					station->unnum_cmd |= DISC_PENDING;
					pri_unnum_setup(cb, station);
				}
				else	/* secondary, close the link */
				{
					/* sdl_abort moved in Defect 160518 */

					if (station->ll.sls.flags & DLC_TRCO)
						session_trace(cb, TRACE_CLOSE);
				
					ex_block.user_sap_corr =
						cb->pl.esap.user_sap_corr;
					ex_block.user_ls_corr =
						station->ll.sls.user_ls_corr;
					ex_block.result_ind = DLC_STAH_RES;
					ex_block.result_code = DLC_INACT_TO;

					/* send exception to user */
#ifdef MULT_PU
					(*station->cid->excp_fa)(&ex_block,
							     station->cid);
#endif
					/* Defect 160518 -- move sdl_abort */
					sdl_abort(cb);

				}
			}
			else	/* inactivity without termination */
			{
				if (cb->station_type == PRIMARY)
				{
					/* set mode to IDLE */
					/* defect 165982 */
					station->poll_mode = IDLE;
					/* end defect 165982 */

					++(cb->idle_count);
					add_ls(cb, &cb->active_list, station);

					/* if the idle timer is not running */
					if (!cb->idle_timer.enabled)
					{
						/* then set it */
						SETTIMER(cb->idle_timer,
								cb->idle_ticks);
						/* and turn it on */
						ENABLE(cb->idle_timer);
					}

					/* move to new station */
					cb->poll_seq_sw = TRUE;
						
					ex_block.user_sap_corr =
						cb->pl.esap.user_sap_corr;
					ex_block.user_ls_corr =
						station->ll.sls.user_ls_corr;
					ex_block.result_ind = DLC_IWOT_RES;
					ex_block.result_code = DLC_INACT_TO;
	
					/* send exception to user */
#ifdef MULT_PU
					(*station->cid->excp_fa)(&ex_block,
							     station->cid);
#endif
				}
			}
		}
	}

	station->ack_pending = FALSE;

}	/**** end of nrm_setup ******************************************/




/************************************************************************/
/*                                                                      */
/* Name:	start_auto_resp                                         */
/*                                                                      */
/* Function:	tell the device handler to start auto response mode     */
/*                                                                      */
/* Notes:	                                                        */
/*                                                                      */
/* Data									*/
/* Structures:	sdlc port control block                                 */
/*		sdlc station control block				*/
/*                                                                      */
/* Returns:	void                                                    */
/*                                                                      */
/************************************************************************/

void		start_auto_resp(cb, station)

PORT_CB		*cb;
LINK_STATION	*station;

{
	struct	T_STRT_AR_PARMS	ar;

	ar.rcv_timer = station->ll.sls.repoll_time;
	ar.tx_rx_addr = station->ll.sdl.secladd;
	ar.tx_cntl = station->last_sent;
	ar.rx_cntl = station->last_rcvd;

	cb->rc = fp_ioctl(cb->fp, MP_START_AR, &ar, NULL);

	if (cb->sdllc_trace)
		sdlmonitor(cb, START_AUTO_RESP, cb->rc,
				station->last_sent, 0, 0);

	if (cb->rc)
	{
		station->auto_resp = AUTO_RESP_FAILED;
		error_log(cb, ERRID_SDL806A, NON_ALERT, 0, FN, LN);
		sdlmonitor(cb, START_AUTO_RESP, 0, 0, "FAIL", 4);
		nrm_setup(cb, station);
	}
	else
		station->auto_resp = TRUE;

}



/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	xmit_endframe                                           */
/*                                                                      */
/* Description:	transmit endframe                                       */
/*                                                                      */
/* Function:	Called in response to a transmission acknoweldgment     */
/*		Close the logical link when DISC - UA/UM sequence	*/
/*              has completed.  Close the logical link on a mid         */
/*              session reset.  Set station to normal response mode     */
/*              when SNRM - UA sequence has completed. Free the         */
/*		mbuf when an XID is sent. Update the transmission	*/
/*		status when an i frame is sent				*/
/*									*/
/* Input:	pointer to the port control block                       */
/*		pointer to the station control block			*/
/*                                                                      */
/* Output:	updated status, and polling lists                       */
/*                                                                      */
/* Normal Exit:	return from call                                        */
/*                                                                      */
/* Error Exit:	none                                                    */
/*                                                                      */
/* Return Type:	void                                                    */
/*                                                                      */
/************************************************************************/

void		xmit_endframe(cb, station)

PORT_CB		*cb;
LINK_STATION	*station;

{
	int	sp_op;
	struct	dlc_getx_arg	ex_block;

	/* if transmit endframe is expected */
	if (cb->flags.xmit_endframe_expected)
	{
		cb->flags.xmit_endframe_expected = FALSE;

		if (cb->flags.enque_disc_pending)
		{
			/* remote initiated disconnect */
			sp_op = DLC_RDISC;
			cb->flags.enque_disc_pending = FALSE;
		}
		else	/* mid session reset */
			sp_op = DLC_MSESS_RE;

		if (station->ll.sls.flags & DLC_TRCO)
			session_trace(cb, TRACE_CLOSE);

		
		/* build exception block for user notification */
		ex_block.user_sap_corr = cb->pl.esap.user_sap_corr;
		ex_block.user_ls_corr = station->ll.sls.user_ls_corr;
		ex_block.result_ind = DLC_STAH_RES;
		ex_block.result_code = sp_op;

		/* send exception to user */
#ifdef MULT_PU
		(*station->cid->excp_fa)(&ex_block, station->cid);
#endif

		/* clean up after station */
		DISABLE(station->abort_timer);
#ifdef MULT_PU
		mpu_remove_station (cb, station);
#endif
	}
	else	/* xmit endframe not expected, log the error */
		error_log(cb, ERRID_SDL0013, NON_ALERT, 0, FN, LN);

}	/**** end of xmit_endframe **************************************/
