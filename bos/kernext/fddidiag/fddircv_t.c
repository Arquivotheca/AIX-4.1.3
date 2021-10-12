static char sccsid[] = "@(#)91	1.1  src/bos/kernext/fddidiag/fddircv_t.c, diagddfddi, bos411, 9428A410j 11/1/93 11:00:58";
/*
 *   COMPONENT_NAME: DIAGDDFDDI
 *
 *   FUNCTIONS: fddi_init_rcv
 *		fddi_read
 *		fddi_undo_init_rcv
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
#include <sys/malloc.h>
#include <sys/dma.h>
#include <sys/mbuf.h>
#include <sys/errno.h>

extern fddi_ctl_t	fddi_ctl;
/*
 * NAME:     fddi_read
 *
 * FUNCTION: read entry point providing means for receiving data for the user
 *	level interface
 *
 * EXECUTION ENVIRONMENT: called from user process environment only
 *
 * NOTES: return a packet if one is available.  If the queue is empty then if
 *	the user specified NDELAY at open time return without a packet.  If 
 *	NDELAY was not specified then put the user to sleep until a packet
 *	arrives or a close is hit.  If the packet is larger then the buffer
 *	the user gave us then if their is an extension truncate the packet and
 *	return CIO_BUF_OVFLW otherwise free up the packet and return EMSGSIZE
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES: acs and open structures
 *
 * RETURNS:  
 *
 * 	If read successful, also returns indirectly the number of bytes
 * 	read through the updating of uiop->uio_resid by uiomove
 *
 *	If the read_extension parameter is given then the 
 *	following codes	may appear in the status field:
 *
 *		CIO_OK
 *		CIO_BUF_OVFLW
 *
 *	Return codes:
 *
 *	EIO	   - Indicates an error.
 *	EACCES	   - Indicates illegal call from kernel-mode user.
 *	ENODEV 	   - Indicates an invalid minor number was specified
 *	EINTR      - Indicates that a system call was interrupted.
 *	EFAULT     - Indicates that an invalid address was supplied.
 *	ENOCONNECT - Indicates the device is not started.
 *	EMSGSIZE   - Indicates data was too large to fit into the 
 *		     receive buffer and no ext was supplied.
 */


int
fddi_read (
	dev_t		devno,
	struct uio	*uiop,
	int		chan,
	cio_read_ext_t	*p_ext)
{
	register fddi_acs_t	*p_acs;
	register fddi_open_t	*p_open;
	int		rc = 0;
	int		len = 0;
	int		total = 0;
	cio_read_ext_t	ext;
	struct  mbuf	*p_mbuf = NULL;
	struct  mbuf	*p_mtmp;
	short		adap;

	/* 
	 * sanity check the minor number 
	 */
	if ( ((adap = minor(devno)) < 0) || (adap >= FDDI_MAX_MINOR) )
	{
		return(ENODEV);
	}
	if ((p_acs = fddi_ctl.p_acs[adap]) == NULL)
	{
		return(ENODEV);
	}

	if ((p_open = fddi_ctl.p_open_tab [chan]) == NULL)
	{
		return(EINVAL);
	}

	if (p_open->devflag & DKERNEL)
	{
		return (EACCES);
	}

	/*
	 * rcv que is not empty so deque an element for 
	 *	the requested read
	 */
	rc = fddi_rcv_deque (p_acs, p_open, &p_mbuf);

	if ((rc != 0) || (p_mbuf == 0))
	{
		return(rc);
	}

	/* We have data for caller so calculate bytes to be read. */
	p_mtmp = p_mbuf;
	while (p_mtmp)
	{
		total += p_mtmp->m_len;
		p_mtmp = p_mtmp->m_next;
	}

	/* check total bytes with what the user gave use to store it in */
	ext.status = CIO_OK;
	if (total > uiop->uio_resid) 
	{
		/* more bytes to read than the user gave us room for */
		if (p_ext != NULL)
		{
			/* give user the data that will fit and set status */
			total = uiop->uio_resid;
			ext.status = CIO_BUF_OVFLW;
		}
		else
		{
			/* don't give any of the read and return error */
			m_freem (p_mbuf);
			return (EMSGSIZE);
		}
	}
	/* For each mbuf move data to user space */
	p_mtmp = p_mbuf;
	while (total > 0)
	{
		/* only copy what the user supplied buffer will hold */
		len = MIN(p_mtmp->m_len, total);

		rc = uiomove(MTOD(p_mtmp,char *),len,UIO_READ,uiop);
		if (rc)
		{
			/* 
			 * Normalize the failure from uiomove to EFAULT
			 */
			rc = EFAULT;
			break;
		}
		total -= len;
		if (p_mtmp->m_next != NULL)
		{
			p_mtmp = p_mtmp->m_next;
		}
	}
	/* 
	 * Update read_extention if one provided and no other errors 
	 *	except possibly the overflow condition
	 */
	if (p_ext != (cio_read_ext_t *) NULL)
	{
		/* give user the extension */
		rc = copyout (&ext, p_ext, sizeof (p_ext));
		if (rc)
		{
			/* copyout can return EIO, ENOSPC, EFAULT */
			rc = EFAULT;
		}
	}

	/* free all the mbufs */
	m_freem (p_mbuf); 

	/* ok */
	if (ext.status != CIO_OK)
		return(EIO);
	else	
		return (rc);
}


/*
 * NAME: fddi_init_rcv
 *                                                                    
 * FUNCTION: Do all the preparation to receive.  Obtain the resources and 
 *	calculate the the resource allocation to the individual descriptor.
 *                                                                    
 * EXECUTION ENVIRONMENT: called in process environment only
 *                                                                   
 * NOTES: 
 *
 *	Called by fddi_open() on the first open for this adapter
 *	Calls:	m_reg() - to register mbuf usage
 *		xmalloc - to obtain the descriptor's memory
 *	This routine does NOT touch the adapter. 
 *
 * ASSUMPTIONS:
 *	Interrupts are already disabled.
 *	caller has checked the state of machine
 *
 * RECOVERY OPERATION: 
 *
 *	If an error occurs undo everything done in this routine.
 *	fddi_undo_init_rcv() will undo everything this routine does
 *
 * DATA STRUCTURES: 
 *
 *	rcv descriptor array in the open structure
 *
 * RETURNS: 
 *
 *	0 	- ok
 *	ENOMEM	- No memory available for mbufs
 */  

int
fddi_init_rcv (
	fddi_acs_t	*p_acs)
{
	int 		x,i;
	char		*p_tmp;
	fddi_rcv_t	*p_rcvd;
	uint		p_d_addr;
	struct xmem	*p_xmd;
	struct mbreq	mbreq;


	/*
	 * Initialize the rearm values which will stay constant
	 *	through this configuration (used to init the constant
 	 * 	section of a rcv descriptor
	 */
	p_acs->rcv.arm_val.cnt = SWAPSHORT( p_acs->rcv.l_adj_buf);
	p_acs->rcv.arm_val.ctl = SWAPSHORT(FDDI_RCV_CTL_BDV| FDDI_RCV_CTL_IFR);
	p_acs->rcv.arm_val.stat = 0;

	/*
	 * Allocate pinned memory for rcv cache.
	 * NB:
	 *	We use the length of the adjusted buffer size
	 *	to avoid problems with cache inconsistency
	 */
	p_tmp = xmalloc((p_acs->rcv.l_adj_buf * FDDI_MAX_RX_DESC), 
			DMA_L2PSIZE, pinned_heap);
	if (p_tmp == NULL)
	{
		FDDI_TRACE ("Rin1", p_acs, 0, 0);
		return (ENOMEM);
	}
	p_acs->rcv.p_rcv_cache = p_tmp;
	/* 
	 * Get generic xmem descriptor for transmit
	 */
	p_acs->rcv.xmd.aspace_id = XMEM_GLOBAL;
	p_d_addr = p_acs->dds.dma_base_addr;

	/* setup dma */
	d_master(p_acs->dev.dma_channel, DMA_READ | DMA_NOHIDE, 
		p_tmp, p_acs->rcv.l_adj_buf*FDDI_MAX_RX_DESC,
		&p_acs->rcv.xmd, p_d_addr);

	/* Calculate the values of memory locations for each descriptor */
	for (i=0; i<FDDI_MAX_RX_DESC; i++)
	{
		p_acs->rcv.desc[i].offset = i * sizeof(struct fddi_adap)
			+ FDDI_RCV_SHARED_RAM; 
		p_acs->rcv.desc[i].p_buf = p_tmp;
		p_acs->rcv.desc[i].p_d_addr = p_d_addr;
		p_d_addr += p_acs->rcv.l_adj_buf;
		p_tmp += p_acs->rcv.l_adj_buf;

	}

	/* initialize rcv indexes */
	p_acs->rcv.rcvd = 0;

        mbreq.low_mbuf = (FDDI_MAX_RX_DESC + (int)(p_acs->dds.tx_que_sz/2));
        mbreq.low_clust = (FDDI_MAX_RX_DESC +(int)(p_acs->dds.tx_que_sz/2));
        mbreq.initial_mbuf  = 0;
        mbreq.initial_clust = 0;
	FDDI_DBTRACE("add ",mbreq.low_mbuf, mbreq.low_clust,0);
        m_reg(&mbreq);

	return (0);
}

/*
 * NAME: fddi_undo_init_rcv
 *                                                                    
 * FUNCTION: undo the initialzation done in fddi_init_rcv
 *                                                                    
 * EXECUTION ENVIRONMENT: called by process environment only
 *                                                                   
 * NOTES: 
 *
 *	Called by: fddi_close () on the last close for this adapter
 *		   or fddi_open if there is an error and the init must be undone
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 *		0 - successful
 */  

void
fddi_undo_init_rcv (
	fddi_acs_t	*p_acs)
{
	int 		i;
	struct mbreq	mbreq;

	d_complete(p_acs->dev.dma_channel, DMA_READ | DMA_NOHIDE, 
		p_acs->rcv.p_rcv_cache, 
		p_acs->rcv.l_adj_buf*FDDI_MAX_RX_DESC,
		&p_acs->rcv.xmd, p_acs->dds.dma_base_addr);

	xmfree(p_acs->rcv.p_rcv_cache, pinned_heap);

	p_acs->rcv.p_rcv_cache=NULL;

	/* initialize rcv indexes */
	p_acs->rcv.rcvd = 0;

        mbreq.low_mbuf = (FDDI_MAX_RX_DESC + (int)(p_acs->dds.tx_que_sz/2));
        mbreq.low_clust = (FDDI_MAX_RX_DESC +(int)(p_acs->dds.tx_que_sz/2));
        mbreq.initial_mbuf  = 0;
        mbreq.initial_clust = 0;
	FDDI_DBTRACE("del ",mbreq.low_mbuf, mbreq.low_clust,0);
        m_dereg(&mbreq);		/* deregister mbuf usage */

	return;
}

