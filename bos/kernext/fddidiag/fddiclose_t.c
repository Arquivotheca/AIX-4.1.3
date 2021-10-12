static char sccsid[] = "@(#)76	1.1  src/bos/kernext/fddidiag/fddiclose_t.c, diagddfddi, bos411, 9428A410j 11/1/93 10:59:52";
/*
 *   COMPONENT_NAME: DIAGDDFDDI
 *
 *   FUNCTIONS: fddi_close
 *		fddi_free_rcvq
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
#include <sys/mbuf.h>
#include <sys/sleep.h>


/* 
 * get access to the global device driver control block
 */
extern fddi_ctl_t	fddi_ctl;

/*
 * NAME:     fddi_close()
 *
 * FUNCTION: FDDI close entry point from kernel.
 *
 * EXECUTION ENVIRONMENT:
 *	Process thread.
 *
 * NOTES:
 *	This routine will close the device for the
 *	specified user.  For each close, this routine will:
 *
 *		remove any functional/group address that this user set
 *		remove any outstanding netids that are in the hash table
 *		Free all rcv data (user mode only)
 *		wait for transmit completes when acknowledgements requested
 *		free the open element for this user
 *		
 *	
 * ROUTINES CALLED:
 *	minor(), lockl(), 
 *
 * RETURNS:
 *		0	- successful
 *		ENODEV	- Indicates that an invalid minor number was specified
 *
 */
int
fddi_close( dev_t	devno,		/* major and minor number */
	    chan_t	chan)		/* mpx channel number */
{
	int 			i,adap;
	int			bus;
	register fddi_acs_t 	*p_acs;
	register fddi_open_t 	*p_open;
	fddi_set_addr_t		saddr;
	fddi_cmd_t		*p_cmd;
	char			*p_addr;
	extern int		fddi_sq_addr_cmplt();

	FDDI_TRACE("ClsB", devno, chan, 0);
	/* sanity check minor num 
	 */
	if ( ((adap= minor(devno)) < 0) || (adap >= FDDI_MAX_MINOR) )
	{
		FDDI_DBTRACE("Cls1", adap, 0, 0);
		return(ENODEV); 
	}
	/* grab device lock to serialize with the 
	 * config entry point
	 */

	if (lockl(&fddi_ctl.fddilock, LOCK_SIGRET) != LOCK_SUCC)
		return(EINTR);

	/* sanity check the ACS ptr 
	 */
	if ( (p_acs = fddi_ctl.p_acs[adap]) == NULL)
	{
		/* unlockl global driver lock */
		unlockl(&fddi_ctl.fddilock);

		FDDI_DBTRACE("Cls2", adap, 0, 0);
		return(ENODEV); 
	}

	if ((p_open = fddi_ctl.p_open_tab [chan]) == NULL)
	{
		/* unlockl global driver lock */
		unlockl(&fddi_ctl.fddilock);
		FDDI_DBTRACE("Cls3", adap, 0, 0);
		return(EINVAL); 
	}

	if (lockl(&p_acs->ctl.acslock,LOCK_SIGRET) != LOCK_SUCC)
	{
		unlockl(&fddi_ctl.fddilock);
		FDDI_DBTRACE("Cls4", adap, 0, 0);
		return(EINTR);
	}
 
	/*
	 * Remove this users netids that in the Net ID hash table.
	 *	This will prevent any new data from coming from the
	 *	net to this user.
	 */
	(void)fddi_remove_netids(p_acs, p_open);

	/* 
	 * Free the addresses for this open_id
	 */
	fddi_free_addr(p_acs, p_open);

	unlockl(&p_acs->ctl.acslock);
	/*
	 * Free all rcv data (user mode only).  At this point
	 * the user cannot receive any more data from the network.
	 */

	if( !(p_open->devflag & DKERNEL) )
	{
		fddi_free_rcvq(p_acs, p_open);
	}

	/*
	 * Remove the acknowledgement from any frames on the software queue
	 * from this user.  The frames will still be transmitted but the 
	 * acknowledgement will be ignored. 
	 */

	fddi_clean_swque(p_acs, p_open);

	/*
	 * See if this user has any outstanding tx acks pending.
	 *	If so then we must wait for them to complete. 
	 */
	if (p_open->tx_acks > 0)
	{
		/* 
		 * User has outstanding acks ... wait for them
		 *	to finish. Transmits guarenteed to be
		 *	gone when we wakeup. (This timer is 
		 *	the same as the tx timeout value.)
		 */
		w_start (&(p_acs->dev.close_wdt));
		e_sleep (&(p_acs->dev.close_event), EVENT_SHORT);
	}

	/* 
	 * remove this open element from the
	 * 	component dump table.
	 */
	fddi_cdt_del (p_open);

	FDDI_TRACE("ClsE", p_acs, p_open,0);

	/* unlockl global driver lock */
	unlockl(&fddi_ctl.fddilock);

	return(0);

} /* end fddi_close() */

/*
 * NAME:     fddi_free_rcvq ()
 *
 * FUNCTION: Frees all the packets in this user's rcv queue.
 *
 * EXECUTION ENVIRONMENT: Process thread or interrrupt environment.
 *
 * NOTES:
 *	This routine will set the open ptrs rcv head and tail ptrs
 *	to NULL after freeing the rcv mbufs (if there are any to
 *	be freed).
 *	
 * ROUTINES CALLED: i_disable(), i_enable(), m_freem()
 *
 * RETURNS:
 *		0	- successful
 *
 */
void
fddi_free_rcvq(	fddi_acs_t	*p_acs,		/* ACS ptr */
		fddi_open_t	*p_open)	/* open ptr */
{
	struct mbuf	*p_mbuf;

	FDDI_DBTRACE("CfrB", p_open->p_rcv_q_head, 
		p_open->p_rcv_q_tail, p_open->rcv_cnt);

	while ( p_open->p_rcv_q_head )
	{
		/*
		 * get the mbuf ptr at the head of the queue.
		 * move the head pointer to the next mbuf on the
		 * chain, then free the current head of the rcv queue.
		 */
		p_mbuf = p_open->p_rcv_q_head;
		p_open->p_rcv_q_head = p_mbuf->m_nextpkt;
		m_freem(p_mbuf);
	}

	/*
	 * set the rcv queue tail ptr to NULL and the
	 * rcv packet counter to 0.
	 */
	p_open->p_rcv_q_tail = NULL;
	p_open->rcv_cnt = 0;

	
	FDDI_DBTRACE("CfrE", 0, 0, 0);
	return;

} /* end fddi_free_rcvq() */

