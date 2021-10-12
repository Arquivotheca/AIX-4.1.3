static char sccsid[] = "@(#)67	1.3  src/bos/kernext/fddi/fddi_cmd.c, sysxfddi, bos411, 9428A410j 3/23/94 15:03:44";
/*
 *   COMPONENT_NAME: SYSXFDDI
 *
 *   FUNCTIONS: cmd_handler
 *		fddi_cmd_to
 *		issue_cmd
 *		issue_pri_cmd
 *		send_cmd
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/sleep.h>
#include "fddiproto.h"

/*
 * NAME: send_cmd
 *                                                                    
 * FUNCTION: Issues an hcr command for a user
 *                                                                    
 * EXECUTION ENVIRONMENT: called from process environment only
 *                                                                   
 * NOTES: 
 *	It will sleep if there is another command from a user currently in
 *	process.  If not it will set this command up to be issued and call
 *	issue command.  It will then sleep.  The individual command completion
 *	routine must wake up the user when it is finished.
 *
 * RETURNS: 
 *
 */  
void
send_cmd(
	fddi_acs_t	*p_acs,
	fddi_cmd_t	*p_cmd)
{
	int		ipri;
	int		bus;

	
	FDDI_TRACE("DscB",p_acs,p_cmd->cmd_code,0);

	p_acs->dev.cmd_status = 0;

	/*
	 * If there is a command from a user in process go to sleep.  Multiple
	 * commands can be sleeping waiting to be issued, first one woken can
	 * be issued.
	 */
	while (p_acs->dev.cmd_blk.cmd_code != 0)
	{
		e_sleep_thread (&p_acs->dev.cmd_wait_event, 
			&p_acs->dev.cmd_lock, LOCK_HANDLER);
	
		if (p_acs->dev.state != OPEN_STATE)
		{
			p_acs->dev.cmd_status = EINVAL;
			FDDI_ETRACE("Dsc2",p_acs,p_acs->dev.state,0);
			return;
		}
	}

	p_acs->dev.cmd_blk = *p_cmd;

	/* 
	 * issue the command
	 */
	bus = BUSMEM_ATT(p_acs->dds.bus_id, (ulong)p_acs->dds.bus_mem_addr);
	if (issue_cmd(p_acs, &p_acs->dev.cmd_blk, bus, TRUE))
	{
		p_acs->dev.cmd_status = EINVAL;
		FDDI_ETRACE("Dsc3",p_acs,p_acs->dev.state, 
			p_acs->dev.cmd_blk.cmd_code);
		BUSMEM_DET(bus);
		return;
	}
	BUSMEM_DET(bus);
	
	/* 
	 * sleep until the completion routine wakes us or the command times out
	 */
	e_sleep_thread(&p_acs->dev.cmd_event, &p_acs->dev.cmd_lock,
		LOCK_HANDLER);

	FDDI_TRACE("DscE",p_acs,p_cmd->cmd_code,0);
	return;
}

/*
 * NAME: issue_cmd
 *                                                                    
 * FUNCTION: issue a command to the adapter 
 *                                                                    
 * EXECUTION ENVIRONMENT: 
 *
 *	Process or interrupt environment.
 *	Interrupts must be disabled to the level of the SLIH before 
 *	calling this routine.
 *                                                                   
 * NOTES: 
 *
 *	Issue the command passed in. Set the 'command in progress' pointer,
 *	start the watchdog timer, get the set count from shared memory and
 *	issue the command.
 *
 *	If a command is in progress then this must be a priority command
 *	trying to go out when a regular command is yet to finish. This is
 *	not a problem because all priority commands are 'queued' in the
 *	pri_que member of the acs.dev struct.
 *
 *	If there is a priority command in progress return to the user command
 *	The users command will be issues by the cmd_handler when the priority
 *	command has completed.
 *
 * RETURNS: 
 *	Boolean set to whether or not there has been a pio failure
 */  

int
issue_cmd (
	fddi_acs_t	*p_acs,
	fddi_cmd_t	*p_cmd, 
	int		bus,
	int		cmd_flag)
{
	int 	ioa;
	int	ipri;

	if (cmd_flag == FALSE)
		ipri=disable_lock(CFDDI_OPLEVEL,&p_acs->dev.cmd_lock);
	FDDI_TRACE("DicB",p_acs,p_cmd->cmd_code,bus);

	FDDI_ASSERT((bus != NULL))
	FDDI_ASSERT((cmd_flag == TRUE || cmd_flag == FALSE));

	if (p_acs->dev.p_cmd_prog)
	{
		/* not yet: command in progress */
		FDDI_TRACE("Dic1",p_acs->dev.p_cmd_prog->cmd_code,0,0);
		return(p_acs->dev.pio_rc);
	}

	/* initialize */
	p_cmd->stat = 0;

	p_acs->dev.p_cmd_prog = p_cmd;

	p_cmd->cpb[0] = p_acs->ls.setcount_lo;
	p_cmd->cpb[1] = p_acs->ls.setcount_hi;

	ioa = BUSIO_ATT(p_acs->dds.bus_id, p_acs->dds.bus_io_addr);

	/* start watchdog timer */
	w_start (&(p_acs->dev.cmd_wdt));

	/* 
	 * string move parameters to shared memory 
	 *	(len includes setcount)
	 */
	PIO_PUTSTRX(bus + FDDI_CPB_SHARED_RAM, &p_cmd->cpb[0], p_cmd->cpb_len);

	/* issue the current command to the adapter */
	PIO_PUTSRX(ioa + FDDI_HCR_REG, p_cmd->cmd_code);
	BUSIO_DET(ioa);

	if (p_acs->dev.pio_rc)
	{
		FDDI_ETRACE("Dic2",p_acs,p_cmd->cmd_code,0);
		w_stop(&(p_acs->dev.cmd_wdt));
		if (cmd_flag == FALSE)
			unlock_enable(ipri,&p_acs->dev.cmd_lock);
		return(p_acs->dev.pio_rc);
	}

	FDDI_TRACE("DicE",p_cmd->stat,p_cmd->cmd_code,p_acs->dev.p_cmd_prog);

	/* ok */
	if (cmd_flag == FALSE)
		unlock_enable(ipri,&p_acs->dev.cmd_lock);

	return(p_acs->dev.pio_rc);
}

/*
 * NAME: issue_pri_cmd
 *                                                                    
 * FUNCTION:  issue a priority command
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt thread only
 *                                                                   
 * NOTES: 
 *
 *	Setup the priority command structure and issue the command.
 *
 * RETURNS: none
 */  
void
issue_pri_cmd (
	fddi_acs_t	*p_acs,
	int		pri_cmd_to_issue,
	int		bus,
	int		cmd_flag)
{
	fddi_cmd_t	*p_cmd;
	int		ipri;

	if (!cmd_flag)
		ipri=disable_lock(CFDDI_OPLEVEL,&p_acs->dev.cmd_lock);

	FDDI_TRACE("DipB",p_acs,pri_cmd_to_issue,0);

	p_cmd = &(p_acs->dev.pri_blk);

	/* common priority command setup */
	p_cmd->ctl = 0;
	p_cmd->cpb_len = 0; 

	/*
	 * 'pri_cmd_to_issue' can indicate more than one command but only one
	 *	command will be issued at a time. The order of the
	 *	if statement prioritizes the commands when more
	 *	than one command is indicated.
	 */
	if (pri_cmd_to_issue & FDDI_PRI_CMD_ULS)
	{
		/* setup command block for Update Links Statistics */
		p_cmd->cmd_code = FDDI_HCR_ULS;
		p_cmd->cmplt = (int(*)()) hcr_uls_cmplt;
		p_cmd->cpb_len = 0;

		/* 
		 * turn off priority command indicator 
		 *	to make sure command doesn't run more than once
		 */
		p_acs->dev.pri_que &= (~FDDI_PRI_CMD_ULS);
	}
	else
	{
		/* add logic for other priority commands here ... */

		/* internal programming error */
		FDDI_ASSERT (0);
	}

	/* issue the command in the pri_blk */
	if (issue_cmd (p_acs, p_cmd, bus, TRUE))
	{
		FDDI_ETRACE("Dip1",p_acs,p_cmd->cmd_code,0);
		if (!cmd_flag)
			unlock_enable(ipri,&p_acs->dev.cmd_lock);
		fddi_logerr(p_acs, ERRID_CFDDI_PIO, __LINE__, __FILE__,
				0, 0, 0);
		bugout(p_acs,NDD_PIO_FAIL,0,0,TRUE);
		return;
	}

	FDDI_TRACE("DipE",p_acs,p_cmd->cmd_code,0);
	if (!cmd_flag)
		unlock_enable(ipri,&p_acs->dev.cmd_lock);
	return ;
}

/*
 * NAME: cmd_handler
 *                                                                    
 * FUNCTION: command completion handler
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt environment (SLIH)
 *                                                                   
 * NOTES: 
 *
 *	This is the generic command completion handler. It calls a
 *	command specific routine to handle any specific actions for the command
 *
 *	The command specific routine will determine what command will
 *	run next. If the specific routine wants to run another command
 *	it will set up the command structure in the ACS and that command
 *	will run at the next available chance. If no other commands are
 *	needed then the specific completion routine will set the 'cmd'
 *	to NULL.  This command set by the completion routine can be delayed
 *	if there is a priority command waiting, until the priority command
 *	has completed.
 *
 * RETURNS: none
 */  

void
cmd_handler (
	fddi_acs_t	*p_acs,
	uint		bus) 
{
	fddi_cmd_t	*p_cmd;
	int		ipri;
	int		state;

	ipri = disable_lock(CFDDI_OPLEVEL,&p_acs->dev.cmd_lock);
	FDDI_TRACE("DchB",p_acs,p_acs->dev.p_cmd_prog->stat,0);

	/* turn off the watchdog timer */
	w_stop (&(p_acs->dev.cmd_wdt));

	/* initialize and set no command in progress */
	p_cmd = p_acs->dev.p_cmd_prog;
	p_acs->dev.p_cmd_prog = NULL;

	/* 
	 * get status from shared memory
	 * and place in low order 2 bytes of the stat field
	 */
	if (p_cmd->stat == 0)
	{
		PIO_GETSRX (bus + FDDI_CPB_RES, ((uchar *)&p_cmd->stat+2));
	}

	/* sanity check for command specific completion function */
	FDDI_ASSERT (p_cmd->cmplt)

	/*
	 * Handle some generic HCR errors
	 */
	if (p_cmd->stat == FDDI_HCR_INV_SETCOUNT)
	{
		/*
		 * Probably one already set...but this
		 *	covers a small window.
		 */
		p_acs->dev.pri_que |= FDDI_PRI_CMD_ULS;
	}
	else
	{
		/* 
		 * Call command specific completion function:
		 *	Completion routines will fill in the cmd structure
		 *	with the next command to go out if one is needed.
		 *	Completion routines report status and make state
		 *	changes depending on the status of the command.
		 *	Also, completion routines initiate LIMBO and bugout.
		 */
		state = p_acs->dev.state;
		(void) (*p_cmd->cmplt)(p_acs, p_cmd, bus, ipri);
		if ((state != p_acs->dev.state) &&
			((p_acs->dev.state == LIMBO_STATE) ||
			 (p_acs->dev.state == LIMBO_RECOVERY_STATE) ||
			 (p_acs->dev.state == DEAD_STATE)))
		{
			unlock_enable(ipri, &p_acs->dev.cmd_lock);
			return;
		}
	}

	/* issue next command */
	if (p_acs->dev.pri_que)
	{
		/*
		 * Issue a priority command from the priority queue
		 */
		issue_pri_cmd (p_acs, p_acs->dev.pri_que, bus, TRUE);
	}
	else if (p_acs->dev.cmd_blk.cmd_code != NULL)
	{
		/*
		 * Issue a command (not a priority command)
		 *	(This may be a rerun of a command that failed)
		 */
		issue_cmd (p_acs, &(p_acs->dev.cmd_blk), bus, TRUE);
	}
	/*
	 * If no commands waiting wakeup anyone waiting on another user command
	 */
	else if (p_acs->dev.cmd_wait_event != EVENT_NULL)
	{
		e_wakeup(&p_acs->dev.cmd_wait_event);
	}

	if (p_acs->dev.pio_rc)
	{
		FDDI_ETRACE("Dch1",p_acs,0,0);
		fddi_logerr(p_acs, ERRID_CFDDI_PIO, __LINE__, __FILE__,
				0, 0, 0);
		unlock_enable(ipri,&p_acs->dev.cmd_lock);
		bugout(p_acs,NDD_PIO_FAIL,0,0,TRUE);
		return;
	} 

	/* ok */
	FDDI_TRACE("DchE",p_acs,p_cmd->stat,0);
	unlock_enable(ipri,&p_acs->dev.cmd_lock);
	return ;
}

/*
 * NAME: fddi_cmd_to
 *                                                                    
 * FUNCTION: watchdog time expiration for HCR command
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt environment 
 *
 * NOTES: 
 *	We just had a watchdog timer expire. Assume that we lost
 *	communication with our adapter. enter limbo.
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS:  none
 */  
void
fddi_cmd_to (
	struct	watchdog	*p_wdt)
{
	fddi_acs_t	*p_acs;
	int		bus;


	/* 
	 * Get ACS ptr from wdt ptr
	 */
	p_acs = (fddi_acs_t *) ((uint) p_wdt - 
		((uint) offsetof (fddi_acs_t, dev) + 
		(uint) offsetof (fddi_acs_dev_t, cmd_wdt)));

	FDDI_ASSERT(p_acs->dev.p_cmd_prog != NULL);
	
	FDDI_ETRACE("Dct ",p_acs, p_acs->dev.p_cmd_prog->cmd_code,0);

	fddi_logerr(p_acs, ERRID_CFDDI_CMD_FAIL, __LINE__, __FILE__,
				0, 0, 0);
	enter_limbo(p_acs,NDD_CMD_FAIL,0,FALSE);

	/* ok */
	return ;
}

