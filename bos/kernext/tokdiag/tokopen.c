static char sccsid[] = "@(#)73	1.22  src/bos/kernext/tokdiag/tokopen.c, diagddtok, bos411, 9428A410j 10/26/93 14:15:51";
/*
 * COMPONENT_NAME: (SYSXTOK) - Token-Ring device handler
 *
 * FUNCTIONS:  tokmpx(), tokopen(), get_mem(), sb_setup(),
 *             sb_undo(), get_mem_undo(), aca_alloc(),
 *             aca_dealloc()
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
#include <sys/dma.h>
#include <sys/dump.h>
#include <sys/errno.h>
#include <sys/err_rec.h>
#include "tok_comio_errids.h"
#include <sys/intr.h>
#include <sys/ioctl.h>
#include <sys/limits.h>
#include <sys/lockl.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/pri.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/time.h>
#include <sys/timer.h>
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
/*
*  get access to the trace table
*/
extern tracetable_t    tracetable;

/*
*  get access to the component dump table
*/
extern cdt_t   ciocdt;

/*
*  get access to the device driver control block
*/
extern dd_ctrl_t   dd_ctrl;

/*****************************************************************************/
/*
 * NAME:     tokmpx
 *
 * FUNCTION: mpx entry point from kernel (before open and after close)
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  0 or errno
 *           if successful, also returns allocated channel number
 *
 */
/*****************************************************************************/
int tokmpx (
	dev_t  devno,     /* major and minor number                         */
	int   *chanp,     /* address for returning allocated channel number */
	char  *channame)  /* pointer to special file name suffix            */
{
	register dds_t *p_dds;
	register int    adap;
	int             saved_intr_level;
	int             ndx;
	int             rc;
	int             tokconfig();

	DEBUGTRACE4 ("MPXb", (ulong)devno, (ulong)(*chanp),
  	                (ulong)channame); /* ciompx begin */

	/* Pin code while we are in config */
	pincode(tokconfig);

	adap = minor(devno);
	if ((p_dds = dd_ctrl.p_dds[adap]) == NULL)
		rc = ENXIO;
	else {
		rc = 0;
		DISABLE_INTERRUPTS(saved_intr_level);
		if (channame == NULL) {     /* this is de-allocate request */
			if (CIO.chan_state[(*chanp)-1] == CHAN_AVAIL)
				rc = ENOMSG;
			else {
				CIO.chan_state[(*chanp)-1] = CHAN_AVAIL;
				CIO.num_allocates--;
			}
			ENABLE_INTERRUPTS(saved_intr_level);
		} else {/* this is allocate request */
			/*
			 * filename extensions are requests for exclusive use
			 * make sure it's ok to let this user get a channel 
                         * and open.
			 */
			if (((*channame != '\0') && (CIO.num_allocates != 0)) ||
				(CIO.num_allocates >= MAX_OPENS))
				rc = EBUSY;
			else {
				for (ndx=0;
				    (ndx < MAX_OPENS) && 
				    (CIO.chan_state[ndx] != CHAN_AVAIL);
				    ndx++); /* NULL statement */
				if (ndx == MAX_OPENS)
					rc = EBUSY;
				else {
					CIO.num_allocates++;
					CIO.chan_state[ndx] = CHAN_ALLOCATED;
				}
			}
			ENABLE_INTERRUPTS(saved_intr_level);
			if (rc == 0) { /* allocate was successful */
				if ( (*channame != '\0') && 
				     (*channame != 'D')  &&
				     (*channame != 'W') ) {
					/* undo the allocation */
					DISABLE_INTERRUPTS(saved_intr_level);
					CIO.chan_state[ndx] = CHAN_AVAIL;
					CIO.num_allocates--;
					ENABLE_INTERRUPTS(saved_intr_level);
					rc = EBUSY;
				} else {
					/*
					 *  save "mode" for device specific 
					 *  code's use
					 */
					/* 
					 * note this is '\0' or a letter
					 */
					CIO.mode = *channame;
					/*
					 * tell kernel the channel number we 
					 * allocated
					 */
					/* 
					 * NOTE WELL! channel number is 1-based
					 */
					*chanp = ndx + 1;
				}
			}
		}
	}

	/* Unpin code before finishing config. */
	unpincode(tokconfig);

	DEBUGTRACE2 ("MPXe", (ulong)rc); /* tokmpx end */
	return (rc);

} /* end tokmpx */

/*****************************************************************************/
/*
 * NAME:     tokopen
 *
 * FUNCTION: open entry point from kernel
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
tokopen (
   dev_t            devno,   /* major and minor number */
   ulong            devflag, /* flags including DKERNEL and DNDELAY */
   chan_t           chan,    /* channel number */
   cio_kopen_ext_t *extptr)  /* this could be pointer to struct uopen_ext */
{
   int rc;
   register int adap;
   register dds_t *p_dds;
   int alloc_size;
   open_elem_t *open_ptr;
   int saved_intr_level;
   int pio_attachment;
   uchar pos2;
   int   first_open=FALSE;
   int   tokconfig();

   TRACE5 ("OPNb", (ulong)devno, (ulong)devflag, (ulong)chan,
		(ulong)extptr); /* tokopen begin */

   rc = 0;
   adap = minor(devno);         /* get minor number */
   /* this shouldn't fail if kernel and device driver are working correctly */
   if ((chan <= 0)                                 ||
       (chan > MAX_OPENS)                          ||
       ((p_dds = dd_ctrl.p_dds[adap]) == NULL) ||
       (CIO.chan_state[chan-1] != CHAN_ALLOCATED)     )
   {
      rc = ENXIO;
   }
   else
   {

      /* make sure we're not in process of shutting down adapter */
      if (CIO.device_state == DEVICE_DISC_IN_PROG)
      {
         rc = ENOTREADY; /* disconnect in progress */
      }
      else
      {
         if (((devflag & DKERNEL)           &&
              ((extptr == NULL)         ||
               (extptr->rx_fn == NULL)  ||
               (extptr->tx_fn == NULL)  ||
               (extptr->stat_fn == NULL)   )   )   )
         {
            rc = EINVAL;
         }
         else
         {
            /* allocate memory for open structure and initialize it */
            alloc_size = sizeof(open_elem_t);
            if (!(devflag & DKERNEL)) /* need space for user process queues */
            {
               alloc_size += p_dds->ddi.rec_que_size * sizeof(rec_elem_t);
               alloc_size += p_dds->ddi.sta_que_size * sizeof(sta_elem_t);
            }

               /*
                *  Allocate the ACA.
		*  This routine only does allocation on the 
		*  very first open to the device.
                */

            if ( ( rc = aca_alloc(p_dds,&first_open) ) != 0 )
            {      /* ACA allocation failed. */
               rc = ENOMEM;
            }
            else if ((open_ptr = (open_elem_t *) KMALLOC (alloc_size)) == NULL)
            {
               rc = ENOMEM;
            }
            else
            {
               bzero (open_ptr, alloc_size); /* xmalloc does NOT zero area */
               open_ptr->alloc_size = alloc_size;
               open_ptr->devno = CIO.devno;
               open_ptr->chan = chan;
               open_ptr->devflag = devflag;
               open_ptr->xmt_event = EVENT_NULL;
               open_ptr->rec_event = EVENT_NULL;
               if (devflag & DKERNEL)
               {
                  open_ptr->rec_fn =  ((cio_kopen_ext_t *)extptr)->rx_fn;
                  open_ptr->xmt_fn =  ((cio_kopen_ext_t *)extptr)->tx_fn;
                  open_ptr->sta_fn =  ((cio_kopen_ext_t *)extptr)->stat_fn;
                  open_ptr->open_id = ((cio_kopen_ext_t *)extptr)->open_id;
               }
               else
               {
                  /* initialize receive queue right after open structure */
                  sll_init_list ((s_link_list_t *)(&(open_ptr->rec_que)),
                     (sll_elem_ptr_t) ((int)open_ptr + sizeof(open_elem_t)),
                     p_dds->ddi.rec_que_size,
                     (int) sizeof(rec_elem_t),
                     (ulong *)(&(RAS.cc.rec_que_high)));

                  /* initialize status queue right after receive queue */
                  sll_init_list ((s_link_list_t *)(&(open_ptr->sta_que)),
                     (sll_elem_ptr_t) (open_ptr->rec_que.limt_ptr),
                     p_dds->ddi.sta_que_size,
                     (int) sizeof(sta_elem_t),
                     (ulong *)(&(RAS.cc.sta_que_high)));
               }

               DISABLE_INTERRUPTS (saved_intr_level);
               CIO.open_ptr[chan-1] = open_ptr;
               CIO.num_opens++;			/* # opens this adapter */
               CIO.chan_state[chan-1] = CHAN_OPENED;
               dd_ctrl.num_opens++;		/* # opens any adapter */

	       if (dd_ctrl.num_opens == 1) /* first open for any adapter */
	       {
                  ENABLE_INTERRUPTS (saved_intr_level);
	          /* register for dump */
	          dmp_add ( ( ( void (*) ())cio_cdt_func) );
	       }
	       else
	       {
                  ENABLE_INTERRUPTS (saved_intr_level);
	       }

               if (!CIO.intr_registered)
               {
                  /* add our interrupt routine to kernel's interrupt chain */
		
		TRACE2("intB", (ulong)p_dds);

                  i_init ((struct intr *)(&(IHS)));
                  CIO.intr_registered = TRUE;

		TRACE2("intE", (ulong)p_dds);

		/* 
		 * Enable the card. 
		 * Get access to pos registers.
		 * Get the current setting of pos reg 2.
		 * Turn on the card enable bit in pos.
		 *
		 */

		pio_attachment = attach_iocc( p_dds );
	
		pos2 = pio_read(p_dds, POS_REG_2);
	
		(void)pio_write(p_dds, POS_REG_2, (pos2 | (CARD_ENABLE) ) );

		detach_iocc(p_dds, pio_attachment);

               }

               if (!CIO.timr_registered)
               {
                  /* add our watchdog timer routine to kernel's list */
                  w_init ((struct watchdog *)(&(WDT)));
                  CIO.timr_registered = TRUE;
               }

               /* add open structure to component dump table */
               cio_add_cdt ("OpenStrc",(char *)open_ptr,open_ptr->alloc_size);

            } /* end else for bad open pointer */
         } /* end else for bad kernel arguments */
      } /* end else for disconnect in progress */
   } /* end else for bad adapter/channel */

   TRACE2 ("OPNe", (ulong)rc); /* tokopen end */
   /*
    *  If this was the first open for the adapter and the open
    *  failed after the code was pinned successfully then need to
    *  unpin the code.
    */
   if ((rc != 0) && (first_open != 0))
       unpincode(tokconfig);

   return (rc);

} /* end tokopen */
/*
 * NAME: aca_alloc()
 *
 * FUNCTION:
 *
 *  Adapter Control Area Allocater.
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *       This routine can only execute on the process thread.
 *
 * NOTES:
 *
 *     The only valid way for the aca_alloc() routine can be called
 *     is when the adap_state variable is set to DEAD_STATE.
 *
 *     1. Call get_mem function which will get the System to Adapter Control
 *        area and zero it out.
 *
 *     2. Call sb_setup() function which will setup the the SCB, SSB,
 *        Adapter Error Log, and Adapter Information Blocks so the adapter can
 *        have access them.
 *
 *     3. Call tx_set() which will setup the Transmit List Chain so the
 *       adapter can have access to it.
 *
 *     4. Call rx_setup() which will setup the Receive List Chain so the
 *        adapter can have access to it.  The Receive Chain will be populated
 *        with PAGESIZE mbufs for receive data.  Each PAGESIZE mbuf will
 *        d_mastered so the adapter can DMA data into it.
 *
 *     Functions Called:
 *               int get_mem()           Get dynamic work block
 *               int sb_setup()          set up the SSB and SCB
 *               int tx_setup()          set up the Transmit List Chain
 *               int tok_recv_setup()    set up the Receive List Chain
 *
 * RECOVERY OPERATION:
 *         Error Cleanup Functions:
 *               int tok_recv_undo()     Undo the Receive Chain setup
 *               int tx_undo()           Undo the Transmit Chain setup
 *               int sb_undo()           Undo the system block setup
 *               int get_mem_undo()      Undo the Control area allocation
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *      None.
 */
int
aca_alloc(dds_t    *p_dds, int *first_open)

{  /* begin function aca_alloc() */

	int                 rc=0,
	                    temp=0,
	                    saved_int_lvl,
	                    tokconfig();

	/*
	 *  if ACA already allocated, then return.
         */
	if ( p_dds->wrk.adap_state != DEAD_STATE)
		return(rc);

	/*
	 *  Since this is the first time opening the driver then
	 *  need to pin code.
	 */
	pincode(tokconfig);
	*first_open = TRUE;

	/*
	 *  Allocate & setup ACA; If fails then return error.
	 */
	if (rc = get_mem(p_dds)) {
		unpincode(tokconfig);
		*first_open = FALSE;
		return(rc);
	}

	/*
	 *  Setup SSB and SCB. If fails then return error.
	 */
	if (rc = sb_setup(p_dds)) {
		temp = get_mem_undo(p_dds);
		unpincode(tokconfig);
		*first_open = FALSE;
		return(rc);
	}

	/*
	 *  Setup transmit list. If fails then return error.
	 */
	if (rc = tx_setup(p_dds)) { /* Transmit List setup failed. */
		temp = sb_undo(p_dds);
		temp = get_mem_undo(p_dds);
		unpincode(tokconfig);
		*first_open = FALSE;
		return(rc);
	}

	/*
	 *  NOTE:
	 *       Must issue the d_master() for the ACA before
	 *       calling the Receive Setup routine.  The tok_recv_setup()
	 *       routine assumes that the ACA has been d_mastered.
	 */

	/*
	 *  set up DMA for the memory block via d_master
	 */
	d_master(p_dds->wrk.dma_chnl_id, DMA_READ|DMA_NOHIDE,
		p_dds->wrk.p_mem_block, PAGESIZE,
		&(p_dds->wrk.mem_block_xmd),
		p_dds->wrk.p_d_mem_block);

	/*
	 *  set state machine to note that the ACA is now
	 * available to use.
	 */
	saved_int_lvl = i_disable(INTCLASS2);
	p_dds->wrk.adap_state = CLOSED_STATE;
	i_enable(saved_int_lvl);


	if (rc = tok_recv_setup(p_dds)) { /* Receive setup fail. return error */
		/*
		 *  NOTE:
		 *  Must issue the d_complete() for the ACA before the call
		 *  to any TX undo routine.  The undo routines assume that the
		 *  ACA has been "un-hidden" via the d_complete() routine
		 */

		saved_int_lvl = i_disable(INTCLASS2);
		p_dds->wrk.adap_state = NULL_STATE;
		i_enable(saved_int_lvl);

		temp = d_complete(p_dds->wrk.dma_chnl_id, DMA_READ,
				p_dds->wrk.p_mem_block, PAGESIZE,
				&(p_dds->wrk.mem_block_xmd),
				p_dds->wrk.p_d_mem_block);

		temp = tx_undo(p_dds);
		temp = sb_undo(p_dds);
		temp = get_mem_undo(p_dds);

		unpincode(tokconfig);
		*first_open = FALSE;
		return(rc);
	}

	/* 
	 * Since this is the very first open.
	 * Zero out the statistics.
	 */

	bzero( &(p_dds->ras), sizeof(query_stats_t) );
	return(rc);

}  /* end function aca_alloc() */

/*
 * NAME: aca_dealloc()
 *
 * FUNCTION:
 *
 *  Adapter Control Area De-Allocater.
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *       This routine can only execute on the process thread.
 *
 * NOTES:
 *
 *     The only valid way for the aca_dealloc() routine can be called
 *     is when the adap_state variable is NOT set to DEAD_STATE.
 *
 *     Functions called:
 *               int tok_recv_undo()     Undo the Receive Chain setup
 *               int tx_undo()           Undo the Transmit Chain setup
 *               int sb_undo()           Undo the system block setup
 *               int get_mem_undo()      Undo the Control area allocation
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *      None.
 */
int
aca_dealloc(dds_t    *p_dds)

{  /* begin function aca_dealloc() */

	int rc=0, sil;
	int pio_attachment;
	uchar pos2;

	sil = i_disable(INTCLASS2);
	if ( (p_dds->wrk.limbo == NO_OP_STATE) &&
	     (p_dds->wrk.adap_state == NULL_STATE) )
	{
		/* we've bugged out. Clean up after
		 * the bug_out() routine.
		 */
		i_enable(sil);
		rc = bug_out_cleanup(p_dds);
	}
	else
	{
		i_enable(sil);

		/* undo the receive block 
		 */
		rc = tok_recv_undo(p_dds);   

	/*
	 *  NOTE:
	 *       Must issue the d_complete() for the ACA before the call
	 *       to any TX undo routine.  The undo routines assume that the
	 *       ACA has been "un-hidden" via the d_complete() routine
	 *
	 *      Research what the state of the xmem descriptor is.
	 *      It may be set up incorrectly at this point
	 *
	 */

		rc = clean_tx(p_dds);

		sil = i_disable(INTCLASS2);
		p_dds->wrk.adap_state = NULL_STATE;
		i_enable(sil);

		rc = d_complete(p_dds->wrk.dma_chnl_id, DMA_READ,
				p_dds->wrk.p_mem_block, PAGESIZE,
				&(p_dds->wrk.mem_block_xmd),
				p_dds->wrk.p_d_mem_block);



		rc = tx_undo(p_dds);           /* undo the transmit block */

		rc = sb_undo(p_dds);           /* undo the System block */

		rc = get_mem_undo(p_dds);      /* Free control block */

		/* Disable the card.  We don't
		 * want to give the card the chance to 
		 * misbehave anymore.
		 *
		 * Get access to pos registers.
		 * Get the current setting of pos reg 2.
		 * Turn off the card enable bit in pos.
		 *
		 */

		pio_attachment = attach_iocc( p_dds );
	
		pos2 = pio_read(p_dds, POS_REG_2);
	
		(void)pio_write(p_dds, POS_REG_2, (pos2 & ~(CARD_ENABLE) ) );

		detach_iocc(p_dds, pio_attachment);


	} /* end if normal de-allocation */

	return(rc);   

} /* end function aca_dealloc() */

/*--------------------------------------------------------------------*/
/*********************  Get Memory Block                ***************/
/*--------------------------------------------------------------------*/

/*
*  This function will:
*          - xmalloc a PAGESIZE chunk of memory for the Adapter Control Area
*          - zero out the Adapter Control Area
*          - set up DMA for the control area using the LAST TCW
*          - set up the xmem descriptor for the control area
*          - d_master the control area
*          - d_complete the control area
*
*  RETURN CODES:
*      0        - Good return
*      ENOBUFS  - Bad. Unable to get required memory
*
*/

int get_mem(dds_t *p_dds)

{  /* begin function get_mem */
   register int rc= 0;
   int sil;            /* saved interrupt level */

  /*
   *  Initialize the DMA channel via the d_init() kernel
   *  service routine.  If the d_init() completes successfully
   *  call the d_unmask() kernel service to enable the channel
   *  off.
   */

    /* get dma channel id by calling d_init */
    p_dds->wrk.dma_chnl_id =
        d_init(p_dds->ddi.dma_lvl, MICRO_CHANNEL_DMA,
               p_dds->ddi.bus_id);

    /* go ahead and enable the dma channel by callin d_unmask */
    if (p_dds->wrk.dma_chnl_id != DMA_FAIL)
        d_unmask(p_dds->wrk.dma_chnl_id);
    else
    {
       /*
        *  Log unknown error and return
        */
        p_dds->wrk.footprint = D_UNMASK_FAIL;
        logerr(p_dds, ERRID_TOK_ERR15);
        return(EFAULT);
    }

   /* get Page size chunk of memory */
p_dds->wrk.p_mem_block = xmalloc(PAGESIZE, PGSHIFT, pinned_heap);

   /* see if memory allocation was successful */
if (p_dds->wrk.p_mem_block == NULL)
{
   d_clear(p_dds->wrk.dma_chnl_id);

   p_dds->wrk.footprint = D_CLEAR_FAIL;
      /*
       *  Log unknown error and return
       */
       logerr(p_dds, ERRID_TOK_ERR15);

   return(ENOBUFS);     /* No! return errno */
}  /* end if no memory available */

/* set up cross memory descriptor */
p_dds->wrk.mem_block_xmd.aspace_id = XMEM_INVAL;
rc = xmattach( p_dds->wrk.p_mem_block, PAGESIZE,
               &(p_dds->wrk.mem_block_xmd), SYS_ADSPACE);
if (rc == XMEM_FAIL )
{  /* oops! undo what's been done so far and return failure */
   d_clear(p_dds->wrk.dma_chnl_id);

   p_dds->wrk.footprint = D_CLEAR_FAIL;
       /*
        *  Log unknown error and return
        */
        logerr(p_dds, ERRID_TOK_ERR15);
   return(ENOBUFS);
}

   /*
   *   Setup the Adapter bringup timer structures
   */

p_dds->wrk.p_bringup_timer = (struct trb *)talloc();

/* see if memory allocation was successful */
if (p_dds->wrk.p_bringup_timer == NULL)
{

   	/* Clean up and reset pointers */
   xmdetach( &(p_dds->wrk.mem_block_xmd) );
   p_dds->wrk.p_d_mem_block = NULL;
   xmfree(p_dds->wrk.p_mem_block, pinned_heap);
   (void)d_clear(p_dds->wrk.dma_chnl_id);

      /*
       *  Log unknown error and return if memory allocation failed
       */
   p_dds->wrk.footprint = TOK_XMALL_FAIL;
   logerr(p_dds, ERRID_TOK_ERR15);

   return(ENOBUFS);     /* return errno */
 }

       /* Turn off Absolute timer.  We want an incremental timer */
p_dds->wrk.p_bringup_timer->flags &= ~(T_ABSOLUTE);

       /* get address of our adapter bringup timer function */
p_dds->wrk.p_bringup_timer->func = ( void (*) ())bringup_timer;

       /* set priority of timer */
p_dds->wrk.p_bringup_timer->ipri = INTCLASS2;

       /* get address of the data for the bringup timer function */
p_dds->wrk.p_bringup_timer->func_data = (unsigned long)
                       &(p_dds->wrk.time_data);

p_dds->wrk.time_data.p_dds = (caddr_t) p_dds;
p_dds->wrk.time_data.run = FALSE;


   /*
   *   Setup the Probation timer structures 
   */

p_dds->wrk.p_probate_timer = (struct trb *)talloc();

/* see if memory allocation was successful */
if (p_dds->wrk.p_probate_timer == NULL)
{

   	/* Clean up and reset pointers */
   xmdetach( &(p_dds->wrk.mem_block_xmd) );
   p_dds->wrk.p_d_mem_block = NULL;
   xmfree(p_dds->wrk.p_mem_block, pinned_heap);
   (void)d_clear(p_dds->wrk.dma_chnl_id);

	/* Stop and Free timers that may be started */

   if (p_dds->wrk.time_data.run)
	 tstop(p_dds->wrk.p_bringup_timer);
   tfree(p_dds->wrk.p_bringup_timer);
      	/*
       	*  Log unknown error and return if memory allocation failed
       	*/
   p_dds->wrk.footprint = TOK_XMALL_FAIL;
   logerr(p_dds, ERRID_TOK_ERR15);

   return(ENOBUFS);     /* return errno */
 }

       /* Turn off Absolute timer.  We want an incremental timer */
p_dds->wrk.p_probate_timer->flags &= ~(T_ABSOLUTE);

       /* get address of our adapter bringup timer function */
p_dds->wrk.p_probate_timer->func = ( void (*) ())bringup_timer;

       /* set priority of timer */
p_dds->wrk.p_probate_timer->ipri = INTCLASS2;

       /* get address of the data for the bringup timer function */
p_dds->wrk.p_probate_timer->func_data = (unsigned long)
                       &(p_dds->wrk.probate_td);

p_dds->wrk.probate_td.p_dds = (caddr_t) p_dds;
p_dds->wrk.probate_td.run = FALSE;


   /*
   *   Setup the Receive Limbo timer structures
   */

p_dds->wrk.p_recv_limbo_timer = (struct trb *)talloc();

/* see if memory allocation was successful */
if (p_dds->wrk.p_recv_limbo_timer == NULL)
{

   	/* Clean up and reset pointers */
   xmdetach( &(p_dds->wrk.mem_block_xmd) );
   p_dds->wrk.p_d_mem_block = NULL;
   xmfree(p_dds->wrk.p_mem_block, pinned_heap);
   (void)d_clear(p_dds->wrk.dma_chnl_id);
 
	/* Stop and Free timers that may be started */

   if (p_dds->wrk.time_data.run)
	 tstop(p_dds->wrk.p_bringup_timer);
   tfree(p_dds->wrk.p_bringup_timer);

   if (p_dds->wrk.probate_td.run)
	 tstop(p_dds->wrk.p_probate_timer);
   tfree(p_dds->wrk.p_probate_timer);
      	/*
       	*  Log unknown error and return if memory allocation failed
       	*/
   p_dds->wrk.footprint = TOK_XMALL_FAIL;
   logerr(p_dds, ERRID_TOK_ERR15);

   return(ENOBUFS);     /* return errno */
 }
       /* Turn off Absolute timer.  We want an incremental timer */
p_dds->wrk.p_recv_limbo_timer->flags &= ~(T_ABSOLUTE);

       /* get address of our adapter bringup timer function */
p_dds->wrk.p_recv_limbo_timer->func = ( void (*) ())bringup_timer;

       /* set priority of timer */
p_dds->wrk.p_recv_limbo_timer->ipri = INTCLASS2;

       /* get address of the data for the bringup timer function */
p_dds->wrk.p_recv_limbo_timer->func_data = (unsigned long)
                       &(p_dds->wrk.recv_limbo_td);

p_dds->wrk.recv_limbo_td.p_dds = (caddr_t) p_dds;
p_dds->wrk.recv_limbo_td.run = FALSE;



   /*
   *   Setup the Functional Address timer structures
   */

p_dds->wrk.p_func_timer = (struct trb *)talloc();

/* see if memory allocation was successful */
if (p_dds->wrk.p_func_timer == NULL)
{
   	/* Clean up and reset pointers */
   xmdetach( &(p_dds->wrk.mem_block_xmd) );
   p_dds->wrk.p_d_mem_block = NULL;
   xmfree(p_dds->wrk.p_mem_block, pinned_heap);
   (void)d_clear(p_dds->wrk.dma_chnl_id);
 
	/* Stop and Free timers that may be started */

   if (p_dds->wrk.time_data.run)
	 tstop(p_dds->wrk.p_bringup_timer);
   tfree(p_dds->wrk.p_bringup_timer);

   if (p_dds->wrk.probate_td.run)
	 tstop(p_dds->wrk.p_probate_timer);
   tfree(p_dds->wrk.p_probate_timer);

   if (p_dds->wrk.recv_limbo_td.run)
	 tstop(p_dds->wrk.p_recv_limbo_timer);
   tfree(p_dds->wrk.p_recv_limbo_timer);

      	/*
       	*  Log unknown error and return if mempory allocation failed
       	*/
   p_dds->wrk.footprint = TOK_XMALL_FAIL;
   logerr(p_dds, ERRID_TOK_ERR15);

   return(ENOBUFS);     /* return errno */
 }
       /* Turn off Absolute timer.  We want an incremental timer */
p_dds->wrk.p_func_timer->flags &= ~(T_ABSOLUTE);

       /* get address of our adapter bringup timer function */
p_dds->wrk.p_func_timer->func = ( void (*) ())bringup_timer;

       /* set priority of timer */
p_dds->wrk.p_func_timer->ipri = INTCLASS2;

       /* get address of the data for the bringup timer function */
p_dds->wrk.p_func_timer->func_data = (unsigned long)
                       &(p_dds->wrk.functional_td);

p_dds->wrk.functional_td.p_dds = (caddr_t) p_dds;
p_dds->wrk.functional_td.run = FALSE;


   /*
   *   Setup the Group Address timer structures
   */

p_dds->wrk.p_group_timer = (struct trb *)talloc();

/* see if memory allocation was successful */
if (p_dds->wrk.p_group_timer == NULL)
{
   	/* Clean up and reset pointers */
   xmdetach( &(p_dds->wrk.mem_block_xmd) );
   p_dds->wrk.p_d_mem_block = NULL;
   xmfree(p_dds->wrk.p_mem_block, pinned_heap);
   (void)d_clear(p_dds->wrk.dma_chnl_id);
 
	/* Stop and Free timers that may be started */

   if (p_dds->wrk.time_data.run)
	 tstop(p_dds->wrk.p_bringup_timer);
   tfree(p_dds->wrk.p_bringup_timer);

   if (p_dds->wrk.probate_td.run)
	 tstop(p_dds->wrk.p_probate_timer);
   tfree(p_dds->wrk.p_probate_timer);

   if (p_dds->wrk.recv_limbo_td.run)
	 tstop(p_dds->wrk.p_recv_limbo_timer);
   tfree(p_dds->wrk.p_recv_limbo_timer);

   if (p_dds->wrk.functional_td.run)
	 tstop(p_dds->wrk.p_func_timer);
   tfree(p_dds->wrk.p_func_timer);

      /*
       *  Log unknown error and return if memory allocation failed
       */
   p_dds->wrk.footprint = TOK_XMALL_FAIL;
   logerr(p_dds, ERRID_TOK_ERR15);

   return(ENOBUFS);     /* return errno */
 }
       /* Turn off Absolute timer.  We want an incremental timer */
p_dds->wrk.p_group_timer->flags &= ~(T_ABSOLUTE);

       /* get address of our adapter bringup timer function */
p_dds->wrk.p_group_timer->func = ( void (*) ())bringup_timer;

       /* set priority of timer */
p_dds->wrk.p_group_timer->ipri = INTCLASS2;

       /* get address of the data for the bringup timer function */
p_dds->wrk.p_group_timer->func_data = (unsigned long)
                       &(p_dds->wrk.group_td);

p_dds->wrk.group_td.p_dds = (caddr_t) p_dds;
p_dds->wrk.group_td.run = FALSE;


  /*
   *   Setup the Ring Information timer structures
   */

p_dds->wrk.p_ring_info_timer = (struct trb *)talloc();

/* see if memory allocation was successful */
if (p_dds->wrk.p_ring_info_timer == NULL)
{
   	/* Clean up and reset pointers */
   xmdetach( &(p_dds->wrk.mem_block_xmd) );
   p_dds->wrk.p_d_mem_block = NULL;
   xmfree(p_dds->wrk.p_mem_block, pinned_heap);
   (void)d_clear(p_dds->wrk.dma_chnl_id);
 
	/* Stop and Free timers that may be started */

   if (p_dds->wrk.time_data.run)
	 tstop(p_dds->wrk.p_bringup_timer);
   tfree(p_dds->wrk.p_bringup_timer);

   if (p_dds->wrk.probate_td.run)
	 tstop(p_dds->wrk.p_probate_timer);
   tfree(p_dds->wrk.p_probate_timer);

   if (p_dds->wrk.recv_limbo_td.run)
	 tstop(p_dds->wrk.p_recv_limbo_timer);
   tfree(p_dds->wrk.p_recv_limbo_timer);

   if (p_dds->wrk.functional_td.run)
	 tstop(p_dds->wrk.p_func_timer);
   tfree(p_dds->wrk.p_func_timer);

   if (p_dds->wrk.group_td.run)
	 tstop(p_dds->wrk.p_group_timer);
   tfree(p_dds->wrk.p_group_timer);

      /*
       *  Log unknown error and return if memory allocation failed
       */
   p_dds->wrk.footprint = TOK_XMALL_FAIL;
   logerr(p_dds, ERRID_TOK_ERR15);

   return(ENOBUFS);     /* return errno */
 }
       /* Turn off Absolute timer.  We want an incremental timer */
p_dds->wrk.p_ring_info_timer->flags &= ~(T_ABSOLUTE);

       /* get address of our adapter bringup timer function */
p_dds->wrk.p_ring_info_timer->func = ( void (*) ())bringup_timer;

       /* set priority of timer */
p_dds->wrk.p_ring_info_timer->ipri = INTCLASS2;

       /* get address of the data for the bringup timer function */
p_dds->wrk.p_ring_info_timer->func_data = (unsigned long)
                       &(p_dds->wrk.ring_info_td);

p_dds->wrk.ring_info_td.p_dds = (caddr_t) p_dds;
p_dds->wrk.ring_info_td.run = FALSE;

 /* ZERO out the control area memory */
 bzero(p_dds->wrk.p_mem_block, PAGESIZE);

/* set the DMA address of the control block area of memory */
/* This will force the use of the last available TCW. */
p_dds->wrk.p_d_mem_block = p_dds->ddi.dma_base_addr +
                                   (NTCW << DMA_L2PSIZE);
p_dds->wrk.dl_tcw_base = p_dds->ddi.dma_base_addr + ((NTCW-1) << DMA_L2PSIZE);

sil = i_disable(INTCLASS2);
p_dds->wrk.adap_state = NULL_STATE;
p_dds->wrk.bugout = NULL;
i_enable(sil);

   return(0);
} /* end function get_mem */

/*--------------------------------------------------------------------*/
/*********************  System Block Setup              ***************/
/*--------------------------------------------------------------------*/

/*
*
*  This function will set up the following in the Adapter Control Area (ACA):
*  Each will be placed in the ACA according to their respective ACA BASE
*  offsets defined in tokdshi.h.
*
*      SCB - System Control Block
*      SSB - System Status Block
*      Product ID        - Address where the Product ID information is stored
*      Adapter Error Log - Location where the adapter will DMA the adapter
*                          error log counters.
*      Ring Information  - Location where the adapter will DMA the adapter
*                          Token-Ring Information.
*      Receive Chain     - Starting location of the Adapter's Receive Chain
*      Transmit Chain    - Starting location of the Adapter's TX chain.
*
*  Each will be placed on an IOCC Cache (64 byte) boundary.  The Bus address
*  of the memory will be calculated and stored in the work area of the DDS.
*
*  INPUT: Pointer to the DDS
*
*  RETURN CODES:
*
*
*/

int sb_setup(dds_t *p_dds)

{
   unsigned short *p_temp; /* used to split the bus address into 2 parts */
   int rc, saved_int_lvl;
   t_tx_list    *p_d_tmp;



p_temp = (unsigned short *) &p_d_tmp;      /* pointer to the TX list pointer */
/* SCB setup */
p_dds->wrk.p_scb =
        (t_scb *)( (int)p_dds->wrk.p_mem_block +
                                     ACA_SCB_BASE);

p_dds->wrk.p_d_scb =
        (t_scb *)( (int)p_dds->wrk.p_d_mem_block +
                                     ACA_SCB_BASE);
p_d_tmp = (t_tx_list *)p_dds->wrk.p_d_scb;

p_dds->wrk.adap_iparms.scb_add1 = *p_temp++;
p_dds->wrk.adap_iparms.scb_add2 = *p_temp--;


/* SSB set up */

p_dds->wrk.p_ssb =
        (t_ssb *)( (int)p_dds->wrk.p_mem_block +
                                     ACA_SSB_BASE);

p_dds->wrk.p_d_ssb =
        (t_ssb *)( (int)p_dds->wrk.p_d_mem_block +
                                       ACA_SSB_BASE);
p_d_tmp = (t_tx_list *)p_dds->wrk.p_d_ssb;

p_dds->wrk.adap_iparms.ssb_add1 = *p_temp++;
p_dds->wrk.adap_iparms.ssb_add2 = *p_temp--;



p_dds->wrk.p_prod_id =
        (tok_prod_id_t *)( (int)p_dds->wrk.p_mem_block +
                                     ACA_PROD_ID_BASE);

p_dds->wrk.p_d_prod_id =
        (tok_prod_id_t *)( (int)p_dds->wrk.p_d_mem_block +
                                       ACA_PROD_ID_BASE);
p_d_tmp = (t_tx_list *)p_dds->wrk.p_d_prod_id;


p_dds->wrk.adap_open_opts.prod_id_addr1 = *p_temp++;
p_dds->wrk.adap_open_opts.prod_id_addr2 = *p_temp--;


/* Adapter Error Log Set up */
p_dds->wrk.p_errlog =
                (tok_adap_error_log_t *)
                        ( (int)p_dds->wrk.p_mem_block +
                                        ACA_ADAP_ERR_LOG_BASE);
p_dds->wrk.p_d_errlog =
                (tok_adap_error_log_t *)
                        ( (int)p_dds->wrk.p_d_mem_block +
                                        ACA_ADAP_ERR_LOG_BASE);

/* Setup Ring Information pointers */

p_dds->wrk.p_ring_info =
                (tok_ring_info_t *)
                        ( (int)p_dds->wrk.p_mem_block +
                                        ACA_RING_INFO_BASE);
p_dds->wrk.p_d_ring_info =
                (tok_ring_info_t *)
                        ( (int)p_dds->wrk.p_d_mem_block +
                                        ACA_RING_INFO_BASE);


p_dds->wrk.p_open_opts =
                (tok_adap_open_options_t *)
                        ( (int)p_dds->wrk.p_mem_block +
                                        ACA_OPEN_BLOCK_BASE);
p_dds->wrk.p_d_open_opts =
                (tok_adap_open_options_t *)
                        ( (int)p_dds->wrk.p_d_mem_block +
                                        ACA_OPEN_BLOCK_BASE);


        /*                                                            */
        /*  Include the system block for component dump table.        */
        /*                                                            */
        cio_add_cdt ("MemBlock", p_dds->wrk.p_mem_block, PAGESIZE);

return(0);
} /* end function sb_setup */
/*--------------------------------------------------------------------*/
/***************        System Block Undo               ***************/
/*--------------------------------------------------------------------*/

int sb_undo(dds_t *p_dds)

{  /* begin function  */

int saved_int_lvl;


        /*
        *  set state machine to note that the ACA is now
        * NOT available to use.
        */
saved_int_lvl = i_disable(INTCLASS2);
p_dds->wrk.adap_state = NULL_STATE;
p_dds->wrk.limbo = PARADISE;
p_dds->wrk.adap_doa = ALIVE_N_WELL;
i_enable(saved_int_lvl);


       /* Reset pointers */
p_dds->wrk.p_scb = NULL;
p_dds->wrk.p_d_scb = NULL;


p_dds->wrk.p_ssb = NULL;
p_dds->wrk.p_d_ssb = NULL;


/* Product ID Information Undo */
p_dds->wrk.p_prod_id = NULL;
p_dds->wrk.p_d_prod_id = NULL;

/* Adapter Error Log Undo */
p_dds->wrk.p_errlog = NULL;
p_dds->wrk.p_d_errlog = NULL;

  /*
   *   Reset the adapter initialization and open parameters
   */

   (void)cfg_adap_parms(p_dds);

/* Ring Information  Undo */
p_dds->wrk.p_ring_info = NULL;
p_dds->wrk.p_d_ring_info  = NULL;

bzero(&(p_dds->wrk.ring_info), sizeof(tok_ring_info_t));
p_dds->wrk.ri_avail = FALSE;

        /*                                                            */
        /*  Remove the system block from the component dump table.    */
        /*                                                            */
        cio_del_cdt ("MemBlock", p_dds->wrk.p_mem_block, PAGESIZE);

return(0);
}  /* end function sb_undo */

/*--------------------------------------------------------------------*/
/***************        Memory Block Undo               ***************/
/*--------------------------------------------------------------------*/

int get_mem_undo(dds_t *p_dds)

{  /* begin function  */

   int rc = 0, sil;


   rc = xmdetach( &(p_dds->wrk.mem_block_xmd) );
       /* Reset pointers */
   p_dds->wrk.p_d_mem_block = NULL;
   xmfree(p_dds->wrk.p_mem_block, pinned_heap);
       /* reset pointers to NULL */
   p_dds->wrk.p_mem_block = NULL;

   sil = i_disable(INTCLASS2);
   p_dds->wrk.adap_state = DEAD_STATE;
   p_dds->wrk.bugout = NULL;
   i_enable(sil);

   /*
   *  TEMP:
   *   This should be temp since the common code does not
   *   allow the DS code to undo what it does in the
   *   XXXdsinitdds() routine.
   */
   (void)d_clear(p_dds->wrk.dma_chnl_id);

   if (p_dds->wrk.time_data.run)
       tstop(p_dds->wrk.p_bringup_timer);

   tfree(p_dds->wrk.p_bringup_timer);


   if (p_dds->wrk.probate_td.run)
      tstop(p_dds->wrk.p_probate_timer);

   tfree(p_dds->wrk.p_probate_timer);

   if (p_dds->wrk.recv_limbo_td.run)
      tstop(p_dds->wrk.p_recv_limbo_timer);

   tfree(p_dds->wrk.p_recv_limbo_timer);

   if (p_dds->wrk.functional_td.run)
      tstop(p_dds->wrk.p_func_timer);

   tfree(p_dds->wrk.p_func_timer);


   if (p_dds->wrk.group_td.run)
      tstop(p_dds->wrk.p_group_timer);

   tfree(p_dds->wrk.p_group_timer);

   if (p_dds->wrk.ring_info_td.run)
      tstop(p_dds->wrk.p_ring_info_timer);

   tfree(p_dds->wrk.p_ring_info_timer);

   return(0);
} /* end function get_mem_undo */
