static char sccsid[] = "@(#)80	1.49  src/bos/kernext/dlc/sdl/sdlkproc.c, sysxdlcs, bos41J, 9520B_all 5/18/95 18:29:12";

/*
 * COMPONENT_NAME: (SYSXDLCS) SDLC Data Link Control
 *
 * FUNCTIONS:
 *	decrement_timers()
 *	sdl_timeout()
 *	timer_handler()
 *	rx_timeout()
 *	repoll_to()
 *	inactivity_to()
 *	repoll_exceeds()
 *	abort_to()
 *	status_handler()
 *	process_station()
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
**      File Name      : 80
**
**	Version Number : 1.49 
**	Date Created   : 95/05/18
**	Time Created   : 18:29:12
*/

#include "sdlc.h"
#include <sys/sleep.h>
#include <sys/user.h>
#include "sys/intr.h"   /* defect 167068 */



/* defect 122577 */
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
/* end defect 122577 */


#define	DLC_CLOSE_POST		0x08000000	/* close the port	*/
#define	SDL_DATA_AVAIL		0x04000000	/* data on ring que	*/
#define	IGNORE_RQ_POST		-1
#define	SDL_TIMER_BLOCKS	10
#define	MAX_MBUF_SIZE		0x1000		/* 4096 decimal		*/
#define	TICKS			10		/* ticks every 100 ms	*/

/* macro to cast an integer to an mbuf pointer				 */
#define	ITOM(_p)		((struct mbuf *)(_p))

void	decrement_timers();
void	sdl_timeout();			/* used in call to timeout	*/
void	timer_handler();
void	rx_timeout();
void	repoll_to();
void	inactivity_to();
void	repoll_exceeds();
void	abort_to();
void	status_handler();
void	process_station();

/**** end of module delcaratons *****************************************/



/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	link_manager                                            */
/*                                                                      */
/* Description:	sdlc link manager                                       */
/*                                                                      */
/* Function:	This routine loops until a it receives a proc term      */
/*              signal.  It waits at the beginning of each iteration    */
/*              to be posted by the interrupt handling functions,  it   */
/*		removes the element to be processed from the app-  	*/
/*		ropriate queue, calls the function that will process	*/
/*		the element, then returns to the wait (sleep) state	*/
/*                                                                      */
/* <<< THREADS >>>                                                      */
/* Input:       et_post (post_type) from the interrupt handler functs   */
/* <<< end THREADS >>>                                                  */
/*                                                                      */
/* Output:	pass element info to the approriate handling function   */
/*                                                                      */
/* Normal Exit:	return from call                                        */
/*                                                                      */
/* Error Exit:	none                                                    */
/*                                                                      */
/* Return Type:	int                                                     */
/*                                                                      */
/************************************************************************/

int		link_manager(cb)

register	PORT_CB	*cb;		/* pointer to port control blk	*/

{

	ulong			qwait_rc;
	struct	status_qe	qe;
	int			d_len;
        LINK_STATION           *poll_marker ; /* Defect 119581 */
#ifdef MULT_PU
	short index;
#endif

	/* initialize the process termination flag			*/
/* defect 115819 */
	cb->dlc.term_kproc = FALSE;
/* end defect 115819 */
/* defect 141966 */
        /* initialize the port control block sanity check for posts */
	cb->port_cb_addr = cb;
/* end defect 141966 */

#ifdef MULT_PU
	/* start the ticker timer for continuous posts at 100 ms*/
#endif
	timeout(sdl_timeout, (caddr_t) cb, HZ/TICKS);


	/* main processing loop						*/
/* defect 115819 */
	while (cb->dlc.term_kproc == FALSE)
/* end defect 115819 */
	{

		/* get next data item, or wait if queue is empty	*/
		qwait_rc = sdl_waitq(cb, cb->dlc.rcv_ringq, &qe);

		if (qwait_rc == IGNORE_RQ_POST)
			continue;

		/* defect 126815 */
		simple_lock(&cb->dlc.lock);
		/* end defect 126815 */


		/*
		** check each bit of the qwait_rc to 
		** determine which events were posted
		*/

		/* received signal */
		if (qwait_rc == EVENT_SIG)
		{
			sdlmonitor(cb, "DBUG", 0, 0, "SIG ", 4);
		}

		/* close post request */
		if (qwait_rc & DLC_CLOSE_POST)
		{
/* defect 141966 */
/* moved sdl_close call to the bottom of this CLOSE_POST case entry */
/* defect 141966 */
#ifdef MULT_PU
			/* The following is only applicable if the port is
			   defined as SECONDARY.  It may still be undefined
			   (non-secondary and non-primary) */

			/* Run through the link station array. If there is a
			   non-NULL entry, a link station exists. Check to
			   see if the cid that the station points to is the
			   closing cid. If so, remove the station. If the
			   closing cid is the sap_cid, remove all stations.
			   Please note that it would have been faster to run
			   throught the linked list of stations, especially
			   in the case of remove-all, but the array method is
			   more compact and bullet-proof.  Also, don't call
			   wakeup here anymore. This is done when the link is
			   removed. */

			if (cb->station_type == SECONDARY)
			{
			    if (cb->flags.mpu_enabled)
			    {
				for (index=1;index<MAX_NUM_STATIONS;index++)
				{
				    if (cb->link_station_array[index] !=
					NULL)
				    {
					if
					((cb->link_station_array[index]->cid
					  == cb->dlc.kcid) ||
					(cb->dlc.kcid == cb->sap_cid))
					{
						mpu_remove_station (cb,
						cb->link_station_array[index]);
					}
				    }
				}
			    }
		
			    else /* non-mpu mode */
			    {
				if (cb->mpu_sta_list.head != NULL)
				{
				    mpu_remove_station (cb,
						cb->mpu_sta_list.head);
				}
			    }
			} /* end if SECONDARY */
/* defect 154624 */
                        else /* primary station */
                        {
                            /* abort all the current link stations */
                            sdl_abort (cb);
                        }
/* end defect 154624 */

			/*
			** removed wakeup on writesleep
			*/
#endif

/* defect 141966 */
			sdlmonitor(cb, "DBUG", qwait_rc, 0, "CPST", 4);
                        sdl_close(cb);
			/* defect 126815 */
			simple_unlock(&cb->dlc.lock);
			/* end defect 126815 */
/* end defect 141966 */
			continue;
		}

		/* if ring queue put failed during receive */
		if (qwait_rc & SDL_BAD_RQPUT_DATA)
		{
			error_log(cb, ERRID_SDL8047, NON_ALERT, 0, FN, LN);
			/* defect 126815 */
			simple_unlock(&cb->dlc.lock);
			/* end defect 126815 */

		
			continue;
		}

		/* if ring queue put failed during receive */
		if (qwait_rc & SDL_BAD_RQPUT_STATUS)
		{
			error_log(cb, ERRID_SDL8047, NON_ALERT, 0, FN, LN);
			sdl_abort(cb);
			pl_blowout(cb);

			/* defect 126815 */
			simple_unlock(&cb->dlc.lock);
			/* end defect 126815 */
	

			
			continue;
		}

		/* ticker timer popped	*/
		if (qwait_rc & SDL_TIMER_POST)
		{
			timer_handler(cb);
		}

		/* ring queue data available	*/
		if ((qwait_rc & SDL_RINGQ_POST)
			|| (qwait_rc == SDL_DATA_AVAIL))
		{
					
			/* it is a received data queue element	*/
			if (qe.type == DLC_RCV_DATA)
			{
				if (cb->station_type == PRIMARY)
					pri_rx_endframe(cb, &qe);
				else
					sec_rx_endframe(cb, &qe);
			}

			else	/* else it is a status block qe */
			{
				status_handler(cb, &qe);
			}
		}
		
		/*
		** Loop, below, sends i-frames to secondary stations (when
		** opened as primary).                            
		** 
		** cb->poll_seq_sw is initialized as TRUE when link station is
		** started and loop is entered.  Poll_marker is set to first  
		** station in cb->active_ls and cb->active_ls is searched for
		** 'active' station, i.e., not marked 'SLOW' or 'IDLE'.   
		** Poll_marker marks search of entire list and logic falls 
		** marks search of entire list and logic falls through if no
		** active stations are found.  'Normal' flow drops through
		** process_station() where frame is transmitted, and
		** cb->poll_seq_sw is set to FALSE.  Code falls through loop
		** and response is awaited, above.  cb->poll_seq_sw is set to
		** TRUE when response is received, so loop is re-entered and
		** cycle repeats.
		*/ 
                poll_marker = NULL ;

		while ((cb->poll_seq_sw)		/* Defect 119581 */
			&& (cb->pl_status == OPENED)
			&& (!cb->flags.no_active))
		{
			/* indicate new tx/rx cycle	*/
                        cb->poll_seq_sw = FALSE;
			sdlmonitor(cb, "LOOP", cb->active_ls, poll_marker, 0, 0);

                        cb->active_ls =
                                (LINK_STATION *) pop_ls(&cb->active_list);

                        if (cb->active_ls)
				{
				if (!poll_marker)
					{
					poll_marker = cb->active_ls;
					}
	                        else if (poll_marker == cb->active_ls)
					{
		                	add_ls(cb, &cb->active_list, cb->active_ls);
       		                 	cb->poll_seq_sw = TRUE;
					sdlmonitor(cb, "END1", cb->active_ls, cb->poll_seq_sw, 0, 0);
					break ;
					}
                                process_station(cb, cb->active_ls);
				}
                        else
                        {
                                sdlmonitor(cb, "NOLS", 0, 0, 0, 0);
                                cb->flags.no_active = TRUE;
                                break;
                        }				/* End Defect 119581 */
		}

#ifdef MULT_PU
		/*
		** removed wakeup on xmit_que_empty
		*/
#endif

		/* defect 126815 */
		simple_unlock(&cb->dlc.lock);
		/* end defect 126815 */

 		

	} /* end of while */

/* defect 141966 */
        /* reset the port control block sanity check for posts */
	cb->port_cb_addr = NULL;
/* end defect 141966 */



}	/**** end of link_manager ***************************************/



/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	init_cb                                                 */
/*                                                                      */
/* Description:	initialize the sdlc process                             */
/*                                                                      */
/* Function:	initialize required fields                              */
/*                                                                      */
/* Input:	pointer to the port control block                       */
/*                                                                      */
/* Output:	updated control block structure                         */
/*                                                                      */
/* Normal Exit:	return from call                                        */
/*                                                                      */
/* Error Exit:	none                                                    */
/*                                                                      */
/* Return Type:	int                                                     */
/*                                                                      */
/************************************************************************/

void	init_cb(cb)

PORT_CB	*cb;

{
	int	rc;

	/* turn on internal (monitor) trace */
	cb->sdllc_trace = TRUE;

	/* set the state of the physical link */
	cb->pl_status = CLOSED;

	/* turn on the no station active flag */
	cb->flags.no_active = TRUE;

	cb->rc = NORMAL;

	cb->max_i_frame = MAX_I_FRAME;

	/* allocate the dlc ringq queue structure */
	cb->dlc.rcv_ringq = (struct ring_queue *)dlc_rqcreate();

	/* allocate the ring queue locks  - defect 127690 */
	lock_alloc (&(cb->dlc.ringq_lock), LOCK_ALLOC_PIN, PORT_LOCK,
                                                       PORT_RINGQ_LOCK);
	simple_lock_init (&(cb->dlc.ringq_lock));

	/* end defect 127690 */


	/* allocate timer control blocks */
	timeoutcf(SDL_TIMER_BLOCKS);


}	/**** end of init_cb ********************************************/



/************************************************************************/
/*                                                                      */
/* Name:	init_proc                                               */
/*                                                                      */
/* Function:	initialize the sdlc kernel process                      */
/*                                                                      */
/* Notes:	intializes and calls the link manager, when the link	*/
/*		manager routine finishes, init_proc also does the clean	*/
/*		up							*/
/*                                                                      */
/* Data									*/
/* Structures:	                                                        */
/*                                                                      */
/* Returns:	void                                                    */
/*                                                                      */
/************************************************************************/

void	init_proc(flag, init_parms, parms_len)

int	flag;
void	*init_parms;
int	parms_len;
{
	struct i_parms
	{
		PORT_CB	*cb;
	} *init;
	struct	sigaction	act;
	struct	sigaction	oact;
	int	rc;

	/*
	** set the parent of the KPROC to INIT so the user process 
	** does not get SIG_CLD
	*/
	setpinit();

/* <<< THREADS >>> */
        init = (struct i_parms *) init_parms;

	/**************************************************************/
	/* call thread_self to get the current thread id of this      */
	/* kproc, and save it where the head code can find it for     */
	/* et_post.                                                   */
	/**************************************************************/
	assert((init->cb->dlc.kproc_tid = thread_self()) != -1)
/* <<< end THREADS >>> */

	/* ignore SIGINT */
	act.sa_handler = SIG_IGN;
	rc = sigaction(SIGINT, &act, &oact);

	
	/*
	** Call the link manager function.  This function will not return
	** until the port is closed
	*/
	rc = link_manager(init->cb);

	/* cancel timer */
	untimeout(sdl_timeout, (caddr_t) init->cb);

	/* free timer control blocks */
	timeoutcf(-SDL_TIMER_BLOCKS);

	/* free the dlc ring queue structure */
	dlc_rqdelete(init->cb->dlc.rcv_ringq);

	/* free the ring queue lock - defect 127690 */
	lock_free (&(init->cb->dlc.ringq_lock));
	/* end defect 127690 */


#ifdef MULT_PU
	/*
	** removed wakeup on kcid->proc_id
	*/
#endif
/* defect 141966 */
        /* insure that dlcmpx is able to issue e_sleep
           by forcing the dispatcher to cycle the ready
           processes */
	do
	{
		delay(10);
	} while (init->cb->dlc.kcid->proc_id == EVENT_NULL);

	/* wake up the head code process */
        /* NOTE: the wakeup must be the last action of the kproc */
	e_wakeup((int *)&init->cb->dlc.kcid->proc_id);
/* end defect 141966 */

}



/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	sdl_rx                                                  */
/*                                                                      */
/* Description:	SDLC received data function handler                     */
/*                                                                      */
/* Function:	This function is called by the device handler to        */
/*              process received data.  This function is running on the */
/*              off level, so it must be as fast as possible.		*/
/*		The main function here is to queue the received data	*/
/*		and post the link_manager process.			*/
/*                                                                      */
/* Input:	Address of the port control block                       */
/*		The status of the device handler after the read		*/
/*		A pointer to an mbuf contining the received data	*/
/*                                                                      */
/* <<< THREADS >>>                                                      */
/* Output:      et_post to the link manager process                     */
/* <<< end THREADS >>>                                                  */
/*                                                                      */
/* Normal Exit:	return from call                                        */
/*                                                                      */
/* Error Exit:	none                                                    */
/*                                                                      */
/* Return Type:	void                                                    */
/*                                                                      */
/************************************************************************/

void	sdl_rx(open_id, cio_read, mptr)

ulong	open_id;			/* address of port cb		*/
struct	read_extension	*cio_read;	/* rx status block		*/
struct	mbuf	*mptr;			/* pointer to received data	*/

{
	PORT_CB	*cb;
	struct	rx_qe	rx_qe;		/* receive data queue element	*/
	ulong	rc;
	int rqlock;			/* defect 127690 */

	cb = (PORT_CB *) open_id;

/* defect 115819 */
/* defect 141966 */
        if /* terminating the kproc or it's already terminated */
	   ((cb->dlc.term_kproc) || (cb->port_cb_addr != cb))
/* end defect 141966 */
/* end defect 115819 */
		return;

	/* add the buffer to the receive ringq	*/
	rx_qe.type = DLC_RCV_DATA;

	rx_qe.status = cio_read->status;

	rx_qe.m_addr = (ulong_t) mptr;

	/* lock the ring queue - defect 127690 */
	rqlock = disable_lock (INTCLASS1, &(cb->dlc.ringq_lock));
	rc =  dlc_rqput(cb->dlc.rcv_ringq, &rx_qe);
	/* unlock the ring queue - defect 127690 */
	unlock_enable (rqlock, &(cb->dlc.ringq_lock));


	/* if the ringq put failed */
	if (rc)
	{
		m_freem(mptr);
		cb->rc = rc;
/* <<< THREADS >>> */
		et_post(SDL_BAD_RQPUT_DATA, cb->dlc.kproc_tid);
	}
	else
	{
		/* post the SDLC link manager	*/
		et_post(SDL_RINGQ_POST, cb->dlc.kproc_tid);
/* <<< end THREADS >>> */
	}

}	/**** end of sdl_rx *********************************************/



/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	sdl_timeout                                             */
/*                                                                      */
/* Description:	SDLC timeout function handler                           */
/*                                                                      */
/* Function:	To notify the link_manger that one unit of time         */
/*              has expired                                             */
/*                                                                      */
/* Input:	Address of the port control block                       */
/*                                                                      */
/* <<< THREADS >>>                                                      */
/* Output:      et_post to the SDLC link manager                        */
/* <<< end THREADS >>>                                                  */
/*                                                                      */
/* Normal Exit:	return from call                                        */
/*                                                                      */
/* Error Exit:	none                                                    */
/*                                                                      */
/* Return Type:	void                                                    */
/*                                                                      */
/************************************************************************/

void	sdl_timeout(cb_addr)

ulong	cb_addr;			/* address of port cb		*/

{
	PORT_CB	*cb;

	cb = (PORT_CB *) cb_addr;

/* defect 115819 */
/* defect 141966 */
        if /* terminating the kproc or it's already terminated */
	   ((cb->dlc.term_kproc) || (cb->port_cb_addr != cb))
/* end defect 141966 */
/* end defect 115819 */
		return;

	timeout(sdl_timeout, (caddr_t) cb, HZ/TICKS);

/* <<< THREADS >>> */
	et_post(SDL_TIMER_POST, cb->dlc.kproc_tid);
/* <<< end THREADS >>> */

}	/**** end of sdl_timeout ****************************************/



/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	sdl_status                                              */
/*                                                                      */
/* Description:	SDLC status function handler                            */
/*									*/
/* Note:	This function runs on the off level			*/
/*                                                                      */
/* Function:	put status block information into ringq, then post      */
/*              the link manager function                               */
/*                                                                      */
/*                                                                      */
/* Input:	address of port control block                           */
/*		pointer to status block from device handler		*/
/*                                                                      */
/* Output:	copy of status block in exception ring queue            */
/*                                                                      */
/* Normal Exit:	return from call                                        */
/*                                                                      */
/* Error Exit:	none                                                    */
/*                                                                      */
/* Return Type:	void                                                    */
/*                                                                      */
/************************************************************************/

void	sdl_status(open_id, mpstatus)

ulong	open_id;			/* addr of port control block	*/
struct	status_block	*mpstatus;	/* pointer to status block	*/

{
	PORT_CB	*cb;
	ulong	rc;
	int rqlock;			/* defect 127690 */

	/* set up control block address	*/
	cb = (PORT_CB *) open_id;

/* defect 115819 */
/* defect 141966 */
        if /* terminating the kproc or it's already terminated */
	   ((cb->dlc.term_kproc) || (cb->port_cb_addr != cb))
/* end defect 141966 */
/* end defect 115819 */
		return;

	/* lock the ring queue - defect 127690 */
	rqlock = disable_lock (INTCLASS1, &(cb->dlc.ringq_lock)); /* 167068 */
	/* enqueue the status block	*/
	rc = dlc_rqput(cb->dlc.rcv_ringq, mpstatus);
	/* unlock the ring queue - defect 127690 */
	unlock_enable (rqlock, &(cb->dlc.ringq_lock));

	/* if the ringq put failed */
	if (rc)
	{
		cb->rc = rc;
/* <<< THREADS >>> */
		et_post(SDL_BAD_RQPUT_STATUS, cb->dlc.kproc_tid);
/* <<< end THREADS >>> */
	}
	else
	{
		/* post the SDLC link manager	*/
/* <<< THREADS >>> */
		et_post(SDL_RINGQ_POST, cb->dlc.kproc_tid);
/* <<< end THREADS >>> */
	}

}	/**** end of sdl_status *****************************************/

#ifdef MULT_PU

/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:        timer_handler                                           */
/*                                                                      */
/* Description:	decrement the count of each enabled timer               */
/*                                                                      */
/* Function:	This routine traverses the station list(s) dec-         */
/*              rementing each abort timer count that is marked         */
/*              as being enabled.  If this causes a timer count to go   */
/*		to zero (i.e. pop) then the approriate function will be	*/
/*		called.  This routine also processes the idle timer.	*/
/*                                                                      */
/* Input:	control block address                                   */
/*                                                                      */
/* Output:	timer count is updated                                  */
/*                                                                      */
/* Normal Exit:	return from call                                        */
/*                                                                      */
/* Error Exit:	none                                                    */
/*                                                                      */
/* Return Type:	void                                                    */
/*                                                                      */
/************************************************************************/

void	timer_handler(cb)

PORT_CB	*cb;				/* pointer to port cb		*/

{

	LINK_STATION	*station, *t_station, *ptr1;
	LINK_STATION    *ptr=cb->retry_list;
	int		rc;

	/* loop through the retry station list and process all retries */
	while (ptr != DLC_NULL) {
		ptr1 = ptr->retry_next;
	
		/*************************/
		/* check for XID retries */
		/*************************/
		if (ptr->x_m != DLC_NULL)
		{
			rc = (*ptr->cid->rcvx_fa)(ptr->x_m, &ptr->x_block,
						  ptr->cid);
			if (rc == DLC_FUNC_OK)
			{
				/* clear the retry func */
				ptr->x_m = DLC_NULL; 
				del_retry (cb,ptr);
			}

			else
			{
				cb->sense = rc;
				error_log(cb, ERRID_SDL0061,
						NON_ALERT, 0, FN, LN);
			}
		}

		/*****************************/
		/* check for I-frame retries */
		/*****************************/
		/* Note - doesn't retry the I-frame if the user has optioned
			  to reset local busy via DLC_EXIT_LBUSY ioctl    */
		if ((ptr->i_m != DLC_NULL) &&
			((ptr->rnr & USER_SET_LBUSY) == FALSE))
		{
			rc = (*ptr->cid->rcvi_fa)(ptr->i_m, &ptr->i_block,
						  ptr->cid);
			if (rc == DLC_FUNC_OK)
			{
				/* clear the retry func */
				ptr->i_m = DLC_NULL;
				ptr->rnr = FALSE;
				del_retry (cb,ptr);

			}

			else /* could not take the I-frame */
			{
				if (cb->rc == (ulong) DLC_FUNC_BUSY)
				{
					/* will wait for DLC_EXIT_LBUSY
					   ioctl from the user */

					/* set "user" local busy and
					   reset any "dlc" local busy */
                                        ptr->rnr = USER_SET_LBUSY;
				}

				else /* assume DLC_FUNC_RETRY */
				{
					/* Will retry the function every
					** 200 ms, and log a temporary
					** error every time the user func
					** fails
					*/

					/* add the station to the retry list
					   (assume already added) */

					/* set "dlc" local busy mode */
					ptr->rnr |= DLC_SET_LBUSY;
				}

				/* error log the retry failure */
				cb->sense = rc;
				error_log(cb, ERRID_SDL0062,
						NON_ALERT, 0, FN, LN);
			}

			/* if no more local busy conditions exist */
/* defect 96156 ix37135 */
			if (ptr->rnr == FALSE)
/* defect 96156 ix37135 */
			{
				/* clear the station local busy sub-state */
				ptr->sub_state &= (~DLC_LOCAL_BUSY);
			}
		}

		ptr = ptr1;  /* increment to next retry station in list */

	}

	/**********************************/
	/* check for network data retries */
	/**********************************/
	if (cb->n_m != DLC_NULL)
	{
		rc = (*cb->sap_cid->rcvn_fa)(cb->n_m, &cb->n_block,
					     cb->sap_cid);
		if (rc == DLC_FUNC_OK)
		{
			/* clear the retry func */
			cb->n_m = DLC_NULL;
		}

		else
		{
			/* error log the retry failure */
			cb->sense = rc;
			error_log(cb, ERRID_SDL0063,
					NON_ALERT, 0, FN, LN);
		}
	}

	/*
	** process repoll timers
	*/


	/* process the active link station */
	if (cb->active_ls)
	{
		/* if not operating in multi-pu mode */
		if (cb->flags.mpu_enabled == FALSE)
		{
			/* if the timer is enabled, then decrement the count
			   since the active station is not in the station
			   list anymore, it has to be done here */
			if (cb->active_ls->abort_timer.enabled)
			{
				--(cb->active_ls->abort_timer.ticks);

				/* if timer reached zero, then process it */
				if (cb->active_ls->abort_timer.ticks == 0)
				{
					DISABLE(cb->active_ls->abort_timer);
					abort_to(cb, cb->active_ls);
				}
			}
		}
 		/* if the timer is enabled, then decrement the count	*/
 		if (cb->repoll_timer.enabled)
 		{
 			--(cb->repoll_timer.ticks);
 
 			/* if the timer reached zero, then process it	*/
 			if (cb->repoll_timer.ticks == 0)
 			{
 				DISABLE(cb->repoll_timer);
 				repoll_to(cb, cb->active_ls);
 				cb->poll_seq_sw = TRUE;
 			}
 
 		}
	}

 	/*
 	** search through all the  stations
 	** looking for enabled abort timers
	** and multi-pu inactivity timers
 	*/
 
	/* if operating in multi-pu mode */
	if (cb->flags.mpu_enabled == TRUE)
	{
		/* process the list of secondary stations */
		station = cb->mpu_sta_list.head;
	}
	else
	{
/* defect 141966 */
		if (cb->station_type == PRIMARY)
		{
			/* process the active list */
			station = cb->active_list.head;
		}
		else /* non-mpu secondary */
		{
			station = 0;
		}
/* end defect 141966 */
	}
		
	while (station)
	{
		t_station = station;
		station = station->next;

		/* if the timer is enabled, then decrement the count	*/
		if (t_station->abort_timer.enabled)
		{
			--(t_station->abort_timer.ticks);

			/* if the timer reached zero, then process it	*/
			if (t_station->abort_timer.ticks == 0)
			{
				DISABLE(t_station->abort_timer);
				abort_to(cb, t_station);
			}

		}
		/* if mpu inactivity timer is enabled */
		if (t_station->inact_timer.enabled)
		{
			/* decrement the count */
			--(t_station->inact_timer.ticks);

			/* if the timer reached zero */
			if (t_station->inact_timer.ticks == 0)
			{
				DISABLE(t_station->inact_timer);
				cb->active_ls = t_station;
				rx_timeout(cb);
			}
		}
	}

	/*
	** process slow timer
	*/
	if (cb->slow_timer.enabled)
	{
		--(cb->slow_timer.ticks);

		if (cb->slow_timer.ticks == 0)
		{
			DISABLE(cb->slow_timer);
			cb->flags.slow_timer_popped = TRUE;
		}
	}

	/*
	** process idle timer
	*/
	if (cb->idle_timer.enabled)
	{
		--(cb->idle_timer.ticks);

		if (cb->idle_timer.ticks == 0)
		{
			DISABLE(cb->idle_timer);
			cb->flags.idle_timer_popped = TRUE;
		}
	}

}	/**** end of timer_handler ***************************************/

#endif



/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	sdl_waitq                                               */
/*                                                                      */
/* Description:	wait for data in device ringq                           */
/*                                                                      */
/* Function:	If there is data in the device ring queue, then the     */
/*              function returns immediatly and the qe passed in to     */
/*              the routine will contain valid data, the function will  */
/*		return SDL_DATA_AVAIL, if there is no data in the    	*/
/*		queue, then the function will go into the e_wait state	*/
/*		until the process is posted.  If this occurs, the 	*/
/*		post value is return by waitq.  If the post value   	*/
/*		indicates  received data or a status block, then the 	*/
/*		qe field will contain valid data otherwise the contents	*/
/*		of qe is not determined					*/
/*                                                                      */
/* Input:	address of device ring queue                            */
/*		address of an empty queue element			*/
/*                                                                      */
/* Output:	one of the following                                    */
/*			rc of SDL_DATA_AVAIL  and data from the queue	*/
/*			rc of SDL_RINGQ_POST  and data from the queue	*/
/*			some other non zero return code and no data	*/
/*                                                                      */
/* Normal Exit:	return from call                                        */
/*                                                                      */
/* Error Exit:	none                                                    */
/*                                                                      */
/* Return Type:	int                                                     */
/*                                                                      */
/************************************************************************/

int	sdl_waitq(cb, qaddr, qe)

PORT_CB	*cb;
ulong	qaddr;				/* address of ring queue	*/
struct	status_qe	*qe;		/* pointer to queue element	*/

{
	int	post_mask;	
	int	rc;
	int rqlock;			/* defect 127690 */

	/* lock the ring queue - defect 127690 */
	rqlock = disable_lock (INTCLASS1, &(cb->dlc.ringq_lock)); /* 167068 */
	rc = dlc_rqget(qaddr, qe);
	/* unlock the ring queue - defect 127690 */
	unlock_enable (rqlock, &(cb->dlc.ringq_lock));

	/*
	** if there is no data available
	*/
	if (rc == DLC_RINGQ_EMPTY)
	{
		performance_trace(cb, DLC_TRACE_WAITB, 0);
/* <<< THREADS >>>                                                      */
		/* then wait on et_post event    */
		post_mask = et_wait(WAIT_MASK, CLR_MASK, EVENT_SHORT);
/* <<< end THREADS >>>

		performance_trace(cb, DLC_TRACE_WAITE, 0);

		/* if post value indicates data available on ringq	*/
		if (post_mask & SDL_RINGQ_POST)
		{
			/* lock the ring queue - defect 127690 */
			rqlock = disable_lock (INTCLASS1,
                                     &(cb->dlc.ringq_lock));/* defect 167068 */
			rc = dlc_rqget(qaddr, qe);
			/* unlock the ring queue - defect 127690 */
			unlock_enable (rqlock, &(cb->dlc.ringq_lock));

			/* if there is a return code	*/
			if (rc)
			{
				/* clear RINQ POST BIT */
				post_mask &= (~SDL_RINGQ_POST);

				if (post_mask == (int)NULL)
				{
					qe = NULL;
					post_mask = IGNORE_RQ_POST;
				}
			}
		}
	}
	else
		/* data was available on the ring queue, so set the rc */
		post_mask = SDL_DATA_AVAIL;

	return(post_mask);

}	/**** end of sdl_waitq ******************************************/



/************************************************************************/
/*									*/
/* Function								*/
/* Name:	status handler    	    				*/
/*									*/
/* Description: process status blocks from the device handler		*/
/*									*/
/* Function:	The following are valid block types:                	*/
/*			Transmit completed				*/
/*			Device start completed				*/
/*			Device halt completed				*/
/*			Receive timeout					*/
/*			X.21 clear					*/
/*			Data Set Ready (DSR) signal lost		*/
/*			Ready for manual dial				*/
/*			Error threshold exceeded (not used)		*/
/*									*/
/*									*/
/* Input:	status block						*/
/*									*/
/* Output:	status block to the user       				*/
/*									*/
/* Normal Exit: return from call					*/
/*									*/
/* Error Exit:	none							*/
/*									*/
/* Return Type:	void							*/
/*									*/
/************************************************************************/

void			status_handler(cb, status)

PORT_CB			*cb;
struct	status_qe	*status;

{
	struct	dlc_getx_arg	st_block;
	struct	dlc_sape_res	*ext;

	LINK_STATION		*station;
	/* removed ulong *held_field; 103136 */
	ulong			result_ind;
	ulong			result_code;


	/* set ext to point to result_ext array at end of dlc_getx_arg	*/
	ext = (struct dlc_sape_res *) st_block.result_ext;

	station = cb->active_ls;

	if (cb->sdllc_trace)
		sdlmonitor(cb, PROC_STATUS_BLOCK, status->type,
				status->op[0], &status->op[1], 4);
	
	/* if physical link is closed and no CIO_ START has been issued */
	if ((cb->pl_status == CLOSED)
		&& (!cb->flags.phy_starting))
	{
		/* then the log the error */
		error_log(cb, ERRID_SDL0003, NON_ALERT, 0, FN, LN);
	}
	else
	{
		/* since link is not closed, accept the acknowledgment */

		switch (status->type)
		{

		/*
		** Transmit has completed
		*/
		case CIO_TX_DONE:

			if (status->op[0] == CIO_OK)
                        {
				/* 103136 - removed transmit ack completely   
				            except for primary I-frames with
				            p/f set. */

				/* Defect 122573 */
				if ((cb->active_ls != NULL) &&
				    (cb->station_type == PRIMARY))
				{
					cb->repoll_timer.ticks =
					  cb->active_ls->ll.sls.repoll_time;
					ENABLE(cb->repoll_timer);
				} /*End Defect 122573*/
			}
			else if (status->op[0] == MP_TX_FAILSAFE_TIMEOUT)
			{
				/* send halt command to the device */
				cb->sense = status->op[0];
				error_log(cb, ERRID_SDL8011, NON_ALERT,
						DLC_SAP_NT_COND, FN, LN);
			}
			else
			{
				/* not expecting write completion */
				error_log(cb, ERRID_SDL8055, NON_ALERT,
						0, FN, LN);
			}

			break;


		/*
		** Start port command has completed
		*/
		case CIO_START_DONE:

		    performance_trace(cb, DLC_TRACE_DHST, 0);

		    if (cb->pl_status == CLOSED)
		    {
			cb->flags.phy_starting = FALSE;

#ifdef MULT_PU
			/* CIO_START was successful */
			if (status->op[0] == CIO_OK)
			{
				result_code = DLC_SUCCESS;
				/* SAP will be enabled and set to OPENED */
				result_ind = DLC_SAPE_RES;
				ext->max_net_send = MAX_MBUF_SIZE;
				ext->lport_addr_len = 0;
				cb->pl_status = OPENED;

				/* fill in SAP status block */
				st_block.user_sap_corr =
					   cb->pl.esap.user_sap_corr;
				st_block.user_ls_corr  =
					   station->ll.sls.user_ls_corr;
				st_block.result_ind    = result_ind;
				st_block.result_code   = result_code;

				/* notify user of ENABLE SAP result */
				/* note: only notify the cid that did the
					 enable */
				(*cb->sap_cid->excp_fa)
						 (&st_block, cb->sap_cid);
			}
			else	/* CIO_START failed */
			{
				result_code = DLC_SAP_NT_COND;
				/* SAP will be disabled and set to CLOSING */
				result_ind = DLC_SAPD_RES;
				cb->pl_status = CLOSED;
				if (cb->num_opened)
					sdl_abort(cb);
				
				/*
				** if a halt command is pending, then 
				** clear the flag, since the start failed
				** there is no point in halting
				*/
				if (cb->flags.halt_pending)
					cb->flags.halt_pending = FALSE;

				/* fill in SAP status block */
				st_block.user_sap_corr =
					       cb->pl.esap.user_sap_corr;
				st_block.user_ls_corr  =
					       station->ll.sls.user_ls_corr;
				st_block.result_ind    = result_ind;
				st_block.result_code   = result_code;

				/* notify all channels active on this port
				   with the same exception */
				run_excepts(cb, &st_block);

				/* zero the sap owner channel id */
				cb->sap_cid = NULL;
			}
#endif
		    }
		    else
			error_log(cb, ERRID_SDL0004, NON_ALERT, 0, FN, LN);

		    /* if halt command is pending, then halt the device */
		    if (cb->flags.halt_pending)
		    {
			cb->flags.halt_pending = FALSE;
			pl_blowout(cb);
		    }

		    break;



		/*
		** Halt port has completed
		*/
		case CIO_HALT_DONE:

/* defect 141966 */
#ifdef MULT_PU
			/*
			** removed wakeup on writesleep
			*/
#endif
			/* removed close_device(cb) if sap_aborted */

			/* if halting the device */
			if (cb->pl_status == CLOSING)
			{
				/*
				** fill in SAP DISABLED status block
				*/
				st_block.user_sap_corr =
						cb->pl.esap.user_sap_corr;
				st_block.user_ls_corr  =
						station->ll.sls.user_ls_corr;
				st_block.result_ind    = DLC_SAPD_RES;
				st_block.result_code   = DLC_SUCCESS;
			
#ifdef MULT_PU
				/* notify all channels active on this port
				   with the same exception */
				run_excepts(cb, &st_block);

				/* zero the sap owner channel id */
				cb->sap_cid = NULL;
#endif

				/* Defect 84001 begin */
				sdl_abort(cb);

				/* reset any physical link starting flag */
				cb->flags.phy_starting = FALSE;
				/* Defect 84001 end */

				/* physical link is now closed */
				cb->pl_status = CLOSED;
			}
/* end defect 141966 */

			break;

		/*
		** Received an async status block from
		** the device handler
		*/
		case	CIO_ASYNC_STATUS:

			/* classify status	*/

			/*
			** if there was a receive timeout or an
			** auto response receive timeout
			*/

			switch (status->op[0])
			{

			/*
			** receive timer popped
			*/
			case	MP_RCV_TIMEOUT:
			
				if (cb->sdllc_trace)
					sdlmonitor(cb, RX_TIMEOUT,
							cb, station, 0, 0);

				if (cb->active_ls)
				/* process receive timeout */
				/* onlf if there is active ls */
					rx_timeout(cb);

				break;
			

			/*
			** receive timer popped while
			** in auto response mode
			*/
			case	MP_AR_RCV_TIMEOUT:


				/* turn off auto_response flag */
				cb->active_ls->auto_resp = FALSE;

				if (cb->sdllc_trace)
					sdlmonitor(cb, END_AUTO_RESP,
						0, 0, "RXTO", 4);

				/* process receive timeout */
				rx_timeout(cb);

				break;

			/*
			** X21 clear signal received
			*/
			case	MP_X21_CLEAR:

				/* same thing as DSR dropping */


			/*
			** Data Set Ready signal dropped
			*/
			case	MP_DSR_DROPPED:

				/* if the link is closing or closed	*/
				if (cb->pl_status == CLOSING ||
				     cb->pl_status == CLOSED)
				{
					/* then DSR drop expected */
				}

				else
				{
					error_log(cb, ERRID_SDL8043,
						NON_ALERT, 0, FN, LN);
	
#ifdef MULT_PU
					 /* added to take care of cleaning
					    stations on a DSR drop */
					sdl_abort(cb);
#endif
					/* close the physical link */
					pl_blowout(cb);

					/*
					** notify the user that the link
					** has closed
					*/
					st_block.user_sap_corr =
						cb->pl.esap.user_sap_corr;
#ifdef MULT_PU
					st_block.user_ls_corr = 0;
#endif
					st_block.result_ind = DLC_SAPD_RES;
					st_block.result_code =
							DLC_SAP_NT_COND;
#ifdef MULT_PU
					/* notify all channels active on
					   this port with the same
					   exception */
					run_excepts(cb, &st_block);

					/* zero the sap owner channel id */
					cb->sap_cid = NULL;
#endif
				}

				break;

			default:

				/* invalid async status, log error */
				cb->sense = status->op[0];
				error_log(cb, ERRID_SDL0065,
						NON_ALERT, 0, FN, LN);
				if (cb->sdllc_trace)
					sdlmonitor(cb, INVALID_ASYNC_OP,
						status->op[0], 0, 0, 0);
				break;

			}	/* async response switch */

			break;


		/*
		** adapter ready for user to 
		** manually dial the phone
		*/
		case	MP_RDY_FOR_MAN_DIAL:
			
			/* set up the status block	*/
			st_block.user_sap_corr = cb->pl.esap.user_sap_corr;
			st_block.user_ls_corr = station->ll.sls.user_ls_corr;
			st_block.result_ind = DLC_DIAL_RES;
			st_block.result_code = DLC_SUCCESS;

			/* send the status block to the user	*/
#ifdef MULT_PU
			(*cb->sap_cid->excp_fa)(&st_block, cb->sap_cid);
#endif

			break;

		/*
		** ADAPTER DETECTED ERRORS:
		**
		** The adapter logs an error for any problems
		** in this class and there is not much the DLC can do
		** with this information.  Since this is the case
		** when the mpqp device is started, the DLC will pass
		** thresholds of 0 to the adapter.  This will
		** prevent the MP_ERR_THRESHLD_EXC exception from occuring.
		** This case was included, however, for completeness
		*/

		case	MP_ERR_THRESHLD_EXC:
			break;

		case	MP_END_OF_AUTO_RESP:


			while (cb->wait_end_ar == EVENT_NULL)
				delay(1);

			if (cb->sdllc_trace)
				sdlmonitor(cb, END_AUTO_RESP,0,0,"DONE",4);

			station->auto_resp = FALSE;

			/* wake up the user process */
			e_wakeup((int *)&cb->wait_end_ar);

			break;
	
		default:
			
			/* unknown status block type, log an error	*/
			cb->sense = status->type;
			error_log(cb, ERRID_SDL0059, NON_ALERT, 0, FN, LN);

			if (cb->sdllc_trace)
				sdlmonitor(cb, INVALID_STATUS_BLOCK, 
						status->type, 0, "NVLD", 4);
		
		}	/* end switch	*/
	}

}	/**** end of status handler *************************************/



/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	rx_timeout                                              */
/*                                                                      */
/* Description:	adapter receive timeout handler                         */
/*                                                                      */
/* Function:	close the logical link if close option was chosen       */
/*		otherwise set the inactivity flag			*/
/*                                                                      */
/* Input:	pointer to port control block                           */
/*                                                                      */
/* Output:	updated poll lists                                      */
/*                                                                      */
/* Normal Exit:	return from call                                        */
/*                                                                      */
/* Error Exit:	none                                                    */
/*                                                                      */
/* Return Type: void                                                    */
/*                                                                      */
/************************************************************************/

void	rx_timeout(cb)

PORT_CB	*cb;

{
	int			rc;
	LINK_STATION		*station;

	station = cb->active_ls;

	if (cb->pl_status != OPENED)
		return;

	/* test for trace enabled on link station */
	if (station->ll.sls.flags & DLC_TRCO)
		timer_trace(cb, DLC_TO_REPOLL_T1);
    
	if (cb->station_type == PRIMARY)
	{
 	 /* since SDLC keeps the repoll timer */
 	 /* ignore this timeout from Device   */
         /* driver. It should never happen.   */

	}

	else	/* must be a secondary inactivity timeout */
	{
		/*
		** if the station pointer is NULL than the exception
		** arrived after the link station was closed 
		*/
		if (station)
		{
			inactivity_to(cb, station);
		}
		else
		{
       	    		/*
			** logical link is closed, ignore the
			** acknowledgment
			*/
		}
	}

}	/* end of rx_timeout ********************************************/



/************************************************************************/
/*                                                                      */
/* Name:	repoll_to                                               */
/*                                                                      */
/* Function:	primary receive timeout handler                         */
/*                                                                      */
/* Notes:	this function is called when the primary station has    */
/*		sent a frame to the secondary station, and there was	*/
/*		no response.  If the frame sent was a command or	*/
/*		an i frame, then it (they) will be retransmitted	*/
/*                                                                      */
/* Data									*/
/* Structures:	port control block                                      */
/*		link station control block				*/
/*                                                                      */
/* Returns:	void                                                    */
/*                                                                      */
/************************************************************************/

void		repoll_to(cb, station)

PORT_CB		*cb;		/* pointer to port control block	*/
LINK_STATION	*station;	/* pointer to link station control blk	*/

{

	struct	dlc_getx_arg	ex_block;	/* gdlc status block	*/
	int			rc;

	performance_trace(cb, DLC_TRACE_T1TO, 0);

	station->s_frame_ct = 0;
	station->last_sent = 0;

        /* Defect 160518 -- move from end of routine to beginning */
        /* increment the repoll counter */
        rc = update_counter(&station->ct.ttl_repolls_sent,
                            &station->repoll_count);

        /* has the total poll count reached the limit */
        if (rc == REACHED_LIMIT)
        {
                /* if repoll count exceeded the threshold */
                if ((station->repoll_count * 2) >= station->ll.sdl.prirpth)
                        error_log(cb, ERRID_SDL0014, NON_ALERT, 0, FN, LN);

                /* reset the counters */
                station->total_poll_count = 0;
                station->repoll_count = 0;

        }
        /* end Defect 160518 */

	/* increment contiguous repoll counter */
	++(station->ct.contig_repolls_sent);

	/* if test response pending */
	if (station->unnum_rsp & TEST_PENDING)
	{
		station->unnum_cmd &= (~TEST_PENDING);
		station->unnum_rsp &= (~TEST_PENDING);

		/* set up exception block	*/
		ex_block.user_sap_corr = cb->pl.esap.user_sap_corr;
		ex_block.user_ls_corr = station->ll.sls.user_ls_corr;
		ex_block.result_ind = DLC_TEST_RES;
		ex_block.result_code = DLC_INACT_TO;

		/* call user's exception handler */
#ifdef MULT_PU
		(*station->cid->excp_fa)(&ex_block, station->cid);
#endif

/* defect 162707 */
		/* Free and reset the station's saved test buffer.      */
		/* Note: The best approach would probably be to let     */
		/* mpqp free the test buffers since dlc only sends them */
		/* once.  I'm leaving the test buffer stored in test.m  */
		/* however, in case we have to transmit it more than    */
		/* once in the future.                                  */

                m_freem(station->test.m);
                station->test.m = NULL;
/* end defect 162707 */

		/*
		** move station to bottom of
		** active list
		*/

		/* put the station in the proper list */
		if (station->mode == NRM)
			add_ls(cb, &cb->active_list, station);
		else
			add_ls(cb, &cb->quiesce_list, station);
	}
	else	/* test response not pending */
	{
                /* put station_mode to appropriate mode */
		if (station->poll_mode != IDLE)
		{
        		if (station->poll_mode == SLOW)
			{
 				--(cb->slow_count);
				station->poll_mode = ACTIVE;
			}  

			/*
			** if the primary contiguous repoll 
			** count exceeds the limit specified in
			** the configuration then call
			** repoll_exceeds
			*/
			if (station->ct.contig_repolls_sent ==
				station->ll.sls.max_repoll)
			{
				repoll_exceeds(cb, station);
			}
			else	/* repoll does not exceed config */
			{
				/*
				** move station to bottom of active list
				*/
				add_ls(cb, &cb->active_list, station);

	 			/* get set for next transmition */
				if (station->unnum_rsp)
				{
					station->unnum_cmd
						|= station->unnum_rsp;
					station->unnum_rsp = FALSE;
				}

				/* set poll only mode */
				else
					station->poll_only = TRUE;

				/* send DISC only once */
				if (station->unnum_cmd & DISC_PENDING)
				station->ct.contig_repolls_sent =
					station->ll.sls.max_repoll - 1;
			}
		 }
		 else  /*it is in idle list */
			{	
				if (station->unnum_rsp)
				{
					station->unnum_cmd
						|= station->unnum_rsp;
					station->unnum_rsp = FALSE;
				}
				/* move station to bottom of active list*/
				add_ls(cb, &cb->active_list, station);
				cb->poll_seq_sw = TRUE;
			}
	}


}	/* end of repoll_to *********************************************/



/************************************************************************/
/*                                                                      */
/* Name:	inactivity_to                                           */
/*                                                                      */
/* Function:	secondary station inactivity timeout handler            */
/*                                                                      */
/* Notes:	this function is called when the device handler		*/
/*		notifies the device driver that it's receive timer has	*/
/*		expired and the link station is secondary.  If the user	*/
/*		has requested inactivity without termination, then this	*/
/*		function notifies the user of the timeout, but does not	*/
/*		close the link station.  If terminate on inactivity has	*/
/*		been selected, then the link station will be closed	*/
/*                                                                      */
/* Data									*/
/* Structures:	port control block					*/
/*		link station control block				*/
/*                                                                      */
/* Returns:	void                                                    */
/*                                                                      */
/************************************************************************/

void		inactivity_to(cb, station)

PORT_CB		*cb;		/* pointer to port control block	*/
LINK_STATION	*station;	/* pointer to link station control blk	*/
{

	struct	dlc_getx_arg	ex_block;

	performance_trace(cb, DLC_TRACE_T3TO, 0);

	timer_trace(cb, DLC_TO_INACT);

        /* error log the inactivity timeout */
	error_log(cb, ERRID_SDL800C, ALERT, 0, FN, LN);
    
        /* increment the secondary inactivity rcvd RAS count */
        INC_COUNTER(station->ct.sec_inact_to, MAX_COUNT);

        /* 
        ** check for: termination option NOT set, or
        **            waiting for a first receive, but 
        **            NOT in negotiable mode
        */                                   

        if ((station->ll.sls.flags & DLC_SLS_HOLD)
               || (station->rec_first_pending)
               && (!(station->ll.sls.flags & (~DLC_SLS_NEGO))))
        {
                ex_block.user_sap_corr = cb->pl.esap.user_sap_corr;
                ex_block.user_ls_corr = station->ll.sls.user_ls_corr;
                ex_block.result_ind = DLC_IWOT_RES;
                ex_block.result_code = DLC_INACT_TO;

                /*
                ** set idle poll mode
                */
		station->poll_mode = IDLE;
		++(cb->idle_count);

                station->inact_pending = TRUE;
        }
        else    /* terminate this session */
        {
                /* cancel abort timer */
                DISABLE(station->abort_timer);
                station->abort_running = FALSE;

                /* clear all the buffers */
#ifdef MULT_PU
		/*
		** set up exception block
		*/
		ex_block.user_sap_corr = cb->pl.esap.user_sap_corr;
		ex_block.user_ls_corr = station->ll.sls.user_ls_corr;
		ex_block.result_ind = DLC_STAH_RES;
		ex_block.result_code = DLC_INACT_TO;

                (*station->cid->excp_fa)(&ex_block, station->cid);
#endif

                if (station->ll.sls.flags & DLC_TRCO)
                {
                        session_trace(cb, TRACE_CLOSE);
                }

#ifdef MULT_PU
		mpu_remove_station (cb, station);
#endif
		/*
		** removed free active_ls
		*/
		cb->active_ls = NULL;
        }

#ifdef MULT_PU
	/*
	** removed call to exception routine
	*/
#endif
    
}	/* end of inactivity_to *****************************************/




/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	repoll_exceeds                                          */
/*                                                                      */
/* Description:	repoll count exceeds limit                              */
/*                                                                      */
/* Function:	This function is called when the number of contig       */
/*              repolls exceeds some predetermined value.  At this      */
/*              point, the link station is no longer considered active  */
/*		and it is placed on the idle list			*/
/*                                                                      */
/* Input:	pointer to port control block                           */
/*		pointer to station control block			*/
/*                                                                      */
/* Output:	updated poll lists                                      */
/*                                                                      */
/* Normal Exit:	return from call                                        */
/*                                                                      */
/* Error Exit:	none                                                    */
/*                                                                      */
/* Return Type:	void                                                    */
/*                                                                      */
/************************************************************************/

void	repoll_exceeds(cb, station)

PORT_CB		*cb;			/* pointer to port ctl block	*/
LINK_STATION	*station;		/* pointer to station ctl block	*/

{
	struct	dlc_getx_arg	ex_block;

	error_log(cb, ERRID_SDL8001, ALERT, 0, FN, LN);

	/*
	** check for:  termination option is NOT set or 
	**		   waiting for the first receive and
	**             there is no disconnect response pending
	*/
	if (((station->ll.sls.flags & DLC_SLS_HOLD)
		|| station->rec_first_pending)
		&& (!(station->unnum_rsp & DISC_PENDING)))
	{

		/* add this instance to idle poll list */
		add_ls(cb, &cb->active_list, station);

		/* if no stations are in idle poll mode */
		if (cb->idle_count == 0 && cb->idle_timer.enabled == FALSE)
		{
	    		/* then start the idle poll timer */
			SETTIMER(cb->idle_timer, cb->idle_ticks);
			ENABLE(cb->idle_timer);
		}
		station->poll_mode = IDLE;
		++(cb->idle_count);

		/* set inactivity pending flag	*/
		station->inact_pending = TRUE;

		/* get set for next transmition */
		if (station->unnum_rsp)
		{
			station->unnum_cmd |= station->unnum_rsp;
			station->unnum_rsp = FALSE;
		}
		else
			/* set poll only mode */
			station->poll_only = TRUE;

		ex_block.user_sap_corr = cb->pl.esap.user_sap_corr;
		ex_block.user_ls_corr = station->ll.sls.user_ls_corr;
		ex_block.result_ind = DLC_IWOT_RES;
		ex_block.result_code = DLC_INACT_TO;

		/* notify user of exception */
#ifdef MULT_PU
		(*station->cid->excp_fa)(&ex_block, station->cid);
#endif
	}

	else	/* start the close logical link procedures */
	{
		/* see if disc was the last command sent */
		if (station->unnum_rsp & DISC_PENDING)
		{
	    		/* close logical link immediately */
			DISABLE(station->abort_timer);
	    		station->abort_running = FALSE;

	    		/* close the llc and clear all transmit buffers */
	    		clear_buffer(cb, station);

                        /* Defect 160518 -- removed Defect 126638 change */

	    		/* if link trace enabled, trace close cmd */
	    		if (station->ll.sls.flags & DLC_TRCO)
				session_trace(cb, TRACE_CLOSE);

			ex_block.user_sap_corr = cb->pl.esap.user_sap_corr;
			ex_block.user_ls_corr = station->ll.sls.user_ls_corr;
			ex_block.result_ind = DLC_STAH_RES;
			ex_block.result_code = DLC_INACT_TO;

			/* notify user of exception */
#ifdef MULT_PU
			(*station->cid->excp_fa)(&ex_block, station->cid);
#endif
                        free(station); /* Defect 160518 */
		}
		else	/* send disconnect command */
		{
    			station->unnum_cmd |= DISC_PENDING;
        			station->disc_reason = DLC_INACT_TO;

    			/*
    			** Set the contiguous repoll count to one less than the
    			** count allowed by the operator.  This will ensure that
    			** only one disconnect will be sent. 
    			** Regardless of the remote response, the link will
    			** close and write an entry into the error log
    			*/
			station->ct.contig_repolls_sent =
					station->ll.sls.max_repoll - 1;

    			/*
			** move this instance to the bottom of the
			** active poll list
			*/
			add_ls(cb, &cb->active_list, station);
		}
	}

}	/* end of repoll_exceeds ****************************************/



/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	abort_to                                            */
/*                                                                      */
/* Description:	abort timer popped                                      */
/*                                                                      */
/* Function:	if the link station is primary, then send out a DISC    */
/*              command and put the station on the active list, if the  */
/*              link station is secondary, then just close the station  */
/*		and notify the user					*/
/*                                                                      */
/* Input:	pointer to the port control block                       */
/*		pointer to the station control block			*/
/*                                                                      */
/* Output:	updated station list                                    */
/*                                                                      */
/* Normal Exit:	return from call                                        */
/*                                                                      */
/* Error Exit:	none                                                    */
/*                                                                      */
/* Return Type:	void                                                    */
/*                                                                      */
/************************************************************************/

void		abort_to(cb, station)

PORT_CB		*cb;
LINK_STATION	*station;

{
	struct	dlc_getx_arg	ex_block;	/* exception block	*/

	performance_trace(cb, DLC_TRACE_T3ABORT, 0);

	/* if link trace is enabled	*/
	if (station->ll.sls.flags & DLC_TRCO)
		timer_trace(cb, DLC_TO_ABORT);

	/* if this timeout is still in effect	*/
	if ((station->abort_cancel == FALSE)
		&& (station->abort_running)
		&& (station->ll_status != CLOSED))
	{
		/*
		** then process the timeout
		*/

		if (cb->station_type == PRIMARY)
		{

			/* set the disconnect reasons	*/
			station->disc_reason = DLC_DISC_TO;
			station->unnum_cmd |= DISC_PENDING; 

			/*
			** set the contiguous repoll count to one less than the 
			** maximum to ensure only one DISC is sent
			*/
			station->ct.contig_repolls_sent =
					station->ll.sls.max_repoll - 1;
			station->ll_status = CLOSING;

			/* if station is in idle poll mode */
			if (station->poll_mode == IDLE)
			{
				station->poll_mode = ACTIVE;
				--(cb->idle_count);
			}

		}
		else	/* it is secondary */
		{

			cb->active_ls = NULL;
                        /* Defect 160518 -- don't reset station_type */
                        /* cb->station_type = 0; */

			if (station->ll.sls.flags & DLC_TRCO)
				session_trace(cb, TRACE_CLOSE);

			ex_block.user_sap_corr = cb->pl.esap.user_sap_corr;
			ex_block.user_ls_corr = station->ll.sls.user_ls_corr;
			ex_block.result_ind = DLC_STAH_RES;
			ex_block.result_code = DLC_DISC_TO;

			/* send exception to user */
#ifdef MULT_PU
			(*station->cid->excp_fa)(&ex_block,station->cid);

			/*
			** remove free station
			*/
			mpu_remove_station (cb, station);
#endif
		}
	}
	else	/* reset abort flags and ignore timeout	*/
	{
		station->abort_cancel = FALSE;
		station->abort_running = FALSE;
	}

}	/**** end of abort_to ***************************************/



/************************************************************************/
/*                                                                      */
/* Name:	process_station                                         */
/*                                                                      */
/* Function:	determine if a link station is eligible to transmit     */
/*                                                                      */
/* Notes:	When a slow poll station comes to the top of the active */
/*		list and the slow timer has not popped, then it should	*/
/*		be returned to the bottom of the list.  If the slow	*/
/*		timer has popped, and this is the first slow poll    	*/
/*		station to be processed since the timer popped, then 	*/
/*		the station address is saved.  When this station gets	*/
/*		to the top of the list again all slow poll stations 	*/
/*		have been processed so the timer can be reset and the	*/
/*		slow poll stations will be ignored until the next timer	*/
/*		pop.							*/
/*									*/
/*		This same scenario is true for the idle timer		*/
/*                                                                      */
/* Data									*/
/* Structures:	sdlc port control block                                 */
/*		sdlc station control block				*/
/*                                                                      */
/* Returns:	void                                                    */
/*                                                                      */
/************************************************************************/

void		process_station(cb, station)

PORT_CB		*cb;
LINK_STATION	*station;

{

	sdlmonitor(cb, "PROS", cb->active_ls, cb->poll_seq_sw, 0, 0);
	sdlmonitor(cb, "PR1S", station->poll_mode, 0, 0, 0);
	sdlmonitor(cb, "PR2S", cb->slow_marker, cb->idle_marker, 0, 0);
	/* if the current station is active and both list markers are null */
	if ((station->poll_mode == ACTIVE)
		&& (!cb->slow_marker)
		&& (!cb->idle_marker))
	{
	sdlmonitor(cb, "PRO1", cb->active_ls, cb->poll_seq_sw, 0, 0);
		/* then process the station */
		pri_transmit(cb, station);
		return;
	}

	/*
	** if the slow marker is pointing to the current station
	** then all the slow poll stations have been processed
	*/
	if (cb->slow_marker == station)
	{
	sdlmonitor(cb, "PRO2", cb->active_ls, cb->poll_seq_sw, 0, 0);
		/* clear slow_timer_popped flag */
		cb->flags.slow_timer_popped = FALSE;

		/* clear the slow_marker */
		cb->slow_marker = NULL;

		/* if there are any stations in slow mode */
		if (cb->slow_count)
		{
	sdlmonitor(cb, "PRO3", cb->active_ls, cb->poll_seq_sw, 0, 0);
			SETTIMER(cb->slow_timer, cb->slow_ticks);
			ENABLE(cb->slow_timer);
		}
	}
	else if ((cb->slow_marker == NULL)
			&& cb->flags.slow_timer_popped
			&& station->poll_mode == SLOW)

		cb->slow_marker = station;

	/*
	** if the idle marker is pointing to the current station
	** then all the idle poll stations have been processed
	*/
	if (cb->idle_marker == station)
	{
	sdlmonitor(cb, "PRO4", cb->active_ls, cb->poll_seq_sw, 0, 0);
		/* clear idle_timer_popped flag */
		cb->flags.idle_timer_popped = FALSE;

		/* clear the idle_marker */
		cb->idle_marker = NULL;

		/* if there are any stations in idle mode */
		if (cb->idle_count)
		{
			SETTIMER(cb->idle_timer, cb->idle_ticks);
			ENABLE(cb->idle_timer);
		}
	}
	else if ((cb->idle_marker == NULL)
			&& cb->flags.idle_timer_popped
			&& station->poll_mode == IDLE)

		cb->idle_marker = station;


	/* if there is a DISC command pending */
	if (station->unnum_cmd & DISC_PENDING)
	{
	sdlmonitor(cb, "PRO5", cb->active_ls, cb->poll_seq_sw, 0, 0);
		/* then make station active */

		if (station->poll_mode == SLOW)
			--(cb->slow_count);
		else if (station->poll_mode == IDLE)
			--(cb->idle_count);

		station->poll_mode = ACTIVE;
	}

	if (station->poll_mode == ACTIVE)
{
	sdlmonitor(cb, "PRO6", cb->active_ls, cb->poll_seq_sw, 0, 0);
		pri_transmit(cb, station);
}

	else if (station->poll_mode == SLOW && cb->flags.slow_timer_popped)
{
	sdlmonitor(cb, "PRO7", cb->active_ls, cb->poll_seq_sw, 0, 0);
		pri_transmit(cb, station);
}

	else if (station->poll_mode == IDLE && cb->flags.idle_timer_popped)
{
	sdlmonitor(cb, "PRO8", cb->active_ls, cb->poll_seq_sw, 0, 0);
		pri_transmit(cb, station);
}

	else
	{
	sdlmonitor(cb, "PRO9", cb->active_ls, cb->poll_seq_sw, 0, 0);
		/*
		** the top station is either in SLOW or IDLE mode
		** and the appropriate timer has not expired
		** so go to the next station
		*/
		add_ls(cb, &cb->active_list, station);
		cb->poll_seq_sw = TRUE;
	}

}	/**** end of process_station ********************************/
