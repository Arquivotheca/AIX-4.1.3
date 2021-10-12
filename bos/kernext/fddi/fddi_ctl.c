static char sccsid[] = "@(#)69	1.16  src/bos/kernext/fddi/fddi_ctl.c, sysxfddi, bos411, 9428A410j 4/6/94 11:29:57";
/*
 *   COMPONENT_NAME: SYSXFDDI
 *
 *   FUNCTIONS: add_filter
 *		add_status
 *		clear_stats
 *		del_filter
 *		del_status
 *		disable_addr
 *		disable_multicast
 *		enable_addr
 *		enable_multicast
 *		fddi_ctl
 *		get_stats
 *		hcr_addr_cmplt
 *		hcr_all_addr_cmplt
 *		hcr_smt_cmplt
 *		hcr_stat_cmplt
 *		mib_addr
 *		mib_get
 *		mib_query
 *		prom_off
 *		prom_on
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
#include <sys/dma.h>
#include <sys/mbuf.h>
#include <sys/poll.h>
#include <sys/sleep.h>
#include <net/net_malloc.h>
#include <sys/malloc.h>
#include <sys/ndd_var.h>

/* externs */
extern	fddi_tbl_t	fddi_tbl;

/*
 * NAME: fddi_ctl
 *
 * FUNCTION: handles the ctl calls to the fddi device driver
 *
 * EXECUTION ENVIRONMENT: process thread: 
 *
 * NOTES: 
 *	lock cmd lock (release it when sleeping for the command)
 *	check each command's parameters for validity (this is important 
 *			as the design assumes invalid commands make to the 
 *			adapter).
 *	set up the command and adjust the data structures assuming the command
 *			will complete successfully.
 *	send the command to the adapter (if this fails it will be due to an
 *			unrelated incident, or pio error, in either case network
 *			recovery mode will be entered, and on recovery the reset
 *			will put the driver in the correct state (as if the 
 *			command had succeeded))
 *	check for pio errors, bugout if any.
 *
 * RETURNS: 
 *	0	- Success
 *	ENETUNREACH - Failed due to being in LIMBO State
 *	ENETDOWN - Failed due to being in DEAD State
 *	EINVAL	- The command was invalid
 *	EOPNOTSUPP - The command was not supported.
 */
int
fddi_ctl(
	fddi_acs_t 	*p_acs,
	int		cmd, 
	caddr_t		arg, 
	int		len)
{
	int 		ipri;
	int		rc = 0;
	fddi_cmd_t	p_cmd;

	ipri = disable_lock (CFDDI_OPLEVEL, &p_acs->dev.cmd_lock);
	FDDI_TRACE("IfcB",p_acs,cmd,arg);

	p_cmd.cmd_code = 0;

	switch (cmd)
	{

		case NDD_GET_STATS:
			if (p_acs->dev.state == LIMBO_STATE ||
				p_acs->dev.state == LIMBO_RECOVERY_STATE)
			{
				FDDI_ETRACE("Ifc1",p_acs,cmd,p_acs->dev.state);
				unlock_enable(ipri, &p_acs->dev.cmd_lock);
				return (ENETUNREACH);
			}
		
			if (p_acs->dev.state == DEAD_STATE)
			{
				FDDI_ETRACE("Ifc2",p_acs,cmd,p_acs->dev.state);
				unlock_enable(ipri, &p_acs->dev.cmd_lock);
				return (ENETDOWN);
			}
			rc = get_stats(p_acs, arg, len, &p_cmd);
			break;

		case NDD_ENABLE_MULTICAST:
			if (p_acs->dev.state == LIMBO_STATE ||
				p_acs->dev.state == LIMBO_RECOVERY_STATE)
			{
				FDDI_ETRACE("Ifc3",p_acs,cmd,p_acs->dev.state);
				unlock_enable(ipri, &p_acs->dev.cmd_lock);
				return (ENETUNREACH);
			}
		
			if (p_acs->dev.state == DEAD_STATE)
			{
				FDDI_ETRACE("Ifc4",p_acs,cmd,p_acs->dev.state);
				unlock_enable(ipri, &p_acs->dev.cmd_lock);
				return (ENETDOWN);
			}
			enable_multicast(p_acs,&p_cmd);
			break;

		case NDD_DISABLE_MULTICAST:
			rc = disable_multicast(p_acs,&p_cmd);
			break;
		
		case NDD_PROMISCUOUS_ON:
			if (p_acs->dev.state == LIMBO_STATE ||
				p_acs->dev.state == LIMBO_RECOVERY_STATE)
			{
				FDDI_ETRACE("Ifc5",p_acs,cmd,p_acs->dev.state);
				unlock_enable(ipri, &p_acs->dev.cmd_lock);
				return (ENETUNREACH);
			}
		
			if (p_acs->dev.state == DEAD_STATE)
			{
				FDDI_ETRACE("Ifc6",p_acs,cmd,p_acs->dev.state);
				unlock_enable(ipri, &p_acs->dev.cmd_lock);
				return (ENETDOWN);
			}
			prom_on(p_acs,&p_cmd);
			break;
		
		case NDD_PROMISCUOUS_OFF:
			rc = prom_off(p_acs,&p_cmd);
			break;
		
		case NDD_ADD_FILTER:
			if (p_acs->dev.state == LIMBO_STATE ||
				p_acs->dev.state == LIMBO_RECOVERY_STATE)
			{
				FDDI_ETRACE("Ifc7",p_acs,cmd,p_acs->dev.state);
				unlock_enable(ipri, &p_acs->dev.cmd_lock);
				return (ENETUNREACH);
			}
		
			if (p_acs->dev.state == DEAD_STATE)
			{
				FDDI_ETRACE("Ifc8",p_acs,cmd,p_acs->dev.state);
				unlock_enable(ipri, &p_acs->dev.cmd_lock);
				return (ENETDOWN);
			}
			rc = add_filter(p_acs,arg,len,&p_cmd);
			break;
		
		case NDD_DEL_FILTER:
			rc = del_filter(p_acs,arg,len,&p_cmd);
			break;
		
		case NDD_MIB_QUERY:
			if (p_acs->dev.state == LIMBO_STATE ||
				p_acs->dev.state == LIMBO_RECOVERY_STATE)
			{
				FDDI_ETRACE("Ifc9",p_acs,cmd,p_acs->dev.state);
				unlock_enable(ipri, &p_acs->dev.cmd_lock);
				return (ENETUNREACH);
			}
		
			if (p_acs->dev.state == DEAD_STATE)
			{
				FDDI_ETRACE("Ifca",p_acs,cmd,p_acs->dev.state);
				unlock_enable(ipri, &p_acs->dev.cmd_lock);
				return (ENETDOWN);
			}
			rc = mib_query(p_acs,arg, len);
			break;
		
		case NDD_MIB_GET:
			if (p_acs->dev.state == LIMBO_STATE ||
				p_acs->dev.state == LIMBO_RECOVERY_STATE)
			{
				FDDI_ETRACE("Ifcb",p_acs,cmd,p_acs->dev.state);
				unlock_enable(ipri, &p_acs->dev.cmd_lock);
				return (ENETUNREACH);
			}
		
			if (p_acs->dev.state == DEAD_STATE)
			{
				FDDI_ETRACE("Ifcc",p_acs,cmd,p_acs->dev.state);
				unlock_enable(ipri, &p_acs->dev.cmd_lock);
				return (ENETDOWN);
			}
			rc = mib_get(p_acs,arg,len);
			break;
		
		case NDD_ADD_STATUS:
			if (p_acs->dev.state == LIMBO_STATE ||
				p_acs->dev.state == LIMBO_RECOVERY_STATE)
			{
				FDDI_ETRACE("Ifcd",p_acs,cmd,p_acs->dev.state);
				unlock_enable(ipri, &p_acs->dev.cmd_lock);
				return (ENETUNREACH);
			}
		
			if (p_acs->dev.state == DEAD_STATE)
			{
				FDDI_ETRACE("Ifce",p_acs,cmd,p_acs->dev.state);
				unlock_enable(ipri, &p_acs->dev.cmd_lock);
				return (ENETDOWN);
			}
			rc = add_status(p_acs,arg,len, &p_cmd);
			break;
		
		case NDD_DEL_STATUS:
			rc = del_status(p_acs,arg,len,&p_cmd);
			break;
		
		case NDD_DUMP_ADDR:
			*(int *)arg = fddi_dump;
			break;
		
		case NDD_ENABLE_ADDRESS:
			if (p_acs->dev.state == LIMBO_STATE ||
				p_acs->dev.state == LIMBO_RECOVERY_STATE)
			{
				FDDI_ETRACE("Ifcf",p_acs,cmd,p_acs->dev.state);
				unlock_enable(ipri, &p_acs->dev.cmd_lock);
				return (ENETUNREACH);
			}
		
			if (p_acs->dev.state == DEAD_STATE)
			{
				FDDI_ETRACE("Ifcg",p_acs,cmd,p_acs->dev.state);
				unlock_enable(ipri, &p_acs->dev.cmd_lock);
				return (ENETDOWN);
			}
			rc = enable_addr(p_acs,arg,len,&p_cmd);
			if (p_acs->addrs.hdw_addr_cnt > 0)
				p_acs->ndd.ndd_flags |= NDD_ALTADDRS;
			break;
		
		case NDD_DISABLE_ADDRESS:
			rc = disable_addr(p_acs,arg,len,&p_cmd);
			if (p_acs->addrs.hdw_addr_cnt == 0)
				p_acs->ndd.ndd_flags &= ~(NDD_ALTADDRS);
			break;
		
		case NDD_MIB_ADDR:
			if (p_acs->dev.state == LIMBO_STATE ||
				p_acs->dev.state == LIMBO_RECOVERY_STATE)
			{
				FDDI_ETRACE("Ifch",p_acs,cmd,p_acs->dev.state);
				unlock_enable(ipri, &p_acs->dev.cmd_lock);
				return (ENETUNREACH);
			}
		
			if (p_acs->dev.state == DEAD_STATE)
			{
				FDDI_ETRACE("Ifci",p_acs,cmd,p_acs->dev.state);
				unlock_enable(ipri, &p_acs->dev.cmd_lock);
				return (ENETDOWN);
			}
			rc = mib_addr(p_acs,arg,len,&p_cmd);
			break;
		
		case NDD_CLEAR_STATS:
			if (p_acs->dev.state == LIMBO_STATE ||
				p_acs->dev.state == LIMBO_RECOVERY_STATE)
			{
				FDDI_ETRACE("Ifcj",p_acs,cmd,p_acs->dev.state);
				unlock_enable(ipri, &p_acs->dev.cmd_lock);
				return (ENETUNREACH);
			}
		
			if (p_acs->dev.state == DEAD_STATE)
			{
				FDDI_ETRACE("Ifck",p_acs,cmd,p_acs->dev.state);
				unlock_enable(ipri, &p_acs->dev.cmd_lock);
				return (ENETDOWN);
			}
			clear_stats(p_acs);
			break;
		
		case CFDDI_GET_TRACE:
			if (p_acs->dev.state == LIMBO_STATE ||
				p_acs->dev.state == LIMBO_RECOVERY_STATE)
			{
				FDDI_ETRACE("Ifcl",p_acs,cmd,p_acs->dev.state);
				unlock_enable(ipri, &p_acs->dev.cmd_lock);
				return (ENETUNREACH);
			}
		
			if (p_acs->dev.state == DEAD_STATE)
			{
				FDDI_ETRACE("Ifcm",p_acs,cmd,p_acs->dev.state);
				unlock_enable(ipri, &p_acs->dev.cmd_lock);
				return (ENETDOWN);
			}
			unlock_enable(ipri, &p_acs->dev.cmd_lock);

			rc = fddi_gtrace(p_acs, arg);

			ipri = disable_lock(CFDDI_OPLEVEL, 
				&p_acs->dev.cmd_lock);
			break;

		default :
			FDDI_ETRACE("Ifcn",p_acs,cmd,EOPNOTSUPP);
			unlock_enable(ipri, &p_acs->dev.cmd_lock);
			return (EOPNOTSUPP);
	};

	if (rc != 0)
	{
		FDDI_ETRACE("Ifco",p_acs,cmd,rc);
		unlock_enable(ipri, &p_acs->dev.cmd_lock);
		return(rc);
	}
	if (p_acs->dev.state == LIMBO_RECOVERY_STATE)
	{
		FDDI_TRACE("IfcD",p_acs, cmd, p_acs->dev.state);
		unlock_enable(ipri, &p_acs->dev.cmd_lock);
		enter_limbo(p_acs, NDD_CMD_FAIL,0,FALSE);
		return(0);
	}

	if (p_acs->dev.state == LIMBO_STATE)
	{
		FDDI_TRACE("IfcD",p_acs,cmd,p_acs->dev.state);
		unlock_enable(ipri, &p_acs->dev.cmd_lock);
		return (0);
	}
		
	if (p_acs->dev.state == DEAD_STATE)
	{
		FDDI_TRACE("IfcD",p_acs,cmd,p_acs->dev.state);
		unlock_enable(ipri, &p_acs->dev.cmd_lock);
		return (0);
	}

	if (p_cmd.cmd_code != 0)
	{
		send_cmd(p_acs, &p_cmd);
	
		if (p_acs->dev.pio_rc)
		{
			FDDI_ETRACE("Ifcp",p_acs,cmd,p_cmd.cmd_code);
			unlock_enable(ipri, &p_acs->dev.cmd_lock);
			fddi_logerr(p_acs, ERRID_CFDDI_PIO, __LINE__, __FILE__,
				0, 0, 0);
			bugout(p_acs,NDD_PIO_FAIL, 0, 0, FALSE);
			return (ENETDOWN);
		}
	
		if (p_acs->dev.cmd_status != 0)
		{
			FDDI_ETRACE("Ifcq",p_acs,p_cmd.cmd_code,
				p_acs->dev.cmd_status);
			unlock_enable(ipri, &p_acs->dev.cmd_lock);
			return(0);
		}
	}
	if (cmd == NDD_GET_STATS)
	{
		bcopy (&p_acs->dev.ls_buf, arg, sizeof(fddi_ndd_stats_t));
	}

	FDDI_TRACE("IfcE",p_acs,cmd,arg);
	unlock_enable(ipri, &p_acs->dev.cmd_lock);
	return(0);
}

/*
 * NAME: get_stats
 *
 * FUNCTION: get the fddi statistics for the user
 *
 * EXECUTION ENVIRONMENT: process thread
 *
 * NOTES: 
 *	Check the size of the area, fail if insufficent size
 *
 * RETURNS: 
 *	0 	- Success
 *	EINVAL  - Insufficient size for the statistics
 */
int
get_stats (
	fddi_acs_t 	*p_acs,
	fddi_ndd_stats_t *stats,
	int		len,
	fddi_cmd_t 	*p_cmd)
{
	int	rc;

	FDDI_TRACE("IgsB",p_acs,len,0);

	if (len != sizeof(fddi_ndd_stats_t))
	{
		FDDI_ETRACE("Igs1",p_acs,len,0);
		return(EINVAL);
	}

	p_cmd->stat = 0;
	p_cmd->pri = 0;
	p_cmd->cmplt = (int(*)()) hcr_stat_cmplt;
	p_cmd->cmd_code = FDDI_HCR_ULS;
	p_cmd->cpb_len = 0;

	FDDI_TRACE("IgsE",p_acs,len,0);
	return(0);	
}

/*
 * NAME: enable_multicast
 *
 * FUNCTION: handle entering multicast mode
 *
 * EXECUTION ENVIRONMENT: process thread
 *
 * NOTES: 
 *	increment the multicast counter
 *	if this is the first call set up multicast mode and send the command to
 *			the adapter.
 *
 * RETURNS: 
 */
void
enable_multicast (
	fddi_acs_t 	*p_acs,
	fddi_cmd_t 	*p_cmd)
{

	FDDI_TRACE("IemB",p_acs,p_acs->dev.multi_cnt,0);
	p_acs->dev.multi_cnt++;
	if (p_acs->dev.multi_cnt == 1)
	{
		p_acs->dev.smt_control |= FDDI_SMT_CTL_MULTI;
		p_acs->ndd.ndd_flags |= NDD_MULTICAST;

		p_cmd->stat = 0;
		p_cmd->pri = 0;
		p_cmd->cmplt = (int(*)()) hcr_smt_cmplt;
		p_cmd->cmd_code = FDDI_HCR_WR_SMT_CTL;
		p_cmd->cpb[2] = SWAPSHORT(p_acs->dev.smt_control);
		p_cmd->cpb_len = 6;
	}
	FDDI_TRACE("IemE",p_acs,p_acs->dev.multi_cnt,0);
	return;
}

/*
 * NAME: disable_multicast
 *
 * FUNCTION: handle exiting multicast mode
 *
 * EXECUTION ENVIRONMENT: process thread
 *
 * NOTES: 
 *	decrement the multicast counter
 *	if this is the last call clear multicast mode and send the command to
 *			the adapter.
 *
 * RETURNS: 
 *	0 	- Success
 *	EINVAL 	- Multicast was not enabled
 */
int
disable_multicast (
	fddi_acs_t 	*p_acs,
	fddi_cmd_t 	*p_cmd)
{

	FDDI_TRACE("IdmB",p_acs,p_acs->dev.multi_cnt,0);

	if (p_acs->dev.multi_cnt == 0)
 	{
		FDDI_ETRACE("Idm1",p_acs,p_acs->dev.multi_cnt,0);
		return(EINVAL);
	}

	p_acs->dev.multi_cnt--;
	if (p_acs->dev.multi_cnt == 0)
	{
		p_acs->dev.smt_control &= ~(FDDI_SMT_CTL_MULTI);
		p_acs->ndd.ndd_flags &= ~(NDD_MULTICAST);

		p_cmd->stat = 0;
		p_cmd->pri = 0;
		p_cmd->cmplt = (int(*)()) hcr_smt_cmplt;
		p_cmd->cmd_code = FDDI_HCR_WR_SMT_CTL;
		p_cmd->cpb[2] = SWAPSHORT(p_acs->dev.smt_control);
		p_cmd->cpb_len = 6;
	}
	FDDI_TRACE("IdmE",p_acs,p_acs->dev.multi_cnt,0);
	return(0);
}

/*
 * NAME: prom_on
 *
 * FUNCTION: handle entering promiscuous mode
 *
 * EXECUTION ENVIRONMENT: process thread
 *
 * NOTES: 
 *	increment the promiscuous counter
 *	if this is the first call set up promiscuous mode and send the command 
 * 			to the adapter.
 *
 * RETURNS: 
 */
void
prom_on (
	fddi_acs_t 	*p_acs,
	fddi_cmd_t 	*p_cmd)
{

	FDDI_TRACE("IpoB",p_acs,p_acs->dev.prom_cnt,0);
	p_acs->dev.prom_cnt++;
	if (p_acs->dev.prom_cnt == 1)
	{
		p_acs->dev.smt_control |= FDDI_SMT_CTL_PROM;
		p_acs->ndd.ndd_flags |= NDD_PROMISC;

		p_cmd->stat = 0;
		p_cmd->pri = 0;
		p_cmd->cmplt = (int(*)()) hcr_smt_cmplt;
		p_cmd->cmd_code = FDDI_HCR_WR_SMT_CTL;
		p_cmd->cpb[2] = SWAPSHORT(p_acs->dev.smt_control);
		p_cmd->cpb_len = 6;
	}
	FDDI_TRACE("IpoE",p_acs,p_acs->dev.prom_cnt,0);
	return;
}

/*
 * NAME: prom_off
 *
 * FUNCTION: handle exiting promiscuous mode
 *
 * EXECUTION ENVIRONMENT: process thread
 *
 * NOTES: 
 *	decrement the promiscuous counter
 *	if this is the last call clear promiscuous mode and send the command to
 *			the adapter.
 *
 * RETURNS: 
 *	0	- Success
 *	EINVAL	- Promiscous has not been enabled
 */
int
prom_off (
	fddi_acs_t 	*p_acs,
	fddi_cmd_t 	*p_cmd)
{

	FDDI_TRACE("IpfB",p_acs,p_acs->dev.prom_cnt,0);

	if (p_acs->dev.prom_cnt == 0)
 	{
		FDDI_ETRACE("Ipf1",p_acs,p_acs->dev.prom_cnt,0);
		return(EINVAL);
	}

	p_acs->dev.prom_cnt--;
	if (p_acs->dev.prom_cnt == 0)
	{
		p_acs->dev.smt_control &= ~(FDDI_SMT_CTL_PROM);
		p_acs->ndd.ndd_flags &= ~(NDD_PROMISC);

		p_cmd->stat = 0;
		p_cmd->pri = 0;
		p_cmd->cmplt = (int(*)()) hcr_smt_cmplt;
		p_cmd->cmd_code = FDDI_HCR_WR_SMT_CTL;
		p_cmd->cpb[2] = SWAPSHORT(p_acs->dev.smt_control);
		p_cmd->cpb_len = 6;
	}
	FDDI_TRACE("IpfE",p_acs,p_acs->dev.prom_cnt,0);
	return(0);
}

/*
 * NAME: add_filter
 *
 * FUNCTION: handle setting the adapter up for the smt,nsa, and beacon filters
 *
 * EXECUTION ENVIRONMENT: process thread
 *
 * NOTES: 
 *	increment the appropriate filter counter
 *	if this is the first call set up a particular filter then set up the 
 *			command and send it to the adapter.
 *
 *	NOTE : other filters are ignored (no device driver action).
 *
 * RETURNS: 
 */
int
add_filter (
	fddi_acs_t 	*p_acs,
	fddi_dmx_filter_t	*p_filter,
	int		len,
	fddi_cmd_t 	*p_cmd)
{
	FDDI_TRACE("IafB",p_acs,p_filter, len);

	if (len < sizeof(ns_8022_t))
	{
		FDDI_ETRACE("Iaf1",p_acs, p_filter, len);
		return(EINVAL);
	}

	if ((p_filter->filter.filtertype == FDDI_DEMUX_MAC ||
		p_filter->filter.filtertype == FDDI_DEMUX_SMT ||
		p_filter->filter.filtertype == FDDI_DEMUX_SMT_NSA) &&
		(len < sizeof(fddi_dmx_filter_t)))
	{
		FDDI_ETRACE("Iaf2",p_acs, p_filter, len);
		return(EINVAL);
	}

	switch (p_filter->filter.filtertype)
	{
	case FDDI_DEMUX_MAC:
	{
		p_acs->dev.bea_cnt++;
		if (p_acs->dev.bea_cnt == 1)
		{
			p_acs->dev.smt_control |= FDDI_SMT_CTL_BEA;
			p_acs->ndd.ndd_flags |= CFDDI_NDD_BEACON;
	
			p_cmd->stat = 0;
			p_cmd->pri = 0;
			p_cmd->cmplt = (int(*)()) hcr_smt_cmplt;
			p_cmd->cmd_code = FDDI_HCR_WR_SMT_CTL;
			p_cmd->cpb[2] = SWAPSHORT(p_acs->dev.smt_control);
			p_cmd->cpb_len = 6;
		}
		break;
	}
	case FDDI_DEMUX_SMT:
	{
		p_acs->dev.smt_cnt++;
		if (p_acs->dev.smt_cnt == 1)
		{
			p_acs->dev.smt_control |= FDDI_SMT_CTL_SMT;
			p_acs->ndd.ndd_flags |= CFDDI_NDD_SMT;
	
			p_cmd->stat = 0;
			p_cmd->pri = 0;
			p_cmd->cmplt = (int(*)()) hcr_smt_cmplt;
			p_cmd->cmd_code = FDDI_HCR_WR_SMT_CTL;
			p_cmd->cpb[2] = SWAPSHORT(p_acs->dev.smt_control);
			p_cmd->cpb_len = 6;
		}
		break;
	}
	case FDDI_DEMUX_SMT_NSA:
	{
		p_acs->dev.nsa_cnt++;
		if (p_acs->dev.nsa_cnt == 1)
		{
			p_acs->dev.smt_control |= FDDI_SMT_CTL_NSA;
			p_acs->ndd.ndd_flags |= CFDDI_NDD_NSA;
	
			p_cmd->stat = 0;
			p_cmd->pri = 0;
			p_cmd->cmplt = (int(*)()) hcr_smt_cmplt;
			p_cmd->cmd_code = FDDI_HCR_WR_SMT_CTL;
			p_cmd->cpb[2] = SWAPSHORT(p_acs->dev.smt_control);
			p_cmd->cpb_len = 6;
		}
		break;
	}
	}
	FDDI_TRACE("IafE",p_acs->dev.bea_cnt,p_acs->dev.smt_cnt,
			p_acs->dev.nsa_cnt);
	return(0);
}

/*
 * NAME: del_filter
 *
 * FUNCTION: handle clearing the smt,nsa, and beacon filters
 *
 * EXECUTION ENVIRONMENT: process thread
 *
 * NOTES: 
 *	decrement the filter's counter
 *	if this is the last call, clear the filter's mode and send the command 
 *			to the adapter.
 *
 *	NOTE : other filters are ignored (no device driver action).
 *
 * RETURNS: 
 *	0 	 - Success
 *	EINVAL	 - The filter had not been added
 */
int
del_filter (
	fddi_acs_t 	*p_acs,
	fddi_dmx_filter_t	*p_filter,
	int		len,
	fddi_cmd_t 	*p_cmd)
{

	FDDI_TRACE("IdfB",p_acs,p_filter, len);

	if (len < sizeof(ns_8022_t))
	{
		FDDI_ETRACE("Idf1",p_acs, p_filter, len);
		return(EINVAL);
	}

	if ((p_filter->filter.filtertype == FDDI_DEMUX_MAC ||
		p_filter->filter.filtertype == FDDI_DEMUX_SMT ||
		p_filter->filter.filtertype == FDDI_DEMUX_SMT_NSA) &&
		(len < sizeof(fddi_dmx_filter_t)))
	{
		FDDI_ETRACE("Idf2",p_acs, p_filter, len);
		return(EINVAL);
	}

	switch (p_filter->filter.filtertype)
	{
	case FDDI_DEMUX_MAC:
	{
		if (p_acs->dev.bea_cnt == 0)
 		{
			FDDI_ETRACE("Idf3",p_acs,p_acs->dev.bea_cnt,0);
			return(EINVAL);
		}

		p_acs->dev.bea_cnt--;
		if (p_acs->dev.bea_cnt == 0)
		{
			p_acs->dev.smt_control &= ~(FDDI_SMT_CTL_BEA);
			p_acs->ndd.ndd_flags &= ~(CFDDI_NDD_BEACON);
	
			p_cmd->stat = 0;
			p_cmd->pri = 0;
			p_cmd->cmplt = (int(*)()) hcr_smt_cmplt;
			p_cmd->cmd_code = FDDI_HCR_WR_SMT_CTL;
			p_cmd->cpb[2] = SWAPSHORT(p_acs->dev.smt_control);
			p_cmd->cpb_len = 6;
		}
		break;
	}
	case FDDI_DEMUX_SMT:
	{
		if (p_acs->dev.smt_cnt == 0)
 		{
			FDDI_ETRACE("Idf4",p_acs,p_acs->dev.smt_cnt,0);
			return(EINVAL);
		}

		p_acs->dev.smt_cnt--;
		if (p_acs->dev.smt_cnt == 0)
		{
			p_acs->dev.smt_control &= ~(FDDI_SMT_CTL_SMT);
			p_acs->ndd.ndd_flags &= ~(CFDDI_NDD_SMT);
	
			p_cmd->stat = 0;
			p_cmd->pri = 0;
			p_cmd->cmplt = (int(*)()) hcr_smt_cmplt;
			p_cmd->cmd_code = FDDI_HCR_WR_SMT_CTL;
			p_cmd->cpb[2] = SWAPSHORT(p_acs->dev.smt_control);
			p_cmd->cpb_len = 6;
		}
		break;
	}
	case FDDI_DEMUX_SMT_NSA:
	{
		if (p_acs->dev.nsa_cnt == 0)
 		{
			FDDI_ETRACE("Idf5",p_acs,p_acs->dev.nsa_cnt,0);
			return(EINVAL);
		}

		p_acs->dev.nsa_cnt--;
		if (p_acs->dev.nsa_cnt == 0)
		{
			p_acs->dev.smt_control &= ~(FDDI_SMT_CTL_NSA);
			p_acs->ndd.ndd_flags &= ~(CFDDI_NDD_NSA);
	
			p_cmd->stat = 0;
			p_cmd->pri = 0;
			p_cmd->cmplt = (int(*)()) hcr_smt_cmplt;
			p_cmd->cmd_code = FDDI_HCR_WR_SMT_CTL;
			p_cmd->cpb[2] = SWAPSHORT(p_acs->dev.smt_control);
			p_cmd->cpb_len = 6;
		}
		break;
	}
	}
	FDDI_TRACE("IdfE",p_acs->dev.bea_cnt,p_acs->dev.smt_cnt,
			p_acs->dev.nsa_cnt);
	return(0);
}

/*
 * NAME: mib_query
 *
 * FUNCTION: returns the mib structure identifying which options the driver is
 *		supporting
 *
 * EXECUTION ENVIRONMENT: process thread
 *
 * NOTES: 
 *	
 * RETURNS: 
 */
int
mib_query (
	fddi_acs_t 	*p_acs,
	generic_mib_t	*mib,
	int		len)
{
	FDDI_TRACE("ImqB",p_acs,mib,len);

	if (len < sizeof(generic_mib_t))
	{
		FDDI_ETRACE("Imq1",p_acs,mib,len);
		return(EINVAL);
	}
	
	mib->ifExtnsEntry.chipset[0] = MIB_READ_ONLY;
	mib->ifExtnsEntry.revware[0] = MIB_NOT_SUPPORTED;
	mib->ifExtnsEntry.mcast_tx_ok = MIB_READ_ONLY;
	mib->ifExtnsEntry.bcast_tx_ok = MIB_READ_ONLY;
	mib->ifExtnsEntry.mcast_rx_ok = MIB_READ_ONLY;
	mib->ifExtnsEntry.bcast_rx_ok = MIB_READ_ONLY;
	mib->ifExtnsEntry.promiscuous = MIB_READ_ONLY;
	mib->ifExtnsTestEntry.community = MIB_NOT_SUPPORTED;
	mib->ifExtnsTestEntry.request_id = MIB_NOT_SUPPORTED;
	mib->ifExtnsTestEntry.type = MIB_NOT_SUPPORTED;
	mib->ifExtnsTestEntry.result = MIB_NOT_SUPPORTED;
	mib->ifExtnsTestEntry.code = MIB_NOT_SUPPORTED;
	mib->RcvAddrTable = MIB_READ_ONLY;

	FDDI_TRACE("ImqE",p_acs,0,0);
	return(0);
}

/*
 * NAME: mib_get
 *
 * FUNCTION: Returns the filled out mib structure
 *
 * EXECUTION ENVIRONMENT: process thread
 *
 * NOTES: 
 *
 * RETURNS: 
 */
int
mib_get (
	fddi_acs_t 	*p_acs,
	generic_mib_t	*mib,
	int		len)
{
	FDDI_TRACE("ImgB",p_acs,mib,len);

	if (len < sizeof(generic_mib_t))
	{
		FDDI_ETRACE("Img1",p_acs, mib, len);
		return(EINVAL);
	}

	bcopy(UNKNOWNCHIPSET,mib->ifExtnsEntry.chipset,CHIPSETLENGTH);
	mib->ifExtnsEntry.revware[0] = 0;
	mib->ifExtnsEntry.mcast_tx_ok = p_acs->ls.mcast_tx_ok;
	mib->ifExtnsEntry.bcast_tx_ok = p_acs->ls.bcast_tx_ok;
	mib->ifExtnsEntry.mcast_rx_ok = p_acs->ls.mcast_rx_ok;
	mib->ifExtnsEntry.bcast_rx_ok = p_acs->ls.bcast_rx_ok;
	if (p_acs->dev.smt_control & FDDI_SMT_CTL_PROM)
		mib->ifExtnsEntry.promiscuous = PROMTRUE;
	else
		mib->ifExtnsEntry.promiscuous = PROMFALSE;
	mib->ifExtnsTestEntry.community = 0;
	mib->ifExtnsTestEntry.request_id = 0;
	mib->ifExtnsTestEntry.type = 0;
	mib->ifExtnsTestEntry.result = 0;
	mib->ifExtnsTestEntry.code = 0;
	mib->RcvAddrTable = p_acs->addrs.hdw_addr_cnt +
		p_acs->addrs.sw_addr_cnt + 2;

	FDDI_TRACE("ImgE",p_acs,0,0);
	return(0);
}

/*
 * NAME: add_status
 *
 * FUNCTION: handles the bad frame status filter
 *
 * EXECUTION ENVIRONMENT: process thread
 *
 * NOTES: 
 * 	if bad frames are being added
 *		increment the bad frame counter
 *		if this is the first call set up bad frame mode and send the 
 *			command to the adapter.
 *
 *	NOTE : other status filters are ignored (no device driver action).
 *
 * RETURNS: 
 */
int
add_status (
	fddi_acs_t 	*p_acs,
	ns_com_status_t	*p_status,
	int		len,
	fddi_cmd_t 	*p_cmd)
{
	FDDI_TRACE("IasB",p_acs,p_acs->dev.bf_cnt,0);

	if (len < sizeof(ns_com_status_t))
	{
		FDDI_ETRACE("Ias1",p_acs, p_status, len);
		return(EINVAL);
	}
	
	if (p_status->mask & NDD_BAD_PKTS)
	{
		p_acs->dev.bf_cnt++;
		if (p_acs->dev.bf_cnt == 1)
		{
			p_acs->dev.smt_control |= FDDI_SMT_CTL_CRC;
			p_acs->ndd.ndd_flags |= CFDDI_NDD_BF;
	
			p_cmd->stat = 0;
			p_cmd->pri = 0;
			p_cmd->cmplt = (int(*)()) hcr_smt_cmplt;
			p_cmd->cmd_code = FDDI_HCR_WR_SMT_CTL;
			p_cmd->cpb[2] = SWAPSHORT(p_acs->dev.smt_control);
			p_cmd->cpb_len = 6;
		}
	}
	FDDI_TRACE("IasE",p_acs,p_acs->dev.bf_cnt,0);
	return(0);
}

/*
 * NAME: del_status
 *
 * FUNCTION: handles the delete of a bad frame status filter
 *
 * EXECUTION ENVIRONMENT: process thread
 *
 * NOTES: 
 * 	if bad frames are being deleted
 *		decrement the bad frame counter
 *		if this is the last call to clear bad frame mode, send the 
 *			command to the adapter.
 *
 *	NOTE : other status filters are ignored (no device driver action).
 *
 * RETURNS: 
 *	0 	 - Success
 *	EINVAL	 - The status had not been added
 */
int
del_status (
	fddi_acs_t 	*p_acs,
	ns_com_status_t	*p_status,
	int		len,
	fddi_cmd_t 	*p_cmd)
{

	FDDI_TRACE("IdsB",p_acs,p_acs->dev.bf_cnt,0);

	if (len < sizeof(ns_com_status_t))
	{
		FDDI_ETRACE("Ids1",p_acs, p_status, len);
		return(EINVAL);
	}
	if (p_status->mask & NDD_BAD_PKTS)
	{
		if (p_acs->dev.bf_cnt == 0)
 		{
			FDDI_ETRACE("Ids2",p_acs,p_acs->dev.bf_cnt,0);
			return(EINVAL);
		}

		p_acs->dev.bf_cnt--;
		if (p_acs->dev.bf_cnt == 0)
		{
			p_acs->dev.smt_control &= ~(FDDI_SMT_CTL_CRC);
			p_acs->ndd.ndd_flags &= ~(CFDDI_NDD_BF);
	
			p_cmd->stat = 0;
			p_cmd->pri = 0;
			p_cmd->cmplt = (int(*)()) hcr_smt_cmplt;
			p_cmd->cmd_code = FDDI_HCR_WR_SMT_CTL;
			p_cmd->cpb[2] = SWAPSHORT(p_acs->dev.smt_control);
			p_cmd->cpb_len = 6;
		}
	}
	FDDI_TRACE("IdsE",p_acs,p_acs->dev.bf_cnt,0);
	return(0);
}

/*
 * NAME: enable_addr
 *
 * FUNCTION: handles enabling a group address
 *
 * EXECUTION ENVIRONMENT: process thread
 *
 * NOTES: 
 *	Check for the address on the hardware list
 *		If found, Increment the reference count and return
 *	Check for room to add it to the hardware list (if there is room then
 *				the software list is empty)
 *		If found, Add it to the hardware list, set up command,and return
 *	Check for the address on the software list
 *		If found, Increment the reference count and return
 *	If the software list is empty
 *		Netmalloc extra blk
 *		Add addr to sw list
 *		Call enable_multicast
 *		return
 *	Add to the software list
 *		if no room netmalloc another extra blk and add it
 *	Return
 *
 * RETURNS: 
 *	0 	- Success
 *	EINVAL	- an illegal (not a group address) was sent by the user
 *	ENOMEM 	- not enough memory to support additional addresses.
 */
int
enable_addr(
	fddi_acs_t 	*p_acs,
	fddi_addr_t 	addr,
	int		len,
	fddi_cmd_t 	*p_cmd)
{
	int 	i,j;
	caddr_t p_tmp_buf;
	caddr_t p_addr;
	fddi_addr_blk_t *p_addr_blk;

	FDDI_TRACE("IeaB",p_acs,*(ulong *)&addr[0], *(ushort *)&addr[4]);
	FDDI_TRACE("IeaC",p_acs->addrs.hdw_addr_cnt,p_acs->addrs.sw_addr_cnt,0);

	if (len < CFDDI_NADR_LENGTH)
	{
		FDDI_ETRACE("Iea1",p_acs, addr, len);
		return (EINVAL);
	}

	if (!(addr[0] & FDDI_GRP_ADDR))
	{
		FDDI_ETRACE("Iea2",p_acs,
			*(ulong *)&addr[0], *(ushort *)&addr[4] );
		return(EINVAL);
	}

	/* Check for address already set on hardware list */
	for (i=0; i<FDDI_MAX_HDW_ADDRS; i++)
	{
		if (p_acs->addrs.hdw_addrs[i].addr_cnt > 0)
			if (!bcmp(addr,p_acs->addrs.hdw_addrs[i].addr,
				CFDDI_NADR_LENGTH))
			{
				p_acs->addrs.hdw_addrs[i].addr_cnt++;
				FDDI_TRACE("IeaE",p_acs->addrs.hdw_addr_cnt, 
					p_acs->addrs.sw_addr_cnt,
					p_cmd->cmd_code);
				return(0);
			}
	}

	/* If there is room on hardware list then add it (extra list is empty)*/
	if (p_acs->addrs.hdw_addr_cnt < FDDI_MAX_HDW_ADDRS)
	{
		for (i=0; i<FDDI_MAX_HDW_ADDRS; i++)
		{
			if (p_acs->addrs.hdw_addrs[i].addr_cnt == 0)
			{
				bcopy(addr, p_acs->addrs.hdw_addrs[i].addr,
					CFDDI_NADR_LENGTH);
				p_acs->addrs.hdw_addrs[i].addr_cnt++;
				p_acs->addrs.hdw_addr_cnt++;

				p_cmd->stat = 0;
				p_cmd->pri = 0;
				p_cmd->cmplt = (int(*)()) hcr_addr_cmplt;
				p_cmd->cmd_code = FDDI_HCR_WRITE_ADDR;
				p_cmd->cpb_len = 14;
				p_cmd->cpb[2] = SWAPSHORT(FDDI_HCR_ALD_SKY);
				p_cmd->cpb[3] = SWAPSHORT (FDDI_HCR_ATD_SKY);

				/*
	 			* Put address in proper format for the
				* adapter
	 			*/
				p_addr = (char *) &(p_cmd->cpb[4]);
				*(p_addr++)=addr[5];
				*(p_addr++)=addr[4];
				*(p_addr++)=addr[3];
				*(p_addr++)=addr[2];
				*(p_addr++)=addr[1];
				*(p_addr++)=addr[0];

				FDDI_TRACE("IeaE",p_acs->addrs.hdw_addr_cnt, 
					p_acs->addrs.sw_addr_cnt,
					p_cmd->cmd_code);
				return(0);
			}
		}
	}

	/* 
	 * Check for the address on the software list 
	 *
	 * For the loop below, i is used to mark the number of addresses checked
	 * and j is used for the actual index of the address in the addr blk.
	 * If i reaches the end of the loop then every address has been compared
	 * and the address has not been previously saved on the list.
	 */

	p_addr_blk = p_acs->addrs.sw_addrs;
	for (i=0, j=0; i<p_acs->addrs.sw_addr_cnt; j++)
	{
		if (j > FDDI_MAX_ADDR_BLK)
		{
			j = 0;
			p_addr_blk = p_addr_blk->next;
			FDDI_ASSERT(p_addr_blk != 0);
		}

		if (p_addr_blk->addrs[j].addr_cnt > 0)
			if (!bcmp(addr, p_addr_blk->addrs[j].addr,
				CFDDI_NADR_LENGTH))
			{
				p_addr_blk->addrs[j].addr_cnt++;
				
				FDDI_TRACE("IeaE",p_acs->addrs.hdw_addr_cnt, 
					p_acs->addrs.sw_addr_cnt,
					p_cmd->cmd_code);
				return(0);
			}
			else 
				i++;
	}

	/*
	 * If the software list is empty
  	 */
	if (p_acs->addrs.sw_addr_cnt == 0)
	{
		p_acs->addrs.sw_addrs = net_malloc( sizeof(fddi_addr_blk_t),
						M_DEVBUF, M_NOWAIT);
		if (p_acs->addrs.sw_addrs == 0)
		{
			FDDI_ETRACE("Iea3", p_acs, p_acs->addrs.sw_addr_cnt,0);
			return(ENOMEM);
		}

		bzero(p_acs->addrs.sw_addrs,sizeof(fddi_addr_blk_t));

		p_acs->addrs.sw_addr_cnt++;
		
		bcopy(addr, p_acs->addrs.sw_addrs->addrs[0].addr,
			CFDDI_NADR_LENGTH);

		p_acs->addrs.sw_addrs->addrs[0].addr_cnt = 1;
		p_acs->addrs.sw_addrs->blk_cnt = 1;

		enable_multicast(p_acs, p_cmd);
		FDDI_TRACE("IeaE",p_acs->addrs.hdw_addr_cnt, 
			p_acs->addrs.sw_addr_cnt,
			p_cmd->cmd_code);
		return(0);
	}

	/* 
	 * Add the address to the extra list 
	 */
	p_addr_blk = p_acs->addrs.sw_addrs;
	j = 0;
	while (TRUE)
	{
		if (j > FDDI_MAX_ADDR_BLK)
		{
			j = 0;

			/* Add another block (current extra list is full) */
			if (p_addr_blk->next == 0)
			{
				
				p_addr_blk->next = net_malloc(
						sizeof(fddi_addr_blk_t),
						M_DEVBUF, 
						M_NOWAIT);
				if (p_addr_blk->next == 0)
				{
					FDDI_ETRACE("Iea4", p_acs, 
						p_acs->addrs.sw_addr_cnt,0);
					return(ENOMEM);
				}

				bzero(p_addr_blk->next,sizeof(fddi_addr_blk_t));

				p_acs->addrs.sw_addr_cnt++;
			
				bcopy(addr, p_addr_blk->next->addrs[0].addr,
					CFDDI_NADR_LENGTH);
				p_addr_blk->next->addrs[0].addr_cnt = 1;
				p_addr_blk->next->blk_cnt = 1;
				p_addr_blk->next->prev = p_addr_blk;

				break;
			}
			p_addr_blk = p_addr_blk->next;
		}

		if (p_addr_blk->addrs[j].addr_cnt == 0)
		{
			p_addr_blk->addrs[j].addr_cnt++;
			bcopy(addr,p_addr_blk->addrs[j].addr,CFDDI_NADR_LENGTH);
			p_addr_blk->blk_cnt++;
			p_acs->addrs.sw_addr_cnt++;
			break;
		}

		j++;
	}
	
	FDDI_TRACE("IeaE",p_acs->addrs.hdw_addr_cnt, p_acs->addrs.sw_addr_cnt,
		p_cmd->cmd_code);
	return(0);
}

/*
 * NAME: clear_stats
 *
 * FUNCTION: clears the counter statistics, resets the start time
 *
 * EXECUTION ENVIRONMENT: process thread
 *
 * NOTES: 
 *
 * RETURNS: 
 */
void
clear_stats(
	fddi_acs_t *p_acs)
{
	FDDI_TRACE("IcsB",p_acs,p_acs->dev.stime,0);
	p_acs->ls.mcast_tx_ok = 0;
	p_acs->ls.bcast_tx_ok = 0;
	p_acs->ls.mcast_rx_ok = 0;
	p_acs->ls.bcast_rx_ok = 0;
	bzero(&p_acs->ndd.ndd_genstats, sizeof (struct ndd_genstats));
	p_acs->dev.stime = lbolt;
	return;
	FDDI_TRACE("IcsE",p_acs,p_acs->dev.stime,0);
}

/*
 * NAME: disable_address
 *
 * FUNCTION: handle disabling a group address
 *
 * EXECUTION ENVIRONMENT: process thread
 *
 * NOTES: 
 *	Check the address 
 *		If not a group address return error
 *	Check the hardware list
 *		If found
 *			Decrement the reference count
 *			If the reference count is > 0 return
 *			If extra list == 0, set up clear address command and 
 *				return
 *			Move address from extra list to hardware list
 *			If extra list still has addresses return
 *			Call disable_multicast
 *			set up Clear All Addresses command 
 *			Return
 *	Check extra list
 *		If found
 *			Decrement the reference count
 *			If reference count is > 0 return
 *			If extra address blk is empty
 *				free it from the list and return memory
 *			If extra list is empty
 *				Call disable_multicast
 *				set up Clear All Addresses command
 *			Return
 *	Return Error
 *			
 * 	NOTE: under the current design everything in the data structures needs
 *		to be taken care of before we send the command.  This is to 
 *		prevent the driver from being out of sync from the adapter
 *		if we enter limbo before the command sequence finishes.  This
 *		is especially true on multiple commands (such as exiting 
 *		extra address multicast mode).
 *
 * RETURNS: 
 *	0 	 - Success
 *	EINVAL	 - An illegal (not a group address) address was sent by the user
 *			or an address which wasn't enabled
 */
int
disable_addr(
	fddi_acs_t 	*p_acs,
	fddi_addr_t 	addr,
	int		len,
	fddi_cmd_t 	*p_cmd)
{
	int 	i,j, rc;
	caddr_t p_tmp_buf;
	caddr_t p_addr;
	fddi_addr_blk_t *p_addr_blk;

	FDDI_TRACE("IdaB",p_acs,*(ulong *)&addr[0], *(ushort *)&addr[4] );
	FDDI_TRACE("IdaC",p_acs->addrs.hdw_addr_cnt,p_acs->addrs.sw_addr_cnt,0);

	if (len < CFDDI_NADR_LENGTH)
	{
		FDDI_ETRACE("Ida1",p_acs, addr, len);
		return(EINVAL);
	}

	if (!(addr[0] & FDDI_GRP_ADDR))
	{
		FDDI_ETRACE("Ida2",p_acs,
			*(ulong *)&addr[0], *(ushort *)&addr[4] );
		return(EINVAL);
	}

	/* Check the hardware list */
	for (i=0; i<FDDI_MAX_HDW_ADDRS; i++)
	{
		/* If this address is set */
		if (p_acs->addrs.hdw_addrs[i].addr_cnt > 0)
		{
			/* Compare the addresses */
			if (!bcmp(addr,p_acs->addrs.hdw_addrs[i].addr,
				CFDDI_NADR_LENGTH))
			{
				p_acs->addrs.hdw_addrs[i].addr_cnt--;
				
				/* Address is still has requests */
				if (p_acs->addrs.hdw_addrs[i].addr_cnt > 0)
				{
					FDDI_TRACE("IdaE",
						p_acs->addrs.hdw_addr_cnt, 
						p_acs->addrs.sw_addr_cnt,
						p_cmd->cmd_code);
					return(0);
				}
				
				/* 
				 * Just need to clear the address on the adapter
				 */
				if (p_acs->addrs.sw_addr_cnt == 0)
				{
					p_acs->addrs.hdw_addr_cnt--;
					p_cmd->stat = 0;
					p_cmd->pri = 0;
					p_cmd->cmplt = (int(*)()) 
							hcr_addr_cmplt;
					p_cmd->cmd_code = FDDI_HCR_CLEAR_ADDR;
					p_cmd->cpb_len = 14;
					p_cmd->cpb[2] = 
						SWAPSHORT(FDDI_HCR_ALD_SKY);
					p_cmd->cpb[3] = 
						SWAPSHORT (FDDI_HCR_ATD_SKY);

					/*
	 				* Put address in proper format for the
					* adapter
	 				*/
					p_addr = (char *) &(p_cmd->cpb[4]);
					*(p_addr++)=addr[5];
					*(p_addr++)=addr[4];
					*(p_addr++)=addr[3];
					*(p_addr++)=addr[2];
					*(p_addr++)=addr[1];
					*(p_addr++)=addr[0];

					FDDI_TRACE("IdaE",
						p_acs->addrs.hdw_addr_cnt, 
						p_acs->addrs.sw_addr_cnt,
						p_cmd->cmd_code);
					return(0);
				}

				/* 
				 * Move an address from extra list to hardware
				 * list.  We do not need to set the address 
 				 * unless this is the last address on the extra
 				 * list as we are in multicast mode.
				 */
				p_addr_blk = p_acs->addrs.sw_addrs;
				for (j=0; j<FDDI_MAX_ADDR_BLK; j++)
					if (p_addr_blk->addrs[j].addr_cnt > 0)
						break;

				p_acs->addrs.hdw_addrs[i] =p_addr_blk->addrs[j];

				p_addr_blk->addrs[j].addr_cnt = 0;

				p_addr_blk->blk_cnt--;
				p_acs->addrs.sw_addr_cnt--;
					
				/* 
				 * if this was the last address on blk, free it 
			 	 */
				if (p_addr_blk->blk_cnt == 0)
				{

					if (p_addr_blk->prev != 0)
						p_addr_blk->prev->next = 	
							p_addr_blk->next;

					if (p_addr_blk->next != 0)
						p_addr_blk->next->prev =
							p_addr_blk->prev;
					
					if (p_acs->addrs.sw_addrs == p_addr_blk)
					{
						p_acs->addrs.sw_addrs = 
							p_addr_blk->next;
					}

					net_free(p_addr_blk, M_DEVBUF);
					
				}

				/* 
				 * This was the last address on the extra list
				 * so prepare to exit multicast mode
				 */
			
				if (p_acs->addrs.sw_addr_cnt == 0)
				{
					disable_multicast(p_acs, p_cmd);
					p_cmd->stat = 0;
					p_cmd->pri = 0;
					p_cmd->cmplt = (int(*)()) 
							hcr_all_addr_cmplt;
					p_cmd->cmd_code = FDDI_HCR_CLEAR_ADDR;
					p_cmd->cpb_len = 8;
					p_cmd->cpb[2] = 
						SWAPSHORT(FDDI_HCR_ALD_SKY);
					p_cmd->cpb[3] = 
						SWAPSHORT (FDDI_HCR_ATD_ALL);
				}

				FDDI_TRACE("IdaE",
					p_acs->addrs.hdw_addr_cnt, 
					p_acs->addrs.sw_addr_cnt,
					p_cmd->cmd_code);
				return(0);
			}
				
		}
	}


	/*
	 * Check the software list for the address
 	 */
	p_addr_blk = p_acs->addrs.sw_addrs;

	for(i=0, j=0; i<p_acs->addrs.sw_addr_cnt; j++)
	{
		/* 
		 * Advance to the next block if this one is done
		 */
		if (j > FDDI_MAX_ADDR_BLK)
		{
			j=0;
			p_addr_blk = p_addr_blk->next;
		}

		/*
		 * If the address is set (the reference count is above 0) 
		 * increment the loop counter (to guarentee we check all of the
		 * addresses) and compare the address to the one we seek
		 */
		if (p_addr_blk->addrs[j].addr_cnt > 0)
		{
			i++;
			if (!bcmp(addr,p_addr_blk->addrs[j].addr,
					CFDDI_NADR_LENGTH))
			{
				/* 
				 * If it found, decrement the counter and 
				 * see if that was the only count for this 
				 * address.
				 */
				p_addr_blk->addrs[j].addr_cnt--;
				if (p_addr_blk->addrs[j].addr_cnt > 0)
				{
					FDDI_TRACE("IdaE",
						p_acs->addrs.hdw_addr_cnt, 
						p_acs->addrs.sw_addr_cnt,
						p_cmd->cmd_code);
					return(0);
				}

				/* 
				 * Since that address slot is now free, 
				 * decrement the count of extra addresses and
				 * the count for this block
				 */
				p_acs->addrs.sw_addr_cnt--;
				p_addr_blk->blk_cnt--;

				/* 
				 * If the block is empty free it 
				 */
				if (p_addr_blk->blk_cnt == 0)
				{
					if (p_addr_blk->prev != 0)
						p_addr_blk->prev->next = 	
							p_addr_blk->next;

					if (p_addr_blk->next != 0)
						p_addr_blk->next->prev =
							p_addr_blk->prev;
					
					if (p_acs->addrs.sw_addrs == p_addr_blk)
					{
						p_acs->addrs.sw_addrs = 
							p_addr_blk->next;
					}

					net_free(p_addr_blk, M_DEVBUF);
				}

				/*
				 * If this is the last extra address (the last
				 * extra block was just freed), then begin
				 * the process of leaving multicast mode.
				 */
				if (p_acs->addrs.sw_addr_cnt == 0)
				{
					disable_multicast(p_acs, p_cmd);

					p_cmd->stat = 0;
					p_cmd->pri = 0;
					p_cmd->cmplt = (int(*)()) 
							hcr_all_addr_cmplt;
					p_cmd->cmd_code = FDDI_HCR_CLEAR_ADDR;
					p_cmd->cpb_len = 8;
					p_cmd->cpb[2] = 
						SWAPSHORT(FDDI_HCR_ALD_SKY);
					p_cmd->cpb[3] = 
						SWAPSHORT (FDDI_HCR_ATD_ALL);
				}
				
				FDDI_TRACE("IdaE",
					p_acs->addrs.hdw_addr_cnt, 
					p_acs->addrs.sw_addr_cnt,
					p_cmd->cmd_code);
				return(0);
			}
		}
	
	}

	
	FDDI_ETRACE("Ida3",p_acs->addrs.hdw_addr_cnt, p_acs->addrs.sw_addr_cnt,
		p_cmd->cmd_code);
	return(EINVAL);
}

/*
 * NAME: mib_addr
 *
 * FUNCTION: returns the address for the adapter
 *
 * EXECUTION ENVIRONMENT: process thread
 *
 * NOTES: 
 *	Returns the cards addresses in source, broadcast, group order.
 *	NOTE: the addresses are put into the ndd_addr_elem_t structure which 
 * 		is put into the area provided in a compact manner (you cannot
 *		index the elements as if they were an array (there isn't the
 *		normal 'c' spacers to fill out the structure)).
 *
 * RETURNS: 
 *	0 	 - Success
 *	E2BIG	 - If the buffer given was not large enough to hold all of the 
 * 			addresses, as many as could fit are copied in.
 */

int
mib_addr(
	fddi_acs_t *p_acs,
	ndd_mib_addr_t *p_mib_addr,
	int	len,
	fddi_cmd_t *p_cmd)
{
	ndd_mib_addr_elem_t 	*p_mib_addr_elem;
	fddi_addr_blk_t		*p_addr_blk;
	int 			i,j;

	FDDI_TRACE("ImaB",p_acs, p_acs->addrs.sw_addr_cnt, 
		p_acs->addrs.hdw_addr_cnt);

	FDDI_TRACE("ImaC",
		p_mib_addr, 
		len,&len);


	if (len < sizeof(ndd_mib_addr_t))
	{
		FDDI_ETRACE("Ima1",p_acs,len, FDDI_MIB_ADDR_MIN_LEN);
		return(EINVAL);
	}

	p_cmd->cmd_code = 0;
	p_mib_addr->count = 0;
	p_mib_addr_elem = &p_mib_addr->mib_addr[0];
	len -= 4;

	/*
	 * If there is room in the block of memory add the broad cast address 
	 */
	if ((p_mib_addr->count + 1) * FDDI_MIB_ADDR_ELEM_LEN <= len)
	{
		p_mib_addr->count++;
		p_mib_addr_elem->status = NDD_MIB_NONVOLATILE;
		p_mib_addr_elem->addresslen = CFDDI_NADR_LENGTH;
		for(i=0; i<CFDDI_NADR_LENGTH; i++)
			p_mib_addr_elem->address[i] = 0xff;
		p_mib_addr_elem = (ndd_mib_addr_elem_t *)
		&p_mib_addr_elem->address[CFDDI_NADR_LENGTH];
	}
	else 
	{
		FDDI_ETRACE("Ima2",p_acs,p_mib_addr->count, 0);
		return (E2BIG);
	}

	/* 
	 * If there is room add the source address for this adapter 
	 */
	if ((p_mib_addr->count + 1) * FDDI_MIB_ADDR_ELEM_LEN <= len)
	{
		p_mib_addr->count++;
		p_mib_addr_elem->status = NDD_MIB_NONVOLATILE;
		p_mib_addr_elem->addresslen = CFDDI_NADR_LENGTH;
		bcopy(p_acs->addrs.src_addr, p_mib_addr_elem->address, 
			CFDDI_NADR_LENGTH);
		p_mib_addr_elem = (ndd_mib_addr_elem_t *)
			&p_mib_addr_elem->address[CFDDI_NADR_LENGTH];
	}
	else 
	{
		FDDI_ETRACE("Ima3",p_acs,p_mib_addr->count, 0);
		return (E2BIG);
	}

	/*
	 * Add the addresses from the hardware list to the buffer, check each
	 * one for room.
	 */
	for (i=0; i<FDDI_MAX_HDW_ADDRS; i++)
	{
		if (p_acs->addrs.hdw_addrs[i].addr_cnt > 0)
		{
			if ((p_mib_addr->count + 1) * FDDI_MIB_ADDR_ELEM_LEN 
				<= len)
			{
				p_mib_addr->count++;
				p_mib_addr_elem->status = NDD_MIB_NONVOLATILE;
				p_mib_addr_elem->addresslen = CFDDI_NADR_LENGTH;
				bcopy(p_acs->addrs.hdw_addrs[i].addr, 
					p_mib_addr_elem->address, 
					CFDDI_NADR_LENGTH);
				p_mib_addr_elem = (ndd_mib_addr_elem_t *)
					&p_mib_addr_elem->address[CFDDI_NADR_LENGTH];
			}
			else 
			{
				FDDI_ETRACE("Ima4",p_acs,p_mib_addr->count, 0);
				return (E2BIG);
			}
		}
	}
	
	/*
	 * Add the software addresses to the buffer, check each one for room
	 */
	p_addr_blk = p_acs->addrs.sw_addrs;

	for (i=0, j=0; i<p_acs->addrs.sw_addr_cnt; j++)
	{
		if (j == FDDI_MAX_ADDR_BLK)
		{
			p_addr_blk = p_addr_blk->next;
			j=0;
			FDDI_ASSERT(p_addr_blk != 0);
		}

		if (p_addr_blk->addrs[j].addr_cnt > 0)
		{
			i++;
			if ((p_mib_addr->count + 1) * FDDI_MIB_ADDR_ELEM_LEN
				<= len)
			{
				p_mib_addr->count++;
				p_mib_addr_elem->status = NDD_MIB_NONVOLATILE;
				p_mib_addr_elem->addresslen = CFDDI_NADR_LENGTH;
				bcopy(p_addr_blk->addrs[j].addr, 
					p_mib_addr_elem->address, 
					CFDDI_NADR_LENGTH);
				p_mib_addr_elem = (ndd_mib_addr_elem_t *)
					&p_mib_addr_elem->address[CFDDI_NADR_LENGTH];
			}
			else 
			{
				FDDI_ETRACE("Ima5",p_acs,p_mib_addr->count, 0);
				return (E2BIG);
			}
		}
	}

	FDDI_TRACE("ImaE",p_acs, p_mib_addr->count,len);
	return (0);
}
	
/*
 * NAME: hcr_stat_cmplt
 *
 * FUNCTION: command completion routine for the get statistics ctl command
 *
 * EXECUTION ENVIRONMENT: interrupt thread
 *
 * NOTES: 
 *	call hcr_uls_cmplt to get the statistics into the acs and check the
 *			statistics for errors
 *	copy the statistics out to the users area
 *
 * RETURNS: 
 */
void
hcr_stat_cmplt (
	fddi_acs_t 	*p_acs,
	fddi_cmd_t	*p_cmd,
	int		bus,
	int		ipri)
{
	FDDI_TRACE("IscB",p_acs,p_cmd->stat,0);
	
	/*
	 * This should not be possible.  The statistics update, has no 
	 * parameters to be incorrect.  
	 */
	if (p_cmd->stat != 0)
	{
		FDDI_ETRACE("Isc1",p_acs, p_cmd->cmd_code, p_cmd->stat);
		p_acs->dev.cmd_status = EIO;
		e_wakeup (&p_acs->dev.cmd_event);
	}
	hcr_uls_cmplt(p_acs,p_cmd,bus, ipri);

	/*
	 * Set the elapsed time in statistics, difference from the open or 
	 * last reset till now.
	 */
	p_acs->ndd.ndd_genstats.ndd_elapsed_time = (lbolt-p_acs->dev.stime)/HZ;

	/* 
	 * Copy the ndd genstats.
	 */
	bcopy(&p_acs->ndd.ndd_genstats, &p_acs->dev.ls_buf.genstats, 
		sizeof(ndd_genstats_t));

	bcopy(&p_acs->ls, &p_acs->dev.ls_buf.fddistats, 
		sizeof(fddi_spec_stats_t));

	p_acs->dev.ls_buf.fddistats.setcount_lo = 	
		SWAPSHORT(p_acs->ls.setcount_lo);
	p_acs->dev.ls_buf.fddistats.setcount_hi = 
		SWAPSHORT(p_acs->ls.setcount_hi);

	p_acs->dev.ls_buf.fddistats.ndd_flags = p_acs->ndd.ndd_flags;
	p_acs->dev.ls_buf.genstats.ndd_ierrors = p_acs->ls.pframe_cnt;
	p_acs->dev.ls_buf.genstats.ndd_xmitque_cur = p_acs->tx.hdw_in_use +
		p_acs->tx.sw_in_use;
	
	p_cmd->cmd_code = 0;
	e_wakeup (&p_acs->dev.cmd_event);
	FDDI_TRACE("IscE",p_acs,0,0);
}
	
/*
 * NAME: hcr_addr_cmplt
 *
 * FUNCTION: handles the completion from setting or clearing a single address
 *
 * EXECUTION ENVIRONMENT: interrupt thread
 *
 * NOTES: 
 *
 * RETURNS: 
 */
void
hcr_addr_cmplt (
	fddi_acs_t 	*p_acs,
	fddi_cmd_t 	*p_cmd,
	int		bus,
	int		ipri)
{
	FDDI_TRACE("IacB",p_acs,p_cmd->cmd_code, p_cmd->stat);

	FDDI_ASSERT(p_cmd->stat == 0);

	p_cmd->cmd_code = 0;

	e_wakeup (&p_acs->dev.cmd_event);

	FDDI_TRACE("IacE",p_acs, p_cmd->cmd_code,0);
}

/*
 * NAME: hcr_all_addr_cmplt
 *
 * FUNCTION: handles the steps for exiting multicast mode due to addresses
 *
 * EXECUTION ENVIRONMENT: interrupt thread
 *
 * NOTES: 
 * 	This routine handles the steps for the driver to handle exiting 
 * 	multicast mode caused by exceeding the hardware addresses.  When it 
 *	was in this mode it did not keep the hardware list upto date on the 
 *	adapter.  So this routine is entered initially to complete the clearing
 * 	of all the addresses on the adapter.  Next it will set each of the 
 * 	addresses on the adapter, listed in the hardware list.  This is the 
 * 	same as the network recovery mode code's logic to reset the address
 * 	on the adapter after limbo.  When the last address is set, this routine
 * 	will re-write the smt_control word (which has had the multicast bit 
 * 	cleared when the command was originally set up).  This command will
 * 	complete in the hcr_smt_cmplt routine which will wake up the 
 * 	user.
 *
 * RETURNS: 
 */
void
hcr_all_addr_cmplt(
	fddi_acs_t 	*p_acs,
	fddi_cmd_t	*p_cmd,
	int		bus,
	int		ipri)
{
	int 		i,j;
	caddr_t		p_addr;
	
	FDDI_TRACE("IaaB",p_acs, p_cmd->cmd_code, p_cmd->stat);

	FDDI_ASSERT(p_cmd->stat == 0);
	
	/* Initialize the addr index to the top of the hardware list */
	if (p_cmd->cmd_code == FDDI_HCR_CLEAR_ADDR)
	{
		p_acs->dev.addr_index = 0;
	}

	/* Find the next address to set to the adapter */
	i = p_acs->dev.addr_index;
	while (i<FDDI_MAX_HDW_ADDRS)
	{
		if (p_acs->addrs.hdw_addrs[i].addr_cnt != 0)
			break;
		i++;
	}

	/* if an address was found, set up the command */
	if (i<FDDI_MAX_HDW_ADDRS)
	{
		p_acs->dev.addr_index = i + 1;
		/*
		* Reset command structure each time
		*/
		p_cmd->pri = 0;
		p_cmd->cmplt = (int(*)()) hcr_all_addr_cmplt;
		p_cmd->cmd_code = FDDI_HCR_WRITE_ADDR;
		p_cmd->cpb_len = 14;
		p_cmd->cpb[2] = SWAPSHORT(FDDI_HCR_ALD_SKY);
		p_cmd->cpb[3] = SWAPSHORT (FDDI_HCR_ATD_SKY);

		/*
	 	* Put address in proper format for the
		* adapter
	 	*/
		p_addr = (char *) &(p_cmd->cpb[4]);
		*(p_addr++)=p_acs->addrs.hdw_addrs[i].addr[5];
		*(p_addr++)=p_acs->addrs.hdw_addrs[i].addr[4];
		*(p_addr++)=p_acs->addrs.hdw_addrs[i].addr[3];
		*(p_addr++)=p_acs->addrs.hdw_addrs[i].addr[2];
		*(p_addr++)=p_acs->addrs.hdw_addrs[i].addr[1];
		*(p_addr++)=p_acs->addrs.hdw_addrs[i].addr[0];

		FDDI_TRACE("IaaD",p_acs, p_cmd->cmd_code,p_acs->dev.addr_index);
		return;
	}
	/* set up the write of the smt_control word */
	p_acs->dev.addr_index = 0;

	p_cmd->stat = 0;
	p_cmd->pri = 0;
	p_cmd->cmplt = (int(*)()) hcr_smt_cmplt;
	p_cmd->cmd_code = FDDI_HCR_WR_SMT_CTL;
	p_cmd->cpb[2] = SWAPSHORT(p_acs->dev.smt_control);
	p_cmd->cpb_len = 6;

	FDDI_TRACE("IaaE",p_acs, p_cmd->cmd_code, 0);
}

/*
 * NAME: hcr_smt_cmplt
 *
 * FUNCTION: Handles the completion to a write of the smt_control word
 *
 * EXECUTION ENVIRONMENT: process thread
 *
 * NOTES: 
 *	command should always succeed
 *
 * RETURNS: 
 */
void
hcr_smt_cmplt (
	fddi_acs_t 	*p_acs,
	fddi_cmd_t 	*p_cmd,
	int		bus,
	int		ipri)
{
	FDDI_TRACE("IstB",p_acs,p_cmd->cmd_code, p_cmd->stat);

	FDDI_ASSERT(p_cmd->stat == 0);

	p_cmd->cmd_code = 0;

	e_wakeup (&p_acs->dev.cmd_event);

	FDDI_TRACE("IstE",p_acs, p_cmd->cmd_code,0);
}
/*
 * NAME: fddi_gtrace
 *
 * FUNCTION: get the fddi adapter's memory for the user
 *
 * EXECUTION ENVIRONMENT: process thread
 *
 * NOTES: 
 *	Check the size of the area, fail if insufficent size
 *
 * RETURNS: 
 *	0 	- Success
 *	ENOMEM  - Unable to allocate enough memory
 */
int
fddi_gtrace (
	fddi_acs_t 	*p_acs,
	fddi_get_trace_t 	*ubuf)
{
	int			rc = 0;
	int			i, ipri;
	int			iocc, bus, ioa;
	caddr_t			p_gt;
	uchar			pos2;

	FDDI_TRACE("IfgB",p_acs,0,0);
	
	p_gt = xmalloc(FDDI_DATAMEM_SIZE,
			PGSHIFT,
			pinned_heap);

	if (p_gt == NULL)
	{
		FDDI_ETRACE("Ifg1",p_acs,0,0);
		return(ENOMEM);
	}

	ipri = disable_lock(CFDDI_OPLEVEL, &p_acs->dev.slih_lock);
	/* 
	 * get the adapter's POS registers
	 */
	iocc = IOCC_ATT( p_acs->dds.bus_id, (IO_IOCC + (p_acs->dds.slot<<16)));
	bus = BUSMEM_ATT(p_acs->dds.bus_id, (uint) p_acs->dds.bus_mem_addr);
	ioa = BUSIO_ATT(p_acs->dds.bus_id, p_acs->dds.bus_io_addr);

	for (i=0;i<8;++i)
		PIO_GETCX(iocc +i, &ubuf->pos[i]);
	
	PIO_GETSTRX(bus, p_gt, FDDI_SRAM_SIZE);

	bcopy (p_gt, &ubuf->sram, FDDI_SRAM_SIZE);

        /* get the SIF regs */
        PIO_GETSRX( ioa + FDDI_HSR_REG, &ubuf->regs.hsr);
        PIO_GETSRX( ioa + FDDI_HCR_REG, &ubuf->regs.hcr);
        PIO_GETSRX( ioa + FDDI_NS1_REG, &ubuf->regs.ns1);
        PIO_GETSRX( ioa + FDDI_NS2_REG, &ubuf->regs.ns2);
        PIO_GETSRX( ioa + FDDI_HMR_REG, &ubuf->regs.hmr);
        PIO_GETSRX( ioa + FDDI_NM1_REG, &ubuf->regs.nm1);
        PIO_GETSRX( ioa + FDDI_NM2_REG, &ubuf->regs.nm2);
        PIO_GETSRX( ioa + FDDI_ACL_REG, &ubuf->regs.acl);

        /*
         * we now flip the D/D bit in POS reg two to remap the
         * adapters memory so we can get at ALISA's Instruction
         * cmd regs.
         * after we get the ICRs, we will map back to the
         * original state
         */
        PIO_GETCX( iocc + FDDI_POS_REG2, &pos2);
        PIO_PUTCX( iocc + FDDI_POS_REG2,((pos2|FDDI_POS2_DD|FDDI_POS2_CEN)
                                                & ~(FDDI_POS2_AR)) );

        PIO_GETSTRX( bus, &ubuf->icrs, sizeof(fddi_icr_cmd_t) );

	BUSMEM_DET(bus);
	BUSIO_DET(ioa);

	bzero(p_gt, FDDI_DATAMEM_SIZE);
	rc = fddi_dma_acc(p_acs, p_gt, FDDI_AMDMEM_SIZE, 0x700);
	if (rc != 0)
		ubuf->status = rc;

	bcopy (p_gt, &ubuf->amd, FDDI_AMDMEM_SIZE);

	bzero(p_gt, FDDI_DATAMEM_SIZE);
	rc = fddi_dma_acc(p_acs, p_gt, FDDI_SKYMEM_SIZE, 0x900);
	if (rc != 0)
		ubuf->status = rc;

	bcopy (p_gt, &ubuf->sky, FDDI_SKYMEM_SIZE);

	bzero(p_gt, FDDI_DATAMEM_SIZE);
	rc = fddi_dma_acc(p_acs, p_gt, FDDI_DATAMEM_SIZE, 0x1000);
	if (rc != 0)
		ubuf->status = rc;

	bcopy (p_gt, &ubuf->dstore, FDDI_DATAMEM_SIZE);

	if (p_acs->dev.pio_rc)
	{
		FDDI_TRACE("Ifg+",p_acs,0,0);
		bugout(p_acs,NDD_PIO_FAIL, 0, 0, FALSE);
	}

        /*
         * restore POS REG 2.  We turn off the adapter
         * reset bit so as to not reset the adapter card.
         */
        PIO_PUTCX(iocc + FDDI_POS_REG2, (pos2 & ~(FDDI_POS2_AR)) );

	IOCC_DET(iocc);
	unlock_enable(ipri, &p_acs->dev.slih_lock);
	xmfree(p_gt, pinned_heap);

	FDDI_TRACE("IfgE",p_acs,rc,0);
	return(0);	
}
/*
 * NAME: fddi_dma_acc()
 *
 * FUNCTION:
 *      This function handles the DMA portion of the FDDI_MEM_ACC ioctl.
 *
 * EXECUTION ENVIRONMENT: called from process environment only
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *      None.
 *
 * DATA STRUCTURES:
 *      This routine modifies the FDDI_MEM_ACC variables that
 *      are in the dev section of the ACS.
 *
 * RETURNS:
 *
 *      EINVAL  Indicates an invalid parameter.
 *      EFAULT  Indicates that an invalid address was specified.
 *      EIO     Indicates an error.  Status field contains
 *              detailed error information.
 */

int
fddi_dma_acc(   fddi_acs_t      *p_acs,         /* ACS ptr */
                char		*p_mem,         /* Mem acces structure */
		uint		len,
		ushort		offset)
{
        int     rc=0, rc2=0, i, j;
        int     opcode;
        ushort  icr;
	uchar 	*kbuf;
	uint	p_d_addr;
	int	iocc,ioa,bus;
	ushort	pos2;
	ushort 	hmr;
	ushort 	tmp;

	FDDI_TRACE("ImaB",p_acs, offset, len);

        /* set up the xmem descriptor for the kernel buffers just xmalloced */
        p_acs->dev.dma_xmd.aspace_id = XMEM_GLOBAL;
	p_d_addr = p_acs->dev.p_d_kbuf;
	p_acs->dev.cmd_status = 0;

        /*
         * we now flush the buffer so as to sync up
         * the cache lines
         */
        (void)vm_cflush( p_mem, len );

        d_master(p_acs->dev.dma_channel, DMA_READ,
                                p_mem, len,
                                &p_acs->dev.dma_xmd,
                                p_d_addr);

        /*
         * build instruction command reg value
         */
        icr = 0xC800;

        /*
         * attach to the bus for accessing the POS regs, shared ram,
         * and the I/O regs.
         */
        bus = BUSMEM_ATT(p_acs->dds.bus_id, (ulong)p_acs->dds.bus_mem_addr);
        ioa = BUSIO_ATT(p_acs->dds.bus_id, p_acs->dds.bus_io_addr);


	PIO_PUTSRX(bus,offset);
	PIO_PUTSRX(bus+14, len);
	PIO_PUTSRX(bus+16, ADDR_HI(p_d_addr));
	PIO_PUTSRX(bus+18, ADDR_LO(p_d_addr));
	PIO_PUTSRX(bus+20, icr);

	PIO_GETSRX(ioa + FDDI_HMR_REG, &hmr);

	PIO_PUTSRX(ioa + FDDI_HMR_REG, 0xffff);

	PIO_GETSRX (ioa + FDDI_HSR_ADDR, &tmp);

	w_start (&(p_acs->dev.dnld_wdt));

	e_sleep_thread (&p_acs->dev.cmd_event, 
			&p_acs->dev.slih_lock, LOCK_HANDLER);

	w_stop (&(p_acs->dev.dnld_wdt));

	if (p_acs->dev.cmd_status != 0)
		rc = EIO;

        /*
         * restore the original Host Mask register
         */
        PIO_PUTSRX( ioa + FDDI_HMR_REG, hmr );

        BUSMEM_DET(bus);
	BUSIO_DET(ioa);

        d_complete(p_acs->dev.dma_channel, DMA_READ,
        	p_mem, len,
        	&p_acs->dev.dma_xmd,
        	p_d_addr);


	FDDI_TRACE("ImaE",p_acs, rc, 0);
        return(rc);
} /* end fddi_dma_acc() */
