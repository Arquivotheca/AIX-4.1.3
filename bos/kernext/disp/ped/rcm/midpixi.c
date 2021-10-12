static char sccsid[] = "@(#)77	1.15.1.7  src/bos/kernext/disp/ped/rcm/midpixi.c, peddd, bos411, 9428A410j 3/31/94 21:36:22";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: fix_color_palette
 *		fix_color_palette_PCB
 *		mid_flush_rcm_fifo1915
 *		mid_set_window_parms_sub1585
 *		write_WID_planes_FIFO
 *		write_WID_planes_PCB1078
 *		write_WID_planes_PCB_queue
 *		write_window_parms_FIFO1449
 *		write_window_parms_PCB1323
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
 ***************************************************************************** 

       Debug variables

 *--------------------------------------------------------------------------* 

    The first set of defines turn on the printf statements in the I/O macros  
    which print out the structure elements (SE) just after they have been       
    set up and just before they are passed to the PCB/FIFO macro write layer.   
    These symbols merely need to be defined to turn the printfs on.             

    The second set of defines turn on special print outs in the code.           

 ***************************************************************************** 
 ******************************************************************************/


#if 0 
#define MIDPIXI_DEBUG_WID_OVERLAYS
#define MIDPIXI_DEBUG_SWP_ORIGIN_CHECK
#endif 

#if 1 
#endif 


#ifdef MIDPIXI_DEBUG_WID_OVERLAYS
#define MIDPIXI_DEBUG_WID  (( (WID_entry -> WID_write_count) & 3) << 2) 
#define MIDPIXI_DEBUG_WID_MASK   0xFFFFFFF3 
#else 
#define MIDPIXI_DEBUG_WID  0
#define MIDPIXI_DEBUG_WID_MASK   0xFFFFFFFF 
#endif 





/****************************************************************************** 
   Includes:  
 ******************************************************************************/

#include <sys/types.h>
#include <sys/syspest.h>
#include <sys/malloc.h>
#include <sys/sleep.h>
#include <sys/xmem.h>
#include <sys/rcm_win.h>
#include <sys/aixgsc.h>
#include <sys/rcm.h>
#include "mid.h"
#include "midhwa.h"
#include "hw_dd_model.h"
#include "hw_typdefs.h"

#include "hw_macros.h"
#include "hw_regs_u.h"  
#include "hw_regs_k.h"  
#include "hw_ind_mac.h"

#include "hw_PCBkern.h"
#include "hw_FIFkern.h"

#include "midddf.h"
#include "midwidmac.h"
#include "mid_deferbuf.h"
#include "midRC.h"
#include "mid_dd_trace.h"

MID_MODULE (midpixi);
#define dbg_middd dbg_midpixi 
#define dbg_syncwid dbg_midpixi 


/*************************************************************************** 
   Externals defined herein: 
 ***************************************************************************/

extern long write_WID_planes_FIFO (
					gscDev 	  *, 
					rcmWG 	  *,	
					mid_wid_t,	
					gRegion	  *, 
					ushort     ) ;

extern long write_WID_planes_PCB_queue  (
					midddf_t  *,
					rcmWG 	  *,	
					mid_wid_t,	
					gRegion	  *, 
					ushort     ) ;



extern long write_WID_planes_PCB  (
					midddf_t  *,
					mid_wid_t,	
					gRegion	  *, 
					ushort     ) ;



extern long write_window_parms_FIFO  (
					gscDev 	  *, 
					mid_wid_t,	
					gRegion	  *, 
					ushort,   
					gPoint,   
					ushort,   
					ushort,   
					ushort,   
					ushort     ) ;


extern long write_window_parms_PCB  (
					midddf_t  *,
					mid_rcx_t *,	
					mid_wid_t,	
					gRegion	  *, 
					gPoint 	  *,   
					ushort,   
					ushort     ) ;
  
extern long fix_color_palette(
				midddf_t  *,           
				mid_wid_t, 
				ushort,
				ushort) ;


extern long fix_color_palette_PCB (
				midddf_t  *,           
				mid_wid_t) ;


long mid_set_window_parms_sub (
				midddf_t	*,  
				gRegion		*,
				gPoint,   
				ushort,   
				ushort,
				ushort		*,
				ulong		*,
				gBox		*,
				gBox		*) ;






/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:    mid_write_WID_planes
  
   Descriptive Name:  Mid-level Graphics Adapter, Device Dependent
                        Write WID Planes    
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      
      Writes the window ID planes (into the FIFO).
  

 *--------------------------------------------------------------------------* 
  
    Restrictions:
                  none
  
    Dependencies:
  
  
 *--------------------------------------------------------------------------* 
  
   Linkage:

  
 *--------------------------------------------------------------------------* 
  
    INPUT:
            . a pointer to the virtual terminal structure
            . the WID to be written, 
            . the exposure region (where to write the WID), 
            . the origin/orientation (upper left or lower left)        
          
    OUTPUT:  none 
          
    RETURN CODES:  none        
          
          
          
                                 End of Prologue                                
 ******************************************************************************/


  
long write_WID_planes_FIFO (
			gscDev	        *pGD,
			rcmWG	  	*pWG,
			mid_wid_t	 WID_parm,	
			gRegion		*exposure_list, 
			ushort		 OriginParm )
{

#if 0
midddf_t 	*ddf 	= (midddf_t *)(pGD -> devHead.display -> free_area);

ushort 		correlator, last_correlator;

short   	boxh, boxw, box;/* exposure rectangle height, width */
ulong     	*box_ptr  ;

mid_wid_status_t	*WID_entry ;
midWG_t			*pmidWG ;

MIDDimensionRectangle dim_rect;


HWPDDFSetup;              /* declares variables and get bus access */


    MID_DD_ENTRY_TRACE (midpixi, 1, WRITE_WID_PLANES_FIFO, ddf,
        			pGD, WID_parm, exposure_list, OriginParm ) ; 



    /*------------------------------------------------------------------*
       echo the input.  
     *------------------------------------------------------------------*/
	 
    BUGLPR (dbg_midpixi, 1, ("Top of write WID planes FIFO \n") ); 
    BUGLPR (dbg_midpixi, 2, 
		("WID_parm = %X, Region @ = 0x%8X,  Origin =  0x%X\n", 
			WID_parm, exposure_list, OriginParm)); 
    BUGLPR (dbg_midpixi, 3,("pGD = 0x%8X, ddf = 0x%8X\n", pGD, ddf));

    MID_DEBUG_REGION (WID_planes, 1, exposure_list) ;


    /*------------------------------------------------------------------*
       Check for a non-NULL region
     *------------------------------------------------------------------*/
	 
    if (exposure_list == NULL)
    { 
	BUGLPR(dbg_midpixi, BUGNFO+1 ,("Exposure list NULL \n"));

        MID_DD_EXIT_TRACE (midpixi, 11, WRITE_WID_PLANES_FIFO, ddf,
        			pGD, WID_parm, exposure_list, 0xF1 ) ; 
	BUGLPR(dbg_midpixi, BUGNFO, ("EXIT 1: write_WID_planes FIFO\n\n"));
	return(0);
    } 



    /*------------------------------------------------------------------*
       Check for a no visible boxes in the region
     *------------------------------------------------------------------*/
	 
    if ( (exposure_list -> numBoxes) == 0)
    { 
	BUGLPR(dbg_midpixi, BUGNFO+1 ,("NO VISIBLE BOXES \n"));

        MID_DD_EXIT_TRACE (midpixi, 11, WRITE_WID_PLANES_FIFO, ddf,
        			pGD, WID_parm, exposure_list, 0xF2 ) ; 
	BUGLPR(dbg_midpixi, BUGNFO, ("EXIT 2:  write_WID_planes FIFO\n\n"));
	return(0);
    } 





     	/*------------------------------------------------------------------*
     	   Check for a context switch in process
        
     	   In this event, the WID write must be delayed. 
     	   Put it on the delay list (if it isn't already).

	   This section removed as it was out of date
      	*------------------------------------------------------------------*/
 	 
 
 





    /*------------------------------------------------------------------*
       This code is for debugging window ID writes. It allows visual checking
       of what is actually written by the microcode.
     *------------------------------------------------------------------*/

#	ifdef MIDPIXI_DEBUG_WID_OVERLAYS
	/********************************************************************** 
    	    Increment the WID written count                                     
 	**********************************************************************/

	WID_entry = &(ddf -> mid_wid_data.mid_wid_entry[WID_parm] ) ;
	WID_entry -> WID_write_count += 1 ;
	BUGLPR(dbg_midpixi, BUGNFO,
		("WID write count = %X \n", WID_entry -> WID_write_count ));
	BUGLPR(dbg_midpixi, BUGNFO,
		("MIDPIXI DEBUG WID = %X \n", MIDPIXI_DEBUG_WID  ));
#	endif 



    /*------------------------------------------------------------------*
       Prepare for I/O operations 
     *------------------------------------------------------------------*/

  	PIO_EXC_ON();             /* enables bus */







  /************************************************************************** 
     WID WRITE LOOP 
     
     Now, for every exposure rectangle in the exposure list, 
     write the rectangle to the specified window ID.         
     
     About the correlator - get one before the for loop.  Use this for
     last call to MID_ClearControlPlanes.  This will be the unique
     correlator.  For all of the other calls use the correlator in 
     ddf->last_WID_change_corr. This correlator value will be valid (1)
     even if this the first pass - initialized in midcfgvt.c. 

     Disable/enable interrupts - we need to disable interrputs to
     prevent commands and correlators from getting out of sync. 
   **************************************************************************/
  BUGLPR(dbg_midpixi, BUGNFO,("numBoxes = %d\n", exposure_list->numBoxes) );

  mid_get_correlator( pGD, &last_correlator);

  correlator = ddf -> mid_last_WID_change_corr;

  BUGLPR(dbg_midpixi, BUGACT,
        ("Got correlator. correlator=0x%X\n", last_correlator));



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
       
       	   The window height must be a positive number, therefore, we must use
       	   the origin type (upper left/lower left) to determine how to make the
       	   proper computation.  As a precursor to the calculation, we verify 
       	   the origin type is correct.  
     	*------------------------------------------------------------------*/

    	if ((OriginParm != REG_ORG_UL) && (OriginParm != REG_ORG_LL))
    	{ 					/* invalid origin passed  */
      	BUGLPR(dbg_midpixi, 1, ("*** ERROR ***  Origin Parm (%d) bad"
                          	"-- Default to UL \n\n", OriginParm));
    	}


    	if (OriginParm == REG_ORG_LL) 		/* Origin is lower left */
    	{ 
      		dim_rect.rht = exposure_list -> pBox[box].ul.y -
				exposure_list -> pBox[box].lr.y;
    	}
    	else /* Origin is Upper Left */
    	{ 
      		dim_rect.rht = exposure_list -> pBox[box].lr.y -
				exposure_list -> pBox[box].ul.y;
    	}


    	/*------------------------------------------------------------------*
     	  Ensure the rectangle has a non-zero size
     	*------------------------------------------------------------------*/
    	if ( (dim_rect.rwth * dim_rect.rht) != 0 )
    	{
    		if ( dim_rect.rht < 0 )
    		{
    			BUGLPR(dbg_midpixi, 1,
			("Box dimensions resulted in a negative height!!\n") );
    			dim_rect.rht =  0 - dim_rect.rht ;
    		}

    		BUGLPR(dbg_midpixi, BUGNFO,("Box = %d by %d at (0x%X, 0x%X)\n",
           	dim_rect.rwth, dim_rect.rht, dim_rect.rulx, dim_rect.ruly ) );

		/* If last box - pass the 'last_correlator' */

		if(box == (exposure_list->numBoxes-1)) {
			correlator = last_correlator;
		}
    		MID_ClearControlPlanesFIF (
			(WID_parm << CLEAR_CONTROL_PLANES_wid_SHIFT) +
                           MIDPIXI_DEBUG_WID , 
                           CLEAR_CONTROL_PLANES_wid_only_MASK &
                           MIDPIXI_DEBUG_WID_MASK , 
                           (&(dim_rect)),
                           CLEAR_CONTROL_PLANES_WID_COMPARE_off, 
                           WID_parm,
			   correlator);
    	}
  }

  /************************************************************** 
   		Update the correlator and flag
   **************************************************************/

  BUGLPR(dbg_midpixi, BUGNFO, ("Correlator now %4X,  flag = %X\n",
	ddf -> mid_last_WID_change_corr, ddf -> WID_change_flag ));

  ddf -> mid_last_WID_change_corr = correlator ; 
  ddf -> WID_change_flag = MID_WID_CHANGE_PEND ;

  BUGLPR(dbg_midpixi, BUGNFO, ("Correlator now %4X,  flag = %X\n",
	ddf -> mid_last_WID_change_corr, ddf -> WID_change_flag ));




  /*******************/
  /* release the bus */
  /*******************/

  PIO_EXC_OFF();    /* disables bus */



	BUGLPR(dbg_midpixi, 1, ("Leaving write_WID_planes FIFO\n\n"));
        MID_DD_EXIT_TRACE (midpixi, 11, WRITE_WID_PLANES_FIFO, ddf,
        			pGD, WID_parm, exposure_list, 0xF0 ) ; 
	return(0);
#endif
}








   
/****************************************************************************** 
    Name: fix_color_palette()                                                 
                                                                             
    Purpose:                                                                
          This routine will associate a specified window's ID with           
          a specified color table.  This association is sent to the adapter   
          (via a FIFO write).                                                  
                                                                           
    Inputs:                                                                
          ddf         : a pointer used to access the device dependent data 
                        structure.  This stucture contains the adaptor's  
                        bus address which is required this routine.      
                                                                        
          WID_parm    :  the window id.                                  
                                                                      
          cp_id_param :  the color palette id.                       
                             
          frame_buffer :  0 = A, non-zero = B

                                                                    
    Calls :                                                        
          HWPDDFSetup : Access the Hardware Bus (Macro)           
                                                                 
          MID_AssocOneWindowWithColorPaletteFIF ()                  
                      : send the appropriate structure element to the  
                        hardware to relate the window and palette ID's
                        and window ID with the correct frame buffer.
                                                                     
    Outputs:  NONE                                                  
                                                                   
 ******************************************************************************/

long fix_color_palette(
			midddf_t       *ddf,           /* ddf address */
			mid_wid_t 	WID_parm, 
			ushort 		cp_id_param,
			ushort		frame_buffer )
{
  mid_wid_status_t	*WID_entry ;

  ushort	color, WIDcolor ;
  ushort	fb_data, WID_fb ; 

  int		saved_intr_priority;	/* Disable/enable interrupts */

  HWPDDFSetup;                		/* defines / inits vars for bus acc */



  BUGLPR(dbg_midpixi, BUGNFO,
	("Entering fix_color_palette -- WID = %X, color = %X, f/b = %X\n",
					 WID_parm, cp_id_param, frame_buffer));
  BUGLPR(dbg_midpixi, BUGNFO+2,("ddf = 0x%8X,  HWP=0x%8X\n",ddf, ddf->HWP));



#if 0
  MID_DD_ENTRY_TRACE (midpixi, 9, ASSOCIATE_COLOR, ddf,
        			ddf, WID_parm, cp_id_param, frame_buffer) ; 

  	MID_DD_ENTRY_TRACE (midpixi, 1, ASSOCIATE_COLOR, ddf,
        		WID_entry-> WID_flags, 
			ddf->dom_delayed_Assoc_updates_count,
			ddf->dom_delayed_Assoc_updates[0],  
			ddf->dom_delayed_Assoc_updates[1] );
#endif

  /********************************************************* 
     Check the passed color   
   *********************************************************/

  color =  cp_id_param ; 
  if (cp_id_param >  ddf->max_color_IDs)
  { 
    BUGLPR(dbg_midpixi,1, ("** ERROR ** Color (%d) out of range (0 - %d)\n",
                                  cp_id_param, ddf->max_color_IDs));
    BUGLPR(dbg_midpixi, BUGNFO,("********** Force to color 0 ******* \n"));
    color = 0 ; 
  }




    /*----------------------------------------------------------------
       Get the current WID color and WID frame buffer.
    ----------------------------------------------------------------*/
    WID_entry = &(ddf -> mid_wid_data.mid_wid_entry[WID_parm] ) ;

    WIDcolor = WID_entry-> pi.pieces.color ;
    WID_fb   = WID_entry-> pi.pieces.flags ;

    MID_DD_ENTRY_TRACE (midpixi, 9, ASSOCIATE_COLOR, ddf, ddf,
        		((WID_parm) << 16 | (WID_entry->state) << 8 |
			  ddf->dom_delayed_Assoc_updates_count),
			( ddf->dom_delayed_Assoc_updates[0] << 24,  
			  ddf->dom_delayed_Assoc_updates[1] << 16,
			  ddf->dom_delayed_Assoc_updates[2] <<  8,
			  ddf->dom_delayed_Assoc_updates[3] ) , 
  			( (cp_id_param << 24) | (frame_buffer << 16) |
        		  MID_DDT_WID_PI(WID_entry->mid_wid)) ) ;


    /*----------------------------------------------------------------
       Check if frame buffer update is required

       First convert the passed frame buffer parameter to the required
       format. 
    ----------------------------------------------------------------*/

    BUGLPR(dbg_midpixi, 1, ("WID = %X,  passed F/B = %X, WID F/B = %X \n",
				WID_parm, frame_buffer, WID_fb));

    fb_data = ASSOC_FRAME_BUFFER_NULL ;

    if ( (WID_entry-> state) != MID_WID_GUARDED )
    {
    	if (((frame_buffer & MID_USING_BACK_BUFFER) != MID_USING_BACK_BUFFER) &&
	         ( (WID_fb & MID_USING_BACK_BUFFER) == MID_USING_BACK_BUFFER) )
    	{
		fb_data = ASSOC_FRAME_BUFFER_A ;
    		WID_entry-> pi.pieces.flags &= ~MID_USING_BACK_BUFFER ;

        	BUGLPR(dbg_midpixi, 1, ("F/B changed to A (%X) \n",
    			WID_entry-> pi.pieces.flags ));
    	}
 

    	if (((frame_buffer & MID_USING_BACK_BUFFER) == MID_USING_BACK_BUFFER) &&
	 	( (WID_fb & MID_USING_BACK_BUFFER) != MID_USING_BACK_BUFFER) )
    	{
		fb_data = ASSOC_FRAME_BUFFER_B ;
    		WID_entry-> pi.pieces.flags |= MID_USING_BACK_BUFFER ;

        	BUGLPR(dbg_midpixi, 1, ("F/B changed to B (%X) \n",
    			WID_entry-> pi.pieces.flags ));
    	}
    }
 
    BUGLPR(dbg_midpixi, 1, ("fb_data =  %X \n", fb_data));






    BUGLPR(dbg_midpixi, 1, ("WG color = %d, WID color = %d\n",color,WIDcolor));

    if ( (color == WIDcolor) && (fb_data == ASSOC_FRAME_BUFFER_NULL) )
    {
    	BUGLPR(dbg_midpixi, 1, ("Exit Fix color: No change! \n\n") );
 
    	MID_DD_EXIT_TRACE (midpixi, 19, ASSOCIATE_COLOR, ddf,
          			ddf, WID_parm, cp_id_param, 0xF1) ; 
  
        return (MID_RC_OK) ;
    }

    WID_entry-> pi.pieces.color = color ;






  /****************************************************************** 

     	   We no longer check for a context switch in process.
     	   Even though the adapter can probably process these PCB commands
           at any time, they have chosen to ignore them during
     	   certain windows.  The only time we can guarantee getting
     	   this command executed, is during a context switch.

     	   So we will stack the I/O until then. 

   ******************************************************************/
 	 
        /*------------------------------------------------------------------*
     	   Disable interrupts
	 *------------------------------------------------------------------*/
  	saved_intr_priority = i_disable (INTMAX) ;  


        if ( !(WID_entry-> WID_flags & WID_DELAYED_ASSOC_UPDATE) )
 	{ 		
        	WID_entry-> WID_flags |= WID_DELAYED_ASSOC_UPDATE ;

 		BUGLPR(dbg_midpixi, 1, ("Put WID next on list \n"));
 		BUGLPR(dbg_midpixi, 1, 
			("count =  %X, first = %X, second = %X \n", 
			ddf->dom_delayed_Assoc_updates_count,
			ddf->dom_delayed_Assoc_updates[0],  
			ddf->dom_delayed_Assoc_updates[1] ));

		ddf->dom_delayed_Assoc_updates
			    [ddf->dom_delayed_Assoc_updates_count] = WID_parm ; 

		ddf->dom_delayed_Assoc_updates_count += 1 ;

 		BUGLPR(dbg_midpixi, 1, ("Put WID next on list \n"));
 		BUGLPR(dbg_midpixi, 1, 
			("count =  %X, first = %X, second = %X \n", 
			ddf->dom_delayed_Assoc_updates_count,
			ddf->dom_delayed_Assoc_updates[0],  
			ddf->dom_delayed_Assoc_updates[1] ));
 	} 		
 


        /*------------------------------------------------------------------*
     	   Re-enable interrupts and exit
	 *------------------------------------------------------------------*/
  	i_enable (saved_intr_priority) ;  


    	MID_DD_EXIT_TRACE (midpixi, 8, ASSOCIATE_COLOR, ddf, ddf,
        		((WID_parm) << 16 | (WID_entry->state) << 8 |
			ddf->dom_delayed_Assoc_updates_count),
			( ddf->dom_delayed_Assoc_updates[0] << 24,  
			  ddf->dom_delayed_Assoc_updates[1] << 16,
			  ddf->dom_delayed_Assoc_updates[2] <<  8,
			  ddf->dom_delayed_Assoc_updates[3] ) , 
  			( (cp_id_param << 24) | (frame_buffer << 16) |
        		  MID_DDT_WID_PI(WID_entry->mid_wid)) ) ;

 	BUGLPR(dbg_midpixi, 1, ("EXIT 3:  fix_color_palette Q\n\n"));
 	return (0) ;
}






   
/****************************************************************************** 
    Name: fix_color_palette_PCB ()                                                 
                                                                             
    Purpose:                                                                
          This routine will associate a specified window's ID with           
          a specified color table.  This association is sent to the adapter   
          (via a PCB write).                                                  
                                                                           
    Inputs:                                                                
          ddf         : a pointer used to access the device dependent data 
                        structure.  This stucture contains the adaptor's  
                        bus address which is required this routine.      
                                                                        
          WID_parm    :  the window id.                                  
                                                                      
                                                                    
    Calls :                                                        
          HWPDDFSetup : Access the Hardware Bus (Macro)           
                                                                 
          MID_AssocOneWindowWithColorPalettePCB ()                  
                      : send the appropriate structure element to the  
                        hardware to relate the window and palette ID's
                        and window ID with the correct frame buffer.
                                                                     
    Outputs:  NONE                                                  
                                                                   
 ******************************************************************************/

long fix_color_palette_PCB(
			midddf_t       *ddf,           /* ddf address */
			mid_wid_t 	WID_parm )
{
  mid_wid_status_t	*WID_entry ;

  midWG_t	*midWG ;
  ushort	WID_color ;
  ushort	WID_fb, fb_data  ; 

  HWPDDFSetup;                		/* defines / inits vars for bus acc */




    	/*----------------------------------------------------------------
	   Get the current WID color and WID frame buffer.
   	 ----------------------------------------------------------------*/
    	WID_entry = &(ddf -> mid_wid_data.mid_wid_entry[WID_parm] ) ;

    	WID_color = WID_entry-> pi.pieces.color ;
    	WID_fb   = WID_entry-> pi.pieces.flags ;


  	BUGLPR(dbg_midpixi, BUGNFO,
	("Entering fix_color_palette_PCB -- WID = %X, color = %X, f/b = %X\n",
					 WID_parm,  WID_color, WID_fb ));
  	BUGLPR(dbg_midpixi, 2,("ddf = 0x%8X,  HWP=0x%8X\n",ddf, ddf->HWP));


    	midWG = WID_entry->pwidWG ;
	MID_DD_ENTRY_TRACE (midpixi, 9, ASSOCIATE_COLOR_PCB, ddf, ddf,
               	    midWG, MID_DDT_WID(WID_parm), MID_DDT_WID_PI(WID_parm));


#if 	DEBUG

	while (midWG != NULL)
	{
  		MID_DD_TRACE_PARMS (midpixi, 1, ASSOCIATE_COLOR_PCB, ddf, ddf,
               	    midWG->pWG, MID_DDT_WID(midWG->wid), MID_DDT_PI(midWG));

		midWG = midWG->pNext ;
	}
    	midWG = WID_entry-> pwidWG ;
#endif


    	/*----------------------------------------------------------------
	   Convert the WID's frame buffer to the command format
   	 ----------------------------------------------------------------*/
    	fb_data = ASSOC_FRAME_BUFFER_NULL ;

    	if ( (WID_entry-> state) != MID_WID_GUARDED )
    	{
    	    if ( (WID_fb & MID_USING_BACK_BUFFER) == MID_USING_BACK_BUFFER )
    	    {
		    fb_data = ASSOC_FRAME_BUFFER_B ;
    	    }
    	    else 
    	    {
		    fb_data = ASSOC_FRAME_BUFFER_A ;
    	    }
    	}
 
        BUGLPR (dbg_midpixi, 1, ("Assoc PCB:   WID = %X,  fb_data =  %X \n", 
					WID_parm, (WID_color | fb_data)) );





  	PIO_EXC_ON();             /* enables bus */

  	/*********************************************************
  	  Now, update ped with the new Window/ColorPalette info 
  	 *********************************************************/

  	MID_AssocOneWindowWithColorPalette (WID_parm, (WID_color | fb_data) );


  	PIO_EXC_OFF();    /* disables bus */


  	MID_DD_EXIT_TRACE (midpixi, 19, ASSOCIATE_COLOR_PCB, ddf, ddf,
               	    	midWG,  MID_DDT_WID(WID_parm),
			MID_DDT_WID_PI(WID_parm) | 0xF0 );

  	BUGLPR(dbg_midpixi, BUGNFO+1,("Leaving fix_color_palette_PCB \n"));
  	return(0);
}




/******************************************************************************/
/*  Name: WRITE_WID_PLANES()                                                  */
/*                                                                            */
/*  Purpose:                                                                  */
/*        This routine will set a specified window's extents and visible areas*/
/*        then  clear the appropriate areas of the window to the specified    */
/*        window ID, based on the passed exposure list.                       */
/*                                                                            */
/*        ClearControlPlanes is used to write the specified window id         */
/*        to the requested areas of the window.                               */
/*                                                                            */
/*  NOTE: This routine specifically does not use the adaptor's checking       */
/*        facility.  This is because 'write_WID_planes()' gets                */
/*        a list of exposure rectangles in order to update them to a new      */
/*        window id. Since we are only sent exposure rectangles that need     */
/*        updating, we will always update the window id and there is no reason*/
/*        to perform a comparison check of the exposure rectangle             */
/*        (see ClearControlPlanes).                                           */
/*                                                                            */
/*                                                                            */
/*  Relavent Definitions:                                                     */
/*        Extent:                                                             */
/*           Defines a rectangular region, completely encompassing the        */
/*           Window.  Extent is described in terms of the rectangle's         */
/*           Upper Left point (ux,uy) and Lower Right point (lx,ly).          */
/*                                                                            */
/*        Region List:                                                        */
/*           Defines the area of the extent not overwritten by other windows. */
/*           This area is defined in terms of a list of rectangles and        */
/*           represents the union of the visible area and the area covered by */
/*           exposure rectangles.                                             */
/*                                                                            */
/*        Exposure List:                                                      */
/*           Defines the area of the extent that is overwritten by other      */
/*           windows This area is defined in terms of a list of rectangles.   */
/*           When the window is moved to a new position or moved above one or */
/*           more overwritten windows, the relavent members of this list      */
/*           must have their window id's changed from the previously          */
/*           overwritten  window id to the new window id.                     */
/*                                                                            */
/*  NOTE: This routine doesn't know or care if it is getting an exposure      */
/*        list or a region list (pointed to by the exposure list variable).   */
/*        This routine merely sets the areas in the list of rectangles to     */
/*        the specified ID.                                                   */
/*                                                                            */
/*        Early versions of code will send region lists (all visible areas    */
/*        of the window).  As the code matures, exposure lists will be be     */
/*        sent, thus only updating areas of the region list that have         */
/*        change.                                                             */
/*                                                                            */
/******************************************************************************/



long write_WID_planes_PCB_queue  (
				midddf_t      *ddf ,
				rcmWG         *pWG ,
				mid_wid_t      WID, 
				gRegion       *exposure_list, 
				ushort         OriginParm) 
{
	midWG_t			*pmidWG ;


    	/*------------------------------------------------------------------*
   	    Trace / echo the input parms 
     	*------------------------------------------------------------------*/
	 
    	MID_DD_ENTRY_TRACE (midpixi, 1, WRITE_WID_PLANES_FIFO, ddf,
        				ddf, pWG, exposure_list, WID) ; 

	MID_DEBUG_REGION (WID_planes, 1, exposure_list) ;



	BUGLPR(dbg_midpixi, BUGNFO,("      Entering write_WID_planes_queue\n"));
	BUGLPR(dbg_midpixi, BUGNFO,("ddf = 0x%X, HWP = 0x%X\n",ddf, ddf->HWP));
	BUGLPR(dbg_midpixi, BUGACT,
        	("WID parm = %X,  exposure_list = 0x%X, Origin Parm = %d\n",
          	WID, exposure_list,OriginParm));




    /*------------------------------------------------------------------*
       Check for a non-NULL region
     *------------------------------------------------------------------*/
	 
    if (exposure_list == NULL)
    { 
	BUGLPR(dbg_midpixi, BUGNFO+1 ,("Exposure list NULL \n"));

        MID_DD_EXIT_TRACE (midpixi, 1, WRITE_WID_PLANES_FIFO, ddf,
        				ddf, pWG, OriginParm, 0xF1 ) ; 
	BUGLPR(dbg_midpixi, BUGNFO, ("EXIT 1: write_WID_planes queue \n\n"));
	return(0);
    } 




    /*------------------------------------------------------------------*
       Check for a no visible boxes in the region
     *------------------------------------------------------------------*/
	 
    if ( (exposure_list -> numBoxes) == 0)
    { 
	BUGLPR(dbg_midpixi, BUGNFO+1 ,("NO VISIBLE BOXES \n"));

        MID_DD_EXIT_TRACE (midpixi, 1, WRITE_WID_PLANES_FIFO, ddf,
        			ddf, pWG, OriginParm, 0xF2 ) ; 
	BUGLPR(dbg_midpixi, BUGNFO, ("EXIT 2:  write_WID_planes queue \n\n"));
	return(0);
    } 



     	/*------------------------------------------------------------------*
     	   Save this WID write until the next context switch

     	   Put it on the delay list (if it isn't already).
      	*------------------------------------------------------------------*/
 	 
 	pmidWG = ((midWG_t *)(pWG->pPriv)) ; 

       	if ( !((pmidWG-> wgflags) & MID_WID_WRITE_DELAYED)  )
 	{ 		
       		pmidWG-> wgflags |= MID_WID_WRITE_DELAYED ;  

 		BUGLPR(dbg_midpixi, 2, ("Put WG on top of list \n"));
 		BUGLPR(dbg_midpixi, 2, ("delay list: 0x%X \n",
					ddf->dom_delayed_WID_writes_top));

 		if (ddf->dom_delayed_WID_writes_bot == NULL)
		{
			ddf->dom_delayed_WID_writes_top = pmidWG ;
		}
		else
		{
			ddf->dom_delayed_WID_writes_bot->pNext_ctx_sw_delay = 
				pmidWG ;
		}

		ddf->dom_delayed_WID_writes_bot = pmidWG ;
 		pmidWG-> pNext_ctx_sw_delay = NULL ;
 	} 		
 
 	BUGLPR(dbg_midpixi, 2, 
 		("delay list: 0x%X \n", ddf->dom_delayed_WID_writes_top));
 	BUGLPR(dbg_midpixi, 2, 
 		("WG next: 0x%X \n", ddf->dom_delayed_WID_writes_top->
					pNext_ctx_sw_delay )) ;
 	BUGLPR(dbg_midpixi, 2, 
 		("WG last: 0x%X \n", ddf->dom_delayed_WID_writes_bot )) ;
 
       	MID_DD_EXIT_TRACE (midpixi, 1, WRITE_WID_PLANES_FIFO, ddf,
         	    pmidWG->wgflags, ddf->dom_delayed_WID_writes_top, 
         			     ddf->dom_delayed_WID_writes_bot, 0xF3 ) ; 

 	BUGLPR(dbg_midpixi, 1, ("EXIT 3:  write_WID_planes_queue \n\n"));
 	return (0) ;
} 								 
 




/******************************************************************************/
/*  Name: WRITE_WID_PLANES()                                                  */
/*                                                                            */
/*  Purpose:                                                                  */
/*        This routine will set a specified window's extents and visible areas*/
/*        then  clear the appropriate areas of the window to the specified    */
/*        window ID, based on the passed exposure list.                       */
/*                                                                            */
/*        ClearControlPlanes is used to write the specified window id         */
/*        to the requested areas of the window.                               */
/*                                                                            */
/*  NOTE: This routine specifically does not use the adaptor's checking       */
/*        facility.  This is because 'write_WID_planes()' gets                */
/*        a list of exposure rectangles in order to update them to a new      */
/*        window id. Since we are only sent exposure rectangles that need     */
/*        updating, we will always update the window id and there is no reason*/
/*        to perform a comparison check of the exposure rectangle             */
/*        (see ClearControlPlanes).                                           */
/*                                                                            */
/*                                                                            */
/*  Relavent Definitions:                                                     */
/*        Extent:                                                             */
/*           Defines a rectangular region, completely encompassing the        */
/*           Window.  Extent is described in terms of the rectangle's         */
/*           Upper Left point (ux,uy) and Lower Right point (lx,ly).          */
/*                                                                            */
/*        Region List:                                                        */
/*           Defines the area of the extent not overwritten by other windows. */
/*           This area is defined in terms of a list of rectangles and        */
/*           represents the union of the visible area and the area covered by */
/*           exposure rectangles.                                             */
/*                                                                            */
/*        Exposure List:                                                      */
/*           Defines the area of the extent that is overwritten by other      */
/*           windows This area is defined in terms of a list of rectangles.   */
/*           When the window is moved to a new position or moved above one or */
/*           more overwritten windows, the relavent members of this list      */
/*           must have their window id's changed from the previously          */
/*           overwritten  window id to the new window id.                     */
/*                                                                            */
/*  NOTE: This routine doesn't know or care if it is getting an exposure      */
/*        list or a region list (pointed to by the exposure list variable).   */
/*        This routine merely sets the areas in the list of rectangles to     */
/*        the specified ID.                                                   */
/*                                                                            */
/*        Early versions of code will send region lists (all visible areas    */
/*        of the window).  As the code matures, exposure lists will be be     */
/*        sent, thus only updating areas of the region list that have         */
/*        change.                                                             */
/*                                                                            */
/******************************************************************************/




long write_WID_planes_PCB  (
				midddf_t      *ddf ,
				mid_wid_t      WID_parm, 
				gRegion       *exposure_list, 
				ushort         OriginParm) 
{
  short   boxh, boxw, box;       /* exposure rectangle height, width */
  ulong     *box_ptr  ;

  mid_wid_status_t	*WID_entry ;

  MIDDimensionRectangle dim_rect;

  HWPDDFSetup;              /* declares variables and get bus access */





    	/*------------------------------------------------------------------*
   	    Trace / echo the input parms 
     	*------------------------------------------------------------------*/
	 
    	MID_DD_ENTRY_TRACE (midpixi, 3, WRITE_WID_PLANES_PCB, ddf,
        			ddf, WID_parm, exposure_list, OriginParm) ; 

	MID_DEBUG_REGION (WID_planes, 1, exposure_list) ;



	BUGLPR(dbg_midpixi, BUGNFO,("         Entering write_WID_planes\n"));
	BUGLPR(dbg_midpixi, BUGNFO,("ddf = 0x%X, HWP = 0x%X\n",ddf, ddf->HWP));
	BUGLPR(dbg_midpixi, BUGACT,
        	("WID parm = %X,  exposure_list = 0x%X, Origin Parm = %d\n",
          	  WID_parm, exposure_list, OriginParm));




    /*------------------------------------------------------------------*
       Check for a non-NULL region
     *------------------------------------------------------------------*/
	 
    if (exposure_list == NULL)
    { 
	BUGLPR(dbg_midpixi, BUGNFO+1 ,("Exposure list NULL \n"));

        MID_DD_EXIT_TRACE (midpixi, 13, WRITE_WID_PLANES_PCB, ddf,
        			ddf, WID_parm, exposure_list, 0xF1 ) ; 
	BUGLPR(dbg_midpixi, BUGNFO, ("EXIT 1: write_WID_planes FIFO\n\n"));
	return(0);
    } 




    /*------------------------------------------------------------------*
       Check for a no visible boxes in the region
     *------------------------------------------------------------------*/
	 
    if ( (exposure_list -> numBoxes) == 0)
    { 
	BUGLPR(dbg_midpixi, BUGNFO+1 ,("NO VISIBLE BOXES \n"));

        MID_DD_EXIT_TRACE (midpixi, 13, WRITE_WID_PLANES_PCB, ddf,
        			ddf, WID_parm, exposure_list, 0xF2 ) ; 
	BUGLPR(dbg_midpixi, BUGNFO, ("EXIT 2:  write_WID_planes FIFO\n\n"));
	return(0);
    } 





    /*------------------------------------------------------------------*
       This code is for debugging window ID writes. It allows visual checking
       of what is actually written by the microcode.
     *------------------------------------------------------------------*/

#ifdef MIDPIXI_DEBUG_WID_OVERLAYS
	/********************************************************************** 
    	    Increment the WID written count                                     
 	**********************************************************************/

	WID_entry = &(ddf -> mid_wid_data.mid_wid_entry[WID_parm] ) ;
	WID_entry -> WID_write_count += 1 ;
	BUGLPR(dbg_midpixi, BUGNFO,
		("WID write count = %X \n", WID_entry -> WID_write_count ));
	BUGLPR(dbg_midpixi, BUGNFO,
		("MIDPIXI DEBUG WID = %X \n", MIDPIXI_DEBUG_WID  ));
#endif 



    /*------------------------------------------------------------------*
       Prepare for I/O operations 
     *------------------------------------------------------------------*/

  	PIO_EXC_ON();             /* enables bus */






  /************************************************************************** 
     WID WRITE LOOP 
     
     Now, for every exposure rectangle in the exposure list, 
     write the rectangle to the specified window ID.         
   **************************************************************************/
  BUGLPR(dbg_midpixi, BUGNFO,("numBoxes = %d\n", exposure_list->numBoxes) );


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
       
       	   The window height must be a positive number, therefore, we must use
       	   the origin type (upper left/lower left) to determine how to make the
       	   proper computation.  As a precursor to the calculation, we verify 
       	   the origin type is correct.
     	*------------------------------------------------------------------*/

    	if ((OriginParm != REG_ORG_UL) && (OriginParm != REG_ORG_LL))
    	{ 					/* invalid origin passed  */
      	BUGLPR(dbg_midpixi, 0, ("*** ERROR ***  Origin Parm (%d) bad"
                          	"-- Default to UL \n\n", OriginParm));
    	}


    	if (OriginParm == REG_ORG_LL) 		/* Origin is lower left */
    	{ 
      		dim_rect.rht = exposure_list -> pBox[box].ul.y -
				exposure_list -> pBox[box].lr.y;
    	}
    	else /* Origin is Upper Left */
    	{ 
      		dim_rect.rht = exposure_list -> pBox[box].lr.y -
				exposure_list -> pBox[box].ul.y;
    	}


    	/*------------------------------------------------------------------*
     	  Ensure the rectangle has a non-zero size
     	*------------------------------------------------------------------*/
    	if ( (dim_rect.rwth * dim_rect.rht) != 0 )
    	{
    		if ( dim_rect.rht < 0 )
    		{
    			BUGLPR(dbg_midpixi, 1,
			("Box dimensions resulted in a negative height!!\n") );
    			dim_rect.rht =  0 - dim_rect.rht ;
    		}

    		BUGLPR(dbg_midpixi, BUGNFO,("Box = %d by %d at (0x%X, 0x%X)\n",
           	dim_rect.rwth, dim_rect.rht, dim_rect.rulx, dim_rect.ruly ) );


    		MID_ClearControlPlanesPCB (
			(WID_parm << CLEAR_CONTROL_PLANES_wid_SHIFT) +
                           MIDPIXI_DEBUG_WID , 
                           CLEAR_CONTROL_PLANES_wid_only_MASK &
                           MIDPIXI_DEBUG_WID_MASK , 
                           (&(dim_rect)),
                           CLEAR_CONTROL_PLANES_WID_COMPARE_off, 
                           WID_parm);


    	}
  }



  /*******************/
  /* release the bus */
  /*******************/

  PIO_EXC_OFF();    /* disables bus */


  MID_DD_EXIT_TRACE (midpixi, 13, WRITE_WID_PLANES_PCB, ddf,
        			ddf, WID_parm, exposure_list, 0xF0 ) ; 

   BUGLPR(dbg_midpixi, BUGNFO,("Leaving write_WID_planes PCB \n\n"));
  return(0);
}



/******************************************************************************/
/*  Name: SET_WINDOW_PARAMETERS_PCB()                                         */
/*                                                                            */
/*  Purpose:                                                                  */
/*        This routine will set a specified window's extents and visible areas*/
/*        of the specified window ID, based on the passed exposure list.      */
/*                                                                            */
/*  Relevent Definitions:                                                     */
/*        Extent:                                                             */
/*           Defines a rectangular region, completely encompassing the        */
/*           Window.  Extent is described in terms of the rectangle's         */
/*           Upper Left point (ux,uy) and Lower Right point (lx,ly).          */
/*                                                                            */
/*        Region List:                                                        */
/*           Defines the area of the extent not overwritten by other windows. */
/*           This area is defined in terms of a list of rectangles and        */
/*           represents the union of the visible area and the area covered by */
/*           exposure rectangles. (This list is not used in this routine, but */
/*           is included for completeness)                                    */
/*                                                                            */
/*        Exposure List:                                                      */
/*           Defines the area of the extent that is overwritten by other      */
/*           windows This area is defined in terms of a list of rectangles.   */
/*           When the window is moved to a new position or moved above one or */
/*           more overwritten  windows, the relavent members of this list     */
/*           must have their window id's changed from the overwritten  window */
/*           to the current window.                                           */
/*                                                                            */
/******************************************************************************/

#define SWP_FLAGS_ORIGIN_LL   		0x0000
#define SWP_FLAGS_ORIGIN_UL   		0x8000
#define SWP_FLAGS_DISABLE_WID_CLIP 	0x4000
#define SWP_FLAGS_GEOMETRY_CHANGED 	0x2000

#define MAX_VIS_RECT 4




long write_window_parms_PCB (
				midddf_t      *ddf , 
				mid_rcx_t      *pmidRCX,     
				mid_wid_t      WID_parm,     
				gRegion        *exposure_list,  
				gPoint         *WindowOrgParam,   
				ushort         widthParam,   
				ushort         heightParam )  
{

  ulong   	vis_rect_count ;
  ushort  	woflags; 
  gBox          window_extents ; 
  gBox          visible_rects[MAX_VIS_RECT];

  HWPDDFSetup;              /* declares variables and get bus access */



  /****************************************************************** 
              	            TOP OF MODULE
   
           First record entry parameters for debug.
   ******************************************************************/


        MID_DD_ENTRY_TRACE (midpixi, 7, WRITE_WINDOW_PARMS_PCB, ddf, ddf,
        			WID_parm, 
				((WindowOrgParam->x<<16) | WindowOrgParam->y),
			 	( (widthParam << 16) | heightParam) ) ; 

        MID_DD_TRACE_PARMS (midpixi, 8, GENERIC_PARM3, ddf, ddf,
        			pmidRCX, exposure_list, 
				(*(ulong *)(&(pmidRCX->flags)) & 0xFFFF0000) |
			 	(pmidRCX->type & 0x0000FFFF) ) ; 


	BUGLPR(dbg_midpixi, BUGNFO,("Entering write_window_parms_PCB\n"));
	BUGLPR(dbg_midpixi, BUGACT, ("WID_parm 0x%X,  exposure_list 0x%X \n",
           				WID_parm, exposure_list));
	BUGLPR(dbg_midpixi, BUGNFO+3,("ddf = 0x%X, midRCX = 0x%X\n",
			  		ddf, pmidRCX ));
	BUGLPR(dbg_midpixi, 2, ("Origin = (%X, %X)\n", 
				WindowOrgParam->x, WindowOrgParam->y )) ;



  	/****************************************************************** 
   	  Most of the processing has been moved to the common subroutine
   	******************************************************************/

	mid_set_window_parms_sub ( ddf, exposure_list,
				   *WindowOrgParam, widthParam, heightParam,
				   &woflags, &vis_rect_count,
				   &window_extents, &visible_rects) ;


  	/****************************************************************** 
   	  Enable I/O operations 
   	******************************************************************/

  	PIO_EXC_ON();             /* enables bus */


  	/************************************************************** 
   	   Invoke the Set Window Parms macro to do the I/O.            
   	**************************************************************/

  	MID_SetWindowParametersPCB( WID_parm, woflags, (&(window_extents)),
                             		vis_rect_count, visible_rects);


  	/*******************/
  	/* release the bus */
  	/*******************/

  	PIO_EXC_OFF();    /* disables bus */



  	MID_DD_EXIT_TRACE (midpixi, 17, WRITE_WINDOW_PARMS_PCB, ddf,
        			0, WID_parm, 
				*((ulong *)(WindowOrgParam)),
				0xF0 ) ;

  	BUGLPR(dbg_midpixi, BUGNFO,("Leaving write_window_parmsPCB\n\n"));
  	return(0);
}




/******************************************************************************/
/*  Name: SET_WINDOW_PARAMETERS_FIF()                                         */
/*                                                                            */
/*  Purpose:                                                                  */
/*        This routine will set a specified window's extents and visible areas*/
/*        of the specified window ID, based on the passed exposure list.      */
/*                                                                            */
/*  Relevent Definitions:                                                     */
/*        Extent:                                                             */
/*           Defines a rectangular region, completely encompassing the        */
/*           Window.  Extent is described in terms of the rectangle's         */
/*           Upper Left point (ux,uy) and Lower Right point (lx,ly).          */
/*                                                                            */
/*        Region List:                                                        */
/*           Defines the area of the extent not overwritten by other windows. */
/*           This area is defined in terms of a list of rectangles and        */
/*           represents the union of the visible area and the area covered by */
/*           exposure rectangles. (This list is not used in this routine, but */
/*           is included for completeness)                                    */
/*                                                                            */
/*        Exposure List:                                                      */
/*           Defines the area of the extent that is overwritten by other      */
/*           windows This area is defined in terms of a list of rectangles.   */
/*           When the window is moved to a new position or moved above one or */
/*           more overwritten  windows, the relavent members of this list     */
/*           must have their window id's changed from the overwritten  window */
/*           to the current window.                                           */
/*                                                                            */
/******************************************************************************/


long write_window_parms_FIFO (
				gscDev         *pdev,  
				mid_wid_t      WID_parm,     
				gRegion        *exposure_list,  
				ushort         RegOriginParam,   
				gPoint         WindowOrgParam,   
				ushort         widthParam,   
				ushort         heightParam,
				ushort         color_palette,
				ushort         frame_buffer )  
{
  ushort 	correlator;
  midddf_t 	*ddf = (midddf_t *)(pdev->devHead.display->free_area);
  mid_rcx_t	*pmidRCX = ddf->current_context_midRCX ; 

  ushort  	woflags ; 
  ulong  	vis_rect_count ;
  gBox  	window_extents ; 
  gBox          visible_boxes[MAX_VIS_RECT];


  HWPDDFSetup;              /* declares variables and get bus access */


  	/****************************************************************** 
              	            TOP OF MODULE
   
           First record entry parameters for debug.
   	******************************************************************/

        MID_DD_ENTRY_TRACE (midpixi, 2, WRITE_WINDOW_PARMS_FIFO, ddf,
        			pdev, WID_parm, 
				*((ulong *)(&(WindowOrgParam))),
			 	((widthParam << 16) | heightParam)) ; 

        MID_DD_TRACE_PARMS (midpixi, 6, GENERIC_PARM3, ddf,
        		    pdev, ddf -> HWP, exposure_list, RegOriginParam) ; 



	BUGLPR(dbg_midpixi, BUGNFO,("Entering write_window_parms_FIFO\n"));
	BUGLPR(dbg_midpixi, BUGNFO,("pdev = 0x%X, HWP = 0x%X\n",pdev,ddf->HWP));
	BUGLPR(dbg_midpixi, BUGNFO,
        	("WID parm = %X exposure_list 0x%X, Origin Parm = %d\n",
          	WID_parm,exposure_list,RegOriginParam));




  	/******************************************************************/
  	/* Call "mid_get_correlator()" to get "unique" correlator.        */ 
  	/******************************************************************/

   	mid_get_correlator( pdev, &correlator);  

   	BUGLPR(dbg_midpixi, 4, ("Got correlator  0x%X\n", correlator));  



  	/****************************************************************** 
   	  Most of the processing has been moved to the common subroutine
   	******************************************************************/

	mid_set_window_parms_sub ( ddf, exposure_list,
				   WindowOrgParam, widthParam, heightParam,
				   &woflags, &vis_rect_count,
				   &window_extents, &visible_boxes) ;


  	/****************************************************************** 
   	  Enable I/O operations 
   	******************************************************************/

  	PIO_EXC_ON();             /* enables bus */


   	/****************************************************************** 
   	  Fix the drawing buffer that will be displayed for this window.
   	******************************************************************/

  	fix_color_palette (ddf, WID_parm, color_palette, frame_buffer) ;


  	/************************************************************** 
     	Invoke the macro to do the actual I/O to the adapter.        
   	**************************************************************/

  	MID_SetWindowParametersFIF(WID_parm, woflags, correlator,
                         (&(window_extents)), vis_rect_count, visible_boxes);


    	/************************************************************** 
     	  Update the correlator and flag
     	**************************************************************/

    	BUGLPR(dbg_midpixi, BUGNFO, ("Correlator now %4X,  flag = %X\n", 
		ddf -> mid_last_WID_change_corr, ddf -> WID_change_flag ));

    	ddf -> mid_last_WID_change_corr = correlator ; 
    	ddf -> WID_change_flag = MID_WID_CHANGE_PEND ;

    	BUGLPR(dbg_midpixi, BUGNFO, ("Correlator now %4X,  flag = %X\n", 
		ddf -> mid_last_WID_change_corr, ddf -> WID_change_flag ));



  	/*******************/
  	/* release the bus */
  	/*******************/

  	PIO_EXC_OFF();    /* disables bus */


  	MID_DD_EXIT_TRACE (midpixi, 15, WRITE_WINDOW_PARMS_FIFO, ddf, pdev, 
		WID_parm, ddf->mid_last_WID_change_corr, ddf->WID_change_flag);

  	BUGLPR(dbg_midpixi, BUGNFO,("Leaving write_window_parmsFIF\n\n"));
  	return(0);
}
   


/******************************************************************************

    Name:  mid_set_window_parms_sub 
                                                                                
    Purpose:                                                                    
          This routine performs the processing common to both the PCB and FIFO  
          forms of Set Window Parameters. 
                                                                                
 *****************************************************************************/


long mid_set_window_parms_sub (
				midddf_t	*ddf,  
				gRegion		*pBoxes_in,
				gPoint		Origin,   
				ushort		width,   
				ushort		height,
				ushort		*flags_out,
				ulong		*rect_count_out,
				gBox		*extents_out,
				gBox		*vis_rects_out)
{
    mid_rcx_t	*pmidRCX = ddf->current_context_midRCX ; 

    ushort  	flags; 
    ulong  	vr_count ; 
    gBox  	extents ; 
    gBox        *pTemp_box_out ; 
    gBox        *pCurrent_box_in ; 

    int           i;

    static gBox   full_screen = { 0, 0, 1279, 1023} ; 
    static gBox   null_box =    { 0, 0, 1, 1} ; 


  /****************************************************************** 
              	            TOP OF MODULE
   
           First record entry parameters for debug.
   ******************************************************************/

        MID_DD_ENTRY_TRACE (midpixi, 5, WRITE_WINDOW_PARMS_SUB, ddf, ddf, 
				pBoxes_in, ((Origin.x<<16) | (Origin.y)),
			 	((width<< 16) | height)) ; 


	BUGLPR(dbg_midpixi, 1, ("Entering window_parms_sub \n"));
	BUGLPR(dbg_midpixi,2,("ddf = 0x%X, pBoxes_in = 0x%X\n",ddf,pBoxes_in));
	BUGLPR(dbg_midpixi, 2,("Origin = %X, %X   width = %X,  height = %X \n",
          			Origin.x, Origin.y, width, height ));


    	/*------------------------------------------------------------------*
    	   Init the flags, then add the "geometry changed" flag if necessary.
     	 *------------------------------------------------------------------*/

    	if (pmidRCX->type != RCX_2D)
    	{ 
		flags = SWP_FLAGS_ORIGIN_LL ;
    	}
    	else
    	{
    		flags = SWP_FLAGS_ORIGIN_UL ;
    	}


    	if (pmidRCX->flags.geometry_changed == 1)
    	{ 
  		pmidRCX->flags.geometry_changed = 0 ;
		flags |= SWP_FLAGS_GEOMETRY_CHANGED ;
    	}



    /*----------------------------------------------------------------------*
       THREE CASES: 
         . NULL Region pointer ==> disables WID clipping 
             (note that the WID parameter is unused in this case)
         . no visible regions 
         . normal window (becuase of the way regions are passed to us, 
            one of the subcases of this is no visible regions(case 2)).
     *----------------------------------------------------------------------*/

#define	BOX_IN		pCurrent_box_in
#define	BOX_OUT		pTemp_box_out

        BOX_OUT = vis_rects_out ; 

    	if ( pBoxes_in == NULL )
    	{ 
        	BUGLPR (dbg_midpixi, 2, ("full screen mode\n"));
       
        	flags |= SWP_FLAGS_DISABLE_WID_CLIP ;

		*extents_out  = full_screen ;
		*BOX_OUT = full_screen ;

		vr_count = 1 ;
    	}




	/**********************************************************************

   	   Not a NULL region, there may be visible check.  Let's find out.
   	   
   	   below vr_count will indicate the number of visible boxes. 
   	   
    	*********************************************************************/

    else 
    { 
        vr_count = 0 ;

	if (pBoxes_in->numBoxes > 0)
    	{ 
        	BUGLPR (dbg_midpixi,BUGNFO, ("Possible visible areas\n"));
       
		MID_DEBUG_REGION (SWP, 1, pBoxes_in) ;

	/**********************************************************************

   	   We have the "normal case" of a window with possible visible regions.
   	   
   	   First compute the real window origin 

    	*********************************************************************/

	extents.ul = Origin ;  
        extents.lr.x = extents.ul.x + width - 1;
        extents.lr.y = extents.ul.y + height - 1;

  	BUGLPR(dbg_midpixi, 3, ("numBoxes = %d \n", pBoxes_in->numBoxes ));
  	BUGLPR(dbg_midpixi, 3, ("upper left corner (x, y) = (%X, %X)\n",
        			      extents.ul.x, extents.ul.y) );
  	BUGLPR(dbg_midpixi, 3, ("lower right corner (x, y) = (%X, %X)\n\n",
              				extents.lr.x, extents.lr.y));

#ifdef	SWP_SUB_DEBUG_TRACE
        MID_DD_TRACE_PARMS (midpixi, 5, WRITE_WINDOW_PARMS_SUB, ddf, ddf, 
			 	0x00AA00AA,   
				((extents.ul.x<<16) | (extents.ul.y)),
				((extents.lr.x<<16) | (extents.lr.y)) ) ;
#endif

	BOX_IN =  pBoxes_in-> pBox ;


  	/************************************************************** 
	   Trim the visible rectangles to the window bounds.
   	**************************************************************/


        for (i = 0; i < pBoxes_in->numBoxes ; i++) 
        { 
#ifdef		SWP_SUB_DEBUG_TRACE
        	MID_DD_TRACE_PARMS (midpixi, 5, WRITE_WINDOW_PARMS_SUB, ddf, 
				ddf, 0xAA000000 | i, 
				((BOX_IN->ul.x<<16) | (BOX_IN->ul.y)),
				((BOX_IN->lr.x<<16) | (BOX_IN->lr.y)) );
#endif


  		/********************************************************** 
	    	       First check if the box is outside the window or not.
   	    	**********************************************************/
         	if ( (BOX_IN->ul.x > extents.lr.x) ||
         	     (BOX_IN->ul.y > extents.lr.y) ||
         	     (BOX_IN->lr.x < extents.ul.x) ||
         	     (BOX_IN->lr.y < extents.ul.y) )  
        	{ 
  			BUGLPR(dbg_midpixi, 3, ("Box outside window \n"));
        	} 
		else 
        	{ 
  		    /********************************************************** 
	    	       Trim the Upper left X 
   	    	    **********************************************************/
         	    if (BOX_IN->ul.x >= extents.ul.x)
		    	BOX_OUT->ul.x = BOX_IN->ul.x ; 
         	    else
		    	BOX_OUT->ul.x = extents.ul.x ; 

         	    
  		    /********************************************************** 
	    	       Trim the Upper left Y 
   	    	    **********************************************************/
         	    if (BOX_IN->ul.y >= extents.ul.y)
		    	BOX_OUT->ul.y = BOX_IN->ul.y ; 
         	    else
		    	BOX_OUT->ul.y = extents.ul.y ; 



  		    /********************************************************** 
	    	       Trim the Lower right X (and correct for off by 1)
   	    	    **********************************************************/
         	    if (BOX_IN->lr.x <= extents.lr.x)
		    	BOX_OUT->lr.x = BOX_IN->lr.x -1 ; 
         	    else
		    	BOX_OUT->lr.x = extents.lr.x ; 


  		    /********************************************************** 
	    	       Trim the Lower right Y (and correct for off by 1)
   	    	    **********************************************************/
         	    if (BOX_IN->lr.y <= extents.lr.y)
		    	BOX_OUT->lr.y = BOX_IN->lr.y -1 ; 
         	    else
		    	BOX_OUT->lr.y = extents.lr.y ; 


  		    BUGLPR(dbg_midpixi, 3, ("Box out at 0x%X \n", BOX_OUT));
  		    BUGLPR(dbg_midpixi, 3, ("Box %d: ul = (%X , %X) \n",
				vr_count, BOX_OUT->ul.x, BOX_OUT->ul.y));
  		    BUGLPR(dbg_midpixi, 3, ("        lr = (%X , %X) \n",
						BOX_OUT->lr.x, BOX_OUT->lr.y));
  		    /********************************************************** 
	    	       Now increment the box count and pointer
   	    	    **********************************************************/
        	    vr_count++  ;

#ifdef   	    SWP_SUB_DEBUG_TRACE
        	    MID_DD_TRACE_PARMS (midpixi, 5, WRITE_WINDOW_PARMS_SUB,ddf,
				 ddf, (vr_count<<16) | i, 
				((BOX_OUT->ul.x<<16) | (BOX_OUT->ul.y)),
				((BOX_OUT->lr.x<<16) | (BOX_OUT->lr.y)) );
#endif
        	    BOX_OUT++ ; 

  		    /********************************************************** 
	    	       If we've reached our max boxes, quit   
   	    	    **********************************************************/
        	    if (vr_count >=  MAX_VIS_RECT )
        	    { 
        	    	break ;
        	    } 
        	} 	/* end of else leg */

        	BOX_IN++ ; 
        } 	/* end of for loop */
		    


  	/************************************************************** 
	   Performance improvement: 
	   
	   The blast HW chip runs much faster with WID clipping off. 
	   The adapter normally clips to a window's extents, but if the
	   window has a visible region that consists of a single 
	   rectangle, the adapter clips to the visible region instead.
	   This clipping is independent of WIDs.  So, we can turn off
	   WID clipping if the visible region of the window is
	   rectangular (num boxes = 1).
   	**************************************************************/

        BOX_OUT = vis_rects_out ; 

	if (vr_count == 1) 
	{
		flags |= SWP_FLAGS_DISABLE_WID_CLIP;
	}
    } 	/* end of possible visible boxes to start with */


        /**************************************************************
           Now check if there really were any visible boxes 
           If not, Set window to a null box. 
        ***************************************************************/

	if (vr_count == 0)
    	{ 
        	BUGLPR (dbg_midpixi,BUGNFO, ("no visible areas\n"));
        	/* brkpoint (ddf, pBoxes_in, Origin, width, height, 
						flags, vr_count); */
       
		*extents_out  = null_box ;
		*BOX_OUT = null_box ;

		vr_count = 1 ;
    	}

	else
    	{ 
		*extents_out  = extents ;
    	}
    } 	/* end of "normal window" case */


        /**************************************************************
           Return the flags and box count (the boxes have already been
           filled in -- possibly correctly). 
        ***************************************************************/

	*flags_out = flags ;
	*rect_count_out = vr_count ;


        /**************************************************************
           Return the flags and box count (the boxes have already been
           filled in -- possibly correctly). 
        ***************************************************************/

        /*brkpoint (ddf, flags,*flags_out, vr_count,*rect_count_out,BOX_OUT);*/

        MID_DD_EXIT_TRACE (midpixi, 20, WRITE_WINDOW_PARMS_SUB, ddf, ddf, 
			flags, vr_count, *(ulong *)BOX_OUT);

        BUGLPR(dbg_midpixi, 1,("Leaving write_window_parms_sub \n\n"));
        return(0);
}
   



   
/***************************************************************************** 
    Name: mid_flush_rcm_fifo()                                                 
                                                                             
    Purpose:                                                                
                                                                           
    Inputs:                                                                
          ddf         : a pointer used to access the device dependent data 
                        structure.  This stucture contains the adaptor's  
                        bus address which is required this routine.      
                                                                        
                                                                    
    Calls :                                                        
          HWPDDFSetup : Access the Hardware Bus (Macro)           
                                                                 
                                                                     
    Outputs:  NONE                                                  
                                                                   
 *****************************************************************************/

long mid_flush_rcm_fifo  (midddf_t       *ddf) 
{
  	HWPDDFSetup;		/* declares and inits vars for bus access */


    	MID_RCM_FIFO_FLUSH_code ; 
  
        return (MID_RC_OK) ;
}
