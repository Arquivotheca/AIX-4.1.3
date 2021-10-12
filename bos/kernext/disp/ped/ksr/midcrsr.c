static char sccsid[] = "@(#)57  1.12.1.3  src/bos/kernext/disp/ped/ksr/midcrsr.c, peddd, bos411, 9428A410j 3/31/94 21:35:40";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: vttdefc
 *		vttmovc
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
#include "hw_ind_mac.h"
#include "hw_PCBkern.h"
#include "hw_FIFrms.h"
#include "hw_FIF2dm1.h"
#include "hw_typdefs.h"
#include "hw_errno.h"
#include "hw_regs_u.h"
#include "hw_regs_k.h"

#include "mid_dd_trace.h"
MID_MODULE ( midcrsr );        /* defines trace variable                    */

BUGXDEF(dbg_middd);

BUGXDEF(dbg_midtext);

/*----------------------------------------------------------------------*/
/*                                                                      */
/*   IDENTIFICATION: VTTDEFC                                            */
/*                                                                      */
/*   DESCRIPTIVE NAME:  Define Cursor                                   */
/*                                                                      */
/*   FUNCTION:  This function defines cursor shapes and assigns         */
/*              cursor ids.                                             */
/*                                                                      */
/*              If VT is active, change shape according to following    */
/*              selector values:                                        */
/*                                                                      */
/*                0 - Invisible cursor                                  */
/*                1 - Single underscore                                 */
/*                2 - Double underscore                                 */
/*                3 - Half Blob                                         */
/*                4 - Mid character double line                         */
/*                5 - Full blob                                         */
/*                Any other value will get double underscore            */
/*                                                                      */
/*   PSEUDO CODE:                                                       */
/*                                                                      */
/*              If VT is not in character mode                          */
/*                  Return INVALID_MODE                                 */
/*                                                                      */
/*              Set cursor image buffer :                               */
/*                                                                      */
/*                  - The cursor image buffer consists of 128 words     */
/*                    representing a 64x64 bit array (64 double-word).  */
/*                    The hot point of the cursor is fixed at the       */
/*                    center of the cursor shape (i.e. between the 64th */
/*                    and 65th words)                                   */
/*                                                                      */
/*              Set cursor color                                        */
/*                                                                      */
/*              Set the top and bottom boundries of the cursor image    */
/*              based on the character size (the cursor hot point       */
/*              matches the center of the character).  For example:     */
/*                                                                      */
/*                  - Single underscore (select = 1)                    */
/*                                                                      */
/*                      Top boundary = The last row of the character    */
/*                          bit set.  For example, for the font size    */
/*                          of 22x9, this number will be the 32nd       */
/*                          double-word (where the hot point is) +      */
/*                          11 double-words (the size of a half char.   */
/*                          length)                                     */
/*                                                                      */
/*                      Bottom boundary = the same as the top boundary  */
/*                                                                      */
/*              Set the cursor image:                                   */
/*                                                                      */
/*                  - For each double-word within the top and bottom    */
/*                    boundary of the cursor image buffer the font      */
/*                    width will be used to decide how many bits        */
/*                    needs to be turned on (the cursor hot point       */
/*                    should match with the center of the character)    */
/*                                                                      */
/*              Send PCB to the adapter to define the cursor            */
/*                                                                      */
/*              If cursor_show                                          */
/*                 move the cursor ( call vttmovc() )                   */
/*                                                                      */
/*   INPUTS:    Vtt structure                                           */
/*              Selector value                                          */
/*              Cursor_show (0 means don't show cursor)                 */
/*                                                                      */
/*   OUTPUTS:   INVALID_MODE                                            */
/*                                                                      */
/*   CALLED BY: vttinit                                                 */
/*              vttact                                                  */
/*              Mode Processor                                          */
/*                                                                      */
/*   CALLS:     vttmovc                                                 */
/*                                                                      */
/*----------------------------------------------------------------------*/

long vttdefc(vp, selector, cursor_show)
struct vtmstruc    *vp;                 /* virtual terminal struct ptr  */
uchar              selector;            /* shape selector               */
ulong              cursor_show;         /* 0 ==> don't show the cursor  */

{
	struct middata  *ld;            /* ptr to local data area       */

	ulong   ccolor;                 /* cursor color table           */
	ulong   cimage[128];            /* cursor image bitmap          */
	ulong   i,j;
	short  centerToBot;             /* center to char base bottom   */
	short  centerToLeft;            /* center to char base left     */
	ushort  hotx, hoty;             /* cursor hot point             */

	struct midddf *ddf = (struct midddf *)vp->display->free_area;

	ushort screen_x, screen_y;

	HWPDDFSetup;                    /* gain access to hardware pointer*/

#	define CURSOR_CENTER		32


        MID_DD_ENTRY_TRACE ( midcrsr, 1, VTTDEFC, ddf,
                0,
                ddf,
                selector,
                cursor_show );

	VDD_TRACE(DEFC, ENTRY, vp);
	
	BUGLPR(dbg_midcrsr, BUGNTA, ("Entering vttdefc \n"));
	BUGLPR(dbg_midcrsr, 1, ("Cursor selector %d\n", selector));
	BUGLPR(dbg_midcrsr, 1, ("Cursor show %d\n", cursor_show));

	if (ddf->hwconfig & MID_BAD_UCODE)
	{
		BUGLPR(dbg_midcrsr, BUGNTA, ("Leaving due to bad ucode \n"));
		return ( 0 );
	}

	/*-----------------------------------------*/
	/* set the local data area pointer         */
	/*-----------------------------------------*/

	ld = (struct middata *)vp->vttld;

	if (ld->vtt_mode != KSR_MODE)
	/*------------------------------------*/
	/* not in character mode              */
	/*------------------------------------*/
	{
		/*------------------------------------*/
		/* Only valid in character (KSR) mode */
		/*------------------------------------*/

		BUGLPR(dbg_midcrsr, 1, ("vttdefc called in MOM!\n"));
		return(INVALID_MODE);
	}

	/*-----------------------------------------*/
	/* Save cursor select and cursor color     */
	/*-----------------------------------------*/

	ld->cursor_shape.blank = FALSE;
	ld->cursor_select = selector;

	for (i = 0; i < 128; i++)
		cimage[i] = (ulong) 0;

	ccolor = (ulong) ld->color_table.rgbval[ld->cursor_color.fg];
	BUGLPR(dbg_midcrsr, BUGNTA, ("Changing cursor color to ccolor 0x%x\n",
		ccolor));

	BUGLPR(dbg_midcrsr, 1, ("char box wd %d ht %d\n",
		ld->char_box.wd,ld->char_box.ht));
	BUGLPR(dbg_midcrsr, 1, ("char box lft_sd %d ascnt %d\n",
		ld->char_box.lft_sd,ld->char_box.ascnt));

	centerToLeft = (ld->char_box.wd) >> 1;
	centerToBot  = (ld->char_box.ht) >> 1;

	BUGLPR(dbg_midcrsr, 1, ("centerToBot %d centerToLeft %d\n",
		centerToBot,centerToLeft));

	switch (selector)
	/*-----------------------------------------*/
	/* on the cursor shape                     */
	/*-----------------------------------------*/
	{
		case 1:
			/*-------------------*/
			/* single underscore */
			/*-------------------*/

			ld->cursor_shape.top = CURSOR_CENTER + centerToBot;
			ld->cursor_shape.bot = ld->cursor_shape.top;

			break;

		case 0:
			/*-------------------*/
			/* invisible         */
			/*-------------------*/

			ld->cursor_shape.blank = TRUE;
			ccolor = 0;
			ld->cursor_shape.top = 64;
			ld->cursor_shape.bot = 62;

			break;

		case 2:
		default:
			/*-------------------*/
			/* double underscore */
			/*-------------------*/
			ld->cursor_shape.top = CURSOR_CENTER + centerToBot - 1;
			ld->cursor_shape.bot = ld->cursor_shape.top + 1;

			break;

		case 3:
			/*-----------------------------------*/
			/* half blob lower half of character */
			/*-----------------------------------*/
			ld->cursor_shape.top = CURSOR_CENTER;
			ld->cursor_shape.bot = CURSOR_CENTER + centerToBot;

			break;

		case 4:
			/*-----------------------------------*/
			/* mid character double line         */
			/*-----------------------------------*/
			ld->cursor_shape.top = CURSOR_CENTER - 1;
			ld->cursor_shape.bot = ld->cursor_shape.top + 1;

			break;

		case 5:
			/*-----------------------------------*/
			/* full blob                         */
			/*-----------------------------------*/
			ld->cursor_shape.top = CURSOR_CENTER - centerToBot;
			ld->cursor_shape.bot = CURSOR_CENTER + centerToBot;

			break;

	}               /* end of switch */

	if(!ld->vtt_active)
		return(0);

	BUGLPR(dbg_midcrsr, 1, ("cursor shape top %d bot %d\n",
		ld->cursor_shape.top,ld->cursor_shape.bot));

	for (j = ld->cursor_shape.top; j <= ld->cursor_shape.bot;j++) {
		/* set on the center pixels for one cursor row */
		i = j<<1;
		BUGLPR(dbg_midcrsr, 1, ("i %d j %d\n", i,j));
		cimage[i++] = 0xffffffff >> (CURSOR_CENTER - centerToLeft);
		cimage[i] = 0xffffffff << (CURSOR_CENTER - centerToLeft);
	}

	hotx = CURSOR_CENTER - (ld->char_box.wd >> 1);
	hoty = 29 + (ld->char_box.ascnt >> 1);

	BUGLPR(dbg_midcrsr, 1, ("hotx %d hoty %d\n", hotx,hoty));

	PIO_EXC_ON();

	BUGLPR(dbg_midcrsr, 1, ("Defining cursor!\n"));

	MID_DefineCursor(0,0x8040,0,0,0,0,ccolor,0,0,hotx,hoty,cimage,128,
		(MID_CHANGE_CURSOR_FLAGS | MID_CHANGE_CURSOR_COLORS |
		MID_CHANGE_CURSOR_IMAGE | MID_CHANGE_HOT_SPOT_COORDINATES))

	BUGLPR(dbg_midcrsr, 1, ("Cursor defined!\n"));

	ps_to_screen_text(1,1,(&screen_x),(&screen_y));

	BUGLPR(dbg_midcrsr, 1, ("Activate cursor!\n"));

	MID_SetActiveCursor(0,
                            screen_x,screen_y);
	BUGLPR(dbg_midcrsr, 1, ("Activate cursor done!\n"));

	PIO_EXC_OFF();

	ld->cursor_show = cursor_show;

	if (cursor_show)
	/*--------------------------*/
	/* the cursor must be moved */
	/*--------------------------*/
	{
		/*-----------------*/
		/* move the cursor */
		/*-----------------*/
		vttmovc(vp);
	}

	VDD_TRACE(DEFC , EXIT, vp);

        MID_DD_EXIT_TRACE ( midcrsr, 1, VTTDEFC, ddf,
                0,
                ccolor,
                ld->cursor_shape.top,
                ld->cursor_shape.bot );


	BUGLPR(dbg_midcrsr, BUGNTA, ("Leaving vttdefc with rc = 0.\n"));
	return(0);

}                                /* end  of vttdefc              */


/*----------------------------------------------------------------------*/
/*                                                                      */
/*   IDENTIFICATION: VTTMOVC                                            */
/*                                                                      */
/*   DESCRIPTIVE NAME:  Move Cursor                                     */
/*                                                                      */
/*   FUNCTION:  Moves the cursor to the specified                       */
/*              row and column position on the screen. Row and          */
/*              column numbers are unity based.                         */
/*                                                                      */
/*   PSEUDO CODE:                                                       */
/*                                                                      */
/*              If not in KSR mode:                                     */
/*                  Return INVALID_MODE                                 */
/*                                                                      */
/*              Update the cursor position in environment structure     */
/*                                                                      */
/*              If cursor position is invalid:                          */
/*                  Set the cursor to the default position (0, 0)       */
/*                                                                      */
/*              Convert presentation space coordinates to MIDDD screen  */
/*              coordinates                                             */
/*                                                                      */
/*              Send commands to the adapter to move the cursor         */
/*                                                                      */
/*   INPUTS:    vtt_cursor structure containg new cursor position       */
/*                                                                      */
/*   OUTPUTS:   INVALID_MODE                                            */
/*                                                                      */
/*   CALLED BY: Mode Processor                                          */
/*              vttcfl                                                  */
/*              vttclr                                                  */
/*              vttcpl                                                  */
/*              vttdefc                                                 */
/*              vtttext                                                 */
/*              vttscr                                                  */
/*                                                                      */
/*   CALLS:     None.                                                   */
/*                                                                      */
/*----------------------------------------------------------------------*/
long vttmovc(vp)
struct vtmstruc *vp;                    /* virtual terminal struct ptr  */
{

struct middata  *ld;                    /* ptr to local data area       */
ushort          i,                      /* loop variable                */
		j,                      /* loop variable                */
		row,                    /* new cursor row               */
		col,                    /* new cursor column            */
		screen_x,               /* screen x coord               */
		screen_y;               /* screen y coord               */

struct midddf *ddf = (struct midddf *)vp->display->free_area;

	HWPDDFSetup;                    /* gain access to hardware pointer*/

	VDD_TRACE(MOVC, ENTRY, vp);

        MID_DD_ENTRY_TRACE ( midcrsr, 1, VTTMOVC, ddf,
                0,
                ddf,
                vp,
                0 );


	BUGLPR(dbg_midcrsr, BUGGID, ("Entering vttmovc \n"));

	if (ddf->hwconfig & MID_BAD_UCODE)
	{
		BUGLPR(dbg_midcrsr, BUGNTA, ("Leaving due to bad ucode \n"));
		return ( 0 );
	}

	/*------------------------------------*/
	/* set the local data area pointer    */
	/*------------------------------------*/

	ld = (struct middata *)vp->vttld;


	if (ld->vtt_mode != KSR_MODE)
	/*------------------------------------*/
	/* not in character mode              */
	/*------------------------------------*/
	{
		/*------------------------------------*/
		/* Only valid in character (KSR) mode */
		/*------------------------------------*/

		BUGLPR(dbg_midcrsr, 1, ("vttmovc called in MOM!\n"));
		return(INVALID_MODE);
	}


	/*----------------------------------------------------------------*/
	/* update the variable that tracks the virtual terminal's current */
	/* cursor position                                                */
	/*----------------------------------------------------------------*/
	ld->cursor_pos.row = row = vp->mparms.cursor.y;
	ld->cursor_pos.col = col = vp->mparms.cursor.x;

	if      ((row > ld->ps_size.ht) ||
		(col > ld->ps_size.wd)  ||
		(row < 1)               ||
		(col < 1))
	/*--------------------------------*/
	/* the cursor position is invalid */
	/*--------------------------------*/
	{
		/*----------------------------------------------*/
		/* set the cursor to the default position - 1,1 */
		/*----------------------------------------------*/

		ld->cursor_pos.row = row = 1;
		ld->cursor_pos.col = col = 1;
	}

	BUGLPR(dbg_midcrsr, BUGGID, ("row=%d, col=%d \n", row, col));

	/*-------------------------------------------------------*/
	/* If virtual terminal is active, translate presentation */
	/* space coordinates to screen corrdinates and update    */
	/* the frame buffer.                                     */
	/*-------------------------------------------------------*/

	if (ld->vtt_active)
	{ 
		PIO_EXC_ON();

		BUGLPR(dbg_midcrsr, BUGACT, 
			("Calling ps_to_screen_text and MID_MoveCursor\n"));

		ps_to_screen_text(row, col, &screen_x, &screen_y);
		MID_MoveCursor (screen_x, screen_y);

		PIO_EXC_OFF();
	}

	VDD_TRACE(MOVC, EXIT, vp);

        MID_DD_EXIT_TRACE ( midcrsr, 1, VTTMOVC, ddf,
                0,
                row,
                col,
                ld->vtt_active );


	BUGLPR(dbg_midcrsr, BUGGID, ("Leaving vttmovc with rc = 0.\n"));

	return(0);

}                                /* end  of  vttmovc             */


