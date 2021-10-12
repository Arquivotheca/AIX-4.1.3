/* @(#)37  1.6.1.3  src/bos/kernext/disp/ped/inc/midksr.h, peddd, bos411, 9439C411g 10/3/94 15:53:11 */
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: *		ps_to_screen_clear
 *		ps_to_screen_text
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


#ifndef _H_MIDDATA.H
#define _H_MIDDATA.H


#define VTT_ACTIVE      1
#define VTT_INACTIVE    0
#define VTT_MONITORED   0
#define VTT_CHARACTER   1

#include <sys/aixfont.h>
#include <vt.h>

/******************************************************************
*   IDENTIFICATION: MIDKSR.H                                      *
*   DESCRIPTIVE name: Virtual Display Driver (VDD) global internal*
*                     data structures for the Middle adapter      *
*                                                                 *
*   FUNCTION: Declare and initialize data structures that are     *
*             global to all Middle DD KSR procedures.             *
*                                                                 *
******************************************************************/

struct middata {

/**********************************************************************/
/*                                                                    */
/* Virtual Driver Mode: Current mode of the Virtual Device Driver     */
/*                             0 => monitored mode                    */
/*                             1 => charaacter mode (KSR)             */
/*                             2 => APA mode (AVT)                    */
/*                                                                    */
/*                      NOTE: the current state of the real device    */
/*                            (rscr_mode) is stored in the RMA.       */
/*                                                                    */
/*                      NOTE: the default mode is character           */
/*                                                                    */
/**********************************************************************/

    long        vtt_mode;

/**********************************************************************/
/*                                                                    */
/* Virtual Driver State: Current state of the Virtual Display Driver  */
/*                             0 => inactive (presentation space      */
/*                                            updated)                */
/*                             1 => active (frame buffer updated)     */
/*                                                                    */
/*                        NOTE: the default state is inactive.        */
/*                                                                    */
/**********************************************************************/


    long        vtt_active;


/**********************************************************************/
/*                                                                    */
/* Offset into the presentation space.  Rather than scroll the        */
/* contents of the presentation space, the Middle moves a pointer,    */
/* scroll_offset, up and down the presentation space.  This pointer   */
/* points to the beginning of the presentation space data, rather     */
/* than the top of the data structure.                                */
/*                                                                    */
/**********************************************************************/


	short           scroll_offset;

/*--------------------------------------------------------------------*/
/*                                                                    */
/*  The data shown on the screen must be centered.  The centering     */
/*  is done with offsets from the lower left corner of the screen.    */
/*                                                                    */
/*--------------------------------------------------------------------*/

	unsigned short          centering_x_offset;

	unsigned short          centering_y_offset;

/**********************************************************************/
/*                                                                    */
/*  Presentation Space:                                               */
/*                                                                    */
/*                      NOTE: the character in the presentation space */
/*                            is initialized as a "space" with a      */
/*                            "green character/black background"      */
/*                            attribute.                              */
/*                                                                    */
/**********************************************************************/

    unsigned long        *pse ;

/*--------------------------------------------------------------------*/
/*                                                                    */
/*  Presentation Space Size (bytes)                                   */
/*                                                                    */
/*              Contains the total number of bytes in the ps .        */
/*              (width = height = -1 implies the ps is not allocated).*/
/*                                                                    */
/*              NOTE: default value is -1                             */
/*                                                                    */
/*  Presentation Space Size (full characters)                         */
/*                                                                    */
/*              Contains the current width and height of the ps       */
/*              (width = height = -1 implies the ps is not allocated) */
/*              in characters.                                        */
/*                                                                    */
/*              NOTE: default value is -1                             */
/*                                                                    */
/*--------------------------------------------------------------------*/

   long ps_bytes;                     /* number of bytes in ps         */
   long ps_words;                     /* number of words in ps         */

   struct {
	short ht;                    /* ps height (row)               */
	short wd;                    /* ps width (height)             */
   } ps_size;                        /* dimensions of ps              */

/*--------------------------------------------------------------------*/
/*                                                                    */
/* Font Table:                                                        */
/*                                                                    */
/*   An array of FONT_HEAD pointers pointing to the fonts             */
/*   currently being used by the virtual terminal.                    */
/*                                                                    */
/*   Also an array index for the current font.                        */
/*                                                                    */
/*--------------------------------------------------------------------*/

#define         FONT_ID_SIZE            8

   aixFontInfoPtr       AIXfontptr[FONT_ID_SIZE];

/*--------------------------------------------------------------------*/
/*                                                                    */
/*  Color table                                                       */
/*                                                                    */
/*  This is a shadow copy of what is in the color table on the        */
/*  adapter for this virtual terminal.  This is used to offer         */
/*  different color pallettes for each VT.                            */
/*                                                                    */
/*                                                                    */
/*--------------------------------------------------------------------*/

/* data for number of entries in colorpal */
#define         VT_NUM_OF_COLORS                16

	struct colorpal color_table;

/*--------------------------------------------------------------------*/
/*                                                                    */
/*  Character glyph Descriptors                                       */
/*                                                                    */
/*  Since KSR does not support proportional spacing, these values     */
/*  apply for all character glyphs in a given font.                   */
/*  Also, since KSR only supports one font type (size) at a time,     */
/*  these values are for any current font.                            */
/*                                                                    */
/*  Note: only lft_sd can legally be negative: all others are error   */
/*  checked in the code.                                              */
/*                                                                    */
/*--------------------------------------------------------------------*/

    struct {
	short ht;              		/* height of character (pels)    */
	short wd;              		/* width of character (pels)     */
	short lft_sd;			/* char base to left of box      */
	short ascnt;			/* baseline to top of box        */
    } char_box;                         /* dimensions of character box   */


/*--------------------------------------------------------------------*/
/*                                                                    */
/* Cursor Attributes:                                                 */
/*                                                                    */
/*              Characteristics of the cursor prior  to the           */
/*              execution of the next VDD command.                    */
/*                                                                    */
/*              NOTE: if the virtural terminal is active, this        */
/*              field contains the attributes of the cursor that      */
/*              is currently being displayed.                         */
/*                                                                    */
/*--------------------------------------------------------------------*/

    struct {
	unsigned short fg;              /* cursor foreground color       */
	unsigned short bg;              /* cursor background color       */
    } cursor_color;                     /* cursor color                  */

    unsigned  short *cursor_data;

/**********************************************************************/
/*                                                                    */
/* Cursor Shape: width and height of the cursor shape                 */
/*                                                                    */
/*               NOTE: the default value is a DOUBLE underscore       */
/*                                                                    */
/*                                                                    */
/**********************************************************************/

    struct cursr_shape {
	unsigned short      top;
	unsigned short      bot;
	unsigned short      blank;
    } cursor_shape ;

    long cursor_select;
    long cursor_show;


/*--------------------------------------------------------------------*/
/*                                                                    */
/* Cursor Position: character offset into the frame buffer            */
/*                  or index into the presentation space              */
/*                  ( ((row-1) * 80) + col-1 )                        */
/*                                                                    */
/*                  NOTE: the default value is 0 (the upper-left      */
/*                        corner of the screen)                       */
/*                                                                    */
/*--------------------------------------------------------------------*/

    struct cursr_pos {
	int                 col  ;
	int                 row  ;
    } cursor_pos ;

/*--------------------------------------------------------------------*/
/*                                                                    */
/* Screen position of top left of TEXT portion in pels.               */
/*                                                                    */
/*--------------------------------------------------------------------*/

     struct {
	  unsigned short pel_x;         /* pel indentation to tl of     */
	                                /* TEXT portion of screen.      */
	  unsigned short pel_y;         /* pel indentation to tl of     */
     } screen_pos;                      /* TEXT portion of screen.      */

/*--------------------------------------------------------------------*/
/*                                                                    */
/*  Attribute:                                                        */
/*                                                                    */
/*       Contains the foreground color, background color,             */
/*       and the current font.                                        */
/*                                                                    */
/*--------------------------------------------------------------------*/

#define MID_MASK        0x0000FFF0
#define ATTRIASCII(attribute)           ((attribute >> 16) & 0x00ff)
#define ATTRIMID(attribute)             (attribute & MID_MASK)

	unsigned long current_attr;

/*--------------------------------------------------------------------*/
/*                                                                    */
/*  Rendering Context Save Area:                                      */
/*                                                                    */
/*       Contains all information needed to save a rendering          */
/*       context when deactivating a monitor mode virtual terminal    */
/*       and to restore a rendering context when activating a monitor */
/*       mode virtual terminal.                                       */
/*                                                                    */
/*--------------------------------------------------------------------*/

	struct _rcx *virtual_rcx;

/*--------------------------------------------------------------------*/
/* The following array is used to store context pointers into during  */
/* hotkey. Only the 0th entry is used in restoring a context the rest */
/* of the array is used for code clarity only. The array size         */
/* to the number of contexts planned for the adapter to hold.         */
/*--------------------------------------------------------------------*/

	rcxPtr prcx_tosave[16];

/*--------------------------------------------------------------------*/
/* Presentation space array of ascii shorts.			      */
/*--------------------------------------------------------------------*/

	ushort	ary[1280];

};      /* END of middata structure */


struct psf_type                             /* will be initialized */
{                                           /* to 0x20022002       */
  unsigned            ps_char1  : 8 ;       /* character code */
  unsigned            ps_attr1  : 8 ;       /* attribute code */
  unsigned            ps_char2  : 8 ;       /* character code */
  unsigned            ps_attr2  : 8 ;       /* attribute code */
} ;

extern struct psf_type *psf ;

/*------------------------------------------------------------------------*/
/*                                                                        */
/*  Macros to convert presentation space coordinates to MIDDD screen      */
/*  coordinates.  Note: the presentation space is in row and column       */
/*  while the screen coordinates are in x and y. Both originate in the    */
/*  upper left corner.  The presentation space ranges from 1 to 80 cols   */
/*  and 1 to 25 rows in the default case. The screen ranges from 0 to     */
/*  1279 in x and 0 to 1023 in y in all cases. The character data is      */
/*  always centered in the middle of the display.                         */
/*                                                                        */
/*------------------------------------------------------------------------*/

/*------------------------------------------------------------------------*/
/*                                                                        */
/*  ps_to_screen_text()                                                   */
/*                                                                        */
/*  For a detailed discussion of fonts on mid, refer to any good text on  */
/*  X fonts: mid supports X fonts.                                        */
/*  Highlights: characters are drawn relative to their origin, not to     */
/*  any corner. To determine the origin from the upper left corner, add   */
/*  the left side to the x coordinate and the ascent to the y coordinate. */
/*  See the code below. It adds the centering offset as described above   */
/*  as well.                                                              */
/*                                                                        */
/*  Note: KSR fonts are a subset of X fonts: that is, KSR fonts are       */
/*  guaranteed not to be proportionally spaced. This is how a single      */
/*  data structure can be used for all glyphs in a font.                  */
/*                                                                        */
/*                                                                        */
/*------------------------------------------------------------------------*/

#define ps_to_screen_text(ps_row,ps_col, screen_x,screen_y)                   \
{                                                                             \
	BUGLPR(dbg_midtext, 3, ("In ps_to_screen\n"));                        \
	BUGLPR(dbg_midtext, 3, ("row %d col %d\n",ps_row,ps_col));            \
	BUGLPR(dbg_midtext, 3, ("center_x_offset %d center_y_offset %d\n",    \
                 ld->centering_x_offset,ld->centering_y_offset));             \
	BUGLPR(dbg_midtext, 3, ("char_box.wd %d char_box.ht %d\n",            \
                 ld->char_box.wd,ld->char_box.ht));                           \
	BUGLPR(dbg_midtext, 3, ("char_box.lft_sd %d char_box.ascnt %d\n",     \
                 ld->char_box.lft_sd,ld->char_box.ascnt));                    \
	if ((ps_row < 0) || (ps_col < 0))                                     \
	{                                                                     \
		BUGLPR(dbg_midtext, 0, ("Error in ps_to_screen\n"));          \
		BUGLPR(dbg_midtext, 0, ("row %d col %d\n",ps_row,ps_col));    \
		return(-1);                                                   \
	}                                                                     \
	/* here is the beef */						      \
	*screen_x = ld->centering_x_offset + ld->char_box.lft_sd +            \
		(ps_col - 1) * ld->char_box.wd;                               \
	*screen_y = ld->centering_y_offset + ld->char_box.ascnt  +            \
		(ps_row - 1) * ld->char_box.ht;                               \
									      \
	if ((*screen_y < 0) || (*screen_y > (Y_MAX+1))                        \
	||  (*screen_x < 0) || (*screen_x > (X_MAX+1)))                       \
	{                                                                     \
		BUGLPR(dbg_midtext, 0, ("Error in ps_to_screen\n"));          \
		BUGLPR(dbg_midtext, 0, ("row %d col %d\n",ps_row,ps_col));    \
		BUGLPR(dbg_midtext, 0, ("x %d y %d\n",*screen_x,*screen_y));  \
		BUGLPR(dbg_midtext, 0, ("centering x %d centering y %d\n",    \
			ld->centering_x_offset,ld->centering_y_offset));      \
		BUGLPR(dbg_midtext, 0, ("char wid %d char ht %d\n",           \
			ld->char_box.wd,ld->char_box.ht));                    \
		return(-1);                                                   \
	}                                                                     \
									      \
	BUGLPR(dbg_midtext, BUGNFO, ("x %d y %d\n",*screen_x,*screen_y));     \
}

/*------------------------------------------------------------------------*/
/*                                                                        */
/*  ps_to_screen_clear()                                                  */
/*                                                                        */
/*  Clears originate at the upper left corner of the box described.       */
/*  They do not use the ascent and left side parameters necessary for     */
/*  fonts.                                                                */
/*                                                                        */
/*                                                                        */
/*------------------------------------------------------------------------*/

#define ps_to_screen_clear(ps_row,ps_col, screen_x,screen_y)                  \
{                                                                             \
	BUGLPR(dbg_midtext, 3, ("In ps_to_screen\n"));                        \
	BUGLPR(dbg_midtext, 3, ("row %d col %d\n",ps_row,ps_col));            \
	BUGLPR(dbg_midtext, 3, ("center_x_offset %d center_y_offset %d\n",    \
                 ld->centering_x_offset,ld->centering_y_offset));             \
	BUGLPR(dbg_midtext, 3, ("char_box.wd %d char_box.ht %d\n",            \
                 ld->char_box.wd,ld->char_box.ht));                           \
	BUGLPR(dbg_midtext, 3, ("char_box.lft_sd %d char_box.ascnt %d\n",     \
                 ld->char_box.lft_sd,ld->char_box.ascnt));                    \
	if ((ps_row < 0) || (ps_col < 0))                                     \
	{                                                                     \
		BUGLPR(dbg_midtext, 0, ("Error in ps_to_screen\n"));          \
		BUGLPR(dbg_midtext, 0, ("row %d col %d\n",ps_row,ps_col));    \
		return(-1);                                                   \
	}                                                                     \
	/* here is the beef */						      \
	*screen_x = ld->centering_x_offset +                                  \
		(ps_col - 1) * ld->char_box.wd;                               \
	*screen_y = ld->centering_y_offset +                                  \
		(ps_row - 1) * ld->char_box.ht;                               \
									      \
	if ((*screen_y < 0) || (*screen_y > (Y_MAX+1))                        \
	||  (*screen_x < 0) || (*screen_x > (X_MAX+1)))                       \
	{                                                                     \
		BUGLPR(dbg_midtext, 0, ("Error in ps_to_screen\n"));          \
		BUGLPR(dbg_midtext, 0, ("row %d col %d\n",ps_row,ps_col));    \
		BUGLPR(dbg_midtext, 0, ("x %d y %d\n",*screen_x,*screen_y));  \
		BUGLPR(dbg_midtext, 0, ("centering x %d centering y %d\n",    \
			ld->centering_x_offset,ld->centering_y_offset));      \
		BUGLPR(dbg_midtext, 0, ("char wid %d char ht %d\n",           \
			ld->char_box.wd,ld->char_box.ht));                    \
		return(-1);                                                   \
	}                                                                     \
									      \
	BUGLPR(dbg_midtext, BUGNFO, ("x %d y %d\n",*screen_x,*screen_y));     \
}

/* define macros for accessing char_box data
*/

#define MID_DEFAULT_FONT	0
#define MID_FONT_PTR ((aixFontInfoPtr)(font_table[font_index].font_ptr))
#define MID_DEF_PTR  ((aixFontInfoPtr)(font_table[MID_DEFAULT_FONT].font_ptr))

/*
	Define constant used in setting up default context for KSR mode
*/
#define UPPER_LEFT_ORG		0x8000	/* KSR -> 2D so window org is U.L  */ 
#define DISABLE_ID_COMPARE	0x4000  /* disable id comparision, i.e, we */
                                        /* want update WID unconditionally */ 
#define EIGHT_BIT_FILL		0x4000
#define TWENTY4_BIT_FILL	0x8000
#define UPDATE_DRAW_FRAME_BUFFER	0x4000 	/* set bit 14 */ 
#define UPDATE_DISPLAY_FRAME_BUFFER	0x2000  /* set bit 13 */
#define BUFFER_A_DRAW_SELECTION		0x2000	/* set bits 13 & 12 to 10 */

#define TRUE	-1 
#define FALSE	0 

#endif
