static char sccsid[] = "@(#)58  1.9.3.8  src/bos/kernext/disp/ped/ksr/midinit.c, peddd, bos411, 9435D411a 8/31/94 15:30:16";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: vttinit
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
#include <sys/iocc.h>
#include <sys/proc.h>
#include <sys/xmem.h>
#include <fcntl.h>
#include <sys/aixfont.h>
#include <sys/syspest.h>
#include <sys/except.h>
#include "mid.h"
#include "midddf.h"
#include "midhwa.h"
#include "bidefs.h"
#include "midksr.h"
#include "hw_dd_model.h"        /* defines MID_DD                       */
#include "hw_macros.h"
#include "hw_PCBrms.h"
#include "hw_FIFrms.h"
#include "hw_FIF2dm1.h"
#include "hw_typdefs.h"
#include "hw_errno.h"
#include "hw_regs_u.h"
#include "hw_regs_k.h"

#include "mid_dd_trace.h"
MID_MODULE ( midinit );        /* defines trace variable                    */


BUGXDEF(dbg_middd);

/*-------------------------------------------------------------------*/
/*                                                                   */
/*   IDENTIFICATION: VTTINIT                                         */
/*                                                                   */
/*   DESCRIPTIVE NAME:  Initialize Virtual Display Driver (VDD)      */
/*                                                                   */
/*   FUNCTION: Initialize the internal state of the device driver:   */
/*                                                                   */
/*   PSEUDO-CODE:                                                    */
/*                                                                   */
/*   IF not initialized,                                             */
/*      - xmalloc local data area                                    */
/*      - initialize current font and zero out local data area       */
/*                                                                   */
/*   IF font is type AIXFONT and                                     */
/*        IF font is 1st good font found                             */
/*           - set font width and height                             */
/*           - set up pointer to font table entry                    */
/*        ELSE                                                       */
/*             IF width and height are same as 1st good font         */
/*                - set up pointer to font table entry for each font */
/*                                                                   */
/*   IF no good font was found (or none passed in)                   */
/*      - set all local data font pointers to default font           */
/*      - set default width and height                               */
/*      - set presentation space dimensions                          */
/*   ELSE                                                            */
/*      - set presentation space dimensions                          */
/*                                                                   */
/*   Calculate centering offset for presentation space               */
/*                                                                   */
/*   IF presentation space already allocated and                     */
/*      IF same size as previous                                     */
/*         THEN                                                      */
/*           - all OK / nop                                          */
/*         ELSE                                                      */
/*           - free up old space and xmalloc and zero out new space  */
/*                                                                   */
/*   Set default foreground and background colors                    */
/*                                                                   */
/*   Initialize each word in the Presentation space to a blank with  */
/*   a normal attribute.                                             */
/*                                                                   */
/*   Set character box values based on font size                     */
/*                                                                   */
/*   Set cursor colors                                               */
/*                                                                   */
/*   Initialize color table                                          */
/*                                                                   */
/*   Set vttmode                                                     */
/*                                                                   */
/*   IF virtual terminal is active                                   */
/*      - update frame buffer                                        */
/*      - initialize cursor                                          */
/*                                                                   */
/*   Return                                                          */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*  INPUTS:    Virtual terminal structure pointer                    */
/*             Pointer to list of font ids                           */
/*             Pointer to Presentation space size structure          */
/*                                                                   */
/*  OUTPUTS:   INVALID_VIRTUAL_TERMINAL_ID                           */
/*             INVALID_FONT_ID                                       */
/*             INVALID_FONT_SIZE                                     */
/*             NO_USABLE_FONT                                        */
/*                                                                   */
/*  CALLED BY: Mode Processor                                        */
/*                                                                   */
/*  CALLS:     querym to get addressability to dds                   */
/*             Select_new_font to set default font                   */
/*             vttdefc to define the cursor                          */
/*                                                                   */
/*-------------------------------------------------------------------*/
long vttinit(vp, font_ids, ps_s)
struct vtmstruc *vp;                /* virtual terminal struct ptr   */
struct fontpal  *font_ids;          /* real font ids                 */
struct ps_s     *ps_s;              /* presentation space size       */
{
	ulong addr;
	int     init_rc;

	long                    ps_bytes_new,
	i,
	j,
	found,
	font_index;
	long                    font_ary[4];    /* TEMP SINGLE FONT SUPPORT */
	struct font_data        *font_table;
	struct aixFontInfoPtr   *aixfontptr;    /* temp ptr to char data    */

	struct phys_displays    *pd = vp->display;
	struct middata          *ld;            /* ptr to local data area   */
	struct midddf           *ddf= (struct midddf *)vp->display->free_area;

	HWPDDFSetup;                            /* gain access to ped H/W   */

	BUGLPR(dbg_midinit, BUGNTA, ("Entering vttinit\n"));


	MID_DD_ENTRY_TRACE ( midinit, 1, VTTINIT, ddf,
	    0,
	    vp,
	    font_ids,
	    ps_s );


	/*---------------------------------*/
	/* initialize the local data area  */
	/*---------------------------------*/

	if ((ld = (struct middata *) vp->vttld) == NULL)
	{
	        BUGLPR(dbg_midinit, BUGNTA,
	            ("ld Malloc(%d)\n",sizeof(struct middata)));
	        ld = (struct middata *)
	            xmalloc(sizeof(struct middata),3,pinned_heap);

	        vp->vttld = (char *) ld;


	        if (ld == NULL)
	        /*-----------------------------*/
	        /* if the malloc failed        */
	        /*-----------------------------*/
	        {
	                BUGLPR(dbg_midinit, 0, ("malloc failed.\n"));
	                return(-1);
	        }

	        BUGLPR(dbg_midinit, BUGNTA,
	                ("Malloc succeeded, ld = 0x%x.\n",ld));

	        SET_ATTRIFONT(ld->current_attr,0);
	        bzero(ld, sizeof(struct middata));
	        BUGLPR(dbg_midinit, BUGNTA, ("zeroed ld.\n"));
	        vp->display->usage++;
	        ld->pse = NULL;
	}
#if 0
	} else
#endif
	{
	        /*---------------------------------*/
	        /* initialize the adapter card     */
	        /*---------------------------------*/

	        init_rc = mid_init(vp->display);
	        if (init_rc)
	        {
	                init_rc = mid_init(vp->display);
	                if (init_rc)
	                {
	                ddf->hwconfig |= MID_BAD_UCODE;
	                BUGLPR(dbg_middd, 1,
	                        ("Cannot initialize adapter.\n"));
	                }
	                else ddf->hwconfig &= ~MID_BAD_UCODE;
	        }
	        else ddf->hwconfig &= ~MID_BAD_UCODE;

	}

        if (! init_rc)
        {
        	mid_init_rcx( pd );
        }

	/*--------------------------*/
	/* Validate the font list.  */
	/*--------------------------*/

	found = FALSE;
	font_table = vp->fonts;


	for (i = 0; i < FONT_ID_SIZE; i++)
	/*---------------------------------------------*/
	/* all font ids passed to this routine         */
	/* There will be at most 8 font ids passed in  */
	/*---------------------------------------------*/
	{
	        /*---------------------------------*/
	        /* set up index into font table    */
	        /*---------------------------------*/

	        font_index =
	                (vp->font_index >= 0) ? vp->font_index : 0;

	        if (font_table[font_index].font_style == 0)
	        /*-----------------------------*/
	        /* The font is of type AIXFONT */
	        /*-----------------------------*/
	        {
	                /*------------------------------*/
	                /* A usable font has been found */
	                /*------------------------------*/

	                if (found == FALSE)
	                /*------------------------------------*/
	                /*  this is the first good font found */
	                /*------------------------------------*/
	                {
	                        /*---------------------------*/
	                        /* set font width and height */
	                        /*---------------------------*/

	                        found = TRUE;
	                        BUGLPR(dbg_midinit, 2,
	                        ("usable font.\nStruct addr in R3:\n"));
	                        BUGLFUNCT(dbg_midinit,BUGNTA,0,
	                            brkpoint(&font_table[font_index]));
	                        ld->char_box.wd =
	                            font_table[font_index].font_width;
	                        ld->char_box.ht  =
	                            font_table[font_index].font_height;
	                        ld->char_box.lft_sd =
	                        MID_FONT_PTR->maxbounds.leftSideBearing;
	                        ld->char_box.ascnt =
	                            MID_FONT_PTR->maxbounds.ascent;

	                        BUGLPR(dbg_midinit,BUGNTA,
	                        ("char sizes in font wid %d ht %d\n",
	                            ld->char_box.wd,ld->char_box.ht));
	                        BUGLPR(dbg_midinit,BUGNTA,
	                    ("char prop's in font lft_sd %d ascnt %d\n",
	                    ld->char_box.lft_sd,ld->char_box.ascnt));

	                        /*-----------------------------------
	                        Error check: are these fields negative?
	                        Note: snf fonts can have a left side
	                        that is negative.
	                        ------------------------------------*/

	                        ASSERT(ld->char_box.wd    > 0 &&
	                            ld->char_box.ht    > 0 &&
	                            ld->char_box.ascnt > 0);

	                        /*-----------------------------------
	                        set all entries of the array of AIXfont
	                        pointers to the first successful font
	                        found.  This increases program
	                        robustness by insuring that any font
	                        index chosen will produce readable
	                        output on the screen.
	                        ------------------------------------*/

	                        for (j = 0;j < FONT_ID_SIZE;j++)
	                        {
	                           ld->AIXfontptr[j] = (aixFontInfoPtr)
	                              (font_table[font_index].font_ptr);
	                        }

	                }
	                else /* not the first successful font        */
	                {
	                        /*-----------------------------------
	                        check current width and height against
	                        first usable font's width and height
	                        Also check the font properties: since
	                        only one copy of the left side and
	                        ascent are kept, they must be the same
	                        across all same fonts.
	                        ------------------------------------*/

	                        if ((ld->char_box.wd ==
	                          font_table[font_index].font_width)
	                        && (ld->char_box.ht ==
	                          font_table[font_index].font_height)
	                        && (ld->char_box.lft_sd ==
	                        MID_FONT_PTR->maxbounds.leftSideBearing)
	                        && (ld->char_box.ascnt ==
	                          MID_FONT_PTR->maxbounds.ascent))
	                        {
	                            ld->AIXfontptr[i] = (aixFontInfoPtr)
	                                font_table[font_index].font_ptr;
	                        }
	                }
	        }       /* end of if font_style == 0 */
	}      /* end of for all font id's */


	ld->ps_size.wd = ps_s->ps_w = 80;
	ld->ps_size.ht = ps_s->ps_h = 25;

	BUGLPR(dbg_midinit, BUGNTA,
	("Presentation space sizes computed:  width %d height %d\n",
	    ld->ps_size.wd, ld->ps_size.ht));


	PIO_EXC_ON();

	BUGLPR(dbg_midinit, 2, ("font index %d\n", font_index));

	addr =  (ulong) font_table[font_index].font_ptr;

	BUGLPR(dbg_midinit,1,
	    ("-->>> call MID_SetActiveFont with addr=%x, modfied addr=%x\n",
	    addr, (0xf0000000 | addr) ));

	MID_SetActiveFont( (0xf0000000|addr), (0xf0000000|addr));


	/*--------------------------------------*/
	/* Flush defer buffer to insure that    */
	/* adapter is correct for KSR mode.     */
	/*--------------------------------------*/

	MID_DEFERBUF_FLUSH;


	PIO_EXC_OFF();


	ps_bytes_new = ((ld->ps_size.ht * ld->ps_size.wd) << 2);

	BUGLPR(dbg_midinit, BUGNFO,
	    ("ps_bytes_new = %d\n", ps_bytes_new));


	/*----------------------------------------------------*/
	/* use ps_size and the glass size to determine the    */
	/* centering offset                                   */
	/*----------------------------------------------------*/

	ld->centering_x_offset =
	    (((X_MAX + 1) - (ld->ps_size.wd * ld->char_box.wd)) >> 1);
	ld->centering_y_offset =
	    (((Y_MAX + 1) - (ld->ps_size.ht * ld->char_box.ht)) >> 1);

	BUGLPR(dbg_midinit, BUGNFO,
	        ("centering_x_offset %d centering_y_offset %d\n",
	        ld->centering_x_offset,ld->centering_y_offset));

	if (ld->pse && (ps_bytes_new == ld->ps_bytes))
	/*----------------------------------------------------*/
	/* Presentation space has already been allocated AND  */
	/* the new amount of space is equal to the previously */
	/* allocated space...                                 */
	/*----------------------------------------------------*/

	{
	        /* all ok */
	}
	else
	{

	        /*---------------------------------------------------------*/
	        /* Get a new allocation of memory for the presention space */
	        /*---------------------------------------------------------*/

	        if (ld->pse)
	        /*-----------------------------------*/
	        /* Space has already been allocated  */
	        /*-----------------------------------*/
	        {
	                /*------------------------------------------*/
	                /* Free up that previously allocated space. */
	                /*------------------------------------------*/

	                xmfree((char *)ld->pse,pinned_heap);

	        }


	        ld->scroll_offset = 0;
	        ld->ps_bytes = ps_bytes_new;
	        ld->ps_words = (ld->ps_bytes) >> 2;

	        BUGLPR(dbg_midinit, BUGNTA, ("PS Malloc(%d)\n",ps_bytes_new));

	        ld->pse = (ulong *)xmalloc(ps_bytes_new, 3, pinned_heap);


	        if (ld->pse == NULL)
	        /*---------------------------------*/
	        /* the malloc failed               */
	        /*---------------------------------*/
	        {
	                BUGLPR(dbg_midinit, 0, ("malloc failed.\n"));
	                return(-1);
	        }


	        bzero(ld->pse, ps_bytes_new);
	}


	/*----------------------------------------------------*/
	/*  zero out current character and OR in a blank      */
	/*----------------------------------------------------*/

	ld->current_attr &= 0xFF00FFFF;
	ld->current_attr |= 0x00200000;


	/*----------------------------------------------------*/
	/*  set the default foreground and background colors  */
	/*----------------------------------------------------*/

	SET_ATTRIBAKCOL(ld->current_attr, 0);
	SET_ATTRIFORECOL(ld->current_attr,2);

	BUGLPR(dbg_midinit, BUGNTA,
	    ("character is 0x%x \n", ATTRIASCII(ld->current_attr)));


	for (i = 0; i < ld->ps_words; i++)
	/*-------------------------------------------------------------------*/
	/* each word in the presentation space                               */
	/*-------------------------------------------------------------------*/

	/*-------------------------------------------------------------------*/
	/* set character code to "spaces" and the attribute code to "normal" */
	/*-------------------------------------------------------------------*/
	{
	        ld->pse[i] = ld->current_attr;
	}


	/*-------------------------------------------------------------------*/
	/* set the cursor colors and shape.                                  */
	/*-------------------------------------------------------------------*/

	ld->cursor_color.fg = 15;
	ld->cursor_color.bg = ATTRIBAKCOL(ld->current_attr);
	ld->cursor_select = 2;

	/*-------------------------------------------------------------------*/
	/* Initialize the color table for this virtual terminal.             */
	/*-------------------------------------------------------------------*/

	ld->color_table.numcolors = pd->display_info.colors_active;

	for (i = 0;i < ld->color_table.numcolors;i++)
	{
	        ld->color_table.rgbval[i] = pd->display_info.color_table[i];
	}

	/*--------------------------------*/
	/* set the vtt mode               */
	/*--------------------------------*/

	ld->vtt_mode = vp->display->display_mode = KSR_MODE;


	if (ld->vtt_active)
	/*--------------------------------*/
	/* the virtual terminal is active */
	/*--------------------------------*/
	{

	        PIO_EXC_ON();

	        /*--------------------------------*/
	        /* update the frame buffer        */
	        /*--------------------------------*/

	        copy_ps(vp);

	        PIO_EXC_OFF();

	        /*--------------------------------*/
	        /* initialize the cursor          */
	        /*--------------------------------*/

	        ld->cursor_pos.row = 0;
	        ld->cursor_pos.col = 0;

	        vttdefc(vp,ld->cursor_select,TRUE);

	}

	MID_DD_EXIT_TRACE ( midinit, 1, VTTINIT, ddf,
	    0,
	    ld->current_attr,
	    ld->cursor_color.fg,
	    ld->cursor_color.bg );


	BUGLPR(dbg_midinit, BUGNTA, ("Leaving vttinit with rc = 0.\n"));
	return(0);

}

