static char sccsid[] = "@(#)94	1.1  src/bos/kernext/fddidiag/fdditx_t.c, diagddfddi, bos411, 9428A410j 11/1/93 11:01:11";
/*
 *   COMPONENT_NAME: DIAGDDFDDI
 *
 *   FUNCTIONS: fddi_init_tx
 *		fddi_undo_init_tx
 *		fddi_write
 *		get_user_mbufs
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
#include <sys/trchkid.h>	/* for performance hooks */
#include <sys/errno.h>
#include <sys/dma.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/iocc.h>
#include <sys/sleep.h>
#include <sys/uio.h>

/* extern data objects */
extern fddi_ctl_t	fddi_ctl;

/* 
 * statics 
 */
static int get_user_mbufs (
	struct uio 	*uiop,		/* uio struct for user data */
	struct mbuf 	**pp_mbuf,	/* mbuf pointer returned here */
	short		*p_gather_cnt, 	/* number of mbufs returned here */
	fddi_acs_t	*p_acs);

/*
 * NAME:     fddi_write
 *
 * FUNCTION: write entry point providing means for transmitting data
 *
 * EXECUTION ENVIRONMENT: process environment only
 *
 * NOTES:
 *
 *	The flag field of the write_extention parameter may consist
 *	of the following:
 *	
 *	CIO_NOFREE_MBUF	- Requests that the mbuf not be freed after
 *			  transmission. The default action will be to
 *			  free the buffer. User-mode processes will
 *			  cause the device driver to always free the mbuf.
 *
 *	CIO_ACK_TX_DONE - Asks the driver to build a CIO_TX_DONE status
 *			  block. The driver will call the kernel status
 *			  function for kernel mode processes or will queue
 *			  the status block for user-mode processes. The
 *			  default is no CIO_ACK_TX_DONE.
 *
 * 	FDDI_TX_LOOPBACK - The packet is xmitted to the host only; not to the 
 *			  media.
 *
 *	FDDI_TX_PROC_ONLY - Adapter returns an error if the frame is not a SIF
 *			  or PMF frame (ILL bit in the descriptor).  The packet
 *			  is processed by the adapter but not xmitted on the 
 *			  media.
 *
 *	FDDI_TX_PROC_XMIT - As above, but the frame is also xmitted on the media
 *
 *	If NULL is passed instead of a write_extention structure the 
 *	default actions will apply.
 *
 * RECOVERY OPERATION:
 *
 *	'Transmit queue full' will be handled depending on whether the caller
 *	is KERNEL or USER and whether DNDELAY was specified or not.
 *	Under a kernel process with DNDELAY set EAGAIN is returned and 
 *	after the next transmit complete the tx_fn() function will be called.
 *	User processes with DNDELAY will return EAGAIN and CIO_TX_FULL in the 
 *	status field of the write_extension struct.
 *	If DNDELAY is set then a kernel and user processes will 
 *	sleep on the tx_event field of the open structure.
 *
 * DATA STRUCTURES:
 *
 *	Data to be written is put into an mbuf chain. The chain will be
 *	upto FDDI_MAX_GATHERS mbufs. The mbuf->m_nextpkt field is not used and
 *	multiple writes using the m_nextpkt field in one call are not allowed.
 *
 *	The mbuf chain is posted directly to the adapter if there is room.
 *	There is no queuing. 
 *
 * RETURNS:  
 *
 *	If the write_extension parameter is given then the 
 *	following codes	may appear in the status field:
 *
 *		CIO_OK
 *		CIO_TX_FULL
 *		CIO_NOT_STARTED
 *		CIO_NOMBUF
 *
 *	Return codes:
 *
 *	ENODEV 	   - Indicates an invalid minor number was specified
 *	ENETDOWN   - Indicates that the network in down. The device
 *		     driver is unable to process the write request.
 *	ENOCONNECT - Indicates the device is not started.
 *	EAGAIN	   - Indicates that the transmit queue is full.
 *	EINVAL     - Indicates that an invalid parameter was specified.
 *	ENOMEM     - Indicates that the driver was unable to allocate 
 *		     required memory.   
 *	EINTR      - Indicates that a system call was interrupted.
 *	EFAULT     - Indicates that an invalid address was supplied.
 *	ENETUNREACH- Network is not reachable
 *	EIO	   - Indicates an error 
 *
 */

int 
fddi_write (
	dev_t           devno,  	/* major and minor number */
	struct uio      *uiop,  	/* pointer to uio structure */
	chan_t		chan,  		/* channel number */
	cio_write_ext_t *p_ext) 	/* caller's write extension */
{
	short		adap;		/* minor number */
	fddi_acs_t	*p_acs;		/* adapter control struct ptr */
	fddi_open_t	*p_open;	/* open structure */
	short		gather_cnt;	/* number of buffers in chain */
	cio_write_ext_t wr_ext;		/* local write extention struct */
	int		rc;
	struct mbuf	*p_mbuf;

	FDDI_TRACE("WrtB", devno, uiop, p_ext);

	/* get ACS and check minor number */
	if (((adap = minor(devno)) < 0) || (adap >= FDDI_MAX_MINOR) || 
	   ((p_acs = fddi_ctl.p_acs[adap]) == NULL))
	{
		/* bad minor number */
		return (ENODEV);
	}

	/* Get the open structure */
	if ((p_open = fddi_ctl.p_open_tab [chan]) == NULL)
	{
		/* Can't do anything with the p_ext without an OPEN ptr! */
		return (EINVAL);
	}

	/* check for caller supplied write extension */
	if (p_ext)
	{
		/* Get callers write extension */
		rc = MOVEIN (p_open->devflag, p_ext, &wr_ext, sizeof (wr_ext));
		if (rc)
		{
			/* failed getting extension */
			return (EFAULT);
		}
	}

	if (p_open->netid_cnt <= 0)
	{
		/* no writing without an active netid */
		if (p_ext)
		{
			wr_ext.status = CIO_NOT_STARTED;
			MOVEOUT (p_open->devflag, 
					&wr_ext, 
					p_ext, 
					sizeof(p_ext));
			return(EIO);
		}
		return (ENOCONNECT);
	}
		
	/* if not in OPEN or LLC state then do a thorough state check */
	if ((p_acs->dev.state != FDDI_OPEN) && 
		(p_acs->dev.state != FDDI_LLC_DOWN))
	{
		/* check what state and return appropriate error code */
		rc = fddi_chk_state (p_acs, (p_ext) ? &wr_ext : NULL);
		if (rc)
		{
			/* put callers write ext */
			if (p_ext)
			{
				/* status code set in 'chk state' routine */
				MOVEOUT (p_open->devflag, 
						&wr_ext, 
						p_ext, 
						sizeof(p_ext));
			}
			/* state not OPEN so return */
			return (rc);
		}
	}

	/* Get caller's data */
	if (p_open->devflag & DKERNEL)
	{
		/*
		 * Check mbufs supplied by caller, gather_cnt is
		 *	set on return.
		 */
		p_mbuf = (struct mbuf *) (uiop->uio_iov->iov_base);
		rc = fddi_chk_kernel_mbufs (p_mbuf, &gather_cnt, p_acs);
	}
	else
	{
		/* 
		 * Put user data into mbufs setting 'p_mbuf' and
		 *	gather_cnt on return.
		 */
		rc = get_user_mbufs (uiop, &p_mbuf, &gather_cnt, p_acs);
		if ((rc == 0) && p_ext)
		{
			/*
			 * Force USER mode callers to always free mbufs.
			 *	(If we force this here then we save a
			 *	check in the interrupt handler.)
			 */
			wr_ext.flag &= (~CIO_NOFREE_MBUF);
		}
	}

	if (rc)
	{
		if (rc == ENOMEM)
		{
			/* failed getting caller's data */
			if (p_ext)
			{
				/* fill in ext */
				wr_ext.status = CIO_NOMBUF;
				MOVEOUT (p_open->devflag, 
					&wr_ext, 
					p_ext, 
					sizeof(p_ext));
				return(EIO);
			}
		}
	
		if (rc == EINVAL)
		{
			if (p_ext)
			{
				wr_ext.status = CIO_BAD_RANGE;
				MOVEOUT (p_open->devflag, 
					&wr_ext, 
					p_ext, 
					sizeof(p_ext));
				return(EIO);
			}
		}

		return(rc);
	}

	/* 
	 *  call nonpageable code to queue/send the request
	 * NOTE: fddi_tx will return a negative number if a state change has
 	 * occurred.
	 */
	wr_ext.status = CIO_OK;

	rc = fddi_tx (p_acs, 
			p_open, 
			(p_ext) ? &wr_ext : NULL, 
			p_mbuf, 
			gather_cnt);

	if (rc < 0)
	{
		/* state change occurred: recheck states/set ext */
		rc = fddi_chk_state (p_acs, (p_ext) ? &wr_ext : NULL);

		/* put caller's write ext */
		if (p_ext)
		{
			MOVEOUT (p_open->devflag, 
				&wr_ext, 
				p_ext, 
				sizeof(p_ext));
			return(EIO);
		}
	}
	else if (wr_ext.status != CIO_OK)
	{
		MOVEOUT(p_open->devflag, &wr_ext, p_ext,sizeof(p_ext));
	}

	FDDI_TRACE("WrtE", p_acs, p_open, rc);
	/* finished */
	return (rc);
}
/*
 * NAME: fddi_init_tx
 *                                                                    
 * FUNCTION: initialize the transmit data objects
 *                                                                    
 * EXECUTION ENVIRONMENT: process environment only
 *                                                                   
 * NOTES: 
 *
 *	Called by: fddi_open on the first open for this adapter.
 *
 *	This routine does nothing to the adapter.
 *	It initializes the host descriptor data structure.
 *	This routine assumes exclusive access to the acs
 *	it is working in. This is achieved by the caller doing:
 *
 *		ACS lock for Process serialization.
 *		disabled adapter for SLIH serialization
 *
 * 	Setup dma for the 'smallframe' cache used to transmit short frames 
 *	(by convention use the dma region at the end of tx region)
 *	!!! must allocate tx before rcv and cache at the end
 * 
 *	Tx watchdog timer is initialized here, cleared in the 'init undo'
 *	routine, started in the 'send frame' routine, and stopped in the 
 *	'tx completion routine'.
 *
 * RECOVERY OPERATION: 
 *
 *	As errors are encounters undo anything done in this routine.
 *	To undo everything done in this routine after it is successful
 *	call fddi_undo_init_tx ().
 *	
 * DATA STRUCTURES: 
 *
 *	The tx host descriptors are used for setting up all transmit
 *	activity with the adapter. The descriptors make up a circular
 *	queue managed by three variables. Lastly, a 'cache' is used to
 *	transfer small frames to the adapter.
 *	The 'cache' is pinned data (p_tx_smallframe) used to
 *	hold 'small frames' for transmission. This cache will be partitioned
 *	into FDDI_MAX_TX_DESC sections. Each section can hold a small
 *	frame to transmit. 
 *	Dma setup to this cache is done one time in this routine.
 *
 *
 * RETURNS: 
 *	0	- successful
 *	ENOMEM	- unable to allocate required resources
 */  

int
fddi_init_tx (
	fddi_acs_t	*p_acs)
{
	fddi_tx_t	*p_host;	/* ptr to descriptor */
	char		*p_sf;		/* ptr to smallframe cache */
	int		i;		/* index */

	FDDI_TRACE ("TinB", p_acs, 0, 0);
	/*
	 * Allocate pinned memory for the tx 'smallframe' cache 
	 */
	p_acs->tx.p_sf_cache = xmalloc(FDDI_SF_CACHE, DMA_L2PSIZE, pinned_heap);
	if (p_acs->tx.p_sf_cache == NULL)
	{
		/* fail malloc */
		FDDI_TRACE ("Tin1", p_acs, 0, 0);
		return (ENOMEM);
	}

	/* 
	 * initialize the user tx que to the configured size.  This is the 
	 * tx que to supplement the que on the adapter 
	 */
	FDDI_DBTRACE("Qsiz", p_acs->dds.tx_que_sz,
			p_acs->dds.tx_que_sz*(sizeof(fddi_tx_que_t)),0);

	p_acs->tx.p_tx_que = xmalloc((p_acs->dds.tx_que_sz * 
			sizeof(fddi_tx_que_t)), FDDI_WORDSHIFT, pinned_heap);

	if (p_acs->tx.p_tx_que == NULL)
	{
		/* fail malloc */
		xmfree(p_acs->tx.p_sf_cache, pinned_heap);
		FDDI_TRACE ("Tin2", p_acs, p_acs->dds.tx_que_sz, 0);
		return (ENOMEM);
	}

	bzero(p_acs->tx.p_tx_que,
		(sizeof(fddi_tx_que_t) * p_acs->dds.tx_que_sz));

	fddi_cdt_add ("tx_que", p_acs->tx.p_tx_que, 
		(sizeof(fddi_tx_que_t) * p_acs->dds.tx_que_sz));

	p_acs->tx.tq_in = 0;
	p_acs->tx.tq_out = 0;
	p_acs->tx.tq_cnt = 0;


	fddi_cdt_add ("sf_cache", p_acs->tx.p_sf_cache, FDDI_SF_CACHE);


	/* Setup generic xmem descriptor */
	p_acs->tx.xmd.aspace_id = XMEM_GLOBAL;

	/* Do one time d_master of 'smallframe' cache sections */
	d_master (p_acs->dev.dma_channel, 
		DMA_WRITE_ONLY, 
		p_acs->tx.p_sf_cache,
		(size_t) FDDI_SF_CACHE, 
		&p_acs->tx.xmd,
		(char *) p_acs->tx.p_d_sf);
	/* 
	 * Setup each tx host descriptor 
	 *	Initialize descriptor
	 *	setup dma address
	 *	setup offset value (never changes)
	 *	setup cache buffer and dma window for smallframes
	 */
	p_host = &(p_acs->tx.desc[0]);
	for (i = 0; i < FDDI_MAX_TX_DESC; i++, p_host++)
	{
		bzero (p_host, sizeof (p_host));
		p_host->p_d_addr = p_acs->tx.p_d_base + (i*DMA_PSIZE);
		p_host->offset = (i) * sizeof (fddi_adap_t);
		p_host->p_sf = p_acs->tx.p_sf_cache + (i*FDDI_SF_BUFSIZ);
		p_host->p_d_sf = p_acs->tx.p_d_sf + (i*FDDI_SF_BUFSIZ);
	}
	/* 
	 * initialize host tx queue variables 
	 *	and sleep cell
	 */
	p_acs->tx.in_use = 0;
	p_acs->tx.nxt_req = 0;
	p_acs->tx.nxt_cmplt = 0;
	p_acs->tx.event = EVENT_NULL;

	/* 
	 * initialize tx watchdog timer 
	 */
	p_acs->tx.wdt.restart = FDDI_TX_WDT_RESTART;
	p_acs->tx.wdt.func = fddi_tx_to;
	w_init (&p_acs->tx.wdt);

	FDDI_TRACE ("TinE", p_acs, 0, 0);
	return (0);
}
/*
 * NAME: fddi_undo_init_tx
 *                                                                    
 * FUNCTION:  reverse what was done in fddi_init_tx
 *                                                                    
 * EXECUTION ENVIRONMENT: process environment
 *                                                                   
 * NOTES: 
 *
 *	Assume: 
 *		The adapter is disabled.
 *		No tx's in progress.
 * 		No tx acknowledgements are pending.
 *
 *	Called by: fddi_close on the last close for this adapter
 *	or after a successful call to fddi_init_tx() and a subsequent
 *	and immediate failure of some other init process.
 *
 * RECOVERY OPERATION: 
 *
 *	We have already encountered an error or we are on our last 
 *	close. So just log errors.
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 */  
void
fddi_undo_init_tx (
	fddi_acs_t	*p_acs)
{
	int		i;
	int		rc;

	FDDI_TRACE ("TuiB", p_acs, p_acs->dds.tx_que_sz, 0);

	/* remove tx timer from system */
	w_clear (&(p_acs->tx.wdt));

	fddi_cdt_del (p_acs->tx.p_sf_cache);

	fddi_cdt_del (p_acs->tx.p_tx_que);

	/* 
	 * reset dma
	 */
	rc = d_complete (
		p_acs->dev.dma_channel, 
		DMA_WRITE_ONLY, 
		p_acs->tx.p_sf_cache,
		FDDI_SF_BUFSIZ, 
		&p_acs->tx.xmd,
		p_acs->tx.p_d_sf);
	if (rc != DMA_SUCC)
	{
		/* 
		 * log this dma error : not serious because we
		 *	are shutdown.
		 */
		FDDI_TRACE ("Tui1", p_acs, rc, 0);
		ASSERT(0);
	}
	/*
	 * Now, free the cache
	 */
	xmfree (p_acs->tx.p_sf_cache, pinned_heap);

	xmfree (p_acs->tx.p_tx_que, pinned_heap);

	FDDI_TRACE ("TuiE", p_acs, 0, 0);

	/* ok */
	return ;
}
/*
 * NAME: get_user_mbufs
 *
 * FUNCTION: copy user data into an mbuf
 *
 * EXECUTION ENVIORNMENT: called in a USER process
 *
 * NOTES:  
 *
 *	Copy Data from USER space to mbufs. This routine
 *	supports a scatter of FDDI_MAX_GATHERS (that is data
 *	can end up in FDDI_MAX_GATHERS mbufs chained together
 *	with the m_next field).
 *	In addition, it checks the size of the request and
 *	leaves room for the mandatory PAD characters that preceed
 *	the frame.
 *
 *	Called by: fddi_write()
 *	Calls to : m_get(), uiomove()
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *
 *	0  - if successful and the:
 *		- mbuf chain via pp_mbuf
 *		- number of mbufs via p_gather_cnt
 *	
 *	ENOMEN	- if the needed mbufs are not available
 *	EFAULT	- if a uiomove fails
 *	EINVAL	- if the request is outside the size of a 
 *		  legal FDDI frame length
 *
 *	Also, on error :
 *		- pp_mbuf will be NULL and
 *		- p_gather_cnt will indicate how far the routine got
 */
static int
get_user_mbufs (
	struct uio 	*uiop,		/* uio struct for user data */
	struct mbuf 	**pp_mbuf,	/* mbuf pointer returned here */
	short		*p_gather_cnt,	/* number of mbufs returned here */
	fddi_acs_t	*p_acs)
{
	struct mbuf 	*p_mbuf;	/* per buffer pointer */
	struct mbuf 	*p_tail;	/* to help chain mbufs */
	int		l_frame;	/* total frame length */
	int 		len;		/* len to go into each mbuf */
	int		rc;		/* return code */

	/* initialize */
	*(pp_mbuf) = NULL;
	*(p_gather_cnt) = 0;
	rc = 0;
	l_frame = uiop->uio_resid;

	/* 
	 * scatter USER data into mbuf(s) 
	 *	The 'len' is the remaining number of bytes
	 *	that need to be uiomoved.
	 */
	while (len = uiop->uio_resid)
	{
		/* 
		 * (FDDI_CLBYTES is guarenteed to be > than MLEN)
		 */
		if (len > MLEN)
		{
			/* 
			 * get a cluster
			 */
			p_mbuf = m_getclust (M_WAIT, MT_DATA);
			if (len > FDDI_CLBYTES)
			{
				/*
				 * move in the first FDDI_CLBYTES first
				 *	we will need another mbuf for the rest
				 */
				len = FDDI_CLBYTES;
			}
		}
		else
		{
			/* 
			 * get a regular mbuf 
			 */
			p_mbuf = m_get (M_WAIT, MT_DATA);
		}
		if (p_mbuf == NULL)
		{
			/* failed getting an mbuf */
			rc = ENOMEM;
			break;
		}
		/* 
		 * move 'len' bytes from USER space to mbuf 
		 * 	(uiop->uio_resid is decremented by len) 
		 */
		if (uiomove (MTOD (p_mbuf, caddr_t), len, UIO_WRITE, uiop))
		{
			
			/* free current mbuf */
			m_free (p_mbuf);

			/* normalize the error */
			rc = EFAULT;

			break;
		}
		/* add to chain */
		p_mbuf->m_len = len;
		if (*pp_mbuf == NULL)
		{
			/* first time through */
			*pp_mbuf = p_mbuf;

			/*
			 * now that we have a ptr to the start of the frame
			 * check for legal frame: correct type and length
			 */
			rc = fddi_chk_frame (p_mbuf, l_frame, p_acs);
			if (rc)
			{
				break;
			}
		}
		else
		{
			p_tail->m_next = p_mbuf;
			p_mbuf->m_next = NULL;
		}
		/* save previous mbuf for next link in chain */
		p_tail = p_mbuf;

		/* increment number of mbufs in frame */
		(*p_gather_cnt)++;
	}
	/* if error and mbufs allocated then free mbufs */
	if ((rc != 0) && (*pp_mbuf != NULL))
	{
		/* free mbuf chain */
		m_freem (*pp_mbuf);
		*pp_mbuf = NULL;
	}

	/* 
	 * zero out the offset because we used uiomove() 
	 */
	uiop->uio_offset = 0;

	if (*p_gather_cnt == 0)
	{
		if (rc == 0)
			rc = EINVAL;
	}
	return (rc);
}
