static char sccsid[] = "@(#)82	1.1  src/bos/kernext/fddidiag/fddiep_b.c, diagddfddi, bos411, 9428A410j 11/1/93 11:00:14";
/*
 *   COMPONENT_NAME: DIAGDDFDDI
 *
 *   FUNCTIONS: fddi_cdt_func
 *		fddi_chk_events
 *		fddi_clean_swque
 *		fddi_free_addr
 *		fddi_get_query
 *		fddi_get_stat
 *		fddi_halt_netid
 *		fddi_query_stats_cmplt
 *		fddi_remove_netids
 *		fddi_send_cmd
 *		fddi_sq_addr_cmplt
 *		fddi_start_netid
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
#include <sys/poll.h>
#include <sys/adspace.h>
#include <sys/iocc.h>
#include <sys/ioacc.h>
#include <sys/sleep.h>

/* 
 * get access to the global device driver control block
 */
extern fddi_ctl_t	fddi_ctl;
extern fddi_trace_t	fdditrace;

/*
 * externs for FDDI component dump table objects
 */
extern struct cdt	*p_fddi_cdt;
extern int		l_fddi_cdt;

/*
 * NAME: fddi_cdt_func
 *                                                                    
 * FUNCTION: component dump function called at dump time
 *                                                                    
 * EXECUTION ENVIRONMENT: process environment only
 *                                                                   
 * NOTES: 
 * 	The function is called during a component dump.  The buffer used for the
 * small frame cache (see tx section) was previously marked to be dumped and 
 * at this point the sf_cache will be filled with the available data from the
 * adapter.  First the 8 bytes of pos regs are saved.  Then the 16 bytes of
 * sif registers.  Finally the 0x100 bytes of shared ram is dumped.
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: ptr to the current component dump table
 */  
struct cdt *
fddi_cdt_func(
	int	pass )
{
	
	int		iocc;
	int 		ioa;
	int		bus;
	int		i,cnt;
	int		loop,off;
	ushort		tmp;
	fddi_acs_t	*p_acs;
	
	if (pass == 1)
	{
		i=0;
		cnt = 0;

		while ((i<FDDI_MAX_MINOR) && (cnt < fddi_ctl.acs_cnt))
		{
			if ((p_acs = fddi_ctl.p_acs[i]) != NULL)
			{	
				cnt++;
				bzero(p_acs->tx.p_sf_cache, FDDI_SF_CACHE);
				iocc =IOCC_ATT(p_acs->dds.bus_id,IO_IOCC+
					(p_acs->dds.slot<<16));
				ioa = BUSIO_ATT(p_acs->dds.bus_id, 
					p_acs->dds.bus_io_addr);
				bus=BUSMEM_ATT(p_acs->dds.bus_id,
					(uint)p_acs->dds.bus_mem_addr);

				for (loop = 0; loop < 8; loop++)
					BUS_GETCX (iocc + loop, &p_acs->tx.p_sf_cache[loop]);
				off = 8;
	
				for (loop = 0; loop < 8; loop++)
				{
					BUS_GETSRX (ioa + loop, &tmp);
					p_acs->tx.p_sf_cache[(2 * loop) + off] = tmp >> 8;
					p_acs->tx.p_sf_cache[(2 * loop) + off + 1] = 0xff & tmp;
				}
				off = 24;
	
				for (loop = 0; loop < 0x100; loop++)
				{
					BUS_GETSRX (bus + loop, &tmp);
					p_acs->tx.p_sf_cache[(2 * loop) + off] = tmp >> 8;
					p_acs->tx.p_sf_cache[(2 * loop) + off + 1] = 0xff & tmp;
				}
			
				BUSMEM_DET(bus);
				BUSIO_DET(ioa);
				IOCC_DET(iocc);
			}
			i++;
		}
	}

	return (p_fddi_cdt);
}

/*
 * NAME: fddi_get_stat
 *                                                                    
 * FUNCTION: Returns a status block if one is available
 *                                                                    
 * EXECUTION ENVIRONMENT: called from process environment only
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 *
 * RETURNS: 
 *	!!! Parameter checking errors caught in fddiioctl()
 *
 */  

int
fddi_get_stat(
	fddi_acs_t	*p_acs, 
	fddi_open_t	*p_open, 
	cio_stat_blk_t	*p_arg,
	ulong		devflag)
{
	cio_stat_blk_t	err_sb;
	int		rc;
	int		ipri;

	if (p_arg == NULL)
	{
		return (EINVAL);
	}
	if (p_open->devflag & DKERNEL)
	{
		return (EACCES);
	}

	ipri = i_disable (INTOFFL1);
	/*
	 * Check to see if the queue is empty?
	 */
	if (p_open->stat_cnt <= 0)
	{
		i_enable (ipri);
		err_sb.code = CIO_NULL_BLK;
		if (MOVEOUT(devflag,&err_sb,p_arg,sizeof(cio_stat_blk_t)))
			return(EFAULT);
		return (0);
	}
	
	i_enable (ipri);
	if (MOVEOUT (devflag, p_open->p_statq + p_open->stat_out, p_arg, 
		sizeof(cio_stat_blk_t)))
	{
		/* if the copyout failed, return */
		return (EFAULT);
	}
	ipri = i_disable (INTOFFL1);
	
	INCR_STATQ_INDEX(p_acs, p_open->stat_out);
	if (p_open->stat_ls_cnt > 0)
		p_open->stat_ls_cnt--;

	if (p_open->stat_que_ovflw == TRUE)
	{
		p_open->stat_que_ovflw = FALSE;

		p_open->stat_ls_cnt = p_acs->dds.stat_que_size;
		err_sb.code = CIO_LOST_STATUS;
		p_open->p_statq[p_open->stat_in] = err_sb;
		INCR_STATQ_INDEX(p_acs, p_open->stat_in);
	}
	else
		p_open->stat_cnt--;
		
	i_enable (ipri);
	return(0);
}

/*
 * NAME: fddi_start_netid
 *                                                                    
 * FUNCTION: 
 *                                                                    
 * EXECUTION ENVIRONMENT: called from process environment only
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 *
 * RETURNS: 
 *	Parameter checking errors caught in fddiioctl()
 *
 */  
void
fddi_start_netid(
	fddi_acs_t	*p_acs, 
	fddi_open_t	*p_open, 
	netid_t		netid)
{
	int ipri;
	
	ipri = i_disable(INTOFFL1);
	/* 
	 * if netid is good, set the netid
	 * table element indexed by netid to the open ptr.
	 */
	p_acs->ctl.p_netids[netid] = p_open;

	++p_open->netid_cnt;
	++p_acs->ctl.netid_cnt;

	i_enable(ipri);
	return;
}

/*
 * NAME: fddi_halt_netid
 *                                                                    
 * FUNCTION: 
 *                                                                    
 * EXECUTION ENVIRONMENT: called from process environment only
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 *
 * RETURNS: 
 *	Parameter checking errors caught in fddiioctl()
 *
 */  
void
fddi_halt_netid(
	fddi_acs_t	*p_acs, 
	fddi_open_t	*p_open, 
	netid_t		netid)
{
	int ipri;
	
	ipri = i_disable(INTOFFL1);

	/* 
	 * if netid is good, set the netid
	 * table element indexed by netid to the open ptr.
	 */
	
	p_acs->ctl.p_netids[netid] = NULL;

	--p_open->netid_cnt;
	--p_acs->ctl.netid_cnt;

	i_enable(ipri);
	return;
}
/*
 * NAME: fddi_clean_swque
 *                                                                    
 * FUNCTION: 
 *                                                                    
 * EXECUTION ENVIRONMENT: called from process environment only
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 *
 * RETURNS: 
 *	Parameter checking errors caught in fddiioctl()
 *
 */  
void
fddi_clean_swque(
	fddi_acs_t	*p_acs, 
	fddi_open_t	*p_open) 
{
	int ipri;
	int i, t;
	fddi_tx_que_t	*p_tx_que;
	
	ipri = i_disable(INTOFFL1);

	t = p_acs->tx.tq_out;

	for (i=0; i<p_acs->tx.tq_cnt; i++)
	{
		DESC_TQ(p_tx_que, t);
		if (p_tx_que->p_open == p_open)
			p_tx_que->p_open = 0;
		INCRE_TQ(t);
	}

	i_enable(ipri);
	return;
}

/*
 * NAME: fddi_get_query
 *                                                                    
 * FUNCTION: 
 *                                                                    
 * EXECUTION ENVIRONMENT: called from process environment only
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 *
 * RETURNS: 
 *	Parameter checking errors caught in fddiioctl()
 *
 */  
int
fddi_get_query(
	fddi_acs_t		*p_acs, 
	struct query_parms	*p_query,
	int			len, 
	ulong			devflag)
{
	int			ipri;
	/* 
	 * With interrupts disabled get and optionally clear the RAS area.
	 */
	
	if (MOVEOUT (devflag, &p_acs->ras, p_query->bufptr, len))
	{
		/* if the copyout failed, return */
		return (EFAULT);
	}

	ipri = i_disable (INTOFFL1);

	if (p_query->clearall == CIO_QUERY_CLEAR)
	{
		bzero (&p_acs->ras, (sizeof(fddi_query_stats_t) - 
					sizeof(fddi_links_t)));
	}

	i_enable (ipri);
	return(0);
}



/*
 * NAME: fddi_send_cmd
 *                                                                    
 * FUNCTION: 
 *                                                                    
 * EXECUTION ENVIRONMENT: called from process environment only
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 *
 * RETURNS: 
 *	Parameter checking errors caught in fddiioctl()
 *
 */  
int				
fddi_send_cmd(
	fddi_acs_t	*p_acs, 
	fddi_open_t	*p_open, 
	fddi_cmd_t 	*p_cmd)
{
	int		ipri;
	int		bus;

	bus = BUSMEM_ATT(p_acs->dds.bus_id, (ulong)p_acs->dds.bus_mem_addr);
	ipri = i_disable(INTOFFL1);
	p_acs->dev.ioctl_status = CIO_OK;
	(void) fddi_issue_cmd(p_acs, p_cmd, bus);
	BUSMEM_DET(bus);
	
	if (e_sleep(&p_acs->dev.ioctl_event, EVENT_SHORT) != EVENT_SUCC)
	{
		/* !!! ERRLOG */
		i_enable(ipri);
		FDDI_TRACE("Isl3",p_acs->dev.state, p_open, 0);
		return(EINTR);
	}
	i_enable(ipri);
	return(0);
}

/*
 * NAME: fddi_query_stats_cmplt
 *                                                                    
 * FUNCTION: completes the results of the query stats command
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt environment (during DDC interrupt)
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: none
 */  
void
fddi_query_stats_cmplt (
	fddi_acs_t	*p_acs,
	fddi_cmd_t	*p_cmd,
	int		bus)
{
	fddi_uls_handler(p_acs,p_cmd,bus);
	e_wakeup(&p_acs->dev.ioctl_event);
	p_cmd->cmd_code=0;
}

/*
 * NAME: fddi_sq_addr_cmplt
 *                                                                    
 * FUNCTION: completes and interprets the results of a set addr command
 *                                                                    
 * EXECUTION ENVIRONMENT: interrupt environment (during DDC interrupt)
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: none
 */  


int
fddi_sq_addr_cmplt(
	fddi_acs_t 	*p_acs, 
	fddi_cmd_t	*p_cmd,
	int		bus)
{

	FDDI_DBTRACE("SsaB", p_acs, p_acs->dev.state, p_cmd);
	if (p_cmd->stat != FDDI_HCR_SUCCESS)
	{
		/*
		 * command failed: wake up the caller 
		 * 	(invalid setcount handled in fddi_cmd_handler())
		 */
		FDDI_TRACE("Ssa1", p_acs, p_cmd->cmd_code, p_cmd->stat);

		switch(p_cmd->stat)
		{
			case FDDI_HCR_NO_ADDR :
				p_acs->dev.ioctl_status = FDDI_NO_ADDR;
				break;
			case FDDI_HCR_INV_ADDRTYPE :
			case FDDI_HCR_CLEAR_SKY :
				p_acs->dev.ioctl_status = CIO_INV_CMD;
				break;
			case FDDI_CIO_TIMEOUT :
				p_acs->dev.ioctl_status = CIO_TIMEOUT;
				break;
			default:
				p_acs->dev.ioctl_status = CIO_HARD_FAIL;
		}

		e_wakeup(&p_acs->dev.ioctl_event);

		p_cmd->cmd_code = 0;
		/* err path return */
		return ;
	}
	
	/* 
	 * Command specific action taken below
	 */
	if (p_cmd->cmd_code == FDDI_HCR_READ_ADDR)
	{
		int 	i;
		int 	off;
		ushort	*p_addr;
		/* 
		 * read count and address from shared memory 
		 *	Swap bytes in the count field but read addresses
		 *	in as they are. 
		 *	Count goes into 6th parameter (index 5)
		 *	Addresses go into 7th param (index 6) and beyond
		 */
		off = 6; 
		p_addr = &p_cmd->cpb[3];
		PIO_GETSRX (bus + FDDI_CPB_SHARED_RAM + off, p_addr);
		off += 2;
		p_addr++;
		for (i = 0; i < (p_cmd->cpb[3]*3); i++)
		{
			PIO_GETSX (bus + FDDI_CPB_SHARED_RAM + off, p_addr);
			off += 2;
			p_addr++;
		}
	}
	p_cmd->cmd_code = 0;
	e_wakeup(&p_acs->dev.ioctl_event);
	FDDI_DBTRACE("SsaE", p_acs, p_acs->dev.state, p_cmd);
	return;
}


/*
 * NAME:     fddi_remove_netids()
 *
 * FUNCTION: 
 *	Removes the Net IDs that have been started by this user.
 *
 * EXECUTION ENVIRONMENT:
 *	Process thread or interrrupt environment.
 *
 * NOTES:
 *	This routine will set the Net ID open ptrs that this user
 *	has registered to NULL.  
 *
 *	
 * ROUTINES CALLED:
 *	i_disable(), i_enable(), 
 *
 * RETURNS:
 *		0	- successful
 *
 */
int
fddi_remove_netids(	
	fddi_acs_t	*p_acs,		/* ACS ptr */
	fddi_open_t	*p_open)	/* open ptr */
{
	int 	ipri;
	int	i;

	FDDI_DBTRACE("CrnB", p_acs->ctl.netid_cnt, p_open->netid_cnt, 0);
	/* 
	 * we must serialize with the OFLV receive processing
	 * routines that look at the netid hash table.
	 */
	ipri = i_disable(INTOFFL1);

	for (i = 0; i < FDDI_MAX_NETIDS; i++)
	{
		/* 
		 * If this Net ID is in use by this user,
		 * set the p_netid[i] array element ptr to NULL.
		 * Decrement the netid counts for the open element
		 * and the ACS control area.
		 */
		if (p_open == p_acs->ctl.p_netids[i])
		{
			p_acs->ctl.p_netids[i] = NULL;
			--p_open->netid_cnt;
			--p_acs->ctl.netid_cnt;
		}
	}
	ASSERT(p_open->netid_cnt == 0);

	i_enable(ipri);

	FDDI_DBTRACE("CrnE", p_acs->ctl.netid_cnt, p_open->netid_cnt, i);

	return(0);

} /* end fddi_remove_netids() */

/*
 * NAME:     fddi_free_addr()
 *
 * FUNCTION: 
 *	Clears the addresses this user set and has left set
 *
 * EXECUTION ENVIRONMENT:
 *	Process thread
 *
 * NOTES:
 *
 *	
 * ROUTINES CALLED:
 *
 * RETURNS:
 *		0	- successful
 *
 */
void
fddi_free_addr(	
	fddi_acs_t	*p_acs,		/* ACS ptr */
	fddi_open_t	*p_open)	/* open ptr */
{
	int 			i,adap;
	int			ipri;
	int			flag;
	fddi_set_addr_t		saddr;
	fddi_cmd_t		*p_cmd;
	char			*p_addr;
	extern int		fddi_sq_addr_cmplt();

	ipri = i_disable(INTOFFL1);
	/*
	 * Remove any addresses set by this user
	 */
	p_cmd = &(p_acs->dev.cmd_blk);
	flag = 0;

	for (i=0; i<FDDI_MAX_ADDRS; i++)
	{
		/*
		 * If this user has addresses; then find out if we are in limbo
		 * if we are then the command cannot be sent down to the card
		 * as the card is either in the middle of a reset or we are 
		 * issuing commands to the hsr automatically, and cannot issue
		 * them from the user process also, as the hcr command register
		 * can handle only one command at a time.  If we aren't in limbo
		 * issue the command and sleep until the cci occurs or there is
		 * a timeout.  In either case go on to the next address.
		 */
		if (p_open->addrs & (1<<i))
			if (p_acs->dev.state != FDDI_LIMBO)
			{
				if (--p_acs->ctl.addrs[i].cnt == 0)
				{
					/*
			 	 	* Reset command structure each time
			 	 	*/
					p_cmd->pri = 0;
					p_cmd->cmplt = (int(*)())
						fddi_sq_addr_cmplt;
					p_cmd->cmd_code = FDDI_HCR_CLEAR_ADDR;
					p_cmd->cpb_len = 14;
					p_cmd->cpb[2]=
						SWAPSHORT(FDDI_HCR_ALD_SKY);
					p_cmd->cpb[3] = 
						SWAPSHORT (FDDI_HCR_ATD_SKY);

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

					/*
			 		* send the clear address command and
					* sleep until the CCI comes in or the
					* command timer pops.
			 		*/
					fddi_send_cmd(p_acs, p_open,p_cmd);
				}
			}
			else
			{
				if ((--p_acs->ctl.addrs[i].cnt) == 0)
					flag = 1;
			}
				

	}

	/*
	* Now if the flag is set, then at least one address was closed (no
	* other user had it open) and we were in limbo.  Because we were in
 	* limbo, the interrupts have been disabled continuously, so as long as
 	* the reset wasn't finished (the limbo_to flag is set to TRUE when the
	* the reset completes and the resetting of the state commences 
	* (including the reseting of the addresses)), we are fine.  The reset
 	* itself clears the addresses and the driver code to recover the 
	* addresses uses the same counters we decrimented (in other words the
	* addresses have been effectively removed without individual commands).
	* If the driver had begun to restore the addresses then we do not 
	* know how far it has gotten, and do not know which commands we would
	* have to issue, so we reissue the reset (by re-entering limbo) and 
	* this will reset only the addresses still active.
	*/

	if ((flag == 1) && (p_acs->dev.limbo_to == FALSE))
		fddi_enter_limbo(p_acs, FDDI_CMD_FAIL, 0);

	i_enable(ipri);
	return;
}



/*
 * NAME: fddi_chk_events
 *                                                                    
 * FUNCTION: check the select events
 *                                                                    
 * EXECUTION ENVIRONMENT: process environment with interrupts disabled
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS:  the events
 */  

ushort
fddi_chk_events (
	fddi_acs_t	*p_acs,
	fddi_open_t	*p_open,
	ushort		events)
{
	ushort		revents = 0;
	int		ipri;

	FDDI_DBTRACE("sceB", p_acs, p_acs->dev.state, events);
	/* 
	 * Disable interrupts to prevent missing a requested event 
	 * 	(i.e. an event occurring between the test for the event 
	 *	and the set of notify).
	 */
	ipri = i_disable (INTOFFL1);

	if ((p_acs->dev.state == FDDI_DEAD) || (p_acs->dev.state == FDDI_LIMBO))
	{
		/* 
		 * return 1's for everything requested 
		 * if in an unrecoverable state 
		 */
		revents = (events & ~POLLSYNC);
		FDDI_DBTRACE("sce1", p_acs, revents, events);
		i_enable(ipri);
		return (revents);
	}
	/* check for rcv data */
	if ((events & POLLIN) && (p_open->rcv_cnt != 0))
	{
		revents |= POLLIN;
	}
	/* check for tx data */
	if ((events & POLLOUT) && (p_acs->tx.in_use != FDDI_MAX_TX_DESC))
	{
		revents |= POLLOUT;
		/*
		 * remember to alert after next tx complete
		 */
		p_acs->tx.needed = TRUE;
	}
	/* check for status */
	if ((events & POLLPRI) && (p_open->stat_cnt != 0))
	{
		revents |= POLLPRI;
	}
	/*
	 * Store the requested events only if all the requests
	 *	are false and POLLSYNC is not set.
	 */
	if (!(revents) && !(events & POLLSYNC))
	{
		/* store for asynchronous notification */
		p_open->selectreq = events;
	}
	/* restore interrupts, unlockl acs and return ok */
	i_enable (ipri);

	FDDI_DBTRACE("sceE", p_acs, revents, events);
	return (revents);
}

