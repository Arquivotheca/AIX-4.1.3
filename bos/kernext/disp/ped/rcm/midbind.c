static char sccsid[] = "@(#)76  1.8.2.1  src/bos/kernext/disp/ped/rcm/midbind.c, peddd, bos411, 9428A410j 6/10/94 18:42:07";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: mid_bind_window
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


/****************************************************************************** 
   Includes:  
 ******************************************************************************/

#include <sys/types.h>
#include <sys/syspest.h>
#include <sys/xmem.h>
#include <sys/rcm_win.h>
#include <sys/aixgsc.h>
#include <sys/rcm.h>
#include "mid.h"
#include "hw_macros.h"
#include "midhwa.h"
#include "mid_deferbuf.h"
#include "midddf.h"
#include "midrcx.h"
#include "midRC.h"
#include "rcm_mac.h"			/* for FIND_GP */
#include "mid_rcm_mac.h"
#include "midwidmac.h"
#include "mid_dd_trace.h"

MID_MODULE (midbind);


/*************************************************************************** 
   Externals defined herein: 
 ***************************************************************************/

extern long mid_bind_window (
                             gscDev *,
                             rcx    *,
	                     rcmWG  *,
	                     rcmWA  *  );


  

/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:    mid_bind_window
  
   Descriptive Name:  Mid-level Graphics Adapter, Device Dependent
                        Bind WIndow Routine 
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      
      Bind the three components:
        . the window attributes, 
        . the window geometry and   
        . the rendering context.  
  
      In this case, "bind" means "associate together".  This is accomplished
      by chaining (or linking) these three structures together. 

      In addition to binding these structures together, the bind_window
      function also makes the newly bound triad active on the adapter. 
      This could result in a change in the context, the pixel interpretation 
      or one or more window IDs. 

      To achieve good performance, the normal case must be that no context
      change occurs at bind time.  This is consistent with the X server
      implementation in which the server uses a single context and switches
      between windows via the bind. 
 
      More information is available on the bind strategy in the Ped RCM
      design document. 


 *--------------------------------------------------------------------------* 
  
    Restrictions:
                  none
  
    Dependencies:
        1.  The rendering context, window geometry structure and
            window attribute structure are all valid.
  
  
 *--------------------------------------------------------------------------* 
  
   Linkage:
           This function is an aixgsc call and, more or less, one of the
           GAI RMS functions as well.

           This function begins as a SetContext call to the RMS layer.
           Both bind_window and set_rcx can be invoked as a result of a
           SetContext depending on the particular transitions being made.
           Refer to the GAI RMS layer spec for more information on this.

           When the RMS layer translates the SetContext into an 
           aixgsc (BIND_WINDOW) call.  This invokes the device independent
           RCM who in turn gives control to the appropriate device dependent 
           bind_window routine.
  
 *--------------------------------------------------------------------------* 
  
    INPUT:
            . a pointer to the virtual terminal structure
            . a pointer to the context structure to be bound 
            . a pointer to the window geometry structure to be bound
            . a pointer to the window attribute structure to be bound
          
    OUTPUT:  none 
          
    RETURN CODES:  none        
          
          
          
                                 End of Prologue                                
 ******************************************************************************/


  
long mid_bind_window (
                             gscDev   *pGD,
                             rcx      *pRCX,
	                     rcmWG    *pWG,
	                     rcmWA    *pWA  ) 
{
	pid_t		pid ; 
	rcmProc		*pProc ; 
	rcx		*pXRCX ; 

	midddf_t  *ddf 	= (midddf_t *)(pGD -> devHead.display -> free_area) ; 

	midWG_t  *pmidWG ;                     /* pmidWG is ptr to DD WG */
	mid_rcx_t  *pmidRCX ;                     /* DD rcx structure ptr */
	mid_wid_t  new_WID ;                    
        mid_wid_status_t  *WID_entry ;          /* WID entry for WGs WID */

	int 	saved_ints ; 
	long 	rc ;  



    /*------------------------------------------------------------------*
       Trace the entry parms
     *------------------------------------------------------------------*/

    MID_DD_ENTRY_TRACE (midbind, 1, BIND, ddf, pGD, pWG, pRCX, pWA ) ;

    MID_DEBUG_WG (Bind, 1, pWG) ; 
    MID_DEBUG_REGION (Bind, 1, pWG->wg.pClip) ;



    /*------------------------------------------------------------------*
       echo the input.  
     *------------------------------------------------------------------*/
	 
    BUGLPR (dbg_midbind, BUGNFO, ("Top of mid_bind\n") ); 
    BUGLPR (dbg_midbind, BUGNFO, ("pGD = 0x%8X,  pRCX = 0x%8X\n",pGD,pRCX));
    BUGLPR (dbg_midbind, BUGNFO, ("pWG = 0x%8X,  pWA =  0x%8X\n\n",pWG,pWA)); 


    /*------------------------------------------------------------------*
       Validate the parameters.
     *------------------------------------------------------------------*/
	 
        if ( pGD == NULL || pRCX == NULL || pWG == NULL || pWA == NULL )
        { 
	        BUGLPR (dbg_midbind, 0, ("NULL parm passed.  EXIT 1 \n\n") ); 
	        return (MID_ERROR) ; 
        } 


    /************************************************************************
        Check for 2D context (and exit immediately)
     ************************************************************************/

	pmidRCX = (mid_rcx_t *)(pRCX->pData) ;
	if (pmidRCX->type == RCX_2D) 
	{ 
		BUGLPR (dbg_midbind, BUGNFO, ("EXIT 1:   2D context \n\n") ); 
		MID_DD_EXIT_TRACE (midbind, 1, BIND, ddf,pGD, pWG, pRCX, 0xF1);
		return (MID_RC_OK) ;
	} 



    /**************************************************************************
       The device independent RCM layer has already linked the window
       geometry and window attribute structures to the context structure.

       Set the flag indicating this window has been bound. 
     *************************************************************************/

	pmidWG = (midWG_t *) pWG -> pPriv ;    /* get mid WG ptr */
	pmidWG -> wgflags |= MID_WG_BOUND ;    /* set the "bound" flag */

	pmidWG -> pRegion = pWG -> wg.pClip ;    /* copy the geometry pointer */

	BUGLPR (dbg_midbind, BUGNFO+2, 
               ("midWG = 0x %X,  flags = 0x %X \n", pmidWG, pmidWG->wgflags) );

    /**************************************************************************
        All the I/O performed in this routine is done to the PCB.

        Instead of domain guarding, we simply disable interrupts
        to prevent context switches. 

     *************************************************************************/

#if 0
	saved_ints = MID_DD_I_DISABLE (ddf) ;



       /*----------------------------------------------------------------------

	    A bind to a new geometry qualifies as a "geometry change" for
	    the context.  Therefore, we will set the context's  
	    "geometry change" flag.  Currently, this is used for GL pattern 
	    regeneration.  We notify the adapter at Set Window Parameter 
	    (context switch) time, whether the geometry has changed.  
	    When the geometry does change, the adapter regenerates the GL 
	    pattern.  (It may do other things for other contexts.)
    
        *-------------------------------------------------------------------*/

	pmidRCX = (mid_rcx_t *)(pRCX->pData) ; 
	pmidRCX->flags.geometry_changed = 1 ;  

#endif


    	if (pRCX == (pGD-> domain[0].pCur))  /* is this the current context */ 
        {

    /**************************************************************************
        First guard the domain.

        The following function locks the domain.  This prevents any
        context switches or any other writes to the FIFO whilst we
        are changing the window parameters on the adapter.
        It is critical that the window parms are changed for the
        correct context.

        Note, we do not use the regular "guard domain" function, since
        it does not protect against a context switch TO ANY context owned
        by this (the current) graphics process.

     *************************************************************************/

       (*pGD->devHead.pCom->rcm_callback-> make_cur_and_guard_dom) (pRCX) ;

        	/*---------------------------------------------------------* 
        	   GET a WID for RENDERING
         	*---------------------------------------------------------*/
                rc = get_WID (MID_GET_WID_FOR_RENDER, pGD, pWG, pRCX) ;

        	new_WID = ((midWG_t *)(pWG -> pPriv)) -> wid ;
                BUGLPR (dbg_midbind, BUGNFO+1, ("WID  = %X\n", pmidWG->wid) );


#if 0
        	if ( rc != MID_OLD_WID )
        	{
#endif
        		/* Note: all I/O goes to the current process's FIFO */

        		mid_flush_WIDs_and_colormaps_update(pGD, ddf);
#if 0
        	}
#endif



                /*********************************************************
                   Notify the adapter of the new window.

                   This is done with the SetWindowParameters command.
                   This command specifies the new window's origin, size

                   and window ID as well as its shape.  The shape is
                   defined with a set of "visible rectangles".  Note that
                   the number of visible rectangles varies with the shape
                   of the window.

                   One more critical note:  We MAY NOT send this command to the
                   adapter until the context has been created on the adapter.
                *********************************************************/


                BUGLPR (dbg_midbind, BUGNFO+2, ("RCX flags  = %8X\n",
                                 ((mid_rcx_t *)(pRCX -> pData)) -> flags) );

                if ( ( ((mid_rcx_t *)(pRCX->pData)) ->
                                        flags.adapter_knows_context) == 1)

                {
                        BUGLPR (dbg_midbind,4, ("adapter knows ctx already\n"));

                        write_window_parms_FIFO (pGD,
                                                pmidWG -> wid,
                                                pWG -> wg.pClip,
                                                MID_LOWER_LEFT_ORIGIN,
                                                pWG -> wg.winOrg,
                                                pWG -> wg.width,
                                                pWG -> wg.height,
                                                pmidWG -> pi.pieces.color,
                                                pmidWG -> pi.pieces.flags
                                                ) ;

                }

	saved_ints = MID_DD_I_DISABLE (ddf) ;

	mid_set_client_clip (pGD, pRCX, pWA->wa.pRegion, pRCX->pWG, 0) ;

	i_enable (saved_ints) ;

        /*********************************************************************
            Finally, we must unlock (enguard) the domain.
        ********************************************************************/

        (*pGD->devHead.pCom->rcm_callback->unguard_domain) (pRCX -> pDomain) ;

       }
       else  /* the binding context is not currently active */
       {
#if 0
                pmidWG -> wgflags |= MID_WG_UPDATED ;
#endif
		pmidRCX->flags.geometry_changed = 1;
       }



#if 0
     	/********************************************************************* 
    	    Finally, we enable interrupts and exit
      	********************************************************************/ 

	i_enable (saved_ints) ;
#endif

	/*------------------------------------------------------------------*
          Exit trace 
	*------------------------------------------------------------------*/
	BUGLPR (dbg_midbind, BUGNFO, ("Bottom of mid_bind\n\n") ); 

	MID_DD_EXIT_TRACE (midbind, 2, BIND, ddf, pGD, pWG, pRCX, 0xF0 ) ;

	return (MID_RC_OK) ;
}

