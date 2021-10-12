static char sccsid[] = "@(#)63  1.26.1.13  src/bos/kernext/disp/ped/rcm/midswitch.c, peddd, bos411, 9428A410j 4/8/94 16:14:06";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: mid_do_switch
 *              mid_end_switch
 *              mid_start_switch
 *              wait_fifo
 *              wait_switch_end
 *
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*****************************************************************************
       Includes:
 *****************************************************************************/

#include <sys/types.h>
#include <sys/syspest.h>
#include <sys/xmem.h>
#include <sys/rcm_win.h>
#include <sys/timer.h>    /* used for TIMER.  temp for debugging */
#include <sys/aixgsc.h>
#include <sys/rcm.h>
#include <sys/intr.h>
#include <sys/display.h>        /* added with font (update of curr process) */
#include <sys/sleep.h>  /* added with font (update of curr process) */
#include <sys/dma.h>    /* added with ctx DMA */
#include "mid_pos.h"            /* must precede midhwa.h */
#include "midhwa.h"        

#include "hw_dd_model.h"
#include "hw_macros.h"
#include "hw_typdefs.h"
#include "hw_regs_k.h"
#include "hw_regs_u.h"
#include "hw_seops.h"   /* CONTEXT_STATE_UPDATE_COMPLETE defined diff places */
                        /* It is also defined in hw_PCBkern.h and hw_PCBrms.h*/
#include "midctx.h"  /* contains definition of DSP status codes */

#include "hw_ind_mac.h"
#include "hw_HCRkern_dd.h"

#include "midddf.h"
#include "midrcx.h"
#include "midRC.h"
#include "midwidmac.h"
#include "mid_rcm_mac.h"        /* for MID_ASSERT */

#include "mid_dd_trace.h"       /* includes disptrc which defines dup symbols */
                                /* POLYPOINT, POLYLINES, POLYSEGMENT */

MID_MODULE (midswitch);



/*****************************************************************************
       Externals:
 *****************************************************************************/

extern mid_start_switch (struct   _gscDev   *pGD,
                         rcxPtr   pRCXold,
                         rcxPtr   pRCXnew,
                         int      *seq_num        ) ;

extern mid_end_switch  (struct   _gscDev   *pGD,
                         rcxPtr   pRCXold,
                         rcxPtr   pRCXnew,
                         int      seq_num        ) ;


extern mid_create_default_context (midddf_t *) ;



#define dbg_middd dbg_midswitch
#define dbg_midpixi dbg_midswitch
#define dbg_SetContext dbg_midswitch



/******************************************************************************

                             Start of Prologue

   Function Name:  mid_start_switch

   Descriptive Name:  Light Context Switch Handler

 *--------------------------------------------------------------------------*

   Overview:

      The RCM does many things, but its' main purpose is to allow any number
      of graphics processes to use the graphics hardware without knowing
      about or interfering with, each other.

      This virtualization of the graphics hardware is accomplished in a
      manner similar to that used to allow system processes to use the
      system CPU without knowing about and interfering with each other:
      each rendering process has an associated state, and no process is
      allowed to monopolize the hardware resources.

      The state in computer graphics is called a context and the graphics
      time slicing is accomplished with assistance from system process
      management, and essentially involves causing the hardware to move
      from one rendering state (or context) to another.  This movement is
      called context switching.


 *--------------------------------------------------------------------------*

   Function:

      This routine is called to attempt a context switch.  Each graphics
      adapter can remember a certain number of rendering contexts.  The
      contexts known to the adapter are referred to as "on the adapter".
      If a context is already on the adapter, then little data needs to
      be transferred to the adapter to perform the context switch (the
      adapter simply needs to be told to use one of the other contexts
      that it already knows about).  This proces is called a light context
      switch because of the small amount of data transferring.

      If a light context switch is sufficient, then this routine will
      in fact, perform the context switch.  If this routine determines
      that the proper context is not on the adapter then that context must
      be transferred (via DMA) to the adapter.  This is called a heavy
      context switch.  The heavy context switch is not handled by this routine.


 *--------------------------------------------------------------------------*

    Restrictions:
                  none

    Dependencies:
                  none


 *--------------------------------------------------------------------------*

    Linkage:

        Context switching can result from three different causes:

        1) A graphics process attempts to write to the virtual memory page
             area that the graphics hardware is associated with. This is
             called a graphics page fault.

        2) A graphics process' GRAPHICS time slice expires with some other
             process waiting to use the hardware.  An example when this
             might occur is when a process has sent all of its rendering
             instructions (to a FIFO structure located on the adapter)
             and is switched out before all of them are read.  The process
             will never cause another graphics page fault, yet its
             instructions must be processed (its FIFO drained).

        3) The graphics hardware sends a FIFO stalled interrupt. This means
             that the current rendering process has sent a request to the
             adapter that will take some time, and no further processing on
             the hardware is possible until this request is fulfilled.
             Rather than have the graphics adapter sit stalled, another
             graphics process is allowed to run.


 *--------------------------------------------------------------------------*

    INPUT:  There are 4 input parameters to the ligt_swtich function:
              . pointer to the virtual terminal structure
              . pointer to the old device independent context structure
              . pointer to the NEW device independent context structure
              . pointer to a sequence number

    OUTPUT:  none

    RETURN CODES:
           - RCM_LIGHT_SWITCH - the light context switch was performed
                                 successfully
           - RCM_HEAVY_SWITCH - a heavy context switch is required


                                 End of Prologue
 ******************************************************************************/



long mid_start_switch (
                       gscDev *pGD,
                       rcx    *pRCX_DIold,
                       rcx    *pRCXnew,
                       int    *seq_num )
{
  struct phys_displays *pd = pGD->devHead.display;
  struct midddf *ddf = (struct midddf *) pd->free_area;

        ulong   bim_status ;
        ulong   mask ;
        ulong   cs_flags ;

        int     our_intr_priority ;
        int     low_water_mark;

        rcx        *last_adapter_RCX ;
        mid_rcx_t  *last_adapter_midRCX ;
        mid_rcx_t  *pmidRCXnew ;
        mid_rcx_t  *pmidRCX_DIold ;

        ushort  old_ctx_handling ;
        ulong   old_context_id ;

        ushort  new_ctx_handling ;
        ulong   newid ;
        char    *old_ctx_addr ;
        ulong   old_ctx_size ;
        char    *new_ctx_addr ;
        ulong   new_ctx_size ;


        rcmWG      *pWG ;
        midWG_t    *pmidWG ;
        mid_wid_t   wid ;

        int           rc ;
#define         _NO     0
#define         _YES    1


  HWPDDFSetup ;



  /*-----------------------------------------------------------------------*
     Get the control blocks for the old context
   *-----------------------------------------------------------------------*/

    last_adapter_midRCX = ddf -> current_context_midRCX ;

    if (last_adapter_midRCX == NULL)
    {
        last_adapter_RCX = NULL ;
    }
    else
    {
        last_adapter_RCX = last_adapter_midRCX -> pRcx ;
    }

    pmidRCXnew = (mid_rcx_t *) pRCXnew -> pData;



  /*-----------------------------------------------------------------------*
     Trace the entry parms
   *-----------------------------------------------------------------------*/

    MID_DD_ENTRY_TRACE (midswitch, 1, SWITCH_RCX, ddf,
                                pGD, pRCXnew, pRCX_DIold, last_adapter_RCX) ;


    BUGLPR(dbg_midswitch, BUGNFO, ("Entering start_switch \n"));

    BUGLPR(dbg_midswitch, BUGNFO,
        ("new RCX = 0x%8X,  old = 0x%8X, last = 0x%8X\n",
         pRCXnew, pRCX_DIold, last_adapter_RCX ));


  /*-----------------------------------------------------------------------*
     ERROR CHECKs for the new context

     The new context pointer must not be NULL.
   *-----------------------------------------------------------------------*/

    MID_ASSERT( (pRCXnew != NULL), SWITCH_RCX, 0xEEEE00F1, pGD, ddf, pRCXnew,
                                                        last_adapter_RCX) ;


    if ( !(ddf->dom_flags & HOT_KEY_IN_PROGRESS) )
    {

  /**************************************************************************
     TRIVIAL CASES

     Check for the following trivial cases:
       . a non-FIFO context (signified by domain number ^= 0).
          No adapter notification is required.

       . a NULL context (signified by the NULL_RCX flag in the new context).
          This should not be confused with the null context pointer.
          No context switch is done.

       . new context already active.
          No context switch is done.

       . The context has already been deleted/deallocated/terminated.
          No context switch is done.

   **************************************************************************/

    if (pRCXnew->domain != MID_FIFO_DOMAIN) /* PCB switch -- nothing to do */
    {
        BUGLPR(dbg_midswitch, 1,("PCB ctx 0x%8X -- no switch \n\n", pRCXnew));
        MID_DD_EXIT_TRACE (midswitch, 1, SWITCH_RCX, ddf,
                                pGD, pRCXnew, last_adapter_RCX, 0xF1 ) ;

        /*----------------------------------------------------------------------
          The following breakpoint is occasionally used to study switches
          to PCB contexts, however, since this is part of normal operations,
          this breakpoint should be used sparingly.
        brkpoint (0xEEEE00F1, pRCXnew, pRCX_DIold, last_adapter_RCX );
         ---------------------------------------------------------------------*/
        return (MID_LIGHT_SWITCH)  ;                    /* very light switch */
    }


    BUGLPR(dbg_midswitch, BUGNFO+1,("New RCX flags: %X \n", pRCXnew ->flags));

    if ( ((pRCXnew ->flags) & RCX_NULL) != 0)
    {
        BUGLPR(dbg_midswitch, 1,("NULL ctx 0x%8X -- no switch \n\n", pRCXnew));
        MID_DD_EXIT_TRACE (midswitch, 1, SWITCH_RCX, ddf,
                                pGD, pRCXnew, last_adapter_RCX, 0xF2 ) ;

        /*----------------------------------------------------------------------
          The following breakpoint is occasionally used to study switches
          to NULL contexts, however, since this is part of normal operations,
          this breakpoint should be used sparingly.
        brkpoint (0xEEEE00F2, pRCXnew, pRCX_DIold, last_adapter_RCX );
         ---------------------------------------------------------------------*/
        return (MID_LIGHT_SWITCH)  ;                    /* very light switch */
    }




  /*--------------------------------------------------------------------------
     CONTEXT ALREADY ACTIVE

     First check if the "new" context is already active.
   -------------------------------------------------------------------------*/


    if ( (pRCXnew == last_adapter_RCX) &&
         ( ((mid_rcx_t *)(pRCXnew->pData))->flags.adapter_knows_context == 1) )
    {
        BUGLPR(dbg_midswitch, BUGNFO, ("context is already active\n"));
        MID_DD_EXIT_TRACE (midswitch, 1, SWITCH_RCX, ddf,
                                pGD, pRCXnew, last_adapter_RCX, 0xF3 ) ;
        return (MID_LIGHT_SWITCH) ;
    }




  /*--------------------------------------------------------------------------
     CONTEXT ALREADY DEALLOCATED (TERMINATED, KILLED, DELETED, IT'S DEAD JIM)

     Check if the "new" context has already been deallocated.
   -------------------------------------------------------------------------*/

    MID_ASSERT( ((pRCXnew->pData) != 0x69), SWITCH_RCX, 0x696900F4, pGD, ddf,
                                                pRCXnew, last_adapter_RCX) ;

    }  /* HOT KEY aviods all these simple cases */




  /**************************************************************************
     DETERMINE IF WE ARE READY

     Since we do not wait synchronously for the adapter to complete its
     portion of a context switch, we do not know (yet) whether, in fact, the
     the adapter finished the last one and is ready for the next.

     The first part of this determination is to check if we are already
     sleeping for the completion of the previous switch.  If so, the
     device independent layer screwed up since it promises not to do this.

   **************************************************************************/

    BUGLPR(dbg_midswitch, 2,("Domain flags: 0x%X \n", ddf -> dom_flags ));

    MID_ASSERT( !((ddf -> dom_flags) & WAITING_FOR_PREVIOUS_COMPLETION),
                SWITCH_RCX, 0xF5, pGD, ddf, pRCXnew, ddf->dom_flags) ;



      /*-----------------------------------------------------------------*
         I/O will occur:  Set up for I/O operations:
          . enable PIO,
          . and save the current bus state.
          . set the BIM access speed to slow to avoid parity errors
       *-----------------------------------------------------------------*/

       PIO_EXC_ON () ;

       MID_SLOW_IO(ddf) ;

        our_intr_priority = pGD -> devHead.display ->
                                interrupt_data.intr.priority - 1 ;

  /*--------------------------------------------------------------------------
     PREVIOUS CONTEXT SWITCH NOT YET FINISHED

     If the previous context switch has not yet finished, just wait for it here.
     Note that there was no previous context switch the first time through, so
     we must be careful not to wait in that case.
   -------------------------------------------------------------------------*/

    if ( last_adapter_midRCX != NULL )
    {
        rc = wait_switch_end (ddf, our_intr_priority) ;

        if ( rc == MID_PREVIOUS_NOT_DONE)
        {
            /*---------------------------------------------------------------*
                Restore the I/O bus.
             *---------------------------------------------------------------*/
            MID_FAST_IO(ddf) ;

            PIO_EXC_OFF () ;


            /*---------------------------------------------------------------*
                Now exit (HEAVY SWITCH)
             *---------------------------------------------------------------*/
            BUGLPR(dbg_midswitch, 1, ("Prev switch NOT DONE YET \n"));

            *(seq_num) = WAITING_FOR_PREVIOUS_COMPLETION ;

            MID_DD_EXIT_TRACE (midswitch, 1, SWITCH_RCX, ddf,
                                pGD, pRCXnew, last_adapter_RCX, 0xF6 ) ;

            return (MID_HEAVY_SWITCH) ;
        }
    }






  /**************************************************************************
     INIT the domain (context switch) flags

     ON:   . "context switch in process"
     OFF:  - almost everything else
   **************************************************************************/

    ddf -> dom_flags &= ~( HEAVY_SWITCHING              |
                           WAITING_FOR_FIFO_AVAILABLE   |
                           SLEEPING_FOR_FIFO_AVAILABLE  |
                           FIFO_ALREADY_AVAILABLE       |

                           WAITING_FOR_PREVIOUS_COMPLETION    |
                           SLEEPING_FOR_PREVIOUS_COMPLETION   |
                           PREVIOUS_SWITCH_ALREADY_COMPLETED  |
                           PREVIOUS_SWITCH_FINALLY_COMPLETED  |

                           CONTEXT_UPDATES_REQUIRED     ) ;

    ddf -> dom_flags |= MID_CONTEXT_SWITCH_IN_PROCESS ;


    BUGLPR(dbg_midswitch, 2, ("dom flags = 0x%4X \n", ddf-> dom_flags));
    BUGLPR(dbg_midswitch, 2,
           ("dom delayed WIDs = 0x%8X \n", ddf->dom_delayed_WID_writes_top));



  /**************************************************************************
     OLD CONTEXT HANDLING

     First figure out what to do with the old context.
     Also, init the old context address and size.

   **************************************************************************/

  old_ctx_addr = NULL;
  old_ctx_size = 0;




  if ( last_adapter_midRCX == NULL )
  {
      old_ctx_handling = MID_NO_OLD_CTX ;
  }

  else if (ddf->dom_flags & HOT_KEY_IN_PROGRESS)
  {

      old_ctx_handling = MID_REMOVE_OLD_CTX_MAX_LENGTH ;
      pmidRCX_DIold = (mid_rcx_t *)pRCX_DIold -> pData ;
      old_context_id = pmidRCX_DIold ;

    /* HOT key case - Set up dmaster for getting context off adapter */
    /* THis could eventually be made into a subroutine.              */
    {
                BUGLPR(dbg_midswitch, 1, ("Hotkey.\n"));

            /*--------------------------------------------------------------
                Initialize the information required for host initiated
                DMA of contexts in ddf.  This information will later be used
                for d_complete.

                Perform the d_master operation to setup for DMA.
            ---------------------------------------------------------------*/

                ddf -> ctx_DMA_HI_sys_addr = pmidRCX_DIold-> hw_rcx ;
                ddf -> ctx_DMA_HI_midRCX = pmidRCX_DIold ;

                ddf -> ctx_DMA_HI_bus_addr =
                                ddf->ctx_DMA_host_init_base_bus_addr +
                                (((ulong) (ddf-> ctx_DMA_HI_sys_addr) & 0xFFF));

                ddf -> ctx_DMA_HI_xs = &(pmidRCX_DIold-> xs) ;

             /*------------------------------------------------------------
                size is the maximum size of the context and NOT the
                actual size of the context on the adapter.  However, the
                adapter remembers the actual size and transfers that
                amount.
             --------------------------------------------------------------*/

                ddf -> ctx_DMA_HI_length = pmidRCX_DIold-> size ;
                ddf -> ctx_DMA_HI_flags = DMA_READ | DMA_NOHIDE;

                old_ctx_addr = ddf-> ctx_DMA_HI_bus_addr;
                old_ctx_size = ddf -> ctx_DMA_HI_length;

                BUGLPR(dbg_midswitch, BUGNTA, ("Calling d_master (\n"));
                BUGLPR(dbg_midswitch, 1, ("sys_addr %x \n",ddf ->
                                            ctx_DMA_HI_sys_addr));
                BUGLPR(dbg_midswitch,1, ("len  %d\n",ddf -> ctx_DMA_HI_length));

                d_master (
                        ddf -> ctx_DMA_HI_channel,
                        ddf -> ctx_DMA_HI_flags,
                        ddf -> ctx_DMA_HI_sys_addr,
                        ddf -> ctx_DMA_HI_length,
                        ddf -> ctx_DMA_HI_xs,
                        ddf -> ctx_DMA_HI_bus_addr
                 ) ;

                /*------------------------------------------------------
                   Set the flag to indicate that the host initiated
                   DMA of a context.
                -------------------------------------------------------*/

                ddf -> dom_flags |= HOST_INITIATED_CTX_DMA_IN_PROCESS;

                BUGLPR(dbg_midswitch, 2,
                        ("dom flags = 0x%4X \n", ddf-> dom_flags));
                BUGLPR(dbg_midswitch, BUGNFO,("New context NOT on adapter.\n"));
     }               /* end of context already known to the adapter */

  }

  else if ( (last_adapter_midRCX -> flags.terminate_context == 1)  ||
            (last_adapter_midRCX -> flags.default_context == 1) )
  {
      last_adapter_midRCX -> flags.context_on_adapter = 0 ;
      last_adapter_midRCX -> flags.adapter_knows_context = 0 ;

      old_ctx_handling = MID_TERMINATE_OLD_CTX ;
      old_context_id = last_adapter_midRCX ;
  }
  else /* Leave the blasted thing on the adapter */
  {

      old_ctx_handling = MID_SAVE_OLD_CTX_ON_ADAPTER ;
      old_context_id = last_adapter_midRCX ;


      BUGLPR(dbg_midswitch, 4,
            ("There was an OLD context (that lived in a shoe).\n"));
  }


  BUGLPR(dbg_midswitch, 2, ("OLD context fate = %d.\n",old_ctx_handling));





  /**************************************************************************
     NEW CONTEXT HANDLING

     Now figure out what to do with the new context.
   **************************************************************************/

  if ( pRCXnew == NULL )
  {
      BUGLPR(dbg_midswitch, BUGNFO+2, ("No NEW context\n"));
      /*-----------------------------------------------------------------*
         NO NEW CONTEXT
       *-----------------------------------------------------------------*/

      new_ctx_handling = MID_NO_NEW_CTX ;

      if ( old_ctx_handling != MID_TERMINATE_OLD_CTX &&
           !(ddf->dom_flags  &  HOT_KEY_IN_PROGRESS) )
      {
           BUGLPR(dbg_midswitch, 1, ("ERROR:  No NEW context specified \n"));
           return (-1) ;
      }
  }


  else
  {
      /*-----------------------------------------------------------------*
         NON-NULL NEW CONTEXT (normal case - switch will occur)

         We will:
           . ensure the window has a valid WID (usable for render)
           . determine the "new context handling" required by the adapter
       *-----------------------------------------------------------------*/

       /*-----------------------------------------------------------------*
           Ensure the window has a valid WID (usable for render).
           This function MUST NOT write the WID planes from this path.
           The WID plane writes are inhibited by the "context switch in
           process" flag set earlier in this routine.

           Then set the processing flag indicating whether new context
           updates are required or not.
       *-----------------------------------------------------------------*/

        if ( (pmidRCXnew-> flags.default_context == 0) &&
             (pmidRCXnew-> type != RCX_2D) )
        {
            BUGLPR(dbg_midswitch, 1, (" Not def or 2D ctx getting wid \n"));

            pWG = pRCXnew -> pWG ;
            if (pWG != NULL)
            {
                pmidWG = ((midWG_t *)(pWG -> pPriv))  ;
                pmidWG->pRegion = pWG->wg.pClip ;

                rc = get_WID (MID_GET_WID_FOR_SWITCH, pGD, pWG, pRCXnew) ;

                if ( (rc == MID_NEW_WID) ||
                     (pmidRCXnew-> flags.geometry_changed == 1)  ||
                     (pmidRCXnew-> flags.adapter_knows_context == 0) )
                {
                        ddf -> dom_flags |= CONTEXT_UPDATES_REQUIRED ;

                        if ( !(pmidWG-> wgflags & MID_WID_WRITE_DELAYED) )
                        {
                                pmidWG -> wgflags |= MID_WID_WRITE_DELAYED ;
                                if (ddf->dom_delayed_WID_writes_top == NULL)
                                {
                                    ddf->dom_delayed_WID_writes_top = pmidWG ;
                                }
                                else
                                {
                                    ddf->dom_delayed_WID_writes_bot->
                                        pNext_ctx_sw_delay = pmidWG ;
                                }

                                ddf->dom_delayed_WID_writes_bot = pmidWG ;
                                pmidWG-> pNext_ctx_sw_delay = NULL ;
                        }
                }       /* new WID or new context */
            }
        }

        BUGLPR(dbg_midswitch, 2, ("dom flags = 0x%4X \n", ddf-> dom_flags));
        BUGLPR(dbg_midswitch, 2,
            ("dom delayed WIDs = 0x%8X \n", ddf-> dom_delayed_WID_writes_top));




        /*-----------------------------------------------------------------*
            Init the new context address and size
        *-----------------------------------------------------------------*/
        new_ctx_addr = 0 ;
        new_ctx_size = 0 ;

        /*-----------------------------------------------------------------*
            Get addressing to the mid context struct (for the new context)
        *-----------------------------------------------------------------*/
        BUGLPR(dbg_midswitch, 5, ("New context type = %d, flags = %4X\n",
                                   pmidRCXnew->type, pmidRCXnew->flags));




        if ( (pmidRCXnew -> flags.adapter_knows_context == 1 ) )
        {
                BUGLPR(dbg_midswitch, BUGNFO+2, ("Old new context.\n"));


            /*----------------------------------------------------------------*
               KNOWN CONTEXT

                  This section inits new request handling when the context has
                  already been on the adapter at least once.
             *----------------------------------------------------------------*/

            if ( pmidRCXnew -> flags.context_on_adapter == 0 )
            {
                BUGLPR(dbg_midswitch, 1, ("context not on adapter.\n"));
                new_ctx_handling = MID_RESTORE_NEW_CTX ;

            /*--------------------------------------------------------------
                Initialize the information required for host initiated
                DMA of contexts in ddf.  This information will later be used
                for d_complete.

                Perform the d_master operation to setup for DMA.
            ---------------------------------------------------------------*/

                ddf -> ctx_DMA_HI_midRCX = pmidRCXnew ;
                ddf -> ctx_DMA_HI_sys_addr = pmidRCXnew-> hw_rcx ;

                ddf -> ctx_DMA_HI_bus_addr =
                                ddf->ctx_DMA_host_init_base_bus_addr +
                                (((ulong) (ddf-> ctx_DMA_HI_sys_addr) & 0xFFF));

                ddf -> ctx_DMA_HI_xs = &(pmidRCXnew-> xs) ;

             /*------------------------------------------------------------
                size is the maximum size of the context and NOT the
                actual size of the context on the adapter.  However, the
                adapter remembers the actual size and transfers that
                amount.
             --------------------------------------------------------------*/

                ddf -> ctx_DMA_HI_length = pmidRCXnew-> size ;
                ddf -> ctx_DMA_HI_flags = DMA_READ | DMA_NOHIDE;

                new_ctx_addr = ddf-> ctx_DMA_HI_bus_addr;
                new_ctx_size = ddf -> ctx_DMA_HI_length;

                BUGLPR(dbg_midswitch, BUGNTA, ("Calling d_master (\n"));
                BUGLPR(dbg_midswitch, 1, ("sys_addr %x \n",ddf ->
                                            ctx_DMA_HI_sys_addr));
                BUGLPR(dbg_midswitch,1, ("len  %d\n",ddf -> ctx_DMA_HI_length));

                d_master (
                        ddf -> ctx_DMA_HI_channel,
                        ddf -> ctx_DMA_HI_flags,
                        ddf -> ctx_DMA_HI_sys_addr,
                        ddf -> ctx_DMA_HI_length,
                        ddf -> ctx_DMA_HI_xs,
                        ddf -> ctx_DMA_HI_bus_addr
                 ) ;

                /*------------------------------------------------------
                   Set the flag to indicate that the host initiated
                   DMA of a context.
                -------------------------------------------------------*/

                ddf -> dom_flags |= HOST_INITIATED_CTX_DMA_IN_PROCESS;

                BUGLPR(dbg_midswitch, 2,
                        ("dom flags = 0x%4X \n", ddf-> dom_flags));
                BUGLPR(dbg_midswitch, BUGNFO,("New context NOT on adapter.\n"));
            }
            else
            {
                BUGLPR(dbg_midswitch, 1, ("context on adapter.\n"));
                new_ctx_handling = MID_SWITCH_TO_NEW_CTX ;

                BUGLPR(dbg_midswitch, BUGNFO, ("New context IS on adapter.\n"));
            }                   /* end of known context still on adapter */
        }               /* end of context already known to the adapter */




        else
        {
            /*----------------------------------------------------------------*
                NEW CONTEXT

                The context has never been on the adapter.
            *-----------------------------------------------------------------*/

            new_ctx_handling = MID_INITIATE_NEW_CTX ;

            BUGLPR(dbg_midswitch, BUGNFO,
                ("NEW Context type = %d \n",pmidRCXnew->type)) ;

        }  /* end of new new context */
    } /* end of non-NULL new context */




     /*********************************************************************
      *********************************************************************

        By this time, we have already set up parameters necessary to tell
        the adapter just what it can do with those contexts.

          OLD CONTEXT PARAMETERS
          . Old Context Handling = old_ctx_handling variable
          . other old context parameters are not applicable


          NEW CONTEXT PARAMETERS
          . New Context Handling = new_ctx_handling variable
          . New Context Type = type from dev dep context structure
          . low water must be larger than the amount of space freed
                by the adapter (at least a typical SE),
                This may also be context type dependent.
          . high water = 32 (= 2 x 16 word store-multiples),
          . context ID = device dependent context structure address
          . other new context parameters are not applicable

      *********************************************************************
      *********************************************************************/


      /*-----------------------------------------------------------------*
         Clear the old FIFO, to ensure WID usage is in sync
       *-----------------------------------------------------------------*/

       sync_WID_requests (ddf) ;



      /*-----------------------------------------------------------------*
         Disable interrupts so the interrupt handler doesn't get control
         and process (and reset) the "FIFO available" status which is
         returned when the adapter recognizes the Context Switch and sets
         "Context switch in process".
       *-----------------------------------------------------------------*/

       mask = ddf->host_intr_mask_shadow | MID_K_HOST_IO_CHANNEL_CHECK ;
       ddf->host_intr_mask_shadow = 0 ;
       MID_WR_HOST_INTR (0) ;


       switch ( pmidRCXnew->type )
       {
            case RCX_2D:        low_water_mark = RCX_2D_LWM_MARK   ; break ;
            case RCX_3DM1:      low_water_mark = RCX_3DM1_LWM_MARK ; break ;
            case RCX_3DM2:      low_water_mark = RCX_3DM2_LWM_MARK ; break ;
       }

       newid = pmidRCXnew ;         /* use the mid RCX address for the ID */

      /*-----------------------------------------------------------------*
       setup the context flags for (low water / hi water) state
       *-----------------------------------------------------------------*/

       if ( (pmidRCXnew-> flags.waiting_for_FIFO_low_water_mark ) &&
           !(pmidRCXnew-> flags.low_water_condition_pending) )
       {
                cs_flags = MID_SC_UPDATES_REQUIRED |
                           MID_SC_WAITING_FOR_LOW_WATER ;
       }
       else
       {
                cs_flags = MID_SC_UPDATES_REQUIRED ;
       }

        MID_DD_TRACE_PARMS (midswitch, 0, START_SWITCH_FATES, ddf, pGD,
                                ((old_ctx_handling) << 16) + new_ctx_handling,
                                cs_flags, newid ) ;


       MID_SetContext_SE ( ddf, cs_flags,
                old_ctx_handling, new_ctx_handling,
                old_context_id,
                old_ctx_addr, old_ctx_size,      /* old ctx addr,leng*/
                newid, pmidRCXnew->type,        /* new ctx ID, type  */
                new_ctx_addr,  new_ctx_size,    /* new ctx addr, len */
                64, low_water_mark);
#if 0
                HIGH_WATER_THRESHOLD, low_water_mark);
#endif

       BUGLPR(dbg_midswitch, BUGNFO, ("Context CCB written \n") );


      /********************************************************************

         Now wait up to 20 microseconds for FIFO available status.  This
         status becomes valid when the "Context switch in process" bit is set.

      *********************************************************************/
       wait_fifo (ddf) ;


      /*-----------------------------------------------------------------*
        Need to reset any pending high or low water interrupts at this time
        to ensure we start in synch with the adapter.
       *-----------------------------------------------------------------*/

       MID_WR_HOST_STAT ( MID_K_HOST_IO_LOW_WATER_MARK |
                          MID_K_HOST_IO_PIO_HI_WATER_HIT ) ;



        /*******************************************************************

           BOOKKEEPING

           Set flags indicating the context is on the adapter.
           Also, update the last context and last process fields.

        *******************************************************************/

       pmidRCXnew -> flags.adapter_knows_context = 1 ;
       pmidRCXnew -> flags.context_on_adapter = 1 ;

       BUGLPR(dbg_midswitch, 4, ("Flags = %4X.\n",pmidRCXnew->flags) );


        if ( ( (last_adapter_RCX != NULL) &&
               (last_adapter_midRCX -> flags.terminate_context == 0) &&
               (last_adapter_midRCX -> flags.default_context == 0) ) )
        {
                ddf -> previous_context_RCX = last_adapter_RCX ;
        }
        else
        {
                ddf -> previous_context_RCX = NULL ;
        }


        ddf -> current_context_midRCX = pmidRCXnew ;
        pGD -> devHead.display -> cur_rcm = pRCXnew -> pProc ;




        /*----------------------------------------------------------*
          Check if we have missed a LOW WATER interrupt.
        *-----------------------------------------------------------*/

        if ((pmidRCXnew-> flags.waiting_for_FIFO_low_water_mark ) &&
           !(pmidRCXnew-> flags.low_water_condition_pending) )
        {
            if ( pmidRCXnew-> low_water_switch_count > LOW_WATER_SWITCH_MAX)
            {
                mid_low_water_com (ddf, pmidRCXnew) ;
                w_stop (&(ddf->mid_lowwater_watchdog.dog)) ;

                MID_DD_TRACE_PARMS (midswitch, 1, LOW_WATER_WATCHDOG, ddf, pGD,
                                pRCXnew, *(ulong *)(&(pmidRCXnew -> flags)),
                                0xEEEE0064 ) ;
            }

            else    /* context waiting for LW */
            {       /* -- restart watchdog timer and incr switch count */

                pmidRCXnew-> low_water_switch_count++ ;
                w_start (&(ddf->mid_lowwater_watchdog.dog)) ;
            }
        }
        else    /*  context switching on is not waiting for low water */
        {                       /* so stop the watchdog(s) */

                w_stop (&(ddf->mid_lowwater_watchdog.dog)) ;
        }



        /************************************************************

         MODULE EXIT -- Set up BIM status mask value

           Do the special processing reuqired for heavy or light switches
           before exitting through the common exit.  The special processing
           entails:
                . setting up the proper BIM mask register value to write.
                . setting up the proper return code.

        *************************************************************/

        if ( ddf -> dom_flags & HEAVY_SWITCHING )
        {
                /*------------------------------------------------------------*
                 While the adapter is working on the context switch, we mask
                 high water interrupts, so avoid the known spurious one, and
                 low water interrupts, to postpone them until the new context
                 is swapped in.
                 The mask is actually written below in common code.
                *-------------------------------------------------------------*/
                mask &= ( ~MID_K_HOST_IO_LOW_WATER_MARK &
                                ~MID_K_HOST_IO_PIO_HI_WATER_HIT ) ;

                *(seq_num) = WAITING_FOR_FIFO_AVAILABLE ;

                BUGLPR (dbg_midswitch, 1, ("EXIT 0: HEAVY switch\n\n"));
                rc = (MID_HEAVY_SWITCH) ;
        }

        else    /* Light switch EXIT */
        {
                /*----------------------------------------------------------*
                 First check if we received the low water condition when the
                 context was previously switched out.
                *-----------------------------------------------------------*/

                if (pmidRCXnew-> flags.low_water_condition_pending)
                {
                    mid_low_water_com (ddf, pmidRCXnew) ;
                }


                /*----------------------------------------------------------*
                 Before writing the interrupt mask we will update it to
                 include the correct settings of the high and low water mark
                 interrupt bits.
                *-----------------------------------------------------------*/

                if ( pmidRCXnew -> flags.waiting_for_FIFO_low_water_mark )
                {
                        mask |= MID_K_HOST_IO_LOW_WATER_MARK ;
                        mask &= ~MID_K_HOST_IO_PIO_HI_WATER_HIT ;
                }
                else
                {
                        mask &= ~MID_K_HOST_IO_LOW_WATER_MARK ;
                        mask |= MID_K_HOST_IO_PIO_HI_WATER_HIT ;
                }


                BUGLPR (dbg_midswitch, 1, ("EXIT 0: LIGHT switch \n\n"));
                rc = MID_LIGHT_SWITCH ;
        }

        /************************************************************
         COMMON MODULE EXIT

           The mask has been calculated -- now write it out.
         *************************************************************/

        ddf->host_intr_mask_shadow = mask ;
        MID_WR_HOST_INTR (mask) ;


        /*******************************************************************
           Now re-enable adapter interrupts and clean up I/O access.
        *******************************************************************/

        MID_FAST_IO(ddf) ;

        PIO_EXC_OFF () ;

        /*------------------------------------------------------------*
         Trace (or printf) the interrupt mask just written.
        *-------------------------------------------------------------*/
        MID_DD_TRACE_PARMS (midswitch, 3, START_SWITCH_INT_MASK, ddf,
                                pGD, *(ulong *)(&(pmidRCXnew -> flags)),
                                mask,
                                ddf -> dom_flags ) ;

        BUGLPR(dbg_midswitch, 2, ("Interrupt mask written = 0x%4X\n", mask));

        /************************************************************
         EXIT
         Trace and leave (with proper return code)
        *************************************************************/

        BUGLPR(dbg_midswitch, 2, ("Context switch is complete\n"));
        MID_DD_EXIT_TRACE (midswitch, 2, SWITCH_RCX, ddf,
                                pGD, pRCXnew,
                                (ddf->current_context_midRCX)->pRcx, rc) ;
        return (rc) ;
}




/******************************************************************************

                               Start of Prologue

   Function Name:    wait_switch_end

   Descriptive Name:  Mid-level Graphics Adapter, Device Dependent
                        Context switch subroutine to wait for the previous
                         context switch to finish

 *--------------------------------------------------------------------------*

   Function:
      Reads the adapter status.  If the adapter is not yet finished with the
      previous context switch, we will return a heavy switch.
      The heavy switch routine will do whatever is necessary (possibly sleep)
      to wait for the previous completion.

 *--------------------------------------------------------------------------*

    Restrictions:
                  none

    Dependencies:
         1. The caller has set the BIM speed to slow.


 *--------------------------------------------------------------------------*

   Linkage:

 *--------------------------------------------------------------------------*

    INPUT:
            . a pointer to the ddf

    OUTPUT:  none

    RETURN CODES:  none



                                 End of Prologue
 ******************************************************************************/

#define DISABLE_INTERRUPTS 1

wait_switch_end (       midddf_t   *ddf ,
                        int         our_intr_priority
                )
{

        ulong    i ;
        int     saved_intr_priority ;

        ulong     status ;
        ulong     int_mask ;

        HWPDDFSetup ;

#define MID_K_HOST_IO_CONTEXT_SWITCH_COMPLETED  MID_K_HOST_IO_DSP_SOFT_INTR0

        BUGLPR (dbg_midswitch, 1, ("Top of wait_switch_end  \n") );
        BUGLPR (dbg_midswitch, 1, ("dom flags = %4X \n", ddf->dom_flags) );

        if ( (ddf -> dom_flags) & PREVIOUS_SWITCH_FINALLY_COMPLETED )
        {
                /***********************************************************
                    In this event, we have already slept for the switch
                    completion.  Since it has already been processed as
                    an interrupt, we have merely set a flag (in the heavy
                    switch routine) to record it's occurance, so we can
                    avoid the I/O here.
                    The status bit we are most concerned about resetting
                    is the FIFO_AVAILABLE status.  The ucode cannot reset
                    this flag - it MUST be reset by the device driver.
                    This flag is valid in wait_fifo when the ucode specifically
                    sets it during CONTEXT_SWITCH_IN_PROGRESS
                 ***********************************************************/
                MID_DD_TRACE_PARMS (midswitch, 1, WAIT_SWITCH_END, ddf,
                                        ddf, 0, ddf->dom_flags, 0xF0 ) ;

                MID_WR_HOST_STAT (
                                MID_K_HOST_IO_CONTEXT_SWITCH_COMPLETED |
                                 MID_K_HOST_IO_HAS_DSP_READ_COMMO   |
                                  MID_K_HOST_IO_PIO_FIFO_AVAILABLE );

                return (MID_RC_OK) ;
        }



#ifdef DISABLE_INTERRUPTS
        /*******************************************************************
           Disable interrupts
        *******************************************************************/
        saved_intr_priority = i_disable (our_intr_priority) ;
#endif



        /*******************************************************************
           READ STATUS
        *******************************************************************/


        MID_RD_HOST_STAT (status) ;
        BUGLPR(dbg_midswitch, 2, ("Status read = %4X \n", status ) );


        if ( (status & MID_K_HOST_IO_CONTEXT_SWITCH_COMPLETED) ==
                       MID_K_HOST_IO_CONTEXT_SWITCH_COMPLETED  )
        {
                /*---------------------------------------------------*
                   Context Switch completed has been read.
                   Now reset it, and exit.
                 *---------------------------------------------------*/
                BUGLPR(dbg_midswitch, 2, ("Ctx switch completed \n") );

                MID_WR_HOST_STAT (
                                MID_K_HOST_IO_CONTEXT_SWITCH_COMPLETED |
                                (status &
                                 (MID_K_HOST_IO_HAS_DSP_READ_COMMO   |
                                  MID_K_HOST_IO_PIO_FIFO_AVAILABLE) ));

#ifdef                          DISABLE_INTERRUPTS
                i_enable (saved_intr_priority) ;
#endif


                /*---------------------------------------------------*
                   Exit
                 *---------------------------------------------------*/
                BUGLPR(dbg_midswitch,2,("Reset BIM CTX switch completed \n") );
                BUGLPR(dbg_midswitch,3, ("dom flags = %4X\n", ddf->dom_flags));


                MID_DD_TRACE_PARMS (midswitch, 1, WAIT_SWITCH_END, ddf,
                                        ddf, status, ddf->dom_flags, 0xF1 ) ;

                return (MID_RC_OK) ;
        }





        /*-----------------------------------------------------------*
            ADAPTER NOT FINISHED with PREVIOUS context switch yet !!!
         *-----------------------------------------------------------*/
        BUGLPR(dbg_midswitch, 2, ("Ctx switch NOT completed \n") );


        /*-----------------------------------------------------------*
            WAIT FOR THE ADAPTER TO FINISH (let it tell us (via interrupt)
                                             when it is finished)

            Read the current interrupt mask and turn on the mask bit for
            "Context Switch complete" to provide an interrupt when the adapter
            finishes.
            Also set a flag to indicating the domain is in this state.
         *-----------------------------------------------------------*/

        /*-----------------------------------------------------------*
            Read the adapter (BIM) interrupt mask
         *-----------------------------------------------------------*/
        int_mask = ddf->host_intr_mask_shadow ;
        BUGLPR(dbg_midswitch, 2,
                        ("interrupt shadow mask read = %4X \n", int_mask ) );

        int_mask = int_mask | MID_K_HOST_IO_CONTEXT_SWITCH_COMPLETED ;

        ddf->host_intr_mask_shadow = int_mask ;
        MID_WR_HOST_INTR (int_mask) ;
        BUGLPR(dbg_midswitch, 2,
                        ("Host interrupt mask written = %4X \n", int_mask ) );



        /*-----------------------------------------------------------*
            Set the domain flag indicating that we are waiting for
             the previous switch to complete.
         *-----------------------------------------------------------*/
        ddf -> dom_flags |= WAITING_FOR_PREVIOUS_COMPLETION ;


#ifdef                  DISABLE_INTERRUPTS
        i_enable (saved_intr_priority) ;
#endif

        MID_DD_TRACE_PARMS (midswitch, 0, WAIT_SWITCH_END, ddf, ddf,
                int_mask, ddf->dom_flags, ddf->current_context_midRCX->pRcx) ;

        /* brkpoint (0xEEEE0741, 0, ddf->dom_flags, int_mask) ; */

        return (MID_PREVIOUS_NOT_DONE) ;

}






/******************************************************************************

                               Start of Prologue

   Function Name:    wait_fifo

   Descriptive Name:  Mid-level Graphics Adapter, Device Dependent
                        Context switch subroutine to wait for FIFO available

 *--------------------------------------------------------------------------*

   Function:
      Wait up to twenty microseconds for the adapter to post new FIFO
      available status.

      The adapter promises to post new FIFO available status within 20 usecs
      of receiving a new Set Context HCR command.  The adapter makes this
      new status available in the BIM status register, howver, it is not
      valid until the "Context switch in progress" bit is set (byt the adapter).

      Therefore, this routine loops on the BIM status, until "context switch
      in progress" is set.  The new "FIFO available" status is then valid.


 *--------------------------------------------------------------------------*

    Restrictions:
                  none

    Dependencies:
         1. The caller has set the BIM speed to slow.


 *--------------------------------------------------------------------------*

   Linkage:

 *--------------------------------------------------------------------------*

    INPUT:
            . a pointer to the ddf

    OUTPUT:  none

    RETURN CODES:  none



                                 End of Prologue
 ******************************************************************************/


wait_fifo ( midddf_t   *ddf )
{

        ulong    i ;


/*---------------------------------------------------------------------------*
    Note:

        Much experimentation was performed on the following loop counter.
        It was discovered that occasionally the adapter could NOT respond
        within about 20 milliseconds!  Presumably this is due to the processing
        required on the particular SE the adapter is working on when the
        context switch is received.

 *----------------------------------------------------------------------------*/

#define TWENTY_USECS    20

        ulong     status ;
        ulong     reset_status ;



/* #define TIMER */
#ifdef TIMER
                struct timestruc_t ct;
                long   time_stamp1;
                long   elapsed_time;
#endif

        HWPDDFSetup ;





        BUGLPR (dbg_midswitch, 1, ("Top of wait_fifo  \n") );

#ifdef TIMER
        ct.tv_nsec = 0;
        time_stamp1 = 0;
        elapsed_time = 0;
        curtime(&ct);
        time_stamp1 = ct.tv_nsec;
#endif


        /*******************************************************************
           READ STATUS loop
        *******************************************************************/

        i = 0 ;

        while (1)
        {
                MID_RD_HOST_STAT (status) ;

                if ( (status & MID_K_HOST_IO_CONTEXT_SWITCH_IN_PROG) ==
                               MID_K_HOST_IO_CONTEXT_SWITCH_IN_PROG  )
                {
                        /*---------------------------------------------------*
                           Context Switch in process has been read.
                           Now reset it, and check for FIFO available.

                           Note that several other status bits may also be
                           reset at this time, if they were set.
                         *---------------------------------------------------*/
                        BUGLPR(dbg_midswitch, 2,
                        ("status %4X  read after %d interations \n", status,i));

                        reset_status =
                                 MID_K_HOST_IO_CONTEXT_SWITCH_IN_PROG |
                                (status &
                                         (MID_K_HOST_IO_HAS_DSP_READ_COMMO   |
                                          MID_K_HOST_IO_PIO_FIFO_AVAILABLE   |
                                          MID_K_HOST_IO_LOW_WATER_MARK       |
                                          MID_K_HOST_IO_PIO_HI_WATER_HIT  )) ;

                        MID_WR_HOST_STAT ( reset_status ) ;


                        BUGLPR(dbg_midswitch, 2,
                                ("Reset BIM status : 0x%4X \n", reset_status));


                        /*---------------------------------------------------*
                            Check for FIFO available: if set, we must wait
                            in heavy switch.
                         *---------------------------------------------------*/
                        if ( (status & MID_K_HOST_IO_PIO_FIFO_AVAILABLE) !=
                                       MID_K_HOST_IO_PIO_FIFO_AVAILABLE   )
                        {
                                ddf -> dom_flags |= HEAVY_SWITCHING ;
                                ddf -> dom_flags |= WAITING_FOR_FIFO_AVAILABLE;
                        }

                        BUGLPR(dbg_midswitch, 1,
                                ("dom flags = 0x%4X \n", ddf -> dom_flags));

                        MID_DD_TRACE_PARMS (midswitch, 1, WAIT_FIFO, ddf, ddf,
                                                i, status, reset_status ) ;
                        return (0) ;
                }

                /*---------------------------------------------------*
                    Status not yet returned, so iterate the loop.
                    First, though, check if we've spun too long.
                 *---------------------------------------------------*/
                i++ ;

                if ( i > TWENTY_USECS )
                {
                        ddf -> dom_flags |= HEAVY_SWITCHING ;
                        ddf -> dom_flags |= WAITING_FOR_FIFO_AVAILABLE ;

#ifdef TIMER
                        curtime(&ct);
                        elapsed_time = ct.tv_nsec - time_stamp1;

                        BUGLPR(dbg_midswitch, 0,
                                ("ct.tv_nsec = 0x%x  time_stamp1 = 0x%x\n",
                                ct.tv_nsec, time_stamp1 ) );
                        BUGLPR(dbg_midswitch, 0,
                                ("wait_fifo elapsed time = %d nanoseconds. \n",
                                elapsed_time ) );
#endif

                        BUGLPR(dbg_midswitch, 1, ("Context Switch hung waiting"
                                                  " for FIFO available \n") );

                        /* brkpoint (0xEEEE0020) ; */

                        MID_DD_TRACE_PARMS (midswitch, 1, WAIT_FIFO, ddf, ddf,
                                        TWENTY_USECS, status, ddf->dom_flags) ;
                        return (-1) ;
                }
       }


        MID_DD_TRACE_PARMS (midswitch, 0, WAIT_FIFO, ddf, ddf,
                                        TWENTY_USECS, status, ddf->dom_flags) ;

        BUGLPR (dbg_midswitch, 1, ("Bottom of wait_fifo\n\n") );
}




/******************************************************************************/
/*
/*                             Start of Prologue
/*
/* Function Name:  mid_end_switch
/*
/* Descriptive Name:  Heavy_Context Switch Handler
/*
/*--------------------------------------------------------------------------*/
/*
/* Function:
/*
/*    This routine is called to perform a heavy context switch.  A heavy
/*    context switch is attempted only if a light context switch is not
/*    possible.  The reader may wish to refer to the light context switch
/*    function -- mid_start_switch for more background on the difference
/*    between the two types of context switches.
/*
/*    The primary difference between a heavy and a light context switch,
/*    is that a heavy context switch requires a large data transfer (the
/*    entire context - several Kbytes) to the adapter.  The light context
/*    switch does not.
/*
/*--------------------------------------------------------------------------*/
/*
/*  Linkage:
/*
/*    This routine performs a large amount of I/O in order to transfer the
/*    new context to the adapter.  As such in cannot run at the interrupt
/*    level.  It is, however, scheduled from the interrupt level to a
/*    special process that handles this long I/O bound context switch.
/*
/*
/*--------------------------------------------------------------------------*/
/*
/*  INPUT:
/*         - a pointer to the graphics device structure
/*         - a pointer to the new rendering context
/*
/*  OUTPUT:
/*         - a sequence number (required to keep binds and context
/*                              switches in synchronization)
/*
/*  RETURN CODES:
/*         - none
/*
/*                               End of Prologue                              */
/******************************************************************************/



long mid_end_switch(pGD, pRCXold, pRCXnew, seq_num)
struct _gscDev *pGD;
rcxPtr  pRCXold;
rcxPtr  pRCXnew;
int seq_num;
{

  struct phys_displays *pd = pGD->devHead.display;
  struct midddf *ddf = (struct midddf *) pd->free_area;

  mid_rcx_t  *pmidRCXnew ;

  int           mask ;

  int           our_intr_priority ;
  int           saved_intr_priority ;

  ulong       rc ;

  int           switch_parm ;


  /**************************************************************************
       Set up the pointer to the bus address (in ddf),
   **************************************************************************/
  HWPDDFSetup ;



        pmidRCXnew = (mid_rcx_t *) pRCXnew -> pData;

        MID_DD_ENTRY_TRACE (midswitch, 1, END_SWITCH, ddf, pGD,
                        seq_num, *((ulong *)(&(pmidRCXnew -> flags))),
                        ddf -> dom_flags) ;

        BUGLPR (dbg_midswitch, 1, ("Top of HEAVY switch\n\n"));



      /*-----------------------------------------------------------------*
         Disable interrupts so the interrupt handler doesn't get control
         and process (and reset) any status (FIFO available or switch complete)
         between the following tests and their respective sleeps.
       *-----------------------------------------------------------------*/

        our_intr_priority = pGD -> devHead.display ->
                                interrupt_data.intr.priority - 1 ;

        saved_intr_priority = i_disable (our_intr_priority) ;


      /********************************************************************
         PREVIOUS CONTEXT SWITCH NOT COMPLETE section

         This section handles the case where the previous context switch
         has not completed and the adapter is not yet ready for the next one.
       *******************************************************************/
        BUGLPR(dbg_midswitch, 2, ("seq_num   = %4X \n",seq_num ));
        BUGLPR(dbg_midswitch, 2, ("dom flags = %4X \n",ddf->dom_flags));

        if ( (seq_num & WAITING_FOR_PREVIOUS_COMPLETION) )
        {
            BUGLPR(dbg_midswitch, 1, ("WAITING FOR previous to comp \n"));

            if ( !( (ddf->dom_flags) & PREVIOUS_SWITCH_ALREADY_COMPLETED) )
            {
                ddf -> dom_flags |= SLEEPING_FOR_PREVIOUS_COMPLETION ;

                ddf -> dom_switch_sleep_word = EVENT_NULL ;
                ddf -> dom_switch_sleep_flag = NULL ;


                BUGLPR(dbg_midswitch, 2, ("dom flags = %4X \n",ddf->dom_flags));
                BUGLPR(dbg_midswitch, 2,
                        ("sleep word = 0x%8X \n", ddf->dom_switch_sleep_word ));

                MID_DD_EXIT_TRACE (midswitch, 2, END_SWITCH, ddf, pGD,
                        ddf -> dom_flags, &(ddf -> dom_fifo_sleep_word), 0xF8);

                if ( ddf->num_graphics_processes )
                /*---------------------------------------------------------*
                   For the graphics process, we'll put it to sleep.
                 *---------------------------------------------------------*/
                        e_sleep ( &(ddf -> dom_switch_sleep_word), 0) ;
                else
                {
                        i_enable (saved_intr_priority) ;

                        while ( !ddf -> dom_switch_sleep_flag )
                        {
                        /* Waiting for Context Switch Complete interrupt.*/
                        }
                        saved_intr_priority = i_disable (our_intr_priority) ;
                }

            }


            /*---------------------------------------------------------*
            Reset the domain flags
                "Waiting for previous" must be reset, the others are
                optional.
            *---------------------------------------------------------*/
            ddf -> dom_flags &= ~WAITING_FOR_PREVIOUS_COMPLETION ;
            ddf -> dom_flags &= ~SLEEPING_FOR_PREVIOUS_COMPLETION ;
            ddf -> dom_flags &= ~PREVIOUS_SWITCH_ALREADY_COMPLETED ;
            ddf -> dom_flags |= PREVIOUS_SWITCH_FINALLY_COMPLETED ;

            BUGLPR(dbg_midswitch, 2, ("dom flags = %4X \n",ddf->dom_flags));

            /*---------------------------------------------------------*
               The previous context switch has finally completed, now
                start the next one.
             *---------------------------------------------------------*/
            BUGLPR(dbg_midswitch, 2, ("calling start switch \n"));

            rc = mid_start_switch (pGD, pRCXold, pRCXnew, &switch_parm) ;
            if (rc == MID_LIGHT_SWITCH)
            {
                i_enable (saved_intr_priority) ;
                return (MID_RC_OK) ;
            }
        }






      /********************************************************************
         WAITING FOR FIFO AVAILABLE section

         This section handles the case where the current context switch
         has not completed.  Currently, these cases include:
           . waiting for FIFO available.

       *******************************************************************/
        BUGLPR(dbg_midswitch,2,("FIFO sect: dom flags= %4X\n",ddf->dom_flags));

        if ( ddf -> dom_flags & WAITING_FOR_FIFO_AVAILABLE )
        {
                BUGLPR(dbg_midswitch, 2, ("Waiting for fifo\n"));

            if ( !( (ddf -> dom_flags) & FIFO_ALREADY_AVAILABLE) )
            {
                BUGLPR(dbg_midswitch, 2, ("fifo NOT available\n"));
                ddf -> dom_flags |= SLEEPING_FOR_FIFO_AVAILABLE ;

                ddf -> dom_fifo_sleep_word = EVENT_NULL ;
                ddf -> dom_fifo_sleep_flag = NULL ;


                BUGLPR(dbg_midswitch, 2, ("dom flags = %4X \n",ddf->dom_flags));
                BUGLPR(dbg_midswitch, 2,
                        ("sleep word = 0x%8X \n", ddf->dom_fifo_sleep_word ));

                MID_DD_EXIT_TRACE (midswitch, 2, END_SWITCH, ddf, pGD,
                        ddf -> dom_flags, &(ddf -> dom_fifo_sleep_word), 0xF9);

                if ( ddf->num_graphics_processes )
                /*---------------------------------------------------------*
                   For the graphics process, we put it to sleep.
                 *---------------------------------------------------------*/
                        e_sleep ( &(ddf -> dom_fifo_sleep_word), 0) ;
                else
                {
                        i_enable (saved_intr_priority) ;

                        while ( !ddf -> dom_fifo_sleep_flag )
                        {
                        /* Waiting for fifo available.                  */
                        }
                        saved_intr_priority = i_disable (our_intr_priority) ;
                }
            }
                BUGLPR(dbg_midswitch, 2, ("fifo available\n"));
        }

        ddf -> dom_flags &= ~SLEEPING_FOR_FIFO_AVAILABLE ;
        ddf -> dom_flags &= ~FIFO_ALREADY_AVAILABLE ;

        BUGLPR(dbg_midswitch, 2, ("dom flags = %4X \n",ddf->dom_flags));

        /*******************************************************************
           Now re-enable interrupts and clean up I/O access.
        *******************************************************************/

        i_enable (saved_intr_priority) ;


        BUGLPR(dbg_midswitch, 2, ("midRCX flags = 0x%4X,  dom flags = 0x%4X \n",
                                        pmidRCXnew-> flags,  ddf-> dom_flags));
        BUGLPR(dbg_midswitch, 2,
                ("dom sleep word = 0x%8X \n", ddf-> dom_switch_sleep_word ));
        BUGLPR(dbg_midswitch, 3,
              ("dom delayed WIDs = 0x%8X \n",ddf->dom_delayed_WID_writes_top));


        MID_DD_EXIT_TRACE (midswitch, 2, END_SWITCH, ddf,
                pGD, pRCXnew, (ddf->current_context_midRCX)->pRcx, 0xF0) ;

        BUGLPR (dbg_midswitch, 1, ("EXIT 0: Bottom of HEAVY switch\n\n"));

        return 0;
}


long mid_do_switch(pGD, pRCXold, pRCXnew)
struct _gscDev *pGD;
rcxPtr  pRCXold;
rcxPtr  pRCXnew;
{
  int  seq;
  struct phys_displays *pd = pGD->devHead.display;
  struct midddf *ddf = (struct midddf *) pd->free_area;

  ulong       rc,our_intr_priority,saved_intr_priority;

  our_intr_priority = pGD -> devHead.display ->
                          interrupt_data.intr.priority - 1 ;

  saved_intr_priority = i_disable (our_intr_priority) ;

  rc = mid_start_switch(pGD, pRCXold,pRCXnew,&seq);

  if (rc == MID_HEAVY_SWITCH)
  {
      mid_end_switch (pGD,pRCXold,pRCXnew,seq) ;
  }

  i_enable (saved_intr_priority) ;

}
