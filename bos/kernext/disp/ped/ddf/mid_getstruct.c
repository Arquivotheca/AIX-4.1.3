static char sccsid[] = "@(#)43  1.2  src/bos/kernext/disp/ped/ddf/mid_getstruct.c, peddd, bos411, 9428A410j 4/28/94 13:45:38";
/*
 * COMPONENT_NAME: PEDDD
 *
 * FUNCTIONS: get_free_pending_req_struct
 *	
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/intr.h>
#include <sys/malloc.h>
#include "midddf.h"
#include "mid_dd_trace.h"

MID_MODULE ( midgetfreestr );   /* defines trace variable                    */

#define PDEV_PRIORITY pdev->devHead.display->interrupt_data.intr.priority

/*----------------------------------------------------------------------------

FUNCTION NAME:  get_free_pending_req_struct

FUNCTION:  This routine returns a pointer to a free data structure in the
	   specified array of pending DDF requests.

RESTRICTIONS:  None

DEPENDENCIES:  Interrupts must be disabled when this routine is invoked.

INPUT:   ddf               - pointer to device dependent physical device
			     structure
	 pending_req_array - array of pending DDF requests to search
	 wd_func           - address of watchdog timer completion handler 
			     routine for the DDF request type currently
			     being processed
	 wd_timeout	   - watchdog timer timeout value for the DDF 
			     request type currently being processed
OUTPUT:  None

RETURNS:  - A pointer is returned to a free data structure in the array
	  - NULL is returned if there are no free data structures in the
	    array and none can be allocated due to lack of memory

-----------------------------------------------------------------------------*/

ddf_data_t *get_free_pending_req_struct(pdev, pending_req_array, wd_func,
					wd_timeout, old_interrupt_priority)
	gscDev  	*pdev;
	ddf_data_t	*pending_req_array;
	void		(*wd_func)();
	ulong		wd_timeout;
	int		*old_interrupt_priority;
{
	midddf_t        *ddf = (midddf_t *) pdev->devHead.display->free_area;
        ddf_data_t	*more_space_ptr;
        ddf_data_t	*current_ptr;
        short           i;


        MID_DD_ENTRY_TRACE(midgetfreestr, 1, GET_FREE_PENDING_REQ_STRUCT, ddf,
                	   0, pending_req_array, 0, 0);
	BUGLPR(dbg_midgetfreestr, 1, 
	       ("Entering get_free_pending_req_struct\n"));
        BUGLPR(dbg_midgetfreestr, 3, ("ddf = 0x%x\n", ddf));
        BUGLPR(dbg_midgetfreestr, 3, ("pending_req_array = 0x%x\n",
	       			      pending_req_array));

        /*---------------------------------------------------------------------
        Determine if there is a free structure in the specified array by
        checking for NULL in the correlator field.  There is one array of
	pending DDF requests for each DDF that can have pending requests.
	Each array is defined with an initial size of ARY_SIZE in the
        "midddf_t" structure.  If all structures in an array are in use, this
        means we have ARY_SIZE interrupts of that DDF type pending and we
        must allocate ARY_SIZE more of these structures in the pinned heap.
        Four will be picked as an initial array size since there should
        rarely be more than four interrupts of any given DDF type pending at
        the same time.  

	The structures in the array are linked together to form a linked
	list.  This is so we do not have to keep track of the array size.
	Search the array from top to bottom until the first available
        structure is found.
        ---------------------------------------------------------------------*/

        current_ptr = pending_req_array;

        while (current_ptr->correlator != NULL) {
         
               	/*-------------------------------------------------------------
                This structure is occupied.  Is it the last structure in the
		array?
                -------------------------------------------------------------*/

                if (current_ptr->next != NULL) {

                        /*-----------------------------------------------------
                        It is not, so try the next structure.
                        -----------------------------------------------------*/

                	current_ptr = current_ptr->next;

                } else {

                	/*-----------------------------------------------------
                       	We have found the last structure in the array and
                       	must allocate more.
                       	-----------------------------------------------------*/
			i_enable(*old_interrupt_priority);

                        more_space_ptr = (ddf_data_t *)
                                 	 xmalloc(ARY_SIZE * sizeof(ddf_data_t),
	                 		 4, pinned_heap);

			*old_interrupt_priority = i_disable(PDEV_PRIORITY);

                        if (more_space_ptr == NULL) { /* xmalloc failed */
                                 
				current_ptr = NULL;
				break;
                        }

                        /*-----------------------------------------------------
                        Initialize the new structures.
                        -----------------------------------------------------*/

                        current_ptr->next = more_space_ptr;

                        for (i = 0; i < ARY_SIZE; i++) {
                                 
                                (more_space_ptr + i)->next = more_space_ptr +
                                               		     (i + 1);
                                (more_space_ptr + i)->correlator = NULL;

                        	/*---------------------------------------------
                        	Initialize the watchdog timer.
                        	---------------------------------------------*/

                                (more_space_ptr+i)->watchdog_data.dog.next =
									  NULL;
                                (more_space_ptr+i)->watchdog_data.dog.prev =
									  NULL;
                                (more_space_ptr+i)->watchdog_data.dog.restart =
							            wd_timeout;
                                (more_space_ptr+i)->watchdog_data.dog.func =
								       wd_func;
                                (more_space_ptr+i)->watchdog_data.dog.count = 0;

#ifdef  ENABLE_WATCHDOGS  /* Disable the watchdog timers.  They can cause   */
                          /* problems when the adapter is legitimately      */
                          /* taking more time than the watchdog is set for. */

                                w_init(&((more_space_ptr+i)->watchdog_data.dog));

#endif
                        }

                        /*-----------------------------------------------------
                        Set the "next" field in the last structure.
                        -----------------------------------------------------*/

                        (more_space_ptr + (ARY_SIZE - 1))->next = NULL;

                        /*-----------------------------------------------------
			Point to the first free structure allocated.
                        -----------------------------------------------------*/

	                current_ptr = current_ptr->next; 

		}	
        } /* end loop */

        MID_DD_EXIT_TRACE (midgetfreestr, 2, GET_FREE_PENDING_REQ_STRUCT, ddf, 
			   0, current_ptr, 0, 0);
	BUGLPR(dbg_midgetfreestr, 2, ("Leaving get_free_pending_req_struct\n"));
        BUGLPR(dbg_midgetfreestr, 3, ("current_ptr = 0x%x\n", current_ptr));

	return (current_ptr);
}
