static char sccsid[] = "@(#)72	1.9  src/bos/kernext/fddi/fddi_open.c, sysxfddi, bos411, 9428A410j 6/18/94 18:00:24";
/*
 *   COMPONENT_NAME: SYSXFDDI
 *
 *   FUNCTIONS: fddi_open
 *		hcr_act_cmplt
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
#include "fddiproto.h"
#include <sys/priv.h>
#include <sys/dma.h>
#include <sys/malloc.h>
#include <sys/sleep.h>

/* 
 * get access to the global device driver control block
 */
extern fddi_tbl_t	fddi_tbl;
extern struct cdt	*p_fddi_cdt;

/*
 * NAME: fddi_open()
 *                                                                    
 * FUNCTION: Open entry point from kernel.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine runs on the process thread.  It assumes interrupts have not
 *	been disabled.
 *	
 *	This routine synchronizes with the config routine (CFG_TERM in 
 *	particular) through the network services.  This routine will only be
 *	called as a result of an ns_alloc.  The first thing CFG_TERM does is
 * 	call ns_dettach which if called first will prevent any subsequent 
 *	calls to open.  If the open is first the ns_dettach will fail because
 * 	of the reference counts.
 *                                                                   
 * NOTES:
 *
 * RETURNS: 
 *		0		- successful
 *		ENOCONNECT	- Indicates that the activation commands failed
 *		EINVAL		- Indicates that an invalid parameter was 
 *					specified
 *		ENETDOWN 	- Indicates that there was a pio failure.
 */  

int
fddi_open( fddi_acs_t *p_acs)
{
	int 		rc = 0;
	int 		ioa;
	fddi_cmd_t	p_cmd;
	int		ipri;
	int		bus;	/* to pass to issue command */

	rc = pincode(fddi_config);
	if (rc != 0)
	{
		return(ENOMEM);
	}

	FDDI_TRACE("OfoB",p_acs,0,0);

	if (p_acs->dev.state != DNLD_STATE)
	{
		FDDI_ETRACE("Ofo1",p_acs->dev.state,0,0);

		/* unpin the driver code */
		unpincode(fddi_config);

		return(EINVAL);
	}

	if ((rc = init_services(p_acs)) != 0)
	{
		FDDI_ETRACE("Ofo2",rc,0,0);

		/* unpin the driver code */
		unpincode(fddi_config);

		return(EINVAL);
	}

	if (fddi_tbl.open_cnt == 0)
	{
		if ((rc = fddi_cdt_init()) != 0)
		{
			FDDI_ETRACE("Ofo3",rc,0,0);
			free_services(p_acs);
			unpincode(fddi_config);
			return(rc);
		}
	}


	if ((rc = init_tx(p_acs)) != 0)
	{
		FDDI_ETRACE("Ofo4",rc,0,0);

		if (fddi_tbl.open_cnt == 0)
			fddi_cdt_undo_init();

		free_services(p_acs);
		unpincode(fddi_config);
		return(rc);
	}


	if ((rc = init_rx(p_acs)) != 0)
	{
		FDDI_ETRACE("Ofo5",rc,0,0);

		free_tx(p_acs);
		if (fddi_tbl.open_cnt == 0)
			fddi_cdt_undo_init();

		free_services(p_acs);
		unpincode(fddi_config);
		return(rc);
	}

	ioa = BUSIO_ATT(p_acs->dds.bus_id, p_acs->dds.bus_io_addr);

	PIO_PUTSRX(ioa + FDDI_HMR_REG, FDDI_HMR_RX_TX);

	BUSIO_DET(ioa);

	/* 
	 * Check for pio errors.
	 */
	if (p_acs->dev.pio_rc)
	{
		FDDI_ETRACE("Ofo6",0,0,0);

		free_tx(p_acs);
		free_rx(p_acs);
		if (fddi_tbl.open_cnt == 0)
			fddi_cdt_undo_init();
		free_services(p_acs);

		/* unpin the driver code */
		unpincode(fddi_config);

		return(ENETDOWN);
	}

	p_acs->dev.state = OPENING_STATE;
	
	/* 
	 * initialize cmd structure for 1st activation command
	 */
	p_cmd.stat = 0;
	p_cmd.pri = 0;
	p_cmd.cmplt = (int(*)()) hcr_act_cmplt;
	p_cmd.cmd_code = FDDI_HCR_START_MCODE;
	p_cmd.cpb_len = 0; 

	/* serialize with SLIH */
	ipri = disable_lock ( CFDDI_OPLEVEL, &p_acs->dev.cmd_lock );

	/* 
	 * send the first activation command : we will be wake up when the 
	 * activation is done.
	 */
	send_cmd(p_acs, &p_cmd);

	/* 
	 * Check the results of the activation commands 
	 */
	if (p_acs->dev.cmd_status != 0)
	{
		FDDI_ETRACE("Ofo7",p_acs->dev.cmd_status,0,0);

		unlock_enable(ipri, &p_acs->dev.cmd_lock);
		free_tx(p_acs);
		free_rx(p_acs);

		if (fddi_tbl.open_cnt == 0)
			fddi_cdt_undo_init();

		free_services(p_acs);
		p_acs->dev.state = DNLD_STATE;

		/* unpin the driver code */
		unpincode(fddi_config);

		return(ENOCONNECT);
	}

	fddi_cdt_add ("fddi_acs", (char *) p_acs,  sizeof (fddi_acs_t));
	fddi_cdt_add ("adapter", p_acs->tx.p_sf_cache, 
			sizeof(fddi_adap_dump_t));

	fddi_tbl.open_cnt++;

	p_acs->ndd.ndd_flags |= NDD_UP | NDD_RUNNING;
	p_acs->dev.state = OPEN_STATE;
	unlock_enable(ipri, &p_acs->dev.cmd_lock);

	FDDI_TRACE("OfoE",0,0,0);
	return(0);

} /* end fddi_open() */

/*
 * NAME: hcr_act_cmplt
 *                                                                    
 * FUNCTION: completion routine for the activation sequence: issues all of 
 *	activation commands in order.
 *                                                                    
 * EXECUTION ENVIRONMENT: 
 *	interrupt environment (during CCI interrupt)
 *
 *	The slih and cmd lock are locked when this routine is called.
 *                                                                   
 * NOTES: 
 *
 *	This routine manages the activation sequence. It can be 
 *	initiated by 'fddi_open' during the first activation or by
 *	network recovery logic during an attempt to reactivate.
 *
 *	Managing the activation sequence is making sure of the order
 *	commands are issued and handling errors. Error handling is state
 *	dependent.
 *
 *	The following sequence of commands are issued to the 
 *	adapter for activation:
 *
 * 		FDDI_HCR_WR_USR_DATA
 * 		FDDI_HCR_WR_PASSWORD
 * 		FDDI_HCR_WRITE_ADDR
 * 		FDDI_HCR_WR_ATT_CLASS
 * 		FDDI_HCR_WR_MAX_TREQ
 * 		FDDI_HCR_WR_TVX_LOW_BND
 * 		FDDI_HCR_CONNECT
 *
 *	To issue a command the routine needs to set up the command to be issued
 *	in the p_cmd structure and return.  The cmd_handler will issue the 
 *	command automaticly.  
 *
 * RECOVERY OPERATION: 
 *
 *	We can be in different states while executing this routine.
 *	As long as no errors occur the processing is independent of
 *	the state we are in. 
 *
 * RETURNS: none
 */  


void 
hcr_act_cmplt (
	fddi_acs_t	*p_acs,
	fddi_cmd_t	*p_cmd,
	int		bus,		/* not used */
	int		ipri)
{
	static void	act_error();
	int		i;
	ushort 		tmppasswd;
	int		rc = 0;

	FDDI_TRACE("OacB",p_acs, p_cmd->cmd_code, p_cmd->stat);

	/* 
	 * Set up the next command based on which command has just completed.
	 */
	switch (p_cmd->cmd_code)
	{
		case FDDI_HCR_START_MCODE:
		{
			/* 
			 * Check the results of the last command 
			 */
			if (p_cmd->stat  != FDDI_HCR_SUCCESS)
			{
				rc = p_cmd->stat;
				break;
			}

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
			 * Check the results of the last command 
			 */
			if (p_cmd->stat  != FDDI_HCR_SUCCESS)
			{
				rc = p_cmd->stat;
				break;
			}

			/* 
			 * Add the address byte swapped to the cpb
			 */
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
			 * Check the results of the last command 
			 */
			if (p_cmd->stat  != FDDI_HCR_SUCCESS)
			{
				rc = p_cmd->stat;
				break;
			}

			/*
			 * Add the address and the descriptors to the 
			 * cpb
			 */
			p_cmd->cpb[2] = 0;
			p_cmd->cpb[3] = SWAPSHORT (FDDI_HCR_ATD_FOR);
			p_addr = (char *) &(p_cmd->cpb[4]);

			*(p_addr++) = p_acs->addrs.src_addr[5];
			*(p_addr++) = p_acs->addrs.src_addr[4];
			*(p_addr++) = p_acs->addrs.src_addr[3];
			*(p_addr++) = p_acs->addrs.src_addr[2];
			*(p_addr++) = p_acs->addrs.src_addr[1];
			*(p_addr++) = p_acs->addrs.src_addr[0];

			p_cmd->cmd_code = FDDI_HCR_WRITE_ADDR;
			p_cmd->cpb_len = 14;

			break;
		}
		case FDDI_HCR_WRITE_ADDR:
		{
			/* 
			 * Check the results of the last command 
			 */
			if (p_cmd->stat  != FDDI_HCR_SUCCESS)
			{
				rc = p_cmd->stat;
				break;
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
			 * Check the results of the last command 
			 */
			if (p_cmd->stat  != FDDI_HCR_SUCCESS)
			{
				rc = p_cmd->stat;
				break;
			}

			/* 
			 * Take the 2's complement of 1/80th of the t_req
			 * value passed in at cfg time to meet the adapter's
			 * write_treq format.  (this is the format the 
			 * microcode expects it in).
			 */
			temp_treq = ~(p_acs->dds.t_req / 80) + 1;

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
			 * Check the results of the last command 
			 */
			if (p_cmd->stat  != FDDI_HCR_SUCCESS)
			{
				rc = p_cmd->stat;
				break;
			}

			/* 
			 * Take the 2's complement of 1/80th of the tvx
			 * value passed in at cfg time to meet the adapter's
			 * write_tvx format.  (This is the format the microcode
			 * is expecting the tvx in).
			 */
			temp_tvx = ~(p_acs->dds.tvx / 80) + 1;

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
			 * Check the results of the last command 
			 */
			if (p_cmd->stat  != FDDI_HCR_SUCCESS)
			{
				rc = p_cmd->stat;
				break;
			}

			/*
			 * The smt_control word has the multicast, smt, crc,
			 * nsa, beacon and promiscuous options or'd into it
			 * so they will be reset during re-activation.
			 */
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

			p_cmd->cmd_code = FDDI_HCR_CONNECT;
			p_cmd->cpb_len = 16;
			break;
		}
		case FDDI_HCR_CONNECT:
		{
			/*  
			 * check for errors 
			 */
			if (p_cmd->stat  != FDDI_HCR_SUCCESS)
			{
				rc = p_cmd->stat;
				break;
			}

			p_cmd->cmd_code = FDDI_HCR_ULS;
			p_cmd->cpb_len = 0;
			break;
		}

		case FDDI_HCR_ULS:
		{
			/*  
			 * check for errors 
			 */
			if (p_cmd->stat  != FDDI_HCR_SUCCESS)
			{
				rc = p_cmd->stat;
				break;
			}

			hcr_uls_cmplt(p_acs,p_cmd,bus, ipri);

			/*
			 * Clear the cmd_code to prevent the command from being
			 * reissued.
			 */
			p_cmd->cmd_code = 0;

			/*
			 * Start the receive (reissue the receive command)
			 */
			if (start_rx(p_acs))
			{
				unlock_enable(ipri, &p_acs->dev.cmd_lock);
				rc = ENETDOWN;
				fddi_logerr(p_acs, ERRID_CFDDI_PIO, 
					__LINE__, __FILE__,
					0, 0, 0);
				bugout(p_acs, NDD_PIO_FAIL, 0, 0, TRUE);
				ipri = disable_lock(CFDDI_OPLEVEL,
						&p_acs->dev.cmd_lock);
				break;
			}

			if (p_acs->dev.state == LIMBO_RECOVERY_STATE) 
			{
				ndd_statblk_t stat;			

				p_acs->ndd.ndd_flags &= ~(NDD_LIMBO);
				p_acs->ndd.ndd_flags |= NDD_RUNNING;
				p_acs->dev.state = OPEN_STATE;

				bzero(&stat, sizeof(ndd_statblk_t));
				stat.code = NDD_LIMBO_EXIT;

				p_acs->ndd.nd_status(p_acs, &stat);

				fddi_logerr(p_acs, 
					ERRID_CFDDI_RCVRY_EXIT, 
					__LINE__, __FILE__, 0, 0, 0);
			}
			else
			{
				e_wakeup(&p_acs->dev.cmd_event);
			}

			break;
		}
		default:
		{
			FDDI_ASSERT (0);
			FDDI_ETRACE("Oac1",p_acs,p_cmd->cmd_code,0);
			rc=EINVAL;
			break;
		}
	} /* end switch (command type) */

	if (rc != 0)
	{
		FDDI_ETRACE("Oac2",p_acs, rc, p_acs->dev.state);

		p_cmd->cmd_code = 0;

		if (p_acs->dev.state == LIMBO_RECOVERY_STATE)
		{
			unlock_enable(ipri, &p_acs->dev.cmd_lock);

			enter_limbo(p_acs, NDD_CMD_FAIL,0, TRUE);
			ipri = disable_lock(CFDDI_OPLEVEL,&p_acs->dev.cmd_lock);
		}
		else
		{ 
			p_acs->dev.cmd_status = EIO;
			e_wakeup(&p_acs->dev.cmd_event);
		}
	}

	/* the cmd block is set */
	FDDI_TRACE("OacE",p_acs, p_cmd->cmd_code, p_cmd->stat);
	return;
}
