static char sccsid[] = "@(#)82	1.24  src/bos/kernext/dlc/sdl/sdlpri.c, sysxdlcs, bos41J, 9520B_all 5/19/95 12:22:38";

/*
 * COMPONENT_NAME: (SYSXDLCS) SDLC Data Link Control
 *
 * FUNCTIONS: 
 *	pri_rx_endframe()
 *	pri_unnum_setup()
 *	pri_transmit()
 *	pri_rx_s_frame()
 *	pri_rx_snrm_resp()
 *	pri_rx_disc_resp()
 *	pri_rx_xid_resp()
 *	pri_rx_test_resp()
 *	pri_send_disc()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1987, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/************************************************************************/

/*
**      File Name      : 82
**
**      Version Number : 1.24
**      Date Created   : 95/05/19
**      Time Created   : 12:22:38
*/


#include "sdlc.h"



/************************************************************************/
/*									*/
/*	primary link station receive data functions			*/
/*									*/
/************************************************************************/

				/* functions defined in this module	*/

void	pri_rx_endframe();	/* process the receive data		*/
void	pri_unnum_setup();	/* prepare to send an unnumbered cmd	*/
void	pri_transmit();		/* determine what to send next		*/
void	pri_rx_s_frame();	/* reject or normal response to s frame	*/
void	pri_rx_snrm_resp();	/* remote contacted or disconnected	*/
void	pri_rx_disc_resp();	/*                                 	*/
void	pri_rx_xid_resp();	/*                                 	*/
void	pri_rx_test_resp();	/*                                 	*/
void	pri_send_disc();	/*					*/



/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	pri_rx_endframe                                         */
/*                                                                      */
/* Description:	primary station received end frame                      */
/*                                                                      */
/* Function:	process the received frame                              */
/*                                                                      */
/* Input:	pointer to the port control block                       */
/*		pointer to the received data queue element		*/
/*                                                                      */
/* Output:	set up for tansmit                                      */
/*                                                                      */
/* Normal Exit:	return from call                                        */
/*                                                                      */
/* Error Exit:	none                                                    */
/*                                                                      */
/* Return Type:	void                                                    */
/*                                                                      */
/************************************************************************/

void		pri_rx_endframe(cb, rx_qe)

PORT_CB		*cb;
struct	rx_qe	*rx_qe;

{

	struct  dlc_radd_res    * ext;   /* defect 168002 */

	struct	dlc_getx_arg	st_block;
	struct	dlc_io_ext	io_ext;

	LINK_STATION		*station;
	int			valid;
	int			data_len;

	station = cb->active_ls;

	if (cb->m)
		data_len = cb->m->m_len;
	else
		data_len = 0;

	/* store the received data pointer in the port cb */
	cb->m = (struct mbuf *) rx_qe->m_addr;

	/* get access to the data area of the mbuf */
	cb->frame_ptr = MTOD(cb->m, uchar *);

	switch (rx_qe->status)
	{
	case	MP_X21_CPS:
	case	MP_X21_DPI:
	case	MP_MODEM_DATA:

		performance_trace(cb, DLC_TRACE_RDIDA, cb->m->m_len);

		/* set up io extension param */
		io_ext.flags = DLC_NETD;
		io_ext.sap_corr = cb->pl.esap.user_sap_corr;
		io_ext.ls_corr = station->ll.sls.user_ls_corr;
		io_ext.dlh_len = HEADER_LENGTH;

		/* send data and extension to user */
#ifdef MULT_PU
		(*cb->sap_cid->rcvn_fa)(cb->m, &io_ext,cb->sap_cid);
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
			/*
			** removed retry logic
			*/
#endif

			/* save the xid block information	*/
			bcopy(&io_ext, &cb->n_block, &io_ext,
					sizeof(struct dlc_io_ext));

			/* save the pointer to the mbuf		*/
			cb->n_m = cb->m;

			/* log a temporary error	*/
			error_log(cb, ERRID_SDL0063, NON_ALERT, 0, FN, LN);
		}

		/*
		** set valid to FALSE so buffer is not processed
		** further by dlc
		*/
		valid = FALSE;
		break;

	case	MP_BUF_OVERFLOW:
			
		/*
		** set overflow flag, but allow buffer to be
		** processed
		*/
		io_ext.flags = DLC_OFLO;
		valid = TRUE;
		break;

	case	CIO_OK:

  		if /* there is active station */
  			(cb->active_ls != NULL)
		{
			/*
			** if logical link is closed, the physical link is not opened
			** or poll/final already received
			*/
			if ((station->ll_status == CLOSED)
				|| (cb->flags.ignore)
				|| (cb->pl_status != OPENED))
			{
				/* then set flag to discard this buffer */
				valid = FALSE;
	/* if poll/final bit is on */
 	if (cb->m && ((cb->frame_ptr[CONTROL] & POLL_MASK) == POLL_MASK))
 	{
			if (station->unnum_cmd == DISC_PENDING) /* Defect 128485 */
			{
			add_ls(cb, &cb->active_list, station);  
			station->ct.contig_repolls_sent =
					station->ll.sls.max_repoll-1;
			} /* End Defect 128485 */
 		cb->poll_seq_sw = TRUE;
 		cb->flags.ignore = TRUE;
 	}
			}
			else	/* logical link is opened */
			{
				/* if address match or broadcast address, good receive */
				if ((cb->frame_ptr[ADDRESS] == station->ll.sls.raddr_name[0])
					|| (cb->frame_ptr[ADDRESS] == BROADCAST))
	  			{
					valid = TRUE;
				}
                                /* otherwise, if negotiable mode, good receive */
                                else if (station->ll.sls.raddr_name[0] == BROADCAST)
                                {
                                        valid = TRUE;
                                }
                                /* otherwise log error */
				else
				{
 					cb->sense =  (station->ll.sls.raddr_name[0] << 8) | cb->frame_ptr[ADDRESS];
					error_log(cb, ERRID_SDL803C,
							NON_ALERT, 0, FN, LN);

					valid = FALSE;

				}	
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


	/* if valid flag is set */
	if (valid)
	{
		/* then accept this buffer */

		/* if link trace is enabled */
		if (station->ll.sls.flags & DLC_TRCO)
			receive_trace(cb);

		if (cb->sdllc_trace)
			sdlmonitor(cb, PRIMARY_RCV, rx_qe->m_addr,
				rx_qe->status, cb->frame_ptr, 4);
		
 		/* if poll/final bit is on */
 	        /* stop the repoll timer   */
 
 		if (cb->m && ((cb->frame_ptr[CONTROL] & POLL_MASK) == POLL_MASK))
 		{
 			cb->poll_seq_sw = TRUE;
 			cb->flags.ignore = TRUE;
 			DISABLE(cb->repoll_timer);
 		}
 		else /* expect more frame to come */
 		     /* reset the timer           */
 			cb->repoll_timer.ticks =
 			  cb->active_ls->ll.sls.repoll_time;	

		/* if station is in idle poll mode, make it active */
		if (station->poll_mode == IDLE)
		{
			station->poll_mode = ACTIVE;
			--(cb->idle_count);
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
                        /********************************************************************/
                        /*             (*cb->excp_fa)(&raddr, cb->mpx);                     */
                        /********************************************************************/
		}

		/* get control byte and data length */
		cb->control_byte = cb->frame_ptr[CONTROL];

		/*
		** turn on the poll final bit for comparison purposes
		*/
		cb->control_byte |= POLL_MASK;

		/*
		** primary operation mode
		*/


		/* when there is a disconnect command pending */
		if (station->unnum_cmd & DISC_PENDING) 
		{
			/* if p/f bit is on */
			if (cb->frame_ptr[CONTROL] & POLL_MASK)
			{
				/* then put on bottom of active list */
				add_ls(cb, &cb->active_list, station);

			}
			m_freem(cb->m);
		}

		/* when last frame sent was an info or s frame */
		else if ((station->last_sent & U_MASK) != U_MASK)
		{
			/* when rcvd data is an info frame */
			if ((cb->frame_ptr[CONTROL] & DATA_MASK) == 0)
			{
				if (station->rnr)
				{
					/* just free the buffer */
					m_freem(cb->m);
				}
				else
				{
/* defect 77670 */
					rcvd_i_frame(cb, station, rx_qe->status);
/* end defect 77670 */
				}

				if (cb->poll_seq_sw)
				{
					add_ls(cb, &cb->active_list, station);
				}
			}

			/* when it is a supervisory frame */
			else if ((cb->frame_ptr[CONTROL] & U_MASK) == S_FRAME)
			{
				pri_rx_s_frame(cb, station);

				/* if poll/final bit set */
				if (cb->frame_ptr[CONTROL] & POLL_MASK)
				{
					add_ls(cb, &cb->active_list, station);
				}
					
				m_freem(cb->m);
			}

			/* otherwise send a disconnect command */
			else
				pri_send_disc(cb, station);

			/* reset the contig repoll counter */
			station->ct.contig_repolls_sent = 0;

		}

		/* when unnumbered response is pending */
		else if (station->unnum_rsp)
		{
			/* clear "receive first frame pending" flag */
			station->rec_first_pending = FALSE;

			/* ignore any incoming data */
			cb->flags.ignore = TRUE;

			/* if in negotiation mode */
			if (station->ll.sls.raddr_name[0] == BROADCAST)
			{
				/* then if receive remote address */
				if (cb->frame_ptr[ADDRESS] != BROADCAST)
				{
					/* change remote address */
#ifdef MULT_PU
					station->ll.sls.raddr_name[0] =
						    cb->frame_ptr[ADDRESS];
#endif
					/* build status block, 168002 */
					st_block.user_sap_corr =
						cb->pl.esap.user_sap_corr;
					st_block.user_ls_corr =
						station->ll.sls.user_ls_corr;
					st_block.result_ind = DLC_RADD_RES;
					st_block.result_code = DLC_SUCCESS;
					ext = (struct dlc_radd_res *)st_block.result_ext;
					ext -> rname_len = 1;
					ext -> rname[0] =
							cb->frame_ptr[ADDRESS];
					/* send status block	*/
#ifdef MULT_PU
					(*station->cid->excp_fa)(&st_block,
							     station->cid);
#endif
					/* end 168002 */
				}
			}

			/* when DISC response pending */
			if (station->unnum_rsp & DISC_PENDING)
			{
				pri_rx_disc_resp(cb, station);
			}

			/* when SNRM response pending */
			else if (station->unnum_rsp & SNRM_PENDING)
			{
				pri_rx_snrm_resp(cb, station);
				station->ct.contig_repolls_sent = 0;
			}

			/* when TEST response pending */
			else if (station->unnum_rsp & TEST_PENDING)
			{
				pri_rx_test_resp(cb, station, rx_qe->status);
				station->ct.contig_repolls_sent = 0;
			}

			/* when XID response pending */
			else  /* station->unnum_rsp & XID_PENDING */
			{
				pri_rx_xid_resp(cb, station, rx_qe->status);
				station->ct.contig_repolls_sent = 0;
			}

		}
		
	}
	else	/* not valid */
	{
		if (cb->m != NULL)
			m_freem(cb->m);
	}

	performance_trace(cb, DLC_TRACE_RCVBE, cb->m->m_len);

}	/**** end of pri_rx_endframe ************************************/


/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	pri_unnum_setup                                         */
/*                                                                      */
/* Description:	primary unnumbered command setup                        */
/*                                                                      */
/* Function:	Set up for transmission according to the type of        */
/*              unnumbered command that is pending                      */
/*                                                                      */
/* Input:	pointer to the port control block                       */
/*		pointer to the station control block			*/
/*                                                                      */
/* Output:	write command to the device handler                     */
/*                                                                      */
/* Normal Exit:	return from call                                        */
/*                                                                      */
/* Error Exit:	none                                                    */
/*                                                                      */
/* Return Type:	void                                                    */
/*                                                                      */
/************************************************************************/

void		pri_unnum_setup(cb, station)

PORT_CB		*cb;
LINK_STATION	*station;

{
	int	options;	/* options for the device handler write */

	/*
	** find the correct command pending
	*/

	/* is there a DISC command pending */
	if (station->unnum_cmd & DISC_PENDING)
	{
		/* get small mbuf to hold command */
		cb->write.m = (struct mbuf *) m_get(M_WAIT, MT_DATA);

		/* set the pointer to the mbuf data area */
		cb->write_frm = MTOD(cb->write.m, uchar *);

		/* set up the DISC information in the header */
		cb->write_frm[CONTROL] = DISC;
		cb->write.m->m_len = HEADER_LENGTH;
		cb->write_frm[ADDRESS] = station->ll.sls.raddr_name[0];
		options = 0;

		/* set flag to note there is now a DISC response pending */
		station->unnum_rsp |= DISC_PENDING;

		/* save the control byte */
		station->last_sent = cb->write_frm[CONTROL];

		/* if the link trace is enabled */
		if (station->ll.sls.flags & DLC_TRCO)
			transmit_trace(cb);

	}	/* end of DISC */

	/* is there a SNRM command pending */
	else if (station->unnum_cmd & SNRM_PENDING) 
	{
		/* get small mbuf to hold command */
		cb->write.m = (struct mbuf *) m_get(M_WAIT, MT_DATA);

		/* set the pointer to the mbuf data area */
		cb->write_frm = MTOD(cb->write.m, uchar *);

		/* set up the SNRM information in the header */
		cb->write_frm[CONTROL] = SNRM;
		cb->write.m->m_len = HEADER_LENGTH;
		cb->write_frm[ADDRESS] = station->ll.sls.raddr_name[0];
		options = 0;

		/* set flag to note there is now a SNRM response pending */
		station->unnum_rsp |= SNRM_PENDING;

		/* save the control byte */
		station->last_sent = cb->write_frm[CONTROL];

		/* if the link trace is enabled */
		if (station->ll.sls.flags & DLC_TRCO)
			transmit_trace(cb);

	}	/* end of SNRM */


	/* is there a TEST command pending */
	else if (station->unnum_cmd & TEST_PENDING) 
	{
		/* replaced options = CIO_NOFREE_MBUF; 103136 */
		options = CIO_ACK_TX_DONE;

		/* complete test buffer already exists */
		cb->write.m = station->test.m;
		cb->write_frm = MTOD(cb->write.m, uchar *);

		/* save the last control byte */
		station->last_sent = TEST;

		/* set the correst response pending flag */
		station->unnum_rsp |= TEST_PENDING;

		INC_COUNTER(station->ct.test_cmd_sent, MAX_COUNT);

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

		/* if link trace is enabled */
		if (station->ll.sls.flags & DLC_TRCO)
			transmit_trace(cb);

	}	/* end of TEST */

	/* is there an XID command pending */
	else if (station->unnum_cmd & XID_PENDING) 
	{
		/* replaced options = CIO_NOFREE_MBUF; 103136 */
		options = CIO_ACK_TX_DONE;

		/* complete xid buffer already exists */
		cb->write.m = station->xid.m;
		cb->write_frm = MTOD(cb->write.m, uchar *);

		/* save the last control byte */
		station->last_sent = XID;

		/* set the correst response pending flag */
		station->unnum_rsp |= XID_PENDING;

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

		/* if link trace is enabled */
		if (station->ll.sls.flags & DLC_TRCO)
			transmit_trace(cb);
	}

	if (cb->sdllc_trace)
#ifdef MULT_PU
		sdlmonitor(cb, WRITE_UNNUM_CMD,
			 station->ll.sls.gdlc_ls_corr, 0, cb->write_frm, 4);
#endif

	cb->rc = sdl_xmit(cb, &cb->write, options, COMMAND_FRAME);

	/* reset unnumbered command pending flag */
	station->unnum_cmd = FALSE;

}	/**** end of pri_unnum_setup ************************************/



/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	pri_transmit                                            */
/*                                                                      */
/* Description:	primary transmit                                        */
/*                                                                      */
/* Function:	Determine what is going to be sent next.  If there is   */
/*              an unnumbered command pending, sent it otherwise send   */
/*		an info frame or a supervisory frame			*/
/*                                                                      */
/* Input:	pointer to the port control block                       */
/*		pointer to the station control block			*/
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

void		pri_transmit(cb, station)

PORT_CB		*cb;
LINK_STATION	*station;

{

	/* process any rcvd data from this point on */
	cb->flags.ignore = FALSE;

	/* if there is an unnumbered command pending */
	if (station->unnum_cmd)
	{
		pri_unnum_setup(cb, station);
	}

	else
	{
		/* if received a close logical command */
		if (station->ll_status == CLOSING)
		{
			/* if there is still data to send */
			if (station->ns != station->in)
			{
				nrm_setup(cb, station);
			}
			else
			{
				/* if remote still needs to be ack'ed */
				if (station->ack_pending)
				{
					/* send RR frame */
					station->poll_only = TRUE;
					nrm_setup(cb, station);
				}
				else
				{
					station->unnum_cmd |= DISC_PENDING; 
					pri_unnum_setup(cb, station);
				}
			}
		}

		else	/* send an info frame or supervisory frame */
		{
			nrm_setup(cb, station);
		}
	}

	/* update the RAS counters and threshold */
	update_counter(&station->ct.ttl_polls_sent, &station->total_poll_count);

}	/**** end of pri_transmit ***************************************/



/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	pri_rx_s_frame                                          */
/*                                                                      */
/* Description:	primary received a supervisory frame                    */
/*                                                                      */
/* Function:	Verify incoming nr count is valid, set up for normal    */
/*              response if frame is valid, or set up for reject        */
/*              response if frame is invalid                            */
/*                                                                      */
/* Input:	pointer to port control block                           */
/*		pointer to station control block			*/
/*                                                                      */
/* Output:	transmission buffer                                     */
/*                                                                      */
/* Normal Exit:	return from call                                        */
/*                                                                      */
/* Error Exit:	none                                                    */
/*                                                                      */
/* Return Type:	void                                                    */
/*                                                                      */
/************************************************************************/

void		pri_rx_s_frame(cb, station)

PORT_CB		*cb;
LINK_STATION	*station;

{
	int	nr_valid;


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

				station->last_rcvd = cb->frame_ptr[CONTROL];

				/* if rcvd control byte is an RR */
				if ((cb->frame_ptr[CONTROL] & RR_MASK)==REC_RR)
				{
					/* reset the poll only indicator */
					station->sub_state &=
							(~DLC_REMOTE_BUSY);
					station->poll_only = FALSE;
				}
				else	/* it is RNR */
				{
					/* set the poll only indicator */
					station->sub_state |= DLC_REMOTE_BUSY;
					station->poll_only = TRUE;
				}
			}
			else	/* error detected */
			{
				station->unnum_cmd |= DISC_PENDING; 
				station->disc_reason = DLC_PROT_ERR;

				/* invalid nr count received */
				INC_COUNTER(station->ct.invalid_i_frame, MAX_COUNT);
				error_log(cb, ERRID_SDL800A, ALERT, 0, FN, LN);

				station->s_frame_ct = 0;
			}
		}
		else	/* invalid information field */
		{
			station->unnum_cmd |= DISC_PENDING; 
			station->disc_reason = DLC_PROT_ERR;

			/* invalid frame received */
			INC_COUNTER(station->ct.invalid_i_frame, MAX_COUNT);
			error_log(cb, ERRID_SDL8009, ALERT, 0, FN, LN);

			station->s_frame_ct = 0;
		}
	}
	else	/* invalid command received */
	{
		station->unnum_cmd |= DISC_PENDING; 
		station->disc_reason = DLC_PROT_ERR;

		/* invalid frame received */
		INC_COUNTER(station->ct.invalid_i_frame, MAX_COUNT);
		error_log(cb, ERRID_SDL8008, ALERT, 0, FN, LN);

		station->s_frame_ct = 0;
	}

}	/**** end of pri_rx_s_frame *************************************/



/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	pri_rx_snrm_resp                                        */
/*                                                                      */
/* Description:	received a response to the snrm that was sent           */
/*                                                                      */
/* Function:	                                                        */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/* Input:	pointer to the port control block                       */
/*		pointer to the station control block			*/
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

void		pri_rx_snrm_resp(cb, station)

PORT_CB		*cb;
LINK_STATION	*station;

{
	struct	dlc_getx_arg	st_block;

	/* reset snrm response pending flag */
	station->unnum_rsp &= (~SNRM_PENDING);

	/* was the response a valid UA */
	if ((cb->frame_ptr[CONTROL] == UA) && (cb->m->m_len == HEADER_LENGTH))
	{
		/* then inform user that remote is being contacted */
		station->mode = NRM;

		station->sub_state |= DLC_CONTACTED;

		/* build status block	*/
		st_block.user_sap_corr = cb->pl.esap.user_sap_corr;
		st_block.user_ls_corr = station->ll.sls.user_ls_corr;
		st_block.result_ind = DLC_CONT_RES;
		st_block.result_code = DLC_SUCCESS;

		/* send status block	*/
#ifdef MULT_PU
		(*station->cid->excp_fa)(&st_block, station->cid);
#endif
	}

	else	/* send a disconnect */
	{
 		if /* if the response is DM and it is lease line */
 			((cb->frame_ptr[CONTROL] == DM) &&  
		   	(cb->pl.esap.flags & DLC_ESAP_NTWK))
			station->unnum_cmd |= SNRM_PENDING; 
		else
		{
			/* set disconnect reason to protocol error */
			station->disc_reason = DLC_PROT_ERR;
			station->unnum_cmd |= DISC_PENDING; 
		}

	}
	/* put this station on the bottom of the active poll list */
	add_ls(cb, &cb->active_list, station);
	m_freem(cb->m);

}	/**** end of pri_rx_snrm_resp ***********************************/



/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	pri_rx_disc_resp                                        */
/*                                                                      */
/* Description:	received a response to the disc that was sent           */
/*                                                                      */
/* Function:	                                                        */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/* Input:	pointer to the port control block                       */
/*		pointer to the station control block			*/
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

void		pri_rx_disc_resp(cb, station)

PORT_CB		*cb;
LINK_STATION	*station;

{
	struct	dlc_getx_arg	st_block;

	/* reset response pending flag */
	station->unnum_rsp &= (~DISC_PENDING);

	/* Defect 160518 -- move update_counter from end to beginning */
	/* increment the repoll counter */
	update_counter(&station->ct.ttl_repolls_sent,
					&station->repoll_count);

	/*
	** test the threshold percent
	*/
	if (station->total_poll_count == (ulong) MAX_LIMIT)
	{
		/* if repoll count has reached the threshold */
		if ((station->repoll_count * 2) >= station->ll.sdl.prirpth)
		{
			error_log(cb, ERRID_SDL0014, NON_ALERT, 0, FN, LN);

			/* reset counters */
			station->total_poll_count = 0;
			station->repoll_count = 0;
		}
	}
	/* End Defect 160518 */

	/*
	** if station receive a  UA or DM response, or DISC due to 
	** inactivity timeout
	*/
	if ((cb->control_byte == UA)
		|| (cb->control_byte == DM)
		|| (station->disc_reason == (ulong) DLC_INACT_TO))
	{
		/* close the logical link */

		/* if there is an i frame attached */
		if (cb->m->m_len != HEADER_LENGTH)
		{
			error_log(cb, ERRID_SDL8009, ALERT, 0, FN, LN);

		}

		/* close llc and free all buffers */
		clear_buffer(cb, station);

		/* reset the abort timers */
		DISABLE(station->abort_timer);
		station->abort_running = FALSE;

		/* reset contiguous repoll count */
		station->ct.contig_repolls_sent = 0;

		/* if the link trace is enabled */
		if (station->ll.sls.flags & DLC_TRCO)
			session_trace(cb, TRACE_CLOSE);

		/*
		** inform the user of the disconnection
		*/

		/* build status block	*/
		st_block.user_sap_corr = cb->pl.esap.user_sap_corr;
		st_block.user_ls_corr = station->ll.sls.user_ls_corr;
		st_block.result_ind = DLC_STAH_RES;
		st_block.result_code = station->disc_reason;

		/* send status block	*/
#ifdef MULT_PU
		(*station->cid->excp_fa)(&st_block,station->cid);
#endif
                free(station); /* Defect 160518 */
	}

	else	/* send disconnect */

	{
		++(station->ct.contig_repolls_sent);

		/* if the contig repoll count reaches the limit */
		if (station->ct.contig_repolls_sent ==
					station->ll.sls.max_repoll)
		{
			/* then close the logical link */
			clear_buffer(cb, station);

			/* reset the abort timers */
			DISABLE(station->abort_timer);
			station->abort_running = FALSE;

			/* did not respond to repoll */
			error_log(cb, ERRID_SDL8001, ALERT, 0, FN, LN);

			if (station->ll.sls.flags & DLC_TRCO)
				session_trace(cb, TRACE_CLOSE);

			/*
			** inform the user of the disconnection
			*/

			/* build status block	*/
			st_block.user_sap_corr = cb->pl.esap.user_sap_corr;
			st_block.user_ls_corr = station->ll.sls.user_ls_corr;
			st_block.result_ind = DLC_STAH_RES;
			st_block.result_code = station->disc_reason;
	
			/* send status block	*/
#ifdef MULT_PU
			(*station->cid->excp_fa)(&st_block, station->cid);
#endif
                free(station); /* Defect 160518 */
		}
		else	/* send DISC due to protocol error */
		{
			station->disc_reason = DLC_PROT_ERR;
			station->unnum_cmd |= DISC_PENDING; 

			add_ls(cb, &cb->active_list, station);  /* Defect 138344 */
		}
	}

	m_freem(cb->m);

}	/**** end of pri_rx_disc_resp ***********************************/



/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	pri_rec_xid_resp                                        */
/*                                                                      */
/* Description:	primary received a response to the XID command          */
/*                                                                      */
/* Function:	                                                        */
/*                                                                      */
/* Input:	pointer to the port control block                       */
/*		pointer to the station control block			*/
/*		adapter receive status word				*/
/*                                                                      */
/* Output:	notify the user                                         */
/*                                                                      */
/* Normal Exit:	return from call                                        */
/*                                                                      */
/* Error Exit:	none                                                    */
/*                                                                      */
/* Return Type:	void                                                    */
/*                                                                      */
/************************************************************************/

void		pri_rx_xid_resp(cb, station, mpqp_stat)

PORT_CB		*cb;
LINK_STATION	*station;
ulong		mpqp_stat;

{
	struct	dlc_io_ext	xid;


	/* if received a valid XID response */
	if (cb->frame_ptr[CONTROL] == XID)
	{

		/*
		** send the data to the user
		*/

		/* build the xid data block	*/
		xid.sap_corr = cb->pl.esap.user_sap_corr;
		xid.ls_corr  = station->ll.sls.user_ls_corr;
		xid.flags    = DLC_XIDD;
		xid.dlh_len  = HEADER_LENGTH;

		/* if the buffer overflowed	*/
		if ((cb->m->m_len - 2 > station->ll.sls.maxif)
			|| (mpqp_stat == MP_BUF_OVERFLOW))
		{
			/* then set the overflow flag	*/
			cb->m->m_len = station->ll.sls.maxif;
			xid.flags |= DLC_OFLO;
			INC_COUNTER(station->ct.adapter_det_rx, MAX_COUNT);
		}

		/*
		** increment the m_data pointer and decrement the m_len
		** to exclude the SDLC header information
		*/
		m_adj(cb->m, HEADER_LENGTH);

		if (cb->sdllc_trace)
#ifdef MULT_PU
			sdlmonitor(cb, RECEIVED_XID, station->ll.sls.gdlc_ls_corr, 
					cb->m, MTOD(cb->m, uchar *), 4);
#endif

		/*
		** since XID was sucessfully received
		*/
		station->unnum_rsp &= (~XID_PENDING);
		m_freem(station->xid.m);		/* free saved xid buf */
		station->xid.m = NULL;
		
		/* send xid data to the user */
#ifdef MULT_PU
		cb->rc = (*station->cid->rcvx_fa)(cb->m, &xid, station->cid );
#endif

		/* put the station back in the proper poll list */
		if (station->mode == NRM)
			add_ls(cb, &cb->active_list, station);
		else
			add_ls(cb, &cb->quiesce_list, station);

		/*
		** if the user function is busy, then all the parameters are
		** stored, and the function is retried every clock tick
		** (200 ms).  Each failed call to the function will log an
		** error
		*/
		if (cb->rc == (ulong) DLC_FUNC_RETRY)
		{
#ifdef MULT_PU

			/* save the xid block information	*/
			bcopy(&xid, &station->x_block,
					     sizeof(struct dlc_io_ext));

			/* add station to retry list if necessary */
			add_retry (cb,station);

			/* save the pointer to the mbuf		*/
			station->x_m = cb->m;
#endif

			/* log a temporary error	*/
			error_log(cb, ERRID_SDL0061, NON_ALERT, 0, FN, LN);
		}
	}
	else	/* send DISC command */
	{
/* defect 154624 */
                m_freem(cb->m);
/* end defect 154624 */
		station->disc_reason = DLC_PROT_ERR;
		station->unnum_cmd |= DISC_PENDING; 
		add_ls(cb, &cb->active_list, station);

		/* free the saved XID */
		m_freem(station->xid.m);
		station->xid.m = NULL;

		/* reset the XID response pending flag */
		station->unnum_rsp &= (~XID_PENDING);
	}

}	/**** pri_rx_xid_resp *******************************************/


/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	pri_rx_test_resp                                        */
/*                                                                      */
/* Description:	primary received a response to the test command         */
/*                                                                      */
/* Function:	                                                        */
/*                                                                      */
/* Input:	pointer to the port control block                       */
/*		pointer to the station control block			*/
/*		adapter read status					*/
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

void		pri_rx_test_resp(cb, station, mpqp_stat)

PORT_CB		*cb;
LINK_STATION	*station;
ulong		mpqp_stat;

{
	struct	dlc_getx_arg	st_block;
	uchar			*frame;
	int			test_len;
	int			op_res;
	int			rc;


	test_len = station->test.m->m_len;

	/* when control byte is DM */
	if (cb->frame_ptr[CONTROL] == DM)
		op_res = DLC_RDISC;

	/* when control byte is not TEST with p/f bit on */
	else if (cb->frame_ptr[CONTROL] != TEST)
		op_res = DLC_PROT_ERR;

	/* when test response with p/f bit on, but no i field */
	else if (cb->m->m_len == HEADER_LENGTH)
		op_res = DLC_NO_RBUF;

	/* when test response with p/f bit on, but bad  i field len */
	else if (cb->m->m_len != test_len)
		op_res = DLC_BAD_DATA;

	/* when there is a buffer overflow */
	else if (((cb->m->m_len - 2) > station->ll.sls.maxif)
			|| (mpqp_stat == MP_BUF_OVERFLOW))
	{
		op_res = DLC_BAD_DATA;

		INC_COUNTER(station->ct.adapter_det_rx, MAX_COUNT);
	}

	/* otherwise it is a test response with the p/f bit on */
	else
	{
		/* get pointer to the saved test buffer data area */
		frame = MTOD(station->test.m, uchar *);

		/*
		** compare the data field of the transmitted buffer
		** with the data field of the received buffer
		** (they should be the same)
		*/
#ifdef MULT_PU
		rc = bcmp(&frame[DATA], &cb->frame_ptr[DATA], (test_len-2));
#endif
		/* if the data compare failed */
		if (rc)
			/* then set the operation result */
			op_res = DLC_BAD_DATA;

		else
			op_res = DLC_SUCCESS;

	}

	/* if bad test response */
	if (op_res != DLC_SUCCESS)
	{
		/* then increment the test failure count */
		INC_COUNTER(station->ct.test_cmd_fail, MAX_COUNT);
	}

	m_freem(station->test.m);
	station->test.m = NULL;

	/* reset response pending flag for TEST */
	station->unnum_rsp &= (~TEST_PENDING);

	/*
	** inform the user that the test command
	** has completed
	*/

	/* build status block	*/
	st_block.user_sap_corr = cb->pl.esap.user_sap_corr;
	st_block.user_ls_corr = station->ll.sls.user_ls_corr;
	st_block.result_ind = DLC_TEST_RES;
	st_block.result_code = op_res;
	
	/* send status block	*/
#ifdef MULT_PU
	(*station->cid->excp_fa)(&st_block, station->cid);
#endif

	/* put the station back in the proper list */
	if (station->mode == NRM)
		add_ls(cb, &cb->active_list, station);
	else
		add_ls(cb, &cb->quiesce_list, station);

	m_freem(cb->m);

}	/**** end of pri_rx_test_resp ***********************************/


/************************************************************************/
/*                                                                      */
/* Name:	pri_send_disc                                           */
/*                                                                      */
/* Function:	send a disconnect command to the remote                 */
/*                                                                      */
/* Notes:	                                                        */
/*                                                                      */
/* Data									*/
/* Structures:	sdlc port control block                                 */
/*                                                                      */
/* Returns:	void                                                    */
/*                                                                      */
/************************************************************************/

void	pri_send_disc(cb, station)

PORT_CB		*cb;		/* pointer to port control block	*/
LINK_STATION	*station;	/* pointer to link station ctrl block	*/

{
	/* if rcvd frame reject response */
	if ((cb->frame_ptr[CONTROL] & POLL_MASK) == FRMR)
	{
		/* 
		** then remote initiated
		** disconnect
		*/
		station->disc_reason = DLC_RDISC;

		/* if the secondary did not give a disconnect reason */
		if (cb->m->m_len < 5)
			error_log(cb, ERRID_SDL800E, ALERT, 0, FN, LN);
		else if (cb->frame_ptr[REASON] == INVALID_COMMAND)
			error_log(cb, ERRID_SDL8004, ALERT, 0, FN, LN);
		else if (cb->frame_ptr[REASON] == INVALID_I_FIELD)
			error_log(cb, ERRID_SDL8005, ALERT, 0, FN, LN);
		else if (cb->frame_ptr[REASON] == COUNT_INVALID)
			error_log(cb, ERRID_SDL8006, ALERT, 0, FN, LN);
		else
			error_log(cb, ERRID_SDL800E, ALERT, 0, FN, LN);
	}

	/* if rcvd DM */
	else if ((cb->frame_ptr[CONTROL] & POLL_MASK) == DM)
	{
		station->disc_reason = DLC_RDISC;
		error_log(cb, ERRID_SDL8002, ALERT, 0, FN, LN);
	}

	else	/* bad response to info/supervisory frame */
	{
		station->disc_reason = DLC_PROT_ERR;
		error_log(cb, ERRID_SDL8019, NON_ALERT, 0, FN, LN);
	}

	/* set DISC pending */
	station->unnum_cmd |= DISC_PENDING; 

	/* if poll/final bit set */
	if (cb->frame_ptr[CONTROL] & POLL_MASK)
	{
		add_ls(cb, &cb->active_list, station);
	}

	m_freem(cb->m);

}
