static char sccsid[] = "@(#)60	1.6.1.7  src/bos/kernext/disp/ped/ksr/midtext.c, peddd, bos411, 9439C411g 10/3/94 15:46:21";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: copy_ps
 *		max
 *		pack_string
 *		set_text_image
 *		vttclr
 *		vttcpl
 *		vttrds
 *		vtttext
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
#define KSR

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
#include "hw_PCBrms.h"
#include "hw_FIFrms.h"
#include "hw_FIF2dm1.h"
#include "hw_typdefs.h"
#include "hw_errno.h"
#include "hw_regs_u.h"
#include "hw_regs_k.h"

#include "mid_dd_trace.h"
MID_MODULE ( midtext );        /* defines trace variable                    */
BUGXDEF(dbg_middd);

#define max(val1,val2)          (val1 > val2) ? val1 : val2;

/*---------------------------------------------------------------------------*/
/* pack_string() 							     */
/* Description - mid ucode needs characters in shorts presentation space     */
/* has data in longs. This function packs P.S. data into correct format.     */
/*---------------------------------------------------------------------------*/

#define pack_string(char_ary,strt_row,strt_col,len,ld)			      \
{									      \
	ushort i;							      \
	ulong	*buf_adr;						      \
	ulong	buf_offst;						      \
	buf_adr = ld->pse;						      \
	buf_offst = (strt_row * ld->ps_size.wd) + strt_col;	      	      \
	for (i = 0;i < len;i++)						      \
	{								      \
		char_ary[i] = ATTRIASCII(*(buf_adr +			      \
		   ((buf_offst + ld->scroll_offset) % ld->ps_words)));	      \
		buf_offst ++;						      \
		/* printf("%c",(char)(char_ary[i])); */			      \
	}								      \
	/* printf("\n"); */						      \
}


/*---------------------------------------------------------------------------*/
/* set_text_image()							     */
/* Description - Check the font attributes to see if they are different      */
/* than what is on the adapter. If one is different, send the commands to    */
/* the adapter to change it and update the attribute data structure.         */
/*---------------------------------------------------------------------------*/

set_text_image(vp,new_attributes)
struct vtmstruc * vp;
ushort new_attributes;		/* lower 16 bits of presentation sp data */
{
	ulong fontid;                       /* font's addr with 4 MS 
                                               bit turned on */
	ushort 	
			new_fg_col,
			new_bg_col,
			new_font_index,
			cur_fg_col, 
			cur_bg_col, 
                        cur_font_index;  /* index into font palette */

	struct middata * ld = vp->vttld;
	struct midddf *ddf = (struct midddf *)vp->display->free_area;


	HWPDDFSetup;			/* gain access to hardware pointer */


	BUGLPR(dbg_midtext,1, ("Entering set_text_image\n"));

	/*------------------------------------------------------------*/
	/* Current font, forground and background color               */
	/* Note: These three ATTRI macros handle all shifts.	      */
	/* To see these, look in /com/sysx/inc/vt.h.                  */
	/*------------------------------------------------------------*/

	cur_fg_col     = ATTRIFORECOL(ld->current_attr);
	cur_bg_col     = ATTRIBAKCOL(ld->current_attr);
	cur_font_index = ATTRIFONT(ld->current_attr); 

	BUGLPR(dbg_midtext,1, 
		("set_text_image: current fg=%d, bg=%d, font index=%d\n", 
		cur_fg_col,cur_bg_col, cur_font_index));

	new_fg_col     = ATTRIFORECOL(new_attributes);
	new_bg_col     = ATTRIBAKCOL(new_attributes);
	new_font_index = ATTRIFONT(new_attributes); 

	BUGLPR(dbg_midtext,1, 
		("set_text_image: new fg=%d, bg=%d, font index=%d\n", 
		new_fg_col,new_bg_col, new_font_index));

	/* ---------------- 
           update local data to reflect new font, foregound and 
           background colors 
        ------------------- */

 	if( cur_fg_col != new_fg_col)
	{
	   SET_ATTRIFORECOL(ld->current_attr,new_fg_col);
	   MID_SetForegroundPixel(new_fg_col);
	   BUGLPR(dbg_midtext,1, 
		("set_text_image: current fg is changed to=%d\n",
		ATTRIFORECOL(ld->current_attr)));
	} 

 	if( cur_bg_col != new_bg_col)
	{
	   SET_ATTRIBAKCOL(ld->current_attr,new_bg_col);
	   MID_SetBackgroundPixel(new_bg_col);
  	   BUGLPR(dbg_midtext,1, 
		("set_text_image: current bg is changed to=%d\n",
		ATTRIBAKCOL(ld->current_attr)));
	} 

	
 	if( cur_font_index != new_font_index)
	{
	   SET_ATTRIFONT(ld->current_attr,new_font_index);

	   /* ---------------- 
              font id will be a unique number used to micro-code to 
              identify different fonts.  For KSR mode, the id will 
              be the virtual address of where the font is in memory.
              Since the 4 MS bits of the address will be always zero  
              (kernel data is in segment zero) and we need a scheme
              to differentiate X font id from KSR font id when we
              have to service a font fault, the 4 MS bit of the 
              address of KSR font are set to 1's.
	   ---------------- */
 
	   fontid = 0xf0000000 | 
                         (ulong) (ld->AIXfontptr[new_font_index]);

	   BUGLPR(dbg_midtext,1, ("set_text_image: SetActiveFont id=%x\n",
                                                     fontid));
	   MID_SetActiveFont(fontid,fontid);
	} 

	BUGLPR(dbg_midtext,1, ("exiting set_text_image\n"));
}


/*-------------------------------------------------------------------*/
/*   IDENTIFICATION: COPY_PS					     */
/*                                                                   */
/*   DESCRIPTIVE NAME:  Copy Presentation Space                      */
/*                                                                   */
/*   FUNCTION: Copies the presentation space of the Virtual Display  */
/*	       Driver (VDD) into the refresh buffer of the adapter.  */
/*	       It also initializes the adapter's cursor shape,	     */
/*	       and cursor postion.				     */
/*								     */
/*   PSEUDO CODE:                                                    */
/*                                                                   */
/*              Calculate the true screen starting address (call     */
/*              ps_to_screen_text()                                  */
/*                                                                   */
/*              For each row of the presentation space:              */
/*                                                                   */
/*                  - Pack characters into shorts                    */
/*                                                                   */
/*                  - check the character attributes & set it        */
/*                                                                   */
/*                  - Write string to the adapter                    */
/*                                                                   */
/*   INPUTS:    *vp     the pointer to the virtual terminal          */
/*                      data structure, defined in vt.h.             */
/*                                                                   */
/*   CALLED BY: activate                                             */
/*              init (only if VT is active)                          */
/*								     */
/*   CALLS:     ps_to_screen_text()                                  */
/*								     */
/*-------------------------------------------------------------------*/

ulong copy_ps(vp)
struct vtmstruc *vp;
{
ulong		*buf_addr;		/* presentation space address	   */
long		buf_offset;		/* offset into the output buffer   */
long		row;			/* row loop variable 		   */
long		col;			/* col loop variable 		   */
ushort		screen_x,		/* screen x value for string start */
      		start_screen_x,		/* screen x value for line start   */
		screen_y;		/* screen y value for line start   */
ulong 		ps_char;                /* 32 bit char in present. space   */
ushort 		cur_font,               /* font index into font palette    */
      		cur_fg_col,
      		cur_bg_col;
ushort		temp_attribute;		/* current string's attribute      */
ushort		this_chars_attribute;	/* current char's attribute        */
ushort       	sub_str_len,            /* len of string of characters     */
                                        /* whose have same font, fg, bg    */
		startcol;		/* column where string starts      */
struct middata  *ld;			/* ptr to local data area	   */

struct midddf 	*ddf = (struct midddf *)vp->display->free_area;


	HWPDDFSetup;			/* gain access to hardware pointer */



	BUGLPR(dbg_midtext, 1, ("Entering copy_ps\n"));

	/*---------------------------------*/
	/* set the local data area pointer */
	/*---------------------------------*/
	ld = (struct middata *)vp->vttld;

	/*-----------------------------------------------------------------*/
	/* calculate frame buffer and presentation buffer starting address */
	/*-----------------------------------------------------------------*/
	buf_addr = ld->pse;
	buf_offset = 0;

	
	/*----------------------------------------*/
	/* calculate true screen starting address */
	/*----------------------------------------*/
	ps_to_screen_text(1, 1, &screen_x, &screen_y);   
	start_screen_x = screen_x;

        MID_DD_ENTRY_TRACE ( midtext, 1, COPY_PS, ddf,
                0,
                buf_addr,
                screen_x,
                screen_y );


	PIO_EXC_ON();

	for (row = 0; row < ld->ps_size.ht; row++)
	/*-----------------------------------------------*/
	/*  the number of rows in the presentation space */
	/*-----------------------------------------------*/
	{
		/*-----------------------------------------------*/
		/* Initialize the variables that may be reset    */
		/* during the for loop.				 */
		/*-----------------------------------------------*/

		startcol = 0;			
		sub_str_len = 0;

		/*-----------------------------------------------*/
		/* Set up the first string's attributes          */
		/*-----------------------------------------------*/

		ps_char = (*(buf_addr + 
			   ((buf_offset + ld->scroll_offset) % ld->ps_words)));
		temp_attribute = ATTRIMID(ps_char); 

		for    (col = 0;
			col < ld->ps_size.wd;
			col++,buf_offset++)
        	{

			/*-----------------------------------------------*/
			/* Set up this characters attributes. For the    */
			/* first character, this will always match the   */
			/* temp_attribute, since buf_offset has not yet  */
			/* been incremented.				 */
			/*-----------------------------------------------*/

	   		ps_char = (*(buf_addr + 
                           ((buf_offset + ld->scroll_offset) % ld->ps_words)));
			this_chars_attribute = ATTRIMID(ps_char); 

			/*-----------------------------------------------*/
			/* Compare the two attributes. In almost all     */
			/* cases they will match. When they don't, the   */
			/* current string is complete and is sent to the */
			/* microcode for drawing. 			 */
			/*-----------------------------------------------*/
			
			if (temp_attribute != this_chars_attribute)
			{
				/*---------------------------------------*/
				/* Set up the string for the ucode.      */
				/*---------------------------------------*/

				pack_string(ld->ary,row,startcol,sub_str_len,ld);

				/*---------------------------------------*/
				/* Set the font and colors in the ucode. */
				/*---------------------------------------*/

				set_text_image(vp,temp_attribute);
				
				/*---------------------------------------*/
				/* Send the string to the ucode.         */
				/*---------------------------------------*/

				MID_Imagetext16(screen_x,screen_y,
					sub_str_len, ld->ary);

				/*---------------------------------------*/
				/* Reset string variables.               */
				/*---------------------------------------*/

				startcol = col;
				screen_x += (ld->char_box.wd * sub_str_len);
				sub_str_len = 0;
				temp_attribute = this_chars_attribute;
				
			}

			/*---------------------------------------------------*/
			/* Increment current number of chars in this string. */
			/*---------------------------------------------------*/

			sub_str_len++;
        	}

		/*-----------------------------------------------*/
		/* Through with this row. Print it.              */
		/*-----------------------------------------------*/

		
		/*---------------------------------------*/
		/* Set up the string for the ucode.      */
		/*---------------------------------------*/

		pack_string(ld->ary,row,startcol,sub_str_len,ld);

		/*---------------------------------------*/
		/* Set the font and colors in the ucode. */
		/*---------------------------------------*/

		set_text_image(vp,temp_attribute);
		
		/*---------------------------------------*/
		/* Send the string to the ucode.         */
		/*---------------------------------------*/

		MID_Imagetext16(screen_x,screen_y,
			sub_str_len, ld->ary);

		/*---------------------------------------*/
		/* Reset string variables.               */
		/*---------------------------------------*/

		screen_y += ld->char_box.ht;
		screen_x = start_screen_x;

	}

	PIO_EXC_OFF();

        MID_DD_EXIT_TRACE ( midtext, 1, COPY_PS, ddf,
                0,
                start_screen_x,
                screen_x,
                screen_y );

	BUGLPR(dbg_midtext, 1, ("Leaving copy_ps with rc = 0.\n"));

}	/* end of copy_ps */


/*---------------------------------------------------------------------*/
/*								       */
/* IDENTIFICATION: VTTCLR					       */
/*								       */
/* DESCRIPTIVE NAME: Clear Rectangle				       */
/*								       */
/* FUNCTION: This function clears a rectangular area on the screen.    */
/*	     It can clear any size rectangle, and can clear any area   */
/*	     on the screen that is screen aligned.		       */
/*								       */
/*	     The following are the input constraints:		       */
/*								       */
/*	     1) This function is valid only in character mode.	       */
/*	     2) The start row, start column, end row, and end	       */
/*		column must all be valid for the current	       */
/*		presentation space.				       */
/*								       */
/* PSEUDO CODE: 						       */
/*                                                                     */
/*            IF not character (KSR) mode                              */
/*               - return                                              */
/*                                                                     */
/*            Set foreground and background colors                     */
/*                                                                     */
/*            Set character attributes:                                */
/*               - IF no display                                       */
/*                   - set foreground color equal to background color  */
/*               - Set the new font                                    */
/*                                                                     */
/*	      Build a structure containing:			       */
/*		 - the character bits for blank			       */
/*                                                                     */
/*            Calculate margins of rectangle to clear                  */
/*                                                                     */
/*	      For each row in the rectangle			       */
/*		 - For each column in the rectangle		       */
/*		      - Clear the presentation space element	       */
/*								       */
/*	      If the virtual terminal is active 		       */
/*		  update the frame buffer ( call copy_ps() )	       */
/*								       */
/*								       */
/* INPUTS:    *vp	   the pointer to the virtual terminal	       */
/*			   data structure, defined in vt.h.	       */
/*	      *sp	   A pointer to a structure of type	       */
/*			   vtt_box_rc_parms, defined in vt.h,	       */
/*			   which contains upper left - lower right     */
/*			   rectangle information.		       */
/*	      attr	   The character attributes, accessed	       */
/*			   using the ATTR macros, defined in vt.h      */
/*	      cursor_show  boolean: should the cursor be moved	       */
/*								       */
/*								       */
/* OUTPUTS:   INVALID_RECTANGLE 				       */
/*	      INVALID_MODE					       */
/*								       */
/* CALLED BY: This routine is called by the Virtual Terminal Mode      */
/*	      Processor.  It is an entry point. 		       */
/*								       */
/*								       */
/* CALLS:     This routine calls vttmovc(), if necessary.	       */
/*	      It also calls copy_ps(), to update the adapter,	       */
/*	      if necessary.					       */
/*								       */
/*								       */
/*---------------------------------------------------------------------*/

long vttclr(vp, sp, attr, cursor_show)
struct vtmstruc    *vp;                 /* virtual terminal struct ptr  */
struct vtt_box_rc_parms *sp;            /* upper-left and lower-right   */
	                                /* corners of the rectangle     */
ulong             attr;                 /* character attribute          */
ulong             cursor_show;          /* if true cursor is moved to   */
	                                /* pos specified in cp_parms    */

{

ulong	ch_attr,	       /* Color char and attr codes	       */
	*buf_addr;	       /* output buffer offset		       */
long	buf_offset,	       /* offset into the output buffer        */
	buf_begin,	       /* starting offset into the output buf  */
	buf_end,	       /* ending offset into the output buf    */
	height, 	       /* height of the rectangle	       */
	i;
short	screen_lr_x,
	screen_lr_y;
MIDDimensionRectangle box;	/* contains area to be cleared         */
ushort	temp;
struct	middata *ld;	       /* ptr to local data area	       */

struct	midddf *ddf = (struct midddf *)vp->display->free_area;

	HWPDDFSetup;	       /* gain access to hardware pointer      */

        MID_DD_ENTRY_TRACE ( midtext, 1, VTTCLR, ddf,
                0,
                sp,
                attr,
                cursor_show );


	VDD_TRACE(CLR, ENTRY, vp);
	
	BUGLPR(dbg_midtext, BUGNTA, ("Entering vttclr \n"));



	/*------------------------------------*/
	/* set the local data area pointer    */
	/*------------------------------------*/

	ld = (struct middata *)vp->vttld;


	if (ld->vtt_mode != KSR_MODE)
	/*------------------------------------*/
	/* not in character mode	      */
	/*------------------------------------*/
	{
		/*------------------------------------*/
		/* Only valid in character (KSR) mode */
		/*------------------------------------*/


		BUGLPR(dbg_midtext, 0, ("vttclr called in MOM!\n"));
		return(INVALID_MODE);
	}


	/*----------------------------------------*/
	/* Build the HIPRF3D attribute using attr.*/
	/*----------------------------------------*/

	temp = 0;

	{
	SET_ATTRIFORECOL(temp,ATTRIFORECOL(attr));
	SET_ATTRIBAKCOL(temp,ATTRIBAKCOL(attr));
	}

	if (ATTRIBLA(attr))
	/*---------------*/
	/* no display	 */
	/*---------------*/
	{
		/*------------------------------------*/
		/* set foreground equal to background */
		/*------------------------------------*/

		SET_ATTRIFORECOL(temp,ATTRIBAKCOL(temp));
	}


	/*---------------------------------------------------------*/
	/* set the new font					   */
	/*---------------------------------------------------------*/

	SET_ATTRIFONT(temp,ATTRIFONT(attr));


	/*---------------------------------------------------------*/
	/* OR a blank character with appropriate attributes        */
	/*---------------------------------------------------------*/

	ch_attr = (0x00200000 | temp);		  /* or in a blank */


	/*---------------------------------------------------------*/
	/* clear a rectangular area in the presentation space	   */
	/*---------------------------------------------------------*/

	buf_addr = (ulong *)ld->pse;


	/*---------------------------------------------------------*/
	/* calculate beginning and ending offsets within each line */
	/*---------------------------------------------------------*/

	buf_begin = (sp->row_ul - 1) * ld->ps_size.wd + (sp->column_ul - 1);
	buf_end   = buf_begin + sp->column_lr - sp->column_ul;


	/*---------------------------------------------------------*/
	/* calculate the height of the rectangle		   */
	/*---------------------------------------------------------*/

	height = (sp -> row_lr) - (sp -> row_ul) + 1;

	for (i = 0; i < height; i++)
	/*---------------------------*/
	/* each row in the rectangle */
	/*---------------------------*/
	{
		/*----------------------------------*/
		/* clear the row		    */
		/*----------------------------------*/

		for (buf_offset = buf_begin;
			buf_offset <= buf_end; buf_offset++)
		/*----------------------------------*/
		/* each character in the row	    */
		/*----------------------------------*/
		{
			/*----------------------------------*/
			/* display the character as a space */
			/* with the specified attribute     */
			/*----------------------------------*/

			*(buf_addr + ((buf_offset + ld->scroll_offset) %
				ld->ps_words)) = ch_attr;
		}


		/*----------------------------------*/
		/* set up to clear the next row     */
		/* by incrementing the begin and end*/
		/* markers by the pres. space width */
		/*----------------------------------*/
	
		buf_begin += ld->ps_size.wd;
		buf_end   += ld->ps_size.wd;
	}


	if (ld->vtt_active)
	/*--------------------------------*/
	/* the virtual terminal is active */
	/*--------------------------------*/
	{
		/*---------------------------------------------*/
		/* update the frame buffer                     */
		/*---------------------------------------------*/

		BUGLPR(dbg_midtext, 1,("vttclr calling ps_to_screen_clear.\n"));
		/* compute the boundaries in screen space */
		ps_to_screen_clear(sp->row_ul,sp->column_ul,
			&box.rulx,&box.ruly);
		BUGLPR(dbg_midtext, 1,("vttclr calling ps_to_screen_clear.\n"));
		ps_to_screen_clear(sp->row_lr + 1,sp->column_lr + 1,
			&screen_lr_x,&screen_lr_y);
		box.rwth = screen_lr_x - box.rulx;
		box.rht = screen_lr_y - box.ruly;
		

		/* open the bus */
		PIO_EXC_ON();

		/* clear the pixels */
		BUGLPR(dbg_midtext, 2, ("vttclr calling clear_box.\n"));
		BUGLPR(dbg_midtext, 2, ("Clear to color : %d.\n",
			ATTRIBAKCOL(temp)));
		clear_box(ddf,vp,&box,ATTRIBAKCOL(temp));

		/* close the bus */
		PIO_EXC_OFF();

	}



	VDD_TRACE(CLR, EXIT, vp);

        MID_DD_EXIT_TRACE ( midtext, 1, VTTCLR, ddf,
                0,
                ch_attr,
                buf_addr,
                height );

	BUGLPR(dbg_midtext, BUGNTA, ("Leaving vttclr with rc = 0.\n"));
	return(0);

}					/* end of vttclr	       */



/*---------------------------------------------------------------------*/
/*								       */
/* IDENTIFICATION: VTTCPL					       */
/*								       */
/* DESCRIPTIVE NAME: Copy Partial Line				       */
/*								       */
/* FUNCTION: vttcpl is effectively an implementation of horizontal     */
/*           scroll in that it copies a portion of the specified line  */
/*	     to another position on the SAME line.  This operation     */
/*	     can be done on as many lines as desired.		       */
/*								       */
/*	     The following are the input constraints:		       */
/*								       */
/*	     1) This function is valid only in character mode.	       */
/*	     2) The start row, start column, end row, and end	       */
/*		column must all be valid for the current	       */
/*		presentation space.				       */
/*								       */
/* PSEUDO CODE: 						       */
/*								       */
/*           IF not character (KSR) mode, return                       */
/*                                                                     */
/*	     IF the source & destination colums are the same	       */
/*	        - return: the copy is trivially successful	       */
/*                                                                     */
/*	     Calculate the start offset for the source & destination   */
/*								       */
/*	     Calculate the number of characters to copy		       */
/*								       */
/*	     Calculate the number of lines to copy		       */
/*								       */
/*	     IF the source is to the right of the destination	       */
/*	        - copy from left to right			       */
/*	     ELSE						       */
/*	        - copy from right to left			       */
/*								       */
/*	     For all lines to copy				       */
/*		- For all characters to copy			       */
/*		     - Copy the characters from the source column      */
/*		       to the destination column		       */
/*								       */
/*	      If the virtual terminal is active 		       */
/*		  update the frame buffer ( call copy_ps() )	       */
/*								       */
/*	      If cursor_show != 0				       */
/*		   move the cursor ( call vttmovc() )		       */
/*								       */
/*								       */
/* INPUTS:    *vp	   The pointer to the virtual terminal	       */
/*			   data structure, defined in vt.h.	       */
/*	      *rc	   A pointer to a structure of type	       */
/*			   vtt_rc_parms, defined in vt.h, which        */
/*			   contains string length, string start index, */
/*			   start row, start column, end row, and       */
/*			   end column. NOTE: since this copy is from   */
/*			   left to right the end row is not a	       */
/*			   destination but rather a count of how       */
/*			   many rows to move.			       */
/*	      cursor_show  boolean: should the cursor be moved	       */
/*								       */
/* OUTPUTS:   INVALID MODE					       */
/*								       */
/* CALLED BY: This routine is called by the Virtual Terminal Mode      */
/*	      Processor.  It is an entry point. 		       */
/*								       */
/*								       */
/* CALLS:     This routine calls vttmovc(), if necessary.	       */
/*	      It also calls copy_ps(), to update the adapter,	       */
/*	      if necessary.					       */
/*								       */
/*---------------------------------------------------------------------*/

long vttcpl(vp, rc, cursor_show)
struct vtmstruc    *vp;                 /* virtual terminal struct ptr  */
struct vtt_rc_parms *rc;                /* string position and length   */
ulong               cursor_show;        /* if true cursor is moved to   */
	                                /* the pos given in cp_parms    */
{


	ulong	*buf_addr;	/* address of the buffer	       */
	long	s_offset,	/* starting offset of the source string*/
		d_offset,	/* starting offset of the target string*/
		length, 	/* number of words to be moved	       */
		slength,	/* number of words to be moved	       */
		no_lines_m1,	/* number of lines to copy minus 1     */
		factor; 	/* move direction indicator	       */
	short	screen_x_ll,
		screen_y_ll,
		screen_x_ur,
		screen_y_ur,
		newx,
		newy;
	struct		middata *ld;	     /* ptr to local data area */

	struct midddf *ddf = (struct midddf *)vp->display->free_area;

	HWPDDFSetup;		    /* gain access to hardware pointer */

	VDD_TRACE(CPL, ENTRY, vp);

        MID_DD_ENTRY_TRACE ( midtext, 1, VTTCPL, ddf,
                0,
                vp,
                rc,
                cursor_show );

	BUGLPR(dbg_midtext, BUGNTA, ("Entering vttcpl \n"));



	/*------------------------------------*/
	/* set the local data area pointer    */
	/*------------------------------------*/

	ld = (struct middata *)vp->vttld;


	if (ld->vtt_mode != KSR_MODE)
	/*------------------------------------*/
	/* not in character mode	      */
	/*------------------------------------*/
	{
		/*------------------------------------*/
		/* Only valid in character (KSR) mode */
		/*------------------------------------*/

		BUGLPR(dbg_midtext, 0, ("vttcpl called in MOM!\n"));
		return(INVALID_MODE);
	}

	/*-----------------------------------------------------------*/
	/* If the source and destination are not the same, then      */
	/* a real copy is required, Else source and destination are  */
	/* the same so no copy is required.                          */
	/*-----------------------------------------------------------*/

	if (rc -> dest_column != rc -> start_column)
	/*-----------------------------------------------------------*/
	/* the source address does not equal the destination address */
	/*-----------------------------------------------------------*/
	{
		/*------------------------------------------------------*/
		/* copy the indicated line segments as requested	*/
		/*------------------------------------------------------*/

		buf_addr = (ulong *)ld->pse;


		/*------------------------------------------------------*/
		/* calculate the starting offset of the first character */
		/* to be moved in the source and destination strings	*/
		/*------------------------------------------------------*/

		s_offset = (rc->start_row - 1) * ld->ps_size.wd +
			(rc->start_column - 1) ;
		d_offset = (rc->start_row - 1) * ld->ps_size.wd +
			(rc->dest_column - 1) ;


		/*-----------------------------------------------------------*/
		/* calculate number of characters to be copied taking into   */
		/* account truncation when the destination is to right of    */
		/* the source. NOTE: truncation is not possible when the     */
		/* destination is to the left of the source because the      */
		/* minimum column number is 1				     */
		/*-----------------------------------------------------------*/

		slength = ld->ps_size.wd - rc->dest_column + 1;
		if (slength > (rc -> string_length))
			slength = rc -> string_length;


		/*-----------------------------------------------------------*/
		/* calculate the number of lines to be copied		     */
		/*-----------------------------------------------------------*/

		no_lines_m1 = (rc -> dest_row) - (rc -> start_row);


		if (rc->dest_column < rc->start_column)
		/*-----------------------------------------------------------*/
		/* the destination is to the left of the source 	     */
		/*-----------------------------------------------------------*/
		{
			/*-----------*/
			/* copy left */
			/*-----------*/

			factor = 1;
		}
		else
		{
			/*---------------------------------------------------*/
			/* copy right to left avoiding source and dest	     */
			/* overlap problems				     */
			/*---------------------------------------------------*/

			factor = -1;


			/*---------------------------------------------------*/
			/* source and destination offsets have now increased */
			/* by the string length since we are copying from    */
			/* right to left.                                    */
			/*---------------------------------------------------*/

			s_offset += slength - 1;
			d_offset += slength - 1;
		}


		for (; no_lines_m1 >= 0; no_lines_m1--)
		/*---------------------------------*/
		/* all lines that are to be copied */
		/*---------------------------------*/
		{
			/*----------------------------------------------*/
			/* copy the next line segment			*/
			/*----------------------------------------------*/

			for (length = slength; length > 0; length--)
			/*----------------------------------------------*/
			/* all characters in the line segment		*/
			/*----------------------------------------------*/
			{
				/*-------------------------------------------*/
				/* copy the character and its corresponding  */
				/* attribute code			     */
				/*-------------------------------------------*/

				*(buf_addr + ((d_offset + ld->scroll_offset) %
					ld->ps_words)) =
				*(buf_addr + ((s_offset + ld->scroll_offset) %
					ld->ps_words));


				/*-------------------------------------------*/
				/* increment if copying from left to right   */
				/* decrement if copying from right to left   */
				/*-------------------------------------------*/

				d_offset += factor;
				s_offset += factor;
			}

			/*----------------------------------------------*/
			/* Reset source and destination offset values   */
			/* after "copy"                                 */
			/*----------------------------------------------*/
			s_offset += ld->ps_size.wd - (slength * factor);
			d_offset += ld->ps_size.wd - (slength * factor);

		}       /* end of if source not equal to destination    */


		if (ld->vtt_active)
		/*--------------------------------*/
		/* the virtual terminal is active */
		/*--------------------------------*/
		{
			/*-------------------------*/
			/* update the frame buffer */
			/*-------------------------*/

			copy_ps(vp);

		}

	}  /* source row != dest row */



	ld->cursor_show = cursor_show;

	if (ld->cursor_show)
	/*--------------------------*/
	/* the cursor must be moved */
	/*--------------------------*/
	{
		/*------------------*/
		/* move the cursor  */
		/*------------------*/
		vttmovc(vp);
	}


	VDD_TRACE(CPL, EXIT, vp);

        MID_DD_EXIT_TRACE ( midtext, 1, VTTCPL, ddf,
                0,
                slength,
                no_lines_m1,
                factor );


	BUGLPR(dbg_midtext, BUGNTA, ("Leaving vttcpl with rc = 0.\n"));

	return(0);

}				      /* end of vttcpl		      */

/*---------------------------------------------------------------------*/
/*								       */
/* IDENTIFICATION: VTTRDS					       */
/*								       */
/* DESCRIPTIVE NAME: Read Screen Segment			       */
/*								       */
/* FUNCTION:  Read the display and attribute codes for a given         */
/*	      segment of the presentation space.                       */
/*	      code and a two-byte attribute code.  The attr codes      */
/*	      are returned in attr, the display codes are returned     */
/*	      in ds.						       */
/*								       */
/* PSEUDO CODE: 						       */
/*                                                                     */
/*            IF not in character (KSR) mode                           */
/*               - return                                              */
/*                                                                     */
/*	      Calculate the address of the first character to read     */
/*								       */
/*	      For each character to read			       */
/*		 - Read the character				       */
/*		 - Transform the character and attribute into the      */
/*                 appropriate codes                                   */
/*		 - Put the codes into the appropriate data structures  */
/*								       */
/* INPUTS:    *ds	   A pointer of type unsigned short, the       */
/*			   display codes are returned here.	       */
/*	      *attr	   A pointer of type unsigned short, the       */
/*			   attr codes are returned here.	       */
/*	      ds_size	   A long, it holds the size of the ds	       */
/*			   array.				       */
/*	      attr_size    A long, it holds the size of the ds	       */
/*			   array.				       */
/*	      *rc	   A pointer to a structure of type	       */
/*			   vtt_rc_parms, (defined in		       */
/*			   /usr/lib/samples/vttcmd.h), which	       */
/*			   contains string length, string start index, */
/*			   start row, start column, end row, and       */
/*			   end column.				       */
/*								       */
/* OUTPUTS:   INVALID MODE					       */
/*								       */
/* CALLED BY: This routine is called by the Virtual Terminal Mode      */
/*	      Processor.  It is an entry point. 		       */
/*								       */
/*								       */
/* CALLS:     None.						       */
/*								       */
/*---------------------------------------------------------------------*/

long vttrds(vp, ds,ds_size,attr,attr_size,rc)
struct vtmstruc *vp;                    /* virtual terminal struct ptr  */
ushort          *ds;                    /* array of display symbols     */
					/* returned by this procedure   */
long            ds_size;                /* size of ds array             */
ushort          *attr;                  /* array of attributes returned */
					/* by this procedure            */
long            attr_size;              /* size of attr array           */
struct vtt_rc_parms *rc;                /* string position and length   */
{


ulong	*buf_addr,
	current_attr;
long	i,
	buf_offset;
struct	middata *ld;			/* ptr to local data area      */

struct midddf *ddf = (struct midddf *)vp->display->free_area;


        MID_DD_ENTRY_TRACE ( midtext, 1, VTTRDS, ddf,
                0,
                vp,
                ds,
                ds_size );

	VDD_TRACE(RDS, ENTRY, vp);
	
	BUGLPR(dbg_midtext, BUGNTA, ("Entering vttrds \n"));


	/*---------------------------------*/
	/* set the local data area pointer */
	/*---------------------------------*/

	ld = (struct middata *)vp->vttld;


	if (ld->vtt_mode != KSR_MODE)
	/*---------------------------------*/
	/* not in character mode	   */
	/*---------------------------------*/
	{
		/*------------------------------------*/
		/* Only valid in character (KSR) mode */
		/*------------------------------------*/
		BUGLPR(dbg_midtext,0,("In vttrds and KSR_MODE is false\n"));

		return(INVALID_MODE);
	}


	/*-----------------------------*/
	/* read the presentation space */
	/*-----------------------------*/

	buf_addr = (ulong *)ld->pse;


	/*----------------------------------------------------------------*/
	/* calculate the address of the first character that must be read */
	/*----------------------------------------------------------------*/

	buf_offset = ((rc->start_row - 1) * ld->ps_size.wd +
	(rc->start_column - 1));


	for (i=0; i < (rc->string_length); i++)
	/*--------------------------------------------------------*/
	/* all characters that must be read from the frame buffer */
	/*--------------------------------------------------------*/
	{
		/*-----------------------------------------------------------*/
		/* read a character from the buffer and convert the character*/
		/* code into a two-byte display code and the attribute into a*/
		/* two-byte attribute code				     */
		/*-----------------------------------------------------------*/

		current_attr = *(ulong *)(buf_addr +
			((buf_offset + ld->scroll_offset) % ld->ps_words));
		buf_offset++;

		/*-----------------------------------------------------------*/
		/* transform the character code into a 2-byte code	     */
		/*-----------------------------------------------------------*/
		/*-----------------------------------------------------------*/
		/* the display code equals the character code		     */
		/*-----------------------------------------------------------*/

		ds[i] = (ushort)ATTRIASCII(current_attr);


		/*-----------------------------------------------------------*/
		/* this used to have extra code to convert codes <31 to >256 */
		/* We don't need that here, I'll just keep two bytes per char*/
		/*-----------------------------------------------------------*/

		/*-----------------------------*/
		/* set the attribute bytes (2) */
		/*-----------------------------*/

		attr[i] = (ushort)ATTRIMID(current_attr);

	}	/* end of for loop */

	VDD_TRACE(RDS, EXIT, vp);

        MID_DD_EXIT_TRACE ( midtext, 1, VTTRDS, ddf,
                0,
                attr,
                attr_size,
                rc );


	BUGLPR(dbg_midtext, BUGNTA, ("Leaving vttrds with rc = 0.\n"));

	return(0);

}				 /* end of vttrds		*/


/*---------------------------------------------------------------------*/
/*								       */
/* IDENTIFICATION: VTTTEXT					       */
/*								       */
/* DESCRIPTIVE NAME: Draw Text					       */
/*								       */
/* FUNCTION:  Write a string of ASCII characters into the              */
/*            presentation space.  If the virtual terminal is          */
/*            active, update the frame buffer for this display.	       */
/*								       */
/* PSEUDO CODE: 						       */
/*                                                                     */
/*            IF not in character (KSR) mode                           */
/*               - return                                              */
/*								       */
/*	      IF reverse video					       */
/*		 - reverse background and foreground colors	       */
/*                                                                     */
/*            IF no display                                            */
/*               - set foreground color equal to background color      */
/*                                                                     */
/*            Set font                                                 */
/*								       */
/*	      Calculate the starting and ending offsets for the        */
/*            input string within the presentation space.     	       */
/*								       */
/*	      For each character to write			       */
/*		 - Put the character and its attributes into the       */
/*		   presentation space				       */
/*								       */
/*	      IF the virtual terminal is active 		       */
/*		 - update the frame buffer ( call MID_ImageText2D )    */
/*								       */
/*	      IF cursor_show is true 			               */
/*		 - move the cursor ( call vttmovc() )		       */
/*								       */
/*								       */
/* INPUTS:    *string	   A character pointer containing an array     */
/*			   of ASCII characters. 		       */
/*	      *rc	   A pointer to a structure of type	       */
/*			   vtt_rc_parms, (defined in		       */
/*			   /usr/lib/samples/vttcmd.h), which	       */
/*			   contains string length, string start index, */
/*			   start row, start column, end row, and       */
/*			   end column.				       */
/*	      *cp	   A pointer to a structure of		       */
/*			   type vtt_cursor (defined in		       */
/*			   /usr/lib/samples/vttcmd.h), which	       */
/*			   contains the new cursor x and y	       */
/*			   position.				       */
/*	      cursor_show  boolean: should the cursor be moved	       */
/*								       */
/* OUTPUTS:   INVALID MODE					       */
/*								       */
/* CALLED BY: This routine is called by the Virtual Terminal Mode      */
/*	      Processor.  It is an entry point. 		       */
/*								       */
/*								       */
/* CALLS:     This routine calls vttmovc(), if necessary.  It also     */
/*	      calls copy_ps(), to update the display's frame	       */
/*	      buffer, if the virtual terminal is active.	       */
/*								       */
/*---------------------------------------------------------------------*/

long vtttext(vp, string, rc, cp, cursor_show)
struct vtmstruc     *vp;                      /* virtual terminal struct ptr  */
char                *string;                  /* array of ascii characters    */
struct vtt_rc_parms *rc;                      /* string position and length   */
struct vtt_cp_parms *cp;                      /* code point base/mask and     */
	                                      /* new cursor position          */
ulong               cursor_show;              /* if true cursor is moved to   */
	                                      /* the pos given in cp_parms    */
{


ulong	*buf_addr,			/* output buffer offset 	 */
	temp_attr;			/* hold the curr char and attr	 */
long	buf_offset,			/* offset into the output buffer */
	this_char,			/* offset of the char to be	 */
					/* displayed			 */
	last_char;			/* offset of last char to be	 */
					/* displayed			 */
ushort	screen_x,			/* true x coordinate		 */
	screen_y;			/* true y coordinate		 */
struct	middata     *ld;		/* ptr to local data area	 */
ushort	str_len;			/* temp string length	         */
ushort	num_rows;			/* # of rows written to display  */

struct	midddf *ddf = (struct midddf *)vp->display->free_area;

	HWPDDFSetup;			/* gain access to adapter	 */

	VDD_TRACE(TEXT, ENTRY, vp);

        MID_DD_ENTRY_TRACE ( midtext, 1, VTTTEXT, ddf,
                0,
                vp,
                string,
                rc );


	BUGLPR(dbg_midtext, BUGNTA, ("Entering vtttext \n"));


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

		BUGLPR(dbg_midtext, 0, ("vtttext called in MOM!\n"));
		return(INVALID_MODE);
	}



	/*---------------------------------------------------------*/
	/* calculate the attribute that should be stored with each */
	/* character of the text string 			   */
	/*---------------------------------------------------------*/

	temp_attr = 0x0000;


	if (ATTRIRV(cp->attributes))
	/*---------------*/
	/* reverse video */
	/*---------------*/
	{
		/*-------------------------------------------*/
		/* exchange background with foreground color */
		/*-------------------------------------------*/

		BUGLPR(dbg_midtext, 2, ("Reverse video\n"));
		SET_ATTRIFORECOL(temp_attr,ATTRIBAKCOL(cp->attributes));
		SET_ATTRIBAKCOL(temp_attr,ATTRIFORECOL(cp->attributes));
	}
	else
	{
		SET_ATTRIFORECOL(temp_attr,ATTRIFORECOL(cp->attributes));
		SET_ATTRIBAKCOL(temp_attr,ATTRIBAKCOL(cp->attributes));
	}


	if (ATTRIBLA(cp->attributes))
	/*---------------*/
	/* no display	 */
	/*---------------*/
	{
		/*-------------------------------------------*/
		/* make foreground equal to background color */
		/*-------------------------------------------*/

		BUGLPR(dbg_midtext, 2, ("blank attribute\n"));
		SET_ATTRIFORECOL(temp_attr,ATTRIBAKCOL(cp->attributes));
	}


	/*---------------------------------------------------------*/
	/* set the new font					   */
	/*---------------------------------------------------------*/

	SET_ATTRIFONT(temp_attr,ATTRIFONT(cp->attributes));


	/*-------------------------------*/
	/* update the presentation space */
	/*-------------------------------*/

	buf_addr = ld->pse;

	BUGLPR(dbg_midtext, BUGNTA,
	("rc->start_row %d ld->ps_size.wd %d rc->start_column %d\n",
	rc->start_row,ld->ps_size.wd,rc->start_column));

	buf_offset = ((rc->start_row - 1) * ld->ps_size.wd +
	(rc->start_column - 1));


	/*---------------------------------------------------------*/
	/* calculate starting and ending offsets into input string */
	/*---------------------------------------------------------*/

	this_char = rc->string_index - 1;
	last_char = this_char + rc->string_length;

	for (; this_char < last_char; this_char++, buf_offset++)
	/*------------------------------------*/
	/* all characters in the input string */
	/*------------------------------------*/
	{
		/*----------------------------------*/
		/* copy them into the output buffer */
		/*----------------------------------*/

		*(buf_addr + ((buf_offset + ld->scroll_offset) % ld->ps_words))
		= (ulong) (((string[this_char] & cp->cp_mask) + cp->cp_base)
		<< 16) | temp_attr;
	}



	if (ld->vtt_active)
	/*--------------------------------*/
	/* the virtual terminal is active */
	/*--------------------------------*/
	{
		/*------------------------------------*/
		/* draw the actual text on the screen */
		/*------------------------------------*/

		/*-----------------------------------------------------------*
		  Set the font and colors in the ucode. 

		  Note that the bus must be enabled AND disabled here since the
		  ps_to_screen_text macro has a conditional return inside it.
		 *-----------------------------------------------------------*/
		PIO_EXC_ON();

		set_text_image(vp,temp_attr);
		
		PIO_EXC_OFF();

		/*---------------------------------------------------------*/
		/* Convert presentation space coodinates to screen         */
		/* coordinates and pack these longs into shorts.  Micro-   */
		/* code requires shorts,  presentation space uses longs.   */
		/*---------------------------------------------------------*/

		ps_to_screen_text(rc->start_row,rc->start_column,&screen_x,
			&screen_y);

		BUGLPR(dbg_midtext, BUGGID, ("x %d y %d wid %d\n",
				screen_x,screen_y,rc->string_length));

		pack_string(ld->ary,(rc->start_row - 1),(rc->start_column - 1),
			rc->string_length,ld);

		BUGLPR(dbg_midtext, BUGNFO, ("vtttext image text call\n"));

		PIO_EXC_ON();

		MID_Imagetext16(screen_x,screen_y,(rc->string_length),ld->ary); 

		PIO_EXC_OFF();
	}

	ld->cursor_show = cursor_show;

	if (ld->cursor_show == cursor_show)
	/*--------------------------*/
	/* the cursor must be moved */
	/*--------------------------*/
	{
		/*-----------------*/
		/* move the cursor */
		/*-----------------*/
		vttmovc(vp);
	}

	VDD_TRACE(TEXT, EXIT, vp);

        MID_DD_EXIT_TRACE ( midtext, 1, VTTTEXT, ddf,
                0,
                cp,
                cursor_show,
                ld->pse );


	BUGLPR(dbg_midtext, BUGNTA, ("Leaving vtttext with rc = 0.\n"));

	return(0);

}				 /* end  of  vtttext		 */

/*---------------------------------------------------------------------*/
