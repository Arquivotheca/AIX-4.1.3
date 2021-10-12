static char sccsid[] = "@(#)71	1.16.1.3  src/bos/kernext/tokdiag/tokclose.c, diagddtok, bos411, 9428A410j 10/26/93 14:01:03";
/*
 * COMPONENT_NAME: (SYSXTOK) - Token-Ring device handler
 *
 * FUNCTIONS: tokclose(), ds_close()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dump.h>
#include <sys/errno.h>
#include "tok_comio_errids.h"
#include <sys/intr.h>
#include <sys/lockl.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/param.h>
#include <sys/pri.h>
#include <sys/sysmacros.h>
#include <sys/time.h>
#include <sys/trchkid.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/watchdog.h>
#include <sys/comio.h>  /* this and following files must be in this sequence */
#include <sys/tokuser.h>
#include "tokdshi.h"
#include "tokdds.h"
#include "tokdslo.h"
#include "tokproto.h"
#include "tokprim.h"

extern tracetable_t    tracetable;
extern cdt_t   ciocdt;
extern dd_ctrl_t   dd_ctrl;


/*****************************************************************************/
/*
 * NAME:     tokclose
 *
 * FUNCTION: close entry point from kernel
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  0 or errno
 *
 */
/*****************************************************************************/
int
tokclose (
   dev_t  devno,  /* major and minor number */
   chan_t chan)   /* channel number */
{
   int             rc;
   register dds_t *p_dds;
   register int    adap;
   open_elem_t    *open_ptr;
   int             saved_intr_level;
   rec_elem_t     *rec_ptr;
   xmt_elem_t     *xmt_ptr;
   int             tokconfig();
   uint            old_fct_addr, new_fct_addr;

   TRACE3 ("CLOb", (ulong)devno, (ulong)chan); /* tokclose begin */

   /* this shouldn't fail if kernel and device driver are working correctly */
   adap = minor(devno);
   if ((chan <= 0)                                 ||
       (chan > MAX_OPENS)                          ||
       ((p_dds = dd_ctrl.p_dds[adap]) == NULL) ||
       (CIO.chan_state[chan-1] != CHAN_OPENED)     ||
       ((open_ptr = CIO.open_ptr[chan-1]) == NULL)    )
   {
      rc = ENXIO;
   }
   else
   {
      rc = 0;

	/* this may be abnormal close w/o halts - make sure all netid's gone */
	while (remove_netid_for_chan (p_dds, chan))
		;	/* NULL statement */

	/* assure nobody else starts using this channel while we're closing */
	CIO.open_ptr[chan-1] = NULL;

	/* this may be abnormal close w/o shutting down ds ioctl's */
	ds_close (p_dds, open_ptr);

	old_fct_addr = functional_address(p_dds);

	if (CIO.netid_table[TOK_MAC_FRAME_NETID].inuse == TRUE)
		p_dds->wrk.mac_frame_active = TRUE;
	else
		p_dds->wrk.mac_frame_active = FALSE;

       /*
        *  If the adapter is up and the functional address will change
	*  as a result of the close, update it with the latest functional
        *  address derived from the netid table.
        */
       if ( (p_dds->cio.device_state == DEVICE_CONNECTED) &&
		(p_dds->wrk.limbo == PARADISE) &&
		(p_dds->wrk.adap_state == OPEN_STATE))
	{
	    new_fct_addr = functional_address(p_dds);
	    if (new_fct_addr != old_fct_addr)
	    {
            	issue_scb_command(p_dds, ADAP_FUNCT_CMD, new_fct_addr);
            	check_scb_command(p_dds);
	    }
	}

      if (!(open_ptr->devflag & DKERNEL)) /* this is user task */
      {
         /* free the mbuf's for all elements on receive que */
         /* new ones cannot appear because there's no netid's for this user */
         while ((rec_ptr = (rec_elem_t *) sll_unlink_first (
            (s_link_list_t *)(&(open_ptr->rec_que)))) != NULL)
         {
            m_freem (rec_ptr->mbufp);

            /*
            At this point, we could free the receive element using
            sll_free_elem, but we're going to free up entire open structure
            which includes the entire receive element linked list.  That means
            there's no point to cleaning up allocated vs. free part of linked
            list.  The same considerations apply to the status queue.
            */
         }
      }

      /* remove transmit requests for this open only */
      tok_free_chan_elem(p_dds, chan);

      /* delete open structure from component dump table */
      cio_del_cdt ("OpenStrc", (char *)open_ptr, open_ptr->alloc_size);

      /* free the open structure (and user process rec que and sta que) */
      if (KMFREE (open_ptr) != 0)
         TRACE2 ("CLO1",
            (ulong)open_ptr); /* tokclose ERROR (free open_ptr failed) */

      DISABLE_INTERRUPTS (saved_intr_level);
      CIO.chan_state[chan-1] = CHAN_CLOSED;

      --dd_ctrl.num_opens;	/* # opens any adapter */
      --CIO.num_opens;		/* # opens this adapter */

      if (CIO.num_opens == 0) /* this is last close for this adapter*/
      {
         CIO.device_state = DEVICE_DISC_IN_PROG;

         if (dd_ctrl.num_opens == 0) /* this is last close for any adapter */
         {
            ENABLE_INTERRUPTS (saved_intr_level);

   	    /* un-register for dump */
	    dmp_del ( ( ( void (*) ())cio_cdt_func) );
         }
	 else
	 {
            ENABLE_INTERRUPTS (saved_intr_level);
         }

         (void) ds_deact (p_dds); /* allow adapter shutdown and cleanup */

           /*
            *  De-allocate the ACA if needed.
            */
         if ( p_dds->wrk.adap_state != DEAD_STATE )
           rc = aca_dealloc(p_dds);

         CIO.device_state = DEVICE_NOT_CONN;

         if (CIO.timr_registered)
         {
            w_stop (&(WDT));
            w_clear (&(WDT));
            CIO.timr_registered = FALSE;
         }

         if (CIO.intr_registered)
         {
            i_clear ((struct intr *)p_dds);
            CIO.intr_registered = FALSE;
         }

         /* consistency check - either trace point indicates cleanup problem */
         if (CIO.num_netids != 0)
            TRACE2 ("CLO2",
               (ulong)CIO.num_netids); /* tokclose ERROR (num_netids != 0) */

         /*
          *  Unpin code
          */
          unpincode(tokconfig);
		

      }
      else
      {
         ENABLE_INTERRUPTS (saved_intr_level);
      }
   }

   TRACE2 ("CLOe", (ulong)rc); /* tokclose end */
   return (rc);
} /* end tokclose */

/*-------------------------  D S _ C L O S E  -----------------------------*/
/*                                                                         */
/*  Removes the Group address associated with this close.                  */
/*                                                                         */
/*-------------------------------------------------------------------------*/

void ds_close (register dds_t       *p_dds,
                 register open_elem_t *open_ptr)
{
        /*                                                                 */
        /*  If this channel is responsible for the Group Address, delete   */
        /*  the group address from the adapter.                            */
        /*                                                                 */
   if (p_dds->cio.device_state == DEVICE_CONNECTED)
   {
        if (p_dds->wrk.group_addr_chan == open_ptr->chan) 
	{
		/*
		 * Clear out the group address
		 * /
            p_dds->wrk.group_address   = 0;
            p_dds->wrk.group_addr_chan = 0;
		/*
		 * Check if we are in Limbo, if not
		 * issue the the Group Address command 
		 * to the adapter.
		 */

	    if ( (p_dds->wrk.limbo == PARADISE ) &&
		 (p_dds->wrk.adap_state == OPEN_STATE) ) {
		issue_scb_command (p_dds, ADAP_GROUP_CMD, 
				   p_dds->wrk.group_address);
		check_scb_command (p_dds);
	    }
        }
   }   /* end if device connected */
}  /* end ds_close */
