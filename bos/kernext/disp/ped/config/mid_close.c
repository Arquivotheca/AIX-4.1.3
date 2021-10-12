static char sccsid[] = "@(#)55	1.8  src/bos/kernext/disp/ped/config/mid_close.c, peddd, bos411, 9428A410j 6/23/94 10:40:06";

/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: mid_close
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


#include <sys/types.h>
#include <sys/iocc.h>
#include <sys/proc.h>
#include <sys/mdio.h>
#include <sys/conf.h>
#include <sys/file.h>
#include <sys/intr.h>
#include <sys/uio.h>
#include <sys/dma.h>
#include <sys/pin.h>
#include <sys/malloc.h>
#include <sys/device.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/errids.h>
#include <sys/syspest.h>
#include <sys/systm.h>
#include <sys/errno.h>
#include "mid.h"
#define Bool unsigned
#include <sys/aixfont.h>
#include <sys/display.h>

#include "ddsmid.h"
#include "midddf.h"
#include "midhwa.h"
/*
#include "midksr.h"
#include "midrcx.h"
#include "midfifo.h"
*/

BUGXDEF(dbg_middd);
BUGXDEF(dbg_midddi);
BUGXDEF(dbg_middma);
BUGXDEF(dbg_midddf);
BUGXDEF(dbg_midrcx);
BUGXDEF(dbg_midevt);
BUGXDEF(dbg_midtext);

#ifndef BUGPVT
#define BUGPVT	99
#endif

#ifndef BUGVPD				/* Set to 0 in Makefile if debugging */
#define BUGVPD	BUGACT
#endif


extern void mid_WID_wakeup();



/***********************************************************************/
/*								       */
/* IDENTIFICATION: MID_CLOSE					       */
/*								       */
/* DESCRIPTIVE NAME: MID_CLOSE - Close routine for MIDDD3D	       */
/*								       */
/* FUNCTION:							       */
/*								       */
/*								       */
/*								       */
/* INPUTS:							       */
/*								       */
/* OUTPUTS:							       */
/***********************************************************************/

long  mid_close(devno,chan,ext)
dev_t devno;
long chan;		    /* Channel number ignored by this routine */
long ext;		    /* Extended system parameter - not used   */

{
	struct phys_displays *pd;
	midddf_t *ddf;
	int status, i;

	/*-------------------------------------------------------------*/
	/* This routine will check the usage field in the Phys_display */
	/* struct and see if it is non-zero. If so we can close the    */
	/* device if not then a -1 is return to the caller	       */
	/*							       */
	/* Here will be the devsw traversal for devices which can have */
	/* multiple instances.					       */
	/*-------------------------------------------------------------*/

	BUGLPR(dbg_middd, BUGVPD, ("Entering mid_close.\n"));

	devswqry(devno,&status,&pd);

	for (; pd; pd = pd->next)
		if (pd->devno == devno)
			break;
	if (!pd)
		return -1;
	
	/*-------------------------------------------------------------*/
	/* test to see if the device is in use			       */
	/*-------------------------------------------------------------*/

	if (pd->usage > 0)

		/*-----------------------------------------------------*/
		/* Error condition - there are still open vts	       */
		/*-----------------------------------------------------*/

		return(-1);

        ddf = (midddf_t *) pd->free_area;

	/*-------------------------------------------------------------*/
	/* Clean up system resources				       */
	/*-------------------------------------------------------------*/

	d_clear(pd->dma_chan_id);

        if (ddf->WID_watch.dog.func == mid_WID_wakeup)
        {
           w_clear(&(ddf->WID_watch.dog)) ;
        }

#define  SWAP_LIST      ddf->ddf_data_swapbuffers_hold_list

        for (i=0; i<MAX_WIDS; i++)
        {
               if ( ! SWAP_LIST[i].watch.dog.restart )
                       break;

               w_clear( &(SWAP_LIST[i].watch.dog) ) ;
        }


	/*-------------------------------------------------------------*/
	/* If we reach here there are no open vts on this display set  */
	/* the open_cnt field to 0.				       */
	/*-------------------------------------------------------------*/

	pd->open_cnt = 0;
	BUGLPR(dbg_middd, BUGVPD, ("Leaving mid_close.\n"));
	return(0);

}
