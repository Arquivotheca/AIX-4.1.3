static char sccsid[] = "@(#)20  1.17.3.11  src/bos/kernext/disp/gem/rcm/gem_vdd.c, sysxdispgem, bos41B, 9506A 1/18/95 12:42:48";
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		bldglyphbuf
 *		cexpd_blt
 *		chkcurs
 *		draw_txt
 *		fifocpy
 *		gem_cardslots
 *		gem_init
 *		gem_wait
 *		gemlog
 *		getport
 *		itof
 *		load_ucode
 *		memcpy
 *		new_cursor
 *		ps_scr
 *		soft_reset
 *		upd_cursor
 *		upd_ps
 *		update_ps
 *		vttact
 *		vttcfl
 *		vttclr
 *		vttcpl
 *		vttdact
 *		vttdefc
 *		vttinit
 *		vttmovc
 *		vttrds
 *		vttscr
 *		vttsetm
 *		vttstct
 *		vttterm
 *		vtttext
 *		wrfifo
 *		
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

#include <sys/errno.h>
#include "gemincl.h"  
#define Bool       unsigned

ulong itof(uint);
ERR_REC(256) ER;

/*************************************************************************/
/*   IDENTIFICATION:  VTTINIT                                            */
/*   DESCRIPTIVE name:  Initialize Gemini  Virtual Display               */
/*                      Driver (VDD)                                     */
/*                                                                       */
/*   FUNCTION: Initialize the internal state of the device driver:       */
/*                                                                       */
/*               - if an invalid virtual terminal id is passed to        */
/*                 this procedure, the presentation space (PS)           */
/*                 width is set to -1                                    */
/*                                                                       */
/*               - If the routine was passed a font list:                */
/*                   - For all of the fonts passed in (normally 8)       */
/*                     validate the fonts by:                            */
/*                       Check to see that the Resource Manager has the  */
/*                       font id.                                        */
/*                       Validate it as being a font the adapter can use */
/*                   - If the fonts are ok then copy index, id and       */
/*                     address into the Vttenv table.                    */
/*                   - If any of the fonts are invalid set corresponding */
/*                     bit in invalid font parameter and set the         */
/*                     presentation space to height and width = 0 and    */
/*                     return INVALID_FONT_ID                            */
/*                                                                       */
/*                   - Now check to see if all of the fonts are the same */
/*                     size. If any are different size then set the      */
/*                     corresponding bit in the invalid font size parm,  */
/*                     and return INVALID_FONT_SIZE.                     */
/*                                                                       */
/*                 Otherwise no font list was passed so:                 */
/*                                                                       */
/*                   - Try to select a default font using the following  */
/*                     criteria:                                         */
/*                    - Would have 80x25 Presentation space with plain   */
/*                      attributes.                                      */
/*                    - If none found, use any 80x25 Pspace font         */
/*                    - If none found, use any valid font                */
/*                    - If no valid font found return NO_USABLE_FONT     */
/*                                                                       */
/*                   - Set all entries in the Vttenv font structure      */
/*                     to the default font.                              */
/*                                                                       */
/*               - the PS is initialized with space characters           */
/*                 with normal video attributes                          */
/*                                                                       */
/*               - the real display screen is cleared if the             */
/*                 if the virtual terminal is active                     */
/*                                                                       */
/*               - the KSR color palette is initialized                  */
/*                 to their default values.                              */
/*                                                                       */
/*               - the cursor is moved to the upper-left corner          */
/*                 of the PS (and displayed on the real screen           */
/*                 if the virtual terminal is active)                    */
/*                                                                       */
/*               - the presentation space (PS) size is calculated        */
/*                 as follows:                                           */
/*                     height = height of the real screen divided        */
/*                              by the height of a character box         */
/*                              (integer division).                      */
/*                     width  = width of the real screen divide by       */
/*                              the width of the character box           */
/*                              (integer division)                       */
/*                                                                       */
/*************************************************************************/
vttinit(vp, font_ids, ps_s)
struct vtmstruc *vp;
struct fontpal  *font_ids;                   /* real font ids            */
struct ps_s *ps_s;                           /* Presentation Space size  */

{
	long  ps_bytes_new, found,
	      font_id, font_w,
	      font_h, font_sz;
	ulong *geo_addrs;

	int    i, j, k, rc, pswords, temp;
	char  *font_addr;
	aixFontInfoPtr finfop;
	Pse    *psp;
	struct gemini_data *ld;
	struct phys_displays *pd;
	struct gem_dds *gem_ddsp;
	struct vtt_box_rc_parms sp;   /* ul and lr corners of rectangle*/

	 struct colorpal lpal;


	/****************************************************************/
	/* default char (4 bytes) used to initialize the Presentation   */
	/* Space -  blank char / fg color = green  / bg color = black   */
	/****************************************************************/
#ifdef GEM_DBUG
	  static unsigned int default_ch_attr = 0x00202100;
#else
	  static unsigned int default_ch_attr = 0x00202000;
#endif GEM_DBUG

#ifdef GEM_DBUG
 printf("vttinit: entered\n");
 printf("vp = %08x  font_ids = %08x ps_s = %08x\n",vp,font_ids,ps_s);
#endif GEM_DBUG

	/******************************************************************/
	/* Get pointer to the Physical Display data structure which is in */
	/* the Virtual Terminal data structure(vtmstruc).                 */
	/******************************************************************/
	pd = vp->display;

	/***************************************************************/
	/* If this is the first time that init is entered for this     */
	/* instance of the GEMINI VDD, then allocate storage for a     */
	/* local data area.                                            */
	/***************************************************************/
	if (vp->vttld == NULL)
	{
		/*********************************************************/
		/* Allocate memory for local data area                   */
		/*********************************************************/
		ld = (struct gemini_data *)xmalloc(sizeof(struct gemini_data),
							     3,pinned_heap);

		/***********************************************************/
		/* If the malloc failed, then put an entry in the error log*/
		/***********************************************************/
		if (ld == NULL)
		{
		     gemlog(vp,NULL,"vttinit","xmalloc",NULL,NULL,UNIQUE_1);
		     return(ENOMEM);
		}

		/**********************************************************/
		/* Clear the storage to zeros. Save the pointer to the    */
		/* local structure in the vtmstruc structure, set the     */
		/* presentation space pointer to NULL, initialize         */
		/* geographical address, set SHP installed flag, and      */
		/* initialize color table.                                */
		/**********************************************************/
		pin(ld,sizeof(struct gemini_data));
		bzero(ld,sizeof(struct gemini_data));
		vp->vttld = (char *) ld;
		vp->display->usage++;
		ld->Vttenv.adp_id = 0x8FFD;
		ld->Vttenv.ps = NULL;
		ld->Vttenv.cursor_pixmap = NULL;
		ld->ipl_shp_fp = &(ld->iplshp_flgs);
		ld->gcardslots = &(ld->gm_crdslots);
		geo_addrs = pd->interrupt_data.intr_args[1];

		pd->interrupt_data.intr_args[2] = ld;

		ld->gcardslots->magic = *geo_addrs++;
		ld->gcardslots->drp = *geo_addrs++;
		ld->gcardslots->gcp = *geo_addrs++;
		ld->gcardslots->shp = *geo_addrs++;
		ld->gcardslots->imp = *geo_addrs++;
		ld->gcardslots->gcp_start = *geo_addrs++;
		if (ld->gcardslots->shp != 0)
		    ld->ipl_shp_fp->shp_flg = 1;
		ld->component = (char *) pd->odmdds;

		/************************************************************/
		/* color table is loaded from DDS to the local color table  */
		/************************************************************/
		ld->ctbl.num_entries = 16;
		for (i=0; i < 16; i++)
		       ld->ctbl.colors[i] = ((struct gem_dds *)
					   pd->odmdds)->color_table[i];

		/************************************************************/
		/* Initialize Device Dependent RCX                          */
		/************************************************************/
		rc = gem_init_rcx(pd, ld);
		if (rc != 0)
		   return(rc);
	}

	/******************************************************************/
	/* Get pointer to local data structure.                           */
	/******************************************************************/
	ld = (struct gemini_data *) vp->vttld;

	/***********************************************************/
	/* Initialize pointer to start of Character Info Data      */
	/* Structures. These structures contain indices to glyphs  */
	/***********************************************************/
	 i = (vp->font_index >= 0) ? vp->font_index : 0;
	 finfop = (aixFontInfoPtr) vp->fonts[i].font_ptr;
	 font_addr = (char *) vp->fonts[i].font_ptr;
	 font_id = vp->fonts[i].font_id;
	 font_w  = vp->fonts[i].font_width;
	 font_h  = vp->fonts[i].font_height;
	 font_sz = vp->fonts[i].font_size;
	 for (j = 0; j < MAX_FONTS; j++)
	 {
	     ld->Vttenv.font_table[j].index  = 0;
	     ld->Vttenv.font_table[j].id = vp->fonts[i].font_id;
	     ld->Vttenv.font_table[j].height = font_h;
	     ld->Vttenv.font_table[j].width  = font_w;
	     ld->Vttenv.font_table[j].size   = font_sz >> 2;
	     ld->Vttenv.font_table[j].fontptr = finfop;
	     ld->Vttenv.font_table[j].fontcindx =
					      (aixCharInfoPtr)(font_addr +
						(BYTESOFFONTINFO(finfop)));
	     ld->Vttenv.font_table[j].glyphptr =
					       (aixGlyphPtr) finfop +
					       ((BYTESOFFONTINFO(finfop)) +
					       (BYTESOFCHARINFO(finfop)));
	 }

	/*****************************************************************/
	/* Use default font                                              */
	/*****************************************************************/
	ld->Vttenv.ps_size.wd = ps_s->ps_w = 80;
	ld->Vttenv.ps_size.ht = ps_s->ps_h = 25;

	/*****************************************************************/
	/* Calculate the starting pixel position of a centered ps        */
	/*****************************************************************/
	ld->Vttenv.scr_pos.pel_x = ((X_RES - font_w *
					  ld->Vttenv.ps_size.wd) / 2) -1;
	ld->Vttenv.scr_pos.pel_y = ((Y_RES - font_h *
					  ld->Vttenv.ps_size.ht) / 2) -1;

	if (ld->Vttenv.scr_pos.pel_x < 0)
		ld->Vttenv.scr_pos.pel_x = 0;

	if (ld->Vttenv.scr_pos.pel_y < 0)
		ld->Vttenv.scr_pos.pel_y = 0;

	/******************************************************************/
	/* Set the character box values for cursor definition             */
	/******************************************************************/
	finfop = (aixFontInfoPtr) ld->Vttenv.font_table[0].fontptr;
	ld->Vttenv.char_box.height = finfop->minbounds.ascent +
						 finfop->minbounds.descent;
	ld->Vttenv.char_box.width = finfop->minbounds.characterWidth;

	/******************************************************************/
	/* Calculate storage for presentation space                       */
	/******************************************************************/
	ps_bytes_new = (long) (( ps_s->ps_w * ps_s->ps_h ) << 2);

	/******************************************************************/
	/*           Allocate the Presentation Space                      */
	/******************************************************************/

	/******************************************************************/
	/* If Space has already been allocated, then free it              */
	/******************************************************************/
	if (ld->Vttenv.ps)
		  xmfree((char *)ld->Vttenv.ps,pinned_heap);

	/******************************************************************/
	/* Get a new allocation of memory for the presention space        */
	/******************************************************************/
	ld->Vttenv.scroll = 0;
	ld->Vttenv.ps_bytes = ps_bytes_new;
	ld->Vttenv.ps_words = (ld->Vttenv.ps_bytes) >> 2;
	ld->Vttenv.ps = (Pse *)xmalloc(ps_bytes_new,3,pinned_heap);

	/******************************************************************/
	/* Log error if memory allocation failed                          */
	/******************************************************************/
	if (ld->Vttenv.ps == NULL)
	{
		gemlog(vp,NULL,"vttinit","xmalloc",NULL,NULL,UNIQUE_2);
		errno = ENOMEM;
		return(ENOMEM);
	}

	/******************************************************************/
	/* Free storage If Space has already been allocated for an        */
	/* expansion buffer                                               */
	/******************************************************************/
	if (ld->Vttenv.expbuf)
		    xmfree(ld->Vttenv.expbuf,pinned_heap);

	/******************************************************************/
	/* Get a new allocation of memory for the expansion buffer        */
	/******************************************************************/
	ld->Vttenv.expbuf_len = (ld->Vttenv.char_box.height *
		     ld->Vttenv.char_box.width) * ld->Vttenv.ps_size.wd;

	ld->Vttenv.expbuf = (char *) xmalloc(
				     ld->Vttenv.expbuf_len,3,pinned_heap);

	/******************************************************************/
	/* Log error if memory allocation failed                          */
	/******************************************************************/
	if (ld->Vttenv.expbuf == NULL)
	{
		gemlog(vp,NULL,"vttinit","xmalloc",NULL,NULL,UNIQUE_3);
		errno = ENOMEM;
		return(ENOMEM);
	}

	/******************************************************************/
	/* If Space has already been allocated for a glyph buffer, then   */
	/* free it                                                        */
	/******************************************************************/
	if (ld->Vttenv.glyphbuf)
		   xmfree(ld->Vttenv.glyphbuf,pinned_heap);

	if (ld->Vttenv.char_box.width <= BITS_IN_BYTE)
		   ld->Vttenv.bytes_in_glyph_row = 1;
	else
	     if (ld->Vttenv.char_box.width % BITS_IN_BYTE)
		    ld->Vttenv.bytes_in_glyph_row =
			   (ld->Vttenv.char_box.width/BITS_IN_BYTE) + 1;
	     else
		     ld->Vttenv.bytes_in_glyph_row =
			      (ld->Vttenv.char_box.width/BITS_IN_BYTE);


	    ld->Vttenv.bytes_in_glyph = ld->Vttenv.bytes_in_glyph_row *
					   ld->Vttenv.char_box.height;

	/******************************************************************/
	/* Calculate glyph buffer size and get a buffer to hold the glyphs*/
	/* and save the address of this buffer                            */
	/******************************************************************/
	ld->Vttenv.glyphbuf_len = ld->Vttenv.bytes_in_glyph *
						  ld->Vttenv.ps_size.wd;
	ld->Vttenv.glyphbuf = (char *) xmalloc(
			       (ld->Vttenv.glyphbuf_len),3,pinned_heap);

	if (ld->Vttenv.glyphbuf == NULL)
	{
		gemlog(vp,NULL,"vttinit","xmalloc",NULL,NULL,UNIQUE_4);
		errno = ENOMEM;
		return(ENOMEM);
	}

	/******************************************************************/
	/* Initialize the Presentation space. Set each entry to 'space'   */
	/*  and the attribute code to fg color 1                          */
	/******************************************************************/
	psp = ld->Vttenv.ps;
	pswords = ld->Vttenv.ps_words;
	for (i = 0; i < pswords; i++,psp++)
	      psp->ps_fw = default_ch_attr;

	/******************************************************************/
	/* Set the cursor to its default position (upper left).           */
	/******************************************************************/
	ld->Vttenv.cursor_pos.col = 1;
	ld->Vttenv.cursor_pos.row = 1;

	/******************************************************************/
	/* Set up the default cursor colors and attributes                */
	/******************************************************************/
	ld->Vttenv.cursor_color.fg = 15;
	ld->Vttenv.cursor_color.bg = 0;

	/******************************************************************/
	/* If the VT is active then it must be a font list change. Clear  */
	/* the screen, else just initialize the VDD                       */
	/******************************************************************/
	if (ld->Vttenv.flg.active)
	{
		ld->Vttenv.flg.cur_vis = FALSE;
		gem_ddsp = (struct gem_dds *) pd->odmdds;
		soft_reset(ld,gem_ddsp->io_bus_mem_start,gem_ddsp->features);

		gem_init(ld);

		sp.row_ul    = 1;
		sp.column_ul = 1;
		sp.row_lr    = ld->Vttenv.ps_size.ht;
		sp.column_lr = ld->Vttenv.ps_size.wd;
		vttclr(vp, &sp, ld->Vttenv.current_attr, 0);

	}
	else
	  {
		ld->Vttenv.flg.cur_vis = TRUE;
		ld->Vttenv.flg.cur_blank = NON_BLANK;
		ld->Vttenv.cursor_pixmap = NULL;
		ld->Vttenv.vtt_mode = KSR_MODE;
		ld->Vttenv.current_attr = 0x2000;
		ld->Vttenv.cursor_select = 2;
	  }

	vttdefc(vp,(uchar)ld->Vttenv.cursor_select,
					   ld->Vttenv.flg.cur_vis);

#ifdef GEM_DBUG
 printf("vttinit: exited\n");
#endif GEM_DBUG
	return(SUCCESS);

}

/********************************************************************/
/*                                                                  */
/* IDENTIFICATION: VTTACT                                           */
/*                                                                  */
/* DESCRIPTIVE Name:  Activate an inactive virtual terminal,        */
/*                    or re-initialize the SLIH as part of          */
/*                    changing the mode.                            */
/*                                                                  */
/* FUNCTION:                                                        */
/*           Set the VDD state to active, reset the adapter and     */
/*           put it in a default state.                             */
/*                                                                  */
/* INPUTS:   Pointer to Virtual Terminal data structure             */
/*                                                                  */
/* OUTPUTS:                                                         */
/*                                                                  */
/* CALLED BY:                                                       */
/*                                                                  */
/* CALLS:                                                           */
/*            soft_reset                                            */
/*            ps_scr                                                */
/*                                                                  */
/********************************************************************/
long vttact(vp)
struct vtmstruc *vp;
{

	/* Set Window Attributes - full screen 8 bits/pixel    */
	static uint se_data1[4] = {SE_SWATLN,0x00000002,0x0,0x05000400};
	uint seg_reg, i, wait_cnt ;
	int parityrc;
	volatile ulong *fifo_cnt0, *fifo_cnt1;
	volatile unsigned char val;
	unsigned char *delay_ptr;
	label_t jmpbuf;

	struct gemini_data *ld;
	struct phys_displays *pd;
	struct gem_dds *gem_ddsp;
	uint retry = 0;

	ld = (struct gemini_data *) vp->vttld;

#ifdef GEM_DBUG
printf("vttact: entered\n");
printf("   ld ptr = %08x\n",ld);
#endif GEM_DBUG

	/***************************************************************/
	/* Set the VDD state to activated                              */
	/***************************************************************/
	ld->Vttenv.flg.active = ACTIVE;
	pd = vp->display;
	gem_ddsp = (struct gem_dds *) pd->odmdds;

	/***************************************************************/
	/* If the current state of MOM or KSR, reset the adapter       */
	/***************************************************************/
	switch (ld->Vttenv.vtt_mode)
	{
	case GRAPHICS_MODE:
		/********************************************************/
		/* Reset adapter to a known state.                      */
		/********************************************************/
		while (retry < 3)
		{
		     if (soft_reset(ld,gem_ddsp->io_bus_mem_start,
						   gem_ddsp->features))
			 retry++;
		     else
			 break;
		}
		if (retry >= 3)
		   return(SUCCESS);

		/********************************************************/
		/* Initialize adapter                                   */
		/********************************************************/
		gem_init(ld);

		/********************************************************/
		/* Wait for FIFO's to go empty. return if adapter hangs */
		/********************************************************/
		if (parityrc = (setjmpx(&jmpbuf)))
		{
		    if (parityrc == EXCEPT_IO)
		    {
		      gemlog(vp,NULL,"vttact","setjmpx",parityrc,
							  NULL,UNIQUE_1);
		      errno = EIO;
		      return(EIO);
		    }
		    else
		       longjmpx(parityrc);
		}

		wait_cnt = 0;
		seg_reg = BUSMEM_ATT(BUS_ID,0x00);
		fifo_cnt0 = (((ulong)ld->fifo_cnt_reg[0]) | seg_reg);
		fifo_cnt1 = (((ulong)ld->fifo_cnt_reg[1]) | seg_reg);
		while (!((*fifo_cnt0 & IN_USE_MASK) == 0 &&
				  (*fifo_cnt1 & IN_USE_MASK) == 0))
	       {
		 delay_ptr = (unsigned char *) (seg_reg + DELAY_ADDR);
		 for (i = 0; i < 0x10000; i++)
		       val = *((uchar volatile *) delay_ptr);
		 wait_cnt++;
		 if (wait_cnt > 100)
		 {
		     clrjmpx(&jmpbuf);
		     BUSMEM_DET(seg_reg);
		     return(SUCCESS);
		 }
	       }
	       clrjmpx(&jmpbuf);
	       BUSMEM_DET(seg_reg);

	       /*******************************************************/
	       /* Call RCM routine to init adapter for its needs      */
	       /*******************************************************/
	       gem_iact(ld);
	       vp->display->visible_vt = vp;

	       break;

	case KSR_MODE:
		/*******************************************************/
		/* Reset the adapter to a base state and reset pointer */
		/* to active vt for interrupt handler.                 */
		/* Note: Soft_reset reloads the color table.           */
		/*******************************************************/
		while (retry < 3)
		{
		     if (soft_reset(ld,gem_ddsp->io_bus_mem_start,
						   gem_ddsp->features))
			 retry++;
		     else
			 break;
		}

		gem_init(ld);

		vp->display->visible_vt = vp;

		/********************************************************/
		/* Set Color Processing Mode to 8 bits/pixel            */
		/********************************************************/
		wrfifo(ImmFIFO,se_data1,sizeof(se_data1),ld);

		/********************************************************/
		/* copy the contents of the presentation space into the */
		/* frame buffer, establish the correct position and     */
		/* shape of the hardware cursor.                        */
		/********************************************************/
		ld->Vttenv.flg.attr_valid = 1;
		ld->Vttenv.current_attr = 0xffff;
		ld->draw_rect.ul_row = 1;
		ld->draw_rect.ul_col = 1;
		ld->draw_rect.lr_row = ld->Vttenv.ps_size.ht;
		ld->draw_rect.lr_col = ld->Vttenv.ps_size.wd;
		ld->draw_rect.attr_valid = 0;
		ld->draw_rect.attr = 0;

		if ((ps_scr(vp)) == SUCCESS)
		      vttdefc(vp,ld->Vttenv.cursor_select,
						ld->Vttenv.flg.cur_vis);
#if 0
	       /*******************************************************/
	       /* Call RCM routine to init adapter for its needs      */
	       /*******************************************************/
	       gem_iact(ld);
#endif
		break;

	default:
		/*********************************************************/
		/* invalid mode specified                                */
		/*********************************************************/
		gemlog(vp,NULL,"vttact","vttact",NULL,
					   INVALID_DISPLAY_MODE,UNIQUE_2);
		break;
	}


#ifdef GEM_DBUG
printf("vttact: exited\n");
#endif GEM_DBUG
	return(SUCCESS);
}

/*************************************************************************/
/*   IDENTIFICATION: VTTDEFC                                             */
/*   DESCRIPTIVE name:  Define Cursor for the Gemini  Display adapter    */
/*   FUNCTION: This command changes the shape of the megapel text cursor */
/*             to one of six predefined shapes. It also allows shapes to */
/*             be defined that consist of a set of consecutive character */
/*             box scan lines.                                           */
/*                                                                       */
/*************************************************************************/
long vttdefc(vp, selector, cursor_show)
struct vtmstruc *vp;
unsigned char   selector;                /* shape selector               */
unsigned int    cursor_show;             /* 0 ==> don't show the cursor  */
{

	short  top,  bottom;
	struct gemini_data *ld;

#ifdef GEM_DBUG
printf("vttdefc: entered\n");
#endif GEM_DBUG

	/****************************************************************/
	/* Return with error if the VDD is called and it is not in KSR  */
	/****************************************************************/
	ld = (struct gemini_data *) vp->vttld;
	if  (ld->Vttenv.vtt_mode != KSR_MODE)
		return(ERROR);

	/****************************************************************/
	/* Save cursor shape selector                                   */
	/****************************************************************/
	ld->Vttenv.cursor_select = selector;

	/***************************************************************/
	/* If the Virtual Terminal is active                           */
	/***************************************************************/
	if (ld->Vttenv.flg.active)
	{
		/********************************************************/
		/* If the current cursor is visible and not blank then  */
		/* ERASE the current cursor                             */
		/********************************************************/
		if ((ld->Vttenv.flg.cur_vis) && (!ld->Vttenv.flg.cur_blank))
			upd_cursor(vp);

		/********************************************************/
		/* Update environment structure with new cursor status  */
		/********************************************************/
		ld->Vttenv.flg.cur_vis = cursor_show;

		/********************************************************/
		/* If the new cursor is to be displayed then update the */
		/* environment structure with its new position          */
		/********************************************************/
		if (cursor_show)
		{
			ld->Vttenv.cursor_pos.col = vp->mparms.cursor.x;
			ld->Vttenv.cursor_pos.row = vp->mparms.cursor.y;
		}

		/********************************************************/
		/* Call subroutine to generate the new cursorshape      */
		/********************************************************/
		new_cursor(vp);

		/********************************************************/
		/* If the new cursor is visible and not blank then DRAW */
		/* it at its new location.                              */
		/********************************************************/
		if ((ld->Vttenv.flg.cur_vis) && (!ld->Vttenv.flg.cur_blank))
			upd_cursor(vp);

	}
	else
	{
		/********************************************************/
		/* The virtual terminal is not active, update the status*/
		/* of the cursor in the environment structure.          */
		/********************************************************/
		ld->Vttenv.flg.cur_vis = cursor_show;

		/********************************************************/
		/* If the cursor has been moved then update environment */
		/********************************************************/
		if (cursor_show)
		{
			ld->Vttenv.cursor_pos.col = vp->mparms.cursor.x;
			ld->Vttenv.cursor_pos.row = vp->mparms.cursor.y;
		}

		/********************************************************/
		/* Call subroutine to generate the new cursor           */
		/********************************************************/
		new_cursor(vp);
	}

#ifdef GEM_DBUG
printf("vttdefc: exited\n");
#endif GEM_DBUG

	return(SUCCESS);
}

/**************************************************************************/
/*   IDENTIFICATION: VTTMOVC                                              */
/*   DESCRIPTIVE name:  Move Cursor for the Gemini  Display Adapter       */
/*   FUNCTION: Moves the text cursor to the specified row and column      */
/*             position on the screen. Row and column are unity based.    */
/*                                                                        */
/**************************************************************************/
long vttmovc(vp)
struct vtmstruc *vp;
{

	struct gemini_data *ld;

#ifdef GEM_DBUG
printf("vttmovc: entered\n");
#endif GEM_DBUG

	/****************************************************************/
	/* If the virtual terminal is NOT in KSR mode, return error     */
	/****************************************************************/
	ld = (struct gemini_data *) vp->vttld;
	if (ld->Vttenv.vtt_mode != KSR_MODE)
	      return(ERROR);

	/****************************************************************/
	/* If the virtual terminal is active                            */
	/****************************************************************/
	if (ld->Vttenv.flg.active)
	{
		/********************************************************/
		/* If the cursor is currently visible and not blank then*/
		/* ERASE the cursor from its current location.          */
		/********************************************************/
		if ((ld->Vttenv.flg.cur_vis) && (!ld->Vttenv.flg.cur_blank))
			upd_cursor(vp);

		/********************************************************/
		/* Update the environment structure which tracks the    */
		/* current position of the cursor                       */
		/********************************************************/
		ld->Vttenv.cursor_pos.col = vp->mparms.cursor.x;
		ld->Vttenv.cursor_pos.row = vp->mparms.cursor.y;

		/********************************************************/
		/* If the cursor is not blank then DRAW the cursor at   */
		/* its new location                                     */
		/********************************************************/
		if (!ld->Vttenv.flg.cur_blank)
			upd_cursor(vp);
	}
	else
	{
		/********************************************************/
		/* Update the environment structure which tracks the    */
		/* current position of the cursor                       */
		/********************************************************/
		ld->Vttenv.cursor_pos.col = vp->mparms.cursor.x;
		ld->Vttenv.cursor_pos.row = vp->mparms.cursor.y;
	}

	/****************************************************************/
	/* Set cursor visible flag                                      */
	/****************************************************************/
	ld->Vttenv.flg.cur_vis = VISIBLE;

#ifdef GEM_DBUG
printf("vttmovc:  exited\n");
#endif GEM_DBUG

	return(SUCCESS);
}

/*************************************************************************/
/*   IDENTIFICATION: VTTTEXT                                             */
/*   DESCRIPTIVE name:  Draw Text for Gemini  Display Adapter            */
/*   FUNCTION: Draw a string of code base qualified ASCII characters into*/
/*             the refresh buffer of the Gemini  display adapter when a  */
/*             virtual terminal is active. If the terminal is inactive,  */
/*             characters are drawn into the presentation space that is  */
/*             managed by the Virtual Display Driver.                    */
/*                                                                       */
/*************************************************************************/
vtttext(vp, string, rc, cp, cursor_show)
struct vtmstruc *vp;
char          *string;                  /* array of ascii characters    */
struct        vtt_rc_parms *rc;         /* string position and length   */
struct        vtt_cp_parms *cp;         /* code point base/mask and     */
					/* new cursor position          */
unsigned int  cursor_show;              /* if true cursor is moved to   */
					/* the pos given in cp_parms    */
{
	int length;                     /* length of string to draw     */
	int indx;                        /* index into font table array  */
	char  temp_color;               /* temp area used for rev video */
	ushort temp_attr;
	Pse  *ps_addr;           /* presentation space address          */
	int  ps_offset;          /* presentation space offset           */
	int  last_char;          /* offset of last char to be displayed */
	int i,j;
	unsigned short  save_attr; /* original attribute save area     */
	struct gemini_data *ld;
	char txt_buf[512];

#ifdef GEM_DBUG
 printf("vtttext: entered\n");
 printf("   string ptr = %08x\n",string);
#endif GEM_DBUG

	/****************************************************************/
	/* If reverse video then exchange background and foreground     */
	/* color indexes                                                */
	/****************************************************************/
	ld = (struct gemini_data *) vp->vttld;
	save_attr = cp->attributes;
	temp_attr = save_attr;
	if (ATTRIRV(temp_attr))
		    temp_attr = ((save_attr & 0xf000) >> 4) |
				((save_attr & 0x0f00) << 4) |
				(save_attr & 0x00ff);

	cp->attributes = temp_attr;

	/****************************************************************/
	/*                 UPDATE PRESENTATION SPACE                    */
	/* Get start address of presentation space                      */
	/****************************************************************/
	ps_addr = (Pse *)ld->Vttenv.ps;

	/****************************************************************/
	/* Calculate the offset into the presentation space where first */
	/* character is to be copied. If the offset is greater than or  */
	/* equal to the presentation space size then adjust offset to be*/
	/* within presentation space.                                   */
	/****************************************************************/
	ps_offset = ld->Vttenv.scroll + ((rc->start_row - 1) *
	    ld->Vttenv.ps_size.wd) + (rc->start_column - 1);
	if (ps_offset >= ld->Vttenv.ps_words)
		ps_offset -= ld->Vttenv.ps_words;
	ps_addr += ps_offset;

	/****************************************************************/
	/* Calculate index into string and number of chars in string    */
	/* to move into presentation space.                             */
	/****************************************************************/
	i = rc->string_index - 1;
	last_char = i + rc->string_length - 1;

	/****************************************************************/
	/* While there are characters to transfer to the presentation   */
	/* space, get a character from the input string, 'and' code pag */
	/* mask to it, add the code page base then shift  result left   */
	/* sixteen and add the attributes. Store resulting full word    */
	/* in the presentation space, increment index and pointer. Copy */
	/* the character into the line buffer.                          */
	/****************************************************************/
	j = 0;
	while (i <= last_char)
	{
	    *(int *)ps_addr = ((string[i] & cp->cp_mask) << 16) + temp_attr;

	/*        *(int *)ps_addr = (((string[i] & cp->cp_mask) +
		    cp->cp_base) << 16) + temp_attr;                    */
		txt_buf[j] = string[i] & cp->cp_mask;
		j++;
		i++;
		ps_addr++;
	}

	/****************************************************************/
	/* If the virtual terminal is active then display the chars     */
	/* at the terminal                                              */
	/****************************************************************/
	if (ld->Vttenv.flg.active)
	{
		ld->Vttenv.flg.attr_valid = TRUE;
		draw_txt(vp,cp,rc,txt_buf,cursor_show);
	}
	else
	{
		/********************************************************/
		/* Update cursor state information                      */
		/********************************************************/
		ld->Vttenv.flg.cur_vis = cursor_show;

		/********************************************************/
		/* If the cursor was moved then update current cursor   */
		/* location                                             */
		/********************************************************/
		if (cursor_show)
		{
			ld->Vttenv.cursor_pos.col = vp->mparms.cursor.x;
			ld->Vttenv.cursor_pos.row = vp->mparms.cursor.y;
		}
	}
	cp->attributes = save_attr;               /* restore attributes*/

#ifdef GEM_DBUG
printf("vtttext: exited\n");
#endif GEM_DBUG

	return(SUCCESS);

}

/**************************************************************************/
/*   IDENTIFICATION: VTTSTCT                                              */
/*   DESCRIPTIVE name:  Set Color Table for the Gemini  Display Adapter   */
/*                      Virtual Display Driver (VDD)                      */
/*   FUNCTION : The color table structure is used to select the set of    */
/*              colors that can be displayed simultaneously. The values   */
/*              stored in this table are device dependent.                */
/*                                                                        */
/**************************************************************************/
long vttstct(vp, color_table)
struct vtmstruc *vp;
struct colorpal *color_table;
{
	int i, num_colors, parityrc;
	uint seg_reg;
	uint *adptr;
	uint se_data[3];                      /* SE fifo data           */
	struct gemini_data *ld;
	label_t jmpbuf;

#ifdef GEM_DBUG
printf("vttstct: entered\n");
#endif GEM_DBUG

	/****************************************************************/
	/* Return error if the VDD is not is KSR mode                   */
	/****************************************************************/
	ld = (struct gemini_data *) vp->vttld;
	if (ld->Vttenv.vtt_mode != KSR_MODE)
		return(-1);

	num_colors = color_table->numcolors;
	if (num_colors > 16)
	    num_colors = 16;

	for (i=0; i < num_colors; i++)
	   ld->ctbl.colors[i] = color_table->rgbval[i];

	if (ld->Vttenv.flg.active)
	{
		/*******************************************************/
		/* Put Color Table in adapter memory and send a Load   */
		/* Base Planes Color Table structure element           */
		/*******************************************************/
		if (parityrc = (setjmpx(&jmpbuf)))
		{
		    if (parityrc == EXCEPT_IO)
		    {
		      gemlog(vp,NULL,"vttsct","setjmpx",parityrc,
							 NULL,UNIQUE_1);
		      errno = EIO;
		      return(EIO);
		    }
		    else
		       longjmpx(parityrc);
		}

		seg_reg = BUSMEM_ATT(BUS_ID,0x00);
		adptr = (unsigned int *) ((char *)ld->a_gem_gmem + 0x4000
						       + (uint)seg_reg);
		for (i=0; i < num_colors; i++)
			*(adptr+i) = (uint) ld->ctbl.colors[i];
		for (i=num_colors; i < 256; i++)
			*(adptr+i) = (uint) ld->ctbl.colors[15];

		clrjmpx(&jmpbuf);
		BUSMEM_DET(seg_reg);
		se_data[0] = CE_LBPCLN;
		se_data[1] = 0x00;
		se_data[2] = GBL_LO_LIMIT + 0x4000;

		wrfifo(ImmFIFO, se_data, sizeof(se_data), ld);

	}

#ifdef GEM_DBUG
printf("vttstct: exited\n");
#endif GEM_DBUG

	return(SUCCESS);
}

/*************************************************************************/
/*   IDENTIFICATION: VTTCPL                                              */
/*   COPYRIGHT:                                                          */
/*   DESCRIPTIVE name:  Copy Line Segment for the Gemini  Adapter        */
/*   FUNCTION: Provides a horizontal scroll function by copying a portion*/
/*             of the specified line to another position on that same    */
/*             line. This operation is repeated for the number of        */
/*             consecutive lines requested.                              */
/*                                                                       */
/*************************************************************************/
vttcpl(vp, rc, cursor_show)
struct vtmstruc *vp;
struct vtt_rc_parms *rc;                /* string position and length   */
unsigned int        cursor_show;        /* if true cursor is moved to   */
					/* the pos given in *vp         */
{
	int len;
	struct ulpel_pos xy_ulpel;
	static uint se_data[11] = {CE_LOGOLN, 0x00,
				   CE_COPYLN, 0x00, 0x00, 0x00, 0x00,
					      0x00, 0x00, 0x00, 0x00};
	union {
	      uint   intval;
	      ushort shortval[2];
	      } tmp;

	struct gemini_data *ld;

#ifdef GEM_DBUG
printf("vttcpl: entered\n");
#endif GEM_DBUG

	/****************************************************************/
	/* Return error if the VDD is not is KSR mode                   */
	/****************************************************************/
	ld = (struct gemini_data *) vp->vttld;
	if (ld->Vttenv.vtt_mode != KSR_MODE)
		return(ERROR);

	/*****************************************************************/
	/* If the virtual terminal is active and cursor is visible and   */
	/* not blank then ERASE the cursor from its current position     */
	/*****************************************************************/
	if ((ld->Vttenv.flg.active) && (ld->Vttenv.flg.cur_vis) &&
					   (!ld->Vttenv.flg.cur_blank))
		upd_cursor(vp);

	/****************************************************************/
	/* If the destination address is not equal to the source address*/
	/* then copy the indicated line segments as requested           */
	/****************************************************************/
	if ((rc->dest_column) != (rc->start_column))
	{
		/********************************************************/
		/* Update the presentation space returns character len  */
		/********************************************************/
		len = update_ps(vp, rc, cursor_show);

		/********************************************************/
		/* If the virtual terminal is active then update display*/
		/********************************************************/
		if (ld->Vttenv.flg.active)
		{
		    /****************************************************/
		    /* Send SET LOGICAL OPERATION and VPM COPY to S.E.  */
		    /* logical operation must be REPLACE for VPM COPY.  */
		    /* No planes are masked for the VPM COPY.           */
		    /****************************************************/
		    GET_ULPEL_XY(rc->start_row,rc->start_column,xy_ulpel);
		    *(se_data+4) = itof(xy_ulpel.x_ul);
		    *(se_data+5) = itof(xy_ulpel.y_ul);
		    GET_ULPEL_XY(rc->start_row,rc->dest_column,xy_ulpel);
		    *(se_data+7) = itof(xy_ulpel.x_ul);
		    *(se_data+8) = itof(xy_ulpel.y_ul);

		    tmp.shortval[0] = (rc->dest_row-rc->start_row+1) *
					    ld->Vttenv.char_box.height;
		    tmp.shortval[1] = len * ld->Vttenv.char_box.width;
		    *(se_data+10) = tmp.intval;

		    wrfifo(ImmFIFO, se_data, sizeof(se_data),ld);

		}
	}

	/****************************************************************/
	/* Check if cursor is to be displayed                           */
	/****************************************************************/
	if (cursor_show)
	{
		/********************************************************/
		/* Update environment structure with new cursor location*/
		/* and DRAW the cursor if terminal active and cursor is */
		/* not blank                                            */
		/********************************************************/
		ld->Vttenv.cursor_pos.col = vp->mparms.cursor.x;
		ld->Vttenv.cursor_pos.row = vp->mparms.cursor.y;
		if ((ld->Vttenv.flg.active) && (!ld->Vttenv.flg.cur_blank))
		       upd_cursor(vp);
	}
	/****************************************************************/
	/* Update Cursor visable flag                                   */
	/****************************************************************/
	ld->Vttenv.flg.cur_vis = cursor_show;

#ifdef GEM_DBUG
printf("vttcpl: exited\n");
#endif GEM_DBUG
	return(SUCCESS);
}

/************************************************************************/
/* Update the presentation space (VTTCPL)                               */
/************************************************************************/
update_ps(vp, rc, cursor_show)
struct vtmstruc *vp;
struct vtt_rc_parms *rc;
unsigned int        cursor_show;
{
	int i, s_offset, d_offset, slength, num_lines;
	Pse *ps_addr, *d_addr;
	struct gemini_data *ld;

	ld = (struct gemini_data *) vp->vttld;

	/****************************************************************/
	/*                 UPDATE PRESENTATION SPACE                    */
	/* Get start address of presentation space                      */
	/****************************************************************/
	d_addr = ps_addr = (Pse *)ld->Vttenv.ps;

	/****************************************************************/
	/* Calculate the offset into the presentation space of the first*/
	/* character of the source and destination strings              */
	/****************************************************************/
	s_offset = ld->Vttenv.scroll + ((rc->start_row - 1) *
	    ld->Vttenv.ps_size.wd) + (rc->start_column - 1);

	d_offset = ld->Vttenv.scroll + ((rc->start_row - 1) *
	    ld->Vttenv.ps_size.wd) + (rc->dest_column - 1);

	/****************************************************************/
	/* If the source offset lies outside of the presentation space  */
	/* then adjust offsets                                          */
	/****************************************************************/
	if (s_offset >= ld->Vttenv.ps_words)
	{
		s_offset -= ld->Vttenv.ps_words;
		d_offset -= ld->Vttenv.ps_words;
	}

	/****************************************************************/
	/* Calculate the number of characters to be copied (taking into */
	/* account truncation when the destination is to the right of   */
	/* source. NOTE: truncation is not possible when the destination*/
	/* is to the left of the source because the minimum column is 1)*/
	/****************************************************************/
	slength = (ld->Vttenv.ps_size.wd - (rc->dest_column)) + 1;
	if (slength > (rc->string_length))
		slength = rc -> string_length;

	/****************************************************************/
	/* Use a local variable to count number of lines to be copied.  */
	/****************************************************************/
	num_lines = rc->dest_row - rc->start_row;

	/****************************************************************/
	/* If destination is to the left of the source then COPY LEFT   */
	/****************************************************************/
	if ((rc->dest_column) < (rc->start_column))
	{
		/********************************************************/
		/* While there are lines to copy                        */
		/********************************************************/
		while (num_lines >= 0)
		{
			/************************************************/
			/* Copy a presentation space entry one at a time*/
			/* from source to destination                   */
			/************************************************/
			for (i=0; i < slength; i++)
			{
				d_addr[d_offset] = ps_addr[s_offset];
				s_offset++;
				d_offset++;
			}

			/************************************************/
			/* Move offsets to next line                    */
			/************************************************/
			s_offset = (s_offset + ld->Vttenv.ps_size.wd)
								- slength;
			d_offset = (d_offset + ld->Vttenv.ps_size.wd)
								- slength;

			/************************************************/
			/* If the source offset lies outside of the     */
			/* presentation space then adjust offsets       */
			/************************************************/
			if (s_offset >= ld->Vttenv.ps_words)
			{
				s_offset -= ld->Vttenv.ps_words;
				d_offset -= ld->Vttenv.ps_words;
			}
			num_lines--;
		}               /* End of do while more lines to copy   */
	}
	else
	{
		/********************************************************/
		/* COPY RIGHT(destination is to the right of the source)*/
		/* Copy from right to left to avoid source and dest     */
		/* overlap problems. Move offsets to last character     */
		/********************************************************/
		d_offset += (slength -1);
		s_offset += (slength -1);

		/********************************************************/
		/* While there are lines to copy                        */
		/********************************************************/
		while (num_lines >= 0)
		{
			/************************************************/
			/* Copy a presentation space entry one at a time*/
			/* source to destination                        */
			/************************************************/
			for (i=0; i < slength; i++)
			{
				d_addr[d_offset] = ps_addr[s_offset];
				s_offset--;
				d_offset--;
			}

			/************************************************/
			/* Move offsets to next line                    */
			/************************************************/
			s_offset = (s_offset + ld->Vttenv.ps_size.wd)
							       + slength;
			d_offset = (d_offset + ld->Vttenv.ps_size.wd)
							       + slength;

			/************************************************/
			/* If the source offset lies outside of the     */
			/* presentation space then adjust offsets       */
			/************************************************/
			if (s_offset >= ld->Vttenv.ps_words)
			{
				s_offset -= ld->Vttenv.ps_words;
				d_offset -= ld->Vttenv.ps_words;
			}
			num_lines--;
		}                  /* End of do while more lines to copy*/
	}
	return(slength);
}

/*************************************************************************/
/*   IDENTIFICATION:  VTTCFL                                             */
/*   COPYRIGHT:                                                          */
/*   DESCRIPTIVE name:  Copy Full Lines for Gemini  Adapter              */
/*   FUNCTION: Copies a sequence of one or more full lines up or down    */
/*             a specified number of lines (clipping at the presentation */
/*             space boundaries). In addition, the cursor can optionally */
/*             be moved.                                                 */
/*                                                                       */
/*************************************************************************/
long vttcfl(vp,s_row, d_row, num_rows, cursor_show)
struct vtmstruc *vp;
int             s_row;                  /* source row                    */
int             d_row;                  /* destination row               */
int             num_rows;               /* number of rows to copy        */
unsigned int    cursor_show;            /* if true cursor is moved to    */
					/* the position given in the *vp */
{
	int  n_row;
	struct ulpel_pos xy_ulpel;
	static uint se_data[11] = {CE_LOGOLN, 0x00,
				   CE_COPYLN, 0x00, 0x00, 0x00, 0x00,
					      0x00, 0x00, 0x00, 0x00};
	union {
	      uint   intval;
	      ushort shortval[2];
	      } tmp;


	struct gemini_data *ld;

#ifdef GEM_DBUG
printf("vttcfl: entered\n");
#endif GEM_DBUG

	/****************************************************************/
	/* Return error if the VDD is not is KSR mode                   */
	/****************************************************************/
	ld = (struct gemini_data *) vp->vttld;
	if (ld->Vttenv.vtt_mode != KSR_MODE)
		return(ERROR);

	/*****************************************************************/
	/* If the virtual terminal is active and cursor is visible and   */
	/* not blank then ERASE the cursor from its current position     */
	/*****************************************************************/
	if ((ld->Vttenv.flg.active) && (ld->Vttenv.flg.cur_vis) &&
					   (!ld->Vttenv.flg.cur_blank))
		upd_cursor(vp);

	/****************************************************************/
	/* Check that source and destinations are unequal               */
	/****************************************************************/
	if (s_row != d_row)
	{
		/********************************************************/
		/* Update the presentation space                        */
		/********************************************************/
		n_row = upd_ps(vp, s_row, d_row,num_rows);

		/********************************************************/
		/* If the virtual terminal is active then update the    */
		/* display                                              */
		/********************************************************/
		if (ld->Vttenv.flg.active)
		{
		    /********************************************************/
		    /* Send SET LOGICAL OPERATION and VPM COPY to S.E. FIFO.*/
		    /* logical operation must be REPLACE for VPM COPY.      */
		    /* No planes are masked for the VPM COPY.               */
		    /********************************************************/
		    GET_ULPEL_XY(s_row, 1, xy_ulpel);
		    *(se_data+4) = itof(xy_ulpel.x_ul);
		    *(se_data+5) = itof(xy_ulpel.y_ul);
		    GET_ULPEL_XY(d_row, 1, xy_ulpel);
		    *(se_data+7) = itof(xy_ulpel.x_ul);
		    *(se_data+8) = itof(xy_ulpel.y_ul);

		    tmp.shortval[0] = n_row * ld->Vttenv.char_box.height;
		    tmp.shortval[1] = ld->Vttenv.ps_size.wd *
					       ld->Vttenv.char_box.width;
		    *(se_data+10) = tmp.intval;

		    wrfifo(ImmFIFO, se_data, sizeof(se_data), ld);

		}
	}

	/****************************************************************/
	/* Update the cursor state information                          */
	/****************************************************************/
	ld->Vttenv.flg.cur_vis = cursor_show;
	if (cursor_show)
	{
		/********************************************************/
		/* Update environment structure with new cursor location*/
		/* and DRAW the cursor if terminal active and cursor is */
		/* not blank                                            */
		/********************************************************/
		ld->Vttenv.cursor_pos.col = vp->mparms.cursor.x;
		ld->Vttenv.cursor_pos.row = vp->mparms.cursor.y;
		if ((ld->Vttenv.flg.active) && (!ld->Vttenv.flg.cur_blank))
		       upd_cursor(vp);
	}

#ifdef GEM_DBUG
printf("vttcfl: exited\n");
#endif GEM_DBUG
	return(SUCCESS);
}

/************************************************************************/
/*Update Presentation Space (VTTCFL)                                    */
/************************************************************************/
upd_ps(vp,s_row, d_row, num_rows)
struct vtmstruc *vp;
int             s_row;
int             d_row;
int             num_rows;

{
	int  s_offset, d_offset, i, j;
	Pse *ps_addr;
	struct gemini_data *ld;

	ld = (struct gemini_data *) vp->vttld;

	/****************************************************************/
	/* Get address of presentation space                            */
	/****************************************************************/
	ps_addr = (Pse *)ld->Vttenv.ps;

	/****************************************************************/
	/* Calculate the starting offset of the first character to move */
	/* in the source string and the first character offset of the   */
	/* destination                                                  */
	/****************************************************************/
	s_offset = ld->Vttenv.scroll + ((s_row -1) * ld->Vttenv.ps_size.wd);
	d_offset = ld->Vttenv.scroll + ((d_row -1) * ld->Vttenv.ps_size.wd);

	/****************************************************************/
	/* If the source or destination offset is greater than or equal */
	/* to size of presentation space then adjust                    */
	/****************************************************************/
	if (s_offset >= ld->Vttenv.ps_words)
		s_offset -= ld->Vttenv.ps_words;

	if (d_offset >= ld->Vttenv.ps_words)
		d_offset -= ld->Vttenv.ps_words;

	/****************************************************************/
	/* If the source row is greater than the destination row then   */
	/* COPY UP                                                      */
	/****************************************************************/
	if (s_row > d_row)
	{
		/********************************************************/
		/* For each row that must be copied move the source row */
		/* to the destination row in the presentation space.    */
		/********************************************************/
		for (i=0; i<num_rows; i++)
		{
			for (j=0; j < ld->Vttenv.ps_size.wd; j++)
			{
				ps_addr[d_offset] = ps_addr[s_offset];
				s_offset++;
				d_offset++;
			}

			/************************************************/
			/* If source or destination offset is greater   */
			/* that or equal to presentation space adjust   */
			/************************************************/
			if (s_offset >= ld->Vttenv.ps_words)
				s_offset = 0;

			if (d_offset >= ld->Vttenv.ps_words)
				d_offset = 0;
		}
	}
	else
	/****************************************************************/
	/* COPY DOWN                                                    */
	/****************************************************************/
	{
		/********************************************************/
		/* Clip any rows that go beyond the end of the          */
		/* presentation space.                                  */
		/********************************************************/
		if (num_rows > (ld->Vttenv.ps_size.ht + 1 - d_row))
			num_rows = ld->Vttenv.ps_size.ht + 1 - d_row;

		/********************************************************/
		/* Initialize source and destination offsets to point   */
		/* to the last character in the last row of the source  */
		/* and destination.                                     */
		/********************************************************/
		s_offset += (ld->Vttenv.ps_size.wd * num_rows) - 1;
		d_offset += (ld->Vttenv.ps_size.wd * num_rows) - 1;

		/********************************************************/
		/* If source or destination offset is greater than or   */
		/* equal to presentation space size then adjust         */
		/********************************************************/
		if (s_offset >= ld->Vttenv.ps_words)
			s_offset -= ld->Vttenv.ps_words;

		if (d_offset >= ld->Vttenv.ps_words)
			d_offset -= ld->Vttenv.ps_words;

		/********************************************************/
		/* For each row that must be copied copy the row        */
		/********************************************************/
		for (i=0; i<num_rows; i++)
		{
			for (j=0; j < ld->Vttenv.ps_size.wd; j++)
			{
				ps_addr[d_offset] = ps_addr[s_offset];
				s_offset--;
				d_offset--;
			}
			/************************************************/
			/* If source or destination offset is beyond    */
			/* presentation space adjust                    */
			/************************************************/
			if (s_offset < 0)
				s_offset = ld->Vttenv.ps_words - 1;

			if (d_offset < 0)
				d_offset = ld->Vttenv.ps_words - 1;
		}
	}
	return(num_rows);
}

/*************************************************************************/
/*   IDENTIFICATION: VTTRDS                                              */
/*   DESCRIPTIVE name:  Read Screen Segment (Gemini )                    */
/*   FUNCTION: Read the entire or partial content of the                 */
/*             presentation space and convert each entry into a          */
/*             two-byte display code and a two-byte attribute code.      */
/*                                                                       */
/*************************************************************************/
long vttrds(vp,ds,ds_size,attr,attr_size,rc)
struct vtmstruc *vp;
unsigned short *ds;                     /* array of display symbols     */
					/* returned by this procedure   */
int    ds_size;                         /* NOT USED. IGNORED.           */
unsigned short *attr;                   /* array of attributes returned */
					/* by this procedure            */
int    attr_size;                       /* NOT USED. IGNORED.           */
struct vtt_rc_parms *rc;                /* string position and length   */
					/* not using string index,dest  */
{

	int             i;
	int             buf_offset;      /* full character (4byte) offset*/
	Pse            *rds_start;       /* real start adr of rds string */
	struct gemini_data *ld;

#ifdef GEM_DBUG
printf("vttrds: entered\n");
#endif GEM_DBUG

	/****************************************************************/
	/* Return error if the VDD is not is KSR mode                   */
	/****************************************************************/
	ld = (struct gemini_data *) vp->vttld;
	if (ld->Vttenv.vtt_mode != KSR_MODE)
		return(ERROR);

	/****************************************************************/
	/* Calculate the Presentation Space offset of the first         */
	/* character that must be read.(offset is in 4-byte characters) */
	/****************************************************************/
	buf_offset =  (((rc -> start_row) - 1) * ld->Vttenv.ps_size.wd)
	    + ((rc -> start_column) - 1) + ld->Vttenv.scroll;

	/****************************************************************/
	/* If ps offset is larger that ps size, adjust the offset       */
	/****************************************************************/
	if (buf_offset >= ld->Vttenv.ps_words)
		buf_offset -= ld->Vttenv.ps_words;

	rds_start = ld->Vttenv.ps + buf_offset;

	/****************************************************************/
	/* Move full character into two separate arrays: one for the    */
	/* 2 byte character, and one for the 2 byte attributes.         */
	/****************************************************************/
	for ( i = 0; i < rc->string_length; i++ )
	{
		*ds++   = rds_start[i].ps_entry.ps_char;
		*attr++ = rds_start[i].ps_entry.ps_attr;
		buf_offset++;
		if (buf_offset >= ld->Vttenv.ps_words)
		{
			buf_offset -= ld->Vttenv.ps_words;
			rds_start -= ld->Vttenv.ps_words;
		}
	}
#ifdef GEM_DBUG
printf("vttrds: exited\n");
#endif GEM_DBUG

       return(SUCCESS);
}

/*************************************************************************/
/*   IDENTIFICATION: VTTSCR                                              */
/*   DESCRIPTIVE name:  Scroll                                           */
/*   FUNCTION: Scrolls the entire contents of the Gemini  ps             */
/*             (and screen if the Virtual Terminal is active )           */
/*             up or down a specified number of lines.                   */
/*             When scrolling the data up, the lines at the top          */
/*             are discarded and the new lines at the bottom are         */
/*             cleared to spaces. When scrolling                         */
/*             the data down, the lines at the bottom                    */
/*             are discarded and the new lines at the top are            */
/*             cleared to spaces.                                        */
/*                                                                       */
/*************************************************************************/
long vttscr(vp, lines, attr, cursor_show)
struct vtmstruc *vp;
int             lines;                  /* number of lines to scroll    */
					/*   >0 ==> scroll data up      */
					/*   <0 ==> scroll data down    */
unsigned int    attr;                   /* attribute associated with all*/
					/* characters of the new lines  */
					/* that appear at either the top*/
					/* or bottom of the screen      */
unsigned int    cursor_show;            /* if true cursor is moved to   */
					/* position given in *vp    s   */

{

	int     s_row;                   /* source row for copy lines    */
	int     s_lastrow;               /* last source row for BLT      */
	int     d_lastrow;               /* last dest   row for BLT      */
	int     no_rows;                 /* No. of rows to be BLTed      */
	struct ulpel_pos xy_ulpel;       /* upper left pel coords of a ch*/

	static uint se_data[11] = {CE_LOGOLN, 0x00,
				   CE_COPYLN, 0x00, 0x00, 0x00, 0x00,
					      0x00, 0x00, 0x00, 0x00};
	union {
	      uint   intval;
	      ushort shortval[2];
	      } tmp;

	struct vtt_box_rc_parms clr;       /* for call to clear rect       */

	struct gemini_data *ld;

#ifdef GEM_DBUG
printf("vttscr: entered\n");
#endif GEM_DBUG

	/****************************************************************/
	/* Return with error if the VDD is called and it is not in KSR  */
	/****************************************************************/
	ld = (struct gemini_data *) vp->vttld;
	if  (ld->Vttenv.vtt_mode != KSR_MODE)
		return(ERROR);

	/******************************************************************/
	/* If all data has been scrolled off screen, clear ps ( and screen*/
	/* if virtual terminal is active). Clear Rectangle does all this. */
	/******************************************************************/
	if ((lines >= ld->Vttenv.ps_size.ht) ||
	    (lines <= (-(ld->Vttenv.ps_size.ht))))
	{
		clr.row_ul    = 1;
		clr.column_ul = 1;
		clr.row_lr    = ld->Vttenv.ps_size.ht;
		clr.column_lr = ld->Vttenv.ps_size.wd;
		vttclr(vp, &clr, attr, cursor_show);
		return(SUCCESS);
	}

	/*****************************************************************/
	/* A partial scroll has been requested:                          */
	/*  - if ps active, blt rectangle text to correct position on scr*/
	/*  - call Clear Rectangle.                                      */
	/*  - scroll the ps.                                             */
	/* Clear Rectangle adds blank lines to either top or bottom of   */
	/* ps. NOTE: this routine updates the real display if the VT is  */
	/* active. It also calls Move Cursor.                            */
	/*****************************************************************/
	if (lines > 0)                     /* add blank lines to bottom  */
	{
		s_row = lines + 1;
		s_lastrow = ld->Vttenv.ps_size.ht;   /* bottom of screen */
		d_lastrow = ld->Vttenv.ps_size.ht - lines;
		clr.row_ul = ld->Vttenv.ps_size.ht - lines + 1;
		clr.column_ul = 1;
		clr.row_lr = ld->Vttenv.ps_size.ht ;
		clr.column_lr = ld->Vttenv.ps_size.wd ;
	}

	/****************************************************************/
	/* If the number of lines is < zero, then add blank lines at top*/
	/****************************************************************/
	else
	  {
		s_row = 1;
		s_lastrow = ld->Vttenv.ps_size.ht + lines;
		d_lastrow = ld->Vttenv.ps_size.ht;
		clr.row_ul = 1 ;
		clr.column_ul = 1 ;
		clr.row_lr = -lines ;
		clr.column_lr = ld->Vttenv.ps_size.wd ;
	  }

	/****************************************************************/
	/* If the VT is active, then BLT a rectangle on the screen      */
	/****************************************************************/
	if (ld->Vttenv.flg.active)
	{
		/*********************************************************/
		/* If the VT is active and cursor is is visible and not  */
		/* blank then ERASE the cursor from its current position.*/
		/* Set cursor visable flag off. It will be set correctly */
		/* by vttclr.                                            */
		/*********************************************************/
		if ((ld->Vttenv.flg.cur_vis) && (!ld->Vttenv.flg.cur_blank))
		{
			upd_cursor(vp);
			ld->Vttenv.flg.cur_vis = 0;
		}

		/*********************************************************/
		/* Send SET LOGICAL OPERATION and VPM COPY to S.E. FIFO. */
		/* logical operation must be REPLACE for VPM COPY.       */
		/* No planes are masked for the VPM COPY.                */
		/*********************************************************/
		GET_ULPEL_XY(s_row, 1, xy_ulpel);
		*(se_data+4) = itof(xy_ulpel.x_ul);
		*(se_data+5) = itof(xy_ulpel.y_ul);
		no_rows = s_lastrow - s_row + 1;
		GET_ULPEL_XY((d_lastrow-no_rows+1), 1, xy_ulpel);
		*(se_data+7) = itof(xy_ulpel.x_ul);
		*(se_data+8) = itof(xy_ulpel.y_ul);

		tmp.shortval[0] = no_rows * ld->Vttenv.char_box.height;

		tmp.shortval[1] = ld->Vttenv.char_box.width *
					       ld->Vttenv.ps_size.wd;
		*(se_data+10) = tmp.intval;

		wrfifo(ImmFIFO, se_data, sizeof(se_data), ld);

	}

	/******************************************************************/
	/* Update the 'scrool' value in the VDD environment structure     */
	/******************************************************************/
	ld->Vttenv.scroll += (lines * ld->Vttenv.ps_size.wd);
	if (((int)ld->Vttenv.scroll) >= ld->Vttenv.ps_words)
		ld->Vttenv.scroll -= ld->Vttenv.ps_words;
	else
	     if (((int)ld->Vttenv.scroll) < 0)
		ld->Vttenv.scroll += ld->Vttenv.ps_words;

	/******************************************************************/
	/* Call vttclr to clear a rectangular area on the screen and      */
	/* position the cursor if required.                               */
	/******************************************************************/
	vttclr(vp, &clr, attr, cursor_show);

#ifdef GEM_DBUG
printf("vttscr: exited\n");
#endif GEM_DBUG

	return(SUCCESS);
}

/*************************************************************************/
/*   IDENTIFICATION: VTTCLR                                              */
/*   DESCRIPTIVE name:  Clear Rectangle                                  */
/*   FUNCTION: Clear a specified upright rectangular area of the         */
/*             frame buffer (if the Virtual Terminal is active)          */
/*             and presentation space (always).                          */
/*             A space code is stored in each character position         */
/*             along with a specified attribute (e.g., normal,           */
/*             reverse video, underline, blink, etc.).                   */
/*                                                                       */
/*             This command uses the Fill Rectangle, Set Interior        */
/*             Style, and Set Interior Color Index FIFO primitives.      */
/*                                                                       */
/*************************************************************************/
long vttclr(vp, sp, attr, cursor_show)
struct vtmstruc *vp;
struct vtt_box_rc_parms *sp;           /* ul and lr corners of rectangle*/
unsigned int    attr;                  /* character attribute           */
ulong           cursor_show;           /* if TRUE, cursor is moved.     */
{
	char          temp_color;        /* temp area for rev video     */
	int           num_row;           /* num of rows in rectangle    */
	int           num_col;           /* num of chars per row in rect*/
	int           i,j;
	Pse           pse_entry;         /* character and attrs         */
	int           ps_begin;          /* word offset to ul of rect   */
	Pse           *psp;              /* ptr to ps (word ptr)        */
	ushort        temp_attr;

	uint se_data[14];                /* S.E. data area for FIFO rite */
	union {
	      uint   intval;
	      ushort shortval[2];
	      } tmp;



	struct ulpel_pos xy_ulpel;       /* upper left pel coord of a ch.*/
	struct gemini_data *ld;
	unsigned int blank = 0x00200000; /* for a blank character        */

#ifdef GEM_DBUG
printf("vttclr entered\n");
#endif GEM_DBUG

	/****************************************************************/
	/* Return with error if the VDD is called and it is not in KSR  */
	/****************************************************************/
	ld = (struct gemini_data *) vp->vttld;
	if  (ld->Vttenv.vtt_mode != KSR_MODE)
		return(ERROR);

	/****************************************************************/
	/* If reverse video then exchange background and foreground     */
	/* color indexes                                                */
	/****************************************************************/
	temp_attr = attr;
	if (ATTRIRV(temp_attr))
		    temp_attr = ((attr & 0xf000) >> 4) |
				((attr & 0x0f00) << 4) |
				(attr & 0x00ff);

	/****************************************************************/
	/* If the VDD is active, then build FIFO commands to clear rect */
	/****************************************************************/
	if (ld->Vttenv.flg.active)
	{
		/********************************************************/
		/* Erase the cursor form its current position           */
		/********************************************************/
		if (ld->Vttenv.flg.cur_vis & (!ld->Vttenv.flg.cur_blank))
			upd_cursor(vp);

		/********************************************************/
		/* Set Logical Operation to REPLACE                     */
		/********************************************************/
		se_data[0] = CE_LOGOLN;
		se_data[1] = 0x00;

		/********************************************************/
		/* Set Interior Style to SOLID                          */
		/********************************************************/
		se_data[2] = SE_ISTYLN;
		se_data[3] = SOLID;

		/********************************************************/
		/* Set Integer Color Index to BACKGROUND Color          */
		/********************************************************/
		se_data[4] = SE_INCILN;
		se_data[5] = ATTRIBAKCOL(temp_attr);

		/*********************************************************/
		/* *** Build an Interior Polygon Structure Element ***   */
		/* It takes 5 pointes to define a closed rectangle. Get  */
		/* upper left hand corner of the char boxes in the four  */
		/* corners of the rectangle to be cleared and modify     */
		/* them to get coordinates of the rectangle              */
		/*********************************************************/
		se_data[6] = CE_IPLGLN;
		se_data[7] = 0x00;
		se_data[8] = 24;
		GET_ULPEL_XY(sp->row_ul,sp->column_ul, xy_ulpel);
		tmp.shortval[0] = xy_ulpel.x_ul;
		tmp.shortval[1] = xy_ulpel.y_ul;
		se_data[9]  = tmp.intval;
		se_data[13] = tmp.intval;

		GET_ULPEL_XY(sp->row_ul,(sp->column_lr + 1), xy_ulpel);
		tmp.shortval[0] = xy_ulpel.x_ul -1;
		tmp.shortval[1] = xy_ulpel.y_ul;
		se_data[10] = tmp.intval;

		GET_ULPEL_XY((sp->row_lr +1),(sp->column_lr + 1),xy_ulpel);
		tmp.shortval[0] = xy_ulpel.x_ul -1;
		tmp.shortval[1] = xy_ulpel.y_ul + 1;
		se_data[11] = tmp.intval;

		GET_ULPEL_XY((sp->row_lr + 1),sp->column_ul, xy_ulpel);
		tmp.shortval[0] = xy_ulpel.x_ul;
		tmp.shortval[1] = xy_ulpel.y_ul + 1;
		se_data[12] = tmp.intval;

		/**********************************************************/
		/* Write the commands to the Immediate BLT FIFO           */
		/**********************************************************/
		wrfifo(ImmFIFO, se_data, sizeof(se_data), ld);
		ld->Vttenv.current_attr = temp_attr;
	}

	/****************************************************************/
	/* Clear presentation space rectangle (always):                 */
	/*                                                              */
	/* Build the 4 byte char/attr to put in the ps rectangle.       */
	/* Calculate begin offset of each line.                         */
	/* Adjust address modulo size of ps.                            */
	/* Calculate end offset of each line.                           */
	/* (ALL offsets are 4-byte word offsets)                        */
	/****************************************************************/
	pse_entry.ps_fw = (uint)(blank | temp_attr);
	psp = ld->Vttenv.ps;                  /* start of ps            */

	/****************************************************************/
	/* Get upper-left corner of rectangle                           */
	/****************************************************************/
	ps_begin = ld->Vttenv.scroll + (((sp->row_ul - 1) *
	    ld->Vttenv.ps_size.wd) + (sp->column_ul - 1));

	/****************************************************************/
	/* If addr larger than ps then wrap                             */
	/****************************************************************/
	if (ps_begin >= ld->Vttenv.ps_words)
		ps_begin -= ld->Vttenv.ps_words;

	/****************************************************************/
	/* Clear line by line in rectangle, moving full character       */
	/* (4 bytes) by full character.                                 */
	/****************************************************************/
	num_row = sp->row_lr - sp->row_ul + 1 ;       /* height of rect */
	num_col = sp->column_lr - sp->column_ul + 1 ; /* width of rect  */

	for ( i = 0; i < num_row; i++)
	{
		for ( j = 0; j < num_col; j++)
			*(psp + ps_begin++) = pse_entry;
		/**********************************************************/
		/* set up to clear next line get to ul of next row of rect*/
		/* check if adr outside of ps if so, adjust adr modulo    */
		/**********************************************************/
		ps_begin += (ld->Vttenv.ps_size.wd - num_col);

		if (ps_begin >= ld->Vttenv.ps_words)
			ps_begin -= ld->Vttenv.ps_words;
	}

	/*****************************************************************/
	/* Update cursor if required                                     */
	/*****************************************************************/
	ld->Vttenv.flg.cur_vis = cursor_show;
	if (cursor_show)
	{
		ld->Vttenv.cursor_pos.row = vp->mparms.cursor.y;
		ld->Vttenv.cursor_pos.col = vp->mparms.cursor.x;

		if (ld->Vttenv.flg.active && ld->Vttenv.flg.cur_vis &&
		    (!ld->Vttenv.flg.cur_blank))
			upd_cursor(vp);
	}
#ifdef GEM_DBUG
printf("vttclr exited\n");
#endif GEM_DBUG

	return(SUCCESS);
}

/**************************************************************************/
/*   IDENTIFICATION: VTTTERM                                              */
/*   DESCRIPTIVE name:  Terminate command for Gemini Display Adapter      */
/*                      Virtual Display Driver (VDD)                      */
/*   FUNCTION : Deallocates local data area.                              */
/*                                                                        */
/**************************************************************************/
long vttterm(vp)
struct vtmstruc *vp;
{
	struct gemini_data *ld;
	struct phys_displays *pd;

	ld = (struct gemini_data *) vp->vttld;

#ifdef GEM_DBUG
printf("vttterm: entered addr ld = %08x\n",ld);
#endif GEM_DBUG

	if (ld->Vttenv.flg.active != INACTIVE)
	   vttdact(vp);

	/****************************************************************/
	/* Free all storage held by this instance of the VDD            */
	/****************************************************************/
       if (ld->GemRCMPriv.shmem != NULL)
	    xmfree((char *)ld->GemRCMPriv.shmem, kernel_heap);
	ld->GemRCMPriv.shmem = NULL;

	if (ld->Vttenv.ps != NULL)
	    xmfree((char *)ld->Vttenv.ps, pinned_heap);
	ld->Vttenv.ps = NULL;

	if (ld->Vttenv.expbuf != NULL)
	    xmfree((char *)ld->Vttenv.expbuf, pinned_heap);
	ld->Vttenv.expbuf = NULL;

	if (ld->Vttenv.glyphbuf != NULL)
	    xmfree((char *)ld->Vttenv.glyphbuf, pinned_heap);
	ld->Vttenv.glyphbuf = NULL;

	if (ld->Vttenv.cursor_pixmap != NULL)
	    xmfree((char *)ld->Vttenv.cursor_pixmap, pinned_heap);
	ld->Vttenv.cursor_pixmap = NULL;

	/****************************************************************/
	/* Set local data area pointer to NULL and decrement usage count*/
	/****************************************************************/
	unpin(ld,sizeof(struct gemini_data));
	xmfree((char *)ld, pinned_heap);
	vp->vttld = NULL;
	pd = vp->display;
	pd->interrupt_data.intr_args[2] = NULL;

	if (vp->display->usage >= 0)
	       vp->display->usage -= 1;

#ifdef GEM_DBUG
printf("vttterm: exited\n");
#endif GEM_DBUG

	return(SUCCESS);
}

/************************************************************************/
/*   IDENTIFICATION: VTTSETM                                            */
/*   DESCRIPTIVE name:  Set Mode Command for the Gemini  Display        */
/*                      Adapter Virtual Display Driver (VDD)            */
/*   FUNCTION: This command sets the mode of the VDD to either          */
/*             character or monitored mode.                             */
/*                                                                      */
/*             If the VDD is active and the mode is character           */
/*             this command issues an Activate which does the           */
/*             following:                                               */
/*                - initializes the adapter (if it was previously       */
/*                  in monitored mode or never previously               */
/*                  initialized)                                        */
/*                - sets the VDD mode to character                      */
/*                - waits for the FIFO's to empty                       */
/*                - Calls VTTACT to initialize the adapter              */
/*                                                                      */
/*             If the VDD is active and the mode is monitored,          */
/*             this command:                                            */
/*                - sets the adapter mode to monitored                  */
/*                                                                      */
/************************************************************************/
long vttsetm(vp,mode)
struct vtmstruc *vp;
int     mode;
{
	struct gemini_data *ld;
	uint seg_reg, i, wait_cnt ;
	int parityrc;
	volatile ulong *fifo_cnt0, *fifo_cnt1;
	volatile unsigned char val;
	unsigned char *delay_ptr;
	label_t jmpbuf;

#ifdef GEM_DBUG
printf("vttsetm: entered\n");
#endif GEM_DBUG

	/***************************************************************/
	/* Return error if mode is not KSR or MONITOR                  */
	/***************************************************************/
	ld = (struct gemini_data *) vp->vttld;
	if ((mode != KSR_MODE) && (mode != MONITOR))
		return(ERROR);

	/***************************************************************/
	/* Set the VDD mode flag in the environment structure. If the  */
	/* VDD is active, then wait for FIFO's to go empty and call    */
	/* vttact.                                                     */
	/***************************************************************/
	ld->Vttenv.vtt_mode = mode;
	if (ld->Vttenv.flg.active)
	{
		if (parityrc = (setjmpx(&jmpbuf)))
		{
		    if (parityrc == EXCEPT_IO)
		    {
		      gemlog(vp,NULL,"vttsetm","setjmpx",parityrc,
							  NULL,UNIQUE_1);
		      errno = EIO;
		      return(EIO);
		    }
		    else
		       longjmpx(parityrc);
		}

		wait_cnt = 0;
		seg_reg = BUSMEM_ATT(BUS_ID,0x00);
		fifo_cnt0 = (((ulong)ld->fifo_cnt_reg[0]) | seg_reg);
		fifo_cnt1 = (((ulong)ld->fifo_cnt_reg[1]) | seg_reg);
		while (!((*fifo_cnt0 & IN_USE_MASK) == 0 &&
				  (*fifo_cnt1 & IN_USE_MASK) == 0))
	       {
		 delay_ptr = (unsigned char *) (seg_reg + DELAY_ADDR);
		 for (i = 0; i < 0x10000; i++)
		       val = *((uchar volatile *) delay_ptr);
		 wait_cnt++;
		 if (wait_cnt > 100)
		    break;
	       }

	       clrjmpx(&jmpbuf);
	       BUSMEM_DET(seg_reg);
	       vttact(vp);
	}

#ifdef GEM_DBUG
printf("vttsetm: exited\n");
#endif GEM_DBUG

	return(SUCCESS);
}

/*************************************************************************/
/*   IDENTIFICATION: VTTDACT                                             */
/*   DESCRIPTIVE name:  Deactivate Command for the Gemini  Display       */
/*                      adapter Virtual Display Driver (VDD)             */
/*   FUNCTION: This routine marks this instance of the VDD as inactive.  */
/*             Check that the FIFO's are empty before returning.         */
/*                                                                       */
/*************************************************************************/
long vttdact(vp)
struct vtmstruc *vp;
{
    uint seg_reg, i, wait_cnt;
    struct gemini_data *ld;
    uint parityrc;
    volatile ulong *fifo_cnt0, *fifo_cnt1;
    volatile unsigned char val;
    unsigned char *delay_ptr;
    label_t jmpbuf;

#ifdef GEM_DBUG
printf("vttdact: entered\n");
#endif GEM_DBUG

    /********************************************************************/
    /* If this instance of the VDD is in KSR mode, then wait for the    */
    /* immediate FIFOs to become empty. If in MONITOR mode, then simply */
    /* mark this VDD as inactive.                                       */
    /********************************************************************/
    ld = (struct gemini_data *) vp->vttld;
    if (ld->Vttenv.vtt_mode == KSR_MODE)
    {
	 if (parityrc = (setjmpx(&jmpbuf)))
	 {
	      if (parityrc == EXCEPT_IO)
	      {
		  gemlog(vp,NULL,"vttdact","setjmpx",parityrc,
						     NULL,UNIQUE_1);
		  errno = EIO;
		  return(EIO);
	      }
	      else
		 longjmpx(parityrc);
	 }

	wait_cnt = 0;
	seg_reg = BUSMEM_ATT(BUS_ID,0x00);
	fifo_cnt0 = (((ulong)ld->fifo_cnt_reg[0]) | seg_reg);
	fifo_cnt1 = (((ulong)ld->fifo_cnt_reg[1]) | seg_reg);
	while (!((*fifo_cnt0 & IN_USE_MASK) == 0 &&
				  (*fifo_cnt1 & IN_USE_MASK) == 0))
	{
	      delay_ptr = (unsigned char *) (seg_reg + DELAY_ADDR);
	      for (i = 0; i < 0x10000; i++)
		     val = *((uchar volatile *) delay_ptr);
	      wait_cnt++;
	      if (wait_cnt > 100)
		break;
	}

	clrjmpx(&jmpbuf);
	BUSMEM_DET(seg_reg);
    }
    ld->Vttenv.flg.active = INACTIVE;

#ifdef GEM_DBUG
printf("vttdact: exited\n");
#endif GEM_DBUG

    return(SUCCESS);
}

/************************************************************************/
/* PROCEDURE NAME:  CEXPD_BLT                                           */
/*                                                                      */
/* DESCRIPTION: This routine builds a buffer which contains a stream    */
/*              of color expanded bytes which can then be BLTed to the  */
/*              GEMINI adapter.                                         */
/*                                                                      */
/* INPUT: vp -  pointer to vtrm struct contaning Gemini_data struct     */
/*        string - A pointer to a character string                      */
/*        rp - pointer to struct containing coordinates of rect. to     */
/*             to be Blted to.                                          */
/*                                                                      */
/************************************************************************/
cexpd_blt(ld,string,rp)
struct gemini_data *ld;
char  *string;
struct vtt_rc_parms *rp;
{
	char *glyph_buf, *expbuf;
	aixGlyphPtr glyphp, temp_ptr;
	uint  i, j, k, rc;
	uint  bytes_in_glyph;
	uint  bytes_in_glyph_row;
	uint  glyph_ptr_incr;
	struct ulpel_pos xy_ulpel;
	uint  byte_cnt, shift_cnt;
	uint  bytes_in_scanline;
	uint  exp_buf_len;
	int   empty, bit_count;
	uint se_data[11];

	union {
		uint   intval;
		ushort shortval[2];
	} tmp;

	union {
		ushort   in_out;
		char     ch_buf[2];
	} wrk;

	/***************************************************************/
	/* Initialize local glyph buffer variables                     */
	/***************************************************************/
	glyph_ptr_incr = 0;
	glyph_buf = ld->Vttenv.glyphbuf;
	glyphp = ld->Vttenv.glyphbuf;
	bytes_in_glyph = ld->Vttenv.bytes_in_glyph;
	bytes_in_glyph_row = ld->Vttenv.bytes_in_glyph_row;
	bytes_in_scanline = (rp->string_length) * (ld->Vttenv.char_box.width);

	/***************************************************************/
	/* Call bldglyphbuf to put glyphs into the buffer              */
	/***************************************************************/
	bldglyphbuf(ld,string,rp->string_length,glyph_buf,bytes_in_glyph);

	/***************************************************************/
	/* Clear the 1 bit/pixel buffer to zeros                       */
	/***************************************************************/
	expbuf = ld->Vttenv.expbuf;
	exp_buf_len = (ld->Vttenv.char_box.height *
		       ld->Vttenv.char_box.width) * rp->string_length;
	exp_buf_len = (exp_buf_len + 0x03) & 0xFFFFFFFC;
	bzero(expbuf,exp_buf_len);

	/***************************************************************/
	/* Initialize local variables                                  */
	/***************************************************************/
	byte_cnt = 0;
	rc = 0;
	shift_cnt = BITS_IN_BYTE;
	wrk.in_out = 0;

	/***********************************************************/
	/* Loop for the height of a character(number of scanlines) */
	/***********************************************************/
	for (j=0; j < ld->Vttenv.char_box.height; j++)
	{
		/*****************************************************/
		/* Loop to process number of characters, one scanline*/
		/* at a time. This means that the glyph buffer is NOT*/
		/* processed serially.                               */
		/*****************************************************/
		for (i=0; i < rp->string_length; i++)
		{
			/**********************************************/
			/* Process the glyph buffer to produce an     */
			/* m X n matrix, where 'm' is the character   */
			/* height and 'n' is the character width times*/
			/* the number of characters in the buffer.    */
			/**********************************************/
			bit_count = ld->Vttenv.char_box.width;
			temp_ptr = glyphp;
			empty = 0;
			while (bit_count > 0)
			{
				/**************************************/
				/* If the input buffer is 'empty',    */
				/* then get a byte from glyph buffer. */
				/**************************************/
				if (empty <= 0)
				{
				    wrk.ch_buf[1] = *temp_ptr++;
				    empty = BITS_IN_BYTE;
				}

				/**************************************/
				/* Move bits from the input buffer to */
				/* the output buffer.                 */
				/**************************************/
				if (bit_count < shift_cnt)
				{
				   wrk.in_out = wrk.in_out << bit_count;
				   shift_cnt -= bit_count;
				   empty -= bit_count;
				   bit_count = 0;
				   wrk.ch_buf[1] &= 0x00;
				}
				else
				   {
				      wrk.in_out = wrk.in_out << shift_cnt;
				      bit_count -= shift_cnt;
				      empty -= shift_cnt;
				      shift_cnt = 0;
				   }

				/**************************************/
				/* If the output buffer is 'full',    */
				/* then write it to the BLT buffer.   */
				/**************************************/
				if (shift_cnt == 0)
				{
					*expbuf++ = wrk.ch_buf[0];
					wrk.ch_buf[0] &= 0x00;
					byte_cnt++;
					shift_cnt = BITS_IN_BYTE;
					if ((empty > 0) & (bit_count >= empty))
					{
					   wrk.in_out =
						    wrk.in_out << empty;
					   bit_count -= empty;
					   shift_cnt -= empty;
					   empty = 0;
					}
				}
			}
			glyphp += bytes_in_glyph;
		}

		/*********************************************************/
		/* If there are any bits left in the input buffer, them  */
		/* move them to the output buffer to complete scanline   */
		/*********************************************************/
		if (shift_cnt != BITS_IN_BYTE)
		{
			wrk.in_out = wrk.in_out << shift_cnt;
			*expbuf++ = wrk.ch_buf[0];
			byte_cnt++;
			shift_cnt = BITS_IN_BYTE;
		}

		/*********************************************************/
		/* Insure that a scanline is a multiple of 32 bits.      */
		/*********************************************************/
		for (k=0; k < byte_cnt % 4; k++)
		{
			   *expbuf++ = 0x00;
			   byte_cnt++;
		}

		glyph_ptr_incr += bytes_in_glyph_row;
		glyphp = glyph_buf + glyph_ptr_incr;
	}

	/***********************************************************/
	/* Put a Write Pixel structure into the Immediate FIFO     */
	/***********************************************************/
	se_data[0] = CE_LOGOLN;
	se_data[1] = 0x00;
	se_data[2] = CE_WRITLN;
	se_data[3] = 0x20;

	GET_ULPEL_XY(rp->start_row,rp->start_column, xy_ulpel);
	se_data[4] = itof(xy_ulpel.x_ul);
	se_data[5] = itof(xy_ulpel.y_ul);
	se_data[6] = 0x00;

	tmp.shortval[0] = ld->Vttenv.char_box.height;
	tmp.shortval[1] = ((rp->string_length * ld->Vttenv.char_box.width) +
	    31) & 0xFFFFFFE0;
	se_data[7] = tmp.intval;

	se_data[8] = (((bytes_in_scanline % 32) + 31) & 0xFFFFFFE0) -
	    (bytes_in_scanline % 32);
	se_data[9] = ATTRIFORECOL(ld->Vttenv.current_attr);
	se_data[10] = ATTRIBAKCOL(ld->Vttenv.current_attr);

	wrfifo(ImmFIFO, se_data, sizeof(se_data), ld);
	rc = wrfifo(ImmBLTFIFO, ld->Vttenv.expbuf, byte_cnt, ld);

	return(rc);
}

/********************************************************************/
/*                                                                  */
/* SUBROUTINE: bldglyphbuf                                          */
/*                                                                  */
/* DESCRIPTION: Access a font raster glyphs and move them into a    */
/*              buffer                                              */
/*                                                                  */
/********************************************************************/
bldglyphbuf(ld,chars,cnt,buff,glyph_size)
struct gemini_data *ld;
char *chars, *buff;
int cnt, glyph_size;
{

	aixCharInfoPtr indexptr;
	aixGlyphPtr glyphp;
	int i, j, offset;
	uint font_select;

	font_select = ATTRIFONT(ld->Vttenv.current_attr);
	/************************************************************/
	/* Loop until all characters have been processed.           */
	/************************************************************/
	for (i=0; i<cnt; i++)
	{
		/**********************************************************/
		/* Use character to increment pointer to aixCharInfo data */
		/* structure for this character. Test the 'exists' bit in */
		/* this structure to determine if glyph exists for this   */
		/* character or it is a value to be added to find glyph.  */
		/**********************************************************/
		indexptr = (ld->Vttenv.font_table[font_select].fontcindx)
		    + chars[i];
		offset = 0;
		while (!indexptr->exists)
		{
			offset += indexptr->byteOffset;
			i++;
			indexptr = (ld->Vttenv.font_table[font_select].fontcindx)
			    + chars[i];
		}

		/**********************************************************/
		/* Calculate pointer to glyph and move glyph into buffer  */
		/**********************************************************/
		glyphp = (ld->Vttenv.font_table[font_select].glyphptr) +
		    (indexptr->byteOffset + offset);
		for (j=0; j < glyph_size; j++)
			*buff++ = *glyphp++;
	}
}

/*******************************************************************/
/*  MODULE:  GEM_INIT                                              */
/*                                                                 */
/*  DESCRIPTION: This routine initializes the GEMINI adapter       */
/*               by sending Default Context and View Table, loading*/
/*               the VDD default color table, and clearing the     */
/*               screen.                                           */
/*                                                                 */
/*******************************************************************/
int gem_init(ld)
   struct gemini_data *ld;
{
    uint *dp;
    long i, j, k, len;
    uint gem_gmem;
    uint str_elem_buff[22];
    uint seg_reg, local_vmeoff;
    int parityrc;

    anno_text *txtPtr;
    dflt_asl *aslPtr;
    dflt_view_tbl *vtblPtr;
    dflt_rcm *rcmPtr;
    label_t jmpbuf;

    uint *temp;


#ifdef GEM_DBUG
    printf("gem_init: entered\n");
#endif GEM_DBUG

	/*******************************************************/
	/* Clear and initialize Annotation Text Areas          */
	/*******************************************************/
	 if (parityrc = (setjmpx(&jmpbuf)))
	 {
	      if (parityrc == EXCEPT_IO)
	      {
		  gemlog(NULL,ld->component,"gem_init","setjmpx",
					    parityrc,NULL,UNIQUE_1);
		  errno = EIO;
		  return(EIO);
	      }
	      else
		 longjmpx(parityrc);
	 }

	seg_reg = BUSMEM_ATT(BUS_ID,0x00);
	gem_gmem = (uint)((char *)ld->a_gem_gmem) + seg_reg;

	/*******************************************************/
	/* Clear Area reserved for use by Annotation Text,     */
	/* Default Asl and Default View Table.                 */
	/*******************************************************/
	local_vmeoff = GLOBAL_OFF + GCP_GVP_LEN + GBL_LO_LIMIT;
	dp = (uint *) ((char *)gem_gmem + GLOBAL_OFF + GCP_GVP_LEN);
	i = sizeof(struct anno_text) + sizeof(dflt_asl) +
		      4 + sizeof(dflt_view_tbl) + sizeof(dflt_rcm);
	bzero(dp,i);

	/*******************************************************/
	/* Initialize Pointers to Text Tables. This code       */
	/* assumes that the GCP BIF has been initialized with  */
	/* the address of the Annotation Text Language Lookup  */
	/* Table pointer.                                      */
	/*******************************************************/
	txtPtr = (struct anno_text *) dp;
	txtPtr->fnt_lookup_off1   = local_vmeoff | TEXT_TBL_LEN;
	txtPtr->fnt_lookup_off2   = local_vmeoff | TEXT_TBL_LEN;
	txtPtr->fnt_lookup_off3   = local_vmeoff | TEXT_TBL_LEN;
	txtPtr->fnt_lookup_off4   = local_vmeoff | TEXT_TBL_LEN;
	txtPtr->fnt_lookup_off5   = local_vmeoff | TEXT_TBL_LEN;
	txtPtr->fnt_lookup_off7   = local_vmeoff | TEXT_TBL_LEN;
	txtPtr->fnt_lookup_off8   = local_vmeoff | TEXT_TBL_LEN;
 /*
	txtPtr->fnt_lookup_off9   = local_vmeoff | TEXT_TBL_LEN;
  */
	txtPtr->fnt_lookup_kata = (char *) (local_vmeoff +
				TEXT_TBL_LEN + LOOKUP_TBL_LEN +
				LOC_TBL_RCD_LEN);
	txtPtr->fnt_lookup_csid10 = (char *) (local_vmeoff +
				TEXT_TBL_LEN + ((LOOKUP_TBL_LEN +
					LOC_TBL_RCD_LEN)*2));

	txtPtr->fnt_lookup_csid9 = (char *) (local_vmeoff +
				TEXT_TBL_LEN + ((LOOKUP_TBL_LEN +
					LOC_TBL_RCD_LEN)*3));

	txtPtr->fnt_locate_off = (char *)
	      ((local_vmeoff + TEXT_TBL_LEN + LOOKUP_TBL_LEN) |
				    LAST_LOC_ENTRY);
	txtPtr->kata_fnt_locate_off = (char *)
	 ((local_vmeoff + TEXT_TBL_LEN + (LOOKUP_TBL_LEN * 2) +
			   LOC_TBL_RCD_LEN) | LAST_LOC_ENTRY);
	txtPtr->csid10_fnt_locate_off = (char *)
	 (local_vmeoff + TEXT_TBL_LEN + (LOOKUP_TBL_LEN * 3) +
			(LOC_TBL_RCD_LEN * 2) | LAST_LOC_ENTRY);

	txtPtr->csid9_fnt_locate_off = (char *)
	 (local_vmeoff + TEXT_TBL_LEN + (LOOKUP_TBL_LEN * 4) +
			(LOC_TBL_RCD_LEN * 3) | LAST_LOC_ENTRY);


	/*******************************************************/
	/* Initialize the first Font Locator Record.           */
	/*******************************************************/
	txtPtr->fnt_loc_rcd1[0] = 0x00010000;
	txtPtr->fnt_loc_rcd1[1] = 0x0000003C;
	txtPtr->fnt_loc_rcd1[2] = 0x0046FFEA;
	txtPtr->fnt_loc_rcd1[3] = 0xFFE70019;
	txtPtr->fnt_loc_rcd1[4] = 0x00000000;
	txtPtr->fnt_loc_rcd1[5] = 0x00000000;
	txtPtr->fnt_loc_rcd1[6] = 0x00000011;

	/*******************************************************/
	/* Initialize KATAKANA Font Locator Record.            */
	/*******************************************************/
	txtPtr->kata_fnt_loc_rcd1[0] = 0x00010000;
	txtPtr->kata_fnt_loc_rcd1[1] = 0x0000003C;
	txtPtr->kata_fnt_loc_rcd1[2] = 0x0046FFEA;
	txtPtr->kata_fnt_loc_rcd1[3] = 0xFFE70019;
	txtPtr->kata_fnt_loc_rcd1[4] = 0x00000000;
	txtPtr->kata_fnt_loc_rcd1[5] = 0x00000000;
	txtPtr->kata_fnt_loc_rcd1[6] = 0x00000010;

	/*******************************************************/
	/* Initialize csid10 Font Locator Record.              */
	/*******************************************************/
	txtPtr->csid10_fnt_loc_rcd1[0] = 0x00010000;
	txtPtr->csid10_fnt_loc_rcd1[1] = 0x0000003C;
	txtPtr->csid10_fnt_loc_rcd1[2] = 0x0046FFEA;
	txtPtr->csid10_fnt_loc_rcd1[3] = 0xFFE70019;
	txtPtr->csid10_fnt_loc_rcd1[4] = 0x00000000;
	txtPtr->csid10_fnt_loc_rcd1[5] = 0x00000000;
	txtPtr->csid10_fnt_loc_rcd1[6] = 0x00000012;

	/*******************************************************/
	/* Initialize csid9 Font Locator Record.               */
	/*******************************************************/
	txtPtr->csid9_fnt_loc_rcd1[0] = 0x00010000;
	txtPtr->csid9_fnt_loc_rcd1[1] = 0x0000003C;
	txtPtr->csid9_fnt_loc_rcd1[2] = 0x0046FFEA;
	txtPtr->csid9_fnt_loc_rcd1[3] = 0xFFE70019;
	txtPtr->csid9_fnt_loc_rcd1[4] = 0x00000000;
	txtPtr->csid9_fnt_loc_rcd1[5] = 0x00000000;
	txtPtr->csid9_fnt_loc_rcd1[6] = 0x00000013;

	/*******************************************************/
	/* Create a Default Adapter State List in Global memory*/
	/*******************************************************/
	local_vmeoff = local_vmeoff + sizeof(struct anno_text);
	dp = (uint *) ((char *) dp + sizeof(struct anno_text));

	aslPtr = (struct dflt_asl *) dp;
	aslPtr->view_tbl_num = 0x01;
	aslPtr->view_tbl_off = local_vmeoff + sizeof(dflt_asl);
	aslPtr->display_mask     = 0xFFFFFFFF;
	aslPtr->line_width_ratio = 0x3F800000;
	aslPtr->poly_mark_size   = 0x41A00000;
	aslPtr->anno_text_ratio  = 0x41400000;
	aslPtr->rcm_mgt_off      = local_vmeoff + sizeof(dflt_asl) +
					4 + sizeof(dflt_view_tbl);
	aslPtr->geo_text_cull = 0x00000001;

	/*******************************************************/
	/* Initialize a one element array of View Table offsets*/
	/*******************************************************/
	local_vmeoff = local_vmeoff + sizeof(dflt_asl) + 4;
	dp = (uint *) ((char *) dp + sizeof(dflt_asl));
	*dp++ = local_vmeoff;

	/*******************************************************/
	/* Create a Default View Table                         */
	/*******************************************************/
	vtblPtr = (struct dflt_view_tbl *) dp;
	vtblPtr->index = 0x00000001;
	vtblPtr->flags = 0x000E0000;
	vtblPtr->trans_matrix[0] = 0x3F800000;
	vtblPtr->trans_matrix[5] = 0x3F800000;
	vtblPtr->trans_matrix[10] = 0x3F800000;
	vtblPtr->trans_matrix[15] = 0x3F800000;
	vtblPtr->p_ref_point[2]   = 0x3F800000;
	vtblPtr->true_view_port[1] = 0x449FE000;
	vtblPtr->true_view_port[3] = 0x447FC000;
	vtblPtr->true_win_to_view_ratio[0] = 0x3F800000;
	vtblPtr->true_win_to_view_ratio[1] = 0x3F800000;
	vtblPtr->true_win_to_view_ratio[2] = 0x3F800000;
	vtblPtr->true_inv_win_to_view_ratio[0] = 0x3F800000;
	vtblPtr->true_inv_win_to_view_ratio[1] = 0x3F800000;
	vtblPtr->true_inv_win_to_view_ratio[2] = 0x3F800000;
	vtblPtr->true_win_vc[1] = 0x449FE000;
	vtblPtr->true_win_vc[3] = 0x447FC000;
	vtblPtr->true_win_vc[5] = 0x3F800000;
	vtblPtr->z_clipp_bdy[1] = 0x3F800000;
	vtblPtr->shear_x = 0x441FE000;
	vtblPtr->shear_y = 0x43FFC000;
	vtblPtr->shear_z = 0xBF800000;

	/*******************************************************/
	/* Initialize Resource Management information          */
	/*******************************************************/
	dp = (uint *) ((char *) dp + sizeof(dflt_view_tbl));
	local_vmeoff = local_vmeoff + sizeof(dflt_view_tbl);
	rcmPtr = (struct dflt_rcm *) dp;
	rcmPtr->win_width = 0x449fe000;
	rcmPtr->win_height = 0x447fc000;
	rcmPtr->flags = 0x00000008;

	/*******************************************************/
	/* Copy Color Table into adapter memory                */
	/*******************************************************/
	dp = (uint *) ((char *) dp + sizeof(dflt_rcm));
	local_vmeoff = local_vmeoff + sizeof(dflt_rcm);
	for (i=0; i < ld->ctbl.num_entries; i++)
	     *(dp+i) = (unsigned int) ld->ctbl.colors[i];
	for (i= ld->ctbl.num_entries; i < 256; i++)
	    *(dp+i) = (unsigned int) ld->ctbl.colors[15];

	clrjmpx(&jmpbuf);
	BUSMEM_DET(seg_reg);

	/*******************************************************/
	/* Send Activate Context and Set Default Drawing State */
	/* structure elements to Immediate and traversal FIFOs */
	/*******************************************************/
	str_elem_buff[0] = CE_ACTCLN;
	str_elem_buff[1] = ((uint) aslPtr & 0x001FFFFF) | GBL_LO_LIMIT;

	wrfifo(ImmFIFO, str_elem_buff, 8, ld);
	wrfifo(TravFIFO, str_elem_buff, 8, ld);

	str_elem_buff[0] = CE_SDDSLN;
	wrfifo(ImmFIFO, str_elem_buff, 4, ld);
	wrfifo(TravFIFO, str_elem_buff, 4, ld);

	/*******************************************************/
	/* Load Base Planes Color Table                        */
	/*******************************************************/
	str_elem_buff[0] = CE_LBPCLN;
	str_elem_buff[1] = 0x00;
	str_elem_buff[2] = local_vmeoff;

	/*******************************************************/
	/* Send Select Buffer Control and Frame Buffer Control */
	/* structure elements to clear ALL Planes. Planes are  */
	/* cleared as follows: Window, Overlay and Base.       */
	/*******************************************************/
	/*******************************************************/
	/* Select Window Frame Buffer                          */
	/*******************************************************/
	str_elem_buff[3] = CE_SDFBLN;
	str_elem_buff[4] = 0x02;

	/*******************************************************/
	/* Clear Window Frame Buffer A                         */
	/*******************************************************/
	str_elem_buff[5] = CE_FCTLLN;
	str_elem_buff[6] = 0x10000000;

	/*******************************************************/
	/* Select Overlay Planes                               */
	/*******************************************************/
	str_elem_buff[7] = CE_SDFBLN;
	str_elem_buff[8] = 0x01;

	/*******************************************************/
	/* Clear Overlay Frame Buffer B                        */
	/*******************************************************/
	str_elem_buff[9] = CE_FCTLLN;
	str_elem_buff[10] = 0xD0000000;

	/*******************************************************/
	/* Clear Overlay Frame Buffer A                        */
	/*******************************************************/
	str_elem_buff[11] = CE_FCTLLN;
	str_elem_buff[12] = 0x10000000;

	/*******************************************************/
	/* Select Base Planes                                  */
	/*******************************************************/
	str_elem_buff[13] = CE_SDFBLN;
	str_elem_buff[14] = 0x00;

	/*******************************************************/
	/* Clear Base Frame Buffer B                           */
	/*******************************************************/
	str_elem_buff[15] = CE_FCTLLN;
	str_elem_buff[16] = 0xD0000000;

	/*******************************************************/
	/* Clear Base Frame Buffer A                           */
	/*******************************************************/
	str_elem_buff[17] = CE_FCTLLN;
#ifdef GEM_DBUG
	str_elem_buff[18] = 0x100A0A0A;  /* flood screen with color */
#else
	str_elem_buff[18] = 0x10000000;
#endif GEM_DBUG

	wrfifo(ImmFIFO,str_elem_buff,76,ld);

#ifdef GEM_DBUG
  printf("gem_init: exited\n");
#endif GEM_DBUG
	return(0);

}

/**************************************************************************/
/*   IDENTIFICATION: DRAW_TXT                                             */
/*   DESCRIPTIVE name:  Draws text to the display                         */
/*                                                                        */
/**************************************************************************/
draw_txt(vp,cp,rp,txt_buf,cursor_show)
struct vtmstruc *vp;
struct vtt_cp_parms *cp;
struct vtt_rc_parms *rp;
char * txt_buf[];
unsigned int cursor_show;
{
	union
		      {
		unsigned short attr_all;     /* 16 bits of attributes   */
		struct
						   {
			unsigned fg_col  :4; /* 4 bits of foregrnd color*/
			unsigned bg_col  :4; /* 4 bits of backgrnd color*/
			unsigned fnt_sel :3; /* font selector           */
			unsigned no_disp :1; /* non displayable char    */
			unsigned bright  :1; /* bright                  */
			unsigned blink   :1; /* blink                   */
			unsigned rev_vid :1; /* reverse video           */
			unsigned u_score :1; /* underscore              */
		} attr;
	} temp;

	uint rc;
	struct gemini_data *ld;

	ld = (struct gemini_data *) vp->vttld;
	rc = 0;
	ld->Vttenv.current_attr = cp->attributes;

	if (!ATTRIBLA(cp->attributes))
	     rc = cexpd_blt(ld,txt_buf,rp);
	chkcurs(vp,cursor_show,cp,rp);
	ld->Vttenv.flg.cur_vis = cursor_show;

	return(rc);

}

/*************************************************************************/
/*   IDENTIFICATION: CHKCURS                                             */
/*   DESCRIPTIVE name:  Check cursor.                                    */
/*   FUNCTION: Check to see if the cursor is overwritten by DTXT; if so, */
/*             just draw cursor at new location (if cursor_show is SHOW) */
/*             if not overwritten, erase it and then draw it.            */
/*                                                                       */
/*************************************************************************/
chkcurs(vp,cursor_show,cp,rc)
struct vtmstruc *vp;
uint cursor_show;
struct vtt_cp_parms *cp;
struct vtt_rc_parms *rc;
{
	struct gemini_data *ld;

	ld = (struct gemini_data *) vp->vttld;
	/****************************************************************/
	/* Update the cursor if necessary                               */
	/****************************************************************/
	if (cursor_show)
	{
		/********************************************************/
		/* If the current cursor is visible and not blank       */
		/********************************************************/
		if ((ld->Vttenv.flg.cur_vis) && (!ld->Vttenv.flg.cur_blank))
		{
			/************************************************/
			/* Check if the cursor was overwritten by text  */
			/************************************************/
			if ((ld->Vttenv.cursor_pos.col < rc->start_column) ||
			    ((rc->start_column + rc->string_length -1) <
			      ld->Vttenv.cursor_pos.col) ||
			    (ld->Vttenv.cursor_pos.row < rc->start_row) ||
			    (rc->start_row < ld->Vttenv.cursor_pos.row))
			{
				/****************************************/
				/* The cursor was not overwritten, Call */
				/* CVTTMOVC to erase the cursor from its*/
				/* current location and move it to  new */
				/* location if cursor location changed  */
				/****************************************/
				if ((ld->Vttenv.cursor_pos.row !=
					       vp->mparms.cursor.y) ||
				    (ld->Vttenv.cursor_pos.col !=
						  vp->mparms.cursor.x))

					vttmovc(vp);

			}
			else
			{
				/****************************************/
				/* The cursor shape was overwritten. We */
				/* know that cursor is visible and not  */
				/* blank so update environment structure*/
				/* and DRAW cursor at new location      */
				/****************************************/
				ld->Vttenv.cursor_pos.col =
						       vp->mparms.cursor.x;
				ld->Vttenv.cursor_pos.row =
						       vp->mparms.cursor.y;
				upd_cursor(vp);
			}
		}
		else
		{
			/************************************************/
			/* The cursor shape was not originally visible  */
			/* so just redraw the cursor shape if not blank */
			/************************************************/

			if (!ld->Vttenv.flg.cur_blank)
			{
				ld->Vttenv.cursor_pos.col =
						       vp->mparms.cursor.x;
				ld->Vttenv.cursor_pos.row =
						       vp->mparms.cursor.y;
				upd_cursor(vp);
			}
		}
	}
	else
	/***************************************************************/
	/* Cursor is a NOSHOW. If current cursor shape is visible and  */
	/* cursor shape was not overwritten and the shape is not blank */
	/* then ERASE the cursor from its current location             */
	/***************************************************************/
	{
		if ((ld->Vttenv.flg.cur_vis) && (!ld->Vttenv.flg.cur_blank) &&
		    ((ld->Vttenv.cursor_pos.col < rc->start_column) ||
			    ((rc->start_column + rc->string_length -1) <
			      ld->Vttenv.cursor_pos.col) ||
		    (ld->Vttenv.cursor_pos.row < rc->start_row) ||
		    (rc->start_row < ld->Vttenv.cursor_pos.row)))
			  upd_cursor(vp);
	}

	ld->Vttenv.flg.cur_vis = cursor_show;
	return(SUCCESS);
}

/********************************************************************/
/* IDENTIFICATION: MEMCPY                                           */
/*                                                                  */
/* DESCRIPTIVE Name:  This routine copies data from a source to a   */
/*                    target buffer using the specified length.     */
/*                    Integer writes are done until the count length*/
/*                    become zero or less less than 4.              */
/*                                                                  */
/* INPUTS:  Sounce - Pointer to source buffer                       */
/*          Target - Pointer to target buffer                       */
/*          length - Number of bytes to copy                        */
/*                                                                  */
/********************************************************************/
void memcpy(target,source,len)
     char *source, *target;
     long len;
{

      volatile long num_int, num_char, *int_src, *int_tgt;
      volatile char *char_src, *char_tgt;
      volatile int i;

      /******************************************************************/
      /* Move source and target buffer pointers to local valriables and */
      /* calculate the number of intergers and characters to be written */
      /******************************************************************/
      int_src = (long *) source;
      int_tgt = (long *) target;
      num_int = len / 4;
      num_char = len % 4;

      /******************************************************************/
      /* Move 4 bytes at a time to the target buffer                    */
      /******************************************************************/
      if (num_int != 0)
      {
	 for (i=0; i < num_int; i++)
	      *int_tgt++ = *int_src++;
      }

      /******************************************************************/
      /* Move 1 byte at a time to the target buffer if necessary        */
      /******************************************************************/
      if (num_char != 0)
      {

	 char_src = (char *) int_src;
	 char_tgt = (char *) int_tgt;
	 for (i=0; i < num_char; i++)
	       *char_tgt++ = *char_src++;
      }

}

/*************************************************************************/
/*   IDENTIFICATION: PS_SCR                                              */
/*   DESCRIPTIVE name:  Move current contents of Presentation space      */
/*                      to display                                       */
/*                                                                       */
/*   FUNCTION  - collect chars one by one until attr changes from        */
/*               the one we saved or until the end of the                */
/*               line or rectangle border is reached or                  */
/*               if the code page changes.                               */
/*             - Send the collected string to vtttext  .                 */
/*             - Continue until all chars of rectangle have been sent.   */
/*                                                                       */
/*       NOTE:                                                           */
/*             In particular, this routine will group all chars          */
/*             on a given line/subline that have the same attrs.         */
/*             When an attr changes, the line/subline is sent to         */
/*             draw_txt. Remember that draw_txt can handle at most one   */
/*             line at a time.                                           */
/*                                                                       */
/*************************************************************************/
ps_scr(vp)
struct vtmstruc *vp;
{
struct vtt_box_rc_parms clr;       /* temp. decl. for call to clear rect       */
  int           indx;                   /* index into font table array  */
  ushort        curnt_attr;             /* current attribute            */
  ushort        curnt_cpbase;           /* current codepg base          */
  ushort        nxt_cpbase;             /* next    codepg base          */
  char          curnt_char;             /* current character            */
  Pse           *psp;                   /* ptr into ps                  */
  int           li_begin;               /* start for the line/subline   */
  struct vtt_rc_parms rc_parms;         /* string position and length   */
  struct vtt_cp_parms cp_parms;         /* code pt and new cursor pos   */
  struct vtt_rc_parms *rc = &rc_parms;  /* ptr                          */
  struct vtt_cp_parms *cp = &cp_parms;  /* ptr                          */
  uint          cursor_show;            /* if true, cursor moved to pos.*/
  short         num_row;                /* local variable - num of rows */
  short         num_col;                /* local variable - num of cols */
  int           i,j,k;
  uint          ret;
  char          savflg;                 /* saving envp flags            */
  char txt_buf[512];
  struct gemini_data *ld;

#ifdef GEM_DBUG
printf("ps_scr: entered\n");
#endif GEM_DBUG

  ld = (struct gemini_data *) vp->vttld;

  savflg = *((char *)&ld->Vttenv.flg);
  ld->Vttenv.flg.cur_blank = BLANK;    /* set cursor blank flag        */
  cursor_show = FALSE;
  psp = ld->Vttenv.ps;
  cp->cp_base = 0;
  ret = 0;

  li_begin = ld->Vttenv.scroll + ( (ld->draw_rect.ul_row - 1) *
	     ld->Vttenv.ps_size.wd) + (ld->draw_rect.ul_col - 1) ;
  /************************************************************/
  /* do a line at a time unless :                             */
  /*    attr changes, or                                      */
  /*    right side of rectangle is reached or                 */
  /*    code page changes.                                    */
  /*                                                          */
  /* i is row increment within rect                           */
  /* j is increment along row  within rect                    */
  /* k is increment in data buffer                            */
  /*                                                          */
  /* Call DTXT.                                               */
  /*                                                          */
  /************************************************************/

  num_row = ld->draw_rect.lr_row - ld->draw_rect.ul_row + 1;
  num_col = ld->draw_rect.lr_col - ld->draw_rect.ul_col + 1;
  rc->string_index = 1;                         /* start pos it buffer  */

  for ( i = 0; i < num_row; i++) {

	if ( li_begin >= ld->Vttenv.ps_words )     /* adr larger than ps     */
		li_begin -= ld->Vttenv.ps_words;

	/*--------------------------------------------------------------*/
	/* Set up current attr and codepg base variables.               */
	/*--------------------------------------------------------------*/
	if (ld->draw_rect.attr_valid)
	   curnt_attr = ld->draw_rect.attr;
	else {
	   curnt_attr = (psp + li_begin)->ps_entry.ps_attr;
	}

	curnt_cpbase = 0;
	nxt_cpbase = 0;
	curnt_char = (char)((psp+li_begin)->ps_entry.ps_char);

	txt_buf[0] = curnt_char;       /* setup first character.       */
	for ( j = 1, k = 1; j < num_col; j++,k++) {
	    li_begin++;                     /* advance to next char */

	    txt_buf[k] = (char)((psp+li_begin)->ps_entry.ps_char);
	    if (curnt_attr != (psp + li_begin)->ps_entry.ps_attr)
	       {
	       /*--------------------------------------------------*/
	       /* set up parms                                     */
	       /* write out buffer                                 */
	       /*--------------------------------------------------*/

	       cp->attributes = curnt_attr;
	       cp->cp_base = curnt_cpbase;
	       curnt_cpbase = nxt_cpbase;
	       rc->string_length = k;
	       rc->start_row = ld->draw_rect.ul_row + i;
	       rc->start_column = ld->draw_rect.ul_col + (j - k);

	       rc->dest_row = rc->start_row;
	       rc->dest_column = ld->draw_rect.ul_col + j;
	       ret = draw_txt(vp,cp,rc,txt_buf,cursor_show);
	       if (ret != SUCCESS)
		     return(ERROR);

	       /*--------------------------------------------------*/
	       /* set curnt attr                                   */
	       /* reset k = 0                                      */
	       /* load next character into buffer                  */
	       /*--------------------------------------------------*/
	       if (ld->draw_rect.attr_valid)
		  curnt_attr = ld->draw_rect.attr;
	       else
		  curnt_attr = (psp + li_begin)->ps_entry.ps_attr;

	       txt_buf[0] = txt_buf[k];  /* get ready for next lp*/
	       k = 0;
	   }
       }

       /*--------------------------------------------------------------*/
       /* Reach here when writing out last part of row.                */
       /* set attrs.                                                   */
       /* write out buffer.                                            */
       /* update li_begin to next rect line char.                      */
       /* set curnt_attr                                               */
       /*--------------------------------------------------------------*/
       cp->attributes = curnt_attr;
       cp->cp_base = curnt_cpbase;
       rc->string_length = k;
       rc->string_index = 1;
       rc->start_row = ld->draw_rect.ul_row + i;
       rc->start_column = ld->draw_rect.ul_col + (j - k);
       rc->dest_row = rc->start_row;
       rc->dest_column = ld->draw_rect.ul_col + (j - 1);
       ret = draw_txt(vp,cp,rc,txt_buf,cursor_show);
       if (ret != SUCCESS)
	     return(ERROR);
       k = 0;
       li_begin++;
  }
  *((char *)&ld->Vttenv.flg) = savflg;

  /*--------------------------------------------------------------------*/
  /* if cursor is visible and not blank then draw the cursor            */
  /*--------------------------------------------------------------------*/
  if ( (ld->Vttenv.flg.cur_vis) && (!ld->Vttenv.flg.cur_blank) )
	upd_cursor(vp);

#ifdef GEM_DBUG
printf("ps_scr: exited\n");
#endif GEM_DBUG
   return(SUCCESS);

}

/*************************************************************************/
/*   IDENTIFICATION: UPD_CURSOR                                          */
/*   DESCRIPTIVE name:  Update Cursor                                    */
/*   FUNCTION: Write the WRITE PIXEL S.E. to S.E. FIFO using XOR logical */
/*             operation. After that write cursor bitmap from system     */
/*             memory to the DATA FIFO. If FIFOs are empty interrupt     */
/*             the corresponding cards.                                  */
/*                                                                       */
/*************************************************************************/
void upd_cursor(vp)
struct vtmstruc *vp;
{
	struct ulpel_pos xy_ulpel;

	static uint se_data[11] = {CE_LOGOLN, 0x06,
				   CE_WRITLN, 0x20, 0x00, 0x00, 0x00,
					      0x00, 0x00, 0x00, 0x00};
	union {
	      uint   intval;
	      ushort shortval[2];
	      } tmp;

	struct gemini_data *ld;

#ifdef GEM_DBUG
  printf("upd_cursor: entered addr vp = %08x\n",vp);
#endif GEM_DBUG

	ld = (struct gemini_data *) vp->vttld;
	/*************************************************************/
	/* Put a WRITE PIXEL Structure Element in the Immediate      */
	/* FIFO. The Logical Op is EOR, thus if the cursor is        */
	/* visable, it will become invisable or vica versa.          */
	/*************************************************************/
	GET_ULPEL_XY((int)ld->Vttenv.cursor_pos.row,
		     (int)ld->Vttenv.cursor_pos.col,xy_ulpel);
	se_data[4] = itof(xy_ulpel.x_ul);
	se_data[5] = itof(xy_ulpel.y_ul);

	tmp.shortval[0] = ld->Vttenv.char_box.height;
	tmp.shortval[1] = (ld->Vttenv.char_box.width + 31) & 0xFFFFFFE0;
	se_data[7] = tmp.intval;

	se_data[8]  = ((((ld->Vttenv.char_box.width) % 32) + 31) &
			  0xFFFFFFE0) - ((ld->Vttenv.char_box.width) % 32);
	se_data[9]  = ld->Vttenv.cursor_color.fg;
	se_data[10] = ld->Vttenv.cursor_color.bg;

	/**************************************************************/
	/* Copy the cursor bitmap to Immediate BLT FIFO               */
	/**************************************************************/
	wrfifo(ImmFIFO, se_data, sizeof(se_data), ld);
	wrfifo(ImmBLTFIFO,(ld->Vttenv.cursor_pixmap),
			  (ld->Vttenv.cursor_pixmap_size), ld);

#ifdef GEM_DBUG
  printf("upd_cursor: exited\n");
#endif GEM_DBUG

}

/*************************************************************************/
/*   IDENTIFICATION: NEW_CURSOR                                          */
/*                                                                       */
/*   DESCRIPTIVE name:  Define Cursor for the Gemini  Display adapter    */
/*                                                                       */
/*   FUNCTION: This command changes the shape of the text cursor         */
/*             to one of six predefined shapes. It also allows shapes to */
/*             be defined that consist of a set of consecutive character */
/*             box scan lines.                                           */
/*     Called by VTTDEFC to define a new cursor and color expand it      */
/*                                                                       */
/*************************************************************************/
long new_cursor(vp)
struct vtmstruc *vp;
{
	int i, bgcnt, fgcnt, size;
	uint *cur_map;
	struct gemini_data *ld;

#ifdef GEM_DBUG
printf("new_cursor: entered\n");
#endif GEM_DBUG

	/****************************************************************/
	/* If the Cursor Pixel Map pointer is not NULL, then free the   */
	/* storage.                                                     */
	/****************************************************************/
	ld = (struct gemini_data *) vp->vttld;
	if (ld->Vttenv.cursor_pixmap != NULL)
		xmfree(ld->Vttenv.cursor_pixmap,pinned_heap);

	/****************************************************************/
	/* Malloc storage for the Cursor Pixel Map and save its size    */
	/****************************************************************/
	size = (((ld->Vttenv.char_box.width/32) + 1) * 4) *
					    ld->Vttenv.char_box.height;
	ld->Vttenv.cursor_pixmap_size = size;
	cur_map = (uint *) xmalloc(size,3,pinned_heap);
	if (cur_map == NULL)
	{
		gemlog(vp,NULL,"new_cursor","xmalloc",NULL,NULL,UNIQUE_1);
		errno = ENOMEM;
		return(ENOMEM);
	}

	ld->Vttenv.cursor_pixmap = (char *) cur_map;
	bzero(cur_map,size);

	/****************************************************************/
	/* Select the requested cursor                                  */
	/****************************************************************/
	switch (ld->Vttenv.cursor_select)
	{
	case 0x00:
		/********************************************************/
		/* Invisible : set cursor blank flag and return         */
		/********************************************************/
		ld->Vttenv.flg.cur_blank = BLANK;
		return(SUCCESS);
		break;
	case 0x01:
		/********************************************************/
		/* Single Underscore : set cursor top and bottom scan   */
		/* lines                                                */
		/********************************************************/
		ld->Vttenv.cur_shape.top = ld->Vttenv.char_box.height - 1;
		ld->Vttenv.cur_shape.bot = ld->Vttenv.char_box.height - 1;
		break;
	case 0x02:
		/********************************************************/
		/* Double Underscore : set cursor top and bottom scan   */
		/* lines                                                */
		/********************************************************/
		ld->Vttenv.cur_shape.top = ld->Vttenv.char_box.height - 2;
		ld->Vttenv.cur_shape.bot = ld->Vttenv.char_box.height - 1;
		break;
	case 0x03:
		/********************************************************/
		/* Half Blob lower half of character : set cursor top   */
		/* and bottom scan lines                                */
		/********************************************************/
		ld->Vttenv.cur_shape.top = ld->Vttenv.char_box.height / 2;
		ld->Vttenv.cur_shape.bot = ld->Vttenv.char_box.height - 1;
		break;
	case 0x04:
		/********************************************************/
		/* Double line thru center of character box : set cursor*/
		/* top and bottom scan lines                            */
		/********************************************************/
		ld->Vttenv.cur_shape.top =
		    (ld->Vttenv.char_box.height/2) - 1;
		ld->Vttenv.cur_shape.bot = ld->Vttenv.char_box.height/2;
		break;
	case 0x05:
		/********************************************************/
		/* Full Blob : set cursor top and bottom scan lines     */
		/********************************************************/
		ld->Vttenv.cur_shape.top = 0x00;
		ld->Vttenv.cur_shape.bot = ld->Vttenv.char_box.height - 1;
		break;
	default:
		/*********************************************************/
		/* Selectors 6-254 are reserved                          */
		/* any value other than 0-5 will get a double underscore!*/
		/*********************************************************/
		ld->Vttenv.cur_shape.top = ld->Vttenv.char_box.height - 2;
		ld->Vttenv.cur_shape.bot = ld->Vttenv.char_box.height - 1;
		break;
	};

	/****************************************************************/
	/* If the top and bottom scan lines are greater than the maximum*/
	/* number of lines(pels) in a character then default to the     */
	/* maximum number of line in the character box                  */
	/****************************************************************/
	if (ld->Vttenv.cur_shape.top > (ld->Vttenv.char_box.height - 1))
		ld->Vttenv.cur_shape.top = ld->Vttenv.char_box.height - 1;

	if (ld->Vttenv.cur_shape.bot > (ld->Vttenv.char_box.height - 1))
		ld->Vttenv.cur_shape.bot = ld->Vttenv.char_box.height - 1;

	cur_map += ld->Vttenv.cur_shape.top;
	for (i=0; i < (ld->Vttenv.cur_shape.bot -
				ld->Vttenv.cur_shape.top) + 1; i++)
	      *cur_map++ = 0xFFFFFFFF;

	ld->Vttenv.flg.cur_blank = NON_BLANK;

#ifdef GEM_DBUG
printf("new_cursor: exited\n");
#endif GEM_DBUG
	return(SUCCESS);
}

/************************************************************************/
/*  IDENTIFICATION: LOAD_UCODE                                          */
/*  DESCRIPTIVE name: Load the micro code for C25 and Weitek based      */
/*                    cards in the Gemini adapter.                      */
/*  FUNCTION: This routine does the following functions.                */
/*              -Initializes Gemini Memory Controller and MBC registers */
/*              -Set Shading processor existance flag                   */
/*              -Get the address of C25 and Weitek micro code and load  */
/*               them into Gemini global memory                         */
/*              -Load the Clear Fifo and Memory Test routines into Drp  */
/*               and ShP (if exists) card(s) and run them.              */
/*              -Load Micro Code Loader routine into the above card(s)  */
/*               and run it                                             */
/*              -Initialize Drp, GCP and ShP BIF Comm area              */
/*              -Set the GCP BIF Run Flag to the right codes for the    */
/*               adapter to load the micro code and Clear Fifo routines */
/*               into the card memory and run them if needed.           */
/*                                                                      */
/*************************************************************************/
#define BIFLEN 512
#define BIF_PATTERN 0x00000080

load_ucode(pd,ld,start_addr)
     struct phys_displays *pd;
     struct gemini_data *ld;
     uint start_addr;
{
	unsigned int *moduleptr;
	unsigned int module_offset;

	int *m_stat_rp;                   /* ptr to Master Stat reg in MBC*/
	int *m_ctrl_rp;                   /* MBC Master Contro reg ptr    */
	int *mem_s_adr_rp;                /* MBC Memory Start Addr reg ptr*/
	int *mem_e_adr_rp;                /* MBC Memory End Addr reg ptr  */
	int *loc_s_adr_rp;                /* MBC Local Start Addr reg ptr */
	int *loc_e_adr_rp;                /* MBC Local End Addr reg ptr   */
	int *glb_s_adr_rp;                /* MBC Global Start Addr reg ptr*/
	int *glb_e_adr_rp;                /* MBC Global end Addr reg ptr  */
	uint volatile *ipnd_r_p;           /* Interrupt Pending reg ptr    */
	int volatile *runerrflgp;         /* BIF Run Flag ptr             */
	volatile int i, j, rc;
	int drp_uc_aoff;                  /* DrP uc adapter offset in glb */
	int shp_uc_aoff;                  /* ShP uc adapter offset in glb */
	uint *uc_strtp;                   /* ptr to start of ucode in glb */
	uint *tbl_strtp;                  /* ptr to start of ucode in glb */
	int *clrfifo_p;                   /* ptr to Clear Fifo rtn        */
	int *memtst_p;                    /* ptr to Memory test rtn       */
	uint *bif_p;                      /* ptr to BIF                   */
	int *bstrp_p;                     /* ptr to Boot Strap rtn        */
	int *glb_44_adr;                  /* offset 0x44 in globa mem ptr */
	int *glb_48_adr;                  /* offset 0x4A in globa mem ptr */
	int *glb_4C_adr;                  /* offset 0x4C in globa mem ptr */
	int *glb_50_adr;                  /* offset 0x50 in globa mem ptr */
	volatile uint *temp;
	int features;
	int mod_cnt;
	uint *offsetptr;
	struct drp_comm *drp_com_p;
	unsigned char *delay_ptr;               /* ptr to bud addr with
						  certain length of delay*/
        volatile unsigned char val;
	uint addr;

	struct gem_dds *gem_ddsp;

	volatile uint *status_reg;
	volatile uint status;


	volatile ulong *fifo_ctl_ptr;

#ifdef GEM_DBUG
printf("load_ucode: entered\n");
#endif GEM_DBUG

	/****************************************************************/
	/* Initialize Local Data structure with GEMINI Global addresses */
	/****************************************************************/
	addr = start_addr & 0x0fffffff;
	ld->a_gem_gmem  = start_addr;
	ld->fifo_ptr[0] = (unsigned long *) (addr + FIF0_0_INPUT);
	ld->fifo_ptr[1] = (unsigned long *) (addr + FIF0_1_INPUT);
	ld->fifo_ptr[2] = (unsigned long *) (addr + FIF0_2_INPUT);
	ld->fifo_ptr[3] = (unsigned long *) (addr + FIF0_3_INPUT);
	ld->fifo_cnt_reg[0] = (unsigned long *) (addr + FIFO_0_USE_CNT);
	ld->fifo_cnt_reg[1] = (unsigned long *) (addr + FIFO_1_USE_CNT);
	ld->fifo_cnt_reg[2] = (unsigned long *) (addr + FIFO_2_USE_CNT);
	ld->fifo_cnt_reg[3] = (unsigned long *) (addr + FIFO_3_USE_CNT);
	ld->gc_r_p   = (unsigned int *) (start_addr + GEM_CONTROL);
	ld->sc_r_p   = (unsigned int *) (start_addr + SYS_CONTROL);
	ld->gadr_r_p = (unsigned int *) (start_addr + GEO_ADDR);

	/****************************************************************/
	/* Initialize local variables                                   */
	/****************************************************************/
	delay_ptr = (unsigned char *) ((start_addr & 0xf0000000) + DELAY_ADDR);
	gem_ddsp = (struct gem_dds *) pd->odmdds;

	uc_strtp = (uint *)(start_addr + GLOBAL_OFF);
	bif_p = (uint *)start_addr;
	m_stat_rp =    (int *)(start_addr + MASTER_STAT);
	m_ctrl_rp =    (int *)(start_addr + MASTER_CTRL);
	mem_s_adr_rp = (int *)(start_addr + MEMORY_START);
	mem_e_adr_rp = (int *)(start_addr + MEMORY_END);
	loc_s_adr_rp = (int *)(start_addr + LOCAL_START);
	loc_e_adr_rp = (int *)(start_addr + LOCAL_END);
	glb_s_adr_rp = (int *)(start_addr + GLOBAL_START);
	glb_e_adr_rp = (int *)(start_addr + GLOBAL_END);
	ipnd_r_p = (uint *)(start_addr + INTR_PENDING);
	runerrflgp = (int *)(start_addr + BIF_RUNFLG_A);
	drp_com_p = (struct drp_comm *) start_addr;

	/****************************************************************/
	/* +++++++++   RESET ADAPTER +++++++++++++++                    */
	/* Reset GEMINI and CVME                                        */
	/****************************************************************/
	*(ld->gc_r_p) = RESET_GEMINI;
	*(ld->gc_r_p) = CLEAR_REG;
	*(ld->gc_r_p) = RESET_CVME;

	/****************************************************************/
	/* DO mode load to initialize Gemini memory controller.         */
	/****************************************************************/
	*(ld->gc_r_p ) = SET_MODELOAD;
	for (i=0; i < DELAY_MODE_LOAD; i++)
	    val = *((uchar volatile *) delay_ptr);
	temp = (uint *)(start_addr + MLOAD_ADDR);
	*temp = 0x00000000;
	for (i=0; i < DELAY_MODE_LOAD; i++)
	    val = *((uchar volatile *) delay_ptr);

	*(ld->gc_r_p) = CLEAR_REG;

	/****************************************************************/
	/* Determine if 2 meg or 4 meg of memory is installed on adapter*/
	/****************************************************************/
	temp = (uint *)(start_addr + TWO_MEG + 0x1000);
	*temp = 0xA5A5A5A5;
	rc = *temp;

	/****************************************************************/
	/* Initialize Master Bus Control(MBC) registers. Set the Master */
	/* Control Register, Local and Global address decode (enable)   */
	/****************************************************************/
	*(ld->gadr_r_p) = (*(ld->gc_r_p)) >> 28;

	*m_ctrl_rp    = 0xC0000000;           /* Reset MBC Registers    */
	*m_ctrl_rp    = 0xE0000000;
	*mem_s_adr_rp = CLEAR_REG;
	*mem_e_adr_rp = CLEAR_REG;
	*loc_s_adr_rp = CLEAR_REG;
	*loc_e_adr_rp = CLEAR_REG;
	*glb_s_adr_rp = GBL_LO_LIMIT;

	if (rc == 0xA5A5A5A5)
	{
            gem_ddsp->features |= FOUR_MEG_INSTALLED;
	    *glb_e_adr_rp = GBL_HI_LIMIT_4M;
	}
	else
	{
            gem_ddsp->features &= ~(FOUR_MEG_INSTALLED);
	    *glb_e_adr_rp = GBL_HI_LIMIT_2M;
	}

	/****************************************************************/
	/* Get slot numbers for Gemini cards.                           */
	/****************************************************************/
	if ((gem_cardslots(ld)) == -1)
	{
	     /***********************************************************/
	     /* There is a missing card in the adapter(GCP or DRP).     */
	     /* Abort the load, log the error and return error code.    */
	     /***********************************************************/
	      gemlog(NULL,ld->component,"load_ucode","load_ucode",
						    NULL,BAD_ID,UNIQUE_1);
	      return(ERROR);
	}

	/****************************************************************/
	/* Initialize frequently used offsets in adapter global memory  */
	/****************************************************************/
	glb_44_adr = (int *)(start_addr + 0x44);
	glb_48_adr = (int *)(start_addr + 0x48);
	glb_4C_adr = (int *)(start_addr + 0x4C);
	glb_50_adr = (int *)(start_addr + 0x50);

	/****************************************************************/
	/* If the SHP is not installed, then clear Master Status Reg and*/
	/* Interrupt Pending Reg else set flag indicating SHP installed */
	/****************************************************************/
	if (ld->gcardslots->shp == 0)
	{
		*(ld->gadr_r_p) = ld->gcardslots->magic;
		*m_stat_rp = 0;
		temp = *ipnd_r_p;                /* clear pending intrs */
	}
	else
		ld->ipl_shp_fp->shp_flg = 1;

	/***************************************************************/
	/* Load c25, SHP GVP and GCP GVP microcode into adapter Global */
	/* memory using memcpy                                         */
	/***************************************************************/
	memcpy(uc_strtp, gem_ddsp->u3.c25ucp, gem_ddsp->c25uclen);

	tbl_strtp = (uint *)((start_addr + gem_ddsp->c25uclen + 3
					    + GLOBAL_OFF) & 0xfffffffc);

	memcpy(tbl_strtp, gem_ddsp->u4.shptblp, gem_ddsp->shptbllen);

	/***************************************************************/
	/* +++++++++ Load c25 micro code into the DRP. +++++++++++++++ */
	/***************************************************************/

	/***************************************************************/
	/* Select the DRP and hold the C25 processor. Initialize ptrs  */
	/* to sizes of different routines in the C25 ucode load file   */
	/***************************************************************/
	*(ld->gadr_r_p) = DRP_G_ADR;
	*(ld->sc_r_p)  &= HOLD_ADAP;

	clrfifo_p = (int *) ((char *) uc_strtp + 0x0020);
	memtst_p  = (int *) ((char *) uc_strtp + *(uc_strtp));
	bstrp_p   = (int *) ((char *) uc_strtp + *(uc_strtp+1));

	drp_uc_aoff = VME_BUS_MEM | *(uc_strtp+2) + GLOBAL_OFF;
	shp_uc_aoff = VME_BUS_MEM | *(uc_strtp+3) + GLOBAL_OFF;

	/***************************************************************/
	/* Load the Clear FIFO routine into the DRPs BIF               */
	/***************************************************************/
	memcpy(bif_p, clrfifo_p, BIFLEN);

	/****************************************************************/
	/* Set up the interface for Clear Fifo routine in DRPs BIF      */
	/****************************************************************/
	*(int *)(start_addr + 0x14) = 0x003C003D;
	*(int *)(start_addr + 0x18) = 0x16A45500;

	*glb_44_adr = 0x607FD001;

	/****************************************************************/
	/* Release the DRPs C25 processor and wait for the Clear FIFO   */
	/* routine to complete.                                         */
	/****************************************************************/
	*(ld->sc_r_p) |= REL_ADAP;
	*(ld->sc_r_p) |= RELS_ADAP;
	if (gem_wait(start_addr + BIF_RUNFLG_A, DRP_ERROR, delay_ptr,ld))
	     return(ERROR);

	/****************************************************************/
	/* Hold the DRPs C25 processor and load memory tests into the   */
	/* DRP's BIF                                                    */
	/****************************************************************/
	*(ld->sc_r_p) &= HOLD_ADAP;
	memcpy(bif_p, memtst_p, BIFLEN);

	/****************************************************************/
	/* Set up the interface for Mem Test routine for exec in DRP BIF*/
	/****************************************************************/
	*glb_44_adr = 0x00000000;
	*glb_48_adr = 0x40004000;
	*glb_4C_adr = 0x00004000;

	/****************************************************************/
	/* Wait for Run flag to be turned OFF, indicating that the      */
	/* Memory test has completed.                                   */
	/****************************************************************/
	*(ld->sc_r_p) |= REL_ADAP;
	*(ld->sc_r_p) |= RELS_ADAP;
	if (gem_wait(start_addr + BIF_RUNFLG_A, DRP_ERROR, delay_ptr,ld))
	     return(ERROR);

	/****************************************************************/
	/* Hold the DRPs C25 processor and load the Bootsrtap Loader    */
	/* into DRP's BIF                                               */
	/****************************************************************/
	*(ld->sc_r_p) &= HOLD_ADAP;
	memcpy(bif_p, bstrp_p, BIFLEN);

	/****************************************************************/
	/* Set up the interface for Ucode Loader rtn for exec in DRP BIF*/
	/****************************************************************/
	*glb_44_adr = drp_uc_aoff;

	/****************************************************************/
	/* Adapter Global mem start address of DRP microcode            */
	/****************************************************************/
	*glb_48_adr = 0x40004000;
	*glb_4C_adr = 0x00004000;
	*glb_50_adr = 0x00420002;

	/****************************************************************/
	/* Release the DRPs C25 processor and wait for the run flag in  */
	/* the DRPs BIF to be reset by the DRP microcode when it has    */
	/* completed initialization.                                    */
	/****************************************************************/
	*(ld->sc_r_p) |= REL_ADAP;
	*(ld->sc_r_p) |= RELS_ADAP;
	if (gem_wait(start_addr + BIF_RUNFLG_A, DRP_ERROR, delay_ptr,ld))
	     return(ERROR);

	/****************************************************************/
	/* Initialize DRP BIF Communication area                        */
	/****************************************************************/
	drp_com_p->tag_cntr = 0;
	if (ld->ipl_shp_fp->shp_flg == 1)
	   drp_com_p->vme_shp_tctr = 0xfff00078 | (SHP_G_ADR << 16);
	else
	   drp_com_p->vme_shp_tctr = 0;
	drp_com_p->im_dt_stp = VME_BUS_MEM | BltImmedFifo;
	drp_com_p->im_dt_rdp = VME_BUS_MEM | BltImmedFifo;
	drp_com_p->tr_dt_stp = VME_BUS_MEM | BltTravFifo;
	drp_com_p->tr_dt_rdp = VME_BUS_MEM | BltTravFifo;
	drp_com_p->vme_pix_flgp =  0xfff00020 | (GCP_G_ADR << 16);
	bif_p = (uint *)&drp_com_p->d_r_intblk;
	*bif_p = 0;

#ifdef GEM_DBUG
printf("load_ucode: DRP LOADED\n");
#endif GEM_DBUG

	/****************************************************************/
	/* If shading processor exists, load its micro code.            */
	/****************************************************************/
	if (ld->ipl_shp_fp->shp_flg == 1)
	{
		/********************************************************/
		/* Select the SHP and hold the C25 processor            */
		/********************************************************/
		*(ld->gadr_r_p) = SHP_G_ADR;
		*(ld->sc_r_p)  &= HOLD_ADAP;

		/********************************************************/
		/* Load the Clear FIFO  into the SHP BIF, starting from */
		/* begining of the BIF.                                 */
		/********************************************************/
		bif_p = (uint *) start_addr;
		memcpy(bif_p, clrfifo_p, BIFLEN);

		/********************************************************/
		/* Set up the interface for Clear Fifo rtn for exec in  */
		/* SHP BIF                                              */
		/********************************************************/
		*(int *) (start_addr + 0x18) = 0x06A45500;
		*(int *) (start_addr + 0x44) = 0x00000000;

		/********************************************************/
		/* Release the SHPs C25 processor and wait for the      */
		/* Clear FIFO routine to complete.                      */
		/********************************************************/
		*(ld->sc_r_p) |= REL_ADAP;
		*(ld->sc_r_p) |= RELS_ADAP;
		if (gem_wait(start_addr + BIF_RUNFLG_A, SHP_ERROR,
							   delay_ptr,ld))
		    return(ERROR);

		/********************************************************/
		/* Load Memory tests code into SHP's BIF from its start */
		/********************************************************/
		*(ld->sc_r_p) &= HOLD_ADAP;
		memcpy(bif_p, memtst_p,BIFLEN);

		/*******************************************************/
		/* Set up the interface for Mem Test routine for exec  */
		/* in SHP BIF                                          */
		/*******************************************************/
		*(int *)(start_addr + 0x18) = 0x16A45500;
		*glb_44_adr = 0x00000000;
		*glb_48_adr = 0x40008000;
		*glb_4C_adr = 0x00008000;

		/********************************************************/
		/* Reslease the SHPs C25 processor and wait for run flag*/
		/* to be cleared indicating memory test complete.       */
		/********************************************************/
		*(ld->sc_r_p) |= REL_ADAP;
		*(ld->sc_r_p) |= RELS_ADAP;
		if (gem_wait(start_addr + BIF_RUNFLG_A,SHP_ERROR,
							   delay_ptr,ld))
		    return(ERROR);

		/********************************************************/
		/* Hold the SHPs C25 processor and load the Bootstrap   */
		/* Loader into SHP's BIF                                */
		/********************************************************/
		*(ld->sc_r_p) &= HOLD_ADAP;
		memcpy(bif_p, bstrp_p, BIFLEN);

		/********************************************************/
		/* Set up the interface for Ucode Loader rtn for exec   */
		/* in SHP BIF                                           */
		/********************************************************/
		*glb_44_adr = shp_uc_aoff;
		*glb_48_adr = 0x40008000;
		*glb_4C_adr = 0x00008000;
		*glb_50_adr = 0x00410001;
		bif_p = (uint *) (start_addr + 0x1f4);
		*bif_p = VME_BUS_MEM | ((uint) tbl_strtp - start_addr);

		/********************************************************/
		/* Release the SHPs C25 processor and wait for the      */
		/* run flag to be cleared by the SHP microcode          */
		/* indicating that it has completed initialization.     */
		/********************************************************/
		*(ld->sc_r_p) |= REL_ADAP;
		*(ld->sc_r_p) |= RELS_ADAP;
		if (gem_wait(start_addr + BIF_RUNFLG_A,SHP_ERROR,
							   delay_ptr,ld))
		    return(ERROR);

		/********************************************************/
		/* Initialize SHP BIF Communication area                */
		/* SHP communication area has the same layout as DRP    */
		/********************************************************/
		bif_p    = (uint *) (start_addr + 0x5c);
		*bif_p++ =  VME_BUS_MEM | (GLOBAL_OFF + GCP_GVP_LEN +
						sizeof(struct anno_text));
		*bif_p++ = GCPTravFifo - (GLOBAL_OFF + GCP_GVP_LEN +
						sizeof(struct anno_text));
		*bif_p++ = 128;
		*bif_p   = 0xfff00024 | (GCP_G_ADR << 16);
		bif_p    = (uint *) (start_addr + 0x78);
		*bif_p++ = 0L;
		*bif_p++ = 0xfff00078 | (DRP_G_ADR << 16);
		*bif_p++ = 0xfff00080 | (DRP_G_ADR << 16);
		*bif_p++ = 0xfff00084 | (DRP_G_ADR << 16);
		*bif_p++ = 0xfff00088 | (DRP_G_ADR << 16);
		*bif_p   = 0xfff0008c | (DRP_G_ADR << 16);
#ifdef GEM_DBUG
printf("load_ucode: SHP LOADED\n");
#endif GEM_DBUG

	}
	else
	      /**********************************************************/
	      /* NO SHP installed. Update DDS to indicate this          */
	      /**********************************************************/
	      gem_ddsp->features |= NO_SHP;

	/****************************************************************/
	/* Load the GCP micro code, First copy it into global memory    */
	/****************************************************************/
	memcpy(uc_strtp, gem_ddsp->u1.gcpucp, gem_ddsp->gcpuclen);

	tbl_strtp = (uint *) ((start_addr + gem_ddsp->gcpuclen + 3
					     + GLOBAL_OFF) & 0xfffffffc);

	memcpy(tbl_strtp, gem_ddsp->u2.gcptblp, gem_ddsp->gcptbllen);

	/****************************************************************/
	/* Select and Hold the GCP processor                            */
	/****************************************************************/
	*(ld->gadr_r_p) = GCP_G_ADR;
	*(ld->sc_r_p)   = CLEAR_REG;

	/****************************************************************/
	/* Setup to run BIF tests and basic diagnostics. We have to     */
	/* initialize BIF memory first.                                 */
	/****************************************************************/
	bif_p = (uint *)start_addr;
	for (i=0; i<128; i++)
	    *bif_p++ = (BIF_PATTERN - i) << 16;

	/****************************************************************/
	/* Release the GCP and wait for run flag to be reset            */
	/****************************************************************/
	*(ld->sc_r_p) = REL_ADAP;
	*(ld->sc_r_p) = RELS_ADAP;
	if (gem_wait(start_addr + BIF_RUNFLG_A,GCP_ERROR,delay_ptr,ld))
		return(ERROR);

	/****************************************************************/
	/* Clear BIF to all 0's except for the last 3 words. GCP        */
	/* requires the full word prior to this area to be 0x0.         */
	/****************************************************************/
	bif_p = (uint *)start_addr;
	for (i=0; i<128; i++)
	     *bif_p++ = 0;
	bif_p--;
	bif_p--;
	bif_p--;

	/****************************************************************/
	/* Load GCP microcode                                           */
	/****************************************************************/
	*bif_p++ = VME_BUS_MEM | ((uint) tbl_strtp - start_addr);
	offsetptr = uc_strtp;
	moduleptr = (unsigned int *) offsetptr + 8;
	mod_cnt = 1;
	do
	{
	   *bif_p = (unsigned int) (((unsigned int) moduleptr) &
				 0x001fffff | GBL_LO_LIMIT);
	   *runerrflgp = 0x00040000;      /* load module cmd  */

	   /*************************************************************/
	   /* Wait for run flag to be cleared by loader                 */
	   /*************************************************************/
	   if (gem_wait(start_addr + BIF_RUNFLG_A, GCP_ERROR,
						   delay_ptr,ld))
		   return(-1);

	   offsetptr++;
	   mod_cnt++;
	   module_offset = *offsetptr;
	   moduleptr = (unsigned int *) (start_addr +
				      GLOBAL_OFF + (module_offset));

	}  while ((*offsetptr != 0) & (mod_cnt <= 7));

	/****************************************************************/
	/* Save the starting address of the GCP microcode for soft reset*/
	/* set '5' in the run flag to give the last module loaded       */
	/* control and wait for run flag to be cleared by GCP microcode */
	/* to indicate initialization has completed.                    */
	/****************************************************************/
	moduleptr = (uint) (*bif_p);
	moduleptr = (uint *) ((uint) moduleptr & 0x00FFFFFF);
	moduleptr = (uint *) ((uint) moduleptr | start_addr);
	ld->gcardslots->gcp_start = ((*moduleptr) >> 16);

	*runerrflgp =  0x00050000;
	if (gem_wait(start_addr + BIF_RUNFLG_A,GCP_ERROR,
						  delay_ptr,ld))
		return(ERROR);

	/****************************************************************/
	/*          ------ Initialize the GCP BIF ---------             */
	/* Select the DrP and get the 8/24 bit plane flag from DRP.     */
	/* Update features field in DDS indicating 8/24. If SHP is      */
	/* NOT installed, then update DDS indicating is is NOT present. */
	/****************************************************************/
	*(ld->gadr_r_p) = DRP_G_ADR;
	bif_p = (uint *) start_addr;
	gem_ddsp->features |= *(bif_p + 0x1D);
	features = (gem_ddsp->features & 0xC0000000) |
		*(bif_p + 0x1D) | (ld->ipl_shp_fp->shp_flg << 2);

	/************************************************************/
	/* If hardware subpixel anti-alias installed then set flag  */
	/* in the GCP 'features' field                              */
	/************************************************************/
	if (gem_ddsp->features & ANTI_ALIAS_INSTALLED)
	  features |= 0x10;

	/************************************************************/
	/* If ISO monitors are supporetd then set refresh rate      */
	/************************************************************/
	if ((gem_ddsp->features) & ISO_SUPPORTED)
	{
	    bif_p = (uint *)drp_com_p->r_d_intblk;
	    if ((gem_ddsp->features) & SEVENTY7_HZ_REQUEST)
		*bif_p++ = SEVENTY7_HZ;
	    else
		*bif_p++ = SIXTY_HZ;
	   *bif_p = 0x00;
	   *(ld->sc_r_p) |= (RELS_ADAP | 0x04000000);
	}

	/****************************************************************/
       /* Select the GCP and set up pointers to the Annotation Text     */
       /* Language Table, start addresses of the Immediate and Traversal*/
       /* FIFOs and the Installed Features.                             */
	/****************************************************************/

       /*****************************************************************/
       /* Select the GCP and copy the GCP GVP microcode into adapter    */
       /* Global memory.                                                */
       /*****************************************************************/
       *(ld->gadr_r_p) = GCP_G_ADR;
       memcpy(uc_strtp, gem_ddsp->u5.gvp5ap, gem_ddsp->gvp5alen);

       /*****************************************************************/
       /* Initialize the GCPs BIF                                       */
       /*****************************************************************/
       bif_p = (uint *) start_addr + (0x30/4);
       *bif_p++ = VME_BUS_MEM + GLOBAL_OFF + 4;

       module_offset = ((*uc_strtp) & 0xFFFF0000) >> 16;
       *bif_p++ = VME_BUS_MEM + GLOBAL_OFF + 4 + module_offset;
       *bif_p   = *uc_strtp;

       bif_p = (uint *) start_addr + (BIF_TEXT_OFF/4);
       *bif_p++ = VME_BUS_MEM | GLOBAL_OFF + GCP_GVP_LEN;

       /*****************************************************************/
       /* If this is a 4 meg adapter, then adjust Immediate FIFO        */
       /*****************************************************************/
       if (gem_ddsp->features & FOUR_MEG_INSTALLED)
       {
	    *bif_p++ = VME_BUS_MEM | (GCPImmedFifo + TWO_MEG);
	    *bif_p++ = VME_BUS_MEM | (GCPImmedFifo + TWO_MEG);
       }
       else
       {
	    *bif_p++ = VME_BUS_MEM | GCPImmedFifo;
	    *bif_p++ = VME_BUS_MEM | GCPImmedFifo;
       }
       *bif_p++ = VME_BUS_MEM | GCPTravFifo;
       *bif_p++ = VME_BUS_MEM | GCPTravFifo;
       *bif_p++ = features;                         /* features field */
       *bif_p++ = PIX_METER_19;
       *bif_p++ = 0x00;
       *bif_p++ = 0x00;

       bif_p = (uint *) start_addr + (0x28/4);
       *bif_p = 0x100;

       /*****************************************************************/
       /* Clear Special address                                         */
       /* 								*/
       /* @gm_ucflags = 0x4209c						*/
       /* @gm_2d_ucflags = 0x1dfbf0					*/
       /*****************************************************************/
       bif_p = (uint *) start_addr + (0x4209c/4);
       *bif_p++ = 0x00;
       *bif_p++ = 0x00;
       *bif_p = 0x00;
       bif_p = (uint *) start_addr + (0x4c/4);
       *bif_p = VME_BUS_MEM | 0x4209c;
       bif_p = (uint *) start_addr + (0x420b0/4);
       *bif_p = VME_BUS_MEM | 0x1dfbf0;

       /*****************************************************************/
       /* Now set '1' in the run flag to tell the GCP that the BIF is   */
       /* initialized and wait for it to be cleared                     */
       /*****************************************************************/
       *runerrflgp =  RUN_FLAG;
       if (gem_wait(start_addr + BIF_RUNFLG_A,GCP_ERROR,
						 delay_ptr,ld))
		return(ERROR);

	ld->a_gem_gmem = addr;

       /*****************************************************************/
	/* Initialize Adapter FIFO Global Memory Pointers               */
       /*****************************************************************/
	bif_p = start_addr | GBL_MEM_PTR_0;
	*bif_p++ = GCPImmedFifo;        /* Init global mem ptr */

	bif_p = start_addr | GBL_MEM_PTR_1;
	*bif_p = BltImmedFifo;

	bif_p = start_addr | GBL_MEM_PTR_2;
	*bif_p = GCPTravFifo;

	bif_p = start_addr | GBL_MEM_PTR_3;
	*bif_p = BltTravFifo;

       /*****************************************************************/
       /* Enable DMA Suspend in FIFO Ctl Regs                           */
       /*****************************************************************/
       fifo_ctl_ptr = (ulong *) (start_addr | FIFO_0_CONTROL);
       *fifo_ctl_ptr = ENABLE_DMA_SUSP;
       fifo_ctl_ptr = (ulong *) (start_addr | FIFO_1_CONTROL);
       *fifo_ctl_ptr = ENABLE_DMA_SUSP;
       fifo_ctl_ptr = (ulong *) (start_addr | FIFO_2_CONTROL);
       *fifo_ctl_ptr = ENABLE_DMA_SUSP;
       fifo_ctl_ptr = (ulong *) (start_addr | FIFO_3_CONTROL);
       *fifo_ctl_ptr = ENABLE_DMA_SUSP;

       /*****************************************************************/
       /* Enable the adapter for interrupts                             */
       /*****************************************************************/
	*(ld->gc_r_p) = ENABLE_INTR;

#ifdef GEM_DBUG
printf("load_ucode: GCP LOADED\n");
printf("load_ucode: exited\n");
#endif

    return(SUCCESS);
}

/*************************************************************************/
/*  IDENTIFICATION: GEM_CARDSLOTS                                        */
/*                                                                       */
/*  DESCRIPTIVE name: Gemini cards slots                                 */
/*                                                                       */
/*  FUNCTION:                                                            */
/*              Reads the system control register for every slot         */
/*              if a valid card id is found, the appropriate             */
/*              entry in the gcardslots structure is filled in with      */
/*              the bits necessary to select the card again with         */
/*                                                                       */
/*  ENVIRONMENT: gemini should be initialized before calling.            */
/*                                                                       */
/*************************************************************************/
#define NGSLOTS 11
#define CARD_MASK 0x00FF0000

gem_cardslots(ld)
	struct gemini_data *ld;
{
	int     i, n;
	int     gslot;
	ulong   id;
	ulong   card, status, *sys_ctl_reg;
	ulong   *geog_reg, *mstrstat, *gem_ctl_reg;

	geog_reg = (ulong *) (((char *) ld->gadr_r_p));
	mstrstat = (ulong *) (((char *) ld->a_gem_gmem) + MASTER_STAT);
	gem_ctl_reg = (ulong *) (((char *) ld->gc_r_p));
	sys_ctl_reg = (ulong *) (((char *) ld->sc_r_p));
	ld->gcardslots->magic = (*gem_ctl_reg) >> 28;

	ld->gcardslots->drp = 0;
	ld->gcardslots->gcp = 0;
	ld->gcardslots->shp = 0;
	ld->gcardslots->imp = 0;

	*geog_reg = ld->gcardslots->magic;     /* select magic card*/

	for ( gslot=0; gslot < NGSLOTS; gslot++ )
	{
	    *geog_reg = gslot;
	    /* cause gm to latch system control reg */
	    id = (*sys_ctl_reg) & CARD_MASK;

	    *geog_reg = ld->gcardslots->magic;     /* select magic card*/
	    status = (*mstrstat) & M_STAT_MASK;
	    if (status)
	    {
	       *mstrstat = 0x00000000;
	       id = 0;
	    }

	    card = gslot;
	    switch ( id )
	    {
		case SHP_ID:
			ld->gcardslots->shp = card ;
			break;

		case GCP_ID:
			ld->gcardslots->gcp = card;
			break;

		case DRP_ID:
			ld->gcardslots->drp = card;
			break;

		case IMP_ID:
			ld->gcardslots->imp = card;
			break;

		default:
			break;
	    }
	}
#ifdef GEM_DBUG
	printf("gem_cardslots: magic    = %08X\n",ld->gcardslots->magic);
	printf("gem_cardslots: drp      = %08X\n",ld->gcardslots->drp);
	printf("gem_cardslots: gcp      = %08X\n",ld->gcardslots->gcp);
	printf("gem_cardslots: shp      = %08X\n",ld->gcardslots->shp);
	printf("gem_cardslots: imp      = %08X\n",ld->gcardslots->imp);
#endif GEM_DBUG

	/******************************************************************/
	/* Check results. We must have found a GCP and DRP to continue    */
	/******************************************************************/
	if ((ld->gcardslots->drp == 0) | (ld->gcardslots->gcp == 0))
	   return(ERROR);
	else
	   return(SUCCESS);

}

/**************************************************************************/
/*  IDENTIFICATION: GEM_WAIT                                              */
/*                                                                        */
/*  DESCRIPTIVE name: Gemini wait                                         */
/*                                                                        */
/*  FUNCTION:                                                             */
/*              This routine will wait a specific amount of time for      */
/*              the run flag in the selected BIF to be reset by the       */
/*              microcode. If it fails to be reset or an error is         */
/*              is detected by the microcode, then an error is logged     */
/*              error returned to caller.                                 */
/*                                                                        */
/**************************************************************************/
gem_wait(flgp,err_ind,delay_ptr,ld)
	int flgp;
	int err_ind;
	unsigned char *delay_ptr;               /* ptr to bus addr with
						  certain length of delay*/
	struct gemini_data *ld;
{
	int i;
	volatile unsigned char val;
	int volatile *runerrflgp;         /* BIF Run Flag ptr             */
	short volatile *runflg_p;         /* BIF Run Flag ptr             */

	runerrflgp = (int *)flgp;
	runflg_p = (short *)runerrflgp;

	for (i=0; i<DELAY_TIME; i++){
	    val = *((uchar volatile *) (delay_ptr));
	    if (!*runflg_p)
	      break;
	}

	/******************************************************************/
	/* Check run flag for non-zero. report error if true              */
	/******************************************************************/
	if (*runerrflgp)
	{
	    if (i == DELAY_TIME)
		 gemlog(NULL,ld->component,"load_ucode","gem_wait",NULL,
						      (err_ind+1),UNIQUE_2);
	    else
		 gemlog(NULL,ld->component,"load_ucode","gem_wait",
					  *runerrflgp,err_ind,UNIQUE_3);
		 return(ERROR);
	}

	return(SUCCESS);

}

/************************************************************************/
/*  IDENTIFICATION: SOFT_RESET                                          */
/*  DESCRIPTIVE NAME: Resets the three adapter cards GCP DrP and ShP    */
/*  FUNCTION: Reseting of each card includes flushing of internal FIFOs */
/*      and reseting of internal flag and pointers. This routine        */
/*       will also clear the memory mapped FIFO registers and           */
/*      disables the Sync and Threshold interrupt bits in Gemini        */
/*            Control register.                                         */
/*                                                                      */
/************************************************************************/
#define SOFT_DELAY_TIME 1000000
soft_reset(ld,gmem_start,inst_feat)
struct gemini_data *ld;
unsigned int gmem_start, inst_feat;
{
	int  i, features;
	volatile unsigned char val;
	uint *bifp;                     /* ptr to BIF locations         */
	struct drp_comm *drp_com_p;
	struct gcp_comm *gcp_com_p;
	ushort volatile *runflg_p;      /* BIF Run Flag ptr             */
	uint volatile *runerrflgp;      /* BIF Run Flag ptr             */
	unsigned char *delay_ptr;       /* ptr to bud addr with
					   certain length of delay      */
	uint *m_ctrl_rp;                /* MBC Master Control reg ptr   */
	uint *m_stat_rp;                /* MBC Master Status reg ptr    */
	uint *mem_s_adr_rp;             /* MBC Memory Start Addr reg ptr*/
	uint *mem_e_adr_rp;             /* MBC Memory End Addr reg ptr  */
	uint *loc_s_adr_rp;             /* MBC Local Start Addr reg ptr */
	uint *loc_e_adr_rp;             /* MBC Local End Addr reg ptr   */
	uint *glb_s_adr_rp;             /* MBC Global Start Addr reg ptr*/
	uint *glb_e_adr_rp;             /* MBC Global end Addr reg ptr  */
	uint gem_gmem;
	uint seg_reg;
	uint offset, *uc_strtp, temp;
	int parityrc;
	label_t jmpbuf;

	volatile ulong *fifo_ctl_ptr;


#ifdef GEM_DBUG
printf("soft_reset: entered\n");
printf("ld = %08x   gmem_start = %08x\n",ld,gmem_start);
#endif GEM_DBUG

	/****************************************************************/
	/* Initialize Local Data structure with GEMINI Global addresses */
	/****************************************************************/
	 if (parityrc = (setjmpx(&jmpbuf)))
	 {
	      if (parityrc == EXCEPT_IO)
	      {
		  gemlog(NULL,ld->component,"soft_reset","setjmpx",
					     parityrc,NULL,UNIQUE_1);
		  errno = EIO;
		  return(EIO);
	      }
	      else
		 longjmpx(parityrc);
	 }

	seg_reg = BUSMEM_ATT(BUS_ID,0x00);
	gem_gmem = (uint)((char *)gmem_start + seg_reg);

	ld->a_gem_gmem = gmem_start;
	ld->fifo_ptr[0] = (unsigned long *) (ld->a_gem_gmem + FIF0_0_INPUT);
	ld->fifo_ptr[1] = (unsigned long *) (ld->a_gem_gmem + FIF0_1_INPUT);
	ld->fifo_ptr[2] = (unsigned long *) (ld->a_gem_gmem + FIF0_2_INPUT);
	ld->fifo_ptr[3] = (unsigned long *) (ld->a_gem_gmem + FIF0_3_INPUT);
	ld->fifo_cnt_reg[0] = (unsigned long *) (ld->a_gem_gmem +
							   FIFO_0_USE_CNT);
	ld->fifo_cnt_reg[1] = (unsigned long *) (ld->a_gem_gmem +
							   FIFO_1_USE_CNT);
	ld->fifo_cnt_reg[2] = (unsigned long *) (ld->a_gem_gmem +
							   FIFO_2_USE_CNT);
	ld->fifo_cnt_reg[3] = (unsigned long *) (ld->a_gem_gmem +
							   FIFO_3_USE_CNT);

	ld->gc_r_p = (unsigned int *)(gem_gmem + GEM_CONTROL);
	ld->sc_r_p = (unsigned int *)(gem_gmem + SYS_CONTROL);
	ld->gadr_r_p = (unsigned int *)(gem_gmem + GEO_ADDR);

	/****************************************************************/
	/* Initialize local variables                                   */
	/****************************************************************/
	m_ctrl_rp    = (uint *) (gem_gmem + MASTER_CTRL);
	m_stat_rp    = (uint *) (gem_gmem + MASTER_STAT);
	mem_s_adr_rp = (uint *) (gem_gmem + MEMORY_START);
	mem_e_adr_rp = (uint *) (gem_gmem + MEMORY_END);
	loc_s_adr_rp = (uint *) (gem_gmem + LOCAL_START);
	loc_e_adr_rp = (uint *) (gem_gmem + LOCAL_END);
	glb_s_adr_rp = (uint *) (gem_gmem + GLOBAL_START);
	glb_e_adr_rp = (uint *) (gem_gmem + GLOBAL_END);
	drp_com_p    = (struct drp_comm *) gem_gmem;
	gcp_com_p    = (struct gcp_comm *) gem_gmem;
	runerrflgp   = (uint *) (gem_gmem + BIF_RUNFLG_A);
	runflg_p     = (ushort *) runerrflgp;
	delay_ptr = (unsigned char *) (seg_reg + DELAY_ADDR);

	/****************************************************************/
	/* Select and hold the GCP (Weitek Processor), DrP and SHP C25  */
	/* processors.                                                  */
	/****************************************************************/
	*(ld->gadr_r_p) = GCP_G_ADR;
	*(ld->sc_r_p)  &= HOLD_ADAP;
	*(ld->gadr_r_p) = DRP_G_ADR;
	*(ld->sc_r_p)  &= HOLD_ADAP;
	if (ld->ipl_shp_fp->shp_flg == 1)
	{
		*(ld->gadr_r_p) = SHP_G_ADR;
		*(ld->sc_r_p)  &= HOLD_ADAP;
	}

	/****************************************************************/
	/* Reset the adapter. Reset GEMINI, Reset CVME, set function cd */
	/****************************************************************/
	*(ld->gc_r_p) = RESET_GEMINI;
	*(ld->gc_r_p) = RESET_CVME;
	*(ld->gc_r_p) = CLEAR_REG;
	*(ld->gc_r_p) = CLEAR_REG | 0x00500000;   /* temp HAK  */

	/****************************************************************/
	/* Select the MAGIC card and initialize the MBC registers.      */
	/****************************************************************/
	*(ld->gadr_r_p) = (*(ld->gc_r_p)) >> 28;
	*m_stat_rp    = CLEAR_REG;
	*m_ctrl_rp    = 0xC0000000;
	*m_ctrl_rp    = 0xE0000000;
	*mem_s_adr_rp = CLEAR_REG;
	*mem_e_adr_rp = CLEAR_REG;
	*loc_s_adr_rp = CLEAR_REG;
	*loc_e_adr_rp = CLEAR_REG;
	*glb_s_adr_rp = GBL_LO_LIMIT;
	if (!(inst_feat & FOUR_MEG_INSTALLED))
	    *glb_e_adr_rp = GBL_HI_LIMIT_2M;
	else
	    *glb_e_adr_rp = GBL_HI_LIMIT_4M;

	/****************************************************************/
	/* Select the GCP and clear the  area of the BIF for microcode  */
	/* Put starting address of microcode into BIF.                  */
	/****************************************************************/
	*(ld->gadr_r_p) = GCP_G_ADR;

	bifp = (uint *)((char *) gcp_com_p + 0x1f0);
	*bifp++ = 0;
	*bifp++ = 0;
	*bifp++ = ld->gcardslots->gcp_start;
	*bifp = 0;

	/****************************************************************/
	/* Zero-out GCP-RIOS data blocks in the GCP BIF                 */
	/****************************************************************/
	gcp_com_p->g_r_hdbkof = 0;
	gcp_com_p->g_r_tdbkof = 0;
	bifp = (uint *)((char *) gcp_com_p + 0x98);
	for (i=0; i<360/sizeof(struct g_r_dblk); i++)
	    *bifp++ = 0;

	/****************************************************************/
	/* Select the DrP                                               */
	/****************************************************************/
	*(ld->gadr_r_p) = DRP_G_ADR;

	/****************************************************************/
	/* Set Soft Reset request in the DrPs BIF                       */
	/****************************************************************/
	bifp = (uint *)drp_com_p->srset_rqst;
	*bifp++ = DRP_SOFTRSET;
	*bifp = 0x00;
	bifp = (uint *)&drp_com_p->d_r_intblk;
	*bifp = 0;

	/****************************************************************/
	/* Release the C25 processor. Wait for the flag in the BIF to   */
	/* be turned OFF by the DrP ucode                               */
	/****************************************************************/
	*runerrflgp =  RUN_FLAG;                      /* set run flag  */
	*(ld->sc_r_p) |= REL_ADAP;
	*(ld->sc_r_p) |= RELS_ADAP;

	for (i=0; i<SOFT_DELAY_TIME; i++)
	{
	    val = *((uchar volatile *) delay_ptr);
	    if (!*runflg_p)
	      break;
	}

	if (*runerrflgp)
	{
	    if (i == SOFT_DELAY_TIME)
		 gemlog(NULL,ld->component,"soft_reset","soft_reset",
					     NULL,(DRP_ERROR+1),UNIQUE_1);
	    else
		 gemlog(NULL,ld->component,"soft_reset","soft_reset",
					 *runerrflgp,DRP_ERROR,UNIQUE_2);
		 clrjmpx(&jmpbuf);
		 BUSMEM_DET(seg_reg);
		 return(ERROR);
	}

	/********************************************************/
	/* Initialize the DrPs BIF                              */
	/********************************************************/
	drp_com_p->tag_cntr = 0;
	drp_com_p->vme_shp_tctr = 0;
	drp_com_p->im_dt_stp = VME_BUS_MEM | BltImmedFifo;
	drp_com_p->im_dt_rdp = VME_BUS_MEM | BltImmedFifo;
	drp_com_p->tr_dt_stp = VME_BUS_MEM | BltTravFifo;
	drp_com_p->tr_dt_rdp = VME_BUS_MEM | BltTravFifo;
	drp_com_p->vme_pix_flgp =  0xfff00020 | (GCP_G_ADR << 16);
	bifp = (uint *)&drp_com_p->d_r_intblk;
	*bifp = 0;

	/********************************************************/
	/* If the Shading Processor is installed, reset it      */
	/********************************************************/
	if (ld->ipl_shp_fp->shp_flg == 1)
	{
		/*************************************************/
		/* SHP is Installed, initialize the SHPs tag     */
		/* counter pointer in the DrPs BIF               */
		/*************************************************/
		drp_com_p->vme_shp_tctr =
				    0xfff00078 | (SHP_G_ADR << 16);
		/*************************************************/
		/* Select SHP and release C25                    */
		/*************************************************/
		*(ld->gadr_r_p) = SHP_G_ADR;
		*(ld->sc_r_p)  |= REL_ADAP;
		*(ld->sc_r_p)  |= RELS_ADAP;

		/*************************************************/
		/* Set Soft Reset request in the SHPs BIF. The   */
		/* request is placed at the same offset into the */
		/* BIF as that of the DrP, so we use drp struct  */
		/*************************************************/
		bifp = (uint *)drp_com_p->srset_rqst;
		*bifp++ = DRP_SOFTRSET;

		/**************************************************/
		/* Release the C25 processor. Wait for the flag   */
		/* in the BIF to be turned OFF by the DrP ucode   */
		/**************************************************/
		*runerrflgp =  RUN_FLAG;
		*(ld->sc_r_p) |= REL_ADAP;
		*(ld->sc_r_p) |= RELS_ADAP;

		for (i=0; i<SOFT_DELAY_TIME; i++)
		{
		   val = *((uchar volatile *) delay_ptr);
		   if (!*runflg_p)
		      break;
		}

		if (*runerrflgp)
		{
		    if (i == SOFT_DELAY_TIME)
		       gemlog(NULL,ld->component,"soft_reset","soft_reset",
					 NULL,(SHP_ERROR+1),UNIQUE_1);
		    else
		       gemlog(NULL,ld->component,"soft_reset","soft_reset",
				     *runerrflgp,SHP_ERROR,UNIQUE_2);

		    clrjmpx(&jmpbuf);
		    BUSMEM_DET(seg_reg);
		    return(ERROR);

		}

		/*************************************************/
		/* Initialize SHP BIF Communication area         */
		/*************************************************/
		bifp    = (uint *) (gem_gmem + 0x5c);
		*bifp++ =  VME_BUS_MEM | (GLOBAL_OFF + GCP_GVP_LEN +
					  sizeof(struct anno_text));
		*bifp++ = GCPTravFifo - (GLOBAL_OFF + GCP_GVP_LEN +
					  sizeof(struct anno_text));
		*bifp++ = 128;
		*bifp   = 0xfff00024 | (GCP_G_ADR << 16);
		bifp    = (uint *) (gem_gmem + 0x78);
		*bifp++ = 0L;
		*bifp++ = 0xfff00078 | (DRP_G_ADR << 16);
		*bifp++ = 0xfff00080 | (DRP_G_ADR << 16);
		*bifp++ = 0xfff00084 | (DRP_G_ADR << 16);
		*bifp++ = 0xfff00088 | (DRP_G_ADR << 16);
		*bifp =   0xfff0008c | (DRP_G_ADR << 16);
	}

	/************************************************************/
	/*    ------ Initialize the GCP BIF ---------               */
	/* Select the DrP and get the 8/24 bit plane flag           */
	/************************************************************/
	*(ld->gadr_r_p) = DRP_G_ADR;
	bifp = (uint *)gem_gmem;
	features =  *(bifp + 0x1D) | (ld->ipl_shp_fp->shp_flg << 2);

	/************************************************************/
	/* If hardware subpixel anti-alias installed then set flag  */
	/* in the GCP 'features' field                              */
	/************************************************************/
	if (inst_feat & ANTI_ALIAS_INSTALLED)
	  features |= 0x10;

	/************************************************************/
	/* If ISO monitors are supporetd then set refresh rate      */
	/************************************************************/
	if (inst_feat & ISO_SUPPORTED)
	{
	    bifp = (uint *)drp_com_p->r_d_intblk;
	    if (inst_feat & SEVENTY7_HZ_REQUEST)
		*bifp++ = SEVENTY7_HZ;
	    else
		*bifp++ = SIXTY_HZ;
	   *bifp = 0x00;
	   *(ld->sc_r_p) |= (RELS_ADAP | 0x04000000);
	}

	/************************************************************/
	/* Select the GCP and set up pointers to the Annotation Text*/
	/* Text Language Table, start addresses of the Immediate and*/
	/* Traversal FIFOs and the Installed Features.              */
	/************************************************************/
	*(ld->gadr_r_p) = GCP_G_ADR;

	uc_strtp = (uint *)(gem_gmem + GLOBAL_OFF);
	bifp = (uint *) gem_gmem + (0x30/4);
	*bifp++ = VME_BUS_MEM + GLOBAL_OFF + 4;
	offset  = ((*uc_strtp) & 0xFFFF0000) >> 16;
	*bifp++ = VME_BUS_MEM + GLOBAL_OFF + 4 + offset;
	*bifp++ = *uc_strtp;
	*bifp++ = 0x00;
	*bifp++ = 0x00;
	*bifp++ = 0x00;
	*bifp   = 0x00;

	bifp    = (uint *) gem_gmem + (BIF_TEXT_OFF/4);
	*bifp++ = VME_BUS_MEM | GLOBAL_OFF + GCP_GVP_LEN;
	if (!(inst_feat & FOUR_MEG_INSTALLED))
	{
	    *bifp++ = VME_BUS_MEM | GCPImmedFifo;
	    *bifp++ = VME_BUS_MEM | GCPImmedFifo;
	}
	else
	{
	    *bifp++ = VME_BUS_MEM | (GCPImmedFifo + TWO_MEG);
	    *bifp++ = VME_BUS_MEM | (GCPImmedFifo + TWO_MEG);
	}
	*bifp++ = VME_BUS_MEM | GCPTravFifo;
	*bifp++ = VME_BUS_MEM | GCPTravFifo;
	temp = (*bifp) & 0xC0000000;
	*bifp++ = features | temp;

       /*******************************************************/
       /* Clear Special addresses                             */
       /*******************************************************/
       bifp = (uint *) gem_gmem + (0x4209c/4);
       *bifp++ = 0x00;
       *bifp++ = 0x00;
       *bifp = 0x00;
       bifp = (uint *) gem_gmem + (0x4c/4);
       *bifp++ = VME_BUS_MEM | 0x4209c;
       *bifp = 0x00;

       bifp = (uint *) gem_gmem + (0x28/4);
       *bifp = 0x100;

	/********************************************************/
	/* Release the GCP                                      */
	/********************************************************/
	*(ld->sc_r_p) |= REL_ADAP;
	*(ld->sc_r_p) |= RELS_ADAP;

	/*******************************************************/
	/* Initialize Adapter FIFO Global Memory Pointers      */
	/*******************************************************/
	bifp = gem_gmem | GBL_MEM_PTR_0;
	*bifp++ = GCPImmedFifo;
	bifp++;
	*bifp = 0x03;
	*bifp = 0x00;

	bifp = gem_gmem | GBL_MEM_PTR_1;
	*bifp++ = BltImmedFifo;
	bifp++;
	*bifp = 0x03;
	*bifp = 0x00;

	bifp = gem_gmem | GBL_MEM_PTR_2;
	*bifp++ = GCPTravFifo;
	bifp++;
	*bifp = 0x03;
	*bifp = 0x00;

	bifp = gem_gmem | GBL_MEM_PTR_3;
	*bifp++ = BltTravFifo;
	bifp++;
	*bifp = 0x03;
	*bifp = 0x00;

	/********************************************************/
	/* Enable DMA Suspend Logic in FIFO Ctl Regs            */
	/********************************************************/
	fifo_ctl_ptr = (ulong *) (gem_gmem | FIFO_0_CONTROL);
	*fifo_ctl_ptr = ENABLE_DMA_SUSP;
	fifo_ctl_ptr = (ulong *) (gem_gmem | FIFO_1_CONTROL);
	*fifo_ctl_ptr = ENABLE_DMA_SUSP;
	fifo_ctl_ptr = (ulong *) (gem_gmem | FIFO_2_CONTROL);
	*fifo_ctl_ptr = ENABLE_DMA_SUSP;
	fifo_ctl_ptr = (ulong *) (gem_gmem | FIFO_3_CONTROL);
	*fifo_ctl_ptr = ENABLE_DMA_SUSP;

	/********************************************************/
	/* Enable Interrupts on the adapter                     */
	/********************************************************/
	*(ld->gc_r_p) = ENABLE_INTR;

	clrjmpx(&jmpbuf);
	BUSMEM_DET(seg_reg);

#ifdef GEM_DBUG
printf("soft_reset: exited\n");
#endif GEM_DBUG
       return(SUCCESS);

}

/***********************************************************************/
/*                                                                     */
/* IDENTIFICATION: GETPORT                                             */
/*                                                                     */
/* DESCRIPTIVE NAME: GETPORT- This routine will walk all the slots     */
/*                   in the machine looking for an adapter that will   */
/*                   match the GEMINI adapter id. If a GEMINI adapter  */
/*                   is found, then the slot number is returned.       */
/*                   Otherwise, a -1 is returned.                      */
/*                                                                     */
/***********************************************************************/
long getport()
{
	char pdat, *pptr;
	static short i,j,found[4];
	caddr_t io_addr;

#ifdef GEM_DBUG
printf("getport: entered\n");
#endif

	io_addr = IOCC_ATT(BUS_ID,0x00);
	for ( i=0; i < 16; i++)
	{
		/*******************************************************/
		/* loop looking for a GEMINI adapter                   */
		/*******************************************************/
		pptr = io_addr + POSREG(0,i) + 0x400000;
		if (*pptr == 0xFD)
		{
			/***********************************************/
			/* first byte matches now check the second     */
			/***********************************************/
			pptr++;
			if (*pptr == 0x8F)
			{
				for (j=0; j<4; j++)
				{
					/*********************************/
					/* we have found this card before*/
					/*********************************/
					if (found[j] = i)
						continue;
					else
					/*********************************/
					/* New slot found. Return the    */
					/* slot id.                      */
					/*********************************/
					{
						found[j] = i;
						IOCC_DET(io_addr);
						return(i);
					}
				}
				/*****************************************/
				/* If we fall through the loop to here we*/
				/* have found a GEMINI adapter. Return   */
				/* the slot id.                          */
				/*****************************************/
				found[0] = i;
				IOCC_DET(io_addr);
				return(i);
			}
		}
	}

	/*************************************************************** */
	/* if we get here then there is no GEMINI adapter installed      */
	/*************************************************************** */
	IOCC_DET(io_addr);

#ifdef GEM_DBUG
printf("getport: exited -- GEMINI adapter not found\n");
#endif
	return (ERROR);
}

/***********************************************************************/
/*  IDENTIFICATION:   GEMERR                                           */
/*                                                                     */
/*  DESCRIPTION:  GEMINI Adapter error logging routine                 */
/***********************************************************************/
gemlog(vp,comp_nm,dmodule,fmodule,return_code,err_indicator,ras_unique)
struct vtmstruc *vp;

char   *comp_nm;     /* Failing Component name                            */

char   *dmodule;     /* Detecting module is the module that called the    */
		     /* failing module Exp: if you are in VTMOUT and call */
		     /* VTMUPD() and VTMUPD returns a error. VTMOUT is the*/
		     /* detecting module and VTMUPD is the failing module.*/

char   *fmodule;     /* Failing module is the module that returned the bad*/
		     /* return code.                                      */

int    return_code;  /* Return code from failing module */

int    err_indicator;/* Error indicator number reside in RAS.h.       */

char   *ras_unique; /* Unique RAS code used to identitify specific error */
		    /* locations for error logging. Nine RAS unique codes*/
		    /* reside in RAS.h.                                  */
{
   struct gem_dds *gem_ddsp;

   if (vp != NULL)
   {
      gem_ddsp = (struct gem_dds *) vp->display->odmdds;
      gemerr(vp,gem_ddsp->comp_name,dmodule,fmodule,return_code,
					     err_indicator,ras_unique);
   }
   else
      gemerr(vp,comp_nm,dmodule,fmodule,return_code,
					     err_indicator,ras_unique);
#ifdef GEM_DISP_LOG
	printf("gemlog: entered errlog info\n");
	if (vp != NULL)
	    printf("Failing Component ==> %s \n",gem_ddsp->comp_name);
	else
	    printf("Failing Component ==> %s \n",comp_nm);
	if (dmodule != NULL)
		printf("Detecting Module ===> %s \n",dmodule);
	if (fmodule != NULL)
		printf("Failing Module =====> %s \n",fmodule);
	printf("return code ========> %08x \n",return_code);
	if (err_indicator != NULL)
	{
		printf("RAS Error Code =====> %08x \n",err_indicator);
		switch (err_indicator)
		{
		   case ALREADY_DEFINED:
			printf("  Adapter is already defined\n");
			break;
		   case INVALID_FONT:
			printf("  An invalid font was passed in font list\n");
			break;
		   case BAD_ID:
			printf(" GCP or DRP card is missing \n");
			break;
		   case DRP_ERROR:
			printf("  DrP FIFO Clear routine failed\n");
			break;
		   case DRP_TIMEOUT:
			printf("  No response from a DrP module TIMEOUT\n");
			break;
		   case SHP_ERROR:
			printf("  SHP FIFO Clear routine failed\n");
			break;
		   case SHP_TIMEOUT:
			printf("  No response from a SHP module TIMEOUT\n");
			break;
		   case GCP_ERROR:
			printf("  GCP FIFO Clear routine failed\n");
			break;
		   case GCP_TIMEOUT:
			printf("  No response from a GCP module TIMEOUT\n");
			break;
		   case BAD_VECTOR_ID:
			printf("  Adapter returned invalid VECTOR ID\n");
			break;
		   case VME_BUS_ERROR:
			printf("  VME Bus Error\n");
			break;
		   case SPURIOUS_INTR:
			printf("  Spurious Interrupt\n");
			break;
		   case FAULT_GCP:
			printf(" External VME fault - GCP \n");
			break;
		   case FAULT_DRP:
			printf(" External VME fault - DRP \n");
			break;
		   case FAULT_SHP:
			printf("  External VME fault - SHP\n");
			break;
		   case UC_FAULT_GCP:
			printf("  Microcode Fault    - GCP\n");
			break;
		   case UC_FAULT_DRP:
			printf("  Microcode Fault    - DRP\n");
			break;
		   case UC_FAULT_SHP:
			printf("  Microcode Fault    - SHP\n");
			break;
		   case PARITY_ERR_HI_GCP:
			printf("  Parity Error HI    - GCP\n");
			break;
		   case PARITY_ERR_HI_DRP:
			printf("  Parity Error HI    - DRP\n");
			break;
		   case PARITY_ERR_HI_SHP:
			printf("  Parity Error HI    - SHP\n");
			break;
		   case PARITY_ERR_LO_GCP:
			printf("  Parity Error LO    - GCP\n");
			break;
		   case PARITY_ERR_LO_DRP:
			printf("  Parity Error LO    - DRP\n");
			break;
		   case PARITY_ERR_LO_SHP:
			printf("  Parity Error LO    - SHP\n");
			break;
		   case BAD_REQUEST:
			printf("  Illegal Request code \n");
			break;
		   case BAD_ORDER:
			printf("  Illegal Graphic Order\n");
			break;
		   case BAD_INTR_CODE:
			printf("  Unrecognized Interrupt reason\n");
			break;
		   case DRP_EXCEPT:
			printf("  DRP Exception Error \n");
			break;
		   case BAD_DRP_CODE:
			printf("  Invalid DRP Interrupt code \n");
			break;
		   case BUS_PARITY_ERR:
			printf("  VME Bus Parity Error\n");
			break;
		   case BUS_TIMEOUT:
			printf("  VME Bus Timeout\n");
			break;
		   case CPU_BUS_TIMEOUT:
			printf("  VME CPU Bus Timeout\n");
			break;

		   default:
		       break;
		}
	}
	if (ras_unique != NULL)
		printf("RAS Unique =========> %s \n",ras_unique);
#endif GEM_DISP_LOG

	return(0);
}

/*------------
 * FUNCTIONAL DESCRIPTION:
 *
  GEMERR error logging routine.
	char   *comp_name     - GTO component name as passed in by configuration
	char   *dmodule      - Detecting module (the module that called the
				failing module)
	char   *fmodule      - Failing module (the module that returned the
				bad return code)
	int    return_code   - Return code from failing module
	int    err_indicator - Error indicator number from
				com/sysx/hftss/include/hftras.h
	char   *ras_unique   - Unique code used to identitify specific error
				locations for error logging.
  ------------*/

#ifndef ERRID_GRAPHICS
#define ERRID_GRAPHICS 0xF10D0EFF
#endif

gemerr(char *comp_name, char *dmodule, char *fmodule,
	int return_code, int err_indicator, char *ras_unique)

{
	/*****************************************************************/
	/* Fill template ID, component name, and detailed data into the  */
	/* error log record.                                             */
	/*****************************************************************/
	ER.error_id = ERRID_GRAPHICS;

	sprintf(ER.resource_name,"%8s",comp_name);

	sprintf(ER.detail_data,"%8s  %8s  %4d  %4d  %s",
	    dmodule,fmodule,return_code,err_indicator,ras_unique);

	/*****************************************************************/
	/* Call system error logging routine                             */
	/*****************************************************************/
	errsave(&ER, ERR_REC_SIZE + (strlen(ER.detail_data)+1));

	return(0);
}

/***********************************************************************/
/*                                                                     */
/*  wrfifo(f,bufptr,length)     *   Large write to fifo                */
/*                              *   meant for writes of >= 32 bytes    */
/*      int f;                  *   fifo number  (0,1,2,3)             */
/*      char *bufptr;           *   pointer to data buffer             */
/*      int length;             *   bytes  of data to write            */
/*                                                                     */
/***********************************************************************/
wrfifo(f,bufptr,len,ld)
uint f, len;
char *bufptr;
struct gemini_data *ld;
{
	int rc, i;
	uint seg_reg, wait_cnt;
	volatile int free_space;
	volatile unsigned char val;
	volatile ulong *fifo_ptr;
	volatile ulong *fifo_cnt;
	unsigned char *delay_ptr;       /* ptr to addr with
					  certain length of delay      */

	volatile ulong in_use;

	label_t jmpbuf;
	void fifocpy();

	/***************************************************************/
	/* Get access to the adapter                                   */
	/***************************************************************/
	if (rc = (setjmpx(&jmpbuf)))
	{
		if (rc == EXCEPT_IO)
		{
			gemlog(NULL,ld->component,"wrfifo","setjmpx",
						rc,NULL,UNIQUE_1);
			errno = EIO;
			return(EIO);
		}
		else
			longjmpx(rc);
	}

	/***************************************************************/
	/* Initialize local variables                                  */
	/***************************************************************/
	seg_reg = BUSMEM_ATT(BUS_ID,0x00);
	wait_cnt = 0;
	rc = 0;
	fifo_ptr = ((ulong)ld->fifo_ptr[f]) | seg_reg;
	fifo_cnt = ((ulong)ld->fifo_cnt_reg[f]) | seg_reg;
	delay_ptr = (unsigned char *) (seg_reg + DELAY_ADDR);

	/***************************************************************/
	/* Enter loop which will do PIO operations to adapter FIFO     */
	/***************************************************************/
	while (len > 0)
	{
		/*******************************************************/
		/* If the GCP FIFO is to be written, then check if the */
		/* FIFO 'in_use' value is greater than 8K bytes. If so,*/
		/* then delay waitung for it to be decremented. Declare*/
		/* error and exit loop if GCP fails to change in_use.  */
		/*******************************************************/
		if (f == ImmFIFO)
		{
		   in_use = ((*fifo_cnt) & IN_USE_MASK);
		   while (in_use > 0x2000)
		   {
		      for (i = 0; i < 0x10000; i++)
			    val = *((uchar volatile *) delay_ptr);
		      in_use = ((*fifo_cnt) & IN_USE_MASK);
		      wait_cnt++;
		      if (wait_cnt > 0x2000)
		      {
			    rc = ERROR;
			    gemlog(NULL,ld->component,"wrfifo",
					 "wrfifo",rc,f,UNIQUE_2);
			    break;
		      }
		   }
		   wait_cnt = 0;
		}

		/*******************************************************/
		/* Determine amount of space 'not in use' in FIFO. Copy*/
		/* maximum amount of data to FIFO.                     */
		/*******************************************************/
		free_space = SixtyFourK - ((*fifo_cnt) & IN_USE_MASK);

		if (free_space >= len)
		{
			fifocpy(fifo_ptr, bufptr, len);
			len = 0;
		}
		else
			if (free_space > 0)
			{
				fifocpy(fifo_ptr, bufptr, free_space);
				bufptr += free_space;
				len -= free_space;
			}
			else
			{
				/****************************************/
				/* The FIFO is full. Delay and then test*/
				/* again. Declare error if FIFO full    */
				/****************************************/
				for (i = 0; i < 0x10000; i++)
				   val = *((uchar volatile *) delay_ptr);

				wait_cnt++;
				if (wait_cnt > 30)
				{
				      rc = ERROR;
				      gemlog(NULL,ld->component,"wrfifo",
					      "wrfifo",rc,f,UNIQUE_3);
				      break;
				}
			}
	}

	clrjmpx(&jmpbuf);
	BUSMEM_DET(seg_reg);

	return(rc);

}

/********************************************************************/
/* IDENTIFICATION: FIFOCPY                                          */
/*                                                                  */
/* DESCRIPTIVE Name:  This routine copies data from a source to a   */
/*                    target fifo using the specified length.       */
/*                    Integer writes are done until the count length*/
/*                    become zero or less less than 4.              */
/*                                                                  */
/* INPUTS:  Sounce - Pointer to source buffer                       */
/*          Target - Pointer to target fifo                         */
/*          length - Number of bytes to copy                        */
/*                                                                  */
/********************************************************************/
void fifocpy(target,source,len)
     uint *source, *target;
     long len;
{
      volatile long num_char;
      volatile char *char_src, *char_tgt;
      volatile int i;

      /******************************************************************/
      /* calculate the number of character moves that maybe required    */
      /******************************************************************/
      num_char = len % 4;

      /******************************************************************/
      /* Move 4 bytes at a time to the target buffer                    */
      /******************************************************************/
      for (i=0; i < (len / 4); i++)
	    *target = *source++;

      /******************************************************************/
      /* Move 1 byte at a time to the target buffer if necessary        */
      /******************************************************************/
      if (num_char != 0)
      {
	 char_src = (char *) source;
	 char_tgt = (char *) target;
	 for (i=0; i < num_char; i++)
	       *char_tgt = *char_src++;
      }

}

/********************************************************************/
/* IDENTIFICATION: ITOF                                             */
/*                                                                  */
/* DESCRIPTIVE Name:  This routine converts an interger to a single */
/*                    precision floating point value.               */
/*                                                                  */
/* INPUTS:  val    - Interger                                       */
/*          return - Float point value                              */
/*                                                                  */
/********************************************************************/
#define ALLBITS         0xffffffff
#define LO31BITS        0x7fffffff
#define LO7BITS         0x0000007f
#define BIT31           0x80000000
#define FEXPBIAS        0x7f

unsigned long itof(val)
register unsigned int val;

{
	register unsigned int tval, sign, expon;

	sign = val & BIT31;

	if (sign)
		tval = ((val ^ ALLBITS) + 1);    /* 2's comp                 */
	else
		tval = (val & LO31BITS);

	if (tval == 0)                           /* zero is special case     */
		expon = 0;                       /* force exponent to zero   */
	else
	{
		expon = 31;
		while ((tval & BIT31) == 0)      /* find leading one bit     */
		{       expon--;
			tval <<= 1;
		}
		tval <<= 1;
		tval >>= 9;
		expon = ((expon + FEXPBIAS) << 23);
	}

	return (sign | expon | tval);

}


