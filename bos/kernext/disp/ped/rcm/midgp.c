static char sccsid[] = "@(#)71	1.8.1.6  src/bos/kernext/disp/ped/rcm/midgp.c, peddd, bos411, 9428A410j 5/12/94 09:40:17";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: mid_check_dev
 *		mid_make_gp
 *		mid_unmake_gp
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*
   OVERVIEW

   The RCM does many things, but its' main purpose is to allow any number of
   graphics processes to use the graphics hardware without knowing about,
   or interfering with, each other.
   This virtualization of the graphics hardware is accomplished in a manner
   similar to that used to allow system processes to use the system CPU without
   knowing about and interfering with each other: each rendering process has
   an associated state, and, no process is allowed to monopolize the hardware
   resources.
   The state in computer graphics is called a context and the graphics time
   slicing is accomplished with assistance from system process management,
   and essentially involves causing the hardware to move from one rendering
   state ( or context ) to another. This movement is called context switching.

   Context switching can result from three different causes: 

   1) A graphics process attempts to write to the virtual memory page area
   that the graphics hardware is associated with. This is called a graphics
   page fault.
   
   2) A graphics process' GRAPHICS time slice expires with some other process
   waiting to use the hardware. An example when this might occur is when a
   process has sent all of its rendering instructions (to a FIFO structure 
   located on the graphics hardware) and is switched out before all of them 
   are read. The process will never cause another graphics page fault, yet 
   its' instructions must be processed (its' FIFO drained).

   3) The graphics hardware sends a FIFO stalled interrupt. This means that 
   the current rendering process has sent a request to the adapter that will 
   take some time, and no further processing on the hardware is possible 
   until this request is fulfilled. Rather than have the graphics adapter 
   sit stalled, another graphics process is allowed to run.


   A scenario of how these calls are used: 

   1) a new window is registered as a graphics process with a make_gp(). 
   This also gives the process an address to write to the hardware with. 

   2) A new window is created. The window is mapped onto the display. This 
   sets it's clipping geometry so a call to create_win_geom is made. 

   3) The new window now needs its' desired pixel interpretation (RGB, 
   8 bit color index, etc) so a call to create_win_attr is made. 

   4) The window now needs this data to be put into a form that can be 
   read by the adapter ucode. The adapter readable data is called the 
   context. The context is created with a create_rcx call. 

   5) The data to write to the window is now created, but it is not 
   yet related together, that is, the context, the window geometry, and 
   window attributes are separate pieces of data. They are bound together 
   with a bind_window call. If a process has more than one window geometry, 	
   (X has many), the process moves between these with bind_window calls. 

   6)  If a process has more than one context (gl apps usually have
   several), the process moves between these with a set_context call.

*/

#include <sys/types.h>
#include <sys/syspest.h>
#include <sys/xmem.h>
#include <sys/rcm_win.h>
#include <sys/aixgsc.h>
#include <sys/rcm.h>
#include "mid.h"
#include "midhwa.h"
#include "midddf.h"

#include <hw_regs_u.h>      /* bus address page boundary definitions */
#include <hw_regs_k.h>      /* bus address page boundary definitions */

#include "mid_rcm_mac.h"
#include "mid_dd_trace.h"

MID_MODULE(midgp) ;
#define    dbg_midrcx dbg_midgp

extern long mid_make_gp(struct _gscDev *,rcmProc *,struct mid_make_gp *,int);
extern long mid_unmake_gp(struct _gscDev *,rcmProc *);

extern long mid_check_dev( struct _gscDev *, caddr_t, pid_t );


long mid_make_gp(gdp, pproc, datap, length)
struct _gscDev *gdp;		/* graphics device structure */
rcmProc *pproc;			/* process pointer */
struct mid_make_gp *datap;	/* data buffer from app requesting data */
int length;			/* length of data buffer */



/***************************************************************************/
/*
/* Allow a process to write to a graphics adapter. The process becomes a 
/* graphics process. Give the process the address of the adapter and other 
/* hardware information necessary to write to the adapter. The data is
/* passed through the mid_make_gp struct passed in by the process making
/* this call.
/*
/*                                                                            
/* pseudo code:                                                               
/*	put the hardware address into the address field of the make_gp struct 
/*	put the hardware configuration into its' field                        
/*	put the minimum ucode version into its' field                         
/*	put the screen width and height into their fields                     
/*
/*	Assume that the structures passed in exist.
*/
/***************************************************************************/

{

	struct phys_displays *pdp = gdp->devHead.display;
	midddf_t *ddf = (midddf_t *)pdp->free_area;
	int seq;

	/*----------------------------*/
	/* gain access to the adapter */
	/*----------------------------*/

	HWPDDFSetup;


    	MID_DD_ENTRY_TRACE (midgp, 2, MID_MAKE_GP, ddf, gdp, 
				gdp, pproc, ddf->num_graphics_processes) ; 

	ASSERT(gdp);
	ASSERT(pdp);
	ASSERT(pdp->free_area);



	BUGLPR(dbg_midrcx, 2,
		("Entering mid_make_gp pdp 0x%x datap 0x%x length %d\n",
		pdp, datap, length));

	if(length < 20) {
		BUGLPR(dbg_midrcx, 2,
			("Make GP failed: structure is too small!! size %d\n",
			length));
		return(-1);
	}

	/* ------------------------------------------------------
	   Set up the graphics device structure pointer(s) 
	 * ------------------------------------------------------ */
	ddf->pdev = gdp ; 


	/* ------------------------------------------------------
	   Set up the hwconfig, screen height and width, etc.
	 * ------------------------------------------------------ */
		datap->bus_addr = (char *)((int)ddf->HWP & 0xfffffff);

		if ( datap->hwconfig & MID_CONFIG )
		{
			datap->hwconfig = ddf->hwconfig;
			ddf->hwconfig |= MID_CONFIG;
			BUGLPR(dbg_midrcx, 1,
				("Make GP for diagnostics\n"));
		}
		else
			datap->hwconfig = ddf->hwconfig;

		datap->ucode_version = 1;

		datap->screen_width_mm = pdp->display_info.screen_width_mm;

		datap->screen_height_mm = pdp->display_info.screen_height_mm;


	if (ddf->num_graphics_processes == 0)
	{
		mid_wid_init (&(ddf->mid_wid_data)) ;

		ddf->num_DWA_WIDS = MID_NUM_WIDS_START_DWA ;
		ddf->mid_guarded_WID_count = 0 ;
		ddf->dom_delayed_Assoc_updates_count = 0;
	}
	
	ddf->num_graphics_processes++ ; 

    	MID_DD_EXIT_TRACE (midgp, 1, MID_MAKE_GP, ddf, gdp, 
				ddf, pproc, ddf->num_graphics_processes) ; 

	return 0;
}



long mid_unmake_gp(gdp, pproc)
struct _gscDev *gdp;
rcmProc *pproc;

/*
/* Unmake gp is for freeing structures that are malloced during make_gp.
/* Since make_gp on the mid adapter does no malloc'ing, unmake_gp is a noop.
*/

{
	struct phys_displays *pdp = gdp->devHead.display;
	midddf_t *ddf = (midddf_t *)pdp->free_area;


    	MID_DD_ENTRY_TRACE (midgp, 2, MID_UNMAKE_GP, ddf, gdp, 
				gdp, pproc, ddf->num_graphics_processes) ; 

	if ( ddf->hwconfig & MID_CONFIG )
	{
		BUGLPR(dbg_midrcx, 1,
			("UnMake GP for diagnostics\n"));
		ddf->hwconfig &= ~MID_CONFIG;
	}

	ddf->num_graphics_processes-- ; 

	if (ddf->num_graphics_processes == 0)
	{ 
		 MID_ASSERT ( (ddf->num_DWA_contexts == 0), MID_UNMAKE_GP, ddf, 
				ddf->num_DWA_contexts, 0, 0, 0) ;
	} 

    	MID_DD_EXIT_TRACE (midgp, 1, MID_UNMAKE_GP, ddf, gdp, 
				gdp, pproc, ddf->num_graphics_processes) ; 
	return 0;
}




/****************************************************************************
                        m i d _ c h e c k _ d e v

 *-------------------------------------------------------------------------* 

    FUNCTION:
	 Convert a passed bus address to a domain number. 

 *-------------------------------------------------------------------------* 

    NOTES:   
          Ped has the following Domains and bus layouts: 

          relative addr    |  NAME            |  Function(s)       |  domain 
          -----------------|------------------|--------------------|-------
          0x0000 - 0x1000  |  MID_KERN_PAGE   |  misc regs, status |  1 **   
          0x1000 - 0x2000  |  MID_SERVER_PAGE |  PCB               |  1
          0x2000 - 0x3000  |  MID_DWA_PAGE    |  FIFO              |  0
          0x3000 - 0x4000  |  MID_IND_PAGE    |  indirect address  |  1 

   ** The macros accessing the PCB must determine whether the PCB
       the PCB is available before writing to it.  This determination is done
       by reading a bit in BIM status (at relative address 0x0084). 
       Therefore, the kernel page must be considered part of the PCB page.


****************************************************************************/




long mid_check_dev( gdp, bus_addr, pid )
struct  _gscDev *gdp;
caddr_t bus_addr;
pid_t   pid;

{

        midddf_t  *ddf = ( midddf_t * ) gdp->devHead.display->free_area;

        HWPDDFSetup;    /* Get base address of the hardware                */

 	/*-----------------------------------------------------------------* 
	    Trace / echo the input parms
 	 *-----------------------------------------------------------------*/

        MID_DD_ENTRY_TRACE (midgp, 2, CHECK_DEV, ddf, gdp, bus_addr, pid, gdp) ;


        BUGLPR(dbg_midrcx, 1,
                ("Enter mid_check_dev() gdp=0x%x bus_addr=0x%x pid=0x%x \n",
                  gdp, bus_addr, pid ) );
        BUGLPR(dbg_midrcx, 4,
                ("DWA_PAGE=0x%x SERVER_PAGE =0x%x IND_PAGE=0x%x K_PAGE=0x%x\n",
                MID_DWA_PAGE, MID_SERVER_PAGE, MID_IND_PAGE, MID_KERN_PAGE ) );

	/* First check to see if this bus address is for our adapter.
	/* Note that MID_***_PAGE addresses have ddf->HWP built into them.
	*/
        if ( ( bus_addr < ( ( MID_BASE_PTR ) & 0x0FFFFFFF ) )  ||
             ( bus_addr >  ( ( MID_IND_PAGE + 0x1000 ) & 0x0FFFFFFF ) ) )
        {
		return(-1);
	}

	/* The bus address is ours. Now we find the domain. */
        if ( ( bus_addr >= ( ( MID_DWA_PAGE ) & 0x0FFFFFFF ) )  &&
             ( bus_addr <  ( ( MID_IND_PAGE ) & 0x0FFFFFFF ) ) )
        {
                BUGLPR(dbg_midrcx, 4, ("MID_DWA_PAGE=0x%x MID_IND_PAGE=0x%x\n",
                        		MID_DWA_PAGE, MID_IND_PAGE ) );

                BUGLPR(dbg_midrcx, 2, ("Leaving  mid_check_dev() domain 0\n") );

        	MID_DD_EXIT_TRACE (midgp, 1, CHECK_DEV, ddf, gdp, 
					bus_addr, pid, 0) ;
                return( 0 );
        }




        /*-------------------------------------------------------------------
          A bus address is in Domain 1 if it is on the MID_SERVER_PAGE
          or on the MID_IND_PAGE.

          This includes any writes to the PCB or indirect memory pages.

          Note that the DWA Page is located between the Server page and
          the Indirect Addressing page.  As long as we check for the DWA
          page first, we can be confident that this test for Domain 0 is OK.
        -------------------------------------------------------------------*/


        if ( ( bus_addr >= ( ( MID_SERVER_PAGE ) & 0x0FFFFFFF ) )  &&
             ( bus_addr <  ( ( MID_IND_PAGE ) + 0x1000 ) & 0x0FFFFFFF ) )
        {
                BUGLPR(dbg_midrcx, 4,
                        ("MID_SERVER_PAGE=0x%x MID_IND_PAGE=0x%x\n",
                        MID_SERVER_PAGE, MID_IND_PAGE ) );

                BUGLPR(dbg_midrcx, 2, ("Leaving  mid_check_dev() domain 1\n") );

        	MID_DD_EXIT_TRACE (midgp, 1, CHECK_DEV, ddf, gdp, 
					bus_addr, pid, 1) ;
                return( 1 );
        }



        /*-------------------------------------------------------------------
          We should not be called for an address on the kernel page.

	  However, any macro that writes to the PCB checks if the PCB
	  is available (and loops).  This check does a read from
	  BIM status at relative address 0x0084, which is on the kernel page.
        -------------------------------------------------------------------*/

        if ( ( bus_addr >= ( (MID_KERN_PAGE ) & 0x0FFFFFFF  ) ) &&
             ( bus_addr <  ( (MID_SERVER_PAGE)  &0x0FFFFFFF ) ) )
        {
#ifdef  PRE_KLUDGE
                BUGLPR(dbg_midrcx, 1,
                ("INVALID bus address. On kernel page.   return -1\n") );
                BUGLPR(dbg_midrcx, 4,
                        ("HWP=0x%x MID_IND_PAGE=0x%x\n",
                        HWP, MID_IND_PAGE ) );
                return( -1 );
#endif
                BUGLPR(dbg_midrcx, 1,
                ("KLUDGE:  Assigning kernel page to domain 1 KERN_PAGE=0x%x\n",
                          MID_KERN_PAGE ) );
                BUGLPR(dbg_midrcx, 2,
                        ("Leaving  mid_check_dev() domain 1\n") );

        	MID_DD_EXIT_TRACE (midgp, 1, CHECK_DEV, ddf, gdp, 
					bus_addr, pid, 1) ;
                return( 1 );
        }


        /*-------------------------------------------------------------------
          A bus address is invalid if it is in any other range.
        -------------------------------------------------------------------*/

        BUGLPR(dbg_midrcx, 0,
                ("INVALID BUS ADDRESS.  bus_addr = 0x%x\n", bus_addr ));

        BUGLPR(dbg_midrcx, 1,
                ("Leaving  mid_check_dev() INVALID bus address. return -2\n") );

        MID_DD_EXIT_TRACE (midgp, 1, CHECK_DEV, ddf, gdp, bus_addr, pid, 0xEE) ;
        return(-2);

}

