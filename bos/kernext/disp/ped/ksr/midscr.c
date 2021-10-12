static char sccsid[] = "@(#)59  1.11  src/bos/kernext/disp/ped/ksr/midscr.c, peddd, bos411, 9428A410j 3/31/94 21:35:51";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: vttscr
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


#define Bool unsigned

#include <sys/types.h>
#include <sys/malloc.h>
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
#include "hw_dd_model.h"
#include "hw_macros.h"
#include "hw_PCBkern.h"
#include "hw_FIFrms.h"
#include "hw_FIF2dm1.h"
#include "hw_typdefs.h"
#include "hw_errno.h"
#include "hw_regs_u.h"
#include "hw_regs_k.h"

#include "mid_dd_trace.h"
MID_MODULE ( midscr );        /* defines trace variable                    */
BUGXDEF(dbg_middd);

/*--------------------------------------------------------------------*/
/*								      */
/*   IDENTIFICATION: VTTSCR					      */
/*								      */
/*   DESCRIPTIVE name:	Scroll screen up or down		      */
/*								      */
/*   FUNCTION:	Scrolls the entire contents of the screen	      */
/*		(or presentation space if the Virtual Terminal is     */
/*		not active) up or down a specified number of lines.   */
/*		When scrolling the data up, the lines at the top      */
/*		are discarded and the new lines at the bottom are     */
/*		cleared to spaces. When scrolling the data down,      */
/*		the lines at the bottom  are discarded and the	      */
/*		new lines at the top are cleared to spaces.	      */
/*								      */
/*								      */
/*   PSEUDO-CODE:						      */
/*        IF not in character (KSR) mode:		     	      */
/*          - Return                			              */
/*                                                                    */
/*        IF number of scroll lines is non-zero                       */
/*            update presentation space to current attributes         */
/*                                                                    */
/*            erase old cursor                                        */
/*                                                                    */
/*	      Set up display attributes                               */
/*                                                                    */
/*            IF "Do not display" attribute is set                    */
/*              - set foreground color equal to background color      */
/*                                                                    */
/*            Set font                                                */
/*                                                                    */
/*            Set up blank attributes                                 */
/*								      */
/*	      IF all the data has been scrolled off the screen:       */
/*	        - Update all entries in the presentation space        */
/*                with blanks and requested attributes.	              */
/*	        - Write contents of presentation space to screen      */
/*	          (clear the screen)				      */
/*        ELSE                                                        */
/*              - test direction to scroll                            */
/*								      */
/*	      IF data is scrolled up: 			              */
/*              - increment scroll pointer to location in buffer      */
/*                where the upper left character is to be stored      */
/*	        - Store a blank for every character in each line at   */
/*	          bottom of screen				      */
/*								      */
/*	      IF data is scrolled down:			              */
/*              - decrement scroll pointer                            */
/*              - continue to increment scrolloffset by the size      */
/*                of the presentation space until we are no           */
/*                longer past top of presentation space               */
/*	        - Store a blank for every character in each line at   */
/*	          top of screen.				      */
/*								      */
/*	      IF virtual terminal is active			      */
/*	        - Write new Presentation space to the adapter	      */
/*								      */
/*	  Move the cursor if requested			              */
/*								      */
/*								      */
/*   CONCEPTS / DEFINITIONS                                           */
/*								      */
/*        scroll_offset                                               */
/*								      */
/*        scroll_offset is a variable used when dealing with the      */
/*        presentation space.  This variable is an offset from the    */
/*        beginning of the presentation space and is used as a        */
/*        "virtual" top for convenience of scrolling.  This allows us */
/*        to scroll in the presentation space by simply adjusting the */
/*        value of scroll_offset instead of actually refreshing the   */
/*        entire presentation space.                                  */
/*								      */
/*								      */
/*								      */
/*								      */
/*								      */
/*								      */
/*   INPUTS:	Number of lines to scroll (>0 up, < 0 down)	      */
/*		Attribute for area being cleared		      */
/*		Cursor Structure containing new cursor position       */
/*		Flag to indicate whether to move cursor 	      */
/*								      */
/*   OUTPUTS:	INVALID_MODE					      */
/*								      */
/*   CALLED BY: Mode Processor					      */
/*								      */
/*   CALLS:	copy_ps 					      */
/*								      */
/*   BUGS:	The line at the bottom that is scrolled up:  color    */
/*		attributes are lost.				      */
/*								      */
/*--------------------------------------------------------------------*/

long vttscr(vp, lines, attr, cursor_show)
struct vtmstruc     *vp;                /* virtual terminal struct ptr  */
long               lines;               /* number of lines to scroll    */
	                                /*   >0 ==> scroll data up      */
	                                /*   <0 ==> scroll data down    */
ulong              attr;                /* attribute associated with all*/
	                                /* characters of the new lines  */
	                                /* that appear at either the top*/
	                                /* or bottom of the screen      */
ulong             cursor_show;          /* if true cursor is moved to   */
	                                /* position given in cp_parms   */
{


long		i;
ulong		*buf_addr,		/* address of the buffer	*/
		blanks; 		/* blanks with the specifed	*/
					/* attribute			*/
ulong           cursor_off=0,           /* erase cursor                 */
		cursor_on =1;           /* display cursor               */
long		buf_offset;		/* offset into pres. space	*/
struct middata   *ld;			/* ptr to local data area	*/
struct	midddf *ddf = (struct midddf *)vp->display->free_area;

	HWPDDFSetup;			/* gain access to hardware pointer */

        MID_DD_ENTRY_TRACE ( midscr, 1, VTTSCR, ddf,
                0,
                ddf,
                vp,
                lines );


	VDD_TRACE(SCR, ENTRY, vp);
	
	BUGLPR(dbg_midscr, BUGNTA, ("Entering vttscr\n"));
	BUGLPR(dbg_midscr, BUGGID,
	("lines to scroll %d, attr 0x%x\n", lines, attr));


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

		BUGLPR(dbg_midscr, 0, ("vttscr called in MOM!\n"));
		return(INVALID_MODE);
	}



	if (lines != 0)
	/*---------------------------*/
	/* there is something to do  */
	/*---------------------------*/
	{
		/*-----------------------------------------*/
		/* will be updating the presentation space */
		/*-----------------------------------------*/

		buf_addr = ld->pse;

		/*--------------------------------------*/
		/* Erase the old cursor 		*/
		/*--------------------------------------*/
		/* erase_cursor(vp, ld, buf_addr);      */

		/*   enable the bus */
		PIO_EXC_ON();

		BUGLPR(dbg_midscr, BUGNTA, 
			("calling MID_HideShowActiveCursor\n"));

		MID_HideShowActiveCursor(cursor_off);

		PIO_EXC_OFF();


		/*-----------------------------------------------------------*/
		/* calculate the attribute that must be stored with every    */
		/* blank character in the new lines at either the top or     */
		/* bottom of the screen 				     */
		/*-----------------------------------------------------------*/

		blanks = 0x0000;

		{
		SET_ATTRIBAKCOL(blanks, ATTRIBAKCOL(attr));
		SET_ATTRIFORECOL(blanks, ATTRIFORECOL(attr));
		}
	
		if (ATTRIBLA(attr))
		/*-------------------------------------------*/
		/* "Do not display text" attribute is set    */
		/*-------------------------------------------*/
		{
			/*-------------------------------------------*/
			/* make foreground equal to background color */
			/*-------------------------------------------*/
	
			SET_ATTRIFORECOL(blanks, ATTRIBAKCOL(blanks));
		}


		/*-----------------------------------------*/
		/* pull in the font			   */
		/*-----------------------------------------*/

		SET_ATTRIFONT(blanks, ATTRIFONT(attr));


		/*---------------------------------------------------------*/
		/* set up a word with one blank with the specified attrib  */
		/*---------------------------------------------------------*/

		blanks |= 0x00200000;


		/*-------------------------------*/
		/* scroll the presentation space */
		/*-------------------------------*/

		if ((lines >= ld->ps_size.ht) || (lines <= -ld->ps_size.ht))
		/*-----------------------------------------*/
		/* all the data is scrolled off the screen */
		/*-----------------------------------------*/
		{
			/*----------------------------------------------*/
			/* fill the screen with background color and	*/
			/* the PS with blanks				*/
			/*----------------------------------------------*/

			for (i = 0; i < ld->ps_words; i++)
			/*--------------------------------*/
			/* the number of words in the PS  */
			/*--------------------------------*/
			{
				/*---------------------------*/
				/* store a blank into the PS */
				/*---------------------------*/

				ld->pse[i] = blanks;
			}

			if (ld->vtt_active)
			/*--------------------------------*/
			/* the virtual terminal is active */
			/*--------------------------------*/
			{
				/*-------------------------*/
				/* update the frame buffer */
				/*-------------------------*/

				copy_ps(vp);

			}       /* lines > 0  */

		}
		else
		{
	
			/*---------------------------------------------------*/
			/* only part of the data has been scrolled	     */
			/* out of the PS				     */
			/*---------------------------------------------------*/

			if (lines > 0)
			/*-------------------------*/
			/* the data is scrolled up */
			/*-------------------------*/
			{
				/*-------------------------------------------*/
				/* New method of scrolling is to simply inc- */
				/* rement a scroll pointer into the buffer   */
				/* to indicate where in the buffer the	     */
				/* current upper left character is stored    */
				/*-------------------------------------------*/

				ld->scroll_offset = (ld->scroll_offset
					+ ld->ps_size.wd * lines) %
					ld->ps_words;

				for	(i = ((ld->ps_size.ht - lines) *
						ld->ps_size.wd);
					i < ((ld->ps_size.ht) *
						ld->ps_size.wd);
						i++)
				/*-------------------------------------*/
				/* every character in the new lines at */
				/* the bottom of screen 	       */
				/*-------------------------------------*/
				{
				/*---------------------------------*/
				/* store a blank into the new line */
				/*---------------------------------*/

					*(buf_addr + ((i +
						ld->scroll_offset)
						% ld->ps_words))
						= blanks;
				}

			}
			else /* Scrolling text downward (lines < 0) */
			{

			/*----------
			Increment the scroll offset by the (negative)
			number of lines scrolled times number of
			characters in a line of the presentation space.
			---------*/

				ld->scroll_offset +=
					ld->ps_size.wd * lines ;

				while (ld->scroll_offset < 0)
				/*----------------------------------*/
				/*  we have scrolled off the top of */
				/*  the presentation space	    */
				/*----------------------------------*/
				/* add the size of the ps space to  */
				/* scroll_offset		    */
				/*----------------------------------*/

					ld->scroll_offset += ld->ps_words;


				for (i = 0;
					i < ((-lines) * ld->ps_size.wd);
					i++)
				/*----------------------------------*/
				/* every character in the new lines */
				/* at the top of the screen.	    */
				/* NOTE: "lines" is negative.	    */
				/*----------------------------------*/
				{
				/*---------------------------------*/
				/* store a blank into the new line */
				/*---------------------------------*/

					*(buf_addr +
					((i + ld->scroll_offset) %
					ld->ps_words))
					= blanks;
				}    /* for */

			}   /* else */

		}   /*else */


		if (ld->vtt_active)
		/*--------------------------------------------------*/
		/* the virtual terminal is active do all things     */
		/* necessary to write entire contents of pspace to  */
		/* adapter					    */
		/*--------------------------------------------------*/
		{
			copy_ps(vp);

		}



	}       /*  if lines != 0  */


	ld->cursor_show = cursor_show;

	if (cursor_show != 0)
	/*--------------------------*/
	/* the cursor must be moved */
	/*--------------------------*/
	{
		/*------------------*/
		/* move the cursor  */
		/*------------------*/
		vttmovc(vp);


		/*--------------------------------------------------*/
		/* turn cursor back on after scrolling.             */
		/*--------------------------------------------------*/

		PIO_EXC_ON();                  

		MID_HideShowActiveCursor(cursor_on);

		PIO_EXC_OFF();
	}

	VDD_TRACE(SCR, EXIT, vp);

        MID_DD_EXIT_TRACE ( midscr, 1, VTTSCR, ddf,
                0,
                attr,
                cursor_show,
                0 );


	BUGLPR(dbg_midscr, BUGNTA, ("Leaving vttscr with rc = 0.\n"));
	return(0);

}				 /* end of vttscr  */
