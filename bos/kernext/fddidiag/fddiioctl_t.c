static char sccsid[] = "@(#)84	1.1  src/bos/kernext/fddidiag/fddiioctl_t.c, diagddfddi, bos411, 9428A410j 11/1/93 11:00:22";
/*
 *   COMPONENT_NAME: DIAGDDFDDI
 *
 *   FUNCTIONS: fddi_halt
 *		fddi_iocinfo
 *		fddi_ioctl
 *		fddi_issue_rcv_cmd1160
 *		fddi_query
 *		fddi_query_addr
 *		fddi_reg_acc
 *		fddi_set_long_addr
 *		fddi_start
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
#include <sys/ioctl.h>
#include <sys/devinfo.h>
#include <sys/poll.h>
#include <sys/mbuf.h>
#include <sys/errno.h>

#include "fddimacro.h"
#include <sys/ioacc.h>
#include <sys/adspace.h>
/* 
 * get access to the global device driver control block
 */
extern fddi_ctl_t	fddi_ctl;

/*
 * NAME: fddiioctl()
 *
 * FUNCTION: FDDI ioctl entry point.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *	Handles the  following ioctls
 *		IOCINFO
 *		CIO_START
 *		CIO_HALT
 *		CIO_QUERY
 *		CIO_GET_STAT
 *		CIO_GET_FASTWRT
 *		FDDI_GRP_ADDR
 *		FDDI_DOWNLOAD_UCODE
 *
 * RECOVERY OPERATION: Information describing both hardware and
 *	software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures, similar to NOTES.
 *
 * ROUTINES CALLED:
 *	minor(), lockl(), fddi_iocinfo(), fddi_start(), fddi_halt(),
 *	fddi_query(), fddi_get_stat(), fddi_get_fastwrt(), fddi_grpaddr(),
 *	fddi_download()
 *
 * RETURNS:
 *		EINVAL	- an invalid parameter	
 *		ENODEV	- an invalid minor number
 *		EIO	- an error
 *		return code from one of the following functions
 *			fddi_iocinfo()
 *			fddi_start()
 *			fddi_halt()
 *			fddi_query()
 *			fddi_get_stat()
 *			fddi_get_fastwrt()
 *			fddi_grpaddr()
 *			fddi_download()
 *			fddi_
 */
extern int fddi_fastwrite (
	int		devno, 		/* major/minor number */
	struct mbuf	*p_mbuf, 	/* chain of frames */
	chan_t		chan, 		/* channel number */
	cio_write_ext_t	*p_ext); 	/* the write ext */


int
fddi_ioctl( dev_t devno,		/* major and minor number */
	    int cmd,		/* ioctl operation */
	    int arg,		/* arg for this cmd, (usually struc ptr) */
	    ulong devflag,	/* flags */
	    chan_t chan,		/* mpx channel num */
	    int ext)		/* optional additional arg */
		
{
	register fddi_acs_t	*p_acs;
	register fddi_open_t	*p_open;
	int rc=0, adap;

	FDDI_TRACE("IfiB", devno, chan, cmd);
	FDDI_TRACE("IfiC", arg, ext, 0);

	/* 
	* sanity check the minor number 
	*/
	if ( ((adap = minor(devno)) < 0) || (adap >= FDDI_MAX_MINOR) )
		return(ENODEV);

	if (lockl(&fddi_ctl.fddilock,LOCK_SIGRET) != LOCK_SUCC)
	{
		return(EINTR);
	}

	if ((p_acs = fddi_ctl.p_acs[adap]) == NULL)
	{
		unlockl(&fddi_ctl.fddilock);
		return(ENODEV);
	}

	if (lockl(&p_acs->ctl.acslock,LOCK_SIGRET) != LOCK_SUCC)
	{
		unlockl(&fddi_ctl.fddilock);
		return(EINTR);
	}

	if ((p_open = fddi_ctl.p_open_tab [chan]) == NULL)
	{
		unlockl(&p_acs->ctl.acslock);
		unlockl(&fddi_ctl.fddilock);
		return(EINVAL);
	}

	unlockl(&fddi_ctl.fddilock);

	switch(cmd)
	{
		case IOCINFO:
			rc = fddi_iocinfo(p_acs, p_open, arg, devflag);
			break;

		case CIO_START:
			rc = fddi_start(p_acs, p_open, arg, devflag);
			break;

		case CIO_HALT:
			rc = fddi_halt(p_acs, p_open, arg, devflag);
			break;

		case CIO_QUERY:
			rc = fddi_query(p_acs, p_open, arg, devflag);
			break;

		case CIO_GET_STAT:
			rc = fddi_get_stat(p_acs, p_open, arg, devflag);
			break;

		case CIO_GET_FASTWRT:
		{
			if (arg == NULL)
			{
				rc = EINVAL;
				break;
			}
			if (!(devflag & DKERNEL))
			{
				rc = EACCES;
				break;
			}

 			/*
   			 * set values in user supplied arg structure
   			 */
  			((cio_get_fastwrt_t *)arg)->status = CIO_OK;
  			((cio_get_fastwrt_t *)arg)->fastwrt_fn = fddi_fastwrite;
  			((cio_get_fastwrt_t *)arg)->chan = p_open->chan;
  			((cio_get_fastwrt_t *)arg)->devno = p_open->devno;

			break;
		}

		case FDDI_SET_LONG_ADDR:
			rc = fddi_set_long_addr(p_acs, p_open, arg, devflag);
			break;

		case FDDI_QUERY_ADDR:
			rc = fddi_query_addr(p_acs, p_open, arg, devflag);
			break;

		case FDDI_DWNLD_MCODE:
			rc = fddi_download(p_acs, p_open, arg, devflag);
			break;

		case FDDI_MEM_ACC:
			rc = fddi_mem_acc(p_acs, p_open, arg, devflag);
			break;

		case FDDI_GET_TRACE:
			rc = fddi_get_trace(p_acs, p_open, arg, devflag);
			break;

		case FDDI_HCR_CMD:
			rc = fddi_hcr_cmd(p_acs, p_open, arg, devflag);
			break;

		case FDDI_REG_ACC :
			rc = fddi_reg_acc(p_acs, p_open, arg, devflag);
			break;

		case FDDI_ISSUE_RCV_CMD :
			rc = fddi_issue_rcv_cmd(p_acs, p_open, arg, devflag);
			break;
		default:
			FDDI_TRACE("Ifi1", cmd, EINVAL, p_acs);
			rc = EINVAL;

	} /* end switch(cmd) */

	unlockl(&p_acs->ctl.acslock);

	FDDI_TRACE("IfiE", p_acs, rc, p_acs->dev.state);
	return(rc);

} /* end fddiioctl() */

/*
 * NAME: fddi_query
 *                                                                    
 * FUNCTION: reads accumulated counter values by FDDI
 *                                                                    
 * EXECUTION ENVIRONMENT: called from process environment only
 *                                                                   
 * NOTES: 
 *	!!! There aren't any counters on the adapter ???
 *
 * 	Logic flow:
 *		Move in callers parameters 
 *		Move out Ras into callers buffer
 *		Move out callers parameters
 *
 * RECOVERY OPERATION: 
 *
 *	None.
 *
 * DATA STRUCTURES: 
 *
 *	The 'arg' parameter points to a fddi_query_stats_t struct.
 *
 * RETURNS: 
 *
 *	EFAULT	Indicates that an invalid address was specified.
 */  

static int
fddi_query (
	fddi_acs_t		*p_acs, 
	fddi_open_t		*p_open, 
	struct query_parms 	*p_arg_query, 
	ulong			devflag)
{
	struct query_parms	query;
	int			len;
	int			rc;
	fddi_cmd_t		*p_cmd;
	extern			int fddi_hcr_cmd_cmplt();

	FDDI_TRACE("IqsB", p_acs, p_arg_query,0);

	if (p_acs->dev.state == FDDI_DEAD)
	{
		FDDI_TRACE("Iqs1", p_arg_query,0,0);
		return(ENETDOWN);
	}

	if (p_acs->dev.state == FDDI_LIMBO)
	{
		FDDI_TRACE("Iqs2", p_arg_query,0,0);
		return(ENETUNREACH);
	}

	if (p_arg_query == NULL)
	{
		FDDI_TRACE("Iqs3", p_arg_query,0,0);
		return (EINVAL);
	}
	/* get the caller's parameters */
	if (MOVEIN (devflag, p_arg_query, &query, sizeof(query)))
	{
		/*
		* normalize the MOVEIN return codes by returning EFAULT to the 
		* user.
		*/
		FDDI_TRACE("Iqs4", p_arg_query,0,0);
		return (EFAULT);
	}

	/* calculate length of the RAS data to return to the caller */
	len = MIN(query.buflen, sizeof(fddi_query_stats_t));	
	query.status = CIO_OK;
	query.buflen = len;

	p_cmd = &(p_acs->dev.cmd_blk);
	p_cmd->pri = 0;
	p_cmd->cmd_code = FDDI_HCR_ULS;
	p_cmd->cmplt = (int(*)()) fddi_query_stats_cmplt;
	p_cmd->cpb_len = 0;

	if (rc = fddi_send_cmd(p_acs, p_open, p_cmd))
	{
		FDDI_TRACE("Iqs5", p_arg_query,rc,0);
		return(rc);
	}

	rc = fddi_get_query(p_acs, &query, len, devflag);

	/* return parameter block to caller */
	if (MOVEOUT (devflag, &query, p_arg_query, sizeof(cio_query_blk_t)))
	{
		/* if the copyout failed, return */
		FDDI_TRACE("Iqs6", p_arg_query,0,0);
		return (EFAULT);
	}
	
	FDDI_TRACE("IqsE", p_arg_query,rc,0);
	return (rc);
}
/*
 * NAME: fddi_start
 *                                                                    
 * FUNCTION: initiates a session with the FDDI device driver
 *                                                                    
 * EXECUTION ENVIRONMENT: called from process environment only
 *                                                                   
 * NOTES: 
 *	This routine assumes that the ACS lock is being held
 *	by the caller 
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 *	'arg' points to a session_blk and contains the following
 *	fields:
 *
 *	status	Contains the possible return values:
 *		CIO_OK
 *		CIO_NETID_FULL
 *		CIO_NETID_DUP
 *
 *	netid	Specifies the network ID the caller will use on the network.
 *		The netid is place in the least significant byte of 
 *		the netid field.
 *
 * ROUTINES CALLED:
 *	fddi_act(), fddi_report_status()
 *
 * RETURNS: 
 *
 *	EINVAL		Indicates an invalid parameter.
 *	ENETDOWN	Indicates an unrecoverable hardware error.
 *	ENOMSG		Indicates an error.
 *	ENOSPC		Indicates the network ID table is full.
 *	EADDRINUSE	Indicates the network ID is in use.
 */  

int
fddi_start(fddi_acs_t *p_acs, 		/* ACS ptr */
	   fddi_open_t *p_open, 	/* Open element ptr */
	   caddr_t *p_arg, 
	   ulong devflag)
{
	cio_sess_blk_t sess_blk;
	cio_stat_blk_t	sb;

	FDDI_DBTRACE("IstB",p_acs->dev.state, p_open, p_arg);
	if (p_arg == NULL)
	{
		return (EINVAL);
	}
	if (p_acs->dev.state == FDDI_DEAD)
	{
		return(ENETDOWN);
	}

	if ((p_acs->dev.state == FDDI_NULL) ||
		(p_acs->ctl.mode == 'C'))
	{
		return(EINVAL);
	}
	
	/*
	 * Copyin the session block the user passed in 
	 */
	if (MOVEIN(devflag, p_arg, &sess_blk, sizeof(sess_blk)))
	{
		/*
		 * normalize the Copyin return codes by returning EFAULT to the 
		 * user.
		 */
		FDDI_DBTRACE("Ist1", p_arg, 0, 0);
		return (EFAULT);
	}
	/* 
	 * Sanity check network ID given. 
	 */
	if (sess_blk.netid >= FDDI_MAX_NETIDS) 
	{
		FDDI_DBTRACE("Ist2", sess_blk.netid, 0, 0);
		sess_blk.status = CIO_NETID_INV;
		if (MOVEOUT(devflag, &sess_blk, p_arg, sizeof(sess_blk)))
		{
			/* if the copyout failed, return */
			return (EFAULT);
		}
		return(EIO);
	}
	
	if (p_acs->ctl.p_netids[sess_blk.netid] != NULL)
	{
		FDDI_DBTRACE("Ist3", sess_blk.netid, 0, 0);
		sess_blk.status = CIO_NETID_DUP;
		if (MOVEOUT(devflag, &sess_blk, p_arg, sizeof(sess_blk)))
		{
			/* if the copyout failed, return */
			return (EFAULT);
		}
		return(EIO);
	}
	
	fddi_start_netid(p_acs, p_open, sess_blk.netid);

	if (p_acs->dev.state == FDDI_DWNLD) 
	{
		/* 
		 * initiate the activation of the adapter.
		 */

		fddi_act (p_acs);
	} 
	else if (p_acs->dev.state != FDDI_INIT) 
	{
		/* 
		 * we are already connected to the net.
		 * let the user know via async status block
		 */
		sb.code = CIO_START_DONE;
		sb.option[0] = CIO_OK;
		sb.option[1] = sess_blk.netid;
		sb.option[3] = 0;
		bcopy (&(p_acs->ctl.long_src_addr[0]), 
			(char *) &sb.option[2], 
			FDDI_NADR_LENGTH);
		fddi_report_status(p_acs, p_open, &sb);
	}

	/* 
	 * NB:
	 *	In the default else case, we have already
	 *	kicked off the adapter activation sequence.
	 *	The activation sequence has not yet completed.
	 *	When it completes, this user will get the 
	 *	CIO_START_DONE block indicating success or failure.
	 */
	sess_blk.status = CIO_OK;
	if (MOVEOUT(devflag, &sess_blk, p_arg, sizeof(sess_blk)))
	{
		/* if the copyout failed, return */
		return (EFAULT);
	}

	FDDI_DBTRACE("IstE", p_acs->dev.state, p_open, 0);
	return(0);

} /* end fddi_start() */

/*
 * NAME: fddi_halt
 *                                                                    
 * FUNCTION: ends a session with FDDI 
 *                                                                    
 * EXECUTION ENVIRONMENT: called from process environment only
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 *	The 'arg' parameter points to a session_blk struct.
 *	It contains the following:
 *	
 *	status	Returned is one of the following:
 *		CIO_OK
 *		CIO_NETID_INV
 *
 *	netid	Specifies netid taken out of the netid table.
 *
 * RETURNS: 
 *
 *	EINVAL	Indicates an invalid parameter.
 *	EFAULT	Indicates that an invalid address was specified.
 *	EIO	Indicates an error.  Status field contains
 *		detailed error information.
 */  

int
fddi_halt(fddi_acs_t *p_acs, 		/* ACS ptr */
	  fddi_open_t *p_open, 	/* Open element ptr */
	  caddr_t *arg, 
	  ulong devflag)
{
	cio_sess_blk_t	sess_blk;
	cio_stat_blk_t	sb;
	int rc;
	
	if (arg == NULL)
	{
		return (EINVAL);
	}
	if (MOVEIN(devflag, arg, &sess_blk, sizeof(sess_blk)))
	{
		/*
		* normalize the Copyin return codes by returning EFAULT to the 
		* user.
		*/
		return (EFAULT);
	}

	/* 
	* Sanity check network ID given. 
	*/
	if (sess_blk.netid >= FDDI_MAX_NETIDS)
	{
		sess_blk.status = CIO_NETID_INV;
		if (MOVEOUT(devflag, &sess_blk, arg, sizeof(sess_blk)))
		{
			/* if the copyout failed, return */
			return (EFAULT);
		}
		return(EIO);
	}
	
	if ( p_open != p_acs->ctl.p_netids[sess_blk.netid] )
	{
		/* 
		 * this user does not "own" this
		 * netid, return error.
		 */
		sess_blk.status = CIO_NETID_INV;
		if (MOVEOUT(devflag, &sess_blk, arg, sizeof(sess_blk)))
		{
			/* if the copyout failed, return */
			return (EFAULT);
		}
		return(EIO);

	}

	fddi_halt_netid(p_acs, p_open, sess_blk.netid);
	/* 
	 * fill out the Halt done
	 * status block and send it to the user.
	 */
	sb.code = CIO_HALT_DONE;
	sb.option[0] = CIO_OK;
	sb.option[1] = sess_blk.netid;
	sb.option[2] = NULL;
	sb.option[3] = NULL;
	fddi_report_status(p_acs, p_open, &sb);

	sess_blk.status = CIO_OK;

	if (MOVEOUT(devflag, &sess_blk, arg, sizeof(sess_blk)))
	{
		/* if the copyout failed, return */
		return (EFAULT);
	}
	return(0);

} /* end fddi_halt() */




/*
 * NAME: fddi_iocinfo
 *                                                                    
 * FUNCTION: returns structure that describes the FDDI device
 *                                                                    
 * EXECUTION ENVIRONMENT: called from process environment only
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 *	The arg parameter points to the devinfo structure
 *	which contains the following fields:
 *	
 *	devtype		filled in with DD_NET_DH in <sys/devinfo.h>
 *	devsubtype	filled in with DD_FDDI in <sys/devinfo.h>
 *
 * 			Found in dds structure:
 *	broadcast_echo	specifies if wrapping of broadcast msgs is supported
 *	rdto		specifies the rcv data transfer offset value.
 *	processor_id	the processor id used by other systems to address this
 *			system.
 * RETURNS: 
 *
 *	0		if successful
 *	EFAULT	-	Indicates that an invalid address was specified.
 *	EINVAL	-	Indicates an invalid parameter.
 */  

static int
fddi_iocinfo (
	fddi_acs_t	*p_acs, 
	fddi_open_t	*p_open, 
	struct devinfo	*p_arg_dev, 
	ulong		devflag)
{
	struct	devinfo	dev;

	if (p_arg_dev == NULL)
	{
		return (EINVAL);
	}
	/* fill in devinfo structure */
	dev.devtype = DD_NET_DH;
	dev.devsubtype = DD_FDDI;
	dev.un.fddi.broad_wrap = FALSE;
	dev.un.fddi.rdto = p_acs->dds.rdto;

	/*
	 * return the HW address that is in the
	 * VPD for the adapter
	 */
	bcopy(&p_acs->ctl.vpd_addr[0], &dev.un.fddi.haddr[0], 
		FDDI_NADR_LENGTH);


	dev.un.fddi.attach_class = p_acs->dev.attach_class;

	/*
	 * return the long source address that we
	 * are configured to run with.  This is the address
	 * that goes into the adapter's FORMAC chip.
	 */
	bcopy(&p_acs->ctl.long_src_addr[0], &dev.un.fddi.netaddr[0],
		FDDI_NADR_LENGTH);

	/* copy to caller space dependent on devflag */
	if (MOVEOUT (devflag, &dev, p_arg_dev, sizeof(struct devinfo)))
	{
		/* if the move failed, return */
		return (EFAULT);
	}

	/* ok */
	return (0);
}


	
/*
 * NAME: fddi_set_long_addr
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

static int
fddi_set_long_addr(
	fddi_acs_t		*p_acs, 
	fddi_open_t		*p_open, 
	cio_get_fastwrt_t	*p_arg,
	ulong			devflag)
{
	fddi_cmd_t 		*p_cmd;
	fddi_set_addr_t		saddr;
	int			rc, i;
	char			*p_addr;
	extern int 		fddi_sq_addr_cmplt();

	FDDI_DBTRACE("IslB",p_acs->dev.state, p_open, p_arg);
	if (p_arg == NULL)
	{
		return (EINVAL);
	}
	if ((p_acs->dev.state != FDDI_OPEN) && 
		(p_acs->dev.state != FDDI_LLC_DOWN))
	{
		if (p_acs->dev.state == FDDI_DEAD)
		{
			return(ENETDOWN);
		}

		if (p_acs->dev.state == FDDI_LIMBO)
		{
			return(ENETUNREACH);
		}

		return(ENOCONNECT);
	}

	if (p_open->netid_cnt == 0)
	{
		saddr.status = CIO_NOT_STARTED;
		if (MOVEOUT(devflag, &saddr, p_arg, sizeof(fddi_set_addr_t)))
		{
			/* if copyout failed, return */
			FDDI_TRACE("Isl1", p_acs->dev.state, 0, 0);
			return (EFAULT);
		}
		FDDI_TRACE("Isl2", p_acs->dev.state, 0, 0);
		return (EIO);
	}
	/*
	 * Copyin the session block the user passed in 
	 */
	if (MOVEIN(devflag, p_arg, &saddr, sizeof(saddr)))
	{
		/*
		 * normalize the Copyin return codes by returning EFAULT to 
		 * the user.
		 */
		return (EFAULT);
	}
	
	rc = 0;
	for (i=0; i<FDDI_MAX_ADDRS; i++)
		if ((p_acs->ctl.addrs[i].cnt != 0) && 
				(!(bcmp(saddr.addr, 
					p_acs->ctl.addrs[i].addr,
					FDDI_NADR_LENGTH))))
			if (saddr.opcode == FDDI_DEL)
			{
				if (!(p_open->addrs & (1<<i)))
				{
					saddr.status = FDDI_NO_ADDR;
					if (MOVEOUT(devflag,
						&saddr, p_arg, 
						sizeof(fddi_set_addr_t)))
					{
						/*if copyout failed, return */
						FDDI_TRACE("Isl4",p_arg,0,0);
						return (EFAULT);
					}
					FDDI_TRACE("Isl5", saddr.status,0,0);
					return(EIO);
				}
					
				p_open->addrs &= (~(1<<i));
				if (--p_acs->ctl.addrs[i].cnt != 0)
				{	
					saddr.status = CIO_OK;
					if (MOVEOUT(devflag,
						&saddr, p_arg, 
						sizeof(fddi_set_addr_t)))
					{
						/* if copyout failed, return */
						FDDI_TRACE("Isl6", p_arg,0,0);
						return (EFAULT);
					}
					FDDI_TRACE("IslE", saddr.status,0,0);
					return(0);
				}
				break;
			}
			else
			if (saddr.opcode == FDDI_ADD)
			{
				p_open->addrs |= (1<<i);
				p_acs->ctl.addrs[i].cnt++;

				saddr.status = CIO_OK;
				if (MOVEOUT(devflag,
					&saddr, p_arg, 
					sizeof(fddi_set_addr_t)))
				{
					/* if copyout failed, return */
					FDDI_TRACE("Isl7", p_arg,0,0);
					return (EFAULT);
				}
				FDDI_TRACE("IslE", saddr.status,0,0);
				return(0);
			}
		
	if ((i == FDDI_MAX_ADDRS) && (saddr.opcode == FDDI_DEL))
	{
		saddr.status = FDDI_NO_ADDR;
		if (MOVEOUT(devflag, &saddr, p_arg, sizeof(fddi_set_addr_t)))
		{
			/* if copyout failed, return */
			FDDI_TRACE("Isl8", p_arg, 0, 0);
			return (EFAULT);
		}
		FDDI_TRACE("Isl9", p_arg, 0, 0);
		return(EIO);
	}
	 
	/*
	 * set up command parameter block
	 */
	p_cmd = &(p_acs->dev.cmd_blk);
	p_cmd->stat = 0;
	p_cmd->pri = 0;
	p_cmd->cmplt = (int(*)()) fddi_sq_addr_cmplt;
	p_cmd->cpb_len = 14;
	p_cmd->cpb[2] = SWAPSHORT(FDDI_HCR_ALD_SKY);
	p_cmd->cpb[3] = SWAPSHORT (FDDI_HCR_ATD_SKY);
	/*
	 * Put address in proper format for adapter
	 */
	p_addr = (char *) &(p_cmd->cpb[4]);
	*(p_addr++) = saddr.addr[5];
	*(p_addr++) = saddr.addr[4];
	*(p_addr++) = saddr.addr[3];
	*(p_addr++) = saddr.addr[2];
	*(p_addr++) = saddr.addr[1];
	*(p_addr++) = saddr.addr[0];

	if (saddr.opcode == FDDI_ADD)
	{
		p_cmd->cmd_code = FDDI_HCR_WRITE_ADDR;
	}
	else if (saddr.opcode == FDDI_DEL)
	{
		p_cmd->cmd_code = FDDI_HCR_CLEAR_ADDR;
	}
	else 
	{
		saddr.status = CIO_INV_CMD;
		if (MOVEOUT(devflag, &saddr, p_arg, sizeof(fddi_set_addr_t)))
		{
			/* if copyout failed, return */
			FDDI_TRACE("Isla", p_arg, 0, 0);
			return (EFAULT);
		}
		FDDI_TRACE("Islb", p_arg, 0, 0);
		return(EIO);
	}

	rc = fddi_send_cmd(p_acs, p_open, p_cmd);
	if (rc != CIO_OK)
	{
		FDDI_TRACE("Islc", rc, p_acs->dev.state, 0);
		return(rc);
	}
	
	saddr.status = p_acs->dev.ioctl_status;
	if ((rc == CIO_OK) && (saddr.status == CIO_OK))
		if (saddr.opcode == FDDI_ADD)
			for (i=0; i<FDDI_MAX_ADDRS; i++)
				if (p_acs->ctl.addrs[i].cnt == 0) 
				{
					p_open->addrs |= (1<<i);
					p_acs->ctl.addrs[i].cnt++;
					/*
				 	* Save address in the format the
				 	*	adapter understands
				 	*/
					bcopy((char *) &saddr.addr,
						p_acs->ctl.addrs[i].addr,
						FDDI_NADR_LENGTH);
					break;
				}

	if (MOVEOUT(devflag, &saddr, p_arg, sizeof(fddi_set_addr_t)))
	{
		/* if the copyout failed, return */
		FDDI_TRACE("Isld", p_arg,0,0);
		return (EFAULT);
	}
	FDDI_DBTRACE("IslE", saddr.status, 0, 0);
	if (saddr.status != CIO_OK)
		return(EIO);
	return(rc);
}

/*
 * NAME: fddi_query_addr
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
fddi_query_addr(
	fddi_acs_t		*p_acs, 
	fddi_open_t		*p_open, 
	fddi_query_addr_t	*p_arg,
	ulong			devflag)
{
	fddi_cmd_t 		*p_cmd;
	char 			*p_addr;	/* tmp ptr for long address */
	fddi_query_addr_t	qaddr;
	int			i, rc;
	extern int		fddi_sq_addr_cmplt();

	FDDI_DBTRACE("IqaB",p_acs->dev.state, p_open, p_arg);
	if (p_arg == NULL)
	{
		return (EINVAL);
	}
	if ((p_acs->dev.state != FDDI_OPEN) && 
		(p_acs->dev.state != FDDI_LLC_DOWN))
	{
	
		if (p_acs->dev.state == FDDI_DEAD)
		{
			FDDI_TRACE("Iqa1", p_arg,0,0);
			return(ENETDOWN);
		}
	
		if (p_acs->dev.state == FDDI_LIMBO)
		{
			FDDI_TRACE("Iqa2", p_arg,0,0);
			return(ENETUNREACH);
		}

		FDDI_TRACE("Iqa3", p_arg,0,0);
		return(ENOCONNECT);

	}
	if (p_open->netid_cnt == 0)
	{
		qaddr.status = CIO_NOT_STARTED;
		if (MOVEOUT(devflag, &qaddr, p_arg, sizeof(fddi_query_addr_t)))
		{
			/* if copyout failed, return */
			FDDI_TRACE("Iqa4", p_arg,0,0);
			return (EFAULT);
		}
		FDDI_TRACE("Iqa5", p_arg,0,0);
		return (EIO);
	}

	qaddr.addr_cnt = 1;
	qaddr.status = CIO_OK;
	/*
	 * get count (swapped when read out of shared mem)
	 *	then get a character pointer to the first address
	 *	to be read.
	 */
	bcopy(p_acs->ctl.long_src_addr,
		qaddr.addrs[0],
		FDDI_NADR_LENGTH);
	

	for (i=0; i<FDDI_MAX_ADDRS; i++)
	{
		if (p_acs->ctl.addrs[i].cnt != 0) 
		{
			bcopy(p_acs->ctl.addrs[i].addr,
				qaddr.addrs[qaddr.addr_cnt],
				FDDI_NADR_LENGTH);
			qaddr.addr_cnt++;
		}
	}
	if (MOVEOUT(devflag, &qaddr, p_arg, sizeof(fddi_query_addr_t)))
	{
		/* if the copyout failed, return */
		FDDI_TRACE("Iqa9", p_arg,0,0);
		return (EFAULT);
	}
	FDDI_DBTRACE("IqaE", qaddr.status, 0, 0);
	return(0);
}


/*****************************************************************************/
/*                                                                           */
/* NAME: ioc_reg                                                             */
/*                                                                           */
/* FUNCTION: ioctl() for reading/writing I/O registers                       */
/*                                                                           */
/* EXECUTION ENVIRONMENT:                                                    */
/*                                                                           */
/*      This routine runs only under the process thread.                     */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*    Input: dds_ptr, arg, devflag                                           */
/*                                                                           */
/*    Output: N/A                                                            */
/*                                                                           */
/*    Called From: xxx_ioctl                                                 */
/*                                                                           */
/*                                                                           */
/* RETURN:  0, ENOMEM, ..                                                    */
/*                                                                           */
/*****************************************************************************/
int fddi_reg_acc (
	fddi_acs_t		*p_acs, 
	fddi_open_t		*p_open, 
	fddi_reg_acc_t		*p_arg,
	ulong			devflag)
{
	int              rc;         /* return code */
	fddi_reg_acc_t   io_data;    /* Diagnostic I/O access data structure */
	int              ioa;        /* I/O access variables */


	FDDI_TRACE("DraB", p_acs, p_open, p_arg);

	if (p_arg == NULL)
	{
		return (EINVAL);
	}
	/*
	 * check to see if the user has opened the driver in 
	 * diagnostic mode
	 */
	if ( p_acs->ctl.mode != 'D' )
	{
		rc = EIO; 
		io_data.status = FDDI_NOT_DIAG_MODE;
		if(MOVEOUT(devflag, &io_data, p_arg, sizeof(fddi_reg_acc_t) ))
			rc = EFAULT;

		FDDI_TRACE("Dra2", p_acs->ctl.mode, io_data.status, rc);
		return(rc);
	}	
	else if ( (p_acs->dev.state != FDDI_DWNLD) &&
		( p_acs->dev.state != FDDI_NULL) )

	{
		rc = EIO;
		io_data.status = FDDI_STARTED;
		if(MOVEOUT(devflag, &io_data, p_arg, sizeof(fddi_reg_acc_t) ))
			rc = EFAULT;

		FDDI_TRACE("Dra3", p_acs->ctl.mode, io_data.status, rc);
		return(rc);
	}
	/*
	 * Get Access to the ioctl parameters -
	 * User space or Kernel space
	 */
	if (rc = MOVEIN (devflag, p_arg, &io_data, sizeof(fddi_reg_acc_t)))
	{
		FDDI_TRACE ("Dra4", (ulong) 0,0,0);
		return(rc);
	}

	/*
	 * Initialize status field in structure
	 */
	io_data.status = (ulong)CIO_OK;

	/*
	 * This is diagnostic mode - test for
	 * valid ranges
	 */

	/*
	 * Test if valid I/O register range and
	 * valid opcode
	 */
	if ( ((io_data.io_reg & 0x01) ||
	      (io_data.io_reg < 0) ||
	      (io_data.io_reg > 14))       ||

	     ((io_data.opcode != FDDI_READ_OP) &&
	      (io_data.opcode != FDDI_WRITE_OP)) )
	{
			/*
			 * Bad parameters sent -
			 * Update status with error
			 */
			io_data.status = (ulong)CIO_BAD_RANGE;
	}
	else
	{
		/*
		 * Get access to the I/O bus to access
		 * I/O registers
		 */
		ioa = BUSIO_ATT (p_acs->dds.bus_id, p_acs->dds.bus_io_addr);

		/*
		 * Determine whether to do a read or
		 * a write
		 */
		if (io_data.opcode == (ushort)FDDI_READ_OP)
		{
			FDDI_TRACE("Dra5",(ulong)(ioa),(ulong)io_data.io_reg,0);
			/*
			 * Read the selected I/O register and save it
			 */
			PIO_GETSRX(ioa+io_data.io_reg, &io_data.io_val);
		}
		else
		{
			FDDI_TRACE("Dra6",(ulong)(ioa),(ulong)io_data.io_reg,0);
			/*
			 * Write the selected I/O
			 * register with data provided
			 */
			PIO_PUTSRX((ioa + io_data.io_reg), io_data.io_val );
		} /* end if for read/write test */

		FDDI_TRACE ("Dra7",(ulong)io_data.io_val,0,0);

		/*
		 * restore IO bus to previous value,
		 * done accessing I/O Reg
		 */
		BUSIO_DET(ioa);

	} /* endif for range test */

	/*
	 * Test for which error to return
	 */
	if (io_data.status == (ulong)CIO_OK)
		rc = 0;
	else
	{
		FDDI_TRACE ("Dra8",1,0,0);
		return (EINVAL);
	}

	/*
	 * Restore the ioctl parameters - User space or
	 * Kernel space
	 */
	if (MOVEOUT(devflag, &io_data, p_arg, sizeof(fddi_reg_acc_t)))
	{
			FDDI_TRACE ("Dra9", 2,0,0);
			return(EFAULT);
        }
	return(0);
} /* End access I/O registers diagnostic ioctl */

/*
 * NAME: fddi_issue_rcv_cmd
 *                                                                    
 * FUNCTION: returns structure that describes the FDDI device
 *                                                                    
 * EXECUTION ENVIRONMENT: called from process environment only
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 *
 *	0		if successful
 */  

static int
fddi_issue_rcv_cmd (
	fddi_acs_t	*p_acs, 
	fddi_open_t	*p_open, 
	struct devinfo	*p_arg_dev, 
	ulong		devflag)
{
	int		ioa;
	FDDI_TRACE("IrcB", p_acs, p_open, p_acs->dev.state);

	if (p_arg_dev == NULL)
	{
		return (EINVAL);
	}
	if ((p_acs->dev.state != FDDI_OPEN) && 
		(p_acs->dev.state != FDDI_LLC_DOWN))
	{
	
		if (p_acs->dev.state == FDDI_DEAD)
		{
			FDDI_TRACE("Irc1", 0,0,0);
			return(ENETDOWN);
		}
	
		if (p_acs->dev.state == FDDI_LIMBO)
		{
			FDDI_TRACE("Irc2", 0,0,0);
			return(ENETUNREACH);
		}

		FDDI_TRACE("Irc3", 0,0,0);
		return(EINVAL);

	}

	if (p_open->netid_cnt == 0)
	{
		FDDI_TRACE("Irc4", 0,0,0);
		return (EIO);
	}

	ioa = BUSIO_ATT (p_acs->dds.bus_id, p_acs->dds.bus_io_addr);

	p_acs->ras.ds.rcv_cmd_cmplt++;
	PIO_PUTSRX(ioa+FDDI_NS1_REG, FDDI_NS1_RCV);

	BUSIO_DET (ioa);
	FDDI_TRACE("IrcE", p_acs, p_open, p_acs->dev.state);
	/* ok */
	return (0);
}
