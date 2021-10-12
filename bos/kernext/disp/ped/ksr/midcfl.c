static char sccsid[] = "@(#)56  1.5.1.2  src/bos/kernext/disp/ped/ksr/midcfl.c, peddd, bos411, 9428A410j 2/28/94 20:24:36";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: min
 *		vttcfl
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


#define Bool unsigned

#include <sys/types.h>
#include <sys/malloc.h>
#include <sys/ioacc.h>
#include <sys/conf.h>
#include <sys/device.h>
#include <sys/mdio.h>
#include <sys/devinfo.h>
#include <sys/file.h>
#include <sys/sysmacros.h>
#include <sys/uio.h>
#include <sys/vnode.h>
#include <sys/iocc.h>
#include <sys/proc.h>
#include <sys/intr.h>
#include <sys/pin.h>
#include <sys/sleep.h>
#include <sys/errids.h>
#include <sys/aixfont.h> 
#include <sys/syspest.h>
#include "mid.h" 
#include "midddf.h"
#include "midhwa.h"
#include "midksr.h" 
#include <fcntl.h>
#include "hw_dd_model.h"	/* defines MID_DD			*/
#include "hw_macros.h"
#include "hw_PCBrms.h"
#include "hw_FIFrms.h"
#include "hw_FIF2dm1.h"
#include "hw_typdefs.h"
#include "hw_errno.h"
#include "hw_regs_u.h"
#include "hw_regs_k.h"

#include "mid_dd_trace.h"
MID_MODULE ( midcfl );        /* defines trace variable                    */

BUGXDEF(dbg_middd);

#define min(val1,val2)          (val1 < val2) ? val1 : val2;

/*---------------------------------------------------------------------*/
/*								       */
/* IDENTIFICATION: VTTCFL					       */
/*								       */
/* DESCRIPTIVE NAME: Copy Full Lines				       */
/*								       */
/* FUNCTION:  This function vertically copies full lines of characters */
/*            from one area of the screen to another.  It can copy any */
/*            number of lines from anywhere on the screen to anywhere  */
/*            on the screen.					       */
/*								       */
/*	      The following are the input constraints:		       */
/*								       */
/*	      1) This function is valid only in character mode.        */
/*	      2) The source and destination rows must be valid for     */
/*		 the current presentation space.		       */
/*	      3) There must be at least as many rows as the number of  */
/*		 rows to copy.					       */
/*								       */
/* PSEUDO CODE: 						       */
/*                                                                     */
/*            IF not in character (KSR) mode                           */
/*               - return INVALID_MODE                                 */
/*                                                                     */
/*	      IF the source and destination rows are the same	       */
/*		 - return: the copy is trivially successful	       */
/*                                                                     */
/*            Calculate starting offset of source and destination      */
/*            strings in presentation space.                           */
/*                                                                     */
/*	      IF the destination row is above the source row	       */
/*		 - copy from top to bottom			       */
/*	      ELSE						       */
/*		 - copy from bottom to top			       */
/*								       */
/*	      IF virtual terminal is active, update frame buffer       */
/*								       */
/*	      IF cursor_show = TRUE				       */
/*		 - move the cursor ( call vttmovc() )		       */
/*								       */
/*								       */
/* INPUTS:    *vp	   the pointer to the virtual terminal	       */
/*			   data structure, defined in vt.h.	       */
/*	      s_row	   the source row number		       */
/*	      d_row	   the destination row number		       */
/*	      num_rows	   the number of rows to copy		       */
/*	      cursor_show  boolean: should the cursor be moved	       */
/*								       */
/*								       */
/* OUTPUTS:   BAD_S_ROW 					       */
/*	      BAD_D_ROW 					       */
/*	      TOO_MANY_ROWS					       */
/*	      INVALID_MODE					       */
/*								       */
/*								       */
/* CALLED BY: This routine is called by the Virtual Terminal Mode      */
/*	      Processor.  It is an entry point. 		       */
/*								       */
/*								       */
/* CALLS:     This routine calls vttmovc(), if necessary.	       */
/*								       */
/*---------------------------------------------------------------------*/

long vttcfl(vp, s_row, d_row, num_rows, cursor_show)
struct vtmstruc    *vp;                    /* virtual terminal struct ptr  */
long               s_row,                  /* source row                   */
	           d_row,                  /* destination row              */
	           num_rows;               /* number of rows to copy       */
ulong              cursor_show;            /* boolean - move the cursor?   */

{

ulong	*buf_addr;		/* address of the buffer		*/
ulong	old_bus;

long	s_offset,		/* starting offset of the source string */
	d_offset,		/* starting offset of the target string */
	buf_offset,		/* offset for erasing the old cursor	*/
	num_words,		/* number of full words to be moved	*/
	factor; 		/* move direction indicator		*/
short	screen_x_ll,
	screen_y_ll,
	screen_x_ur,
	screen_y_ur,
	screen_x,
	screen_y,
	newx,
	newy;

struct	       middata *ld;		      /* ptr to local data area */

struct midddf *ddf = (struct midddf *)vp->display->free_area;

	HWPDDFSetup;		     /* gain access to hardware pointer */

        MID_DD_ENTRY_TRACE ( midcfl, 1, VTTCFL, ddf,
                0,
                ddf,
                vp,
                num_rows );


	VDD_TRACE(CFL, ENTRY, vp);
	
	BUGLPR(dbg_midcfl, BUGNTA, ("Entering vttcfl \n"));



	/*---------------------------------*/
	/* set the local data area pointer */
	/*---------------------------------*/

	ld = (struct middata *)vp->vttld;


	if (ld->vtt_mode != KSR_MODE)
	/*-----------------------*/
	/* not in character mode */
	/*-----------------------*/
	{
		/*------------------------------------*/
		/* Only valid in character (KSR) mode */
		/*------------------------------------*/

		BUGLPR(dbg_midcfl, 0, ("vttcfl called in MOM!\n"));
		return(INVALID_MODE);
	}


	if (s_row == d_row)
	/*---------------------------------------------------*/
	/* the source address equals the destination address */
	/*---------------------------------------------------*/
	{
		/*------------------------*/
		/* There is nothing to do */
		/*------------------------*/


		return(0);
	}


	buf_addr = (ulong *)ld->pse;


	/*--------------------------------------------------------*/
	/* calculate the starting offset of the first line to be  */
	/* moved in the source and destination strings		  */
	/*--------------------------------------------------------*/

	s_offset = ((s_row - 1) * ld->ps_size.wd);
	d_offset = ((d_row - 1) * ld->ps_size.wd);


	/*-----------------------------------------------------------------*/
	/* calculate the number of full words to be copied.		   */
	/* the number of rows to copy cannot exceed the number of rows in  */
	/* the presentation space.					   */
	/*-----------------------------------------------------------------*/

	num_words = (ld->ps_size.ht + 1) - d_row;
	num_words = min(num_words, num_rows);
	num_words *= ld->ps_size.wd ;


	if (s_row > d_row)
	/*-------------------------------------*/
	/* the destination is above the source */
	/*-------------------------------------*/
	{

		/*-----------------------------------------------------*/
		/* copy top to bottom, ie, copy top source line to top */
		/* of destination.  Then copy 2nd line of source to    */
		/* line of destination, etc ...                        */
		/*-----------------------------------------------------*/

		factor = 1;

	}
	else
	{
		/*-----------------------------------------------------*/
		/* copy bottom to top, i.e, copy bottom line of source */
		/* to bottom line of destination.  Then copy next to   */
		/* last line of soucre, etc ...                        */
		/*-----------------------------------------------------*/

		factor = -1;
		s_offset += num_words - 1;
		d_offset += num_words - 1;
	}


	for (; num_words > 0; num_words--)
	/*---------------------------------*/
	/* all lines that are to be copied */
	/*---------------------------------*/
	{
		/*----------------------------*/
		/* copy the next line segment */
		/*----------------------------*/

		*(buf_addr + ((d_offset + ld->scroll_offset) % ld->ps_words)) =
		*(buf_addr + ((s_offset + ld->scroll_offset) % ld->ps_words));
		s_offset += factor;
		d_offset += factor;
	}


	if (ld->vtt_active)
	/*--------------------------------*/
	/* the virtual terminal is active */
	/*--------------------------------*/
	{
		/*--------------------------------*/
		/* update the frame  buffer	  */
		/*--------------------------------*/

		copy_ps(vp);

	}


	if (ld->cursor_show = cursor_show /* assignment */)
	/*--------------------------*/
	/* the cursor must be moved */
	/*--------------------------*/
	{
		/*------------------*/
		/* move the cursor  */
		/*------------------*/
		vttmovc(vp);
	}


	VDD_TRACE(CFL, EXIT, vp);
	
        MID_DD_EXIT_TRACE ( midcfl, 1, VTTCFL, ddf,
                0,
                s_row,
                d_row,
                cursor_show );


	BUGLPR(dbg_midcfl, BUGNTA, ("Leaving vttcfl with rc = 0.\n"));

	return(0);

}					/* end	of  vttcfl	       */
