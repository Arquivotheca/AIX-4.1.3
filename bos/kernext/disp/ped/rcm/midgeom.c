static char sccsid[] = "@(#)68  1.11.3.5  src/bos/kernext/disp/ped/rcm/midgeom.c, peddd, bos411, 9428A410j 6/22/94 10:26:50";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: mid_create_win_geom
 *		mid_delete_win_geom
 *		mid_flush_WIDs_and_colormaps_update
 *		mid_update_win_geom
 *		write_WID_planes_fifo
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
#include <sys/adspace.h>
#include <sys/syspest.h>
#include <sys/malloc.h>
#include <sys/xmem.h>
#include <sys/rcm_win.h>
#include <sys/aixgsc.h>
#include <sys/rcm.h>
#include <gai/gai.h>
#include <rcm_mac.h>

#include "mid.h"
#include "midhwa.h"
#include "mid_deferbuf.h"
#include "midddf.h"
#include "midrcx.h"
#include "midwidmac.h"
#include "midRC.h"
#include "mid_rcm_mac.h"
#include "mid_dd_trace.h"

#include "hw_FIFkern.h"
#include <sys/sleep.h>

#define PDEV_PRIORITY (pGD->devHead.display->interrupt_data.intr.priority-1)

/***************************************************************************** 
       Externals: 
 *****************************************************************************/

extern long mid_create_win_geom (
                                  gscDev   *,
                                  rcmWG    *,
	                          create_win_geom *) ; 

extern long mid_update_win_geom (
                                  gscDev   *,
                                  rcx      *,
                      	          rcmWG    *,
                      	          int,   
                      	          rcmWG    * ) ;

extern long mid_delete_win_geom (
                                  gscDev   *,
                                  rcmWG    * ) ;

MID_MODULE (midgeom) ;

#define dbg_middd   dbg_midgeom



/****************************************************************************** 
                              
                             Start of Prologue  
  
   Function Name:    mid_create_win_geom  
  
   Descriptive Name:  Create Window Geometry Structure 
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      
  
     This routine creates and initializes any necessary internal device 
     driver data structures required by the device driver for the 
     mid-level graphics adapte (Ped and Lega).
  
     A small data structure is allocated from the normal (non-pinned)  
     memory heap.  Then it is chained to the device independent window     
     geometry structure which has just been created by the device independent
     RCM code which invoked this routine.
  
 *--------------------------------------------------------------------------* 
  
    Restrictions:
                  none
  
    Dependencies:
                  none
  
  
 *--------------------------------------------------------------------------* 
  
    Linkage:
           This function is one of the GAI functions.  Hence it is available
           as an aixgsc call.  

           The function is initiated by the window manager when a new        
           window is created.  The window manager performs a CreateWinGeom   
           which is handled by the device indenpendent RCM portion of the
           RMS layer.  It is the device indenpendent RCM code that actually
           invokes this routine. 
  
 *--------------------------------------------------------------------------* 
  
    INPUT:  There are 3 input parameters to the general GAI create_win_geom
             function:
              . pointer to the virtual terminal structure
              . pointer to the device independent window geometry structure
              . pointer to a parameter list
          
             Only the DI window geometry pointer is used.
          
    OUTPUT:  none 
          
    RETURN CODES:  none         
          
          
                                 End of Prologue                                
 ******************************************************************************/


long mid_create_win_geom (
                           gscDev           *pGD,
                           rcmWG            *pWG,
                           create_win_geom  *arg ) 

{
 midWG_t    *pmidWG ;
 midddf_t	*ddf ;            /*  ddf address */
 
		int  rc;


    ddf =  (midddf_t *) (pGD -> devHead.display -> free_area) ; 

    MID_DD_ENTRY_TRACE (midgeom, 2, CREATE_WG, ddf, pGD, pWG, 0, pGD) ; 


    /*-------------------------------------------------------------
        First, a little error checking.
     *---------------------------------------------------------------*/



        BUGLPR(dbg_midgeom, BUGNFO+1, ("Top of mid_create_win_geom:"
                                     "pGD = 0x%x, WG = 0x%x \n", pGD, pWG) );

        /*-------------------------------------------------------------------* 
            First validate the parameters.
        *--------------------------------------------------------------------*/
	if (pGD == NULL || pWG == NULL)
	{
		BUGLPR(dbg_midgeom, 0, ("pGD or pWG was NULL !!\n\n") );
		return (MID_PARM_ERROR) ;
	}






        /*********************************************************************
            First allocate the device dependent window geometry structure.
            A 16 byte boundary is used to force a nice alignment should   
            the WG be displayed on a debugging tool (kdbx, for example).  
            Then check to ensure that the memory was actually allocated.      
        **********************************************************************/
	pmidWG = (midWG_t *) xmalloc (sizeof(midWG_t), 4, pinned_heap) ;

	if (pmidWG == NULL) 
	{
		BUGLPR(dbg_midgeom, 0, 
		        ("Memory not available in kernel heap!\n\n") );
                return (MID_NO_MEMORY) ;
	}
	 
        /*-------------------------------------------------------------------* 
            Now chain the DD win geom structure to the DI win geom structure 
             and vice versa.
            Also initialize the window ID to the null WID, 
            and the PI to front buffer and color palette 0.
        *--------------------------------------------------------------------*/
	pWG -> pPriv = (genericPtr) pmidWG ;

	pmidWG -> pWG = pWG ;
	pmidWG -> wid = MID_WID_NULL ;
	pmidWG -> wgflags = 0 ;
	pmidWG -> pi.PI   = 0 ;

	/*------------------------------------------------------------------*
	    Set up for potential getImage X extension which requires us to
	    write back to user space the number of the screen buffer being
	    displayed for this geometry.

	    The user space buffer flag must initialized now, from data held
	    within the driver.

	    We memorize the complete double word of segment register value
	    and effect address of the user space flag because it makes it easy
	    to synchronize their updates.

	    It is also true that the RCM memorizes the value of the pointer
	    into user space on any win geom create/update.  But, we also
	    memorize it here because we want to have both values directly
	    available for unitary update.

	    The srval/eaddr pair may be used after geometry creation.

	    We don't synchronize with interrupt code for geometry creation
	    because there should be no swaps going on for this window yet.
	    Therefore, interrupt disabling is not required.
	 *------------------------------------------------------------------*/


	RCM_VALIDATE_CDB_ADDR (ddf->pdev, pWG->wg.pCurrDispBuff, 
				pmidWG->CDBaddr, rc) ;

	if (rc == -1)			/* if invalid ... */
	{
		xmfree (pmidWG, pinned_heap);
		pWG->pPriv = NULL;		/* sanitation */

		return (EINVAL);
	}
	else if (rc > 0)		/* if valid ... */
	{
		gscCurrDispBuff_t  newgscCurrDispBuff;

		/*  figure out what user space setting should be */
		newgscCurrDispBuff.buffer =
		     (pmidWG->pi.pieces.flags & MID_USING_BACK_BUFFER) ?
				gsc_DispBuff_B  :
				gsc_DispBuff_A	;

		/*   Update our record of user area */
		pmidWG->CurrDispBuff.buffer = newgscCurrDispBuff.buffer;

		RCM_UPDATE_CURR_DISP_BUFF (ddf->pdev, pmidWG->CDBaddr, 
					&newgscCurrDispBuff.buffer, rc) ;
		if (rc != XMEM_SUCC)
		{
		    xmfree (pmidWG, pinned_heap);
		    pWG->pPriv = NULL;		/* sanitation */

		    return (EINVAL);
		}
	}
	/* else, no buffer */

        BUGLPR(dbg_midgeom, BUGNFO+2, ("New WG:   pWG = 0x%x\n", pmidWG-> pWG));
        BUGLPR(dbg_midgeom, BUGNFO+2, ("          wid =    %d\n", pmidWG->wid));
        BUGLPR(dbg_midgeom, BUGNFO, ("Bottom of create_win_geom: WG = 0x%x,"
                                    " midWG = 0x%x\n\n", pWG, pmidWG ) );

        MID_DD_EXIT_TRACE (midgeom, 1, CREATE_WG, ddf, pGD, pWG, pmidWG, pGD) ; 

	return  0;
}



/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:    mid_update_win_geom  
  
   Descriptive Name:  Update Window Geometry Structure 
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      
  
     The tasks handled by this routine fall into two basic categories:  
       . updating the necessary internal software structures and
       . updating the adapter hardware to ensure the visuals are correct. 

     Since this is the "update window geometry" routine, the data structures
     to update are the device dependent window geometry structures.      

     Then we ensure that the visuals are correct.  The cases handled by this
     routine have been greatly simplified.  All @d window management is
     handled by X.  Therefore, this routine only handles 3D windows. 
     (One caveat:  X does create, update and bind one window.  This is
     done to allow DMA to operate properly.)
     
 *--------------------------------------------------------------------------* 
  
  
    Restrictions:
                   none
  
    Dependencies:
                   none
  
 *--------------------------------------------------------------------------* 
  

    Linkage:
           This function is one of GAI functions.  Hence it is available
           as an aixgsc call.  

           In addition, the "update window geometry" function is part of
           the RMS software layer.  When a graphic process needs to update
           a window geometry, it invokes UpdateWinGeom.  UpdateWInGeom forms 
           the interface to this routine.  UpdateWinGeom is also the routine
           responsible for updating the device independent window geometry  
           data structure.  Note that this is done AFTER calling this routine
           so we may have the benefit of looking at the old window geometry. 
           
  
 *--------------------------------------------------------------------------* 
  
    INPUT:  There are 5 input parameters to the general GAI update_win_geom
             function:
              . pointer to the virtual terminal structure
              . pointer to the current rendering context for this window.
                         NOTE THAT THIS POINTER IS NOT VALID IS ALL CASES.
                         EG, when the geometry is changed affecting a 
                         non-active window.
              . pointer to a DI window geometry structure containing the       
                         changes to the window geometry structure.
                         Note that this is a "normal" window geometry structure
                         however, only the changed fields are valid. 
              . a changemask -- This is a set of flags indicating which fields
                         in the window geometry structure are to be changed.
              . pointer to the DI window geometry structure (before any
                         updates have been applied).

    OUTPUT:  none 
          
    RETURN CODES:  none         
          
          
                                 End of Prologue                                
 ******************************************************************************/


long mid_update_win_geom (
                           gscDev   *pGD,
                           rcx      *pRCX, 
                           rcmWG    *pWGchanges,
                           int       changemask,
                           rcmWG    *pWG)
  
{
/***************************************************************************** 
   Local variables: 
 *****************************************************************************/
    	midddf_t  *ddf ;            /*  ddf address */

	pid_t	 pid ;
	rcmProc	*pProc ;

	rcx	*pXRCX ;
    	mid_rcx_t  *pmidRCX ;	/*  DD RCX address */
    	midWG_t   *pmidWG ;

    	mid_wid_t new_WID ;        		/* new WID from get WID */
    	gRegion  *exposure_list ;  /* Window area that has changed */
                               /* Note that this function is not yet */
                               /*  implemented through the RMS layer.*/
    	int  rc1 ;                  /* temp holding spot for return code */
	int  rc = MID_RC_OK;

    ulong     *new_color_handle ;
    ushort    new_color ;
    int       saved_int ;
    int       color_changed ;
#             define  _NO  0  
#             define  _YES 1   



/***************************************************************************** 
 ***************************************************************************** 

   Start of code

   First set up our address to the ddf. 

 ***************************************************************************** 
 *****************************************************************************/

    ddf =  (midddf_t *) (pGD -> devHead.display -> free_area) ; 

    MID_DD_ENTRY_TRACE (midgeom, 1, UPDATE_WG, ddf,
			 pRCX, pWG, pWGchanges, changemask) ; 


    /********************************************************************* 
           Echo the input parms. 
     *********************************************************************/

    BUGLPR(dbg_midgeom, BUGNFO, ("Top of mid_update_WG: " 
                        "WG = 0x%8X , mask = 0x%4X \n", pWG, changemask) );
    BUGLPR(dbg_midgeom, BUGNFO+1,
     ("pGD = 0x%8X, pRCX = 0x%8X, changes = 0x%8X\n", pGD, pRCX, pWGchanges) );


    /********************************************************************* 
      Perform error checking on the passed parameters: 
     *********************************************************************/

    if ( (pGD == NULL) || (pWGchanges == NULL) || (pWG == NULL) ) 
    { 
        BUGLPR(dbg_midgeom, 0, ("NULL parameter found\n") );
        BUGLPR(dbg_midgeom, 0, ("****** UPDATE WG NOT PERFORMED *****\n\n\n"));
        return (MID_ERROR) ; 
    } 


    /********************************************************************* 
      For DEBUG purposes, we can print the old and new WG regions here.
     *********************************************************************/

    MID_DEBUG_WG (Update_WG, 3, pWG) ; 
    MID_DEBUG_REGION (Update_WG, 3, pWG->wg.pClip) ;

    MID_DEBUG_WG (Update_WG, 1, pWGchanges) ; 
    MID_DEBUG_REGION (Update_WG, 1, pWGchanges->wg.pClip) ;




   /************************************************************************** 
        Check the context type -- exit immediately for a 2D context.
    **************************************************************************/

	if (pRCX != NULL) 
	{
	    pmidRCX = (mid_rcx_t *)(pRCX->pData) ; 
	    if (pmidRCX ->type == RCX_2D) 
	    {
                    BUGLPR(dbg_midgeom,BUGNFO,("EXIT 3:  update_win_geom\n\n"));
        	    MID_DD_EXIT_TRACE (midgeom, 1, UPDATE_WG, ddf, 
					pGD, pWG, pmidWG, 0xF3) ; 
                    return (MID_RC_OK) ;
	    }
	}



/***************************************************************************** 
 ***************************************************************************** 

   Start of real functional code

 ***************************************************************************** 
 *****************************************************************************/


   /************************************************************************** 
        Get the mid WG address
    **************************************************************************/

    pmidWG = ((midWG_t *)(pWG->pPriv)) ; 




/***************************************************************************** 

    CHECK FOR COLOR MAP CHANGE 

   Determine whether a color mapping change is part of the win geom update. 
   Set an internal flag accordingly, and record new color in the device  
   dependent window geometry. 

 *****************************************************************************/

    color_changed = _NO ;                 /* init color change flag off */
    if ( (gCWcolormap & changemask) != 0) 
    {
        color_changed = _YES ;            /* set color change flag on */
        new_color_handle = pWGchanges -> wg.cm_handle ;
        new_color = (ushort) (*new_color_handle);

        pmidWG-> pi.pieces.color = new_color ; 
        pmidWG-> wgflags |= MID_CM_UPDATED ;

        BUGLPR (dbg_midgeom, BUGNFO+2, ("color has changed \n") );      
        BUGLPR (dbg_midgeom, BUGNFO+2, ("handle = 0x%8X\n",new_color_handle));
        BUGLPR (dbg_midgeom, BUGNFO+2, ("color = %d \n", new_color) );      
    }

    BUGLPR(dbg_midgeom, BUGNFO+2, ("color flag = 0x%x \n", color_changed ) );




   /************************************************************************** 

    UPDATE THE CLIPPING REGION 

	The clipping region (aka visible region) of the window is kept in a
	structure hanging off (pointed to by) the DI WG.  This region is
	updated via the function call we are now handling.  However, this 
	pointer is not updated to the new area until after we return to the  
	DI layer.  Since we will need this information later, (when we write 
	the WID planes for instance) we will save it now.  And, in fact,
	what we are saving is the pointer to the new visible region!
	
    **************************************************************************/

    pmidWG->pRegion = pWGchanges->wg.pClip ; 
    exposure_list =  pmidWG-> pRegion ; 


    if (pRCX != NULL)  /* ensure ctx ptr is valid */ 
    {  
       	pid = getpid() ; 
       	FIND_GP (pGD, pProc) ;
    	MID_ASSERT ( (pProc != NULL), UPDATE_WG, pGD, ddf, pProc, pRCX, pWG) ;

       	pXRCX = pProc->pDomainCur[0] ; 

	/* 
           make sure we write to X's FIFO 
        */

       	MID_GUARD_AND_MAKE_CUR (pGD, pXRCX) ; 

#if 0
        /*
           clip the new wg to screen dimensions
        */
        {
                int x1, y1, h1, w1;

                x1 = pWGchanges->wg.winOrg.x;
                y1 = pWGchanges->wg.winOrg.y;
                h1 = pWGchanges->wg.height;
                w1 = pWGchanges->wg.width;

                if(x1 < 0
                || x1 > BUFXMAXP+1
                || y1 < 0
                || y1 > BUFYMAXP+1
                || (h1 + y1) > BUFYMAXP+1
                || (h1 + y1) < 0
                || (w1 + x1) > BUFXMAXP+1
                || (w1 + x1) < 0) {
                        if(x1 < 0) {
                                w1 -= -x1;
                                x1 = 0;
                        }
                        if(y1 < 0) {
                                h1 -= -y1;
                                y1 = 0;
                        }
                        if(x1 > BUFXMAXP+1) {
                                w1 -= x1 - (BUFXMAXP+1);
                                x1 = BUFXMAXP+1;
                        }
                        if(y1 > BUFYMAXP+1) {
                                h1 -= y1 - (BUFYMAXP+1);
                                y1 = BUFYMAXP+1;
                        }
                        if(h1 < 0)
                                h1 = 0;
                        if(w1 < 0)
                                w1 = 0;
                        if((h1 + y1) > BUFYMAXP+1)
                                h1 = BUFYMAXP+1 - y1;
                        if((h1 + y1) < 0)
                                h1 = 0;
                        if((w1 + x1) > BUFXMAXP+1)
                                w1 = BUFXMAXP+1 - x1;
                        if((w1 + x1) < 0)
                                w1 = 0;

                        pWGchanges->wg.winOrg.x = x1;
                        pWGchanges->wg.winOrg.y = y1;
                        pWGchanges->wg.height = h1;
                        pWGchanges->wg.width = w1;
                }
        }
#endif

    	/********************************************************************* 
     		To ensure list integrity we disable interrupts here
    	*********************************************************************/

    	saved_int = MID_DD_I_DISABLE (ddf) ;

   	/******************************************************************* 

    	If this is a geometry change, then set the context's "geometry changed"
    	flag.  Currently, this is used for GL pattern regeneration.  We notify
    	the adapter at Set Window Parameter (context switch) time, whether the
    	geometry has changed.  When the geometry does change, the adapter
    	regenerates the GL pattern.  (It may do other things for other contexts.
    
  	  Here we check if the geomtry actually did change and the context is 
    	currently bound to this geometry. If so, we mark the context as 
    	"geometry changed". 
    
    	********************************************************************/

#define  ANY_GEOMETRY_CHANGE   gCWwinOrg+gCWwidth+gCWheight+gCWclip

    	if ( (changemask & ANY_GEOMETRY_CHANGE) != 0 )	/* geom changed */
    	{  
	    pmidRCX = (mid_rcx_t *)(pRCX->pData) ; 
            pmidRCX->flags.geometry_changed = 1 ;  

            	/* brkpoint (ddf, pRCX, pWG, changemask, 0xCCCCCCCC) ; */
            	BUGLPR(dbg_midgeom, 2, 
			("geometry changed, flags = %X \n", pmidRCX->flags) );

    	}  

        /*---------------------------------------------------------* 
           GET A WID for RENDERING
         *---------------------------------------------------------*/

        rc1 = get_WID (MID_GET_WID_FOR_RENDER, pGD, pWG, pRCX) ;
        new_WID = ((midWG_t *)(pWG -> pPriv)) -> wid ;

        if ( rc1 == MID_OLD_WID ) 
        {  
            /*---------------------------------------------------------* 
               WRITE WINDOW ID PLANES 

                The region written need only be the newly exposed areas
                of the window in the case. 
                Note, however, that our window manager does not yet support 
		the exposure list interface, so we must write the entire 
		visible region.   
             *---------------------------------------------------------*/

            exposure_list =  pmidWG -> pRegion ; 

            WRITE_WID_PLANES ( pGD,
                               pWG, 
                               new_WID, 
                               exposure_list,     
                               MID_UPPER_LEFT_ORIGIN ) ;
        }  

    	i_enable (saved_int) ;

	/* 
           Note: all I/O goes to X's FIFO  
        */
	mid_flush_WIDs_and_colormaps_update(pGD, ddf);

       	MID_UNGUARD_DOMAIN (pGD, pXRCX) ; 

    }  

    /*-----------------------------------------------------------------*
	   UPDATE CurrDispBuff POINTER
     *-----------------------------------------------------------------*/
    if ( (gCWcurrDispBuff & changemask) != 0) 
    {
	int  old_interrupt_priority;

	/*
	 *  Update the srval/eaddr info in unitary manner, since swap
	 *  interrupts may be possible.
	 *
	 *  Validate the new address.
	 */
	old_interrupt_priority = i_disable (PDEV_PRIORITY);

	RCM_VALIDATE_CDB_ADDR (ddf->pdev, 
				pWGchanges->wg.pCurrDispBuff, 
				pmidWG->CDBaddr, rc1 ) ;

	/* if validate failed, then pmidWG->CDBaddr will have been cleared */
	if (rc1 > 0)
        {
                gscCurrDispBuff_t  newgscCurrDispBuff;

                /*  figure out what user space setting should be */
                newgscCurrDispBuff.buffer =
                     (pmidWG->pi.pieces.flags & MID_USING_BACK_BUFFER) ?
                                gsc_DispBuff_B  :
                                gsc_DispBuff_A  ;

                /*   Update our record of user area */
                pmidWG->CurrDispBuff.buffer = newgscCurrDispBuff.buffer;

                RCM_UPDATE_CURR_DISP_BUFF (ddf->pdev, pmidWG->CDBaddr,
                                        &newgscCurrDispBuff.buffer, rc1 ) ;

                if (rc1 != XMEM_SUCC)
                {		    	/* remove user area from use */
		    /* make sure pmidWG->CDBaddr is marked 'unused' */

		    pmidWG->CDBaddr.eaddr = pmidWG->CDBaddr.srval = 0; 

		    rc = EINVAL;
                }
	}
	else if (rc < 0)		/* invalid buffer ... */
        {
		/* pmidWG->CDBaddr has been marked 'unused' */
		rc = EINVAL;
	}
	/* else, not used */

	i_enable (old_interrupt_priority);
    }

    /*-----------------------------------------------------------------* 
           COMMON EXIT 
     *-----------------------------------------------------------------*/

    MID_DEBUG_REGION (Update_WG, 10, pmidWG -> pRegion) ; 
 
    BUGLPR(dbg_midgeom, BUGNFO, ("Bottom of mid_update_win_geom \n\n") );      
    MID_DD_EXIT_TRACE (midgeom, 2, UPDATE_WG, ddf, pGD, pWG, pmidWG, 0xF0) ; 
    return (rc) ;
}



/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:    mid_delete_win_geom  
  
   Descriptive Name:  Delete Window Geometry Structure 
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      
  
     This routine performs all processing necessary to delete the window
     geometry with respect to the mid-level graphics device driver. 
  
     This consists of
       . freeing the memory allocated for the DD window geometry structure,   
       . freeing a window ID that may be in use by this window.  (This step
           is handled by another routine which we call.) 
  

 *--------------------------------------------------------------------------* 
  
    Restrictions:
                  none
  
    Dependencies:
                  none
  
  
 *--------------------------------------------------------------------------* 
  
    Linkage:
           This function is one of GAI functions.  Hence it is available
           as an aixgsc call.  

           It is called by the window manager when a window geometry is  
           deleted. 
           
  
 *--------------------------------------------------------------------------* 
  
    INPUT:  There are 2 input parameters to the general GAI update_win_geom
             function:
              . pointer to the virtual terminal structure
              . pointer to the window geometry structure
          
    OUTPUT:  none 
          
    RETURN CODES:  none         
          
          
                                 End of Prologue                                
 ******************************************************************************/



long mid_delete_win_geom (
                           gscDev   *pGD,
                           rcmWG    *pWG )

{
/***************************************************************************** 
   Local variables: 
 *****************************************************************************/
midddf_t     *ddf ;                 /* ddf pointer */



    /************************************************************************* 
       First set up our address to the ddf. 
     *************************************************************************/

    ddf =  (midddf_t *) (pGD -> devHead.display -> free_area) ; 
    MID_DD_ENTRY_TRACE (midgeom, 1, DELETE_WG, ddf, pGD, pWG, 0, 0) ; 



/*---------------------------------------------------------------------------* 
   Echo the input parms. 
 *---------------------------------------------------------------------------*/
    BUGLPR(dbg_midgeom, BUGNFO, ("Top of mid_delete_win_geom \n") ) ;   
    BUGLPR(dbg_midgeom, BUGNFO, ("pGD = 0x%x,  pWG = 0x%x \n", pGD, pWG ) );



    /************************************************************************* 
       Now delete the window's ID from whatever list is may be on.  
       If this frees up a window ID, it should be moved to the UNUSED list.
       All this is done in the following call. 
     *************************************************************************/

    mid_delete_WID (MID_NULL_REQUEST, ddf, pWG) ;    



    /************************************************************************* 
      Now finally free the memory for this structure 
     *************************************************************************/

    xmfree ( ((midWG_t *)(pWG -> pPriv)), pinned_heap) ;



    BUGLPR(dbg_midgeom, BUGNFO, ("EXIT mid_delete_win_geom \n\n") );      
    MID_DD_EXIT_TRACE (midgeom, 2, DELETE_WG, ddf, pGD, pWG, 0, 0xF0) ; 
    return (MID_RC_OK) ;
}

/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:   mid_flush_WIDs_and_colormap_update
  
   Descriptive Name: 
                    
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      
      This routine writes any delayed window geometry changes to  
      the adapter through X's FIFO.  This includes the following tasks: 
        . writing the WID planes (as required) 
	. associating color maps to WIDs (as required)


 *--------------------------------------------------------------------------* 
  
    Restrictions:
                  none
  
    Dependencies:
       1. This routine performs I/O.  
  
  
 *--------------------------------------------------------------------------* 
  
   Linkage:
           This routine is called from mid_update_win_geom

  
 *--------------------------------------------------------------------------* 
  
    INPUT:
            . a pointer to the device dependent structure 
          
    OUTPUT:  none 
          
    RETURN CODES:  none        
          
          
                                 End of Prologue                                
 ******************************************************************************/


mid_flush_WIDs_and_colormaps_update ( gscDev * pGD, midddf_t *ddf  )
{
	int i;
	midWG_t    *pDelayed ;
	midWG_t    *temp ;

	mid_wid_t       WID_id ; 
	mid_wid_status_t       *WID_entry ; 

	ushort 	WID_color, WID_fb, fb_data;

        HWPDDFSetup;   


        BUGLPR(dbg_midgeom, 1, ("Entering mid_flush_WIDs_and_colormap_update\n"));

        MID_DD_ENTRY_TRACE (midgeom, 1, FLUSH_WIDS_COLORMP_UPDATE, ddf,
        			ddf, ddf->dom_delayed_WID_writes_top, 
        			     ddf->dom_delayed_WID_writes_bot, 
                        	     ddf->dom_delayed_Assoc_updates_count ) ; 

  	PIO_EXC_ON();             /* enables bus */

	/*---------------------------------------------------* 
       	  Write the WID planes for all necessary windows.
       	  This is done by satisfying all the delayed WID writes.
      	 *---------------------------------------------------*/
	pDelayed = ddf-> dom_delayed_WID_writes_top ; 

       	BUGLPR(dbg_midgeom, 1, ("Top of delayed WID loop," 
				" first WG = 0x%8X \n", pDelayed )) ;

	while (pDelayed != NULL)
	{
	    if ((pDelayed->wgflags) & MID_WID_WRITE_DELAYED)
	    { 
		pDelayed-> wgflags &= ~(MID_WID_WRITE_DELAYED);

       	        BUGLPR(dbg_midgeom, 1, ("     delayed WID "
			          "write for WG = 0x%8X \n", pDelayed )) ;

		write_WID_planes_fifo (ddf, pDelayed -> wid, 
						pDelayed -> pRegion); 
                
		temp = pDelayed -> pNext_ctx_sw_delay ;
		pDelayed -> pNext_ctx_sw_delay = NULL ;

		pDelayed = temp ;
	    } 
	    else 
	    { 
       	        BUGLPR(dbg_midgeom, 0, ("DELAYED WID FLAG "
			          "NOT SET, WG = 0x%8X \n", pDelayed )) ;

                MID_ASSERT(0, UPDATE_WG , ddf, 
				ddf-> dom_delayed_WID_writes_top, 
						pDelayed, 0, 0) ;
       	        break ;
	    } 
	}

	ddf-> dom_delayed_WID_writes_top = NULL ;
	ddf-> dom_delayed_WID_writes_bot = NULL ;

	/*---------------------------------------------------* 
      	  Perform any delayed Assoc WID with color palette I/O.
      	 *---------------------------------------------------*/
       	BUGLPR(dbg_midgeom, 1, 
			("Top of delayed Assoc loop,  count = %X \n", 
			ddf-> dom_delayed_Assoc_updates_count )) ;

	for (	i=0 ; i < ddf-> dom_delayed_Assoc_updates_count ; i += 1 ) 
	{
		WID_id = ddf-> dom_delayed_Assoc_updates[i];

	        BUGLPR(dbg_midgeom, 1, ("     delayed Assoc for WID = %X \n",
					 	WID_id )) ;

		WID_entry = &(ddf->mid_wid_data.mid_wid_entry[WID_id]);

    		/*---------------------------------------------------------
	          Get the current WID color and WID frame buffer.
   	 	----------------------------------------------------------*/
    		WID_color = WID_entry-> pi.pieces.color ;
    		WID_fb   = WID_entry-> pi.pieces.flags ;

    		/*---------------------------------------------------------
	   	Convert the WID's frame buffer to the command format
   	 	----------------------------------------------------------*/
    		fb_data = ASSOC_FRAME_BUFFER_NULL ;

    		if ( (WID_entry-> state) != MID_WID_GUARDED )
    		{
    	    		if ((WID_fb & MID_USING_BACK_BUFFER) == 
                                                  MID_USING_BACK_BUFFER )
    	    		{
		    		fb_data = ASSOC_FRAME_BUFFER_B ;
    	    		}
    	    		else 
    	    		{
		    		fb_data = ASSOC_FRAME_BUFFER_A ;
    	    		}
    		}
 
        	BUGLPR (dbg_midgeom, 1, ("Assoc FIFO: WID = %X, fb_data =  %X \n", 
					WID_id, (WID_color | fb_data)) );

  		/*********************************************************
  	  	Now, update ped with the new Window/ColorPalette info 
  	 	*********************************************************/

 	MID_AssocOneWindowWithColorPaletteFIF(WID_id, (WID_color | fb_data) );

		WID_entry-> WID_flags &= ~(WID_DELAYED_ASSOC_UPDATE) ;

       	    	MID_DD_TRACE_PARMS (midgeom, 3, UPDATE_WG , 
					ddf, WID_id, WID_entry,
                        		WID_color, fb_data );
	} 

	ddf-> dom_delayed_Assoc_updates_count = 0 ;

  	PIO_EXC_OFF();    /* disables bus */

        MID_DD_ENTRY_TRACE (midgeom, 1, FLUSH_WIDS_COLORMP_UPDATE, ddf,
        			ddf, ddf->dom_delayed_WID_writes_top, 
                        	ddf->dom_delayed_Assoc_updates_count,
                       	       (ddf->dom_delayed_Assoc_updates_count | 0xF0)) ; 
 }



write_WID_planes_fifo (ddf, WID_parm, exposure_list)
midddf_t	* ddf;
mid_wid_t	 WID_parm;
gRegion		*exposure_list; 
{

   int box;
   ulong     	*box_ptr  ;

   midWG_t			*pmidWG ;

   MIDDimensionRectangle dim_rect;

   HWPDDFSetup;   



   MID_DD_ENTRY_TRACE (midgeom, 1, WRITE_DELAY_WID_PLANE_FIFO, ddf,
       			ddf, WID_parm, exposure_list, MID_UPPER_LEFT_ORIGIN ) ; 

    /*------------------------------------------------------------------*
       echo the input.  
     *------------------------------------------------------------------*/
	 
    BUGLPR (dbg_midgeom, 1, ("Top of mid write WID planes FIFO \n") ); 
    BUGLPR (dbg_midgeom, 2, 
		("WID_parm = %X, Region @ = 0x%8X,  Origin assumed UL\n", 
			WID_parm, exposure_list)); 

    MID_DEBUG_REGION (WID_planes, 1, exposure_list) ;


    /*------------------------------------------------------------------*
       Check for a non-NULL region
     *------------------------------------------------------------------*/
	 
    if (exposure_list == NULL)
    { 
	BUGLPR(dbg_midgeom, BUGNFO+1 ,("Exposure list NULL \n"));

        MID_DD_EXIT_TRACE (midgeom, 11, WRITE_DELAY_WID_PLANE_FIFO, ddf,
        			ddf, WID_parm, exposure_list, 0xF1) ; 
	BUGLPR(dbg_midgeom, BUGNFO, ("EXIT 1: write_WID_planes_fifo\n\n"));
	return(0);
    } 

    /*------------------------------------------------------------------*
       Check for a no visible boxes in the region
     *------------------------------------------------------------------*/
	 
    if ( (exposure_list -> numBoxes) == 0)
    { 
	BUGLPR(dbg_midgeom, BUGNFO+1 ,("NO VISIBLE BOXES \n"));

        MID_DD_EXIT_TRACE (midgeom, 11, WRITE_DELAY_WID_PLANE_FIFO, ddf,
        			ddf, WID_parm, exposure_list, 0xF2 ) ; 
	BUGLPR(dbg_midgeom, BUGNFO, ("EXIT 2:  write_WID_planes_fifo\n\n"));
	return(0);
    } 


    /************************************************************************** 
     WID WRITE LOOP 
     
     Now, for every exposure rectangle in the exposure list, 
     write the rectangle to the specified window ID.         
     
    **************************************************************************/

   BUGLPR(dbg_midgeom, BUGNFO,("numBoxes = %d\n", exposure_list->numBoxes) );


   for (box=0; box < exposure_list->numBoxes; box++)
   { 

    	dim_rect.rulx = exposure_list->pBox[box].ul.x;
    	dim_rect.ruly = exposure_list->pBox[box].ul.y;

    	/*------------------------------------------------------------------*
       	Calculate window width 
     	*------------------------------------------------------------------*/
    	dim_rect.rwth = exposure_list -> pBox[box].lr.x -
			exposure_list -> pBox[box].ul.x;


    	/*------------------------------------------------------------------*
       	Calculate window height 

      		X calculates all the regions.  An upper left origin is 
		assumed in all cases. 
     	*------------------------------------------------------------------*/

	dim_rect.rht = exposure_list -> pBox[box].lr.y -
				exposure_list -> pBox[box].ul.y;


    	/*------------------------------------------------------------------*
     	  Ensure the rectangle has a non-zero size
     	*------------------------------------------------------------------*/
    	if ( (dim_rect.rwth * dim_rect.rht) != 0 )
    	{
    		if ( dim_rect.rht < 0 )
    		{
		     MID_BREAK(WRITE_DELAY_WID_PLANE_FIFO,ddf, &dim_rect,0,0,0);

    		     BUGLPR(dbg_midgeom, 1,
			("Box dimensions resulted in a negative height!!\n") );
    			dim_rect.rht =  0 - dim_rect.rht ;
    		}

    		BUGLPR(dbg_midgeom, BUGNFO,("Box = %d by %d at (0x%X, 0x%X)\n",
           	dim_rect.rwth, dim_rect.rht, dim_rect.rulx, dim_rect.ruly ) );

    		MID_ClearControlPlanesFIF (
			(WID_parm << CLEAR_CONTROL_PLANES_wid_SHIFT), 
                           CLEAR_CONTROL_PLANES_wid_only_MASK ,
                           (&(dim_rect)),
                           CLEAR_CONTROL_PLANES_WID_COMPARE_off, 
                           WID_parm, 0x0dac);
    	}
   }

   BUGLPR(dbg_midgeom, 1, ("Leaving write_WID_planes fifo\n\n"));
   MID_DD_EXIT_TRACE (midgeom, 11, WRITE_DELAY_WID_PLANE_FIFO, ddf,
        			ddf, WID_parm, exposure_list, 0xF0 ) ; 
   return(0);
}
