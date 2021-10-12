static char sccsid[] = "@(#)79	1.2  src/bos/kernext/dmpa/dmpa_xmit.c, sysxdmpa, bos411, 9439B411a 9/28/94 17:07:42";
/*
 *   COMPONENT_NAME: (SYSXDMPA) MP/A DIAGNOSTICS DEVICE DRIVER
 *
 *   FUNCTIONS: mpawrite
 *		write_block
 *		write_get_user
 *		
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

/*
* COMPONENT_NAME: (sysx_diag_MPA) Multiprotocol Single Port Device Driver
*
*  FUNCTIONS:   mpawrite
*
*  ORIGINS: 27
*
*  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
*  combined with the aggregated modules for this product)
*                   SOURCE MATERIALS
*  (C) COPYRIGHT International Business Machines Corp. 1990
*  All Rights Reserved
*
*  US Government Users Restricted Rights - Use, duplication or
*  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*
*
*--------------------------------------------------------------------------
*/

#include <net/spl.h>
#include <sys/adspace.h>
#include <errno.h>
#include <sys/device.h>
#include <sys/dma.h>
#include <sys/ioacc.h>
#include <sys/mbuf.h>
#include <sys/dmpauser.h>
#include <sys/dmpadd.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/uio.h>
#include <sys/xmem.h>
#include <sys/trchkid.h>
#include <sys/ddtrace.h>
#include <sys/except.h>
#include <sys/errids.h>


/*-------------------------  M P Q W R I T E  --------------------------*/
/*                                                                      */
/*  NAME: mpawrite                                                      */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Provides the write interface to the MPA device for both        */
/*      user level and kernel level transmits.                          */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Mpqwrite is part of a loadable device driver which is           */
/*      reentrant and always pinned.  This routine cannot be called     */
/*      from offlevel or interrupt level, it can only be called from    */
/*      the process environment.                                        */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*                                                                      */
/*  RETURNS:                                                            */
/*      0       Write succeeded.                                        */
/*      n       Error number for error that occurred.                   */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED: que_command,                            */
/*                                                                      */
/*----------------------------------------------------------------------*/
int
mpawrite (
	dev_t           dev,                  /* device number */
	struct uio      *uiop,                  /* user I/O structure */
	int             chan,                   /* channel (always zero) */
	t_write_ext     *extptr)                /* write extension */
{
	struct acb      *acb;                 /* ptr to adap. ctrl block */
	t_write_ext     wr_ext;                 /* write extension */
	xmit_elem_t *xmitp;
	dma_elem_t *dmap;
	dma_elem_t *tmp_dmap;
	struct mbuf *mbufp;
	int rc = 0;
	int xfer_len;
	int xfer_off;
	int fpl;
	int spl;
	caddr_t xfer_addr;
	int i;


	DDHKWD4(HKWD_DD_MPADD, DD_ENTRY_WRITE, 0, dev, chan,
		uiop->uio_iov->iov_base, uiop->uio_resid);

	if ( ((acb = get_acb(minor(dev))) == NULL) ||
		!(OPENP.op_flags & OP_OPENED)      ||
		!(acb->flags & STARTED_CIO)          ) {
		DDHKWD2(HKWD_DD_MPADD, DD_EXIT_WRITE, ENXIO, dev,0xB1);
		return ENXIO;
	}

	if ((OPENP.op_mode&DWRITE) == 0) {
	   DDHKWD2(HKWD_DD_MPADD, DD_EXIT_WRITE, ENXIO, dev,0xB2);
	   return EACCES;
	}

	/* if it is a kernel process uio_resid is 0 */
	if (((OPENP.op_mode & DKERNEL) == 0) &&  (uiop->uio_resid == 0)) {
		DDHKWD2(HKWD_DD_MPADD, DD_EXIT_WRITE, 0, dev,0xB3);
		return 0;
	}
	if(acb->flags&MPADEAD) {
		DDHKWD2(HKWD_DD_MPADD, DD_EXIT_WRITE, ENODEV, dev,0xB4);
		return ENODEV;
	}
	/*
	** handle the extension parameter
	*/
	if (extptr == NULL) {
		/*
		** build our own with defaults
		*/
		wr_ext.cio_ext.flag = 0;        /* don't acknowledge, do free mbuf */
		wr_ext.cio_ext.write_id = 0;    /* doesn't matter */
		wr_ext.cio_ext.netid = 0;      /* default netid for this write */

	} else {
		/*
		** get the caller-provided extension
		*/
		rc = COPYIN(OPENP.op_mode , extptr, &wr_ext, sizeof(wr_ext));
		if (rc) {
			DDHKWD2(HKWD_DD_MPADD, DD_EXIT_WRITE, EFAULT, dev,0xB5);
			return EFAULT;
		}
		if (!(OPENP.op_mode & DKERNEL)) {
			/* always free user mode mbuf */
			wr_ext.cio_ext.flag &= ~CIO_NOFREE_MBUF;
		}
	}

	DISABLE_INTERRUPTS(fpl);

	/*
	** First, get the data into mbufs
	*/
	if (OPENP.op_mode&DKERNEL) {
		/*
		** this is a kernel process,
		** the uio pointer is an mbuf pointer
		*/
		mbufp = (struct mbuf *)(uiop->uio_iov->iov_base);
	} else {
		/*
		** the caller is a user process
		** get the data into an mbuf chain
		*/
		mbufp = NULL;
		while ((rc = write_get_user(acb, uiop, &mbufp)) == ENOMEM) {
			/* Couldn't get an mbuf or cluster... */
			if (rc = write_block(acb, uiop, &acb->mbuf_event)) {
				if (mbufp) {
					DISABLE_INTERRUPTS(spl);
					m_freem(mbufp);
					mbufp = NULL;
					if( acb->mbuf_event != EVENT_NULL )
						e_wakeup(&acb->mbuf_event);
					ENABLE_INTERRUPTS(spl);
				}
				DDHKWD2(HKWD_DD_MPADD, DD_EXIT_WRITE, rc, dev,0xB6);
				ENABLE_INTERRUPTS(fpl);
				return rc;
			}
		}
		if (rc) {
			/*
			** uiomove() failed when called from
			** write_get_user(), so return
			*/
			DDHKWD2(HKWD_DD_MPADD, DD_EXIT_WRITE, rc, dev,0xB7);
			ENABLE_INTERRUPTS(fpl);
			return rc;
		}
	}
	/*
	** Make sure the request length is valid
	*/
	if (mbufp->m_len > MAX_FRAME_SIZE ) {
		if ((OPENP.op_mode & DKERNEL) == 0 && mbufp) {
			DISABLE_INTERRUPTS(spl);
			m_freem(mbufp);
			mbufp = NULL;
			if (acb->mbuf_event != EVENT_NULL)
				e_wakeup(&acb->mbuf_event);
			ENABLE_INTERRUPTS(spl);
		}
		DDHKWD2(HKWD_DD_MPADD, DD_EXIT_WRITE, EINVAL, dev,0xB8);
		ENABLE_INTERRUPTS(fpl);
		return EINVAL;
	}


	/*
	** Now, get a transmit element
	*/
	xmitp = acb->xmit_free;
	if(xmitp == NULL) {
		if ((OPENP.op_mode&DKERNEL) == 0 && mbufp) {
			DISABLE_INTERRUPTS(spl);
			m_freem(mbufp);
			mbufp = NULL;
			if (acb->mbuf_event != EVENT_NULL)
				e_wakeup(&acb->mbuf_event);
			ENABLE_INTERRUPTS(spl);
		}
		wr_ext.cio_ext.status = (ulong)CIO_TX_FULL;
		if (extptr != NULL)
			(void)COPYOUT(OPENP.op_mode, &wr_ext, extptr,
				sizeof(wr_ext));
		DDHKWD2(HKWD_DD_MPADD, DD_EXIT_WRITE, ENOMEM, dev,0xB9);
		ENABLE_INTERRUPTS(fpl);
		return ENOMEM;
	}
	acb->xmit_free=xmitp->xm_next;
	bzero(xmitp,sizeof(xmit_elem_t));

	/*
	 * Put this element on the active xmit q for this adapter
	*/
	if(acb->act_xmit_head==NULL) {  /* its first one */
	    acb->act_xmit_head=xmitp;
	    acb->act_xmit_tail=xmitp;
	}
	else {        /* its going on the end of a chain */
	    acb->act_xmit_tail->xm_next=xmitp;
	    acb->act_xmit_tail=xmitp;
	}

	/*
	** Tie the mbuf to the transmit pointer.
	*/
	xmitp->xm_mbuf = mbufp;

	/*
	** We have a valid transmit element
	** Now, we initialize the transmit element, and if using dma
	** we get a dma_element and call dma_request to try to
	** start the dma and kick off the xmit on the adapter.
	*/
	xmitp->xm_length = mbufp->m_len;
	xmitp->xm_netid = wr_ext.cio_ext.netid;
	xmitp->xm_ack = wr_ext.cio_ext.flag;
	xmitp->xm_writeid = wr_ext.cio_ext.write_id;
	xmitp->xm_state |= XM_ACTIVE;

	if(acb->flags&PIO_MODE) {

	    /* using PIO so just start xmit command on adapter */
	    xmitp->xm_flags |= USE_PIO;
	    xmitp->xm_data = MTOD(mbufp, caddr_t);

	    /* send MPA xmit command */
	    acb->cmd_parms.cmd=XMIT_CMD;
	    acb->cmd_parms.parm[0]=xmitp->xm_length;
	    acb->cmd_parms.parm[1]=(xmitp->xm_length>>8);
	    if(acb->state.oper_mode_8273&SET_BUFFERED_MODE) {
	       acb->cmd_parms.parm[2]=wr_ext.adr;
	       acb->cmd_parms.parm[3]=wr_ext.cntl;
	       acb->cmd_parms.parm_count=4;
	    }
	    else acb->cmd_parms.parm_count=2;
	    if( (rc=que_command(acb)) ) {
		    free_xmit_elem(acb, xmitp);
		    DDHKWD2(HKWD_DD_MPADD, DD_EXIT_WRITE, rc, dev,0xBA);
		    ENABLE_INTERRUPTS(fpl);
		    return rc;
	    }
	}
	else {     /* using DMA setup dma element and start dma */
	    /*
	    ** Now, get a dma element
	    */
	    dmap = acb->dma_free;
	    if(dmap == NULL) {
		 if (rc = write_block(acb, uiop, &acb->dmabuf_event)) {
			 free_xmit_elem(acb,xmitp);
			 wr_ext.cio_ext.status = (ulong)CIO_TX_FULL;
			 if (extptr != NULL)
				 (void)COPYOUT(OPENP.op_mode, &wr_ext, extptr,
					 sizeof(wr_ext));
			 DDHKWD2(HKWD_DD_MPADD, DD_EXIT_WRITE, rc, dev,0xBB);
			 ENABLE_INTERRUPTS(fpl);
			 return rc;
		 }
	    }
	    acb->dma_free=dmap->dm_next;
	    bzero(dmap,sizeof(dma_elem_t));

	    /* Now init the dma element */

	    dmap->p.xmit_ptr = xmitp;
	    dmap->dm_buffer = MTOD(mbufp,caddr_t);
	    dmap->dm_length = mbufp->m_len;
	    dmap->dm_xmem = M_XMEMD(mbufp);
	    dmap->dm_req_type = DM_XMIT;
	    dmap->dm_state = DM_READY;
	    dmap->dm_flags = DMA_NOHIDE;
	    dmap->adr      = wr_ext.adr;
	    dmap->cntl     = wr_ext.cntl;

	    /*
	    **  Before I can set up this xmit dma, I must disable the
	    **  Outstanding recv and d_complete the read d_slave call
	    **  even though the read has not yet occured. I will leave
	    **  the DM_RECV dma element off the q and hold it.
	    **  Then put it back on when there are no more xmits on the q.
	    **  We don't stop recv for PIO xfers.
	    */
	    if ( (rc = stoprecv(acb)) ) {
		  free_dma_elem(acb,dmap);
		  free_xmit_elem(acb,xmitp);
		  DDHKWD2(HKWD_DD_MPADD, DD_EXIT_WRITE, rc, dev,0xBC);
		  ENABLE_INTERRUPTS(fpl);
		  return rc;
	    }

	    if ( (rc = dma_request(acb,dmap)) ) {
		    free_dma_elem(acb, dmap);
		    free_xmit_elem(acb, xmitp);
		    DDHKWD2(HKWD_DD_MPADD, DD_EXIT_WRITE, rc, dev,0xBD);
		    ENABLE_INTERRUPTS(fpl);
		    return rc;
	    }
	}
	++acb->stats.ds.xmit_sent;

	wr_ext.cio_ext.status = (ulong)CIO_OK;
	if (extptr != NULL) {
		(void)COPYOUT(OPENP.op_mode, &wr_ext, extptr,sizeof(wr_ext));
	}
	ENABLE_INTERRUPTS(fpl);

	DDHKWD2(HKWD_DD_MPADD, DD_EXIT_WRITE, 0, dev,0);
	return( 0 );
}
/**************************************************************************
**
** NAME:        write_block
**
** FUNCTION:
**
** EXECUTION ENVIRONMENT:
**
** NOTES:
**
** RETURNS:
**
*****************************************************************************/
int
write_block (
	struct acb *acb,
	struct uio *uiop,
	int *eventp)
{
	int spl;

	/*
	** action will depend on whether DNDELAY is set
	*/
	if (uiop->uio_fmode & DNDELAY) {
		if (OPENP.op_mode  & DKERNEL) {
			/*
			** set up flags so xmt_fn gets called
			** when conditions change
			*/
			DISABLE_INTERRUPTS(spl);
			OPENP.op_flags |= XMIT_OWED;
			ENABLE_INTERRUPTS(spl);
		}
		return EAGAIN;
	}
	/*
	** block by sleeping
	*/
	if (SLEEP(eventp) != EVENT_SUCC)
		return EINTR;
	return 0;
} /* write_block() */
/*****************************************************************************
**
** NAME:        write_get_user
**
** FUNCTION:    get the user process's data into an mbuf chain
**
** EXECUTION ENVIRONMENT: process thread only
**
** NOTES:
**
** RETURNS:  0 or errno
**
*****************************************************************************/
static int
write_get_user(
	struct acb *acb,             /* pointer to dds structure */
	struct uio *uiop,               /* pointer to uio structure */
	struct mbuf **mbufph)           /* addr for returning mbuf chain ptr */
{
	struct mbuf *mbufp;             /* current one */
	struct mbuf *mbufpprev;         /* previous one */
	int bytes_to_move;              /* amount to write */
	int rc;

	*mbufph = NULL;
	while ((bytes_to_move = uiop->uio_resid) > 0) {
		/*
		** get an mbuf
		*/
		mbufp = m_get(M_WAIT, MT_DATA);
		if (mbufp == NULL) {
		    return (ENOMEM);
		}
		if (*mbufph == NULL)            /* this is first time */
			/* caller gets beginning of chain */
			*mbufph = mbufp;
		else
			/* link this one to previous one */
			mbufpprev->m_next = mbufp;
		mbufpprev = mbufp;      /* save for the next time thru loop */
		if( bytes_to_move > MLEN ) {
			rc = m_clget(mbufp);
			if( rc == 0 ) {
				return (ENOMEM);
			}
			mbufp->m_len = CLBYTES; /* set length to 4096 as */							/* workaround for m_clgetm */
						/* problem (defect 163915) */	
			if (bytes_to_move > CLBYTES)
				/* we'll need another mbuf in chain */
				bytes_to_move = CLBYTES;
		}
		/*
		** move the data into the mbuf (uiop->uio_resid will
		** be decremented)
		*/
		if (uiomove(MTOD(mbufp, uchar *), bytes_to_move,
			 UIO_WRITE, uiop)) {
			return EFAULT;
		}
		/*
		** now change the mbuf length to reflect the actual data size
		*/
		mbufp->m_len = bytes_to_move;
	}
	return 0;
} /* write_get_user() */




