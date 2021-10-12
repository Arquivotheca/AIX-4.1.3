static char sccsid[] = "@(#)05  1.3.1.5  src/bos/kernext/disp/ped/ddf/midddf.c, peddd, bos41J, 9511A_all 3/13/95 17:20:07";
/*
 * COMPONENT_NAME: PEDDD
 *
 * FUNCTIONS: mid_ddf
 *	
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/xmem.h>
#include <sys/rcm_win.h>
#include <sys/rcm.h>
#include <rcm_mac.h>

#include "midddf.h"
#include "midRC.h"		
#include "mid_dd_trace.h"		

BUGXDEF(dbg_midddf);

/*----------------------------------------------------------------------------
Define interrupt disable/enable value.  This value defined in middef.c   
----------------------------------------------------------------------------*/

#define PDEV_PRIORITY (pdev->devHead.display->interrupt_data.intr.priority-1)

/*----------------------------------------------------------------------------
This section was added to support the aixgsc call for X to access the DWA WID.
It needs to be added to mid.h on the next release.
----------------------------------------------------------------------------*/


#define MID_GETWID         337 

typedef struct mid_getwid {
    rcmWG 	*pWG_handle;		/* Window Geometry handle */
    int    	wid;                   	/* current wid */
    int    	state;                 	/* current wid state */
} mid_getwid_t;



/*----------------------------------------------------------------------------

				M I D _ D D F

Device Dependent Function "driver" routine (mid_ddf)


DESCRIPTION

This function performs device specific functions for users of the Pedernales 
and Lega device drivers.  "mid_ddf()"  interfaces to aixgsc() to service the 
following functions:

	- get character position
	- get condition
	- get text font index
	- get color
	- get projection matrix
	- get modeling matrix
	- frame buffer swap
	- end render

A DDF function call is generated based on the function requested (cmd).  This
eventually leads to a hardware interrupt and a call to the corresponding
interrupt handler which will read the data off the adapter if data is  
provided.  This data is then returned to the caller.


INPUT PARAMETERS:

pdev		Pointer to device specific structure.  (in inc/sys/rcm.h)
cmd		Specific device-dependent function to perform.
ddi		Pointer to structure which contains arguments to go to the
		device specific function.
ddi_len		Length of argument structure.


RETURN VALUES:

userarg		Pointer to user structure where output is to be placed.



PSEUDOCODE:

----------------------------------------------------------------------------*/



long mid_ddf(pdev, cmd, ddi, ddi_len, userarg)
gscDev *pdev;				/* device structure pointer	     */
int cmd;				/* specific dev-dep-fun to do	     */
void *ddi;				/* ptr to struct with arguments      */
int ddi_len;				/* len of structure		     */
void *userarg;				/* pointer to user segment	     */

{
  
	/*---------------------------------------------------------------------
	Set up the  "ddf"  pointer to point to the  "free_area"  within
	the physical displays structure (phys_displays).              
	                                                                
	Support of multiple displays requires that each display keep    
	its own copy of the  "phys_displays"  structure.  The free area
	pointer within this structure will point to the  "midddf_t"   
	structure for each display.  This structure allows DDF functions
	and their corresponding interrupt handlers to communicate.     
	---------------------------------------------------------------------*/

	midddf_t	*ddf = 
			(midddf_t *) pdev->devHead.display->free_area;

	unsigned short	correlator;
	int		rc;
	short		i;
	mid_getwid_t    *wid_buf;
	rcmWG		*pWG = NULL;
	midWG_t		*midWG;

						
	BUGLPR(dbg_midddf, 1, ("Entering mid_ddf."));
	BUGLPR(dbg_midddf, 4, ("mid_ddf arguments follow: "));
	BUGLPR(dbg_midddf, 4, 
		("pdev=0x%x, cmd=%d, ddi_len=%d userarg=0x%x", 
		pdev, cmd, ddi_len, (long) userarg));


	if (  cmd != MID_TRANSFER_WID_TO_DD 
	   && cmd != MID_RESET_TO_DEFAULT_WIDS )
	{ 

	/*---------------------------------------------------------------------
	Get "unique" correlator.
	---------------------------------------------------------------------*/

	mid_get_correlator( pdev, &correlator);
	BUGLPR(dbg_midddf, 4, 
		("Got correlator. correlator=0x%x\n", correlator));



	/*---------------------------------------------------------------------
	AND out first nibble of correlator so that identifier can be OR'ed in.
	---------------------------------------------------------------------*/

	correlator &= 0x0FFF;

	} 


	/*---------------------------------------------------------------------
	Switch on "cmd" for specific function call to make, passing in  
	the "pdev" pointer and a function-specific structure pointer.   
	This function-specific structure is used to receive output fro
	the DDF call, making it available to the original  "aixgsc()"
	caller.                                                     
	                                                                
	Before making the DDF function call, call "mid_get_correlator()"
	which calculates and returns a unique correlator.  This        
	correlator will be used to associate this particular call with
	the results which end up in the status control block on the  
	adapter.  The correlator is a 16-bit unsigned integer which is  
	simply incremented by one to get the next "unique" value. 
	---------------------------------------------------------------------*/

	switch( cmd )
	{		/*.............swapbuffers..........................*/

		case 	MID_SWAPBUFFERS:		/* mask=0x3000 */

			BUGLPR(dbg_midddf, 3, 
				("MID_SWAPBUFFERS 0x%x \n", cmd));

			correlator |= SWAPBUFFERS_MASK;  /* add 4-bit ID */
			rc = mid_swapbuffers(pdev, 
				((mid_swapbuffers_t *) userarg), correlator); 

			break;



			/*...........get current character position..........*/

		case 	MID_GETCPOS:			/* mask=0x2000 */

			correlator |= GETCPOS_MASK;  /* add 4-bit identifier */

			BUGLPR(dbg_midddf, 3, 
				("MID_GETCPOS 0x%x correlator=0x%x\n", 
				cmd, correlator));

			rc = mid_getcpos(pdev, ((mid_getcpos_t *) userarg), 
				correlator); 
			break;


			/*.................get current color.................*/

		case 	MID_GETCOLOR:			/* mask=0x1000 */

			BUGLPR(dbg_midddf, 3, 
				("MID_GETCOLOR 0x%x \n", cmd));

			correlator |= GETCOLOR_MASK; /* add 4-bit identifier */
			rc = mid_getcolor(pdev, ((mid_getcolor_t *) userarg), 
				correlator);

			break;


			/*..............get current 3DM1 condition...........*/

		case 	MID_GETCONDITION:		/* mask=0x4000 */

			BUGLPR(dbg_midddf, 3, 
				("MID_GETCONDITION 0x%x \n", cmd));

			correlator |= GETCONDITION_MASK; /* add 4-bit ID */
			rc = mid_getcondition(pdev, 
				((mid_getcondition_t *) userarg), correlator);

			break;


			/*.........get current 3DM1 text font index.........*/

		case 	MID_GETTEXTFONTINDEX:		/* mask=0x7000 */

			BUGLPR(dbg_midddf, 3, 
				("MID_GETTEXTFONTINDEX 0x%x \n", cmd));

			correlator |= GETTEXTFONTINDEX_MASK; /* add 4-bit ID */
			rc = mid_gettextfontindex(pdev, 
				((mid_gettextfontindex_t *)userarg),correlator);
			break;




			/*..... Add WID to DWA list (courtesy of X)  .......*/

		case 	MID_TRANSFER_WID_TO_DD:		

			BUGLPR(dbg_midddf, 3, ("MID_TRANSFER_WID_TO_DD \n"));

			rc = mid_add_WID (ddf, (mid_transfer_WID_t *)userarg);

			if (rc != MID_RC_OK)	return (rc) ;
			break;

			


			/*.. Request (by X) to reset to default DWA WIDS ..*/

		case 	 MID_RESET_TO_DEFAULT_WIDS:		

			BUGLPR(dbg_midddf, 3, ("MID_RESET_TO_DEFAULT_WIDS\n"));

			if (ddf->num_DWA_contexts != 0)	  return (MID_ERROR) ;

                	mid_wid_init (&(ddf->mid_wid_data)) ;

                	ddf->num_DWA_WIDS = MID_NUM_WIDS_START_DWA ;
                	ddf->mid_guarded_WID_count = 0 ;

			break;
			

			/*..............Do End Render for 3dm1...............*/

		case 	MID_END_RENDER:			/* mask=0x8000 */

			BUGLPR(dbg_midddf, 3, ("MID_END_RENDER\n"));

			correlator |= ENDRENDER_MASK; /* add 4-bit ID */
			rc = mid_endrender(pdev, correlator);

			break;

			/*..............Do Get WID for X.....................*/

                case    MID_GETWID:                    

                        BUGLPR(dbg_midddf, 3,
                                ("MID_GETWID 0x%x correlator=0x%x\n",
                                cmd, correlator));

                        wid_buf = (mid_getwid_t *) ddi;
			FIND_WG (pdev, wid_buf->pWG_handle, pWG);
			if (! pWG) 
			{ rc = -1;
			  break;
			}	

			midWG = ((struct _midWG *) (pWG -> pPriv)) ;
			
                        wid_buf = (mid_getwid_t *) userarg;
                        wid_buf->wid = midWG->wid;
			wid_buf->state = ddf->mid_wid_data.mid_wid_entry[wid_buf->wid].state;

                        break;

			/*...............invalid "cmd" value................*/

		default:
			rc = -1;
			BUGLPR(dbg_midddf, 0, ("Invalid DDF command = 0x%x\n", 
				cmd));
			break;
	}


	BUGLPR(dbg_midddf, 1, ("Leaving  mid_ddf."));

	return(0);      /*  end of mid_ddf()  */
}
