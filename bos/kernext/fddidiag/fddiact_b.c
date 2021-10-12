static char sccsid[] = "@(#)73	1.2  src/bos/kernext/fddidiag/fddiact_b.c, diagddfddi, bos411, 9428A410j 11/8/93 09:48:57";
/*
 *   COMPONENT_NAME: DIAGDDFDDI
 *
 *   FUNCTIONS: fddi_act
 *		fddi_act_cmplt
 *		fddi_close_wait
 *		fddi_shutdown_adap
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
#include <sys/mbuf.h>
#include <sys/adspace.h>
#include <sys/iocc.h>
#include <sys/ioacc.h>
#include <sys/dma.h>
#include <sys/sleep.h>
#include "fddi_comio_errids.h"

/*
 * NAME: fddi_act()
 *                                                                    
 * FUNCTION: Initiates the activation of the adapter in response
 *	to the 1st CIO_START ioctl.
 *                                                                    
 * EXECUTION ENVIRONMENT: Process environment.
 *                                                                   
 * NOTES: 
 *
 *	This routine starts the activation process. This first activation
 *	process will continue asynchronously until completion. Completion
 *	is detected in fddi_act_cmplt.
 *	
 * RECOVERY OPERATION: 
 *
 *	None
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 */  

void
fddi_act ( 
	fddi_acs_t 	*p_acs)
{
	fddi_cmd_t	*p_cmd;
	int		ipri;
	int		bus;	/* to pass to issue command */


	FDDI_DBTRACE("ActB", p_acs, p_acs->dev.state, 0);

	/* 
	 * initialize cmd structure for 1st activation command
	 */
	p_cmd = &(p_acs->dev.cmd_blk);
	p_cmd->stat = 0;
	p_cmd->pri = 0;
	p_cmd->cmplt = (int(*)()) fddi_act_cmplt;
	p_cmd->cmd_code = FDDI_HCR_START_MCODE;
	p_cmd->cpb_len = 0; 

	/* serialize with SLIH */
	ipri = i_disable ( INTCLASS2 );

	p_acs->dev.state = FDDI_INIT;

	/* 
	 * issue command 
	 */
	bus = BUSMEM_ATT(p_acs->dds.bus_id, (ulong)p_acs->dds.bus_mem_addr);
	fddi_issue_cmd (p_acs, p_cmd, bus);
	BUSMEM_DET(bus);

	/* restore interrupt level */
	i_enable (ipri);

	FDDI_DBTRACE("ActE", p_acs, p_acs->dev.state, 0);
	return;
}

/*
 * NAME: fddi_act_cmplt
 *                                                                    
 * FUNCTION: completion routine for the activation sequence
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt environment (during CCI interrupt)
 *                                                                   
 * NOTES: 
 *
 *	This routine manages the activation sequence. It can be 
 *	driven by 'fddi_act' during the first activation or by
 *	the network recovery mode logic during an attempt to
 *	reactivate.
 *
 *	Managing the activation sequence is making sure of the order
 *	commands are issued and handling errors. Error handling is state
 *	dependent.
 *
 *	The following sequence of commands are issued to the 
 *	adapter for activation:
 *
 *		FDDI_HCR_WR_USR_DATA
 *		FDDI_HCR_WR_PASSWORD
 *		FDDI_HCR_WR_LONG_ADDR
 *		FDDI_HCR_WR_ATT_CLASS
 *		FDDI_HCR_CONNECT
 *
 * RECOVERY OPERATION: 
 *
 *	We can be in different states while executing this routine.
 *	As long as no errors occur the processing is independent of
 *	the state we are in. All state dependent action is taken
 *	care of in the act_error () routine.
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: none
 */  


int 
fddi_act_cmplt (
	fddi_acs_t	*p_acs,
	fddi_cmd_t	*p_cmd,
	int		bus)		/* not used */
{
	static void	act_error();
	int		i;
	ushort 		tmppasswd;

	FDDI_DBTRACE("AacB", p_acs, p_acs->dev.state, p_cmd->cmd_code);
	FDDI_DBTRACE("AacC", p_cmd->stat, 0, 0);

	/* 
	 * add the SetCount protocol to CPB 
	 *	All configuration commands use setcount except CONNECT.
	 *	The CONNECT command setup will write over setcount.
	 */

	/* specific command setup */
	switch (p_cmd->cmd_code)
	{
		case FDDI_HCR_START_MCODE:
		{
			/* 
			 * Here we need to put the 32 byte User Data
			 * into the cpb.  We also need to swap the 
			 * User data.
			 *
			 * We will go thru the User Data char array
			 * by multiples of 2, assign that element and the next 
			 * to the CPB (after byte swapping).
			 */
			if (p_cmd->stat  != FDDI_HCR_SUCCESS)
			{
				FDDI_TRACE("Aac1",p_cmd->cmd_code, 
					p_cmd->stat, 0);
			}

			p_cmd->cmd_code = FDDI_HCR_WR_USR_DATA;
			for (i=0; i<(FDDI_USR_DATA_LEN >> 1); i++)
			{
				/* 
				 * The user data needs to be put into reverse
				 * order. 
				 */
				p_cmd->cpb[2+i] = SWAPSHORT((ushort)
					p_acs->dds.user_data[
					((FDDI_USR_DATA_LEN >> 1) - 1)<<1]);
			}
			p_cmd->cpb_len = FDDI_USR_DATA_LEN + 4; 
			break;
		}
		case FDDI_HCR_WR_USR_DATA:
		{
			char 	*p_addr;	/* tmp ptr for long address */
			/*
			 * Write user data command failures are ignored.
			 */
			/* setup command */

			if (p_cmd->stat  != FDDI_HCR_SUCCESS)
			{
				FDDI_TRACE("Aac2",p_cmd->cmd_code, 
						p_cmd->stat, 0);
			}
			p_addr = (char *) &(p_cmd->cpb[2]);

			*(p_addr++) = p_acs->dds.pmf_passwd[7];
			*(p_addr++) = p_acs->dds.pmf_passwd[6];
			*(p_addr++) = p_acs->dds.pmf_passwd[5];
			*(p_addr++) = p_acs->dds.pmf_passwd[4];
			*(p_addr++) = p_acs->dds.pmf_passwd[3];
			*(p_addr++) = p_acs->dds.pmf_passwd[2];
			*(p_addr++) = p_acs->dds.pmf_passwd[1];
			*(p_addr++) = p_acs->dds.pmf_passwd[0];

			p_cmd->cmd_code = FDDI_HCR_WR_PASSWORD;
			p_cmd->cpb_len = FDDI_PASSWD_SZ + 4;
			break;
		}
		case FDDI_HCR_WR_PASSWORD:
		{
			char 	*p_addr;	/* tmp ptr for long address */

			/*
			 * Write password command failures are ignored.
			 * setup destination address register (the FORMAC)
			 * 	set the address: starting at the 2nd parameter
			 */
			if (p_cmd->stat  != FDDI_HCR_SUCCESS)
			{
				FDDI_TRACE("Aac3",p_cmd->cmd_code, 
					p_cmd->stat, 0);
			}
			p_cmd->cpb[2] = 0;
			p_cmd->cpb[3] = SWAPSHORT (FDDI_HCR_ATD_FOR);
			p_addr = (char *) &(p_cmd->cpb[4]);
			*(p_addr++) = p_acs->ctl.long_src_addr[5];
			*(p_addr++) = p_acs->ctl.long_src_addr[4];
			*(p_addr++) = p_acs->ctl.long_src_addr[3];
			*(p_addr++) = p_acs->ctl.long_src_addr[2];
			*(p_addr++) = p_acs->ctl.long_src_addr[1];
			*(p_addr++) = p_acs->ctl.long_src_addr[0];
			/* 
			 * set command and length of CPB 
			 */
			p_cmd->cmd_code = FDDI_HCR_WRITE_ADDR;
			p_cmd->cpb_len = 14;

			break;
		}
		case FDDI_HCR_WRITE_ADDR:
		{
			/* 
			 * setup command
			 *	attachment class goes into 3rd parameter
			 */
			if (p_cmd->stat  != FDDI_HCR_SUCCESS)
			{
				FDDI_TRACE("Aac4",p_cmd->cmd_code, 
					p_cmd->stat, 0);
			}
			p_cmd->cpb[2] = SWAPSHORT(p_acs->dev.attach_class);
			p_cmd->cmd_code = FDDI_HCR_WR_ATT_CLASS;
			p_cmd->cpb_len = 6;
			break;
		}
		case FDDI_HCR_WR_ATT_CLASS:
		{
			int 	temp_treq;	

			/* 
			 * Take the 2's complement of 1/80th of the t_req
			 * value passed in at cfg time to meet the adapter's
			 * write_treq format.
			 */
			if (p_cmd->stat  != FDDI_HCR_SUCCESS)
			{
				FDDI_TRACE("Aac5",p_cmd->cmd_code, 
					p_cmd->stat, 0);
			}
			temp_treq = ~(p_acs->dds.t_req / 80) + 1;
			/* 
			 * set command and length of CPB 
			 */
			p_cmd->cpb[2] = SWAPSHORT((ushort) 
				temp_treq & 0xFFFF);
			p_cmd->cpb[3] = SWAPSHORT((ushort) 
				(temp_treq >> 16) );
			p_cmd->cpb[4] = SWAPSHORT((ushort) FDDI_PRI_PATH);

			p_cmd->cmd_code = FDDI_HCR_WR_MAX_TREQ;
			p_cmd->cpb_len = 10;

			break;
		}
		case FDDI_HCR_WR_MAX_TREQ:
		{
			int 	temp_tvx;

			/* 
			 * Take the 2's complement of 1/80th of the tvx
			 * value passed in at cfg time to meet the adapter's
			 * write_tvx format.
			 */
			if (p_cmd->stat  != FDDI_HCR_SUCCESS)
			{
				int 	temp_treq;	
				temp_treq = ~(p_acs->dds.t_req / 80) + 1;
				FDDI_TRACE("Aac6",p_cmd->stat, 
					p_acs->dds.t_req, temp_treq);
			}
			temp_tvx = ~(p_acs->dds.tvx / 80) + 1;
			/* 
			 * set command and length of CPB 
			 */
			p_cmd->cpb[2] = SWAPSHORT((ushort) 
				temp_tvx & 0xFFFF);
			p_cmd->cpb[3] = SWAPSHORT((ushort) 
				(temp_tvx >> 16) );
			p_cmd->cpb[4] = SWAPSHORT((ushort) FDDI_PRI_PATH);

			p_cmd->cmd_code = FDDI_HCR_WR_TVX_LOW_BND;
			p_cmd->cpb_len = 10;
			break;
		}
		case FDDI_HCR_WR_TVX_LOW_BND:
		{
			/*
			 * set SMT CONTROL word parameter and
			 * set SMT ERROR and EVENT mask words
			 * 	(these assignments set 32 bit quantities)
			 *	(Doesn't use SetCount protocol)
			 */
			if (p_cmd->stat  != FDDI_HCR_SUCCESS)
			{
				FDDI_TRACE("Aac7",p_cmd->cmd_code, 
					p_cmd->stat, p_acs->dds.tvx);
			}
			p_cmd->cpb[2] = SWAPSHORT(p_acs->dev.smt_control);
			p_cmd->cpb[3] = SWAPSHORT((ushort) 
				p_acs->dev.smt_error_mask & 0xFFFF);
			p_cmd->cpb[4] = SWAPSHORT((ushort) 
				p_acs->dev.smt_error_mask >> 16);
			p_cmd->cpb[5] = SWAPSHORT((ushort) 
				p_acs->dev.smt_event_mask & 0xFFFF);
			p_cmd->cpb[6] = SWAPSHORT((ushort) 
				p_acs->dev.smt_event_mask >> 16);
			p_cmd->cpb[7] = SWAPSHORT((ushort) 0);

			/* 
			 * set command and length of CPB 
			 */
			p_cmd->cmd_code = FDDI_HCR_CONNECT;
			p_cmd->cpb_len = 16;
			/*
			 * Arm receives before doing the CONNECT command
			 *	so when CONNECT is successfull we are 
			 *	ready.
			 */
			fddi_start_rcv (p_acs, bus);
			break;
		}
		case FDDI_HCR_CONNECT:
		{
			p_cmd->cmd_code = NULL;
			/*  
			 * check for errors 
			 */
			if (p_cmd->stat  != FDDI_HCR_SUCCESS)
			{
				FDDI_TRACE("Aac8",p_cmd->cmd_code, 
					p_cmd->stat, 0);
				fddi_enter_limbo(p_acs, FDDI_ADAP_INIT_FAIL, 0);
				return(0);
			}
			/*
			 * If 1st START then always report status 
			 *	Connection failures are not always reported
			 *	when the CONNECT command is given so we 
			 *	assume we are connected.
			 */
			if (p_acs->dev.state == FDDI_INIT) 
			{
				/*
				 * 1st START done
				 */
				p_acs->dev.state = FDDI_OPEN;
				fddi_conn_done(p_acs, CIO_OK);
			}
			else if (p_acs->dev.state == FDDI_LIMBO)
			{
				cio_stat_blk_t	sb; 	/* status blk */

				/* issue status */
				p_acs->dev.state = FDDI_OPEN;
				sb.code = CIO_ASYNC_STATUS;
				sb.option[0] = CIO_NET_RCVRY_EXIT;
				sb.option[1] = 0;
				sb.option[2] = 0;
				sb.option[3] = 0;
				fddi_async_status(p_acs, &sb);
				fddi_logerr(p_acs, ERRID_FDDI_RCVRY_EXIT,
					__LINE__, __FILE__);

			}
			break;
		}
		default:
		{
			FDDI_DBTRACE("Aac9", 
					p_acs, 
					p_acs->dev.state,
					p_cmd->cmd_code);
			ASSERT (0);
			break;
		}
	} /* end switch (command type) */

	FDDI_DBTRACE("AacE", p_acs, p_acs->dev.state, p_cmd->cmd_code);

	/* the cmd block is set */
	return(0);
}


/*
 * NAME: fddi_shutdown_adap
 *                                                                    
 * FUNCTION: deactivate the adapter
 *                                                                    
 * EXECUTION ENVIRONMENT: Process environment.
 *	This routine disables interrupts to INTOFFL1 to serialize with
 *	oflv.
 *
 * NOTES: 
 *	This routine will take whatever action is necessary to quies the
 * 	adapter. The action taken is dependent on the current state.
 *
 *		FDDI_OPEN
 *		FDDI_LLC_DOWN
 *			The adapter is currently inserted on the ring.
 *			Clean up after any pending TX via the
 *			fddi_undo_start_tx() routine.  We need to clear
 *			the OFLV work queues.
 *
 *		FDDI_INIT
 *			We are in the process of activating the adapter.
 *			Stop the activation cmd timer and
 *			clear the OFLV work queues.
 *			
 *		FDDI_LIMBO
 *			We are in some phase of limbo.  
 *			Stop the limbo and cmd timers, clear the 
 *			OFLV work queues, error log the termination of limbo.
 *
 *		FDDI_DEAD
 *			The adapter is already quiesed.  Nothing
 *			to do for this state.
 *
 *	The adapter will always be reset and disabled via POS 2.
 *	The OFLV work queues (dev.oflv_events and dev.pri_que)
 *	will be cleared.  If there were no carry over errors detected,
 *	the state will be set to FDDI_DWNLD (if the state is not FDDI_NULL).
 *	If there was a carry over error detected, the state 
 *	will be set to FDDI_NULL.
 *
 *
 *	NB:
 *	The reason the adapter is always reset is to handle the 
 *	following scenario:
 *		1. the device is configured
 *		2. the device is opened and work is done on
 *		   the device.  The work done can be anything, the
 *		   key being that the microcode has been started.
 *		   Once the microcode is started the self tests 
 *		   results are lost.
 *		3. The device is unconfigured.
 *		4. The device is reconfigured.
 *		   If the device has not been reset, when the download
 *		   ucode ioctl runs, it will get potentially bad results
 *		   from the self tests and fail configuration.
 *
 *	It was decided to always reset the adapter and wait for the adapter
 *	to settle on the last close.  Where "settle" means to wait long 
 *	enough for the adapter to POS ID to be written out to the POS 0 and
 *	POS 1 by the adapter.  The MC spec allows 1 second for this to
 *	occur.
 *
 * RECOVERY OPERATION: 
 *
 *	None.
 *
 * DATA STRUCTURES: 
 *
 *	Cleanup Tx and cmd queues.
 *	The host descriptor structures for tx and rcv are cleared
 *	but the receive queue for user process reads will be maintained.
 *
 * RETURNS: none
 *
 */  
void 
fddi_shutdown_adap (
	fddi_acs_t	*p_acs)
{
	fddi_cmd_t	*p_cmd;
	int		ipri; 
	uint		bus;
	uint		iocc;
	uint		state;

	FDDI_DBTRACE("AsaB", p_acs, p_acs->dev.state, p_acs->tx.in_use);

	/*
	 * get access to the pos regs
	 */

	ipri = i_disable ( INTCLASS2 );

	state = p_acs->dev.state;
	p_acs->dev.state = FDDI_CLOSING;

	p_acs->dev.p_cmd_prog = NULL;
	if ( (state == FDDI_OPEN) ||
		(state == FDDI_LLC_DOWN) )
	{
		/*
		 * The FDDI_OPEN and FDDI_LLC_DOWN states are the
		 * same for the de-activation sequence.
		 */
		/* check for pending tx */
		if (p_acs->tx.in_use > 0)
		{
			w_stop (&(p_acs->tx.wdt));

			/* set a timer and sleep 
			 * 	The purpose of this timer is to
			 *	give the rest of the pending tx's
			 *	time to go out. If they are not
			 *	gone at the end of this timer then
			 *	they will never go (ie something is wrong).
			 *
			 */
			w_start (&(p_acs->dev.close_wdt));
			e_sleep (&(p_acs->dev.close_event), EVENT_SHORT);
		}
		/*
		 * NB:
		 *	We defer the cleaning up of the TX stuff
		 *	via fddi_undo_start_tx() until after the reset
		 *	of the adapter has taken place.
		 */
	}
	else if ( state == FDDI_INIT )
	{
		/* 
		 * we are in the middle of the activation sequence.
		 * stop the cmd timer, clear the OFLV work queue.
		 * 
		 * There is no TX nor RCV stuff to undo or stop.
		 */
		
		w_stop( &(p_acs->dev.cmd_wdt) );

	}
	else if (state == FDDI_LIMBO)
	{
		
		/*
		 * we are in some phase of limbo. 
		 * Error Log that we are terminating Limbo. Kill Limbo by
		 * resetting the adapter. Stop any timers that may be
		 * running, clear the OFLV work queue, and the
		 * priority cmd queue.
		 * 
		 * There is no TX nor RCV stuff to undo or stop.
		 */
		
		fddi_logerr(p_acs, ERRID_FDDI_RCVRY_TERM,
			__LINE__, __FILE__);


		w_stop( &(p_acs->dev.limbo_wdt) );
		w_stop( &(p_acs->dev.cmd_wdt) );
	}
	else if (state == FDDI_DEAD)
	{
		/*
		 * we've had a fatal error.
		 * !!! handle the cleanup of bug out.
		 * !!! handle carry over errors that will require us
		 * to goto the FDDI_NULL state
		 */
		FDDI_TRACE("Asa3", p_acs, p_acs->dev.state, 0);
	}
	else
	{
		/*
		 * never got to the OPEN state so not much to
		 *	do to deactivate ...
		 */
		FDDI_TRACE("Asa4", p_acs, p_acs->dev.state, 0);
	}

	/* 
	 * what ever our state is, we will reset and disable the adapter
	 * via the Card Enable bit in POS 2.
	 */
	
	iocc = IOCC_ATT(p_acs->dds.bus_id, (IO_IOCC+(p_acs->dds.slot<<16)) );
	PIO_PUTCX((iocc+FDDI_POS_REG2),  ( (p_acs->dev.pos2|FDDI_POS2_AR) 
					& ~(FDDI_POS2_CEN))); 
	p_acs->dev.oflv_events = 0;
	p_acs->dev.pri_que = 0;

	/* We should be able to delay now, the card has been reset and 
	 * interrupts arriving while the card is disabled, interrupts will
	 * be redisabled when the delay for the cards cpu is finished 
	 */

	i_enable (ipri);

	/*
	 * we now delay to allow the adapter reset affects to 
	 * settle. Then write the pos registers back out.
	 */
	delay(10*HZ);

	ipri = i_disable ( INTCLASS2 );

	PIO_PUTCX( (iocc + FDDI_POS_REG6), p_acs->dev.pos6 );
	PIO_PUTCX( (iocc + FDDI_POS_REG7), p_acs->dev.pos7 );
	PIO_PUTCX( (iocc + FDDI_POS_REG5), p_acs->dev.pos5 );
	PIO_PUTCX( (iocc + FDDI_POS_REG4), p_acs->dev.pos4 );
	PIO_PUTCX( (iocc + FDDI_POS_REG3), p_acs->dev.pos3 );
	PIO_PUTCX( (iocc + FDDI_POS_REG2), p_acs->dev.pos2 );

	IOCC_DET(iocc);

	/*
	 * clean up after the transmits
	 */
	if ( (state == FDDI_OPEN) || (state == FDDI_LLC_DOWN) )
		fddi_undo_start_tx(p_acs);
	/*
	 * If we have had a carry over error (a FDDI_MEM_ACC ioctl
	 * for example), no matter what our state is, we will go to
	 * the FDDI_NULL state to force a download a ucode.
	 *
	 * Otherwise, if we ever made it past the FDDI_NULL state we
	 * will go to the FDDI_DWNLD state.
	 */

	if ((p_acs->dev.carryover == TRUE) || (p_acs->dev.stest_cover == TRUE))
		p_acs->dev.state = FDDI_NULL;
	else if (p_acs->dev.state != FDDI_NULL)
		p_acs->dev.state = FDDI_DWNLD;

	i_enable (ipri);

	FDDI_DBTRACE("AsaE", p_acs, p_acs->dev.state, 0);
	return;
} /* end fddi_shutdown_adap() */

void
fddi_close_wait (
	struct	watchdog	*p_wdt)
{
	fddi_acs_t		*p_acs;
	int			ipri;

	p_acs = (fddi_acs_t *) ((uint)p_wdt - 
		((uint)offsetof (fddi_acs_t, dev) + 
		(uint)offsetof (fddi_acs_dev_t, close_wdt)));

	ipri = i_disable ( INTCLASS2 );
	FDDI_DBTRACE("tcwB", p_acs, p_acs->dev.state, 0);

	p_acs->dev.oflv_events |= FDDI_CLOSE_WDT_IO;
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
	i_enable (ipri);

	return;
}
