static char sccsid[] = "@(#)58	1.3.2.4  src/bos/kernext/disp/ped/config/cfg_term.c, peddd, bos411, 9428A410j 6/23/94 10:40:01";
/*
 * COMPONENT_NAME: PEDDD
 *
 * FUNCTIONS: cfg_term
 *	
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <sys/types.h>
#include <sys/intr.h>
#include <sys/malloc.h>
#include <sys/syspest.h>

#include "midddf.h"

BUGXDEF(dbg_middd);

#ifndef BUGVPD				/* Set to 0 in Makefile if debugging */
#define BUGVPD	BUGACT
#endif

extern long mid_intr();
extern void mid_timeout();
extern void mid_timeout_gettextfontindex();
extern void mid_timeout_getcondition();
extern void mid_timeout_getcolor();
extern void mid_timeout_getcpos();
extern void mid_timeout_swap();
extern void mid_timeout_lowwater();
extern void mid_timeout_endrender();
extern void mid_WID_wakeup();





/***********************************************************************/
/*								       */
/* IDENTIFICATION: CFG_TERM					       */
/*								       */
/* DESCRIPTIVE NAME: CFG_TERM - Config Terminate routine for MIDDD3D   */
/*								       */
/* FUNCTION:							       */
/*								       */
/*								       */
/*								       */
/* INPUTS:							       */
/*								       */
/* OUTPUTS:							       */
/***********************************************************************/

long cfg_term(devno)
dev_t devno;

{
	int status;
	int minors;
	int i, rc = 0;
	label_t jmpbuf;
	unsigned int save_bus_acc = 0;
	midddf_t        *ddf;
	struct phys_displays *pd,*last;

	BUGLPR(dbg_middd, BUGVPD, ("Entering cfg_term\n"));

	devswqry(devno,&status,&pd);
	last = NULL;
	minors = 0;

	/*----------
	  Search the linked list of phys_displays, matching
	  on devno.  We use minors to count the number of
	  phys_display structures that didn't match.  After
	  we find an entry, we increment minors one more time
	  to make sure that the only case in which it remains
	  zero is if the one we were looking for was really
	  the last one.  We use last to update the previous
	  phys_display to point around the one we are deleting.
	  If last is NULL, we need to update the devsw entry.
	  ----------*/

	while (pd) {
		if (pd->devno == devno)
			break;
		last = pd;
		++minors;
		pd = pd->next;
	}
	if (!pd)
		return -1;

	if (pd->next)
		++minors;


	/*---------------------------------------------------*/
	/* Now that we have the pd see if we are open.	     */
	/* if so return -1.				     */
	/*---------------------------------------------------*/

	if (pd->open_cnt != 0)
		return( EBUSY );

	/*---------------------------------------------------*/
	/* set the pointer in the previous structure         */
	/*---------------------------------------------------*/

	if (last)
		last->next = pd->next;

	ddf = (midddf_t *) pd->free_area;


#ifdef  ENABLE_WATCHDOGS  /* Disable the watchdog timers.  They can cause   */
                          /* problems when the adapter is legitimately      */
                          /* taking more time than the watchdog is set for. */


	/*---------------------------------------------------*/
	/* Clear the watchdog timers.                        */
	/*---------------------------------------------------*/

	w_clear(&ddf->mid_watch.dog);

  	/*
           if w_init() was called for this watch dog, then clear it.
           If it wasn't and we tried to clear it, machine will crash.

           There is a case where cfg_term was called by config manager
           during boot time to unconfig the Lega due to some error.  This
           was done before mid_init_ddf.c was called
        */

        if (ddf->mid_lowwater_watchdog.dog.func == mid_timeout_lowwater)
        {
           w_clear(&(ddf->mid_lowwater_watchdog.dog)) ;
        }


	for ( i=0; i<ARY_SIZE; i++ )
	{

		if ( ! ddf->ddf_data_getcpos[i].watchdog_data.dog.restart )
			break;

		w_clear( &(ddf->ddf_data_getcpos[i].watchdog_data.dog) );

		w_clear( &(ddf->ddf_data_getcolor[i].watchdog_data.dog) );

		w_clear( &(ddf->ddf_data_getcondition[i].watchdog_data.dog) );

		w_clear( &(ddf->ddf_data_gettextfontindex[i].watchdog_data.dog) );
		w_clear( &(ddf->ddf_data_endrender[i].watchdog_data.dog) );
	}
#endif


	/*---------------------------------------------------*/
	/* Removes the interrupt handler		     */
	/*---------------------------------------------------*/
	i_clear( pd );
	
	/*---------------------------------------------------*/
	/* We now have the pointer to the display structure  */
	/* free the ddf structure and the display structure  */
	/*---------------------------------------------------*/

	xmfree( ddf->default_context_RCX, pinned_heap );
	xmfree( ddf->current_context_midRCX, pinned_heap );
	xmfree( ddf->int_trace.top, pinned_heap );
	xmfree( pd->odmdds, pinned_heap );
	xmfree( pd->free_area, pinned_heap );
	xmfree( pd, pinned_heap );

	if ( ( rc = unpincode( ( void * ) mid_intr ) ) != 0 )
	{
		BUGLPR(dbg_middd, 0,
		("Did not unpin interrupt code successfully - %d\n", rc));
	}

#ifdef  ENABLE_WATCHDOGS  /* Disable the watchdog timers.  They can cause   */
                          /* problems when the adapter is legitimately      */
                          /* taking more time than the watchdog is set for. */

	if ( ( rc = unpincode( ( void * ) mid_timeout ) ) != 0 )
	{
		BUGLPR(dbg_middd, 0,
			("Did not unpin watchdog timer successfully\n"));
	}

	if ((rc = unpincode((void *) mid_timeout_getcpos)) != 0)
	{
		BUGLPR(dbg_middd, 0,
			("Did not pin watchdog timer successfully\n"));
		return(rc);
	}

	if ((rc = unpincode((void *) mid_timeout_getcolor)) != 0)
	{
		BUGLPR(dbg_middd, 0,
			("Did not pin watchdog timer successfully\n"));
		return(rc);
	}

	if ((rc = unpincode((void *) mid_timeout_getcondition)) != 0)
	{
		BUGLPR(dbg_middd, 0,
			("Did not pin watchdog timer successfully\n"));
		return(rc);
	}

	if ((rc = unpincode((void *) mid_timeout_gettextfontindex)) != 0)
	{
		BUGLPR(dbg_middd, 0,
			("Did not pin watchdog timer successfully\n"));
		return(rc);
	}

	if ((rc = unpincode((void *) mid_timeout_lowwater)) != 0)
        {
                BUGLPR(dbg_middd, 0,
                        ("Did not unpin watchdog timer successfully\n"));
                return(rc);
        }

	if ((rc = unpincode((void *) mid_timeout_endrender)) != 0)
	{
		BUGLPR(dbg_middd, 0,
			("Did not unpin watchdog timer successfully\n"));
		return(rc);
	}
#endif


	if ((rc = unpincode((void *) mid_timeout_swap)) != 0)
	{
		BUGLPR(dbg_middd, 0,
			("Did not pin watchdog timer successfully\n"));
		return(rc);
	}


	if ((rc = unpincode((void *) mid_WID_wakeup)) != 0)
	{
		BUGLPR(dbg_middd, 0,
			("Did not pin watchdog timer successfully\n"));
		return(rc);
	}



	/*----------
	  If this is the last minor device, delete ourselves from
	  the device switch table and leave.
	  ----------*/
	if (!minors) {
		devswdel( devno );
		return(0);
	}

      /*else	TBD:
		We need to change the pointer in the devsw table,
		but there is currently no mechanism for doing so.
		This needs to be fixed!
		MIDDD3D_devsw.d_dsdptr = (char *) pd; */

	BUGLPR(dbg_middd, BUGACT, ("Leaving cfg_term\n"));

	return ( 0 );
}
