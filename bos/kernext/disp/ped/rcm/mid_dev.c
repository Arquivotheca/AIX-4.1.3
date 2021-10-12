static char sccsid[] = "@(#)55  1.8  src/bos/kernext/disp/ped/rcm/mid_dev.c, peddd, bos411, 9428A410j 4/21/94 10:48:45";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: mid_dev_init
 *		mid_dev_term
 *		mid_init_rcx
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


/***************************************************************************** 
       Includes:  
 *****************************************************************************/

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/dma.h>
#include <sys/xmem.h>
#include <sys/syspest.h>
#include <sys/rcm_win.h>
#include <sys/aixgsc.h>
#include <sys/rcm.h>
#include "mid.h"
#include "midhwa.h"
#include "midddf.h"
#include "midrcx.h"             /* BUS access definitions in here       */
#include "midRC.h"             

#include <sys/malloc.h>
#include <sys/pin.h>
#include "mid_dd_trace.h"

#include <hw_regs_u.h>      /* bus address page boundary definitions */
#include <hw_regs_k.h>      /* bus address page boundary definitions */

MID_MODULE (middev);

/***************************************************************************** 
       Externals: 
 *****************************************************************************/

extern mid_init_rcx (struct phys_displays *);

extern long mid_dev_init ( gscDevPtr	pdev );

extern long mid_dev_term ( gscDevPtr	pdev );



/******************************************************************************

                             Start of Prologue

   Function Name:    mid_init_rcx

   Descriptive Name:   Initialize protection for the different domains and
                       authorization masks associated with those domains.

 *--------------------------------------------------------------------------*

   Function:   There are two domains which must be protected from each other.
               These domains are:

                  DOMAIN 0:  Used for DWA (fifo) page.

                  DOMAIN 1:  Used for both the SERVER (pcb) page and the
                             IND (indirect addressing) page.


   The following shows how the bus address space is arranged on this adapter:


        + + + + + + + + + + + + + + + + + + + +  HWP + 0x4000 bytes
        +                                     +
        +     Indirect Addressing Page        +
        +                                     +
        +         ( MID_IND_PAGE )            +
        +                                     +
        + + + + + + + + + + + + + + + + + + + +  HWP + 0x3000 bytes
        +                                     +
        +  Direct Window Addressing Page      +
        +                                     +
        +        ( MID_DWA_PAGE )             +
        +                                     +
        + + + + + + + + + + + + + + + + + + + +  HWP + 0x2000 bytes
        +                                     +
        +          X-Server Page              +
        +                                     +
        +       ( MID_SERVER_PAGE )           +
        +                                     +
        + + + + + + + + + + + + + + + + + + + +  HWP + 0x1000 bytes
        +                                     +
        +           Kernel Page               +
        +                                     +
        +        ( MID_KERN_PAGE )            +
        +                                     +
        + + + + + + + + + + + + + + + + + + + +  HWP + 0x0 bytes


 *--------------------------------------------------------------------------*

    Restrictions:
                  none
 
    Dependencies:
                  none
 
 
 *--------------------------------------------------------------------------* 
  
    Linkage:

 *--------------------------------------------------------------------------* 
  
    INPUT:   Physical Display structure of the adapter.
              
          
          
    OUTPUT:  none 
          
    RETURN CODES:  none         
          
          
                                 End of Prologue                                
 ******************************************************************************/


mid_init_rcx(pd)
struct phys_displays *pd;
{
        struct midddf *ddf = pd->free_area;

        HWPDDFSetup;
        BUGLPR(dbg_middev, 4, ("Hardware base address = 0x%x\n",
                HWP ));



        ASSERT(pd);
        ASSERT(pd->free_area);
        BUGLPR(dbg_middev, 1, ("Entering mid_init_rcx pd=0x%x\n", pd));



        /*-------------------------------------------------------------------
        There are 2 domains.  One for the fifo page of bus memory address
        space, and one combined domain for the pcb page and indirect
        memory page.
        -------------------------------------------------------------------*/

        pd->num_domains = 2;


        /*-------------------------------------------------------------------
        Is this a DWA device?  Yes it is.
                0 == NO
                1 == YES
        -------------------------------------------------------------------*/

        pd->dwa_device  = 1;


        /*-------------------------------------------------------------------
	Indicate no Bus I/O
        -------------------------------------------------------------------*/

	pd->io_range = 0xffff0000;

        /*-------------------------------------------------------------------
	
	We no longer arbitrarily set the authority mask for each of the
	domains.  The authority masks are assigned from the device indep-
	endent layer.  The TCW's set up by the d_protect call are done
	in the mid_dev_init function.

        -------------------------------------------------------------------*/

        BUGLPR(dbg_middev, 1, ("Leaving mid_init_rcx\n"));

        return 0;


}




/****************************************************************************

	Function Name: mid_dev_init

	Descriptive Name: Device dependent device initialization

 * ------------------------------------------------------------------------*

	Function:

	This routine does the set up of the TCW's via the d_protect call.
	

	Dependencies:

	The bus authorization mask in the display structure is set up in
	the device independent dev_init function.


***************************************************************************/

mid_dev_init(pdev)
gscDevPtr	pdev;

{

	struct phys_displays *pd = pdev->devHead.display;
	struct midddf *ddf = pd->free_area;


        HWPDDFSetup;
        BUGLPR(dbg_middev, 4, ("Hardware base address = 0x%x\n",
                HWP ));



        ASSERT(pd);
        ASSERT(pd->free_area);

        /*-------------------------------------------------------------------
                                PROTECT  DOMAIN 0
        -------------------------------------------------------------------*/

        BUGLPR(dbg_middev, 4, ("Protecting Domain0\n"));
        BUGLPR(dbg_middev, 1,
           ("Authorization mask for Domain0 %d\n",pd->busmemr[0].auth_mask));


        d_protect ( NULL,
                  ( 0xFFFFFFF & MID_DWA_PAGE ),
                    4 * 1024,
                    pd->busmemr[0].auth_mask,
                    BUS_ID );




        /*-------------------------------------------------------------------
                                PROTECT  DOMAIN 1
        -------------------------------------------------------------------*/

        BUGLPR(dbg_middev, 4, ("Protecting Domain1\n"));
        BUGLPR(dbg_middev, 1,
           ("Authorization mask for Domain1 %d\n",pd->busmemr[1].auth_mask));

	d_protect ( NULL,
		  ( 0xFFFFFFF & MID_KERN_PAGE ),
		    4*1024,
		    pd->busmemr[1].auth_mask,
		    BUS_ID );

        d_protect ( NULL,
                  ( 0xFFFFFFF & MID_SERVER_PAGE ),
                    4*1024,
                    pd->busmemr[1].auth_mask,
                    BUS_ID );

        d_protect ( NULL,
                  ( 0xFFFFFFF & MID_IND_PAGE ),
                    4*1024,
                    pd->busmemr[1].auth_mask,
                    BUS_ID );

	/*
	 *  Set flag indicating that device needs to have adapter
	 *  state locked (or guarded) before dma processing begins.
	 *  This is because this device makes a callback to guard
	 *  the domain during dma AFTER the dma locks are set.  This
	 *  is done to prevent deadlock.
	 */
	pdev->devHead.flags |= DEV_DMA_LOCK_ADAPTER;

        BUGLPR(dbg_middev, 1, ("Leaving mid_dev_init\n"));

        return 0;

}

/****************************************************************************

	Function Name: mid_dev_term

	Descriptive Name: Device dependent device termination cleanup

 * ------------------------------------------------------------------------*

	Function:

	This function is currently not being used.  The intent is to use
	this when the device independent layer does dev_term.  This happens
	when the last graphics process is killed on the device.

***************************************************************************/

mid_dev_term(pdev)
gscDevPtr	pdev;
{
        BUGLPR(dbg_middev, 4, ("mid_dev_term !!! \n"));
	return (0);
}
