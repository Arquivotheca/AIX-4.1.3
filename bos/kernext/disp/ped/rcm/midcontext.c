static char sccsid[] = "@(#)65  1.14.2.3  src/bos/kernext/disp/ped/rcm/midcontext.c, peddd, bos411, 9428A410j 3/21/94 13:36:43";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: deallocate_context
 *              mid_create_rcx
 *              mid_delete_rcx
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
#include <sys/errno.h>
#include <sys/dma.h>
#include <sys/xmem.h>
#include <sys/rcm_win.h>
#include <sys/syspest.h>
#include <sys/sleep.h>

#include <sys/aixgsc.h>
#include <sys/rcm.h>
#include <gai/gai.h>

#include "mid.h"
#include "midhwa.h"
#include "midddf.h"
#include "midrcx.h"             /* BUS access definitions in here       */
#include "midRC.h"
#include "mid_rcm_mac.h"

#include <sys/malloc.h>
#include <sys/pin.h>
#include "mid_dd_trace.h"

MID_MODULE (midcontext);

/*****************************************************************************
       Externals:
 *****************************************************************************/

extern long mid_create_rcx (
                            gscDev *,
                            rcx *,
                            create_rcx *,
                            mid_create_rcx_t * ) ;


extern long mid_delete_rcx (gscDev *,
                            rcx *   );

extern long deallocate_context (
                                gscDev          *pGD,
                                midddf_t        *ddf,
                                mid_rcx_t       *pmidRCX ) ;

#define PDEV_PRIORITY ( pGD->devHead.display->interrupt_data.intr.priority-1 )

/******************************************************************************

                             Start of Prologue

   Function Name:    mid_create_rcx

   Descriptive Name:  Create Rendering Context

 *--------------------------------------------------------------------------*

   Function:

     This routine creates and initializes any necessary internal device
     driver rendering context data structures required by the device driver
     for the mid-level graphics adapter (Ped and Lega).

     It also allocates pinned memory to be used later (by the heavy context
     switch code) to save a context from the adapter.

     The data structure is allocated from the pinned heap, because it must
     be referenced on the interrupt level.  This structure is then chained to
     the device independent rendering context data structure which has
     just been created by the device independent RCM code which invoked
     this routine.

 *--------------------------------------------------------------------------*

    Restrictions:
                  none

    Dependencies:
                  none


 *--------------------------------------------------------------------------*

    Linkage:
           This function is one of the GAI functions.  Hence it is available
           as an aixgsc call.

 *--------------------------------------------------------------------------*

    INPUT:  There are 3 input parameters to the general GAI create_rcx
             function:
              . pointer to the virtual terminal structure
              . pointer to the device independent rendering context structure
              . pointer to a parameter list.  This parameter list supplies
                 us with the context type (2D, 3DM1, 3DM2).


    OUTPUT:  none

    RETURN CODES:  none


                                 End of Prologue
 ******************************************************************************/



long mid_create_rcx(
                    gscDev   *pGD,
                    rcx      *pRCX,
                    create_rcx *arg,
                    mid_create_rcx_t  *pDD_parms )
{

  struct phys_displays *pd    = pGD->devHead.display;
  midddf_t             *ddf   = (midddf_t *) pd->free_area;

  mid_rcx_t            *pmidRCX ;
  mid_rcx_type_t        ctx_type ;



  int           malsize,
                size_with_header ;

  mid_ctx_save_chain_t  *save_area_header ;
  char                  *hw_save_area ;




    MID_DD_ENTRY_TRACE (midcontext, 2, MID_CREATE_RCX, ddf,
                                pGD, pRCX, pDD_parms -> rcx_type, 0) ;



    /*-------------------------------------------------------------
        First, a little error checking.
     *---------------------------------------------------------------*/




    BUGLPR(dbg_midcontext,BUGNFO,("Top of create context\n"));
    BUGLPR(dbg_midcontext,BUGNFO,
          ("pGD = 0x%8X, pRCX = 0x%8X, ddf = 0x%8X \n", pGD, pRCX, ddf));


  /*****************************************************************/
  /*****************************************************************/
  /*                                                               */
  /* First verify the context type and translate it into a memory  */
  /* for the subsequent allocation of the context save area.       */
  /*                                                               */
  /* There is a max size for each type of context so set a variable */
  /* for malloc to use as size based on ctx type                    */
  /*                                                               */
  /*****************************************************************/
  /*****************************************************************/

  ctx_type = pDD_parms -> rcx_type;

  switch ( ctx_type )
  {
     case RCX_2D:     malsize = MAX_CTX_LEN_2D ;
                      break;

     case RCX_3DM1:   malsize = MAX_CTX_LEN_3DM1;
                      break;

     case RCX_3DM2:   malsize = MAX_CTX_LEN_3DM2;
                      break;

     case RCX_PCB:    malsize = 0 ;
                      break;

     default:
          BUGLPR(dbg_midcontext, 0, ("INVALID context type passed in\n"));
          malsize = MID_MAX_CTX_SIZE ;
          break;
  }




  /*****************************************************************
     Now allocate space for the context data structure.
   *****************************************************************/

  pmidRCX = xmalloc (sizeof(struct mid_rcx), 4, pinned_heap) ;

  if ( pmidRCX == NULL )
  {
        BUGLPR(dbg_midcontext,0,("** ERROR ** PED DD RCX alloc failed\n\n"));
        BUGLPR(dbg_midcontext, BUGNFO, ("Leaving mid_create_rcx\n"));
        return (MID_ERROR) ;
  }



  /*****************************************************************
     Now initialize the data structure just allocated.

     First fill it with zeros, then link the DI and DD context
     structures and save the passed context type.
   *****************************************************************/

  bzero (pmidRCX, sizeof(struct mid_rcx)) ;

  pRCX -> pData = (genericPtr) pmidRCX ;

  /* set distinct timeslices for FIFO (domain 0) and PCB (domain 1) */
  if (pRCX -> domain == MID_PCB_DOMAIN)
    pRCX -> timeslice = PED_PCB_DEFAULT_TIMESLICE;
  else
    pRCX -> timeslice = PED_FIFO_DEFAULT_TIMESLICE;

  pmidRCX -> pRcx = pRCX ;
  pmidRCX -> type = ctx_type ;
  pmidRCX -> xs.aspace_id = XMEM_INVAL;

  BUGLPR(dbg_midcontext, BUGNFO, ("mid_rcx: 0x%8X\n",pmidRCX));





  /****************************************************************************

     ALLOCATION of hardware context SAVE AREA

     This section of code allocates pinned memory for context space save area.
     A leading test is made for a NULL context.  (No save area is allocated
     for a NULL CONTEXT, since it will never be put on the adapter.)
     No save area is allocated for a PCB CONTEXT either.  PCB contexts are
     used for synchronizing access to the PCB, but do not include any hardware
     state data.

   ****************************************************************************/

    if ( ( pRCX -> flags & RCX_NULL) == 0 && pRCX->domain == MID_FIFO_DOMAIN)
    {

        /*****************************************************************
           This next section of code allocates a hardware context save area.
           We have already converted the context type to a memory size above.
         *****************************************************************/

        BUGLPR(dbg_midcontext, BUGNFO+2, ("Creating HW context save area \n"));

        size_with_header = malsize + sizeof (mid_ctx_save_chain_t) ;
        BUGLPR(dbg_midcontext, BUGNFO,
                ("type = %d, size(w/hdr) = 0x%X, size (w/o hdr) = 0x%X\n",
                  ctx_type, size_with_header, malsize));

        hw_save_area = ((char *)
                        (xmalloc (size_with_header, 4, pinned_heap) ))  ;

        if( hw_save_area == NULL )
        {
            BUGLPR(dbg_midcontext,0, ("**ERROR**  save area alloc failed\n\n"));
            BUGLPR(dbg_midcontext, BUGNFO, ("Leaving mid_create_rcx\n"));
            /*----------------------------------------------------------------*
               The allocation of the save area for the hardware context failed.
               We will give back the area for the data structure and return.
             *----------------------------------------------------------------*/
            xmfree(pmidRCX, pinned_heap);

            return (MID_ERROR) ;
        }



        /*****************************************************************

           ALLOCATION of the context SAVE AREA was SUCCESSFUL

         *****************************************************************/

        pmidRCX -> flags.context_space_allocated = 1 ;

        BUGLPR(dbg_midcontext, BUGNFO+1, ("hw area = 0x%8X\n", hw_save_area));
        pmidRCX -> size = malsize;
        pmidRCX -> hw_rcx = hw_save_area ;

        pmidRCX-> context_sleep_event_word = EVENT_NULL ;
        pmidRCX-> pNext_sleep = NULL ;



        /*****************************************************************

           now do the cross memory attach to set up for DMA

         *****************************************************************/
        BUGLPR(dbg_midcontext, BUGNFO+2, ("cross memory attach \n") );


        if  ( xmattach ( hw_save_area,
                         malsize,
                         &pmidRCX->xs,
                         SYS_ADSPACE    ) != XMEM_SUCC )
        {
            /*----------------------------------------------------------------*
               Now the cross memory attach failed.  Give back both the HW
               context save area and the area for the data structure and return.
             *----------------------------------------------------------------*/
            xmfree(hw_save_area, pinned_heap);
            xmfree(pmidRCX, pinned_heap);
            BUGLPR(dbg_midcontext, 0,
            ("** ERROR ** xmattach: hw_rcx = 0x%4X, size =%8X, xmem =%8X\n\n",
                                    hw_save_area, malsize, &pmidRCX->xs));
            BUGLPR(dbg_midcontext, BUGNFO, ("Leaving mid_create_rcx\n"));
            return (MID_ERROR) ;
        }

        pmidRCX->xs.aspace_id = XMEM_GLOBAL;    /* indicates kernal memory */


    } /* end of allocation required */


  /*****************************************************************
     Now increment the DWA context count, if necessary.
   *****************************************************************/

  if (pmidRCX->type != RCX_2D )          ddf->num_DWA_contexts++ ;



    BUGLPR(dbg_midcontext, BUGNFO+1,
          ("hw_rcx 0x%8X size 0x%8X\n", pmidRCX->hw_rcx, pmidRCX->size));
    BUGLPR(dbg_midcontext, BUGNFO+1, ("rcx flags = %4X\n", pmidRCX->flags));


    /*********************************************************************
       Trace at the exit point
     *********************************************************************/
    BUGLPR(dbg_midcontext, BUGNFO, ("Leaving mid_create_rcx\n\n"));


    MID_DD_EXIT_TRACE (midcontext, 1, MID_CREATE_RCX, ddf,
                                pGD, pRCX, pDD_parms -> rcx_type, pmidRCX) ;

    return (MID_RC_OK) ;
}









/******************************************************************************

                             Start of Prologue

   Function Name:    mid_delete_context

   Descriptive Name:  Delete Context

 *--------------------------------------------------------------------------*

   Function:

     This routine deletes a context.
     This includes:
      . deleting the device dependent rcx data structure.
      . freeing any space being used to save the adapter's context
        data in system memory

 *--------------------------------------------------------------------------*

   pseudo code:

     If context is currently on the adapter,
        tell the adapter to trash (terminate) the context

     If the context is currently saved in space from the normal heap,
        free the memory

     If the context is currently saved in space from the pinned heap,
        detach the context save area
        free the memory


     free the device driver data structure
     return


 *--------------------------------------------------------------------------*

    Restrictions:
                  none

    Dependencies:
                  none


 *--------------------------------------------------------------------------*

    Linkage:
           This function is one of the GAI functions.  Hence it is available
           as an aixgsc call.

 *--------------------------------------------------------------------------*

    INPUT:  These are the input parameters to the delete_context
             function:
              . pointer to the virtual terminal structure
              . pointer to the device independent rendering context structure


    OUTPUT:  none

    RETURN CODES:  none


                                 End of Prologue
 ******************************************************************************/


long mid_delete_rcx(pGD, pRCX)

struct _gscDev *pGD;
struct _rcx *pRCX;

{
struct phys_displays *pd    = pGD->devHead.display;
midddf_t             *ddf   = (midddf_t *) pd->free_area;

mid_rcx_t       *pmidRCX ;              /* dev dep RCX pointer */
mid_rcx_t       *search_midRCX,
                *search_Prev ;
rcmWG           *pWG;

long             rc ;
int              old_int ;
ulong            switch_parm ;
int              pick_mode;




    MID_DD_ENTRY_TRACE (midcontext, 1, MID_DELETE_RCX, ddf, pGD,
                pRCX, (ddf -> current_context_midRCX) -> pRcx, pRCX->flags) ;


    BUGLPR(dbg_midcontext, 1, ("Top of delete context\n"));
    BUGLPR(dbg_midcontext, 2,
          ("pGD = 0x%8X, pRCX = 0x%8X, ddf = 0x%8X \n", pGD, pRCX, ddf));




    pmidRCX = ((mid_rcx_t *)(pRCX -> pData)) ;


    /*****************************************************************

      Special handling for NULL contexts and the PCB context:

       The adapter need not be notified of the context deletion for NULL
       context or the PCB context.  Therefore, we handle these contexts in
       this section, which simply cleans up and deallocates structures.

     *****************************************************************/

    if ( ((pRCX -> flags) & RCX_NULL) == RCX_NULL ||
           pRCX->domain == MID_PCB_DOMAIN)
    {

        BUGLPR(dbg_midcontext, 1, ("NULL context\n"));

        /*********************************************************************
          Now free the memory used to save the context.  This was done in
          deallocate_ctx_save_area - but all that has moved as so all that
          was left to do was o free memory - this cannot be done out of
          the interrupt environment.
        *********************************************************************/

        if (pmidRCX -> flags.context_space_allocated == 1)

                xmfree(pmidRCX -> hw_rcx, pinned_heap);

        deallocate_context (pGD, ddf, pmidRCX) ;

        BUGLPR(dbg_midcontext, 1, ("Leaving mid_delete_rcx\n\n"));

        MID_DD_EXIT_TRACE (midcontext, 1, MID_DELETE_RCX, ddf,
                        pGD, 0xF0C0FF1, pmidRCX -> type, pmidRCX) ;

        return (MID_RC_OK) ;
    }

    /*********************************************************************
        Remember the pick mode, so that we can wake up the serializer.
        But, don't wake it up till the end of this function.
    *********************************************************************/

    pick_mode = ddf->pickDataBlock.eventcmd & PICK_PENDING;




    /*********************************************************************

      FORCE the CONTEXT to be CURRENT (and GUARD THE DOMAIN)

          To actually terminate the context from the adapter, we must make
          it current, before we can tell the adapter to terminate it.

          The device independent layer will later force a context switch
          away from this context if it is current, so all we need do now
          is ensure that it is current!

    *********************************************************************/

    (*pGD -> devHead.pCom -> rcm_callback -> make_cur_and_guard_dom) (pRCX) ;



    /*-------------------------------------------------------------------------

        Now set the "terminate context" flag.  This will cause the following
        things to happen when this context is next switched out.
          . Notify the adapter to terminate the context
          . invoke the "dealloc_context" routine to free the memory

        NOTE AGAIN that these things happen at the next context switch which
                   is forced below.

        Also set the CONTEXT_DELETION flag.  This inhibits WID writes for the
        default context (during context deletion).

     *------------------------------------------------------------------------*/

    pmidRCX -> flags.terminate_context = 1 ;
    ddf->dom_flags |= MID_CONTEXT_DELETION_IN_PROCESS ;





    /*********************************************************************

      We must remove the context from any device dependent context lists
      it might be on.  An example of this is the swap buffer sleep list.

      Before any list changes, disable interrupts.

    *********************************************************************/

    old_int = i_disable (INTMAX) ;

    if (pmidRCX -> flags.waiting_for_WID == 1)
    {
        search_Prev = ddf->WID_sleep_top ;


        BUGLPR(dbg_midcontext, 1,
                ("On sleep list, top = 0x%8X\n", ddf-> WID_sleep_top )) ;

        /*-------------------------------------------------------------------
           If this context is on the top of the list remove just update the
           the anchor pointer.
         *-------------------------------------------------------------------*/
        if (search_Prev == pmidRCX)
        {
                BUGLPR(dbg_midcontext, 2, ("On top of list\n" )) ;

                ddf-> WID_sleep_top = pmidRCX-> pNext_sleep ;

                if (pmidRCX->pNext_sleep == NULL)
                {
                        ddf-> WID_sleep_bottom = NULL ;
                }
        }

        /*-------------------------------------------------------------------
           Else, not on top of the list, remove from middle.
             . Search for it, remembering its predecessor, then remove it.
         *-------------------------------------------------------------------*/
        else
        {
                BUGLPR(dbg_midcontext, 2, ("Looping through sleep list \n" )) ;

                for ( search_midRCX = search_Prev-> pNext_sleep ;
                      search_midRCX != NULL ;
                      search_midRCX = search_midRCX-> pNext_sleep )
                {
                        if (search_midRCX == pmidRCX)
                        {
                            BUGLPR(dbg_midcontext, 2,
                                ("Context FOUND on SWAP SLEEP LIST \n"));

                            search_Prev->pNext_sleep = pmidRCX->pNext_sleep;

                            if (pmidRCX->pNext_sleep == NULL)
                            {
                                ddf-> WID_sleep_bottom = search_Prev ;
                            }

                            break ;
                        }

                        BUGLPR(dbg_midcontext, 2,
                                ("prev ctx = 0x%8X, curr ctx = 0x%8X \n",
                                        search_Prev, search_midRCX ) ) ;
                        search_Prev = search_midRCX ;
                }

                MID_ASSERT ( (search_midRCX != NULL ), MID_DELETE_RCX,
                                        ddf, pRCX, pmidRCX, 0, 0) ;
        }


    }





    /*********************************************************************

      Now actually trigger the terminate, by forcing the context
      off the adapter.

    *********************************************************************/

    rc = mid_start_switch (pGD, pRCX, ddf->default_context_RCX, &switch_parm) ;

   /*-------------------------------------------------------------------
      Now re-enable interrupts.

      REMEMBER that we have forced the terminated context onto the adapter.
    *-------------------------------------------------------------------*/

    i_enable (old_int) ;


    if (rc == MID_HEAVY_SWITCH)
    {
        mid_end_switch (pGD, pRCX, ddf-> default_context_RCX, switch_parm) ;
    }


    ddf->dom_flags &= ~MID_CONTEXT_DELETION_IN_PROCESS ;


  /*****************************************************************
     Now decrement the DWA context count, if necessary.
   *****************************************************************/

  if (pmidRCX->type != RCX_2D )          ddf->num_DWA_contexts-- ;



    /*********************************************************************

      If the context is bound to a WG (screen area), it may have a WID.
      We will clean that WID up here.

    *********************************************************************/

        pWG = pmidRCX->pRcx->pWG ;
        if ( pWG ) mid_delete_WID (1, ddf, pWG) ;


    /*********************************************************************

      DEALLOCATE any ADAPTER SAVE AREA(s)

          This was done in deallocate_ctx_save_area - but all that has moved
          as so all that was left to do was a free memory - this cannot be
          done out of the interrupt environment.

    *********************************************************************/

        if (pmidRCX -> flags.context_space_allocated == 1)

                xmfree(pmidRCX -> hw_rcx, pinned_heap);


    /*********************************************************************

      DEALLOCATE the device dependent data area

    *********************************************************************/

    deallocate_context (pGD, ddf, pmidRCX) ;



    /*********************************************************************

      UNGUARD the domain

      Finally, we unguard the domain to allow somebody else to have a
      crack at the adapter.

    *********************************************************************/

    (*pGD -> devHead.pCom -> rcm_callback -> unguard_domain) (pRCX->pDomain) ;


    /*********************************************************************

        Wakeup the pick serializer, if this context was in pick mode.

    *********************************************************************/

    if (pick_mode)
    {
        /* Mask off interrupts  */
        old_int = i_disable( PDEV_PRIORITY );

        /*
            Note:  This cleanup is similar to the end-pick situation, except
            that no data will be returned, and the context has already been
            deleted to the microcode.

            if there is any process waiting to do pick event
                    wake it up and let it process
            else
                    unlock the lock
        */

        if (ddf->pending_pick_headPtr != NULL)  /* if process is waiting */
            unblock_beginPick_event(ddf);
        else /* no more processes waiting so unlock the lock */
            ddf->PickEventInProgress = 0;

        /* Turn interrupts back on  */
        i_enable( old_int );
    }


    BUGLPR(dbg_midcontext, 1, ("Leaving mid_delete_rcx\n\n"));

    MID_DD_EXIT_TRACE (midcontext, 1, MID_DELETE_RCX, ddf,
                                pGD, pRCX, pmidRCX -> type, pmidRCX) ;

    return (MID_RC_OK) ;
}










/******************************************************************************

                             Start of Prologue

   Function Name:    deallocate_context

   Descriptive Name:  Deallocate the device dependent Context Structure

 *--------------------------------------------------------------------------*

   Function:

     This routine handles the deallocation of the device dependent context
     structure.


 *--------------------------------------------------------------------------*

   pseudo code:




 *--------------------------------------------------------------------------*

    Restrictions:
                  none

    Dependencies:
                  none


 *--------------------------------------------------------------------------*

    Linkage:
           This routine is called from the switch routine when we are
           switching away from a context that has been deleted (terminated).

 *--------------------------------------------------------------------------*

    INPUT:  These are the input parameters to the delete_context
             function:
              . pointer to the virtual terminal structure
              . pointer to the device dependent device driver structure
              . pointer to the device dependent rendering context structure


    OUTPUT:  none

    RETURN CODES:  none


                                 End of Prologue
 ******************************************************************************/


long deallocate_context (
                        gscDev          *pGD,
                        midddf_t        *ddf,
                        mid_rcx_t       *pmidRCX )
{



    MID_DD_ENTRY_TRACE (midcontext, 1, DEALLOCATE_CONTEXT, ddf, pGD,
                        pmidRCX, pmidRCX->type, *(ulong *)(&(pmidRCX->flags)) );


    BUGLPR (dbg_midcontext, 1, ("Deallocate context 0x%8X\n", pmidRCX->pRcx));
    BUGLPR (dbg_midcontext, 2,
           ("pGD = 0x%8X, midRCX = 0x%8X, ddf = 0x%8X \n", pGD, pmidRCX, ddf));
    BUGLPR (dbg_midcontext, 2, ("RCX type = %X,  midRCX flags = 0x%X \n",
                        pmidRCX->type, *((ulong *)(&(pmidRCX -> flags))) ));


    pmidRCX -> pRcx -> pData = 0x69 ;
    pmidRCX -> pRcx = 0x69 ;



        /*-------------------------------------------------------------------*
            Deallocate deallocate any existing clip list,
         *-------------------------------------------------------------------*/
#define RCX_COMPLEX_CLIP                (&(pmidRCX->client_clip.pClipComplex))

        if (gREGION_BOXPTR (RCX_COMPLEX_CLIP))
        {
                xmfree ( (gREGION_BOXPTR (RCX_COMPLEX_CLIP)), pinned_heap) ;
        }


    xmfree( pmidRCX, pinned_heap);




    BUGLPR(dbg_midcontext, BUGNFO, ("Leaving deallocate_context \n\n"));

    MID_DD_EXIT_TRACE (midcontext, 2, DEALLOCATE_CONTEXT, ddf, pGD,
        ddf, (ddf->current_context_midRCX)->pRcx, ddf->current_context_midRCX);

    return (MID_RC_OK) ;
}


