static char sccsid[] = "@(#)82	1.7  src/bos/kernext/mpa/mpa_close.c, sysxmpa, bos41J, 9520B_all 5/17/95 14:53:38";
/*
 *   COMPONENT_NAME: (SYSXMPA) MP/A SDLC DEVICE DRIVER
 *
 *   FUNCTIONS: free_all_resources
 *		mpa_close
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <errno.h>
#include <sys/adspace.h>
#include <sys/device.h>
#include <sys/ddtrace.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/ioctl.h>
#include <sys/malloc.h>
#include <sys/timer.h>
#include <sys/sleep.h>
#include <sys/mpadd.h>
#include <sys/sysmacros.h>
#include <sys/trchkid.h>


 /*ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ MPA_CLOSE ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
 º NAME: mpa_close                                                          º
 º                                                                          º
 º FUNCTION: Called to close a single port of the multiprotocol             º
 º           adaptor.                                                       º
 º                                                                          º
 º EXECUTION ENVIRONMENT:                                                   º
 º                                                                          º
 º      Preemptable        : Yes                                            º
 º      VMM Critical Region: Yes                                            º
 º      Runs on Fixed Stack: Yes                                            º
 º      May Page Fault     : No                                             º
 º                                                                          º
 º (NOTES:) More detailed description of the function, down to              º
 º      what bits / data structures, etc it manipulates.                    º
 º                                                                          º
 º (RECOVERY OPERATION:) Information describing both hardware and           º
 º      software error recovery.                                            º
 º                                                                          º
 º (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.  º
 º                                                                          º
 º RETURNS: What this code returns (NONE, if it has no return value)        º
 ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ*/

int mpa_close ( dev_t            dev,
		int             chan,
		int             ext )

{
	int             spl, rc;
	struct acb      *acb;         /* pointer to adapter control block */

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ log a trace hook ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	DDHKWD5(HKWD_DD_MPADD,DD_ENTRY_CLOSE,0,dev,chan,ext,0,0);
	MPATRACE4("ClsE",dev,chan,ext);

	if ((acb = get_acb(minor(dev))) == NULL) 
	{
		MPATRACE2("Cle1",dev);
		return ENODEV;
	}

	if (lockl(&acb->adap_lock, LOCK_SIGRET) != LOCK_SUCC) 
	{
		MPATRACE2("Cle2",dev);
		return EINTR;
	}


	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ now we follow down to remove the select queue data structure ³
	³ the channel information data structure and null out the      ³
	³ pointer in the dds to the channel info ds                    ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

	DISABLE_INTERRUPTS(spl);

	if /* close being called before CIO_HALT */
	( acb->flags & STARTED_CIO )
	{
		/* Defect 178174 */
		acb->flags &= ~STARTED_CIO; /* mark as not started */
		/* End Defect 178174 */
		if( (rc=shutdown_adapter(acb)) ) 
		{
			MPATRACE3("Cle3",dev,rc);
			ENABLE_INTERRUPTS(spl);
          		return rc;
     		}

     		free_active_q(acb);
	}
	ENABLE_INTERRUPTS(spl);

	OPENP.mpa_kopen.rx_fn = NULL;
	OPENP.mpa_kopen.stat_fn = NULL;
	OPENP.mpa_kopen.tx_fn = NULL;
	OPENP.mpa_kopen.open_id = 0;

	MPATRACE3("ClsX",dev,chan);

        free_open_struct(acb);

	OPENP.op_mode = 0;      /* remove device flags */

	acb->flags &= ~STARTED_CIO; /* mark as not started */

	unlockl(&acb->adap_lock);

	DDHKWD5(HKWD_DD_MPADD,DD_EXIT_CLOSE,0,dev,chan,ext,0,0);
	return(0);
} /* mpa_close() */

/*
 * Note that this routine is only called from mpa_term_dev which
 * implies that it is always called with the adapter lock held and
 * interrupts enabled.  Since the first thing done is to shutdown the
 * adapter, interrupts are not disabled when all the resources are
 * freed.  This makes the freeing easier.
 */
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
  int           one_started = 0;


  /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ prior to freeing all the resources I want to make sure I get ³
  ³ no more irpts from the adapter, so I am shuting it down here.³ 
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
  shutdown_adapter(acb);

  /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ free all dma elements on active list ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
  if ( (dmap=acb->act_dma_head) != NULL) 
  {
     while (dmap) 
     {
	next_dp = dmap->dm_next;

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ if DMA_STARTED already do d_complete ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	if(dmap->dm_state == DM_STARTED && !one_started) 
	{
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

  /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ Now that I know there are no dmas active d_clear dma channel. ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
  if (acb->flags & MPADMACHAN) 
  {
	  d_clear(acb->dma_channel);
  }
  acb->flags &= ~MPADMACHAN;


  /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ free all dma elements on free list ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
  if ( (dmap=acb->dma_free) != NULL) 
  {
     while (dmap) 
     {
	next_dp = dmap->dm_next;
	KFREE(dmap);
	dmap = next_dp;
     }
     acb->dma_free = NULL;
  }


  /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ free all irpt elements on active list ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
  if ( (irptp=acb->act_irpt_head) != NULL) 
  {
     while (irptp) 
     {
	next_ip = irptp->ip_next;
	KFREE(irptp);
	irptp = next_ip;
     }
     acb->act_irpt_head = acb->act_irpt_tail = NULL;
  }

  /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ Now that I know there are no irpts comming i_clear irpt struct(s) ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
  if (acb->flags & MPAIINSTALL3) 
  {
	 i_clear(&acb->ih_structA);
  }
  acb->flags &= ~MPAIINSTALL3;

  if (acb->flags & MPAIINSTALL4) 
  {
	 i_clear(&acb->ih_structB);
  }
  acb->flags &= ~MPAIINSTALL4;

  /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ free all irpt elements on free list ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
  if ( (irptp=acb->irpt_free) != NULL) 
  {
     while (irptp) 
     {
	next_ip = irptp->ip_next;
	KFREE(irptp);
	irptp = next_ip;
     }
     acb->irpt_free = NULL;
  }

  /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ free all xmit elements on active list ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
  if ( (xmitp=acb->act_xmit_head) != NULL) 
  {
     while (xmitp) 
     {
	next_xp = xmitp->xm_next;
	free_xmit_elem(acb,xmitp);
	xmitp = next_xp;
     }
     acb->act_xmit_head = acb->act_xmit_tail = NULL;
  }

  /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ free all xmit elements on free list ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
  if ( (xmitp=acb->xmit_free) != NULL) 
  {
     while (xmitp) 
     {
	next_xp = xmitp->xm_next;
	KFREE(xmitp);
	xmitp = next_xp;
     }
     acb->xmit_free = NULL;
  }

  /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ free all recv elements on active list ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
  if ( (recvp=acb->act_recv_head) != NULL) 
  {
     while (recvp) 
     {
	next_rp = recvp->rc_next;
        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
        ³ free any unused MBUFs at this time ³
        ³ (this was added for defect #091440)³
        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        if (recvp->rc_state & RC_MBUF)
        {
                m_freem((struct mbuf *)recvp->rc_mbuf);
                recvp->rc_state &= ~RC_MBUF;
        }
	KFREE(recvp);
	recvp = next_rp;
     }
     acb->act_recv_head = acb->act_recv_tail = NULL;
  }

  /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ free all recv elements on free list ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
  if ( (recvp=acb->recv_free) != NULL) 
  {
     while (recvp) 
     {
	next_rp = recvp->rc_next;
	KFREE(recvp);
	recvp = next_rp;
     }
     acb->recv_free = NULL;
  }

  /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ Clear the acb flags not cleared in other locations. ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
  acb->flags &= ~NEED_IRPT_ELEM;
  acb->flags &= ~NEED_RECV_ELEM;
  acb->flags &= ~NEED_DMA_ELEM;
  acb->flags &= ~RECV_DMA_ON_Q;
  acb->flags &= ~STARTED_CIO;

  return;
}         /* free_all_resources() */
