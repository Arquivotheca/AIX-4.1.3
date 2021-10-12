static char sccsid[] = "@(#)79	1.35  src/bos/kernext/dlc/sdl/sdlioctl.c, sysxdlcs, bos41B, 412_41B_sync 12/3/94 12:25:56";
/*
 * COMPONENT_NAME: (SYSXDLCS) SDLC Data Link Control
 *
 * FUNCTIONS: 
 *	pr_ioctl()
 *	start_ls()
 *	valid_sls()
 *	halt_ls()
 *	sdl_contact()
 *	sdl_alter()
 *	enable_sap()
 *	disable_sap()
 *	sdl_test()
 *	query_sap()
 *	query_ls()
 *	sdl_trace()
 *	enter_lbusy()
 *	exit_lbusy()
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
**      File Name      : 79
**
**      Version Number : 1.28
**      Date Created   : 92/12/15
**      Time Created   : 11:33:20
*/


#include "sdlc.h"
#include <errno.h>


/* defect 126815 */
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
/* end defect 126815 */


#define	TRACE_SIZE	sizeof(struct dlc_trace_arg)
#define CONFIG_BYTES	sizeof(struct logical_link)
#define HOW_IS_LINE     1

/************************************************************************/
/*									*/
/*	i/o control functions						*/
/*									*/
/************************************************************************/
				/* functions delcared in this module	*/

int	pr_ioctl();		/* process type of ioctl		*/
int	start_ls();		/* start an sdlc link station		*/
int	valid_sls();		/* is start link station call valid	*/
int	halt_ls();		/* halt an sdlc link station		*/
int	sdl_contact();		/* contact a remote link station	*/
int	sdl_alter();		/* alter link station operating params	*/
int	enable_sap();		/* start the physical link		*/
int	disable_sap();		/* halt the physical link		*/
int	sdl_test();		/* send a test command to the remote	*/
int	query_sap();		/* query the sap for operating stats	*/
int	query_ls();		/* query the link station for stats	*/
int	sdl_trace();		/* turn on/off the sdlc link trace	*/
int	enter_lbusy();		/* enter local busy mode		*/
int	exit_lbusy();		/* exit local busy mode			*/

/**** end of module wide declarations ***********************************/



/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	pr_ioctl                                                */
/*                                                                      */
/* Description:	process the ioctl command                               */
/*                                                                      */
/* Function:	Calls the approriate subroutine to process the          */
/*              ioctl command.                                          */
/*                                                                      */
/* Input:	dd_ioctl parameter                                      */
/*                                                                      */
/* Output:	selected device options                                 */
/*                                                                      */
/* Normal Exit:	return from call                                        */
/*                                                                      */
/* Error Exit:	none                                                    */
/*                                                                      */
/* Called by:	sdlc head                                               */
/*                                                                      */
/* Return Type:	int 		                                        */
/*                                                                      */
/************************************************************************/

int			pr_ioctl(dev, op, arg, flag, mpx)

ulong			dev, op, flag;
caddr_t			arg;
struct	dlc_chan	*mpx;

{

	int		status;		/* command status	*/
	PORT_CB		*cb;		/* port control block	*/
	int		dont_unlock;
	int		rc;
        struct          devinfo sdlinfo;

	cb = (PORT_CB *) mpx->cb;


/* defect 126815 */
  simple_lock(&cb->dlc.lock);
/* end defect 126815 */

	
	/* write monitor trace record */
	if (cb->sdllc_trace)
		sdlmonitor(cb, PR_IOCTL, op, cb->pl_status, 0, 0);


	switch (op)
	{

	case DLC_ENABLE_SAP:

#ifdef MULT_PU
		status = enable_sap(cb, mpx, arg);
#endif

		break;

	case DLC_DISABLE_SAP:

#ifdef MULT_PU
		status = disable_sap(cb, mpx, arg);
#endif
		break;

	case DLC_START_LS:

#ifdef MULT_PU
		status = start_ls(cb, mpx, arg);
#endif
		break;

	case DLC_HALT_LS:

#ifdef MULT_PU
		status = halt_ls(cb, mpx, arg);
#endif
		break;

	case DLC_ALTER:

#ifdef MULT_PU
		status = sdl_alter(cb, mpx, arg);
#endif
		break;

	case DLC_TRACE:

#ifdef MULT_PU
		status = sdl_trace(cb, mpx, arg);
#endif
		break;

	case DLC_TEST:

#ifdef MULT_PU
		status = sdl_test(cb, mpx, arg);
#endif
		break;

	case DLC_CONTACT:

#ifdef MULT_PU
		status = sdl_contact(cb, mpx, arg);
#endif
		break;

	case DLC_QUERY_SAP:

#ifdef MULT_PU
		status = query_sap(cb, mpx, arg);
#endif
		break;

	case DLC_QUERY_LS:

#ifdef MULT_PU
		status = query_ls(cb, mpx, arg);
#endif
		break;

	case DLC_ENTER_LBUSY:

#ifdef MULT_PU
		status = enter_lbusy(cb, mpx, arg);
#endif
		break;

	case DLC_EXIT_LBUSY:

#ifdef MULT_PU
		status = exit_lbusy(cb, mpx, arg);
#endif
		break;

	case IOCINFO:
	         
	 /**********************************************************/
         /* load the device class                                  */
 	 /**********************************************************/

   		 sdlinfo.devtype = DD_DLC;

	  /**********************************************************/
	  /* clear the flags                                        */
 	  /**********************************************************/

 		 sdlinfo.flags = 0;

 	 /**********************************************************/
      	 /* load the device subclass                               */
         /**********************************************************/

		sdlinfo.devsubtype = DS_DLCSDLC;

	 /**********************************************************/
         /* write out the iocinfo to the user's space              */
         /**********************************************************/

 		 if                         /* kernel space        */
 		 (mpx->state == KERN)
		    bcopy(&sdlinfo, arg, 3);
	         else                       /* application space   */
 		   copyout(&sdlinfo, arg, 3);
                 return (0);
                             /* end get_iocinfo             */

		break;


	default:

		/* invalid command */
		cb->sense = op;
		error_log(cb, ERRID_SDL0066, NON_ALERT, 0, FN, LN);
		status = EINVAL;
		break;
	}		/* end switch */



/* defect 126815 */
  simple_unlock(&cb->dlc.lock);
/* end defect 126815 */

	

	return(status);

}	/**** end of pr_ioctl *******************************************/



/************************************************************************/
/*									*/
/* Function								*/
/* Name:	start_ls						*/
/*									*/
/* Description: ioctl DLC_START_LS command handler			*/
/*									*/
/* Function:	check for valid user link correlator			*/
/*		check session limit					*/
/*		copy configuration data 				*/
/*		return GDLC link correlator				*/
/*		initialize station control block			*/
/*									*/
/* Input:	dlc_sls_arg ioctl structure				*/
/*									*/
/* Output:	cio_start command to the device handler 		*/
/*		initialized station storage				*/
/*									*/
/* Normal Exit: return from call					*/
/*									*/
/* Error Exit:	none							*/
/*									*/
/* Called by:	sdlc_link_station					*/
/*									*/
/* Return Type: int							*/
/*									*/
/************************************************************************/

#ifdef MULT_PU
int	start_ls(cb, mpx, arg)

PORT_CB	*cb;
struct dlc_chan *mpx;
ulong	arg;

#endif

{					

	struct dlc_getx_arg	st_block;
	struct dlc_stas_res	*ext;

	struct		logical_link	config;
	struct		dlc_getx_arg	ex_block;
    struct		T_CHG_PARMS	new_parm;

	int		valid;
	int		rc;
	int		result_code;

	LINK_STATION	*station;



	/* preset error codes	*/
	cb->rc = NORMAL;
	result_code = DLC_LS_NT_COND;


	/* set ext to point to array at end of st_block */
	ext = (struct dlc_stas_res *) st_block.result_ext;

	/* copy user data into kernel space	*/

#ifdef MULT_PU
	if (mpx->state & KERN)
#endif
		bcopy(arg, &config, CONFIG_BYTES);
	else
	{
		cb->rc = copyin(arg, &config, CONFIG_BYTES);

		if (cb->rc)
		{
	    		error_log(cb, ERRID_SDL8066, NON_ALERT, 0, FN, LN);
	    		return(EIO);
		}
	}

#ifdef MULT_PU
	/*  check if in mpu mode. If so, do not let a station be opened
		as primary or negotiable */
	if ( (cb->flags.mpu_enabled) && ((config.sls.flags & DLC_SLS_NEGO) ||
		 (config.sls.flags & DLC_SLS_STAT)) ) {
		error_log(cb, ERRID_SDL806B, NON_ALERT, 0, FN, LN);
		return (EINVAL);
	}
#endif

	valid = valid_sls(cb, &config);

	if (valid)
	{
		station = (LINK_STATION *) create_ls();
		if (station == NULL)
		{
			error_log(cb, ERRID_SDL8065, NON_ALERT, 0, FN, LN);
			valid = FALSE;
			cb->rc = ENOMEM;
		}
		else
		{
			/*
			** space for the link station has been allocated
			** return the address as the link station correlator
			*/
#ifdef MULT_PU

			/* initialize the cid value */
                        /* don't initialize any elements to zero here
                           because a bzero was done in create_ls() */
			station->cid = mpx;

#endif
		}
	}

	if (cb->sdllc_trace)
#ifdef MULT_PU
		sdlmonitor(cb, START_LINK_STATION,
			   station->ll.sls.gdlc_ls_corr, cb->rc, 0, 0);
#endif

	/* if link trace is enabled	*/
	if (station->ll.sls.flags & DLC_TRCO)
		session_trace(cb, TRACE_OPEN);


	/* if all the checks passed (ie valid) then open the link	*/
	if (valid)
	{
		/* 
		** copy device characteristics from configuration to
		** station storage.
		*/
		bcopy(&config, &station->ll.sls, CONFIG_BYTES);

		/* If link trace is being enabled then set it up	*/
		if (station->ll.sls.flags & DLC_TRCO)
		{

			/* See if LONG TRACE bit is set */
			if (station->ll.sls.flags & DLC_TRCL)
			{
				/* set max trace size to 224 bytes	*/
				station->max_trace_size = TRACE_LONG_SIZE;
			}
			else	/* tracing short entries	*/
			{
				/* set max trace size to 80 bytes	*/
				station->max_trace_size = TRACE_SHORT_SIZE;
			}
		}

		/* 
		** if the available data size in a buffer is less than the
		** user requested, then set maxif (max i_frame) to the smaller
		** value 
		*/
		if (station->ll.sls.maxif > MAX_I_FRAME)
			station->ll.sls.maxif = MAX_I_FRAME;

		/*
		** respond according to station type
		*/

		/* configured to negotiate	*/
		if (station->ll.sls.flags & DLC_SLS_NEGO)
		{
			/* set remote station address to BROADCAST	*/
			config.sls.len_raddr_name = 1;
			config.sls.raddr_name[0] = BROADCAST;
		        station->ll.sls.len_raddr_name = 1; 
 		        station->ll.sls.raddr_name[0] = BROADCAST;


			/* if negotiable is set, primary flag must be set */
			station->ll.sls.flags |= DLC_SLS_STAT;
		}
			
		/* if station is not primary	*/
		if (!(station->ll.sls.flags & DLC_SLS_STAT))
		{
			/* then need to open as secondary	*/

#ifdef MULT_PU
			/* add it to the station poll list	*/
			add_sta_to_list(cb, &(cb->mpu_sta_list), station);
			/* enter the address of the new link station into
			   the link_station_array */
			cb->link_station_array[config.sdl.secladd] = station;
			/* Return the secondary station address as the
			   link correlator */
			config.sls.gdlc_ls_corr = (ulong) config.sdl.secladd;
#endif
			/* initialize logical link	*/
			cb->active_ls = station;
			cb->flags.no_active = FALSE;
			cb->poll_seq_sw = FALSE;
			cb->station_type = SECONDARY;
			station->mode = NDM;
			cb->saved_inact_to = station->ll.sls.inact_time;
		}
		else	/* need to open as primary	*/
		{
			/*
			** initialize logical link
			*/
			
			/* add it to the quiesce poll list	*/
			add_ls(cb, &cb->quiesce_list, station);

#ifdef MULT_PU
			cb->link_station_array[config.sls.raddr_name[0]] =
								    station;
			/* return remote station address as link correlator */
			config.sls.gdlc_ls_corr =
					   (ulong) config.sls.raddr_name[0];
#endif
			cb->station_type = PRIMARY;
			station->mode = QUIESCE;
			station->unnum_cmd = FALSE;
			station->s_frame_ct = 0;
			station->saved_repoll = station->ll.sls.repoll_time;

			/* if the link is not multi-point	*/
			if (!(cb->pl.esap.flags & DLC_ESAP_LINK))
			{
				/*
				** set flag to ignore incoming frames before
				** the link starts the first transmit
				*/
				cb->flags.ignore = TRUE;
				cb->flags.no_active = TRUE;
				cb->poll_seq_sw = TRUE;
			}
		}

		/*
		** Initialize Logical Link
		*/

		/* reset internal counters	*/
		station->nr = 0;
		station->ns = 0;
		station->ack_nr = 0;
		station->tx_ns = 0;
		station->saved_ns = 7;
		station->abort_cancel = 0;
		station->abort_running = 0;
		station->poll_only = 0;
		station->unnum_rsp = FALSE;
		station->unnum_cmd = FALSE;
		station->frmr_response_pending = FALSE;
		station->inact_pending = FALSE;
		station->ack_pending = FALSE;
		station->rnr = FALSE;
		station->xid.m = NULL;
		station->test.m = NULL;
		station->poll_mode = ACTIVE;

		/* set logical link status to open	*/
		station->ll_status = OPENED;
		++(cb->num_opened);
		station->retransmission_flag = FALSE;
		station->repoll_count = 0;
		station->total_poll_count = 0;
		station->burst_rexmitted = 0;
		station->total_burst_xmitted = 0;
		station->conn_pending = FALSE;
		station->disc_reason = 0;
		station->sub_state = 0;
		station->last_sent = 0;
		station->last_rcvd = 0;

		/*
		** forced timeout and idle timeout are specified
		** in 1 second intervals.  Since there are ten
		** ticks in a second, multiply the number of ticks
		** by ten
		*/
#ifdef MULT_PU
		station->abort_ticks = station->ll.sls.force_time * 10;
		station->inact_ticks = station->ll.sls.inact_time * 10;
#endif
		cb->idle_ticks = station->ll.sdl.priilto * 10;
		cb->slow_ticks = station->ll.sdl.prislto * 10;

		/* if we are running on a leased line do the following	*/
		if (cb->pl.esap.flags & DLC_ESAP_NTWK)
			/* set receive_first_pending flag	*/
			station->rec_first_pending = TRUE;
		else
			/* clear receive_first_pending flag	*/
			station->rec_first_pending = FALSE;

		/*
		** initialize RAS counters
		*/
		bzero(&station->ct, sizeof(struct counters));

		/*
		** send new parameters to adapter
		*/
#ifdef MULT_PU
		/* if the station type is primary, or
		   the port is operating in multi-PU mode */
		if ((cb->station_type == PRIMARY) ||
		    (cb->flags.mpu_enabled == TRUE))
#endif
		{
			/* do not let device driver do repoll timer */
			new_parm.rcv_timer = 0;
			new_parm.chg_mask = CP_RCV_TMR; 
		}
		else	/* station is secondary	*/
		{
			new_parm.chg_mask = (CP_RCV_TMR | CP_POLL_ADDR);
			/* tell adapter to select only this address	*/
			new_parm.poll_addr = station->ll.sdl.secladd;
			/* receive timeout value			*/
			new_parm.rcv_timer = station->ll.sls.inact_time * 10;
		}

		if (cb->pl_status != CLOSING)
		{

			rc = fp_ioctl(cb->fp, MP_CHG_PARMS, &new_parm, NULL);

			if (rc)
			{
				cb->sense = rc;
				error_log(cb, ERRID_SDL8056,
						NON_ALERT, 0, FN, LN);
				cb->rc = rc;
			}
		}

			
		/* fill in status block plus extension fields	*/
		st_block.user_sap_corr	= cb->pl.esap.user_sap_corr;
		st_block.user_ls_corr	= station->ll.sls.user_ls_corr;
		st_block.result_ind	= DLC_STAS_RES;
		st_block.result_code	= DLC_SUCCESS;
		ext->maxif		= station->ll.sls.maxif;
		ext->max_data_off	= WOFFSET;

		/* notify user that link station is opened	*/
#ifdef MULT_PU
		(*mpx->excp_fa)(&st_block, mpx);
#endif

	}

	else	/* not valid	*/
		return(cb->rc);

#ifdef MULT_PU
	if (mpx->state & KERN)
#endif
		bcopy(&config, arg, CONFIG_BYTES);
	else
	{
		rc = copyout(&config, arg, CONFIG_BYTES);
		if (rc)
		{
			error_log(cb, ERRID_SDL8067, NON_ALERT, 0, FN, LN);
			cb->rc = EIO;
		}
	}

	return(cb->rc);

}	/**** end of start_ls *******************************************/



/************************************************************************/
/*									*/
/* Function								*/
/* Name:	valid_sls						*/
/*									*/
/* Description: determine if all the start link station values are	*/
/*		valid.							*/
/*									*/
/* Function:	Verify that all the start link station parameters	*/
/*		are valid						*/
/*									*/
/* Input:	SDLC control block					*/
/*		pointer to dlc_sls_arg struct passed in by user 	*/
/*									*/
/* Output:	TRUE (all data valid) or FALSE				*/
/*									*/
/* Normal Exit: Integer return code					*/
/*									*/
/* Error Exit:	None							*/
/*									*/
/* Return Type: int							*/
/*									*/
/************************************************************************/

int	valid_sls(cb, config)

PORT_CB			*cb;
struct	logical_link	*config;

{
        /* check if physical link is opened */
 	if ((cb->pl_status != OPENED ))
 	{
 		error_log(cb, ERRID_SDL0030, NON_ALERT, 0, FN, LN);
 		cb->rc = EINVAL;
 		return(FALSE);
 
 	}
#ifdef MULT_PU
	/* if the link is defined as secondary */	
	if ((config->sls.flags & DLC_SLS_STAT) == 0)
		/* If in mpu mode and address specified is in use. */
		if ( (cb->flags.mpu_enabled && 
		   (cb->link_station_array[config->sdl.secladd] != DLC_NULL))
		/* OR, if not in mpu mode and num. opens greater than one */
			|| (!(cb->flags.mpu_enabled) &&
					     ((int)cb->num_opened > 0)))
#endif
	{
		/* then this start link station is invalid */
		error_log(cb, ERRID_SDL0005, NON_ALERT, 0, FN, LN);
		cb->rc = EINVAL;
		return(FALSE);
	}

	/* if the link is defined as primary, but NOT multi-point  */	
#ifdef MULT_PU
	if ( ((config->sls.flags & DLC_SLS_STAT) == DLC_SLS_STAT)
		&& ((cb->pl.esap.flags & DLC_ESAP_LINK) == 0)
#endif
		/* and there is already a station opened */
		&& ((int)cb->num_opened > 0))
	{
		/* then this start link station is invalid */
		error_log(cb, ERRID_SDL0005, NON_ALERT, 0, FN, LN);
		cb->rc = EINVAL;
		return(FALSE);
	}
    
	/* make sure transmit window count is between 1 and 7	*/
	if ((config->sls.xmit_wind > 7) ||
	    (config->sls.xmit_wind < 1))
	{
	    cb->sense = config->sls.xmit_wind;
	    error_log(cb, ERRID_SDL0039, NON_ALERT, 0, FN, LN);
	    cb->rc = EINVAL;
	    return(FALSE);
								    
	}

	/* check forced disconnect timer not equal to zero	*/
	if (config->sls.force_time == 0)
	{
	    cb->sense = config->sls.force_time;
	    error_log(cb, ERRID_SDL0040, NON_ALERT, 0, FN, LN);
	    cb->rc = EINVAL;
	    return(FALSE);
	}
    
	/* check half duplex mode	*/
	if (config->sdl.duplex != 0)
	{
	    cb->sense = config->sdl.duplex;
	    error_log(cb, ERRID_SDL0041, NON_ALERT, 0, FN, LN);
	    cb->rc = EINVAL;
	    return(FALSE);
	}
    
#ifdef MULT_PU
	/*
	** There was logic here to check repoll threshold and idle timeout.
	** This should only be done for primary, so was moved to the primary
	** section below
	*/
#endif
    
	/* check retransmission count value	*/
	if (config->sdl.retxct == 0)
	{
	    cb->sense = config->sdl.retxct;
	    error_log(cb, ERRID_SDL0045, NON_ALERT, 0, FN, LN);
	    cb->rc = EINVAL;
	    return(FALSE);
	}
    
	/* check retransmission threshold value */
	if ((config->sdl.retxth == 0) || (config->sdl.retxth > 100))
	{
	    cb->sense = config->sdl.retxth;
	    error_log(cb, ERRID_SDL0046, NON_ALERT, 0, FN, LN);
	    cb->rc = EINVAL;
	    return(FALSE);
	}
    
	/* if primary or negotiable station	*/
	if ((config->sls.flags & DLC_SLS_STAT) ||
		(config->sls.flags & DLC_SLS_NEGO))  
	{
	    /* check length of the address field    */
	    if (config->sls.len_raddr_name != 1)
	    {
	    	    cb->sense = config->sls.len_raddr_name;
	    	    error_log(cb, ERRID_SDL0047, NON_ALERT, 0, FN, LN);
		    cb->rc = EINVAL;
		    return(FALSE);
	    }
	
#ifdef MULT_PU
		/* check repoll threshold value */
		if (config->sdl.prirpth == 0)
		{
	    		cb->sense = config->sdl.prirpth;
	    		error_log(cb, ERRID_SDL0042, NON_ALERT, 0, FN, LN);
	    		cb->rc = EINVAL;
	    		return(FALSE);
		}
		/* check idle list timeout value	*/
		if (config->sdl.priilto == 0)
		{
	   		cb->sense = config->sdl.priilto;
	    	error_log(cb, ERRID_SDL0043, NON_ALERT, 0, FN, LN);
	    	cb->rc = EINVAL;
	    	return(FALSE);
		}
#endif

	    if (config->sls.raddr_name[0] == 0)
	    {
	    	    cb->sense = config->sls.raddr_name[0];
	    	    error_log(cb, ERRID_SDL0048, NON_ALERT, 0, FN, LN);
		    cb->rc = EINVAL;
		    return(FALSE);
	    }
	
	    /* check remote address equal to 255    */
	    if (config->sls.raddr_name[0] == 255)
	    {
		    /* if link bit is set then link is multipoint */
		    if (cb->pl.esap.flags & DLC_ESAP_LINK) 
		    {
	    	    	    cb->sense = config->sls.raddr_name[0];
	    	    	    error_log(cb, ERRID_SDL0048, NON_ALERT, 0, FN, LN);
			    cb->rc = EINVAL;
			    return(FALSE);
		    }
	    }
	
	    /* check maximum repoll count	    */
	    if (config->sls.max_repoll == 0)
	    {
	    	    cb->sense = config->sls.max_repoll;
	    	    error_log(cb, ERRID_SDL0050, NON_ALERT, 0, FN, LN);
		    cb->rc = EINVAL;
		    return(FALSE);
	    }
	
	    /* check maximum repoll timer value     */
	    if (config->sls.repoll_time == 0)
	    {
	    	    cb->sense = config->sls.repoll_time;
	    	    error_log(cb, ERRID_SDL0051, NON_ALERT, 0, FN, LN);
		    cb->rc = EINVAL;
		    return(FALSE);
	    }

	} /* end if primary or negotiable station */
    
	/* if the link is secondary or negotiable */
	if (((config->sls.flags & DLC_SLS_STAT) == 0) ||
		(config->sls.flags & DLC_SLS_NEGO))  
	{
		/* check secondary local address limits (1-254) */
		if ((config->sdl.secladd < 1) || (config->sdl.secladd > 254))
		{
	    	        cb->sense = config->sdl.secladd;
	    	        error_log(cb, ERRID_SDL0052, NON_ALERT, 0, FN, LN);
			cb->rc = EINVAL;
			return(FALSE);
		}
	    
		/* check inactivity timer value 	*/
		if (config->sls.inact_time == 0)
		{
			cb->sense = config->sls.inact_time;
	    	        error_log(cb, ERRID_SDL0053, NON_ALERT, 0, FN, LN);
			cb->rc = EINVAL;
			return(FALSE);
		}
	}

	/* check if station is negotiable	*/
	if (config->sls.flags & DLC_SLS_NEGO)
	{
		/* if link bit is set then link is multipoint */
		if (cb->pl.esap.flags & DLC_ESAP_LINK) 
		{
	    	        error_log(cb, ERRID_SDL0054, NON_ALERT, 0, FN, LN);
			cb->rc = EINVAL;
			return(FALSE);
		}
	}

	return(TRUE);

}	/**** end of valid_sls ******************************************/



/************************************************************************/
/*									*/
/* Function								*/
/* Name:	halt_ls 						*/
/*									*/
/* Description: DLC_HALT_LS command handler				*/
/*									*/
/* Function:	If secondary, then start the abort timer		*/
/*									*/
/* Input:	DLC_HALT_LS ioctl					*/
/*									*/
/* Output:	set abort_running flag					*/
/*		set disc_command_pending flag				*/
/*		dequeue the close_send_command queue element without	*/
/*			acknowledgment					*/
/*									*/
/* Normal Exit: return from call					*/
/*									*/
/* Error Exit:	none							*/
/*									*/
/* Called by:	sdlc_link_station					*/
/*									*/
/* Return Type: void							*/
/*									*/
/************************************************************************/

#ifdef MULT_PU
int	halt_ls(cb, mpx, arg)

PORT_CB	*cb;
struct dlc_chan *mpx;
ulong	arg;
#endif

{
	LINK_STATION		*station;
	struct	dlc_corr_arg	corr;
	struct	dlc_getx_arg	ex;
	int	rc;

	cb->rc = NORMAL;

	/* get parameter block from user space		*/
#ifdef MULT_PU
	if (mpx->state & KERN)
#endif
		bcopy(arg, &corr, sizeof(struct dlc_corr_arg));
	else
	{
		cb->rc = copyin(arg, &corr, sizeof(struct dlc_corr_arg));
		if (cb->rc)
		{
	    		error_log(cb, ERRID_SDL8066, NON_ALERT, 0, FN, LN);
			cb->rc = EIO;
			return(cb->rc);
		}
	}
        /* check for valid link station correlator, defect 163699 */
        if ((corr.gdlc_ls_corr < 0) || (corr.gdlc_ls_corr > MAX_NUM_STATIONS))
        {
                cb->sense = (ulong_t)corr.gdlc_ls_corr;
                error_log(cb, ERRID_SDL0032, NON_ALERT, 0, FN, LN);
                return(EINVAL);
        }

#ifdef MULT_PU
	/* Get link station from the link station array in port control
	   block.  The array index is the secondary link station address
	   and is also the correlator */
	station = (LINK_STATION *) cb->link_station_array[corr.gdlc_ls_corr];
#endif

	/* check for valid link station correlator */
	if (!exists(cb, station))
	{
		cb->sense = (ulong_t)station;
    		error_log(cb, ERRID_SDL0073, NON_ALERT, 0, FN, LN);
		return(EINVAL);
	}

	/* If the link is closed then log an error	*/
	if (station->ll_status == CLOSED || station->ll_status == CLOSING)
	{
		cb->sense = station->ll_status;
	    	error_log(cb, ERRID_SDL0038, NON_ALERT, 0, FN, LN);
		cb->rc = EINVAL;
	}

	/* do special processing if the link is secondary	*/
	else if (cb->station_type == SECONDARY)
	{
#ifdef MULT_PU
		SETTIMER(station->abort_timer, station->abort_ticks);
#endif
		ENABLE(station->abort_timer);
		station->abort_running = TRUE;

	}

	/* the link is quiesced, send out disconnect and close link */
	else if (in_list(&cb->quiesce_list, station))
	{
		/* set the abort timer	*/
#ifdef MULT_PU
		SETTIMER(station->abort_timer, station->abort_ticks);
#endif
		ENABLE(station->abort_timer);
		station->abort_running = TRUE;


		/* remove from poll list */
		delete_ls(cb, &cb->quiesce_list, station);

		/* set disconnect command pending	*/
		station->unnum_cmd |= DISC_PENDING;
		station->ll_status = CLOSING;


		/* if no link station is currently active, start transmission */
 		/* flag.on_active only indicate that there is no */
 		/* station in the active list, but the current   */
 		/* station could be active which is not in list  */
 		if ((cb->flags.no_active) && (cb->active_ls == NULL))
		{
			cb->poll_seq_sw = FALSE;
			cb->active_ls = station;
			cb->flags.no_active = FALSE;
			pri_transmit(cb, station);
		}
		else	/* add this station to the active list */
			add_ls(cb, &cb->active_list, station);
		
	}

	/*
	** if station is in idle list, then send a DISC command    
	** OR if station is not in queisce list and is not in NRM
	** then send a DISC command
	*/

	else if (station->poll_mode == IDLE || station->mode != NRM)
	{
		/* set the abort timer	*/
#ifdef MULT_PU
		SETTIMER(station->abort_timer, station->abort_ticks);
#endif
		ENABLE(station->abort_timer);
		station->abort_running = TRUE;
		
		/* if station is in idle poll mode, make it active */
		if (station->poll_mode == IDLE)
		{
			station->poll_mode = ACTIVE;
			--(cb->idle_count);
		}

		/* set disconnect command pending	*/
		station->unnum_cmd |= DISC_PENDING;
		station->ll_status = CLOSING;
		/* make sure only send DISC once */
		station->ct.contig_repolls_sent =
			station->ll.sls.max_repoll - 1;

	}

	else	/* this instance is primary & active	*/
	{
		/* set the abort timer	*/
#ifdef MULT_PU
		SETTIMER(station->abort_timer, station->abort_ticks);
#endif
		ENABLE(station->abort_timer);
		station->abort_running = TRUE;

                if (station->poll_mode == SLOW) /* Defect 116268 */
                   --(cb->slow_count);
                else if (station->poll_mode == IDLE)
                   --(cb->idle_count);
                station->poll_mode = ACTIVE ; /* end Defect 116268 */

		station->ll_status = CLOSING;
	} 

	return(cb->rc);

}	/**** end of halt_ls ********************************************/



/************************************************************************/
/*									*/
/* Function								*/
/* Name:	sdl_contact						*/
/*									*/
/* Description: ioctl DLC_CONTACT command handler			*/
/*									*/
/* Function:	primary station sends out a set normal response mode	*/
/*		command (SNRM) to secondary station.			*/
/*									*/
/* Input:	DLC_CONTACT ioctl					*/
/*									*/
/* Output:	transmit SRNM						*/
/*									*/
/* Normal Exit: return from call					*/
/*									*/
/* Error Exit:	none							*/
/*									*/
/* Called by:	sdlc_link_station					*/
/*									*/
/* Return Type: int							*/
/*									*/
/************************************************************************/

#ifdef MULT_PU
int	sdl_contact(cb, mpx, arg)

PORT_CB	*cb;
struct dlc_chan *mpx;
ulong	arg;

#endif

{

	LINK_STATION		*station;
	struct	dlc_corr_arg	corr;
	int			rc;

	cb->rc = NORMAL;

	/* get parameter block from user space	*/
#ifdef MULT_PU
	if (mpx->state & KERN)
#endif
		bcopy(arg, &corr, sizeof(struct dlc_corr_arg));
	else
	{
		cb->rc = copyin(arg, &corr, sizeof(struct dlc_corr_arg));
		if (cb->rc)
		{
	    		error_log(cb, ERRID_SDL8066, NON_ALERT, 0, FN, LN);
			return(EIO);
		}
	}
	/* check for valid link station correlator, defect 163696 */
	if ((corr.gdlc_ls_corr < 0) || (corr.gdlc_ls_corr > MAX_NUM_STATIONS))
	{
		cb->sense = (ulong_t)corr.gdlc_ls_corr;
		error_log(cb, ERRID_SDL0032, NON_ALERT, 0, FN, LN);
		return(EINVAL);
	}


#ifdef MULT_PU
	station = (LINK_STATION *) cb->link_station_array[corr.gdlc_ls_corr];
#endif

	/* check for valid link station correlator */
	if (!exists(cb, station))
	{
		cb->sense = (ulong_t)station;
    		error_log(cb, ERRID_SDL0070, NON_ALERT, 0, FN, LN);
		return(EINVAL);
	}

	if (station->ll_status != OPENED)
	{
		/* then log an error */
		cb->sense = station->ll_status;
	    	error_log(cb, ERRID_SDL0020, NON_ALERT, 0, FN, LN);
		return(EINVAL);
	}

	if /* station is not primary */
	   ((cb->station_type != PRIMARY) 
		/* or if there is an unnumbered command pending */
		|| (station->unnum_cmd)
		/* or if there is an unnumbered response pending */
		|| (station->unnum_rsp)
		/* or if the station is not in the quiesce list */
		|| (in_list(&cb->quiesce_list, station) == FALSE)
		/* or if the station is not quiesced */
	   	|| (station->mode != QUIESCE))
	{
		/* then log an error */
		cb->sense = cb->station_type;
	    	error_log(cb, ERRID_SDL0020, NON_ALERT, 0, FN, LN);
		return(EINVAL);
	}

	/* remove from poll list */
	delete_ls(cb, &cb->quiesce_list, station);

	/* set SNRM COMMAND PENDING indicator */
	station->unnum_cmd |= SNRM_PENDING;


	/* if no link station is currently active, start transmission */
 	if ((cb->flags.no_active) && (cb->active_ls == NULL))
	{
		cb->poll_seq_sw = FALSE;
		cb->active_ls = station;
		cb->flags.no_active = FALSE;
		pri_transmit(cb, station);
	}
	else	/* add this station to the active list */
		add_ls(cb, &cb->active_list, station);


	return(cb->rc);

}	/**** end of sdl_contact ****************************************/



/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:        sdl_alter                                               */
/*                                                                      */
/* Description: ioctl DLC_ALTER command handler                         */
/*                                                                      */
/* Function:    This function is called when the user issues an ioctl   */
/*              with the DLC_ALTER option.  There are several param-    */
/*              eters than can be changed with this command.            */
/*              The station control block is updated with the new in-   */
/*              formation.  In some cases, the parameters that are      */
/*              changed may effect the current device handler parms,    */
/*              when this is the case, an fp_ioctl is issued to update  */
/*              those parameters as well.                               */
/*                                                                      */
/* Input:       pointer to struct dlc_alter_arg                         */
/*                                                                      */
/* Output:      alter the repoll timeout values                         */
/*              set snrm_command_pending                                */
/*              alter transmit window value                             */
/*              alter secondary inactivity timeout value                */
/*              alter inactivity with termination option                */
/*              set test_command_pending                                */
/*              alter station mode                                      */
/*                                                                      */
/* Normal Exit: return from call                                        */
/*                                                                      */
/* Error Exit:  none                                                    */
/*                                                                      */
/* Return Type: int                                                     */
/*                                                                      */
/************************************************************************/

#ifdef MULT_PU
int     sdl_alter(cb, mpx, arg)

PORT_CB *cb;
struct dlc_chan *mpx;
ulong   arg;

#endif

{

	struct      T_CHG_PARMS     new_parm;
	struct      dlc_alter_arg   new_config;
	LINK_STATION                *station;
	int                         rc;
	ulong       alter_flags;


	cb->rc = NORMAL;

	/* get parameter block from user space      */
#ifdef MULT_PU
	if (mpx->state & KERN)
#endif
		bcopy(arg, &new_config, sizeof(struct dlc_alter_arg));
	else
	{
		cb->rc = copyin(arg, &new_config, sizeof(struct dlc_alter_arg));
		if (cb->rc)
		{
			error_log(cb, ERRID_SDL8066, NON_ALERT, 0, FN, LN);
			return(EIO);
		}
	}


	/* pre set result flags to zero */
	new_config.result_flags = 0;

	/* save original flag configuration */
	alter_flags = new_config.flags;

	/* clear flags that are not supported */
	alter_flags &= (~(DLC_ALT_RTE | DLC_ALT_AKT));

	/* check for valid link station correlator, defect 163696 */
	if ((new_config.gdlc_ls_corr < 0) || (new_config.gdlc_ls_corr > MAX_NUM_STATIONS))
	{
		cb->sense = (ulong_t)new_config.gdlc_ls_corr;
		error_log(cb, ERRID_SDL0032, NON_ALERT, 0, FN, LN);
		return(EINVAL);
	}

#ifdef MULT_PU
	station =
	  (LINK_STATION *)cb->link_station_array[new_config.gdlc_ls_corr];
#endif

	/* check for valid link station correlator */
	if (!exists(cb, station))
	{
		cb->sense = (ulong_t)station;
    		error_log(cb, ERRID_SDL0072, NON_ALERT, 0, FN, LN);
		return(EINVAL);
	}

	if (station->ll_status != OPENED)
		return(EINVAL);

	/*
	** The flags field of new_config contains the ORed value of
	** all the parameters that are to be altered.
	*/

	if (station->ll_status == OPENED)
	{
		/* does the repoll_time parameter need to be changed */
		if ((alter_flags & DLC_ALT_RTO))
		{
			/* clear this bit */
			alter_flags &= (~DLC_ALT_RTO);
/* defect 109243 */
			if ((new_config.repoll_time == 0) ||
			    (new_config.repoll_time > 255))
/* end defect 109243 */
			{
				/* invalid repoll_time value */
				cb->sense = new_config.repoll_time;
				error_log(cb, ERRID_SDL006B,
					NON_ALERT, 0, FN, LN);
				cb->rc = EINVAL;
			}

			else /* set new value in station block */
				station->ll.sls.repoll_time =
					new_config.repoll_time ;
		}

		/* command: alter transmit window size              */
		if ((alter_flags & DLC_ALT_XWIN))
		{
			alter_flags &= (~DLC_ALT_XWIN);

			/*
			** If an invalid window size was passed, then fail the
			** alter command
			*/

			if ((new_config.xmit_wind < 1)
				|| (new_config.xmit_wind > 7))
			{
				cb->sense = new_config.xmit_wind;
				error_log(cb, ERRID_SDL006C, NON_ALERT,
								0, FN, LN);
				cb->rc = EINVAL;
			}
			else
				station->ll.sls.xmit_wind =
						new_config.xmit_wind;
		}

		/* command: change maximum i field length           */
		if ((alter_flags & DLC_ALT_MIF))
		{
			alter_flags &= (~DLC_ALT_MIF);

/* defect 109243 */
			if ((new_config.maxif == 0) ||
			    (new_config.maxif > MAX_I_FRAME))
			{
				cb->sense = new_config.maxif;
				error_log(cb, ERRID_SDL006A,
						NON_ALERT, 0, FN, LN);
				cb->rc = EINVAL;
			}
			else
				station->ll.sls.maxif = new_config.maxif;
/* end defect 109243 */
		}

		/* command: change maximum repoll count           */
 		if ((alter_flags & DLC_ALT_MXR))
 		{
 			alter_flags &= (~DLC_ALT_MXR);
/* defect 109243 */
			if ((new_config.max_repoll == 0) ||
			    (new_config.max_repoll > 255))
 
 			{
 				cb->sense = new_config.max_repoll;
 				error_log(cb, ERRID_SDL006A,
 						NON_ALERT, 0, FN, LN);
 				cb->rc = EINVAL;
 			}
			else
				station->ll.sls.max_repoll =
						new_config.max_repoll;
/* end defect 109243 */
 		}
 
		/* command: alter secondary timeout value    */
		if ((alter_flags & DLC_ALT_ITO))
		{
			alter_flags &= (~DLC_ALT_ITO);

			if (cb->station_type == SECONDARY)
			{
/* defect 109243 */
				if ((new_config.inact_time == 0) ||
				    (new_config.inact_time > 255))
/* end defect 109243 */
				{
					cb->sense = new_config.inact_time;
					error_log(cb, ERRID_SDL0069,
						NON_ALERT, 0, FN, LN);
					cb->rc = EINVAL;
				}

				else /* set new value */
				{
					station->ll.sls.inact_time =
						new_config.inact_time;

#ifdef MULT_PU
					/* the port is not operating in
							multi-PU mode */
					if (cb->flags.mpu_enabled == FALSE)
					{
						/*
						** send change parm command
						** to device handler
						*/
						new_parm.chg_mask =
						     CP_RCV_TMR;
						new_parm.rcv_timer =
						new_config.inact_time * 10;

						if (cb->pl_status !=
						     CLOSING)
						{

							rc = fp_ioctl(
							      cb->fp,
							      MP_CHG_PARMS,
							      &new_parm,
							      NULL);
							if (rc)
							{
							     cb->sense = rc;
							     error_log(cb,
							      ERRID_SDL8056,
							      NON_ALERT,
							      0, FN, LN);
							     cb->rc = rc;
							}
						}
					}
#endif
				}
			}

			else /* station must be primary */
			{
				error_log(cb, ERRID_SDL0021,
					NON_ALERT, 0, FN, LN);
				cb->rc = EINVAL;
			}
		}

		/* command: set inactivity with termination	*/
		if ((alter_flags & DLC_ALT_IT2))
		{
			alter_flags &= (~DLC_ALT_IT2);
			station->ll.sls.flags &= (~DLC_SLS_HOLD);
		}

		/* command: set inactivity without termination	*/
		if ((alter_flags & DLC_ALT_IT1))
		{
			alter_flags &= (~DLC_ALT_IT1);
			station->ll.sls.flags |= DLC_SLS_HOLD;
		}

		/* command: alter force halt timeout */
		if (alter_flags & DLC_ALT_FHT)
		{
/* defect 109243 */
			alter_flags &= (~DLC_ALT_FHT);
			if ((new_config.force_time == 0) ||
			    (new_config.force_time > 16383))
/* end defect 109243 */
			{
				cb->sense = new_config.force_time;
				error_log(cb, ERRID_SDL0068,
						NON_ALERT, 0, FN, LN);
				cb->rc = EINVAL;
			}
			else
			{
				station->ll.sls.force_time = 
					new_config.force_time;
#ifdef MULT_PU
				station->abort_ticks =
				      new_config.force_time * 10;
#endif
			}
		}

		/* command: set mode primary        */
		if ((alter_flags & DLC_ALT_SM1))
		{
			alter_flags &= (~DLC_ALT_SM1);

			/*
			** station is in normal disconnnect
			** mode, and NO unnumbered 
			** command/response pending
			*/
			if ((cb->station_type == SECONDARY)
			    && (station->mode == NDM)
			    && (station->unnum_cmd == FALSE)
#ifdef MULT_PU
			    && (station->unnum_rsp == FALSE)
				&& !(cb->flags.mpu_enabled))
#endif
			{
				/* set station type to PRIMARY */
				cb->station_type = PRIMARY;
				station->mode = QUIESCE;
				station->unnum_cmd = FALSE;
				cb->poll_seq_sw = TRUE;
				station->s_frame_ct = 0;
				cb->idle_timer.enabled = FALSE;
				station->saved_repoll =
					station->ll.sls.repoll_time;
				new_config.result_flags = DLC_MSP_RES;

				/* Defect 160519 */
				/* remove from active poll list */
				/* if SECONDARY it is not in active_list*/
				/* delete_ls(cb, &cb->active_list, station); */
				delete_ls(cb, &(cb->mpu_sta_list), station);
				/* End Defect 160519 */

				/* place in bottom of quiesce pl */
				add_ls(cb, &cb->quiesce_list, station);

				/* 
				** send change parameters command
				** to the device handler
				*/
				new_parm.chg_mask = CP_RCV_TMR;
				new_parm.rcv_timer = 0;

				if (cb->pl_status != CLOSING)
				{

					rc = fp_ioctl(cb->fp,
						MP_CHG_PARMS, &new_parm, NULL);

					if (rc)
					{
						cb->sense = rc;
						error_log(cb, ERRID_SDL8056,
							NON_ALERT, 0, FN, LN);
						cb->rc = rc;
					}
				}
			}

			else /* set mode_failure return code  */
			{
				cb->rc = EINVAL;
				new_config.result_flags = DLC_MSPF_RES;
			}
		}

		/* command: set mode secondary              */
		if ((alter_flags & DLC_ALT_SM2))
		{
			alter_flags &= (~DLC_ALT_SM2);

			/* check status */

			/*
			** station is primary, in quiesce mode,
			** in quiesce list, no unnum cmd/resp
			** pending and has only one logical
			** link opened
			*/
			if ((station->mode == QUIESCE)
			    && (cb->station_type == PRIMARY)
			    && (station->unnum_cmd == FALSE)
			    && (station->unnum_rsp == FALSE)
			    && (in_list(&cb->quiesce_list, station))
			    && (cb->num_opened <= 1))
			{
				cb->station_type = SECONDARY;
				station->mode = NDM;

				/* remove station from quiesce list */
				delete_ls(cb, &cb->quiesce_list,station);

				/* Defect 160519 */
				/* add_ls(cb, &cb->active_list, station); */
				/* add it to the station poll list      */
				add_sta_to_list(cb, &(cb->mpu_sta_list),
					station);
				/* End Defect 160519 */

				cb->poll_seq_sw = FALSE;
				cb->active_ls = station;

				/*
				** set up the change parms struct
				** for the device handler
				*/
				new_parm.chg_mask = (CP_RCV_TMR | CP_POLL_ADDR);
				new_parm.poll_addr = station->ll.sdl.secladd;
				new_parm.rcv_timer =
					station->ll.sls.inact_time * 10;

				new_config.result_flags = DLC_MSS_RES;

				if (cb->pl_status != CLOSING)
				{

					rc = fp_ioctl(cb->fp,
						MP_CHG_PARMS, &new_parm, NULL);
					if (rc)
					{
						cb->sense = rc;
						error_log(cb, ERRID_SDL8056,
							NON_ALERT, 0, FN, LN);
						cb->rc = rc;
					}
				}
			}
			else
			{
				cb->rc = EINVAL;
				new_config.result_flags = DLC_MSSF_RES;
			}
		}
	}

	/*
	** as each alter command is completed, its flag bit is cleared
	** so if there are any bits still turned on, then an invalid 
	** alter flag was passed down
	*/
	if (alter_flags)
	{
		cb->sense = alter_flags;
		error_log(cb, ERRID_SDL0067, NON_ALERT, 0, FN, LN);
		cb->rc = EINVAL;
	}

#ifdef MULT_PU
	if (mpx->state & KERN)
#endif
		bcopy(&new_config, arg, sizeof(struct dlc_alter_arg));
	else
	{
		rc = copyout(&new_config, arg,
			sizeof(struct dlc_alter_arg));
		if (rc)
		{
			error_log(cb, ERRID_SDL8067, NON_ALERT, 0, FN, LN);
			cb->rc = EIO;
		}
	}

	if (cb->sdllc_trace)
		sdlmonitor(cb, CHANGE_PARAMETER, new_config.flags, cb->rc, 0,0);

	return(cb->rc);

}       /**** end of sdl_alter ******************************************/



/************************************************************************/
/*									*/
/* Function								*/
/* Name:	enable_sap						*/
/*									*/
/* Description: open the physical link (async)				*/
/*									*/
/* Function:	open physical link					*/
/*		initialize flags and storage				*/
/*									*/
/* Input:	ENABLE_SAP ioctl					*/
/*		point to dlc_esap_arg					*/
/*									*/
/* Output:	CIO_START ioctl to device handler			*/
/*		call excpetion entry on error				*/
/*									*/
/* Normal Exit: Return NORMAL to calling subroutine			*/
/*									*/
/* Error Exit:	Return ENODEV to calling subroutine			*/
/*		call excp_fa() with status block			*/
/*									*/
/* Called by:	send_command_handler					*/
/*									*/
/* Return Type: int							*/
/*									*/
/************************************************************************/

#ifdef MULT_PU
int enable_sap(cb, mpx, arg)

PORT_CB			*cb;
struct dlc_chan *mpx;
ulong			arg;

#endif

{
	register	int	failure;
	int	rc;
	struct	physical_link	pl_config;
	struct	dlc_getx_arg	exception;
	t_err_threshold		thresh;		/* mpqp error thresholds*/


	/* pre set return codes */
	failure = FALSE;
	rc = NORMAL;

#ifdef MULT_PU
	/*
	** removed user's function address copy from channel to port cb
	*/
#endif

	/* copy user data into kernel space		*/
#ifdef MULT_PU
    if (mpx->state & KERN)
#endif
		bcopy(arg, &pl_config, sizeof (struct physical_link));
	else
	{
		cb->rc = copyin(arg, &pl_config, sizeof (struct physical_link));
		if (cb->rc)
		{
			error_log(cb, ERRID_SDL8066, NON_ALERT, 0, FN, LN);
			return(EIO);
		}
	}
#ifdef MULT_PU
	/* Check if the SAP has been enabled already. If it has, return the
	   original SAP correlator and set status to EBUSY */
	if ((cb->pl_status == OPENED) || ((cb->pl_status == CLOSED) &&
	    (cb->flags.phy_starting)))
	{
		/* set the sap correlator to whatever the first sap
		   correlator on the port was */
		pl_config.esap.gdlc_sap_corr = cb->pl.esap.gdlc_sap_corr;
		if (mpx->state & KERN)
			bcopy(&pl_config, arg, sizeof(struct physical_link));
		else /* user process */
		{
			rc = copyout(&pl_config, arg,
				sizeof(struct physical_link));
			if (rc)
			{
				error_log(cb, ERRID_SDL8067,
						NON_ALERT, 0, FN, LN);
				return(EIO);
			}
		}
		if (cb->flags.mpu_enabled)
			/* return EBUSY for multi-PU */
			return(EBUSY);
		else
			/* return EINVAL for non-mpu because thats what it
			   does today */
			return(EINVAL);
	}
#endif

        pl_config.mpqp.poll_addr = 0;
        pl_config.mpqp.select_addr = 0;
	/* save physical link information in port control block */
	bcopy(&pl_config, &cb->pl, sizeof(struct physical_link));
	
	/* physical links status should be closed 	*/
	/* and phy_starting flag should be false        */
	if ((cb->pl_status == CLOSED)
		&& (!cb->flags.phy_starting)
		&& (cb->rc == NORMAL))
	{

		/* check for valid link station maximium */

		/* if point to point link */
		if ((pl_config.esap.flags & DLC_ESAP_LINK) == 0)
		{
			if (pl_config.esap.max_ls != 1) 
			{
				error_log(cb, ERRID_SDL0057, NON_ALERT,
								0, FN, LN);
				failure = TRUE;
				cb->rc = EINVAL;
				return(cb->rc);
			}
		}

		/* else it is a multi-point link */
		else
		{
			if (pl_config.esap.max_ls == 0)
			{
				error_log(cb, ERRID_SDL0058, NON_ALERT,
								0, FN, LN);
				failure = TRUE;
				cb->rc = EINVAL;
				return(cb->rc);
			}
		}
		pl_config.esap.gdlc_sap_corr = pl_config.esap.user_sap_corr;

		/* clear the threshold structure */
		bzero(&thresh, sizeof(t_err_threshold));

		/* fill in the threshold values */
                thresh.tx_err_thresh    =       HOW_IS_LINE;
		/* PMR 9x296,403, Apar ix28151, Defect 71232 */
		thresh.rx_err_thresh    =       5000;
                thresh.tx_err_percent   =       TRANSMIT_PERCENT;
                thresh.rx_err_percent   =       RECEIVE_PERCENT;
                thresh.tx_underrun_thresh       =HOW_IS_LINE;
                thresh.tx_cts_drop_thresh       =HOW_IS_LINE;
                thresh.tx_cts_timeout_thresh    =HOW_IS_LINE;
                thresh.tx_fs_timeout_thresh     =HOW_IS_LINE;
                thresh.rx_overrun_err_thresh    =HOW_IS_LINE;
		/* Defect 61591, ix28151 */
                thresh.rx_abort_err_thresh      =5000;
                thresh.rx_frame_err_thresh      =HOW_IS_LINE;
                thresh.rx_par_err_thresh        =HOW_IS_LINE;

		cb->pl.mpqp.p_err_threshold = &thresh;

		performance_trace(cb, DLC_TRACE_STDH, 0);

		rc = fp_ioctl(cb->fp, CIO_START, &cb->pl.mpqp, NULL);

		if (cb->sdllc_trace)
			sdlmonitor(cb, START_DEVICE, 0, rc, 0, 0);

		if (rc != NORMAL)
		{
			error_log(cb, ERRID_SDL8052, NON_ALERT, 0, FN, LN);
			failure = TRUE;
			cb->rc = rc;
		}

		if (failure == FALSE)
		{

			/* if link is point to point */
			if ((pl_config.esap.flags & DLC_ESAP_LINK) == 0)
			{
				/* then force the session limit to 1 */
				cb->pl.esap.max_ls = 1;
			}

			cb->flags.phy_starting = TRUE;
#ifdef MULT_PU
			/* log the cid of the process that did the
			   ENABLE_SAP */
			cb->sap_cid = mpx;

			/* initialize the "busy" retry anchor */
			cb->retry_list = NULL;
#endif
		}
 		/* clean up the cb control board, user may close */
                /* sap then enable sap again, so need to clean   */
  		/* up some variables                             */
#ifdef MULT_PU
		/*
		** removed retry logic
		*/
#endif
 		cb->n_m =0;
	 	cb->poll_seq_sw =0;
 		cb->slow_count  =0;
 		cb->idle_count  =0;
	}

#ifdef MULT_PU
	/*
	** removed else with errlog SDL0016
	*/
#endif


	/* If failure during START */
	if (failure == TRUE)
	{
		cb->rc = EIO;
	}
	else
	{
#ifdef MULT_PU
		if (mpx->state & KERN)
#endif

			bcopy(&pl_config, arg, sizeof(struct physical_link));
		else
		{
			rc = copyout(&pl_config, arg,
				sizeof(struct physical_link));
			if (rc)
			{
				error_log(cb, ERRID_SDL8067,
						NON_ALERT, 0, FN, LN);
				cb->rc = EIO;
			}
		}
	}

	return(cb->rc);

}	/**** end of enable_sap *****************************************/



/************************************************************************/
/*									*/
/* Function								*/
/* Name:	disable_sap						*/
/*									*/
/* Description: send halt command to device handler			*/
/*									*/
/* Function:	issue a halt command to the device handler		*/
/*									*/
/* Input:	SDLC port control block 				*/
/*									*/
/* Output:	halt device command					*/
/*									*/
/* Normal Exit: return NORMAL rc					*/
/*									*/
/* Error Exit:	return error rc 					*/
/*									*/
/* Return Type: int							*/
/*									*/
/************************************************************************/

#ifdef MULT_PU
int	disable_sap(cb, mpx, arg)

PORT_CB	*cb;
struct dlc_chan *mpx;
ulong	arg;

#endif

{
	struct	dlc_corr_arg	corr;
	int	rc;

	cb->rc = NORMAL;

#ifdef MULT_PU
	if (mpx->state & KERN)
#endif
		bcopy(arg, &corr, sizeof(struct dlc_corr_arg));
	else
	{
		cb->rc = copyin(arg, &corr, sizeof(struct dlc_corr_arg));
		if (cb->rc)
		{
			error_log(cb, ERRID_SDL8066, NON_ALERT, 0, FN, LN);
			return(EIO);
		}
	}

#ifdef MULT_PU
	/* if one of the three cases is true
	**	1) physical link is CLOSED and the device is NOT starting
	**	2) physical link is CLOSING
	**  3) The process attempting the disable is not the process that
	**     performed the enable.
	*/
	if ((cb->pl_status == CLOSED && !(cb->flags.phy_starting))
		|| (cb->pl_status == CLOSING) || (cb->sap_cid != mpx))
#endif
	{
		/* then the halt is invalid */
		error_log(cb, ERRID_SDL0028, NON_ALERT, 0, FN, LN);
		cb->rc = EINVAL;
	}


	/* otherwise, begin to close link */
	else
	{
		/* if there are link stations opened */
		if (cb->num_opened)
		{
			/* then close all link stations */
			sdl_abort(cb);
		}

		/* send a halt command to the device handler */
		pl_blowout(cb);

	}

#ifdef MULT_PU
	if (mpx->state & KERN)
#endif
		bcopy(&corr, arg, sizeof(struct dlc_corr_arg));
	else
	{
		rc = copyout(&corr, arg, sizeof(struct dlc_corr_arg));
		if (rc)
		{
			error_log(cb, ERRID_SDL8067, NON_ALERT, 0, FN, LN);
			cb->rc = EIO;
		}
	}

	return(cb->rc);

}	/**** end of disable_sap ****************************************/


/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	sdl_test                                                */
/*                                                                      */
/* Description:	DLC_TEST ioctl command handler                          */
/*                                                                      */
/* Function:	build and transmit a test buffer                        */
/*                                                                      */
/* Input:	                                                        */
/*                                                                      */
/* Output:	transmit test buffer                                    */
/*                                                                      */
/* Normal Exit:	return from call                                        */
/*                                                                      */
/* Error Exit:	none                                                    */
/*                                                                      */
/* Called by:	pr_ioctl                                                */
/*                                                                      */
/* Return Type: int                                                     */
/*                                                                      */
/************************************************************************/

#ifdef MULT_PU
int	sdl_test(cb, mpx, arg)

PORT_CB	*cb;
struct dlc_chan *mpx;
ulong	arg;

#endif

{
	struct dlc_corr_arg	corr;
	LINK_STATION		*station;
	int			index;
	int			rc;
	int			data_bytes;

	cb->rc = NORMAL;

	/* copy parameter block into kernel space	*/
#ifdef MULT_PU
	if (mpx->state & KERN)
#endif
		bcopy(arg, &corr, sizeof(struct dlc_corr_arg));
	else
	{
		cb->rc = copyin(arg, &corr, sizeof(struct dlc_corr_arg));

		if (cb->rc)
		{
			error_log(cb, ERRID_SDL8066, NON_ALERT, 0, FN, LN);
			return(EIO);
		}
	}

	/* check for valid link station correlator, defect 163696 */
	if ((corr.gdlc_ls_corr < 0) || (corr.gdlc_ls_corr > MAX_NUM_STATIONS))
	{
		cb->sense = (ulong_t)corr.gdlc_ls_corr;
		error_log(cb, ERRID_SDL0032, NON_ALERT, 0, FN, LN);
		return(EINVAL);
	}

	/* get addressability to the station control block	*/
#ifdef MULT_PU
	station = (LINK_STATION *) cb->link_station_array[corr.gdlc_ls_corr];
#endif

	/* check for valid link station correlator */
	if (!exists(cb, station))
	{
		cb->sense = (ulong_t)station;
    		error_log(cb, ERRID_SDL0071, NON_ALERT, 0, FN, LN);
		return(EINVAL);
	}

	/*
	** If any one of the following is true:
	**    1) Station is not PRIMARY
	**    2) Station is not OPENED
	**    3) There is an unnumbered command pending
	**    4) There is an unnumbered response pending
	** Then a test buffer can NOT be sent
	*/
	if ((cb->station_type != PRIMARY)
	    || (station->ll_status != OPENED)
	    || (station->unnum_cmd)
	    || (station->unnum_rsp))

	{
		error_log(cb, ERRID_SDL0022, NON_ALERT, 0, FN, LN);
		cb->rc = EINVAL;
	}

	else
	{
		/* if the station is in the quiesce list */
		if (in_list(&cb->quiesce_list, station))
		{
			/* then remove it */
			delete_ls(cb, &cb->quiesce_list, station);
		}

		/*
		** build the test buffer
		*/

		/* get an mbuf and cluster from the buffer pool	*/
		cb->m = (struct mbuf *) m_get(M_WAIT, MT_DATA);
		m_clget(cb->m);

		if (cb->m)
		{
			/* max length of test data is 256 */
			data_bytes = ((station->ll.sls.maxif < 256) ?
			    station->ll.sls.maxif : 256);
			cb->m->m_len = HEADER_LENGTH + data_bytes;

			cb->frame_ptr = MTOD(cb->m, uchar *);
			cb->frame_ptr[CONTROL] = TEST;
			cb->frame_ptr[ADDRESS] = station->ll.sls.raddr_name[0];

			/* fill in test buffer	*/
			for (index=HEADER_LENGTH; index<cb->m->m_len; index++)
				cb->frame_ptr[index] = index;

			/* save the buffer address	*/
			station->test.m = cb->m;

			station->unnum_cmd |= TEST_PENDING;

			/* if no link station is currently active */
 			if ((cb->flags.no_active) && (cb->active_ls == NULL))
			{
				/* then transmit the buffer */

				cb->poll_seq_sw = FALSE;
				cb->active_ls = station;
				cb->flags.no_active = FALSE;
				pri_transmit(cb, station);
			}

			else if (cb->active_ls != station)
				add_ls(cb, &cb->active_list, station);
		}

		else	/* could not get an mbuf	*/
		{
			error_log(cb, ERRID_SDL8046, NON_ALERT, 0, FN, LN);
			cb->rc = ENOMEM;
		}
	}

	return(cb->rc);

}	/**** end of sdl_test *******************************************/



/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	query_sap                                               */
/*                                                                      */
/* Description:	get information about physical link (SAP)               */
/*                                                                      */
/* Function:	                                                        */
/*                                                                      */
/* Input:	control block pointer                                   */
/*		parameter block pointer					*/
/*                                                                      */
/* Output:	updated paramter block                                  */
/*                                                                      */
/* Normal Exit:	return status of operation                              */
/*                                                                      */
/* Error Exit:	none                                                    */
/*                                                                      */
/* Return Type:	int                                                     */
/*                                                                      */
/************************************************************************/

#ifdef MULT_PU
int	query_sap(cb, mpx, arg)

PORT_CB	*cb;
struct dlc_chan *mpx;
ulong	arg;

#endif

{
	struct	dlc_qsap_arg	qsap;

#ifdef MULT_PU
	if (mpx->state & KERN)
#endif
		bcopy(arg, &qsap, sizeof(struct dlc_qsap_arg));
	else
	{
		cb->rc = copyin(arg, &qsap, sizeof(struct dlc_qsap_arg));
		if (cb->rc)
		{
			error_log(cb, ERRID_SDL8066, NON_ALERT, 0, FN, LN);
			return(EIO);
		}
	}

	/* can't query a sap that does not exist yet */
	if (cb->pl_status == CLOSED)
	{
		return(EINVAL);
	}

	qsap.user_sap_corr = cb->pl.esap.user_sap_corr;
	switch(cb->pl_status)
	{
	case OPENED:
		qsap.sap_state = DLC_OPENED;
		break;
	case CLOSING:
		qsap.sap_state = DLC_CLOSING;
		break;
	}
		
	strcpy(qsap.dev, cb->dlc.namestr); 
	qsap.devdd_len = 0;

#ifdef MULT_PU
	if (mpx->state & KERN)
#endif
		bcopy(&qsap, arg, sizeof(struct dlc_qsap_arg));
	else
	{
		cb->rc = copyout(&qsap, arg, sizeof(struct dlc_qsap_arg));
		if (cb->rc)
		{
			error_log(cb, ERRID_SDL8067, NON_ALERT, 0, FN, LN);
			cb->rc = EIO;
		}
	}

	return(cb->rc);
}



/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	query_ls                                                */
/*                                                                      */
/* Description:	get information about link station                      */
/*                                                                      */
/* Function:	fill in the parameter block with data from the port     */
/*		control block						*/
/*                                                                      */
/* Input:	control block pointer                                   */
/*		parameter block pointer					*/
/*                                                                      */
/* Output:	updated paramter block                                  */
/*                                                                      */
/* Normal Exit:	return status of operation                              */
/*                                                                      */
/* Error Exit:	none                                                    */
/*                                                                      */
/* Return Type:	int                                                     */
/*                                                                      */
/************************************************************************/

#ifdef MULT_PU
int	query_ls(cb, mpx, arg)

PORT_CB	*cb;
struct dlc_chan *mpx;
ulong	arg;

#endif

{
	LINK_STATION		*station;
	struct	dlc_qls_arg	qls;
	int	rc;

	/* get parameter block from user space	*/

#ifdef MULT_PU
	if (mpx->state & KERN)
#endif
		bcopy(arg, &qls, sizeof(struct dlc_qls_arg));
	else
	{
		cb->rc = copyin(arg, &qls, sizeof(struct dlc_qls_arg));
		if (cb->rc)
		{
			error_log(cb, ERRID_SDL8066, NON_ALERT, 0, FN, LN);
			return(EIO);
		}
	}

	/* check for valid link station correlator, defect 163696 */
	if ((qls.gdlc_ls_corr < 0) || (qls.gdlc_ls_corr > MAX_NUM_STATIONS))
	{
		cb->sense = (ulong_t)qls.gdlc_ls_corr;
		error_log(cb, ERRID_SDL0032, NON_ALERT, 0, FN, LN);
		return(EINVAL);
	}

	/* get addressability to the link station	*/
#ifdef MULT_PU
	station = (LINK_STATION *) cb->link_station_array[qls.gdlc_ls_corr];
#endif

	/* check for valid link station correlator */
	if (!exists(cb, station))
	{
		cb->sense = (ulong_t)station;
    		error_log(cb, ERRID_SDL006F, NON_ALERT, 0, FN, LN);
		return(EINVAL);
	}

	if ((exists(cb, station)) && (cb->pl_status != CLOSED))
	{
		/* copy information into the param block	*/
		qls.user_sap_corr = cb->pl.esap.user_sap_corr;
		qls.user_ls_corr = station->ll.sls.user_ls_corr;
		strcpy(qls.ls_diag, station->ll.sls.ls_diag); 
		switch(station->ll_status)
		{
		case OPENING:
			qls.ls_state = DLC_OPENING;
			break;
		case OPENED:
			qls.ls_state = DLC_OPENED;
			break;
		case CLOSING:
			qls.ls_state = DLC_CLOSING;
			break;
		default:
			qls.ls_state = DLC_INACTIVE;
		}

		qls.ls_sub_state = station->sub_state;

		/* copy counter information into param block */
		bcopy(&station->ct, &qls.counters, 
					sizeof (struct counters)); 

		qls.protodd_len = 0;
	}
	else	/* either phys link was closed or invalid station */
	{
		if (cb->pl_status == CLOSED)
		    error_log(cb, ERRID_SDL0031, NON_ALERT, 0, FN, LN);
		else
		    error_log(cb, ERRID_SDL0032, NON_ALERT, 0, FN, LN);
		cb->rc = EINVAL;
	}

	/* copy parameter block back to user space */
#ifdef MULT_PU
	if (mpx->state & KERN)
#endif
		bcopy(&qls, arg, sizeof(struct dlc_qls_arg));
	else
	{
		rc = copyout(&qls, arg, sizeof(struct dlc_qls_arg));
		if (rc)
		{
			error_log(cb, ERRID_SDL8067, NON_ALERT, 0, FN, LN);
			cb->rc = EIO;
		}
	}

	return(cb->rc);

}	/**** end of query_ls *******************************************/



/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	sdl_trace                                               */
/*                                                                      */
/* Description:	DLC_TRACE command handler                               */
/*                                                                      */
/* Function:	Turn on/off trace flag                                  */
/*              Set trace long/short                                    */
/*                                                                      */
/* Input:	pointer to port control block                           */
/*		parameter block supplied by the user			*/
/*                                                                      */
/* Output:	update internal control structure                       */
/*                                                                      */
/* Normal Exit:	return integer rc                                       */
/*                                                                      */
/* Error Exit:	none                                                    */
/*                                                                      */
/* Return Type:	int                                                     */
/*                                                                      */
/************************************************************************/

#ifdef MULT_PU
int	sdl_trace(cb, mpx, arg)

PORT_CB	*cb;
struct dlc_chan *mpx;
ulong	arg;

#endif

{

    struct	dlc_trace_arg	trace;
    LINK_STATION		*station;
    int				rc;

    /* copy data from user space */

#ifdef MULT_PU
    if (mpx->state & KERN)
#endif
    	bcopy(arg, &trace, TRACE_SIZE);
    else
    {
    	cb->rc = copyin(arg, &trace, TRACE_SIZE);

    	if (cb->rc)
    	{
		error_log(cb, ERRID_SDL8066, NON_ALERT, 0, FN, LN);
		return(EIO);
    	}
    }

    /* check for valid link station correlator, defect 163696 */
    if ((trace.gdlc_ls_corr < 0) || (trace.gdlc_ls_corr > MAX_NUM_STATIONS))
    {
            cb->sense = (ulong_t)trace.gdlc_ls_corr;
            error_log(cb, ERRID_SDL0032, NON_ALERT, 0, FN, LN);
            return(EINVAL);
    }


#ifdef MULT_PU
    station = (LINK_STATION *) cb->link_station_array[trace.gdlc_ls_corr];
#endif

    /* check for valid link station correlator */
    if (!exists(cb, station))
    {
	cb->sense = (ulong_t)station;
    	error_log(cb, ERRID_SDL0074, NON_ALERT, 0, FN, LN);
	return(EINVAL);
    }

    /* make sure physical link is opened */
    if (cb->pl_status == OPENED)
    {
	/* make sure link station correlator is valid	*/
	if (exists(cb, station))
	{
	    /* make sure link station is opened */
	    if (station->ll_status == OPENED)
	    {
	        /* if trace is being turned on */
	        if (trace.flags & DLC_TRCO)
	        {
		    /* turn on the the station trace flag */
		    station->ll.sls.flags |= DLC_TRCO;

		    /* set the trace channel	*/
		    station->ll.sls.trace_chan = trace.trace_chan;

		    /* if trace long was requested	*/
		    if (trace.flags & DLC_TRCL)
		        station->max_trace_size = TRACE_LONG_SIZE;
		    else
		        station->max_trace_size = TRACE_SHORT_SIZE;
	        }

	        else    /* clear the trace flag */
		    station->ll.sls.flags &= (~DLC_TRCO);
	    }
	    else    /* link station is not opened */
	    {
		error_log(cb, ERRID_SDL0031, NON_ALERT, 0, FN, LN);
		cb->rc = EINVAL;
	    }
    	}

	else    /* correlator is not valid */
	{
	    error_log(cb, ERRID_SDL0032, NON_ALERT, 0, FN, LN);
	    cb->rc = EINVAL;
	}

    }
    else    /* SAP has not been opened */
    {
	error_log(cb, ERRID_SDL0030, NON_ALERT, 0, FN, LN);
	cb->rc = EINVAL;
    }

#ifdef MULT_PU
    if (mpx->state & KERN)
#endif
    	bcopy(&trace, arg, sizeof(struct dlc_trace_arg));
    else
    {
    	rc = copyout(&trace, arg, sizeof(struct dlc_trace_arg));
    	if (rc)
    	{
		error_log(cb, ERRID_SDL8067, NON_ALERT, 0, FN, LN);
		cb->rc = EIO;
    	}
    }

    return(cb->rc);

}   /**** end of sdl_trace **********************************************/



/************************************************************************/
/*                                                                      */
/* Name:	enter_lbusy                                             */
/*                                                                      */
/* Function:	enter local busy mode                                   */
/*                                                                      */
/* Notes:	set local busy substate                                 */
/*                                                                      */
/* Data									*/
/* Structures:	sdlc port control block                                 */
/*                                                                      */
/* Returns:	                                                        */
/*                                                                      */
/************************************************************************/

#ifdef MULT_PU
int	enter_lbusy(cb, mpx, arg)

PORT_CB	*cb;
struct dlc_chan *mpx;
ulong	arg;

#endif

{
	LINK_STATION		*station;
	struct	dlc_corr_arg	corr;
	int	rc;

	cb->rc = NORMAL;

	/* get parameter block from user space		*/

#ifdef MULT_PU
	if (mpx->state & KERN)
#endif
		bcopy(arg, &corr, sizeof(struct dlc_corr_arg));
	else
	{
		cb->rc = copyin(arg, &corr, sizeof(struct dlc_corr_arg));
		if (cb->rc)
		{
	    		error_log(cb, ERRID_SDL8066, NON_ALERT, 0, FN, LN);
			cb->rc = EIO;
			return(cb->rc);
		}
	}

	/* check for valid link station correlator, defect 163696 */
	if ((corr.gdlc_ls_corr < 0) || (corr.gdlc_ls_corr > MAX_NUM_STATIONS))
	{
		cb->sense = (ulong_t)corr.gdlc_ls_corr;
		error_log(cb, ERRID_SDL0032, NON_ALERT, 0, FN, LN);
		return(EINVAL);
	}

	/* get link station address from correlator block	*/
#ifdef MULT_PU 
	station = (LINK_STATION *) cb->link_station_array[corr.gdlc_ls_corr];
#endif

	/* check for valid link station correlator */
	if (!exists(cb, station))
	{
		cb->sense = (ulong_t)station;
    		error_log(cb, ERRID_SDL006E, NON_ALERT, 0, FN, LN);
		return(EINVAL);
	}

	/* link station must be opened */
	if (station->ll_status != OPENED)
		return(EINVAL);

	/* set the station substate to local busy */
	station->sub_state |= DLC_LOCAL_BUSY;

	/* set the user requested local busy flag */
#ifdef MULT_PU
	station->rnr |= USER_SET_LBUSY;

	if (mpx->state & KERN)
#endif
		bcopy(&corr, arg, sizeof(struct dlc_trace_arg));
	else
	{
		rc = copyout(&corr, arg, sizeof(struct dlc_trace_arg));
		if (rc)
		{
			error_log(cb, ERRID_SDL8067, NON_ALERT, 0, FN, LN);
			cb->rc = EIO;
		}
	}

	return(cb->rc);
}




/************************************************************************/
/*                                                                      */
/* Name:	exit_lbusy                                              */
/*                                                                      */
/* Function:	exit the local busy mode                                */
/*                                                                      */
/* Notes:	clear the local busy substate                           */
/*		if there is data function call outstanding, retry it	*/
/*                                                                      */
/* Data									*/
/* Structures:	sdlc port control block                                 */
/*                                                                      */
/* Returns:	int                                                     */
/*                                                                      */
/************************************************************************/

#ifdef MULT_PU
int	exit_lbusy(cb, mpx, arg)

PORT_CB	*cb;
struct dlc_chan *mpx;
ulong	arg;

#endif

{
	LINK_STATION		*station;
	struct	dlc_corr_arg	corr;
	int	rc;

	cb->rc = NORMAL;

	/* get parameter block from user space		*/

#ifdef MULT_PU
	if (mpx->state & KERN)
#endif
		bcopy(arg, &corr, sizeof(struct dlc_corr_arg));
	else
	{
		cb->rc = copyin(arg, &corr, sizeof(struct dlc_corr_arg));
		if (cb->rc)
		{
	    		error_log(cb, ERRID_SDL8066, NON_ALERT, 0, FN, LN);
			cb->rc = EIO;
			return(cb->rc);
		}
	}

	/* check for valid link station correlator, defect 163696 */
	if ((corr.gdlc_ls_corr < 0) || (corr.gdlc_ls_corr > MAX_NUM_STATIONS))
	{
		cb->sense = (ulong_t)corr.gdlc_ls_corr;
		error_log(cb, ERRID_SDL0032, NON_ALERT, 0, FN, LN);
		return(EINVAL);
	}

	/* get link station address from correlator block	*/
#ifdef MULT_PU
	station = (LINK_STATION *) cb->link_station_array[corr.gdlc_ls_corr];
#endif


	/* check for valid link station correlator */
	if (!exists(cb, station))
	{
		cb->sense = (ulong_t)station;
    		error_log(cb, ERRID_SDL006D, NON_ALERT, 0, FN, LN);
		return(EINVAL);
	}

	/* link station must be opened */
	if (station->ll_status != OPENED)
		return(EINVAL);

	/* must be in local busy to exit local busy */
	if (!(station->sub_state & DLC_LOCAL_BUSY))
		return(EINVAL);

#ifdef MULT_PU
	/* clear any user initiated local busy */
	station->rnr &= ~USER_SET_LBUSY;

	if (station->i_m != DLC_NULL)
	{
		rc = (*mpx->rcvi_fa)(station->i_m, &station->i_block, mpx);

		if (rc == DLC_FUNC_OK)
		{
			/* clear the retry func */
			station->i_m = DLC_NULL;

			/* clear any dlc initiated local busy */
			station->rnr &= ~DLC_SET_LBUSY;

			/* remove station from the retry list if possible */
			del_retry(cb,station);

		}
		else /* still can't take the I-frame */
		{
			if (cb->rc == (ulong) DLC_FUNC_BUSY)
			{
				/* will wait for another DLC_EXIT_LBUSY
				   ioctl from the user */

				/* set "user" local busy mode */
				station->rnr |= USER_SET_LBUSY;
			}
			else /* assume DLC_FUNC_RETRY */
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

			/* error log the retry failure */
			cb->sense = rc;
			error_log(cb, ERRID_SDL0062, NON_ALERT, 0, FN, LN);
		}
	}

	/* if no more local busy conditions exist */
	if (station->rnr = 0)
	{
		/* clear the station local busy sub-state */
		station->sub_state &= (~DLC_LOCAL_BUSY);
	}


	if (mpx->state & KERN)
#endif

		bcopy(&corr, arg, sizeof(struct dlc_trace_arg));
	else
	{
		rc = copyout(&corr, arg, sizeof(struct dlc_trace_arg));
		if (rc)
		{
			error_log(cb, ERRID_SDL8067, NON_ALERT, 0, FN, LN);
			cb->rc = EIO;
		}
	}

	return(cb->rc);
}
