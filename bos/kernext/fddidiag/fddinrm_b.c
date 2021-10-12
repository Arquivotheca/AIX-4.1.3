static char sccsid[] = "@(#)86	1.2  src/bos/kernext/fddidiag/fddinrm_b.c, diagddfddi, bos411, 9428A410j 11/8/93 09:51:39";
/*
 *   COMPONENT_NAME: DIAGDDFDDI
 *
 *   FUNCTIONS: fddi_bugout
 *		fddi_cycle_limbo
 *		fddi_enter_limbo
 *		fddi_get_stest
 *		fddi_limbo_cmplt1015
 *		fddi_limbo_to
 *		fddi_run_stest
 *		fddi_wakeup_users
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
#include <sys/iocc.h>
#include <sys/ioacc.h>
#include <sys/adspace.h>
#include "fddi_comio_errids.h"
#include <sys/sleep.h>
/* 
 * get access to the global device driver control block
 */
extern fddi_ctl_t	fddi_ctl;
extern fddi_trace_t	fdditrace;

/*
 * NAME: fddi_wakeup_users()
 *                                                                    
 * FUNCTION: This routine will wakeup and/or notify those users
 *	that are waiting for TX or RCV events.
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt environment
 *                                                                   
 * NOTES: 
 *	This routine will go through alll the open elements and
 *	selnotify those users who have pending selects.
 *	
 *	Those users' who rcv event != EVENT_NULL will be woken up.
 *	If the tx event != EVENT_NULL, all users sleeping on this
 *	event will be woken up.
 *
 * RECOVERY OPERATION: none.
 *
 * DATA STRUCTURES: 
 *	Modifies the ACS tx event work and each open elements selectreq 
 *
 * RETURNS:  none
 */  
void
fddi_wakeup_users(fddi_acs_t	*p_acs)
{
	int 		i;
	int 		opens;
	fddi_open_t	*p_open;

	FDDI_DBTRACE("NwuB", p_acs, p_acs->ctl.open_cnt, 0);
	/* 
	 * while not all the users of this acs 
	 * have not been notified
	 */
	opens = 0;
	i = 0;
	while ( opens < p_acs->ctl.open_cnt )
	{
		p_open = fddi_ctl.p_open_tab [i];
		if ( (p_open != NULL) && (p_open->p_acs == p_acs) )

		{
			/* wakeup sleeping user */
			if (p_open->rcv_event != EVENT_NULL)
				e_wakeup(&p_open->rcv_event);
			
			/* notify the user of event */
			if (p_open->selectreq == 0)
			{
				selnotify(p_open->devno, 
					p_open->chan,
					p_open->selectreq);
				p_open->selectreq = 0;
			}
			++opens;
		}
		++i;
	} /* end while */

	/*
	 * if there is a user sleeping on a TX event
	 * wake that user up.
	 */
	if ( p_acs->tx.event != EVENT_NULL)
	{
		e_wakeup( (int *) &p_acs->tx.event);
	}
	p_acs->tx.needed = FALSE;

	if ( p_acs->dev.ioctl_event != EVENT_NULL)
	{
		p_acs->dev.ioctl_status = FDDI_CMD_FAIL;
		e_wakeup(&p_acs->dev.ioctl_event);
	}
	if  ( p_acs->dev.close_event != EVENT_NULL )
	{
		e_wakeup (&(p_acs->dev.close_event));
	}


	FDDI_DBTRACE("NwuE", p_acs, p_acs->ctl.open_cnt, 0);
	return;
} /* end fddi_wakeup_users() */

/*
 * NAME: fddi_get_stest()
 *                                                                    
 * FUNCTION: 
 *	This routine will obtain the results of the adapter's self tests
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt environment
 *                                                                   
 * NOTES: 
 *	The adapter will be reset via POS reg 2.  The limbo watch
 *	dog timer will be started.
 *
 * RECOVERY OPERATION: none.
 *
 * DATA STRUCTURES: 
 *
 * RETURNS:  none
 */  
void
fddi_get_stest(fddi_acs_t	*p_acs)
{
	int	iocc;
	int	i;
	int	bus;
	uchar	pos;
	int		ipri;

	FDDI_TRACE("NgsB", p_acs, 0, 0);

	ipri = i_disable ( INTCLASS2 );
	iocc = IOCC_ATT(p_acs->dds.bus_id, (IO_IOCC+(p_acs->dds.slot<<16)) );
	PIO_GETCX( (iocc + FDDI_POS_REG2), &pos);
	pos = pos & ~(FDDI_POS2_AR);
	PIO_PUTCX( (iocc+FDDI_POS_REG2), (pos | FDDI_POS2_CEN) );

	/*
	* Read all the individual Self Test results.
	* Each test result is 2 bytes long.
	*/
	bus = BUSMEM_ATT(p_acs->dds.bus_id, (ulong)p_acs->dds.bus_mem_addr);
	for ( i=0; i<FDDI_STEST_CNT;++i)
	{
		BUS_GETSRX(bus+(i<<1),&(p_acs->dev.stestrc[i]));
		FDDI_TRACE("Ngs1", i, (ulong)p_acs->dev.stestrc[i], (i<<1));
	}
	BUSMEM_DET(bus);

	PIO_PUTCX( (iocc + FDDI_POS_REG2), pos);
	IOCC_DET(iocc);
	i_enable(ipri);
	FDDI_TRACE("NgsE", p_acs, 0, 0);
	return;
} /* end fddi_run_stest() */
/*
 * NAME: fddi_run_stest()
 *                                                                    
 * FUNCTION: 
 *	This routine will initiate the running of adapter
 *	self tests.
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt environment
 *                                                                   
 * NOTES: 
 *	The adapter will be reset via POS reg 2.  The limbo watch
 *	dog timer will be started.
 *
 * RECOVERY OPERATION: none.
 *
 * DATA STRUCTURES: 
 *
 * RETURNS:  none
 */  
void
fddi_run_stest(fddi_acs_t	*p_acs,
		ulong		rc1,
		ulong		ac)
{
	int	iocc;
	uchar	pos;

	FDDI_TRACE("NrsB", p_acs, rc1, ac);

	if ((rc1 == FDDI_PIO_FAIL) && (ac & FDDI_EXCEPT_IO))
	{
		FDDI_TRACE("Nrs1", p_acs, 0, 0);
		return;
	}
	/*
	 * Write out pos 2 with the Adapter Reset (AR) and 
	 * Card Enable (CEN) bits set.
	 */

	p_acs->dev.limbo_to = TRUE;
	iocc = IOCC_ATT(p_acs->dds.bus_id, (IO_IOCC+(p_acs->dds.slot<<16)) );
	PIO_PUTCX( (iocc+FDDI_POS_REG2), (p_acs->dev.pos2 | FDDI_POS2_AR  
					| FDDI_POS2_CEN) );
	IOCC_DET(iocc);

	w_start( &(p_acs->dev.limbo_wdt) );

	FDDI_TRACE("NrsE", p_acs, 0, 0);
	return;
} /* end fddi_run_stest() */

/*
 * NAME: fddi_cycle_limbo()
 *                                                                    
 * FUNCTION: 
 *	This routine is called in response to a LEC being detected  
 *	while we are in some form of limbo.
 *
 * EXECUTION ENVIRONMENT: interrupt environment
 *                                                                   
 * NOTES: 
 *
 *	This routine will re-initiate recovery starting from 
 *	ground zero (i.e. and adapter reset) IF the current state is
 *	not FDDI_
 *
 * RECOVERY OPERATION: 
 *	The recovery operation initiated is an adapter reset.
 *
 * DATA STRUCTURES: 
 *	The p_acs->dev.state var may be changed.	
 *
 * RETURNS:  none
 */  
void
fddi_cycle_limbo(
	fddi_acs_t	*p_acs,		/* ACS pointer */
	ulong		rc1,		/* primary reason code */
	ulong		ac)		/* specific adapter code */
{
	uint	bus;

	FDDI_TRACE("NclB", p_acs, p_acs->dev.state, rc1);
	FDDI_TRACE("NclC", ac, 0, 0);

	p_acs->dev.oflv_events = 0;	/* clear oflv work queue */
	p_acs->dev.pri_que = 0;		/* clear priority HCR queue */
	/*
	 * There may also be a cmd in progress, if so fail cmd
	 * with a timeout error.
	 *
	 */
	if (p_acs->dev.p_cmd_prog) 
	{
		FDDI_TRACE("Ncl2", p_acs->dev.p_cmd_prog->cmd_code,
				0, 0);
		p_acs->dev.p_cmd_prog->stat = FDDI_CIO_TIMEOUT;
		fddi_cmd_handler(p_acs, bus);
	}

	/*
	 * We must now stop the recovery process that
	 * is already in progress and start it over from the
	 * beginning.
	 */
	
	p_acs->dev.p_cmd_prog = NULL;

	fddi_run_stest(p_acs, rc1, ac);

	FDDI_TRACE("NclE", p_acs, 0, 0);
	return;
} /* end fddi_cycle_limbo() */

/*
 * NAME: fddi_enter_limbo()
 *                                                                    
 * FUNCTION: 
 *	This routine is called in response to a LEC being detected.  
 *
 * EXECUTION ENVIRONMENT: interrupt environment
 *                                                                   
 * NOTES: 
 *	This routine will:
 *
 *		1.  Purge all pending TX request.  If TX acknowlegdement was
 *		    requested, CIO_TX_ERR will be the status indicated. 
 *		    Flushing of the TX descs. is done because we 
 *		    cannot guarantee that the data in the TX descs. 
 *		    actually went out on the wire successfully.
 *		2.  All cmds pending (ioctls) will be failed.  
 *		3.  All sleeping processes will be woken up and their pending
 *		    operations failed.
 *		4.  Entry point road blocks will be set up.
 *		5.  All attached users will be notified.
 *		6.  After backing up, Limbo recovery logic is initiated.  The 
 *		    LEC determines the recovery logic initiated.  
 *                                                                    
 *	The device will not be fully functional when this routine returns.
 *	The device state will either be FDDI_LIMBO or FDDI_LLC_DOWN.  The
 *	state is dependent on the LEC. 
 *
 *	The following operations to the FDDI device will fail while the 
 *	device is in limbo:
 *
 *		1. Transmit of LLC data.  Either via the write entry point or
 *		   the fastwrite function call.  
 *
 *		2. FDDI_SET_LONG_ADDR ioctl, either to add or 
 *		   delete an address.
 *
 *		3. FDDI_QUERY_ADDR ioctl.
 *
 * RECOVERY OPERATION: 
 *
 *	The recovery logic initiated is dependent on the entry condition.
 *	Possible entry conditions and actions taken are:
 *
 *	Condition 		Location detected	Action taken
 *
 *	FDDI_TX_ERROR		tx_handler()		- Run Self Tests
 *	FDDI_ADAP_CHECK		ACI bit in HSR 		- Run Self Tests
 *	FDDI_REMOTE_SELF_TEST	STR in event word low	- Run Self Tests
 *	FDDI_PATH_TEST		PTF in event word low	- Run Self Tests
 *	FDDI_MC_ERROR		CCW,CCR,NSF,DPR in HSR	- Run Self Tests
 *	FDDI_CMD_FAIL		cmd handler routines	- Run Self Tests
 *	FDDI_PIO_FAIL		fddi_slih() read of HSR	- Run Self Tests
 *	FDDI_ADAP_INIT_FAIL	activation routines	- Run Self Tests
 *	FDDI_RCV_ERROR		rcv_handler()		- Run Self Tests
 *
 *
 * DATA STRUCTURES: 
 *	Modifies the dev.state variable.
 *
 * RETURNS:  none
 */  
void
fddi_enter_limbo(
	fddi_acs_t	*p_acs,		/* ACS pointer */
	ulong		rc1,		/* primary reason code */
	ulong		ac)		/* specific adapter code */
{
	uint		state;		/* save old state */
	uint		bus;


	FDDI_TRACE("NelB", p_acs, p_acs->dev.state, rc1);
	FDDI_TRACE("NelC", ac, p_acs->dev.carryover, p_acs->dev.piowall);

	p_acs->ras.ds.adap_err_cnt++;

        w_stop( &(p_acs->dev.cmd_wdt) );
        w_stop( &(p_acs->dev.close_wdt) );
        w_stop( &(p_acs->dev.limbo_wdt) );
        w_stop( &(p_acs->dev.dnld_wdt) );
        w_stop( &(p_acs->tx.wdt) );

	/*
	 * We can only go into limbo when we are in the FDDI_INIT,
	 * FDDI_OPEN, FDDI_LLC_DOWN, or FDDI_LIMBO device driver state.
	 */

	if ( (p_acs->dev.state != FDDI_INIT) && 
		(p_acs->dev.state != FDDI_OPEN) &&
		(p_acs->dev.state != FDDI_LLC_DOWN) &&
		(p_acs->dev.state != FDDI_LIMBO) )
	{
		/*
		 * we cannot enter limbo in this state
		 * trace the error
		 */
		FDDI_TRACE("Nel2", p_acs->dev.state, 0, 0);
		return;
	}	


	/* 
	 * if we are in the process of connecting to
	 * the net for the 1st time.  Send out the
	 * START Done status blocks.
	 */
	if ( p_acs->dev.state == FDDI_INIT )
	{
		fddi_conn_done(p_acs, CIO_OK);
	}


	/*
	 * if we are already in some form of limbo,
	 * we need to "cycle" limbo.
	 */
	if ( p_acs->dev.state == FDDI_LIMBO )
	{
		fddi_cycle_limbo(p_acs, rc1, ac); 
		return;
	}

	/*
	 * fail any ioctl that is pending, only if we 
	 * were in the FDDI_OPEN or FDDI_LLC_DOWN state could we have
	 * an ioctl pending 
	 */
	p_acs->dev.oflv_events = 0;
	p_acs->dev.oflv_copy = 0;
	p_acs->dev.pri_que = 0;
	if ( ((p_acs->dev.state == FDDI_OPEN) || 
		(p_acs->dev.state == FDDI_LLC_DOWN) ) ) 
	{
		if (p_acs->dev.p_cmd_prog == &p_acs->dev.cmd_blk) 
		{
			
			bus = BUSMEM_ATT(p_acs->dds.bus_id, 
				(uint)p_acs->dds.bus_mem_addr);
			p_acs->dev.p_cmd_prog->stat = FDDI_CIO_TIMEOUT;
			fddi_cmd_handler(p_acs, bus);
			BUSMEM_DET(bus);
		}
	}

	/* 
	 * build the CIO_NET_RCVRY_ENTER async status block and send 
	 * it to all the attached users. 
	 * log the error.
	 */

	p_acs->ctl.limbo_blk.code = CIO_ASYNC_STATUS;
	p_acs->ctl.limbo_blk.option[0] = CIO_NET_RCVRY_ENTER;
	p_acs->ctl.limbo_blk.option[1] = rc1;
	p_acs->ctl.limbo_blk.option[2] = ac;
	p_acs->ctl.limbo_blk.option[3] = 0;
	fddi_logerr(p_acs, ERRID_FDDI_RCVRY_ENTER,
		__LINE__, __FILE__);
	fddi_async_status(p_acs, &p_acs->ctl.limbo_blk);
	/*
	 * wake up all users that may be sleeping
	 * on TX or RCV data...or have a select
	 * pending for one of these
	 */
	fddi_wakeup_users(p_acs);


	/*
	 * One of the following entry conditions
	 * has occured:
	 *
	 *	FDDI_TX_ERROR
	 *	FDDI_ADAP_CHECK
	 *	FDDI_REMOTE_SELF_TEST
	 *	FDDI_PATH_TEST
	 *	FDDI_MC_ERROR
	 *	FDDI_CMD_FAIL
	 *	FDDI_PIO_FAIL
	 *	FDDI_ADAP_INIT_FAIL
	 *	FDDI_RCV_ERROR
	 *
	 * we need to shut things down completely.
	 * Set up device road blocks for the entry points.
	 * Run the adapter's self test.  If self tests
	 * complete successfully, we must go thru the entire
	 * activation sequence to bring the adapter back
	 * on line.
	 * If self tests fail, we will go to the FDDI_DEAD 
	 * state.  The device will then be un-useable.
	 */
	fddi_undo_start_tx(p_acs);

	/* 
	 * initialize rcv index (the card initializes its copy in the 
	 * reset) 
	 */
	p_acs->rcv.rcvd = 0;

	/*
	 * purge any cmds that may be pending (normal)
	 *
	 */
	p_acs->dev.cmd_blk.cmd_code = NULL;

	/*
	 * change the state to indicate that we are now 
	 * in Network Recovery Mode.
	 */
	p_acs->dev.state = FDDI_LIMBO;	
	fddi_run_stest(p_acs, rc1, ac);

	FDDI_TRACE("NelE", p_acs, p_acs->dev.state, rc1);
	return;

} /* end fddi_enter_limbo() */

/*
 * NAME: fddi_bugout()
 *                                                                    
 * FUNCTION: 
 *	This routine is called in response to a fatal error being detected.
 *	The fatal error will determine the device shutdown logic to be
 *	performed.  The device will no longer be functional when this
 *	routine returns.  The device state will be FDDI_DEAD.
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt environment
 *                                                                   
 * NOTES: 
 *	Fatal errors that are handled by this routine are:
 *		
 *	Condition		Location detected
 *
 *	FDDI_PIO_FAIL		fddi_pio_retry()
 *	FDDI_MC_ERROR
 *	FDDI_ADAP_CHECK
 *	FDDI_SELF_TEST		fddi_limbo_to()
 *	FDDI_REMOTE_DISCONNECT	RDF Event Word low
 *	FDDI_MAC_DISCONNECT	MD Event Word low
 *
 *	After the appropriate fatal error cleanup has been done, this 
 *	routine will:
 *
 *		1.  Purge all pending TX request.  If TX acknowlegdement was
 *	    	    requested, CIO_TX_ERR will be the status indicated.
 *		2.  All cmds pending (ioctls) will be failed.  
 *		3.  All sleeping processes will be woken up and their pending
 *	    	    operations failed.
 *		4.  Entry point road blocks will be set up.
 *		5.  All other system resources that can be freed on the
 *	    	    interrupt thread will be freed.
 *		6.  All attached users will be notified via a CIO_HARD_FAIL
 *		    async status block.
 *		7.  Disable the adapter card via POS.
 *
 *	After a fatal error has been detected, the device is no longer 
 *	operational.  The following operations will fail:
 *
 *		1. transmit of data via write entry point and the fast write 
 *		   function call.
 *		2. CIO_START ioctl.
 *		3. FDDI_SET_LONG_ADDR ioctl (add and delete) of address.
 *		4. FDDI_QUERY_ADDR ioctl.
 *		4. FDDI_DWNLD_MCODE ioctl.
 *		5. FDDI_HCR_CMD ioctl.
 *		6. FDDI_ACC_DMA ioctl.
 *
 * RECOVERY OPERATION: 
 *	This routine can not fail.
 *
 * DATA STRUCTURES: 
 *	This routine modifiest the device state machine p_acs->dev.state.
 *	
 * RETURNS:  none
 */  
void 
fddi_bugout (
		fddi_acs_t	*p_acs,	/* ACS ptr */
		ulong		rc1,	/* primary reason */
		ulong		rc2,	/* secondary reason */
		ulong		ac)	/* specific adapter code */
{

	uint		state;
	uint 		iocc;
	uint		bus;
	uint		ipri;
	uint		opens;
	uint		i;
	fddi_open_t	*p_open;

	FDDI_TRACE("NboB", p_acs, p_acs->dev.state, rc1);
	FDDI_TRACE("NboC", rc2, ac, 0);

	p_acs->ras.ds.adap_err_cnt++;

        w_stop( &(p_acs->dev.cmd_wdt) );
        w_stop( &(p_acs->dev.close_wdt) );
        w_stop( &(p_acs->dev.limbo_wdt) );
        w_stop( &(p_acs->dev.dnld_wdt) );
        w_stop( &(p_acs->tx.wdt) );

	/* Set up permanent road blocks for the entry points.
	 * Build the CIO_HARD_FAIL async status
	 * block and send to all attached users of this
	 * device.
	 * Error log the error.
	 */

	/* 
	 * if we are in some form of limbo,
	 * log error that we have terminated
	 * network recovery mode.
	 */
	if ( (p_acs->dev.state == FDDI_LIMBO) ||
		(p_acs->dev.state == FDDI_LLC_DOWN) )
	{
		fddi_logerr(p_acs, ERRID_FDDI_RCVRY_TERM,
			__LINE__, __FILE__);
	}
	/*
	 * notify all attached users that we are
	 * no longer functional
	 */
	p_acs->ctl.limbo_blk.code = CIO_ASYNC_STATUS;
	p_acs->ctl.limbo_blk.option[0] = CIO_HARD_FAIL;
	p_acs->ctl.limbo_blk.option[1] = rc1;
	p_acs->ctl.limbo_blk.option[2] = rc2;
	p_acs->ctl.limbo_blk.option[3] = ac;
	fddi_logerr(p_acs, ERRID_FDDI_DOWN,
		__LINE__, __FILE__);


	/*
	 * If we are in the middle of shutting down 
	 * the device due to the last close, we can not
	 * bug out.
	 */

	if (p_acs->dev.state == FDDI_CLOSING)	
	{
		FDDI_TRACE("Nbo1", rc2, ac, 0);
		return;
	}
	/*
	 * save our current state. change the
	 * state to DEAD. serialize with the
	 * slih that will be checking the state. 
	 */
	state = p_acs->dev.state;
	p_acs->dev.state = FDDI_DEAD;
	p_acs->dev.oflv_events = 0;
	p_acs->dev.oflv_copy = 0;


	/*
	 * get access to the iocc 
	 */
	iocc = IOCC_ATT(p_acs->dds.bus_id, (IO_IOCC+(p_acs->dds.slot<<16)));

	/* 
	 * We must reset the card via POS
	 * regs and disable the card via POS regs so that
	 * we do not leave the card  in an indeterminate 
	 * state.
	 */
	
	PIO_PUTCX( (iocc+FDDI_POS_REG2), 
		((p_acs->dev.pos2 & ~(FDDI_POS2_CEN)) |
		 FDDI_POS2_AR));

	IOCC_DET(iocc);
	/*
	 * NB:
	 *	At this point the adapter is no longer functioning.
	 *	It has been disabled.  No routines from this point
	 *	should access the card.
	 */

	p_acs->dev.piowall = TRUE;
	/* 
	 * determine what our state is.  
	 * Our state determines what has to be done to bring
	 * the device driver to the DEAD state.
	 */

	switch ( state )
	{
		case FDDI_DEAD:
		{
			/* 
			 * we have already gone to the dead state.
			 */
			FDDI_TRACE("Nbo2", p_acs, 0, 0);
			ASSERT(0);
			break;
		}
		case FDDI_NULL:
		{
			/*
			 * Things to clean up in this state are:
			 * 	- If there is an FDDI_MEM_ACC or
			 *	  FDDI_HCR_CMD ioctl in progress, we will
			 *	  let its' timers catch it and it will fail
			 *	  on its' own.
			 *
			 * Valid bug out conditions in this state are:
			 *	FDDI_PIO_FAIL
			 *	FDDI_ADAP_CHECK
			 *	FDDI_MC_ERROR
			 */
			break;
		}
		case FDDI_DWNLDING:
		{
			/*
			 * Valid bug out conditions in this state are:
			 *	FDDI_PIO_FAIL
			 *	FDDI_ADAP_CHECK
			 *	FDDI_MC_ERROR
			 *
			 * Things to clean up in this state are:
			 * 	- The Download ioctl needs to be failed.
			 *	
			 * NB:
			 *	We don't do anything to stop the ioctl 
			 *	thread.  We will let the ioctl wdt pop
			 *	and the fddi_dnld_to() routine will
			 *	catch us.  The fddi_dnld_to() routine will
			 *	not set the state to FDDI_NULL if a fatal
			 *	error has occured.
			 *	
			 */

			if (p_acs->dev.p_cmd_prog == &p_acs->dev.cmd_blk) 
			{
				bus = BUSMEM_ATT(p_acs->dds.bus_id, 
					(uint)p_acs->dds.bus_mem_addr);
				p_acs->dev.p_cmd_prog->stat = FDDI_CIO_TIMEOUT;
				fddi_cmd_handler(p_acs, bus);
				BUSMEM_DET(bus);
			}

			break;
		}
		case FDDI_DWNLD:
		{
			/*
			 * Things to clean up in this state are:
			 * 	- If there is an FDDI_MEM_ACC or
			 *	  FDDI_HCR_CMD ioctl in progress, we will
			 *	  let the ioctl timer catch these ioctls
			 *	  and they will fail gracefully with 
			 *	  timeouts.
			 *	- Due to the nature of the FDDI_HCR_CMD ioctl,
			 *	  diagnostics can do a hcr cmd that will 
			 *	  cause us to issue a priority cmd, therefore
			 *	  we must stop any priority cmds.
			 *
			 * Valid bug out conditions in this state are:
			 *	FDDI_PIO_FAIL
			 *	FDDI_ADAP_CHECK
			 *	FDDI_MC_ERROR
			 *	
			 *	!!! Need to investigate if we can get
			 *	the following bug out conditions from
			 *	diagnostics dinking with HCR cmds.
			 *		FDDI_REMOTE_DISCONNECT
			 *		FDDI_SELF_TEST
			 *		FDDI_PATH_TEST
			 */
			/* stop any priority cmds */
			if ( p_acs->dev.p_cmd_prog == &p_acs->dev.pri_blk )
			{
				p_acs->dev.p_cmd_prog->cmd_code = 0;
				p_acs->dev.p_cmd_prog->cmplt = NULL; 
				p_acs->dev.p_cmd_prog = NULL;
			}
			p_acs->dev.pri_que = 0;

			break;
		}
		case FDDI_INIT:
		{
			/*
			 * Things to clean up in this state are:
			 *	- The activation sequence must be
			 *	  halted.  The CIO_START_DONE blocks
			 *	  need to be sent out with CIO_HARD_FAIL
			 *	  indicators.
			 *
			 * Valid bug out conditions in this state are:
			 *	FDDI_PIO_FAIL
			 *	FDDI_ADAP_CHECK
			 *	FDDI_MC_ERROR
			 */

			if ( p_acs->dev.p_cmd_prog )
			{
				p_acs->dev.p_cmd_prog->cmd_code = 0;
				p_acs->dev.p_cmd_prog->cmplt = NULL; 
				p_acs->dev.p_cmd_prog = NULL;
			}
			p_acs->dev.pri_que = 0;

			fddi_conn_done(p_acs, CIO_HARD_FAIL);
			break;
		}
		case FDDI_OPEN:
		{
			/*
			 * Things to clean up in this state are:
			 *	- If there is an FDDI_SET_LONG_ADDR ioctl
			 *	  pending, we will failit with a timeout error
			 *	- Fail all pending transmit w/ FDDI_TX_ERROR
			 *	- wakeup any users that may be sleeping
			 *	  on TX or RCV
			 *	- Stop any priority CMDs that may be running
			 *	  or scheduled to run.
			 *
			 * Valid bug out conditions in this state are:
			 *	FDDI_PIO_FAIL
			 *	FDDI_ADAP_CHECK
			 *	FDDI_MC_ERROR
			 *	FDDI_REMOTE_DISCONNECT
			 *	FDDI_SELF_TEST
			 *	FDDI_PATH_TEST
			 */

			fddi_undo_start_tx(p_acs);

			/*
			 * There is nothing to do for the RCV end.
			 */
			/* stop any ioctl that may be running */	
			if (p_acs->dev.p_cmd_prog == &p_acs->dev.cmd_blk) 
			{
				bus = BUSMEM_ATT(p_acs->dds.bus_id, 
					(uint)p_acs->dds.bus_mem_addr);
				p_acs->dev.p_cmd_prog->stat = FDDI_CIO_TIMEOUT;
				fddi_cmd_handler(p_acs, bus);
				BUSMEM_DET(bus);
			}
			/* stop any priority cmds */
			if ( p_acs->dev.p_cmd_prog == &p_acs->dev.pri_blk )
			{
				p_acs->dev.p_cmd_prog->cmd_code = 0;
				p_acs->dev.p_cmd_prog->cmplt = NULL; 
				p_acs->dev.p_cmd_prog = NULL;
			}
			p_acs->dev.pri_que = 0;
			/*
			 * wake up any user's that may be
			 * sleeping for TX or RCVs
			 */
			
			fddi_wakeup_users(p_acs);

			break;
		}
		case FDDI_LLC_DOWN:
		{
			/*
			 * Things to clean up in this state are:
			 *	- Fail all pending transmit w/ FDDI_TX_ERROR
			 *	- Stop any priority CMDs that may be running
			 *	  or scheduled to run.
			 *	- error log that we are terminating limbo
			 *
			 * Valid bug out conditions in this state are:
			 *	FDDI_PIO_FAIL
			 *	FDDI_ADAP_CHECK
			 *	FDDI_MC_ERROR
			 *	FDDI_REMOTE_DISCONNECT
			 *	FDDI_SELF_TEST
			 *	FDDI_PATH_TEST
			 */

			fddi_undo_start_tx(p_acs);

			/*
			 * There is nothing to do for the RCV end.
			 */
			
			/* stop any ioctl that may be running */	
			if (p_acs->dev.p_cmd_prog == &p_acs->dev.cmd_blk) 
			{
				bus = BUSMEM_ATT(p_acs->dds.bus_id, 
					(uint)p_acs->dds.bus_mem_addr);
				p_acs->dev.p_cmd_prog->stat = FDDI_CIO_TIMEOUT;
				fddi_cmd_handler(p_acs, bus);
				BUSMEM_DET(bus);
			}
			/* stop any priority cmds */
			if ( p_acs->dev.p_cmd_prog == &p_acs->dev.pri_blk )
			{
				p_acs->dev.p_cmd_prog->cmd_code = 0;
				p_acs->dev.p_cmd_prog->cmplt = NULL; 
				p_acs->dev.p_cmd_prog = NULL;
			}
			p_acs->dev.pri_que = 0;
			/*
			 * wake up all users that may be sleeping
			 * on TX or RCV data...or have a select
			 * pending for one of these
			 */
			fddi_wakeup_users(p_acs);
			break;
		}
		case FDDI_LIMBO:
		{
			/*
			 * Things to clean up in this state are:
			 *	- Determine what phase of the recovery
			 *	  sequence we are in and stop it.
			 *	- error log that we are terminating limbo
			 *	- Stop any priority CMDs that may be running
			 *	  or scheduled to run.
			 *
			 * Valid bug out conditions in this state are:
			 *	FDDI_PIO_FAIL
			 *	FDDI_ADAP_CHECK
			 *	FDDI_MC_ERROR
			 *	FDDI_SELF_TEST
			 *	FDDI_REMOTE_DISCONNECT (*)
			 *	FDDI_MAC_DISCONNECT (*)
			 *
			 *	(*) This two conditions are only valid
			 *	    if the LEC was a TX timeout.
			 */
			/* !!! determine if self test timer is 
			 * running, if so stop it.
			 * !!! determine if we are in the TX resync
			 * phase of limbo, if so take action to stop
			 * the timer used
			 */
			/* stop any priority cmds */
			if ( p_acs->dev.p_cmd_prog == &p_acs->dev.pri_blk )
			{
				p_acs->dev.p_cmd_prog->cmd_code = 0;
				p_acs->dev.p_cmd_prog->cmplt = NULL; 
				p_acs->dev.p_cmd_prog = NULL;
			}
			p_acs->dev.pri_que = 0;
			break;
		}
		default:
		{
			FDDI_TRACE("Nbo3", p_acs, p_acs->dev.state, rc1);
			ASSERT(0);
			break;
		}
	} /* end switch on p_acs->dev.state */


	fddi_async_status(p_acs, &p_acs->ctl.limbo_blk );


	FDDI_TRACE("NboE", p_acs->dev.state, 0, 0);
	return;
} /* end fddi_bugout() */


/*
 * NAME: fddi_limbo_to()
 *                                                                    
 * FUNCTION: 
 *	This routine will check the resutls to the adapters'
 *	self tests.  If the tests were successful, the egress of
 *	limbo will be initiated.  If the tests failed, fddi_bugout()
 *	will be called to shutdown the device.
 *                                                                    
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt environment
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: none.
 *
 * DATA STRUCTURES: 
 *
 * RETURNS:  none
 */  
void
fddi_limbo_to(struct watchdog	*p_wdt)
{
	fddi_acs_t	*p_acs;
	int		ipri;
	uint		iocc, bus, ioa;
	ushort		hsr;
	int		i;
	fddi_cmd_t	*p_cmd;
	uchar		error;


	/* get ACS */
	p_acs = (fddi_acs_t *) ((uint) p_wdt - 
		((uint) offsetof (fddi_acs_dev_t, limbo_wdt) + 
		 (uint) offsetof (fddi_acs_t, dev)));

	ipri = i_disable ( INTCLASS2 );
	FDDI_TRACE("NltB", p_wdt, 0, 0);

	p_acs->dev.oflv_events |= FDDI_LIMBO_WDT_IO;
	if (p_acs->dev.oflv_running == FALSE)
	{
		/*
		 * schedule offlevel process and set running flag.
		 *	oflv_running is set to FALSE after the last
		 *	check of the hsr in fddi_oflv();
		 */
		i_sched (&p_acs->dev.ihs);
		p_acs->dev.oflv_running = TRUE;
	}

	FDDI_TRACE("NltE", p_acs->dev.state, error, 0);
	i_enable(ipri);
	return;
} /* end fddi_limbo_to() */

/*
 * NAME: fddi_limbo_cmplt()
 *                                                                    
 * FUNCTION: 
 *	This routine will take the driver from a successfuly check
 *	of the self tests results to the beginning of the adapter
 *	activation sequence (FDDI_WR_USR_DATA).
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt environment
 *                                                                   
 * NOTES: 
 *	If an error is detected, this routine will enter limbo.
 *	This routine is called by the fddi_cmd_handler().  This routine
 *	determines is the Exit Diagnostic mode cmd succeeded.  If so,
 *	it will set the addresses that have been set by the users (if
 *	there are any).
 *
 *	When all the addresses have been set, this routine will initiate
 *	the connection sequence by issueing the Write User Data cmd.  The
 *	fddi_act_cmplt() routine will then take over the egress of limbo.  We
 *	will be in the FDDI_OPEN state when the connection sequence 
 *	successfully completes.
 *
 * RECOVERY OPERATION: none.
 *
 * DATA STRUCTURES: 
 *
 * RETURNS:  none
 */  
int
fddi_limbo_cmplt(fddi_acs_t	*p_acs,
		fddi_cmd_t	*p_cmd,
		uint		bus)
{
	uint	iocc;
	uchar	*p_addr;
	int 	i;

	FDDI_TRACE("NlcB", p_acs, p_cmd->stat, p_cmd->cmd_code);
	if (p_cmd->stat == FDDI_CIO_TIMEOUT)
	{
		FDDI_TRACE("Nlc1", p_cmd->stat, p_cmd->cmd_code, bus);
		p_cmd->cmd_code = 0;
		/*
		 * we have a cmd timeout.  "enter" limbo
		 */
		fddi_enter_limbo(p_acs, FDDI_CMD_FAIL, 0);
		return;
	}

	if (p_cmd->stat != FDDI_HCR_SUCCESS)
	{
		FDDI_TRACE("Nlc2", p_cmd->stat, bus, 0);
		p_cmd->cmd_code = 0;
		fddi_enter_limbo(p_acs, FDDI_CMD_FAIL, 0);
		return;
	}

	i = p_acs->ctl.addr_index;

	while (i<FDDI_MAX_ADDRS)
	{
		if (p_acs->ctl.addrs[i].cnt != 0)
			break;
		i++;
	}

	if (i<FDDI_MAX_ADDRS)
	{
		p_acs->ctl.addr_index = i + 1;
		/*
		* Reset command structure each time
		*/
		p_cmd->pri = 0;
		p_cmd->cmplt = (int(*)()) fddi_limbo_cmplt;
		p_cmd->cmd_code = FDDI_HCR_WRITE_ADDR;
		p_cmd->cpb_len = 14;
		p_cmd->cpb[2] = SWAPSHORT(FDDI_HCR_ALD_SKY);
		p_cmd->cpb[3] = SWAPSHORT (FDDI_HCR_ATD_SKY);

		/*
	 	* Put address in proper format for the
		* adapter
	 	*/
		p_addr = (char *) &(p_cmd->cpb[4]);
		*(p_addr++)=p_acs->ctl.addrs[i].addr[5];
		*(p_addr++)=p_acs->ctl.addrs[i].addr[4];
		*(p_addr++)=p_acs->ctl.addrs[i].addr[3];
		*(p_addr++)=p_acs->ctl.addrs[i].addr[2];
		*(p_addr++)=p_acs->ctl.addrs[i].addr[1];
		*(p_addr++)=p_acs->ctl.addrs[i].addr[0];

		return(0);
	}

	p_acs->ctl.addr_index = 0;

	/*
	 * start the connection process. the
	 * fddi_act_cmplt() routine will take us 
	 * to the FDDI_OPEN state.
	 */

	p_cmd = &(p_acs->dev.cmd_blk);
	p_cmd->stat = 0;
	p_cmd->pri = 0;
	p_cmd->cmplt = (int(*)()) fddi_act_cmplt;


	/* 
	 * Here we need to put the 32 byte User Data
	 * into the cpb.  We also need to swap the 
	 * User data.
	 *
	 * We will go thru the User Data char array
	 * by multiples of 2, assign that element and the next 
	 * to the CPB (after byte swapping).
	 */

	p_cmd->cmd_code = FDDI_HCR_WR_USR_DATA;
	for (i=0; i<(FDDI_USR_DATA_LEN >> 1); i++)
	{
		/* 
		 * we add 2 here to account for the
		 * offset into the CPB where the 
		 * User Data will start.
		 */
		p_cmd->cpb[2+i] = SWAPSHORT((ushort)
			p_acs->dds.user_data[i<<1]);
	}
	p_cmd->cpb_len = FDDI_USR_DATA_LEN + 4; 

	FDDI_TRACE("NlcE", p_acs->dev.state, 0, 0);
	return(0);
} /* end fddi_limbo_cmplt() */
			
			
