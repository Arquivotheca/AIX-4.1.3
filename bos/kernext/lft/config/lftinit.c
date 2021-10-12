static char sccsid[] = "@(#)95  1.7  src/bos/kernext/lft/config/lftinit.c, lftdd, bos41J, 9517B_all 4/26/95 17:34:42";
/*
 *   COMPONENT_NAME: LFTDD
 *
 *   FUNCTIONS: get_devsw_dataptr
 *		get_disp_dataptr
 *		lft_init
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
/* -------------------------------------------------------------------  *
 * Functions								*
 *									*
 * lft_init 		- Initializes lft				*
 * get_disp_dataptr	- Gets the display device's data pointer	*
 * get_devsw_dataptr	- Gets the devsw table device's data pointer	*
 * -------------------------------------------------------------------- */
 
/* -------------------------------------------------------------------- * 
 * Includes							        * 
 * -------------------------------------------------------------------- */

#include <lft.h>
#include <sys/signal.h>       
#include <sys/sysmacros.h>   
#include <sys/lockl.h>   
#include <sys/syspest.h>    
#include <sys/malloc.h>    
#include <sys/device.h> 
#include <sys/sleep.h>   
#include <graphics/gs_trace.h>
#include <lft_debug.h>

#define CURSOR_SHAPE	5		/* 0 - Invisible cursor		
					   1 - Single underscore
					   2 - Double underscore
					   3 - half blob
					   4 - Mid character double line
					   5 - Full blob		*/

GS_MODULE(lftinit);			/* TRACE registration		*/
BUGVDEF(db_lftinit,99);

/*
	externs
*/

extern int		fsp_enq();
extern long		lft_pwrmgr();
extern lft_ptr_t	lft_ptr;
 
/* --------------------------------------------------------------------	*
 *  Name:		lft_init					*
 *  Description:	Initialize the lft subsystem			*
 *  Parameters:								*
 *	input			None	
 *  Process:								*
 *	This function is invoked when the lft subsystem is configured   *
 *	This function:							*
 *		o Opens all of the display DD and saves the fp's 	*
 *		  in the lft_disp_t					*
 *		o Allocates and initializes the vtmstruc for each	*
 *		  display in the lft_disp_t struct			*
 *		o Initializes the device drivers by invoking the 	*
 *		  device dependent initialization functions		*
 *		o Activates each display via a call to the device 	*
 *		  specific activate function				*
 *									*
 *	Return:								*
 *	    0 = Success							*
 *	    Otherwise an error code is returned				*
 *--------------------------------------------------------------------- */

int
lft_init()
{  
    int	        		i, count, rc;
    lft_dds_t			*dds_ptr;
    struct vtmstruc		*vtm_ptr;
    struct phys_displays	*pd;
    struct ps_s			pr_sp;	/* presentation space 		*/

    GS_ENTER_TRC(HKWD_GS_LFT, lftinit, 1, lftinit, lft_ptr, 0, 0, 0, 0);

    dds_ptr = lft_ptr->dds_ptr;		/* set up the dds_ptr		*/
    
    /*
      Open all of the available displays.  The lft_dds contains an array of 
      lft_disp structures.  The lft_disp structure contains all of the 
      required information about the display
    */

    for(i = 0, count = 0; i < dds_ptr->number_of_displays; i++)
    {
	/*
	  Open the display
	*/
	rc = fp_opendev(dds_ptr->displays[i].devno,DWRITE,NULL,
			NULL, &dds_ptr->displays[i].fp);
	if(rc)
	{
	    lfterr(NULL,"LFTDD","lftinit", "fp_opendev", rc, LFT_FP_OPENDEV, UNIQUE_1);
	    BUGLPR(db_lftinit, BUGNFO, ("lftinit: cannot open display %s. rc = %x\n", dds_ptr->displays[i].devname, rc));
 	    dds_ptr->displays[i].fp = -1;
	    dds_ptr->displays[i].fp_valid = FALSE;
	    continue;
	}

	/* Get the d_dsdptr from the devsw table of the display.  This
	   is a pointer to the phys_displays data structure
        */
	rc = get_disp_dataptr(dds_ptr->displays[i].devno, &pd);
	if(rc)
	{
	    lfterr(NULL,"LFTDD","lftinit", "get_disp_dataptr", rc, LFT_GET_PD, UNIQUE_2);
	    BUGLPR(db_lftinit, BUGNFO, ("lftinit: cannot get d_dsdptr for %s\n", dds_ptr->displays[i].devname));
	    dds_ptr->displays[i].fp = -1;
	    dds_ptr->displays[i].fp_valid = FALSE;
	    continue;
	}

	/*
	  Allocate the vtm struct and initialize it 
	*/
	vtm_ptr = xmalloc(sizeof(struct vtmstruc), 0, pinned_heap);
	if(vtm_ptr == NULL)
	{
	    lfterr(NULL,"LFTDD","lftinit", "xmalloc", 0, LFT_ALLOC_FAIL, UNIQUE_3);
	    BUGLPR(db_lftinit, BUGNFO, ("lftinit: cannot allocate vtm struct\n"));
	    return(ENOMEM);
	}
	bzero(vtm_ptr, sizeof(struct vtmstruc));

	vtm_ptr->display 		= pd;
	vtm_ptr->mparms.cp_mask    	= 0xFF;	/* 8 bit ASCII mask   	*/
	vtm_ptr->mparms.cp_base    	= 0;
	vtm_ptr->mparms.attributes	= ATTRIBUTES_INITIAL;
	vtm_ptr->mparms.cursor.x   	= 1;
	vtm_ptr->mparms.cursor.y   	= 1;
	vtm_ptr->vtid	    	   	= 0;
	vtm_ptr->vtm_mode		= KSR_MODE;
	vtm_ptr->font_index 		= dds_ptr->displays[i].font_index;
	vtm_ptr->number_of_fonts	= dds_ptr->number_of_fonts;
	vtm_ptr->fonts			= lft_ptr->fonts;
	vtm_ptr->fsp_enq		= fsp_enq;

 	/*
	  Initialize the vtmptr in the displays data structure
	*/
	dds_ptr->displays[i].vtm_ptr = vtm_ptr;
	
	/*
	  Initialize the lftanchor and vtm_ptr in the phys_displays structure
	*/
	pd->lftanchor 	= lft_ptr;
	pd->visible_vt	= vtm_ptr;	

	pr_sp.ps_w      = -1;
	pr_sp.ps_h      = -1;
	/*
	  Invoke the device specific initialization function
	*/
	rc = (*pd->vttinit)(vtm_ptr, NULL, &pr_sp);
	if(rc)
	{
	    lfterr(NULL,"LFTDD","lftinit", "vttinit", rc, LFT_VTTINIT, UNIQUE_4);
	    BUGLPR(db_lftinit, BUGNFO, ("lftinit: vttinit failed for %s\n",
				    dds_ptr->displays[i].devname));

	    dds_ptr->displays[i].fp = -1;
	    dds_ptr->displays[i].fp_valid = FALSE;
	/*
 	   Clean up some pointers and then free the vtm structure
	*/
	    dds_ptr->displays[i].vtm_ptr = NULL;
	    pd->lftanchor = NULL;
	    pd->visible_vt = NULL;
	    xmfree(vtm_ptr, pinned_heap);
	    continue;
	}
	/*
	  Invoke the device specific activate function
	*/
	rc = (*pd->vttact)(vtm_ptr);
	if(rc)
	{
	    lfterr(NULL,"LFTDD","lftinit", "vttact", rc, LFT_VTTACT, UNIQUE_5);
	    BUGLPR(db_lftinit, BUGNFO, ("lftinit: vttact failed for %s\n",
				    dds_ptr->displays[i].devname));
	    dds_ptr->displays[i].fp = -1;
	    dds_ptr->displays[i].fp_valid = FALSE;
	/*
 	   Clean up some pointers and then free the vtm structure
	*/
	    dds_ptr->displays[i].vtm_ptr = NULL;
	    pd->lftanchor = NULL;
	    pd->visible_vt = NULL;
	    xmfree(vtm_ptr, pinned_heap);
	    continue;
	}

        /*
           With the arrival of planar graphics, we have to force the console finder to run again when
           users install a new graphics adapter in the system.  This is done by removed the defined
           console by deleting the "syscons" CuAt.  When this happens, the console finder (cfgcon)
           will prompts users to select the output devices for the console

           However by default DPMS is enabled (if pwr_mgr_time[0] is not zero), so users only see
           one messages: "Hit Fx key to select this device as the console".  To get around this we
           need another flag, "enable_dpms".  It is set by the LFT configuration method when it
           detects that the console has not been defined.

         */
                                     /* if Display Power Management is enabled and driver has support for */
        if ( dds_ptr->enable_dpms && (dds_ptr->pwr_mgr_time[0]!=0) && (pd->vttpwrphase != NULL) )
        {
                rc = (*pd->vttpwrphase)(pd,4);    /* turn off the display */
        }

	/*
	  Done with a display
	*/
	count++;			/* We initialized another display */
	dds_ptr->displays[i].fp_valid = TRUE;
    }					/* end of the for loop		  */
    /*
      Make sure we initialized a dispaly
    */
    if(!count)
    {
	lfterr(NULL,"LFTDD","lftinit", NULL, count, LFT_NO_DISP, UNIQUE_6);
	BUGLPR(db_lftinit, BUGNFO, ("lftinit: No displays were initialized\n"));
	return(ENODEV);
    }

    /*	
      Default display initialization
    */
    if(dds_ptr->default_disp_index >= 0)      /* User has init default disp*/
    {
	if(dds_ptr->displays[dds_ptr->default_disp_index].fp == -1)
	{
	    lfterr(NULL,"LFTDD","lftinit", NULL, 0, LFT_DEF_DISP, UNIQUE_7);
	    BUGLPR(db_lftinit, BUGNFO, ("lftinit: Default display did not initialize\n"));
	    /*
		We need to clean up - terminate any display device driver we
		had initialized and free all of the vtm structures and watchdogs.  Fortunately
		lft_term() does exactly this
	    */
	    lft_term();
	    return(ENODEV);
	}
	else
	{
          /*
            Display Power Management - start 2 watchdogs - one to detect keyboard's activity
            and the other to do Display Power Managemnet for the default display.  Note
            other none default display has been turned off.

	    Note we only want to start DPMS if there is a working default display and flag
            to indicate that we need to.
          */

	  if ( (dds_ptr->enable_dpms) && (lft_ptr->dds_ptr->pwr_mgr_time[0] != 0) )
             lft_pwrmgr(START_LFT_DPM_WD);
	}
    }


    GS_EXIT_TRC0(HKWD_GS_LFT, lftinit, 1, lftinit);
    return(SUCCESS);

}
/* --------------------------------------------------------------------	*
 * Name:         get_devsw_dataptr					*
 * Description:  Get the Device Switch Table  Device's Data Pointer	*
 * Parameters:								*
 *	input   dev_t   devno;          Device Number			*
 *	output  char    **dataptr;      Pointer to device data are	*
 *									*
 * Process:								*
 *      A query is performed on the Device Switch Table using the 	*
 *      "devno".  If the device is found then the pointer to the 	*
 *      device's data area is returned through the output parameter	*
 *      "dataptr".  Otherwise, "dataptr" is set to null indicating	*
 *      that the device was not found.					*
 * 									*
 * Returns:								*
 *	deviceptr = device's data area if the device is found		*
 *	Otherwise deviceptr = NULL					*
 *      errno's  - EINVAL (from devswqry) or ENODEV		  	*
 * --------------------------------------------------------------------	*/
int
get_devsw_dataptr( devno, dataptr)
dev_t   devno;
char    **dataptr;

{
    int		rc;		/* Return code				*/
    int		status;		/* Status of device in switch table	*/
    char	*devswdptr;	/* pointer to data area of device	*/

    /* 
      Query the device switch table
    */
    rc = devswqry( devno, (uint *)&status, (caddr_t *)&devswdptr );
    if (rc)
    {
	lfterr(NULL,"LFTDD","lftinit", "devswqry", rc, LFT_DEVSWQRY, UNIQUE_8);
	BUGLPR( db_lftinit, BUGNFO, ("lftinit: Could not query devsw table\n"));
	return(rc);
    }
    /*
      Make sure the device is being used at that location.  If the device
      is not found, then set the pointer to NULL. If found, the dataptr may
      or maynot be NULL.
    */
    if( (status & DSW_UNDEFINED) || (devswdptr == NULL) )
    {
	lfterr(NULL,"LFTDD","lftinit", NULL, 0, LFT_GET_PD, UNIQUE_9);
	BUGLPR( db_lftinit, BUGNFO, ("lftinit: Bad status or dataptr is NULL\n"));
	*dataptr = (char *) 0;		/* Device is not in the table	*/
	rc = ENODEV;
    }
    else
    {
	*dataptr = devswdptr;		/* Found the device		*/
	rc = SUCCESS;
    }
    return(rc);
}

/* --------------------------------------------------------------------	*
 * Name:		get_disp_dataptr				*
 * Description:		Get display data pointer			*
 * Parameters:								*
 *	input		dev_t	devno	Major/minor number		*
 *	output		struct  phys_displays **dispptr Ptr to display	*
 *									*
 * Process:								*
 *	The display device drivers keeps a linked list of data areas	*
 *	for each display that is attached to the system.  This list is	*
 *	kept in the device driver area and is accessible through the	*
 * 	"devsw" table.  This list must be traversed to find the 	*
 *	specific display type that is being asked for. The devno 	*
 * 	identifies each data node					*
 *									*
 * Returns:								*
 * 									*
 *	SUCCESS is returned if successful in finding the display data	*
 *	ptr. A pointer to the display data node that matches the devno 	*
 *	is also returned as an output parameter.			*
 *									*
 * 	If not successful, EINVAL or ENODEV  is returned and the 	*
 * 	pointer is set to NULL						*
 * --------------------------------------------------------------------	*/

int
get_disp_dataptr( devno, display_ptr )
dev_t			devno;		/* Major/minor numbers		*/
struct phys_displays	**display_ptr;	/* ptr to display ptr		*/
{
    int rc;
    struct phys_displays *dispptr;	/* linked list of displays	*/

    /*
      Get the pointer to the first node of the display data node list  
    */
    rc = get_devsw_dataptr( devno, &dispptr);
    if(rc)			/* Trouble!			*/
    {
	lfterr(NULL,"LFTDD","lftinit", "get_devsw_dataptr", rc, LFT_GET_PD, UNIQUE_10);
	BUGLPR(db_lftinit, BUGNFO, ("lftinit: get_devsw_dataptr failed\n"));
	return (rc);
    }

    for( rc = ENODEV; dispptr != NULL; dispptr = dispptr->next )
    {
	if( devno == dispptr->devno )
	{
	    *display_ptr = dispptr;
	    rc = SUCCESS;
	    break;
	}
    }	/* end of for */
    return (rc);
}
