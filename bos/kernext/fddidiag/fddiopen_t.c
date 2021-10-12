static char sccsid[] = "@(#)87	1.1  src/bos/kernext/fddidiag/fddiopen_t.c, diagddfddi, bos411, 9428A410j 11/1/93 11:00:37";
/*
 *   COMPONENT_NAME: DIAGDDFDDI
 *
 *   FUNCTIONS: fddi_1st_acs_open
 *		fddi_1st_dd_open
 *		fddi_init_open_elem
 *		fddi_mpx
 *		fddi_open
 *		fddi_undo_1st_acs_open
 *		fddi_undo_1st_dd_open
 *		grow_open_tab
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1990,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "fddiproto.h"
#include <sys/errno.h>
#include <sys/priv.h>
#include <sys/dma.h>
#include <sys/malloc.h>
#include <sys/sleep.h>
#include <sys/iocc.h>
#include <sys/ioacc.h>
#include <sys/adspace.h>

/* 
 * get access to the global device driver control block
 */
extern fddi_ctl_t	fddi_ctl;
extern fddi_trace_t	fdditrace;
extern struct cdt	*p_fddi_cdt;

/*
 * NAME: fddi_mpx()
 *                                                                    
 * FUNCTION: Allocates a mpx channel number.  Checks for Diagnostic Mode
 *	requests.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine executes on the process thread.
 *                                                                   
 * NOTES:
 *	This routine will allocate a channel number and it's corresponding
 *	open element (fddi_open_t).  The open element will be paritally
 *	initialized and put into the global open table.
 *
 *	The open element is partially initialized since not all the 
 *	necessary information will not be available until the open 
 *	entry point is called.  We do not fill in the ACS ptr in the
 *	open element so as to keep us from trying to give this user
 *	asynchronous status information before the open entry point
 *	gives us all the necessary information.
 *
 *	On the last de-allocate (close) for this device (minor number), 
 * 	this routine will:
 *		free all left over netid elements from 
 *		  previously failed starts
 *		set state machines
 *		shutdown the adapter card
 *		disable the adapter
 *		un-register the slih
 *
 *	On the last close for the device driver, this routine will:
 *		unregister the component dump areas
 *		unregister the component dump table
 *		unpin the code
 *
 * RECOVERY OPERATION:
 *	None.
 *
 * DATA STRUCTURES:
 *	Data structures accessed and modified by this routine:
 *		global FDDI control structure
 *		global Adapter Control Structure for this minor number
 *
 * ROUTINES CALLED:
 *	minor(), devswadd(), devswdel(), lockl(),
 *
 * RETURNS: 
 *		0	- successful
 *		EPERM	- Indicates that the device was already open
 *			  and that the Diagnostic mode open request
 *			  was denied
 *		EACCES	- Indicates that the user did not have root
 *			  authority and that the Diagnostic Mode open
 *			  request was denied.
 *		ENODEV	- Indicates an invalid minor number was specified
 *		EBUSY	- Unable to allocate required resources
 *		ENOMEM	- Unable to allocate required memory
 */  

int
fddi_mpx(dev_t devno,		/* major and minor number */
	int *p_chan,		/* addr for returning allocated channel num */
	char *p_channame)	/* ptr to special file name suffix */

{
	register fddi_acs_t 	*p_acs;
	register fddi_open_t	*p_open;
	int 			adap, rc;
	chan_t			chan;
	extern	int		fddi_enter_diag_cmplt();
	fddi_cmd_t 		*p_cmd;

	FDDI_TRACE("MpxB", devno, p_chan, *p_channame);

	/* sanity check the minor number */
	if ( ((adap = minor(devno)) < 0) || (adap >= FDDI_MAX_MINOR) )
	{
		FDDI_TRACE("Mpx1", adap, 0, 0);
		return(ENODEV);
	}

	/* 
	 * grab device lock to serialize with the 
	 * 	config entry point
	 */
	rc = lockl(&fddi_ctl.fddilock, LOCK_SIGRET);
	if( rc != LOCK_SUCC )
	{
		return(EINTR);
	}
	/*
	 * sanity check the ACS ptr
	 */
	if ((p_acs = fddi_ctl.p_acs[adap]) == NULL)
	{
		FDDI_TRACE("Mpx2", adap, 0, 0);
		unlockl(&fddi_ctl.fddilock);
		return(ENODEV);
	}
	if (p_channame == NULL) /* this is de-allocate request */
	{
		/*
		 * sanity checks and get open pointer
		 */
		ASSERT ((*p_chan < fddi_ctl.open_tab_size))
		p_open = fddi_ctl.p_open_tab [*p_chan];
		ASSERT (p_open) 

		/*
		 * remove pointer to this open structure from table
		 */
		fddi_ctl.p_open_tab[*p_chan] = NULL;
		fddi_ctl.open_cnt--;
		p_acs->ctl.open_cnt--;

		if (p_fddi_cdt)
		{
			fddi_cdt_del(p_open);
		}
		/*
		 * free the status queue 
		 */
		if(p_open->p_statq != NULL)
		{
			xmfree( p_open->p_statq, pinned_heap );
		}
		/*
		 * free the open element
		 */
		xmfree( p_open, pinned_heap );

		/*
		 * Check if this is the last deallocate for this adapter
		 */
		if (p_acs->ctl.open_cnt == 0)
		{	
			/*
			 * open count is zero so incase this was DIAG MODE
			 *	reset mode.
			 */

			/*
			 * Make sure that we got through the
			 *	1st Acs open before we try and undo 
			 * 	any more stuff.
			 */
			if (p_acs->ctl.first_open == FALSE)
			{
				/* 
				 * last close for this ACS
				 * undo the things that have been done up 
				 * to this point
				 */
				if (p_acs->ctl.mode == 'C')
				{
					/*
	 				* set up command parameter block
	 				*/
					p_cmd = &(p_acs->dev.cmd_blk);
					p_cmd->stat = 0;
					p_cmd->pri = 0;
					p_cmd->cmplt = 
						(int(*)())fddi_enter_diag_cmplt;
					p_cmd->cpb_len = 0;
					p_cmd->cmd_code = FDDI_HCR_DIAG_MODE;
					(void)fddi_send_cmd(p_acs,p_open,p_cmd);
				}
				else
					fddi_shutdown_adap(p_acs);
				fddi_undo_1st_acs_open(p_acs);
				p_acs->ctl.first_open = TRUE;
			}

			p_acs->ctl.mode = NULL;	

			if (fddi_ctl.open_cnt == 0)
			{	
				/* 
				 * last close for the device driver
				 * undo the things that were done on the 
				 * first open.  
				 */
				if (fddi_ctl.first_open == FALSE)
				{
					fddi_undo_1st_dd_open();
					fddi_ctl.first_open = TRUE;
				}
			}
		} /* end last close for this ACS */

	}
	else /* this is allocate request */
	{
		/* 
		 * If p_channame points to NULL this is
		 * a normal open request.  If p_channame points
		 * to a 'D', this is diagnostic open request.
		 *
		 * We allow the request for normal if the
		 * this minor has not already been opened 
		 * in diagnostic mode (p_acs->ctl.mode = 'D').
		 *
		 * We allow the request for diagnostics if this
		 * is the first open request for this minor
		 * number.
		 */
		if (((*p_channame == '\0') && (p_acs->ctl.mode != 'D') &&
						(p_acs->ctl.mode != 'C')) ||
		    ((*p_channame == 'D') && (p_acs->ctl.open_cnt == 0)) ||
		    ((*p_channame == 'C') && (p_acs->ctl.open_cnt == 0)))
		{ 
			/*
			 * check for 1st open
			 */
			if ( p_acs->ctl.open_cnt == 0 )
			{
				/* 
				 * if diagnostic mode request, check user's 
				 * 	authority.
				 */
				if (((*p_channame=='D') || (*p_channame=='C'))
					&& (rc = privcheck(RAS_CONFIG)))
				{
					FDDI_TRACE("Mpx4", rc,0,0);
					unlockl(&fddi_ctl.fddilock);
					return (EACCES);
				}

				/* 
				 * first open so save the mode
				 */
				p_acs->ctl.mode = *p_channame;
			} 
			/*
			 * Allocate open structure
			 */
			p_open = xmalloc(sizeof(fddi_open_t), FDDI_WORDSHIFT,
					pinned_heap);
			if (p_open == NULL)
			{
				FDDI_TRACE("Mpx5", rc,0,0);
				unlockl(&fddi_ctl.fddilock);
				return (ENOMEM);
			}
			/* 
			 * NB:
			 *	We do not save the p_acs ptr in
			 *	the open element yet.  We do
			 *	not have all the neccessary information
			 *	to describe this user (i.e. kernel
			 *	mode or user mode).  By not putting
			 *	the ACS ptr in the open element, it
			 *	prevents us from trying to pass 
			 *	asynchronous status to this user
			 *	before the open entry point gets 
			 *	called.
			 */
			
			/*
			 * Find slot in the open table to allocate
			 * The slot index will be the channel number.
			 */
			for (chan = 0; chan < fddi_ctl.open_tab_size; chan++)
			{
				if (fddi_ctl.p_open_tab[chan] == NULL)
                                {
                                        break;
                                }
                        }
                        if (chan == fddi_ctl.open_tab_size)
                        {
				/* 
				 * grow open table size if nessecary 
				 */
				rc = grow_open_tab();
				if (rc)
				{
					FDDI_TRACE("Mpx6", rc,0,0);
					unlockl(&fddi_ctl.fddilock);
					return (rc);
				}
			}
			/* allocate channel */
			fddi_ctl.p_open_tab[chan] = p_open;

                        /* initialize p_open */
                        bzero(p_open, sizeof(fddi_open_t) );
                        p_open->chan = chan;
                        p_open->devno = devno;

			/* return channel number */
			*p_chan = chan;

			/* increment counters */
			fddi_ctl.open_cnt++;
			p_acs->ctl.open_cnt++;
		} 			
		else
		{
			/*
			 * Allocation failed: invalid request
			 */
			if (*p_channame == 'D')
				rc = EPERM;
			else	
				rc = EINVAL; 	
			FDDI_TRACE("Mpx7", rc,0,0);
		}
	} /* end if allocate reqest */

	/* unlockl global driver lock */
	unlockl(&fddi_ctl.fddilock);

	FDDI_TRACE("Mpxl", *p_chan, p_acs, 0);
	FDDI_TRACE("MpxE", p_open, rc, *p_chan);
	return(rc);

} /* end fddimpx */

/*
 * NAME: fddi_open()
 *                                                                    
 * FUNCTION: Open entry point from kernel.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine runs on the process thread.  It can page fault.
 *	It serializes with the config and close entry points via the 
 *	global driver lock (fddi_ctl.fddilock).
 *                                                                   
 *                                                                   
 * NOTES:
 *	On the 1st open for the FDDI device driver, this routine will:
 *		pin the driver code
 *		register the dump routine
 *		add to the component dump table
 *
 *	On the 1st open for this minor number, this routine will:
 *		register the slih
 *		enable the card
 *		initilize host tx descriptors
 *		initilize host rcv descriptors
 *		allocate other resources as required
 *
 *	On each opens, this routine will:
 *		init the open element
 *
 *
 * RECOVERY OPERATION: NONE
 *
 * DATA STRUCTURES: 
 *	Data structures accessed by this routine:
 *		global FDDI control structure
 *		global Adapter Control Structure for this minor number
 *		open element for this user
 *
 * ROUTINES CALLED:
 *	minor(), lockl(), xmalloc(), fddi_dev_first_open()
 *
 * RETURNS: 
 *		0	- successful
 *		ENODEV	- Indicates that an invalid minor number 
 *			  or channel was specified
 *		EINVAL	- Indicates that an invalid parameter was specified
 *		ENOMEM	- Indicates that the device driver was unable to 
 *			  allocate the required memory
 *
 */  

int
fddi_open( dev_t	devno,		/* major and minor number */
	   ulong	devflag,	/* flags...DKERNEL, DNDELAY */
	   chan_t	chan,		/* mpx channel number */
	   cio_kopen_ext_t *p_ext)	/* may be ptr to open extension */
{
	int adap, rc;
	register fddi_acs_t *p_acs;
	register fddi_open_t *p_open;


	FDDI_TRACE("OpnB", devno, chan, p_ext);

	/* sanity check the minor number 
	 */
	if ( ((adap = minor(devno)) < 0) || (adap >= FDDI_MAX_MINOR) )
	{
		return(ENODEV);
	}
	/* grab global device lock */
	rc = lockl(&fddi_ctl.fddilock, LOCK_SIGRET);
	if( rc != LOCK_SUCC )
	{
		return(EINTR); 
	}


	if ((p_acs = fddi_ctl.p_acs[adap]) == NULL)
	{
		/* unlockl global driver lock */
		unlockl(&fddi_ctl.fddilock);
		FDDI_TRACE("Opn1", adap, ENODEV, 0);
		return(ENODEV);
	}


	if ((p_open = fddi_ctl.p_open_tab [chan]) == NULL)
	{
		/* unlockl global driver lock */
		unlockl(&fddi_ctl.fddilock);
		FDDI_TRACE("Opn2", p_acs, chan, EINVAL);
		return(EINVAL);
	}

	if (p_open->p_acs != NULL)
	{
		/* unlockl global driver lock */
		unlockl(&fddi_ctl.fddilock);
		FDDI_TRACE("Opn3", p_acs, p_open, p_open->p_acs);
		return(EINVAL);
	}
		
	/* 
	 * do 1st FDDI device driver open stuff if needed 
	 */
	if ( fddi_ctl.first_open )
	{
		rc = fddi_1st_dd_open();
		if ( rc != 0 )
		{
			/* unlockl global driver lock */
			unlockl(&fddi_ctl.fddilock);

			FDDI_TRACE("Opn5", p_acs, p_open, rc);
			return(rc);
		}
		fddi_ctl.first_open = FALSE;
	} /* end if 1st open for the device driver */


	/* 
	 * do 1st adapter open stuff if needed
	 */
	if ( p_acs->ctl.first_open )
	{
		rc = fddi_1st_acs_open(p_acs, devno);
		if ( rc != 0 )
		{
			
			/* unlockl global driver lock */
			unlockl(&fddi_ctl.fddilock);

			FDDI_TRACE("Opn6", p_acs, p_open, 0);
			return(rc);
		}
		p_acs->ctl.first_open = FALSE;
	} /* end if 1st open for this minor number */

	if ( !(devflag & DKERNEL) )
	{
		/* malloc the user's status que 
		 * from the pinned heap
		 */
		p_open->p_statq=xmalloc( (sizeof(cio_stat_blk_t) * 
					 p_acs->dds.stat_que_size), 
					 FDDI_WORDSHIFT, pinned_heap);

		if ( p_open->p_statq == NULL )
		{
			/* if there is only 1 outstanding open
			 * for this minor number (adapter), we need to
			 * undo what was done for the 1st open for this
			 * adapter.
			 */
			if ( p_acs->ctl.open_cnt == 1 )
			{
				fddi_undo_1st_acs_open(p_acs);
				p_acs->ctl.first_open = TRUE;
	
			}

			/* if there is only 1 outstanding open for the
			 * entire FDDI device driver, we need to undo
			 * what was done for the 1st open for the
			 * FDDI device driver 
			 */
			if ( fddi_ctl.open_cnt == 1 )
			{
				fddi_undo_1st_dd_open();
				fddi_ctl.first_open = TRUE;
	
			}
			/* unlockl global driver lock */
			unlockl(&fddi_ctl.fddilock);
			FDDI_TRACE("Opn7", p_acs, p_open, 0);
			return(ENOMEM);

		} /* end if open element allocation failure */
	} /* end if user mode */

	/* 
	 * initialize the open element 
	 */
	fddi_init_open_elem (p_acs, p_open, devflag, p_ext);

	/* add open elem to component dump table */
	fddi_cdt_add ("Open", p_open, sizeof (fddi_open_t));

	/* 
	 * if we are in some form of limbo or if we've had a hard failure,
	 * 	let the user know via async status block.
	 */
	if ((p_acs->dev.state == FDDI_DEAD ) ||
	    (p_acs->dev.state == FDDI_LIMBO))
	{
		fddi_report_status(p_acs, p_open, &p_acs->ctl.limbo_blk);
	}

	if (p_acs->dev.state == FDDI_LLC_DOWN)
	{
		cio_stat_blk_t	sb; 		/* status blk */

		sb.code = CIO_ASYNC_STATUS;
		sb.option[0] = FDDI_RING_STATUS;
		sb.option[1] = FDDI_LLC_DISABLE;

		fddi_report_status(p_acs, p_open, &sb);
	}


	/* unlockl global driver lock */
	unlockl(&fddi_ctl.fddilock);

	FDDI_TRACE("OpnE", p_acs, p_open, rc);
	return(rc);

} /* end fddi_open() */

/*
 * NAME: fddi_init_open_elem()
 *                                                                    
 * FUNCTION: 
 *	Initializes the passed in open element.
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt environment
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION:  NONE
 *
 * DATA STRUCTURES: 
 *	manipulates the passed in open element.
 *
 * RETURNS: VOID
 *	
 */  
void
fddi_init_open_elem (	
	fddi_acs_t	*p_acs,
	fddi_open_t	*p_open,
	ulong		devflag,
	cio_kopen_ext_t	*p_ext)
{
	int i;

	FDDI_TRACE("OioB", p_acs, p_open,devflag);

	p_open->devflag = devflag;
	p_open->tx_fn_needed = FALSE;
	p_open->tx_acks = 0;
	p_open->selectreq = 0;
	p_open->netid_cnt = 0;

	/*
	 * If kernel user, initialize the kernel mode portion
	 * of the open element.
	 */
	if ( p_open->devflag & DKERNEL )
	{
		p_open->open_id = p_ext->open_id;
		p_open->rcv_fn = p_ext->rx_fn;
		p_open->stat_fn = p_ext->stat_fn;
		p_open->tx_fn = p_ext->tx_fn;
	}

	/* initialize the user mode portion of
	 * the open element
	 */
	p_open->p_rcv_q_head = NULL;
	p_open->p_rcv_q_tail = NULL;
	p_open->rcv_cnt = 0;
	p_open->rcv_event = EVENT_NULL;
	p_open->stat_cnt = 0;
	p_open->stat_ls_cnt = 0;
	p_open->stat_que_ovflw = FALSE;
	p_open->stat_in = 0;
	p_open->stat_out = 0;
	p_open->stat_cnt = 0;
	p_open->addrs = 0;
	p_open->p_acs = p_acs;
	FDDI_TRACE("OioE", p_ext, p_open->rcv_fn, p_open->stat_fn);

	return;
} /* end fddi_init_open_elem() */

/*
 * NAME:     fddi_1st_dd_open()
 *
 * FUNCTION: 
 * 	Does the necessary functions for handling the very 1st
 *	open to the FDDI device driver.
 *
 * EXECUTION ENVIRONMENT:
 *	The routine can execute on the process thread only.
 *
 * NOTES:
 *	This routine will:
 *		pin the driver code
 *		initialize the component dump table
 *		register the component dump routine
 *		add an entry in the table
 *
 * ROUTINES CALLED:
 *	pincode(),
 * RETURNS:
 *		return code from pincode
 */
int
fddi_1st_dd_open( )
{
	int rc;

	FDDI_TRACE("OdoB", 0, 0, 0);

	rc = pincode(fddi_slih);
	if (rc != 0)
	{
		return(ENOMEM);
	}
	/* 
	 * initialize compenent dump table 
	 * 	register component dump routine 
	 * 	add the fddi control area for dump to the dump table 
	 */
	rc = fddi_cdt_init ();
	if (rc != CIO_OK)
	{
		unpincode(fddi_slih);
		return(rc);
	}

	FDDI_TRACE("OdoE", 0, 0, rc);

	return (rc);

} /* end fddi_1st_dd_open() */
/*
 * NAME:     fddi_1st_acs_open()
 *
 * FUNCTION: 
 * 	Does the necessary functions for handling the very 1st
 *	open to a minor number (device/ACS).
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *	This routine will:
 *		initialize DMA 
 *		register the slih
 *		enable the card
 *		initilize tx descriptors
 *		initilize rcv descriptors
 *
 * ROUTINES CALLED:
 *	
 * RETURNS:
 *		0 	- successful
 *		ENOMEM	- unable to allocate required memory
 */

int
fddi_1st_acs_open( 
	fddi_acs_t	*p_acs,
	dev_t		devno)		/* not used */
{
	int 	iocc;
	uint	ioa;
	int 	rc;
	ushort	hsr;
	int	i;

	FDDI_TRACE("OaoB", p_acs, 0,0);

	/* 
	 * initialize DMA via d_init(), d_unmask() 
	 */
	p_acs->dev.dma_channel = d_init(p_acs->dds.dma_lvl, 
					MICRO_CHANNEL_DMA, 
					p_acs->dds.bus_id);
	if (p_acs->dev.dma_channel == DMA_FAIL)
	{
		/*
		 * log unknown error, set footprint, trace
		 * and return error.
		 */
		FDDI_TRACE("Oao1", p_acs, p_acs->dev.dma_channel,0);
		return(EFAULT);
	}
	d_unmask(p_acs->dev.dma_channel);

	/* 
	 * initialize the ACS 
	 */
	p_acs->dev.limbo_to = FALSE;
	p_acs->ctl.addr_index = 0;

	p_acs->dev.smt_event_mask = FDDI_SMT_EVNT_MSK;
	p_acs->dev.smt_error_mask = FDDI_SMT_ERR_MSK;
	p_acs->dev.piowall = FALSE;
	p_acs->dev.carryover = FALSE;
	p_acs->dev.stest_cover = FALSE;

	/* 
	 * allocate timers
	 * set up watch dog timer
	 */
	p_acs->dev.ioctl_event = EVENT_NULL;
	p_acs->dev.close_event = EVENT_NULL;
	p_acs->dev.limbo_event = EVENT_NULL;

	/* 
	 * initialize the DOWNLOAD watchdog timer 
	 */
	p_acs->dev.dnld_wdt.next = NULL;
	p_acs->dev.dnld_wdt.prev = NULL;
	p_acs->dev.dnld_wdt.restart = 0;
	p_acs->dev.dnld_wdt.count = 0;
	p_acs->dev.dnld_wdt.restart = FDDI_DNLD_WDT_RESTART;
	p_acs->dev.dnld_wdt.func = (void(*)())fddi_dnld_to; 
	w_init( &p_acs->dev.dnld_wdt);

	/* 
	 * initialize the COMMAND watchdog timer 
	 */
	p_acs->dev.cmd_wdt.next = NULL;
	p_acs->dev.cmd_wdt.prev = NULL;
	p_acs->dev.cmd_wdt.restart = FDDI_CMD_WDT_RESTART;
	p_acs->dev.cmd_wdt.count = 0;
	p_acs->dev.cmd_wdt.func = (void(*)())fddi_cmd_to;
	w_init( &p_acs->dev.cmd_wdt);

	/* 
	 * initialize the close watchdog timer 
	 */
	p_acs->dev.close_wdt.next = NULL;
	p_acs->dev.close_wdt.prev = NULL;
	p_acs->dev.close_wdt.restart = FDDI_TX_WDT_RESTART;
	p_acs->dev.close_wdt.count = 0;
	p_acs->dev.close_wdt.func = (void(*)())fddi_close_wait;
	w_init( &p_acs->dev.close_wdt);

	/* 
	 * initialize the limbo watchdog timer 
	 */

#define FDDI_LIMBO_RESET_TIME	(15)	/* 15 secs to reset & run self tests */
	p_acs->dev.limbo_wdt.next = NULL;
	p_acs->dev.limbo_wdt.prev = NULL;
	p_acs->dev.limbo_wdt.restart = FDDI_LIMBO_RESET_TIME;
	p_acs->dev.limbo_wdt.count = 0;
	p_acs->dev.limbo_wdt.func = (void(*)())fddi_limbo_to;
	w_init( &(p_acs->dev.limbo_wdt) );

	/* initialize the smt thresholds */
	p_acs->dev.thresh_rtt = 0;
	p_acs->dev.thresh_trc = 0;
	p_acs->dev.thresh_stuck = 0;
	p_acs->dev.thresh_tme = 0;
	p_acs->dev.thresh_sbf = 0;

	/*
	 * For setup (allocate) the DMA space for this adapter:
	 * see fddi.design
	 */

	p_acs->rcv.l_adj_buf = d_roundup (FDDI_MAX_PACKET);
	p_acs->tx.p_d_sf = p_acs->dds.dma_base_addr + 
			   (FDDI_MAX_RX_DESC * p_acs->rcv.l_adj_buf);

	if (p_acs->tx.p_d_sf & (PAGESIZE - 1))
	{
		/* round up */
		p_acs->tx.p_d_sf &= (~(PAGESIZE - 1));
		p_acs->tx.p_d_sf += PAGESIZE;
	}
	p_acs->tx.p_d_base = p_acs->tx.p_d_sf + FDDI_SF_CACHE;
	p_acs->dev.p_d_kbuf[0] = p_acs->tx.p_d_base;
	p_acs->dev.p_d_kbuf[1] = p_acs->dev.p_d_kbuf[0]+FDDI_MAX_DMA_PARTITION;
	p_acs->dev.p_d_kbuf[2] = p_acs->dev.p_d_kbuf[1]+FDDI_MAX_DMA_PARTITION;

	/* initialize TX */
	if( rc=fddi_init_tx(p_acs) )	
	{
		/*
		 * undo DMA initialization and watchdog timer.
		 * 	(fddi_init_tx() undoes itself on failure)
		 */
		w_clear( &p_acs->dev.dnld_wdt );
		w_clear( &p_acs->dev.cmd_wdt );
		w_clear( &p_acs->dev.close_wdt );
		w_clear( &p_acs->dev.limbo_wdt );
		d_clear(p_acs->dev.dma_channel);
		FDDI_TRACE("Oao2", p_acs, rc, 0);
		return(rc);
	}
	/* 
	 * initialize rcv,
	 */
	if( rc=fddi_init_rcv(p_acs) )
	{
		/* undo TX stuff that was just done */
		/*
		 * undo DMA initialization, deallocate
		 * timers, and watchdog timer.
		 */
		fddi_undo_init_tx(p_acs);
		w_clear( &p_acs->dev.dnld_wdt );
		w_clear( &p_acs->dev.cmd_wdt );
		w_clear( &p_acs->dev.close_wdt );
		w_clear( &p_acs->dev.limbo_wdt );
		d_clear(p_acs->dev.dma_channel);
		FDDI_TRACE("Oao3", p_acs, rc, 0);
		return(rc);
	}
	/* zero out the RAS counters */
	bzero(&(p_acs->ras), sizeof(fddi_stats_t) );

	/* 
	 * register slih  
	 */
	p_acs->ihs.next = NULL;
	p_acs->ihs.handler = (int(*)())fddi_slih; 
	p_acs->ihs.bus_type = p_acs->dds.bus_type;
	p_acs->ihs.flags =0;
	p_acs->ihs.level = p_acs->dds.bus_intr_lvl;
	p_acs->ihs.priority = INTCLASS2;
	p_acs->ihs.bid = p_acs->dds.bus_id;
	if (rc = i_init ((struct intr *) (&p_acs->ihs)) != INTR_SUCC ) 
	{
		fddi_undo_init_tx(p_acs);
		fddi_undo_init_rcv(p_acs);
		w_clear( &p_acs->dev.dnld_wdt );
		w_clear( &p_acs->dev.cmd_wdt );
		w_clear( &p_acs->dev.close_wdt );
		w_clear( &p_acs->dev.limbo_wdt );
		d_clear(p_acs->dev.dma_channel);
		FDDI_TRACE("Oao4", p_acs, rc,0);
		return(rc);
	}
	
	for (i=0; i<FDDI_MAX_ADDRS; i++)
		p_acs->ctl.addrs[i].cnt = 0;

	/*
	 * register our offlevel handler:
	 * The following macro can be used to initialize an intr structure
	 * so that an off-level request can be scheduled for class 0.
	 * An off-level request is scheduled via i_sched.
	 */
	INIT_OFFL1 (&p_acs->dev.ihs, fddi_oflv, p_acs->dds.bus_id);
	/* 
	 * ADD to component dump table 
	 */
	fddi_cdt_add ("ACS", p_acs, sizeof (fddi_acs_t));

	/*  
	 * enable the card 
	 */
	iocc = IOCC_ATT(p_acs->dds.bus_id, IO_IOCC + (p_acs->dds.slot << 16));
	PIO_PUTCX(iocc+FDDI_POS_REG2, (p_acs->dev.pos2 | FDDI_POS2_CEN));
	IOCC_DET(iocc);

	ioa = BUSIO_ATT (p_acs->dds.bus_id, p_acs->dds.bus_io_addr);

	/*
	 * Read in the hsr to clear it of the misc. interrupts the 
	 * card generates when first enabled.
	 */
	PIO_GETSRX(ioa, &hsr);

	/*
	 * Unmask  interrupts for CCI (activation) and for ERRORS
	 * If we had an interrupt waiting it will happen inbetween
	 *	the TRACE calls.
	 */
	FDDI_TRACE("Oao5", p_acs, 0, 0);
	PIO_PUTSRX (ioa + FDDI_HMR_REG, FDDI_HSR_BASE_INTS);
	BUSIO_DET (ioa);
	FDDI_TRACE("Oao6", p_acs, 0, 0);

	FDDI_TRACE("OaoE", p_acs, 0,0);
	return(0);

} /* end fddi_1st_acs_open() */

/*
 * NAME:     fddi_undo_1st_dd_open()
 *
 * FUNCTION: 
 *	Undoes what was done by the fddi_1st_dd_open() routine. 	
 *
 *
 * EXECUTION ENVIRONMENT:
 *	This routine can execute on the process thread only.
 *
 * NOTES:
 *	This routine will:
 *		remove an entry from the component dump table
 *		un-register the component dump routine
 *		unpin the driver code
 *
 * ROUTINES CALLED:
 *	
 * RETURNS:
 *		return code from unpincode()
 */
int
fddi_undo_1st_dd_open( )
{
	FDDI_TRACE("OudB", 0, 0,0);

	/* 
	 * remove the FDDI control area from the dump table
	 */

	/* 
	 * un-register the component dump routine
	 */
	fddi_cdt_undo_init ();

	/* unpin the driver code */
	unpincode(fddi_slih);

	FDDI_TRACE("OudE", 0, 0,0);
	return(0);

} /* end fddi_undo_1st_dd_open() */
/*
 * NAME:     fddi_undo_1st_acs_open()
 *
 * FUNCTION: 
 *	Undoes what was done by the fddi_1st_acs_open() routine. 	
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *	This routine will:
 *		remove from the component dump table
 *		unregister the slih
 *		disable the card
 *		clear tx descriptors
 *		clear rcv descriptors
 *
 * ROUTINES CALLED:
 *	
 * RETURNS:
 */
int
fddi_undo_1st_acs_open( fddi_acs_t	*p_acs)		/* ACS pointer */
{
	int ioa;

	FDDI_TRACE("OuaB", p_acs, 0,0);

	/* clear TX and RCV */
	fddi_undo_init_tx(p_acs);
	fddi_undo_init_rcv(p_acs);

	/* 
	 * Stop and de-allocate timers
	 */
	w_stop( &p_acs->dev.dnld_wdt );
	w_stop( &p_acs->dev.cmd_wdt );
	w_stop( &p_acs->dev.close_wdt );
	w_stop( &p_acs->dev.limbo_wdt );

	w_clear( &p_acs->dev.dnld_wdt );
	w_clear( &p_acs->dev.cmd_wdt );
	w_clear( &p_acs->dev.close_wdt );
	w_clear( &p_acs->dev.limbo_wdt );

	/* un-initialize DMA via d_clear()  */
	d_clear(p_acs->dev.dma_channel);

	/* remove the ACS from the dump table */
	fddi_cdt_del (p_acs);

	/* un register slih  */
	i_clear( (struct intr *)(&p_acs->ihs) ); 

	/* ok */
	FDDI_TRACE("OuaE", p_acs, 0,0);
	return (0);

} /* end fddi_undo_1st_acs_open() */

/*
 * NAME: grow_open_tab()
 *                                                                    
 * FUNCTION: expand the size of the open table
 *                                                                    
 * EXECUTION ENVIRONMENT: process environment
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION:  NONE
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 *		0 	- success
 *		ENOMEM  - out of memory
 */  
static int
grow_open_tab ()
{
	fddi_open_t	*p_new;
	fddi_open_t	*p_old;

	/* save old table */
	p_old = fddi_ctl.p_open_tab;

	/* allocate new FDDI_OPEN_TAB_SIZE table */
	p_new = xmalloc (sizeof (fddi_open_t *) * (fddi_ctl.open_tab_size + 
			FDDI_OPEN_TAB_SIZE), FDDI_WORDSHIFT, pinned_heap);

	if (p_new == NULL)
	{
		return (ENOMEM);
	}

	bzero(p_new, ( sizeof(fddi_open_t *) * (fddi_ctl.open_tab_size + 
			FDDI_OPEN_TAB_SIZE)) );
	if (p_old != 0)
	{
		/* Not first time; copy old table into new table */
		bcopy ( p_old, p_new, 
			fddi_ctl.open_tab_size * sizeof (fddi_open_t *));

		/*
		 * delete old table from dump
		 */
		fddi_cdt_del (p_old);

		/* free old table */
		xmfree (p_old, pinned_heap);

		/*
		 * Add new open table to dump 
		 */
		fddi_cdt_add ("Opentab", p_new,
			(fddi_ctl.open_tab_size + FDDI_OPEN_TAB_SIZE) 
			* sizeof (fddi_open_t *));
	}
	/* load new table */
	fddi_ctl.p_open_tab = p_new;
	fddi_ctl.open_tab_size += FDDI_OPEN_TAB_SIZE;

	/*
	 * Don't add this structure to the dump table yet
	 *	because it hasn't been initialized...
	 *	It is added in fddi_cdt_init()
	 */

	return (0);
}
