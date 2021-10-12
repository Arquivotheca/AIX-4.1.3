static char sccsid[] = "@(#)70        1.1  src/bos/kernext/dmpa/dmpa_close.c, sysxdmpa, bos411, 9428A410j 4/30/93 12:48:53";
/*
 *   COMPONENT_NAME: (SYSXDMPA) MP/A DIAGNOSTICS DEVICE DRIVER
 *
 *   FUNCTIONS: free_active_q
 *		free_all_resources
 *		mpaclose
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

#include <errno.h>
#include <sys/adspace.h>
#include <sys/device.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/ioctl.h>
#include <sys/malloc.h>
#include <sys/timer.h>
#include <sys/sleep.h>
#include <sys/dmpauser.h>
#include <sys/dmpadd.h>
#include <sys/sysmacros.h>
#include <sys/trchkid.h>
#include <sys/ddtrace.h>


/*
 * NAME: mpaclose
 *
 * FUNCTION: Called to close a single port of the multiprotocol
 *           adaptor.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Preemptable        : Yes
 *      VMM Critical Region: Yes
 *      Runs on Fixed Stack: Yes
 *      May Page Fault     : No
 *
 * (NOTES:) More detailed description of the function, down to
 *      what bits / data structures, etc it manipulates.
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *      software error recovery.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */

int mpaclose ( dev_t            dev,
		int             chan,
		int             ext )

{
	int             spl;
	struct acb      *acb;         /* pointer to adapter control block */

	/* log a trace hook */
	DDHKWD5 (HKWD_DD_MPADD, DD_ENTRY_CLOSE,0,dev,chan,0,0,0);


	if ((acb = get_acb(minor(dev))) == NULL) {
		DDHKWD1(HKWD_DD_MPADD, DD_EXIT_CLOSE, ENODEV, dev);
		return ENODEV;
	}

	DISABLE_INTERRUPTS(spl);

	if (lockl(&acb->adap_lock, LOCK_SIGRET) != LOCK_SUCC) {
		DDHKWD1(HKWD_DD_MPADD, DD_EXIT_IOCTL, EINTR, dev);
		return EINTR;
	}



	/* now we follow down to remove the select queue data structure */
	/* the channel information data structure and null out the      */
	/* pointer in the dds to the channel info ds                    */

	if /* device was opened by a kernel llc */
	( OPENP.op_mode & DKERNEL )
	{
		OPENP.mpa_kopen.rx_fn = NULL;
		OPENP.mpa_kopen.stat_fn = NULL;
		OPENP.mpa_kopen.tx_fn = NULL;
		OPENP.mpa_kopen.open_id = 0;
	}

	OPENP.op_mode = 0;      /* remove device flags */


	/* First disable interrupts from the adapter. */

	acb->flags &= ~STARTED_CIO;

	acb->flags &= ~OPEN_DIAG;
	if (acb->flags & MPA_CE_OPEN) {
		acb->flags &= ~MPA_CE_OPEN;
	}

	unlockl(&acb->adap_lock);
	ENABLE_INTERRUPTS(spl);
	DDHKWD5(HKWD_DD_MPADD,DD_EXIT_CLOSE,0,dev,chan,0,0,0);

	return(0);
}

void
free_all_resources (struct acb *acb)
{
  irpt_elem_t   *irptp;
  irpt_elem_t   *next_ip;
  recv_elem_t   *recvp;
  recv_elem_t   *next_rp;
  xmit_elem_t   *xmitp;
  xmit_elem_t   *next_xp;
  dma_elem_t    *dmap;
  dma_elem_t    *next_dp;
  stat_elem_t   *statp;
  stat_elem_t   *next_sp;
  int           one_started = 0;
  int           spl;


  /*
  ** prior to freeing all the resources I want to make sure I get
  ** no more irpts from the adapter, so I am shuting it down here.
  */
  shutdown_adapter(acb);

  DISABLE_INTERRUPTS(spl);

  /* free all dma elements on active list */
  if ( (dmap=acb->act_dma_head) != NULL) {
     while (dmap) {
	next_dp = dmap->dm_next;
	/* if DMA_STARTED already do d_complete */

	if(dmap->dm_state == DM_STARTED && !one_started) {
	   one_started= 1;
	   d_complete(acb->dma_channel, dmap->dm_flags,
		dmap->dm_buffer, dmap->dm_length,
		dmap->dm_xmem, NULL);
	}
	KFREE(dmap);
	dmap = next_dp;
     }
     acb->act_dma_head = acb->act_dma_tail = NULL;
  }

  /*
  ** Now that I know there are no dmas active d_clear dma channel.
  */
  if (acb->flags & MPADMACHAN) {
	  d_clear(acb->dma_channel);
  }
  acb->flags &= ~MPADMACHAN;


  /* free all dma elements on free list */
  if ( (dmap=acb->dma_free) != NULL) {
     while (dmap) {
	next_dp = dmap->dm_next;
	KFREE(dmap);
	dmap = next_dp;
     }
     acb->dma_free = NULL;
  }


  /* free all irpt elements on active list */
  if ( (irptp=acb->act_irpt_head) != NULL) {
     while (irptp) {
	next_ip = irptp->ip_next;
	KFREE(irptp);
	irptp = next_ip;
     }
     acb->act_irpt_head = acb->act_irpt_tail = NULL;
  }

  /*
  ** Now that I know there are no irpts comming i_clear irpt struct(s)
  */
  if (acb->flags & MPAIINSTALL3) {
	 i_clear(&acb->caih_struct);
  }
  acb->flags &= ~MPAIINSTALL3;

  /* free all irpt elements on free list */
  if ( (irptp=acb->irpt_free) != NULL) {
     while (irptp) {
	next_ip = irptp->ip_next;
	KFREE(irptp);
	irptp = next_ip;
     }
     acb->irpt_free = NULL;
  }

  /* free all xmit elements on active list */
  if ( (xmitp=acb->act_xmit_head) != NULL) {
     while (xmitp) {
	next_xp = xmitp->xm_next;
	free_xmit_elem(acb,xmitp);
	xmitp = next_xp;
     }
     acb->act_xmit_head = acb->act_xmit_tail = NULL;
  }

  /* free all xmit elements on free list */
  if ( (xmitp=acb->xmit_free) != NULL) {
     while (xmitp) {
	next_xp = xmitp->xm_next;
	KFREE(xmitp);
	xmitp = next_xp;
     }
     acb->xmit_free = NULL;
  }

  /* free all recv elements on active list */
  if ( (recvp=acb->act_recv_head) != NULL) {
     while (recvp) {
	next_rp = recvp->rc_next;
	/*
	** Free any mbufs associated with this element
	*/
	if (recvp->rc_mbuf_head && (OPENP.op_mode & DKERNEL) == 0) {
		m_freem(recvp->rc_mbuf_head);
		recvp->rc_mbuf_head = NULL;
	}
	KFREE(recvp);
	recvp = next_rp;
     }
     acb->act_recv_head = acb->act_recv_tail = NULL;
  }

  /* free all recv elements on free list */
  if ( (recvp=acb->recv_free) != NULL) {
     while (recvp) {
	next_rp = recvp->rc_next;
	KFREE(recvp);
	recvp = next_rp;
     }
     acb->recv_free = NULL;
  }


  /* free all stat elements on active list */
  if ( (statp=acb->act_stat_head) != NULL) {
     while (statp) {
	next_sp = statp->st_next;
	KFREE(statp);
	statp = next_sp;
     }
     acb->act_stat_head = acb->act_stat_tail = NULL;
  }

  /* free all stat elements on free list */
  if ( (statp=acb->stat_free) != NULL) {
     while (statp)  {
	next_sp = statp->st_next;
	KFREE(statp);
	statp = next_sp;
     }
     acb->stat_free = NULL;
  }

  /*
  ** Clear the acb flags not cleared in other locations.
  */
  acb->flags &= ~NEED_IRPT_ELEM;
  acb->flags &= ~NEED_RECV_ELEM;
  acb->flags &= ~NEED_DMA_ELEM;
  acb->flags &= ~PIO_MODE;
  acb->flags &= ~NO_RECV;
  acb->flags &= ~RECV_DMA_ON_Q;
  acb->flags &= ~STARTED_CIO;


  ENABLE_INTERRUPTS(spl);
  return;
}         /* free_all_resources() */

void
free_active_q (struct acb *acb)
{
  irpt_elem_t   *irptp;
  irpt_elem_t   *next_ip;
  recv_elem_t   *recvp;
  recv_elem_t   *next_rp;
  xmit_elem_t   *xmitp;
  xmit_elem_t   *next_xp;
  dma_elem_t    *dmap;
  dma_elem_t    *next_dp;
  stat_elem_t   *statp;
  stat_elem_t   *next_sp;
  stat_elem_t   *tmp_statp;
  int           spl;



  DISABLE_INTERRUPTS(spl);

  /* remove all dma elements from the active list */
  if ( (dmap=acb->act_dma_head) != NULL) {
     while (dmap) {
	next_dp = dmap->dm_next;
	free_dma_elem(acb,dmap);
	dmap = next_dp;
     }
     acb->act_dma_head = acb->act_dma_tail = NULL;
  }

  /* remove all irpt elements from the active list */
  if ( (irptp=acb->act_irpt_head) != NULL) {
     while (irptp) {
	next_ip = irptp->ip_next;
	free_irpt_elem(acb,irptp);
	irptp = next_ip;
     }
     acb->act_irpt_head = acb->act_irpt_tail = NULL;
  }

  /* remove all recv elements from the active list */
  if ( (recvp=acb->act_recv_head) != NULL) {
     while (recvp) {
	next_rp = recvp->rc_next;
	free_recv_elem(acb,recvp);
	recvp = next_rp;
     }
     acb->act_recv_head = acb->act_recv_tail = NULL;
  }

  /* remove all xmit elements from the active list */
  if ( (xmitp=acb->act_xmit_head) != NULL) {
     while (xmitp) {
	next_xp = xmitp->xm_next;
	free_xmit_elem(acb,xmitp);
	xmitp = next_xp;
     }
     acb->act_xmit_head = acb->act_xmit_tail = NULL;
  }


  /* remove all stat elements from the active list */
  if ( (statp=acb->act_stat_head) != NULL) {
     while (statp) {
	next_sp = statp->st_next;
		/*
		** Take the element from the "active" list.
		*/
		if (acb->act_stat_head==statp)
			acb->act_stat_head =statp->st_next;
		else {
		   tmp_statp = acb->act_stat_head;
		   while( (tmp_statp->st_next != statp) &&
				      (tmp_statp != NULL) )
			  tmp_statp = tmp_statp->st_next;
		   ASSERT(tmp_statp);
		   tmp_statp->st_next = statp->st_next;
		}
		/*
		** This also resets the active flag
		*/
		bzero(statp,sizeof(stat_elem_t));

		/*
		** Add this buffer to the "free" list.
		*/
		if (acb->stat_free == NULL)
		   acb->stat_free = statp;
		else {
		   tmp_statp = acb->stat_free;
		   while(tmp_statp->st_next != NULL)
			  tmp_statp=tmp_statp->st_next;
		   tmp_statp->st_next = statp;
		}
	statp = next_sp;
     }
     acb->act_stat_head = acb->act_stat_tail = NULL;
  }

  /*
  ** Clear the acb flags associated with the freed elements.
  */
  acb->flags &= ~NEED_IRPT_ELEM;
  acb->flags &= ~NEED_RECV_ELEM;
  acb->flags &= ~NEED_DMA_ELEM;
  acb->flags &= ~PIO_MODE;
  acb->flags &= ~NO_RECV;
  acb->flags &= ~RECV_DMA_ON_Q;
  acb->flags &= ~STARTED_CIO;

  ENABLE_INTERRUPTS(spl);
  return;
}        /* free_active_q() */

