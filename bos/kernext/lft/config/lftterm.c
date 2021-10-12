static char sccsid[] = "@(#)96  1.5  src/bos/kernext/lft/config/lftterm.c, lftdd, bos411, 9438B411a 9/20/94 09:10:24";
/*
 *   COMPONENT_NAME: LFTDD
 *
 *   FUNCTIONS: lft_term
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

/* ---------------------------------------------------------------------- * 
 * Includes								  * 
 * ---------------------------------------------------------------------- */

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

extern long lft_pwrmgr();

extern lft_ptr_t	lft_ptr;

GS_MODULE(lftterm);			/* TRACE registration		*/
BUGVDEF(db_lftterm, 99);

/* --------------------------------------------------------------------	*
 * Name:		lft_term					*
 * Description:		lft shutdown					*
 * Parameters:								*
 *	input		None						*
 * Purpose:								*
 *	This function is invoked out of lft_config if the command is    *
 *	CFG_TERM.  This function undoes what was done in the lft_init() *
 *									*
 * --------------------------------------------------------------------	*/

int
lft_term()
{
    int				rc;
    int				i;
    lft_dds_t			*dds_ptr;
    struct vtmstruc		*vtm_ptr;
    struct phys_displays	*pd = NULL;

    GS_ENTER_TRC(HKWD_GS_LFT, lftterm, 1, lftterm, lft_ptr, 0, 0, 0, 0);
    dds_ptr = lft_ptr->dds_ptr;

    /*
      stop Display Power Management - remove both watchdog timers 
    */
    lft_pwrmgr(REMOVE_LFT_DPM_WD);

    /*
      Now for the displays
    */
    for(i = 0; i < dds_ptr->number_of_displays; i++)
    {
   	if((int)dds_ptr->displays[i].fp != -1)
	{
	    vtm_ptr = dds_ptr->displays[i].vtm_ptr;
	    pd 	    = vtm_ptr->display;

	    /* If Display Power Management is enabled, make sure we turn on 
             * each display, because all displays except the defualt display 
             * have been turned off 
             */
	    if( (dds_ptr->pwr_mgr_time[0] != 0) && (pd->vttpwrphase != NULL))
	    {
	      rc=(*pd->vttpwrphase)(pd,1);
	    }

	    /*
	      Device specific deactivate function
	    */
	    rc = (*pd->vttdact)(vtm_ptr);
	    if(rc)
	    {
		lfterr(NULL,"LFTDD","lftterm", "vttdact", rc, LFT_VTTDACT, UNIQUE_1);
		BUGLPR(db_lftterm, BUGNFO, ("lftterm: vttdact failed for %s\n", dds_ptr->displays[i].devname));
	    }
	    /* 
	      Device specific terminate function
	    */
	    rc = (*pd->vttterm)(vtm_ptr);
	    if(rc)
	    {
		lfterr(NULL,"LFTDD","lftterm", "vttterm", rc, LFT_VTTTERM, UNIQUE_2);

		BUGLPR(db_lftterm, BUGNFO, ("lftterm: vttterm failed for %s\n", dds_ptr->displays[i].devname));
	    }

	    /*
	      Free the vtm structure
	    */
	    xmfree(vtm_ptr, pinned_heap);
	    /*
	      Close the devices
	    */
	    fp_close(dds_ptr->displays[i].fp);
	}	/* end of fp processing */
    }		/* end of for loop 	*/
}
	
