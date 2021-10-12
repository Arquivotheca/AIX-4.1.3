static char sccsid[] = "@(#)10	1.70.1.12  src/bos/kernext/disp/sky/vtt2drvr.c, sysxdispsky, bos411, 9436D411a 9/6/94 18:46:08";
/*
 *   COMPONENT_NAME: SYSXDISPSKY
 *
 *   FUNCTIONS: SELECT_BLINK
 *		SELECT_BRIGHT
 *		SELECT_NEW_BGCOL
 *		SELECT_NEW_FGCOL
 *		SELECT_NEW_FONT
 *		SELECT_NO_DISP
 *		SELECT_UNDERSCORE
 *		SET_ATTRIBUTES
 *		async_mask
 *		change_cursor_shape
 *		copy_ps
 *		draw_char
 *		draw_text
 *		getport
 *		if
 *		load_font
 *		load_palette
 *		reset_color
 *		reset_mono
 *		set_character_attributes1283
 *		sky_close
 *		sky_config
 *		sky_define
 *		sky_dev_init
 *		sky_dev_term
 *		sky_enable_event4331
 *		sky_ioctl
 *		sky_make_gp
 *		sky_open
 *		sky_pxblt
 *		skyway_reset
 *		sync_mask
 *		vttact
 *		vttcfl
 *		vttclr
 *		vttcpl
 *		vttdact
 *		vttddf
 *		vttdefc
 *		vttdma
 *		vttinit
 *		vttmovc
 *		vttrds
 *		vttscr
 *		vttsetm
 *		vttstct
 *		vttterm
 *		vtttext
 *		vtttpwrphase
 *		while
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/iocc.h>
#include <sys/adspace.h>
#include <sys/mdio.h>
#include <sys/conf.h>
#include <sys/intr.h>
#include <sys/uio.h>
#include <sys/dma.h>
#include <sys/pin.h>
#include <sys/malloc.h>
#include <sys/device.h>
#include <sys/except.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/errids.h>
#include <sys/errno.h>
#include <sys/systm.h>
#define Bool unsigned
#include <sys/aixfont.h>
#include <vt.h>
#include <sys/display.h>
#include <sys/disptrc.h>
#include "vtt2ddf.h"
#include "vtt2def.h"
#include "ddsent.h"
#include "vtt2ldat.h"
#include "skyras.h"	/* error codes for skyway */


#define SELECT_NEW_FGCOL(ld,char_attr)		\
 ld->colset = FALSE;                             \
 ld->fgcol = ((char_attr << 16) >> 28);

#define SELECT_NEW_BGCOL(ld,char_attr)		 \
 ld->colset = FALSE;                             \
 ld->old_bgcol = ld->bgcol;			 \
 ld->bgcol = ((char_attr << 20) >> 28);

#define SELECT_NEW_FONT(vp,char_attr,tmp)   \
 tmp = ((char_attr << 24) >> 29);	\
 tmp = tmp & 0x00000007; \
 if (tmp != ld->cur_font)   \
 {					  \
   load_font(vp,tmp);			  \
   ld->cur_font = tmp;	     \
 }





#define SELECT_NO_DISP(char_attr,tmp)	 \
 tmp = ((char_attr << 27) >> 31);

#define SELECT_BRIGHT(char_attr,tmp) \
 tmp = ((char_attr << 28) >> 31);

#define SELECT_BLINK(char_attr,tmp) \
 tmp = ((char_attr << 29) >> 31);

#define SELECT_UNDERSCORE(char_attr,tmp) \
 tmp = ((char_attr << 31) >> 31);

#define SET_ATTRIBUTES(attr,tmp_attr)				   \
 tmp_attr = 0x0000;						   \
 if (attr & 0x0002)  /* in reverse video mode */		   \
  {								     \
  tmp_attr = ((attr & 0xf000) >> 4) | ((attr & 0x0f00) << 4) |	  \
	      (attr & 0x00ff);					  \
  }								   \
 else								   \
  tmp_attr =  attr;						   \




#define SCRN_WIDTH			80
#define SCRN_HEIGHT			25
#define X_RES                           1280
#define Y_RES                           1024


#define SPACE_A 0x00200000	/* ASCII code for a space in the high	  */
				/* bytes, low bytes reserved for the	  */
				/* character attribute			  */
#define BLANKS_NULL_ATTR  0x00200000	/* one blank with null attrib	*/




/*----------------------------------------------------------------------*/
/* Function prototypes							*/
/*----------------------------------------------------------------------*/
/*  The local functions start on or about line 200. */
/*  The global functions start on or about line 1143. */

static void change_cursor_shape();
static void copy_ps();
static void draw_char();
static void draw_text();
static void load_font();
static void load_palette();
static void reset_color();
static void reset_mono();
static void select_new_bg();
static void select_new_fg();
static void select_new_font();
static void set_bold_attr();
static long set_character_attributes();
static void set_underline_attr();
static void sky_pxblt();
static void skyway_reset();
static void load_font_table();


static long vttact();
static long vttcfl();
static long vttclr();
static long vttcpl();
static long vttdact();
static long vttddf();
static long vttdefc();
static long vttdma();
static long vttinit();
static long vttmovc();
static long vttrds();
static long vttterm();
static long vtttext();
static long vttscr();
static long vttsetm();
static long vttstct();
static long vttpwrphase();
static long sync_mask();
static long async_mask();
long sky_open();
long sky_init();
long sky_close();
long sky_ioctl();
extern int sky_intr();
extern void cda_timeout();
extern long sky_dev_init();
extern long sky_dev_term();
extern int sky_err();

struct t_ps_s {
   long   ps_w,ps_h;
};

uint	      tempful;
uchar	      tempbyte;

#define SKYERR 1

/*----------------------------------------------------------------*
*	    Start of local functions				  *
*----------------------------------------------------------------*/


/***********************************************************************/
/*								       */
/* IDENTIFICATION: CHANGE_CURSOR_SHAPE				       */
/*								       */
/* DESCRIPTIVE NAME: Change cursor to requested shape		       */
/*								       */
/* FUNCTION:  This routine sets the SRAM for the cursor in the SKYWAY  */
/*	      adapter to conform to the requested shape.	       */
/*								       */
/*	      The code first sets up the cursor colors using the       */
/*	      foreground color as the default.			       */
/*								       */
/*	      A loop is executed setting the SRAM to an invisible      */
/*	      display for each row until the index equals the	       */
/*	      specified top line of the visible cursor. 	       */
/*								       */
/*	      A second loop sets the character box width worth of pels */
/*	      to cursor color 1 and the remaining pel data for the row */
/*	      to invisible.					       */
/*								       */
/*	      Once the rows actually containing the cursor data have   */
/*	      been set the remaining SRAM area is set to invisible.    */
/*								       */
/* INPUTS:    top, bottom - ushorts defining the cursor shape	       */
/*								       */
/* OUTPUTS:   NONE.						       */
/*								       */
/* CALLED BY: VTTDEFC						       */
/*								       */
/* CALLS:     None.						       */
/*								       */
/***********************************************************************/

static void change_cursor_shape(vp, top, bottom)
struct vtmstruc *vp;
ushort		top,
		bottom;   /* top and bottom line numbers in cursor */

{
ushort		bytecnt,count1,count2;	    /* work variables */

union {
  ulong full;
  unsigned char bdat[4];
} workword;

char volatile *rptr,*dptr;
struct skyway_data *ld;
int parityrc;
struct ddsent *dds;
label_t jmpbuf;
caddr_t base_addr;

dds = (struct ddsent *) vp->display->odmdds;

ld = (struct skyway_data *) vp->vttld;

/* Set parity handler */
parityrc = setjmpx(&(jmpbuf));
if (parityrc != 0)
{
   if (parityrc == EXCEPT_IO)
   {
      ld->iop      = (ulong)ld->iop & (~ (ulong)base_addr);        /* offset + base_addr - base_addr */ 
      ld->io_idata = (ulong)ld->io_idata & (~ (ulong)base_addr);   /* offset + base_addr - base_addr */ 

      BUSMEM_DET(base_addr);

      /* Log an error and return */
      sky_err(vp,"SKYDD","vtt2drvr","setjmpx", 
              parityrc, SKY_SETJMP, SKY_UNIQUE_1);
      return;
   }
   else
      longjmpx(parityrc);
}


base_addr = BUSMEM_ATT(BUS_ID,0);   /* set up a seg. reg to do io to bus mem space */

ld->iop      = (ulong)ld->iop | (ulong)base_addr;         /* offset + base_addr */ 
ld->io_idata = (ulong) ld->io_idata | (ulong) base_addr;   /* offset + base_addr */ 

/* initialize the pointers to the index and data_b registers */
rptr = (char *) &( ld->iop->index );
dptr = (char *) &( ld->iop->data_b );


/* Turn off display of cursor */

/* Access and initialize the Brooktree command register.	*/
/* Specifics are on p. 30 of Addendum document. 		*/
*ld->io_idata = 0x6006;
*ld->io_idata = 0x6443;


*ld->io_idata = SPRITE_ADR_LO_P0 ;    /* Trun on command function */
*ld->io_idata = SPRITE_CNTRL_P0 | CURSOR_OFF;	/* 6a04 */

if (top == 255) 		  /* Cursor should be blanked out */
{

/* Set up index registers on the adapter to point to first sprite byte. */
   *ld->io_idata = SPRITE_ADR_LO_P0;
   *ld->io_idata = SPRITE_ADR_HI_P0;

   for (count1 = 0; count1 < LAST_CURSOR_BYTE; count1++)
	  /* clear out all of cursor memory */
   {
      *ld->io_idata = SPRITE_IMAGE_P0 | INVIS_DATA;
      *ld->io_idata = SPRITE_IMAGE_P1 | INVIS_DATA;
   }

   ld->iop      = (ulong)ld->iop & (~ (ulong)base_addr);        /* offset + base_addr - base_addr */ 
   ld->io_idata = (ulong)ld->io_idata & (~ (ulong)base_addr);   /* offset + base_addr - base_addr */ 

   BUSMEM_DET(base_addr);

   clrjmpx(&(jmpbuf));
   return;
}


/* Set up index registers on the adapter to point to first sprite byte. */
*ld->io_idata = SPRITE_ADR_LO_P0 ;
*ld->io_idata = SPRITE_ADR_HI_P0 ;
bytecnt = 0;
while ( bytecnt < (top * BYTES_CURSOR_ROW))  /* Loop until first defined */
{					     /* cursor row		 */
   *ld->io_idata = SPRITE_IMAGE_P0 | INVIS_DATA;
   *ld->io_idata = SPRITE_IMAGE_P1 | INVIS_DATA;
   bytecnt++;
}


*ld->io_idata = 0x6003;


*rptr = 0x38;
*dptr = 0xff; /*workword.bdat[1]; */
*dptr = 0xff; /*workword.bdat[2]; */
*dptr = 0xff; /*workword.bdat[3]; */

for (; top <= bottom; top++)
{
   count1 = ld->Vttenv.character_box.width;
   count2 = 0;

   do			/* Loop sending a byte of cursor data at a time  */
   {			/* to the adapter. Switch provides mechanism to  */
			/* align non byte aligned cursor data		 */
      switch (count1)
      {
	 case 1: /* Only one pel left to set */
		 *ld->io_idata = SPRITE_IMAGE_P0 | ONE_CSR_PEL;
		 *ld->io_idata = SPRITE_IMAGE_P1 | ONE_CSR_PEL;
		 count1 = 0;
		 break;

	 case 2: /* Two pels left to set */
		 *ld->io_idata = SPRITE_IMAGE_P0 | TWO_CSR_PEL;
		 *ld->io_idata = SPRITE_IMAGE_P1 | TWO_CSR_PEL;
		 count1 = 0;
		 break;

	 case 3: /* Three pels left to set */
		 *ld->io_idata = SPRITE_IMAGE_P0 | THREE_CSR_PEL;
		 *ld->io_idata = SPRITE_IMAGE_P1 | THREE_CSR_PEL;
		 count1 = 0;
		 break;

	 case 4: /* Set four pels */
		 *ld->io_idata = SPRITE_IMAGE_P0 | FOUR_CSR_PEL;
		 *ld->io_idata = SPRITE_IMAGE_P1 | FOUR_CSR_PEL;
		 count1 = 0;
		 break;

	 case 5: /* Three pels left to set */
		 *ld->io_idata = SPRITE_IMAGE_P0 | FIVE_CSR_PEL;
		 *ld->io_idata = SPRITE_IMAGE_P1 | FIVE_CSR_PEL;
		 count1 = 0;
		 break;

	 case 6: /* Set four pels */
		 *ld->io_idata = SPRITE_IMAGE_P0 | SIX_CSR_PEL;
		 *ld->io_idata = SPRITE_IMAGE_P1 | SIX_CSR_PEL;
		 count1 = 0;
		 break;

	 case 7: /* Set seven pels */
		 *ld->io_idata = SPRITE_IMAGE_P0 | SEVEN_CSR_PEL;
		 *ld->io_idata = SPRITE_IMAGE_P1 | SEVEN_CSR_PEL;
		 count1 = 0;
		 break;

	 default: /* Set eight pels */
		 *ld->io_idata = SPRITE_IMAGE_P0 | EIGHT_CSR_PEL;
		 *ld->io_idata = SPRITE_IMAGE_P1 | EIGHT_CSR_PEL;
		 count1 -= 8;
		 break;
      }  /* end of switch */

      count2 += 1;
      bytecnt ++;

   } while ( count1 > 0 );	  /* loop if more pel data */

   for (; count2 < BYTES_CURSOR_ROW; count2++) /* This loop sets the data */
   {					       /* past the width of cursor*/
      *ld->io_idata = SPRITE_IMAGE_P0 | INVIS_DATA;
      *ld->io_idata = SPRITE_IMAGE_P1 | INVIS_DATA;
      bytecnt++;
   }

}   /* end loop for the lines within the cursor block */


/* Now set the data below the requested cursor definition area */
/* to the end of the cursor SRAM			       */

for (; bytecnt < LAST_CURSOR_BYTE; bytecnt++)
{
   *ld->io_idata = SPRITE_IMAGE_P0 | INVIS_DATA;
   *ld->io_idata = SPRITE_IMAGE_P1 | INVIS_DATA;
}


*ld->io_idata = SPRITE_ADR_LO_P0 ;    /* Turn on command function */
*ld->io_idata = SPRITE_CNTRL_P0 | CURSOR_ON;   /* 6c44 */

ld->iop      = (ulong)ld->iop & (~ (ulong)base_addr);        /* offset + base_addr - base_addr */ 
ld->io_idata = (ulong)ld->io_idata & (~ (ulong)base_addr);   /* offset + base_addr - base_addr */ 

BUSMEM_DET(base_addr);
clrjmpx(&(jmpbuf));

}					   /* End change cursor shape */



/***********************************************************************/
/*								       */
/* IDENTIFICATION: COPY_PS					       */
/*								       */
/* DESCRIPTIVE NAME: Copy contents of the presentation space to screen */
/*								       */
/* FUNCTION: Copies the presentation space of the Virtual Display      */
/*	     Driver (VDD) into the refresh buffer of the adapter.      */
/*	     It also initializes the adapter's cursor shape,	       */
/*	     and cursor postion.				       */
/*								       */
/* INPUTS:   Virtual terminal pointer				       */
/*	     Flag indicating whether ok to skip draw of blanks	       */
/*								       */
/* OUTPUTS:   NONE.						       */
/*								       */
/* CALLED BY: vttcpl						       */
/*	      vttcfl						       */
/*								       */
/* CALLS:     draw_text 					       */
/*								       */
/***********************************************************************/


static void copy_ps(vp,draw_blanks)
struct vtmstruc *vp;
char draw_blanks;
{

ulong		*buf_addr;		/* presentation space address	  */
long		buf_offset;		/* offset into the output buffer  */
register long	row;
struct skyway_data *ld;

   ld = (struct skyway_data *) vp->vttld;

   /*****************************************************************
    * calculate frame buffer and presentation buffer start addresses *
    *****************************************************************/
   buf_addr = ld->Vttenv.pse;

   buf_offset = 0;

   for (row = 0; row < ld->Vttenv.ps_size.ht; row++)
   {
      /* send the string */
      draw_text(vp,(buf_addr + ((buf_offset + ld->Scroll_offset) % 
                ld->Vttenv.ps_words)),(ushort)ld->Vttenv.ps_size.wd, 
                0,(ushort)row, FALSE, draw_blanks);

      buf_offset += ld->Vttenv.ps_size.wd;
   }

}				 /* end of copy_ps		 */

/***********************************************************************/
/*								       */
/* IDENTIFICATION: DRAW_CHAR					       */
/*								       */
/* DESCRIPTIVE NAME: Draw a character to the skyway adapter.	       */
/*								       */
/* FUNCTION:  This is an example or what the comment blocks should     */
/*	      look like in the VDD code. The words in this block will  */
/*	      give a detailed description of what the module does.     */
/*								       */
/* CALLED BY: Draw_text 					       */
/*								       */
/* CALLS:     sky_pxblt 					       */
/*								       */
/***********************************************************************/

static void draw_char(vp,x,y,char_offset)
struct vtmstruc *vp;

ushort	x,y,char_offset;

{

  aixGlyphPtr charptr;	      /* workarea pointer to character bitmap  */
  ulong indexptr;	/* workarea pointer to character index		*/
  char * pixptr;			  /* pointer to pixel map	*/
  long tempfull;
  struct skyway_data *ld;

  ld = (struct skyway_data *) vp->vttld;


  /* Calculate offset to the bit mapped character image */
  indexptr =  (aixCharInfoPtr) ld->fontcindex[char_offset].byteOffset;

  /* Use index ptr to get address of actual glyph map */
  charptr = (aixGlyphPtr)((ulong)ld->Font_map_start + (ulong)indexptr);


  /* Convert to vram address */

  pixptr = (char *) charptr;

  /* wait for other operations to complete */
  if (ld->poll_type)
     while(ld->cop->pollreg > 0x7f);
  else
     while(ld->cop->pi_control > 0x7f);


  /* Set up pixel map B as the source map */

  ld->cop->pix_index = PIX_MAP_B;	      /* Set up Pixel Map B	   */

  ld->cop->pixmap_base = pixptr;	      /* pass base address of VRAM */

  if (!(ld->hwset))
  {
     ld->cop->pix_hw = ((ld->Vttenv.character_box.height - 1) << 16) | 
                         ld->Vttenv.font_box_width;
     ld->cop->pixmap_fmat = PIX_FMAT_1BPP;
     ld->hwset = TRUE;
  }

  /* Pixel map is established do a Pxblt to send character  */

  tempfull = POBackReg | POForeReg | POStepBlt | PODestA | POPatB ;

  /* Now call the blt routine and send the character over	   */

  sky_pxblt( ld, 0, 0, x, y, ld->Vttenv.character_box.width-1,
  ld->Vttenv.character_box.height-1, ld->fgcol , ld->bgcol, 3, 3, tempfull);

}


/***********************************************************************/
/*								       */
/* DESCRIPTION OF MODULE:					       */
/*								       */
/* IDENTIFICATION :   draw_text 				       */
/*								       */
/* DESCRIPTIVE NAME:  Draw text to entry  adapter.		       */
/*								       */
/*								       */
/* FUNCTION:  Loops through passed in string searching for substrings  */
/*	      with identical attributes. Draws characters on at a time */
/*	      with calls to draw_char.				       */
/*								       */
/* INPUTS:   Virtual terminal pointer				       */
/*	     Pointer to string					       */
/*	     String length					       */
/*	     X and Y position to draw char			       */
/*	     Flag for whether attributes are constant		       */
/*	     Flag indicating whether ok to skip draw of blanks	       */
/*								       */
/* OUTPUTS:   NONE.						       */
/*								       */
/* CALLED BY: copy_ps() 					       */
/*	      vtttext() 					       */
/*								       */
/* CALLS:  draw_char						       */
/*								       */
/*								       */
/***********************************************************************/


static void draw_text(vp, string, string_length, x, y,
		      constant_attributes,draw_blanks)

struct vtmstruc *vp;
ulong			string[];
ushort			string_length, x, y;
long			constant_attributes;
char			draw_blanks;
{

    long		j,tmp_attr, tmp_char,
			characters_written,
			attributes_change;

    ushort		current_character, cur_x, cur_y;
    short		characters_to_draw ;
    struct skyway_data *ld;
    int parityrc;
    struct ddsent *dds;
    label_t jmpbuf;
    caddr_t base_addr;

    dds = (struct ddsent *) vp->display->odmdds;

    ld = (struct skyway_data *) vp->vttld;

    attributes_change = FALSE;
    characters_written = 0;
    cur_x = (x * ld->Vttenv.character_box.width) + ld->center_x;
    cur_y = (y * ld->Vttenv.character_box.height) + ld->center_y;

    if (constant_attributes)
    {
       current_character = 0;
       tmp_attr = string[current_character] & 0x0000FFFF;
       if (ld->Vttenv.current_attr
		  != tmp_attr)
       {
	  /******************************************
	  * set the new character attributes......  *
	  ******************************************/
	  set_character_attributes(vp,string[current_character]);
       }

       /* Now that attributes are established send string out */

       /* Set parity handler */

       parityrc = setjmpx(&(jmpbuf));
       if (parityrc != 0)
       {
	  if (parityrc == EXCEPT_IO)
	  {
	     BUSMEM_DET(base_addr);
             ld->cop = (ulong)ld->cop & (~ (ulong)base_addr);     /* base + offset - base */

	     /* Log an error and return */
	     sky_err(vp,"SKYDD","vtt2drvr","setjmpx",parityrc, 
                     SKY_SETJMP, SKY_UNIQUE_2);
	     return;
	  }
	  else
	     longjmpx(parityrc);
       }

       base_addr = BUSMEM_ATT(BUS_ID,0);
       ld->cop = (ulong)ld->cop | (ulong)base_addr;     /* base + offset */

       for (j = current_character; j < string_length; j++)
       {
	  tmp_char = string[j] >> 16;
	  if (draw_blanks)
	     draw_char(vp, cur_x, cur_y, tmp_char);
	  else	/* Not drawing blanks */
	  {
	     if (tmp_char != 0x20)  /* not space */
				    /* have to draw   */
		draw_char(vp, cur_x, cur_y, tmp_char);

	    else      /* Char is a blank if different bg must draw */
	    {
	       if (ld->bgcol != ld->old_bgcol)
		  draw_char(vp, cur_x, cur_y, tmp_char);
	    }
	  }
	  cur_x += ld->Vttenv.character_box.width;
       }

       ld->cop = (ulong)ld->cop & (~ (ulong)base_addr);     /* base + offset - base */
       BUSMEM_DET(base_addr);

       clrjmpx(&(jmpbuf));

    }

    else
    {
	/* This code loops until it finds a character with changed     */
	/* attributes. When it finds one it writes out all non changed */
	/* characters then changes the attributes on the requested     */
	/* character resets the change flag and continues.	       */

	 do
	 {   /* non-constant attributes */

	    for (current_character = characters_written;
		 current_character < string_length;
		 current_character++)
	    {
	       tmp_attr = (string[current_character] << 16) >> 16;
	       if  (tmp_attr != ld->Vttenv.current_attr )
	       {
		  attributes_change = TRUE;
		  break;
	       }
	    }

	    characters_to_draw = current_character - characters_written;
	    if ( characters_to_draw > 0 )
	    {
	       /* Set parity handler */

	       parityrc = setjmpx(&(jmpbuf));
	       if (parityrc != 0)
	       {
		  if (parityrc == EXCEPT_IO)
		  {
	     	     BUSMEM_DET(base_addr);
             	     ld->cop = (ulong)ld->cop & (~ (ulong)base_addr);     /* base + offset - base */

		     /* Log an error and return */
		     sky_err(vp,"SKYDD","vtt2drvr","setjmpx", 
                             parityrc, SKY_SETJMP, SKY_UNIQUE_3);
		     return;
		  }
		  else
		     longjmpx(parityrc);
	       }

	       base_addr = BUSMEM_ATT(BUS_ID,0);
               ld->cop = (ulong)ld->cop | (ulong)base_addr;     /* base + offset */

	       /* write string of characters to adapter */
	       for (j = characters_written; j < current_character; j++)
	       {
		  tmp_char = string[j] >> 16;
		  if (draw_blanks)
		     draw_char(vp, cur_x, cur_y, tmp_char);
		  else	/* Not drawing blanks */
		  {
		     if (tmp_char != 0x20)   /* not space */
					    /* have to draw   */
			draw_char(vp, cur_x, cur_y, tmp_char);

		     else /* Char is a blank if different bg must draw */
		     {
			if (ld->bgcol != ld->old_bgcol)
			   draw_char(vp, cur_x, cur_y, tmp_char);
		     }
		  }
		  cur_x += ld->Vttenv.character_box.width;
	      }

              ld->cop = (ulong)ld->cop & (~ (ulong)base_addr);     /* base + offset - base */
	      BUSMEM_DET(base_addr);

	      clrjmpx(&(jmpbuf));

	    }


	    if (attributes_change && current_character < string_length)
	    {
		/******************************************
		* set the new character attributes......  *
		******************************************/

	       set_character_attributes(vp,string[current_character]);
	       attributes_change = FALSE;
	    }
	    characters_written = current_character ;

	} while ( characters_written < string_length );
    }  /* End of the else attributes change block */

}		/* end of draw text */



/************************************************************************/
/*									*/
/* DESCRIPTION OF MODULE:						*/
/*									*/
/* IDENTIFICATION :   LOAD_FONT()					*/
/*									*/
/* DESCRIPTIVE NAME:  Load fonts into the adapter.			*/
/*									*/
/*									*/
/* FUNCTION: This function is used as a hybrid version to the method	*/
/*	     of loading characters onto the adapter as they are used.	*/
/*	     This routine will take the used table and load all those	*/
/*	     characters down to the adapter when it is activated. This	*/
/*	     will make the copying of the presentation space quicker.	*/
/*	     The used table will be stepped through checking for non-	*/
/*	     zero entries. A non-zero entry will contain		*/
/*							 a one which	*/
/*	     indicates the character is on the adapter at the same	*/
/*	     offset at which it lies within the font table.		*/
/*							 an offset from */
/*	     the start of a packed character map. This method will	*/
/*	     allow the VDD to BLT the packed map out of the adapter	*/
/*	     when deactivated and restore it with one operation when	*/
/*	     the terminal is reactivated.				*/
/*									*/
/* FUNCTION: This function is used to load the entire glyph portion of	*/
/*	     the font table onto the adapter with one block transfer.	*/
/*	     The dimensions of the table are calculated and the proper	*/
/*	     registers set in the adapter. The actual PxBlt is done	*/
/*	     with a call to sky_pxblt.					*/
/*									*/
/* INPUTS:   Virtual Terminal pointer					*/
/*	     The ID of the fort to load on the adapter			*/
/*									*/
/* OUTPUTS:  Down loaded font on the adapter.				*/
/*									*/
/* CALLED BY: activate							*/
/*									*/
/* CALLS:    sky_pxblt to perform a pixel block transfer.		*/
/*									*/
/************************************************************************/

static void load_font(vp,id)
struct vtmstruc *vp;
short id;
{
 long i,cnt;
 struct skyway_data *ld;
 long volatile *sp,*dp,delay;
 struct ddsent *dds;
 int parityrc;
 label_t jmpbuf;
 caddr_t base_addr;

  dds = (struct ddsent *) vp->display->odmdds;


 ld =  (struct skyway_data *) vp->vttld;
 /* Set parity handler */
 ld->hwset = FALSE;

 parityrc = setjmpx(&(jmpbuf));
 if (parityrc != 0)
 {
    if (parityrc == EXCEPT_IO)
    {
       BUSMEM_DET(base_addr);
       ld->iop = (ulong)ld->iop & (~ (ulong)base_addr );   /* base + offset - base */

       /* Log an error and return */
       sky_err(vp,"SKYDD","vtt2drvr","setjmpx", parityrc, 
               SKY_SETJMP, SKY_UNIQUE_4);
       return;
    }
    else
       longjmpx(parityrc);
 }

 base_addr = BUSMEM_ATT(BUS_ID,0);
 ld->iop = (ulong)ld->iop | (ulong)base_addr;   /* base + offset */

 dds = (struct ddsent *) vp->display->odmdds;

 /* Set up pointers to font parts */
 ld->fonthptr = (aixFontInfoPtr) ld->Vttenv.font_table[id].addr;

 ld->fontcindex = (aixCharInfoPtr)
	      ((ulong)(ld->fonthptr) + BYTESOFFONTINFO(ld->fonthptr));

 ld->glyphptr = (aixGlyphPtr)
	    ((ulong)(ld->fontcindex) + BYTESOFCHARINFO(ld->fonthptr));

 ld->iop->mem_acc_mode = 0x08;

 sp = (long *) ld->glyphptr ;

 if (ld->disp_type == SKY_COLOR)
 {
    dp = dds->vram_start + COLOR_FONT_OFFSET + (ulong)base_addr;
    ld->Font_map_start = (char *) dds->vram_start + COLOR_FONT_OFFSET;
 }
 else
 {
    dp = dds->vram_start + MONO_FONT_OFFSET + (ulong)base_addr;
    ld->Font_map_start = (char *) dds->vram_start + MONO_FONT_OFFSET;
 }

 cnt = (BYTESOFGLYPHINFO(ld->fonthptr)>>2);
 for ( i = 0; i < cnt; dp++,sp++,i++)
 {
    *dp = *sp;
    for (delay = 0; delay < 10; delay++);
 }
 ld->Vttenv.character_box.height = ld->fonthptr->minbounds.ascent + 
                                   ld->fonthptr->minbounds.descent;

 ld->Vttenv.character_box.width = ld->fonthptr->minbounds.characterWidth;

 i = 0;
 if (ld->fonthptr->minbounds.characterWidth & 0x7)
    i = 1;

 ld->Vttenv.font_box_width =
	 ((ld->fonthptr->minbounds.characterWidth>>3) + i) << 3;

 ld->Vttenv.font_box_width--;


 clrjmpx(&(jmpbuf));

 ld->iop = (ulong)ld->iop & (~ (ulong)base_addr );   /* base + offset - base */

 BUSMEM_DET(base_addr);

}			/* end of load font */



/***********************************************************************/
/*								       */
/* IDENTIFICATION: LOAD_PALETTE 				       */
/*								       */
/* DESCRIPTIVE NAME: Load Palette into Adapter			       */
/*								       */
/* FUNCTION:  Takes the default color table ct_k and downloads the     */
/*	      RGB information contained in the table into the adapter  */
/*	      palette area. This code will handle either 16 or 256     */
/*	      colors depending on whether 8Bpp mode is ever	       */
/*	      implemented in the KSR code. The number of colors sent   */
/*	      to the adapter will be based on a define KSR_8BPP.       */
/*	      Since SKYWAY accepts colors in two methods repeating     */
/*	      sequences of packed 24 bit RGB values or a fullword      */
/*	      with RBGX (X being junk), this routine will have to      */
/*	      restructure the default color table to a downloadable    */
/*	      format. A static table sky_palette will be built with    */
/*	      the format RBGX and saved for downloading. The variable  */
/*	      palette_converted will be set to 1 so future conversion  */
/*	      isn't necessary unless the palette has been changed by   */
/*	      the application. (See vttstct).			       */
/*								       */
/* INPUTS:    Virtual terminal pointer				       */
/*								       */
/* OUTPUTS:   Table containing skyway oriented rgb values. Adapter     */
/*	      with palette loaded.				       */
/*								       */
/* CALLED BY: vttact when entering KSR mode			       */
/*								       */
/* CALLS:     None.						       */
/*								       */
/***********************************************************************/

static void load_palette(vp)
struct vtmstruc *vp;

{
  int	i;

  union {
    ulong full;

    struct {
      uchar  char1,
	     char2,
	     char3,
	     char4;
      } byte;
  } workword;

  struct skyway_data *ld;
  int parityrc;
  struct ddsent *dds;
  label_t jmpbuf;
  caddr_t base_addr;

  dds = (struct ddsent *) vp->display->odmdds;

  ld = (struct skyway_data *) vp->vttld;

  /* Set parity handler */

  parityrc = setjmpx(&(jmpbuf));
  if (parityrc != 0)
  {
     if (parityrc == EXCEPT_IO)
     {
	BUSMEM_DET(base_addr);
        ld->io_idata = (ulong) ld->io_idata & (~ (ulong) base_addr);

	/* Log an error and return */
	sky_err(vp,"SKYDD","vtt2drvr ","setjmpx", parityrc, 
                SKY_SETJMP, SKY_UNIQUE_5);
	return;
     }
     else
	longjmpx(parityrc);
  }


  base_addr = BUSMEM_ATT(BUS_ID,0);
  ld->io_idata = (ulong) ld->io_idata | (ulong) base_addr;

  /* Set palette index to 0 */
  *ld->io_idata = PALETTE_INDEX_LO;

  /* Tell the adapter we will use the rbgx method of loading colors */
  *ld->io_idata = PALETTE_SEQUENCE ;

  for (i=0; i < ld->ct_k.nent; i++)
  {
     workword.full = ld->ct_k.rgbval[i];

     /* Switch the blue and green rgb values */
     *ld->io_idata = PALETTE_DATA | workword.byte.char2;
     *ld->io_idata = PALETTE_DATA | workword.byte.char3;
     *ld->io_idata = PALETTE_DATA | workword.byte.char4;

  }    /* end for loop */

  ld->io_idata = (ulong) ld->io_idata & (~ (ulong) base_addr);
  BUSMEM_DET(base_addr);
  clrjmpx(&(jmpbuf));


} /* end of load_palette */


/***********************************************************************/
/*								       */
/* IDENTIFICATION: RESET_COLOR					       */
/*								       */
/* DESCRIPTIVE NAME: Reset color entry adapter			       */
/*								       */
/* FUNCTION:  This routine issues the pixel interface commands needed  */
/*	      to reset the adapter into a base condition. The	       */
/*	      following operations are performed:		       */
/*								       */
/* INPUTS:    Local data area pointer				       */
/*								       */
/* OUTPUTS:   Reset Skyway Adapter.				       */
/*								       */
/* CALLED BY: skyway_reset					       */
/*								       */
/* CALLS:     None						       */
/*								       */
/***********************************************************************/

static void reset_color(ld)

struct skyway_data *ld;
{

  /* Reset CRTC Registers on the color adapter */

  /* Set up cursor magic numbers */
  if (ld->poll_type)
  {
     ld->cursor_x = 0x1AC;
     ld->cursor_y = 0x1A;
     *ld->io_idata = MEM_CONFIG | 0x0E;
     *ld->io_idata = 0x0100 | 0x02;

     /* Horizontal Sync Position Register */
     *ld->io_idata = HORIZ_SYNC_POS_REG | 0x0006;
     /* Horizontal Sync Pulse Start Register */
     *ld->io_idata = HORIZ_SYNC_PUL_START | 0x00AC;

     /* Horizontal Sync Pulse End Register 1 */
     *ld->io_idata = HORIZ_SYNC_PUL_END1 | 0x0093;

     /* Horizontal Sync Pulse End Register 2 */
     *ld->io_idata = HORIZ_SYNC_PUL_END2 | 0x00C5;

     /* Horizontal Blanking End Register = end of active picture row */
     *ld->io_idata = HORIZ_BLANKING_END | 0x00DB;

     /* Vertical Sync Pulse Start Registers Hi */
     *ld->io_idata = VERT_SYNC_PUL_START_LO | 0x0001;

     /* Vertical Sync Pulse End Register */
     *ld->io_idata = VERT_SYNC_PUL_END | 0x0004;
  }
  else
  {
     ld->cursor_x = 0x198;
     ld->cursor_y = 0x1A;
     *ld->io_idata = MEM_CONFIG | S64_M256_PW64;

     /* Horizontal Sync Pulse Start Register */
     *ld->io_idata = HORIZ_SYNC_PUL_START | PEL1304;

     /* Horizontal Sync Pulse End Register 1 */
     *ld->io_idata = HORIZ_SYNC_PUL_END1 | PEL1104;

     /* Horizontal Sync Pulse End Register 2 */
     *ld->io_idata = HORIZ_SYNC_PUL_END2 | PEL1504;

     /* Horizontal Blanking End Register = end of active picture row */
     *ld->io_idata = HORIZ_BLANKING_END | PEL1280;

     /* Vertical Sync Pulse Start Registers Lo */
     *ld->io_idata = VERT_SYNC_PUL_START_LO | TWO ;

     /* Vertical Sync Pulse End Register */
     *ld->io_idata = VERT_SYNC_PUL_END | FIVE;
  }

  /* Set memory config register to establish serializer widths	*/
  /* and module type						*/


   /* Initialize RAMDAC for brooktree part */
   *ld->io_idata = 0x6006;	 /*	 60  INDX REG = 06   */
   *ld->io_idata = 0x6440;	 /*	 64  CMD REG  = 40   */
   *ld->io_idata = 0x6004;	 /*	 60  INDX REG = 04   */
   *ld->io_idata = 0x640F;	 /*	 64  READ MSK = 0F set for 4bpp */
   *ld->io_idata = 0x6005;	 /*	 60  INDX REG = 05   */
   *ld->io_idata = 0x6400;	 /*	 64  BLNK MSK = 00   */
   *ld->io_idata = 0x6007;	 /*	 60  INDX REG = 07   */
   *ld->io_idata = 0x6400;	 /*	 64  TEST REG = 00   */



  *ld->io_idata = CLOCK_FREQ_SELECT | TWO;
  /* turn off sprite */
  *ld->io_idata = SPRITE_CONTROL;

  /* set palette masking off */
  *ld->io_idata = PALETTE_MASK;

  /* Set display mode register to composite sync and drive reset */
  *ld->io_idata = DISPLAY_MODE_1 | MODE_1_DATA;
  /* Set display mode register to 4Bpp */
  *ld->io_idata = DISPLAY_MODE_2 | FOURBPP;

  /* Set up CTRC values for a 1280x1024 screen */

  /* Horizontal Total Register */

  /* Set to 1760 pel times */
  *ld->io_idata = HORIZ_TOTAL_REG | HORIZ_TOTAL_1760;

  /* Horizontal Display End Register = end of active picture row */

  /* Set to 1280 pel times */
  *ld->io_idata = HORIZ_DISPLAY_END | PEL1280;

  /* Horizontal Blanking Start Register = end of active picture row */

  /* Set to 1280 pel times */
  *ld->io_idata = HORIZ_BLANKING_START | PEL1280;


  /* Vertical Total Registers Hi/Lo */

  *ld->io_idata = VERT_TOTAL_LO | ONE_F;
  *ld->io_idata = VERT_TOTAL_HI | FOUR;

  /* Vertical Display End Registers = end of active picture area */

  *ld->io_idata = VERT_DISPLAY_END_LO | FF;
  *ld->io_idata = VERT_DISPLAY_END_HI | THREE ;

  /* Vertical Blanking Start Registers Hi/Lo */

  *ld->io_idata = VERT_BLANK_START_LO | FF;
  *ld->io_idata = VERT_BLANK_START_HI | THREE ;

  /* Vertical Blanking End Registers Hi/Lo */

  *ld->io_idata = VERT_BLANK_END_LO | ONE_F;
  *ld->io_idata = VERT_BLANK_END_HI | FOUR ;

  /* Vertical Sync Pulse Start Registers Hi */
  *ld->io_idata = VERT_SYNC_PUL_START_HI | FOUR;


  /* Vertical Line Compare Registers Hi/Lo - sets end of scrollable */
  /* picture area						    */
     /* Set to 1024th line */

     /* Set lo byte */
  *ld->io_idata = VERT_LINE_COMPARE_LO;
     /* Set hi byte */
  *ld->io_idata = VERT_LINE_COMPARE_HI | FORTY;

  /* Set display mode register to composite sync and drive reset */
  *ld->io_idata = DISPLAY_MODE_1 | MODE_1_DATA;

}


/***********************************************************************/
/*								       */
/* IDENTIFICATION: RESET_MONO					       */
/*								       */
/* DESCRIPTIVE NAME: Reset mono entry adapter			       */
/*								       */
/* FUNCTION:  This routine issues the pixel interface commands needed  */
/*	      to reset the adapter into a base condition. The	       */
/*	      following operations are performed:		       */
/*								       */
/* INPUTS:    Local data area pointer				       */
/*								       */
/* OUTPUTS:   Reset Skyway Adapter.				       */
/*								       */
/* CALLED BY: skyway_reset					       */
/*								       */
/* CALLS:     None						       */
/*								       */
/***********************************************************************/
static void reset_mono(ld)

struct skyway_data *ld;
{

  /* Set up cursor magic numbers */
  if (ld->poll_type)
  {
     ld->cursor_x = 0x194;
     ld->cursor_y = 0x1F;
     *ld->io_idata = MEM_CONFIG | 0x000D;
     *ld->io_idata = 0x0100 | 0x007E;

     /* Horizontal Sync Position Register */
     *ld->io_idata = HORIZ_SYNC_POS_REG | 0x0006;

     /* Horizontal Sync Pulse Start Register */
     *ld->io_idata = HORIZ_SYNC_PUL_START | 0x00B4;

     /* Horizontal Sync Pulse End Register 1 */
     *ld->io_idata = HORIZ_SYNC_PUL_END1 | 0x00D4;

     /* Horizontal Sync Pulse End Register 2 */
     *ld->io_idata = HORIZ_SYNC_PUL_END2 | PEL1504;

     /* Horizontal Blanking End Register = end of active picture row */
     *ld->io_idata = HORIZ_BLANKING_END | 0x00E1;

     /* Vertical Sync Pulse Start Registers Hi */
     *ld->io_idata = VERT_SYNC_PUL_START_LO | 0x00FF;
     *ld->io_idata = VERT_SYNC_PUL_START_HI | THREE;

     /* Vertical Sync Pulse End Register */
     *ld->io_idata = VERT_SYNC_PUL_END | 0x0007;
  }
  else
  {
     ld->cursor_x = 0x1A0;
     ld->cursor_y = 0x19;
     *ld->io_idata = MEM_CONFIG | MONO_CONFIG;

     /* Horizontal Sync Pulse Start Register */
     *ld->io_idata = HORIZ_SYNC_PUL_START | PEL1448;

     /* Horizontal Sync Pulse End Register 1 */
     *ld->io_idata = HORIZ_SYNC_PUL_END1 | PEL1704;

     /* Horizontal Sync Pulse End Register 2 */
     *ld->io_idata = HORIZ_SYNC_PUL_END2 | PEL1504;

     /* Horizontal Blanking End Register = end of active picture row */
     *ld->io_idata = HORIZ_BLANKING_END | PEL1280;

     /* Vertical Sync Pulse Start Registers Lo */
     *ld->io_idata = VERT_SYNC_PUL_START_LO | TWO ;
     *ld->io_idata = VERT_SYNC_PUL_START_HI | FOUR;

     /* Vertical Sync Pulse End Register */
     *ld->io_idata = VERT_SYNC_PUL_END | 0x0008;
  }


  /* Reset CRTC Registers on the mono adapter */

   /* Initialize RAMDAC for brooktree part */
   *ld->io_idata = 0x6006;	 /*	 60  INDX REG = 06   */
   *ld->io_idata = 0x6440;	 /*	 64  CMD REG  = 40   */
   *ld->io_idata = 0x6004;	 /*	 60  INDX REG = 04   */
   *ld->io_idata = 0x640F;	 /*	 64  READ MSK = 0F set for 4bpp */
   *ld->io_idata = 0x6005;	 /*	 60  INDX REG = 05   */
   *ld->io_idata = 0x6400;	 /*	 64  BLNK MSK = 00   */
   *ld->io_idata = 0x6007;	 /*	 60  INDX REG = 07   */
   *ld->io_idata = 0x6400;	 /*	 64  TEST REG = 00   */



  *ld->io_idata = CLOCK_FREQ_SELECT | TWO;
  /* turn off sprite */
  *ld->io_idata = SPRITE_CONTROL;

  /* set palette masking off */
  *ld->io_idata = PALETTE_MASK;

  /* Set display mode register to composite sync and drive reset */
  *ld->io_idata = DISPLAY_MODE_1 | MODE_1_DATA_M;
  /* Set display mode register to 4Bpp */
  *ld->io_idata = DISPLAY_MODE_2 | FOURBPP;

  /* Set up CTRC values for a 1280x1024 screen */

  /* Horizontal Total Register */

  /* Set to 1760 pel times */
  *ld->io_idata = HORIZ_TOTAL_REG | HORIZ_TOTAL_1808;

  /* Horizontal Display End Register = end of active picture row */

  /* Set to 1280 pel times */
  *ld->io_idata = HORIZ_DISPLAY_END | PEL1280;

  /* Horizontal Blanking Start Register = end of active picture row */

  /* Set to 1280 pel times */
  *ld->io_idata = HORIZ_BLANKING_START | PEL1280;


  /* Vertical Total Registers Hi/Lo */
  *ld->io_idata = VERT_TOTAL_LO | ONE_F;
  *ld->io_idata = VERT_TOTAL_HI | FOUR;

  /* Vertical Display End Registers = end of active picture area */
  *ld->io_idata = VERT_DISPLAY_END_LO | FF;
  *ld->io_idata = VERT_DISPLAY_END_HI | THREE ;

  /* Vertical Blanking Start Registers Hi/Lo */
  *ld->io_idata = VERT_BLANK_START_LO | FF;
  *ld->io_idata = VERT_BLANK_START_HI | THREE ;

  /* Vertical Blanking End Registers Hi/Lo */
     /* Set to 1056 lines */

     /* Set lo byte */
  *ld->io_idata = VERT_BLANK_END_LO | ONE_F;
     /* Set hi byte */
  *ld->io_idata = VERT_BLANK_END_HI | FOUR ;


  /* Vertical Sync Pulse End Register */


  /* Vertical Line Compare Registers Hi/Lo - sets end of scrollable */
  /* picture area						    */
     /* Set to 1024th line */

     /* Set lo byte */
  *ld->io_idata = VERT_LINE_COMPARE_LO;
     /* Set hi byte */
  *ld->io_idata = VERT_LINE_COMPARE_HI | FORTY;

}


/*************************************************************************/
/*									 */
/* DESCRIPTION OF MODULE:						 */
/*									 */
/*    IDENTIFICATION :	 set_character_attributes			 */
/*									 */
/*    FUNCTION:  This function invokes several macros to set the	 */
/*		 attributes for a character that is being requested to	 */
/*		 be drawn.						 */
/*									 */
/*    INPUTS:	 Virtual terminal pointer				 */
/*		 New attribute value					 */
/*									 */
/*    OUTPUTS:	 None.							 */
/*									 */
/*    CALLED BY: draw_text						 */
/*									 */
/*    CALLS:	 None							 */
/*									 */
/*************************************************************************/

static long set_character_attributes(vp, new)

struct vtmstruc *vp;
long new;
{
 ulong tmp;
 struct skyway_data *ld;

 ld = (struct skyway_data *) vp->vttld;

 SELECT_NEW_FONT(vp,new,tmp);

 SELECT_NEW_BGCOL(ld,new);

 SELECT_NEW_FGCOL(ld,new);

 ld->Vttenv.current_attr = new ;

}			/* end of set_character_attributes */





/***********************************************************************/
/*								       */
/* IDENTIFICATION: SKY_PXBLT					       */
/*								       */
/* DESCRIPTIVE NAME: Pixel Block Transfer routine for Skyway adapter   */
/*								       */
/* FUNCTION:  This routine issues a pixel block transfer command to    */
/*	      the Skyway adapter. The inputs to the routine are:       */
/*								       */
/*		1. The destination x and y pointers		       */
/*		2. The dimensions of the block to be transferred       */
/*		3. Requested color values for fore and background      */
/*		4. The mix values parameters for colors 	       */
/*		5. The contents of the pixel operation register        */
/*		6. A pointer to the coprocessor registers.	       */
/*								       */
/*	      These parameters are stored into the appropriate	       */
/*	      register in the coprocessor. The storing of the	       */
/*	      pixel operation register is what causes the invocation   */
/*	      of the operation. No error chacking is done to see       */
/*	      if an error in the operation occurred.		       */
/*								       */
/*  INPUTS:   Destination map x,y coordinates - ushorts 	       */
/*	      Operation dimensions - ushorts			       */
/*	      Foreground and background colors - ulongs 	       */
/*	      Foreground and background mix values - uchars	       */
/*	      Pixel Operation register - ulong			       */
/*	      Pointer to the coprocessor registers - sky_cop_regs *    */
/*								       */
/*	      Pixel map format and sizes are assumed to be set up      */
/*								       */
/* OUTPUTS:   None.						       */
/*								       */
/* CALLED BY: skyway_reset					       */
/*	      vttscroll 					       */
/*								       */
/* CALLS:     None.						       */
/*								       */
/***********************************************************************/

static void
sky_pxblt(ld,srcx,srcy,destx,desty,dim1,dim2,fgcolor,bgcolor,fgmix,bgmix,
	  pixelop)

struct skyway_data *ld;
ushort	srcx,srcy;		/* X,Y from source */
ushort	destx,desty,dim1,dim2;	/* X,Y coordinates and size of Blt box */
ulong	fgcolor,bgcolor;	/* Selected color indexes	      */
char   fgmix,bgmix;		/* Selected color mix values	      */
ulong	pixelop;		/* Requested pixel operation register */

{
  /* It assumes the pixel map has already been defined in the	*/
  /* registers. 						*/

  union {
    uint  full;

    struct {
      ushort tophalf;
      ushort bothalf;
    } halfs;
  } tempword;

  union {
    ushort  half;

    struct {
      char topbyte;
      char botbyte;
    } bytes;
  } temphalf;


  /* Set up x,y parameters for PxBlt */
  /* Set source x,y at 0,0 */
  ld->cop->pat_yx = (long) 0;

  /* Set source x,y */
  tempword.halfs.tophalf = srcy;
  tempword.halfs.bothalf = srcx;
  ld->cop->src_yx = tempword.full;

  /* Set destination x,y */
  tempword.halfs.tophalf = desty;
  tempword.halfs.bothalf = destx;
  ld->cop->dst_yx = tempword.full;

  /* Set up operation dimension registers */
  tempword.halfs.tophalf = dim2;
  tempword.halfs.bothalf = dim1;
  ld->cop->opdim21 = tempword.full;

  if (!(ld->colset))
  {
     ld->cop->fgd_color = fgcolor ;
     ld->cop->bgd_color = bgcolor ;
     ld->colset = TRUE;
  }

#ifdef SETMIX
  /* Set mix registers */
  temphalf.bytes.topbyte = bgmix;
  temphalf.bytes.botbyte = fgmix;
  ld->cop->bgfgmix = temphalf.half;
#endif

  /* Issue operation command by passing pixel operation register */
  ld->cop->pixel_op_reg = pixelop;


} /* End of PxBlt Routine */





/***********************************************************************/
/*								       */
/* IDENTIFICATION: SKYWAY_RESET 				       */
/*								       */
/* DESCRIPTIVE NAME: Reset SKYWAY adapter			       */
/*								       */
/* FUNCTION:  This routine issues the pixel interface commands needed  */
/*	      to reset the adapter into a base condition. The	       */
/*	      following operations are performed:		       */
/*								       */
/*		1. Set up operating mode			       */
/*		2. Establish memory configuration		       */
/*		3. Set up CRT controller registers		       */
/*		4. Clear the screen				       */
/*		5. Turn Sprite off				       */
/*								       */
/* INPUTS:    None.						       */
/*								       */
/* OUTPUTS:   Reset Skyway Adapter.				       */
/*								       */
/* CALLED BY: INIT						       */
/*								       */
/* CALLS:     sky_pxblt();					       */
/*								       */
/***********************************************************************/


static void skyway_reset(ld,base_addr)
struct skyway_data *ld;
ulong base_addr;
{


  uint	 tempfull;	     /* 32 bit value for loading temp colors */
  long	 i;

  union rgbval {
   long  full;

   struct byte {
     char junk;
     char red;
     char green;
     char blue;
   } byte;
  } rgbdat;

  int parityrc;
  label_t jmpbuf;


  /* Set parity handler */
  parityrc = setjmpx(&(jmpbuf));
  if (parityrc != 0)
  {
     if (parityrc == EXCEPT_IO)
     {
	/* Log an error and return */
	sky_err(NULL,"SKYDD","vtt2drvr","setjmpx",parityrc, 
                SKY_SETJMP, SKY_UNIQUE_6);

  	ld->cop      =  (ulong)ld->cop & (~ base_addr );    /* base + offset - base = offset    */
  	ld->iop      =  (ulong)ld->iop & (~ base_addr );    /* expect ~ base_addr == 0x0fffffff */
  	ld->io_idata =  (ulong)ld->io_idata & (~ base_addr );

	return;
     }
     else
	longjmpx(parityrc);
  }

 ld->cop      =  (ulong)ld->cop | base_addr;    /* base + offset */
 ld->iop      =  (ulong)ld->iop | base_addr;    
 ld->io_idata =  (ulong)ld->io_idata | base_addr;

  /* Terminate operations on adapter */
  ld->cop->pi_control |= 0x20;

  if (ld->poll_type)
     while(ld->cop->pollreg > 0x7f);
  else
     while(ld->cop->pi_control > 0x7f);

  /* Set memory access mode to motorola mode and 4Bpp	*/
  ld->iop->mem_acc_mode = MOTO_ORDER_4BPP;

  /* turn off all interrupts */

  ld->iop->int_enable = 0x00;
  ld->iop->int_status = 0xff;
  ld->iop->vmem_int_enable = 0x00;
  ld->iop->vmem_int_stat   = 0xff;
  /* Set up operating mode register to value for native motorola mode and
     memory decoding enabled						  */

  ld->iop->op_mode = NATMOTOROLA;

  /* Set pc Vram window to 0 */
  ld->iop->pc_vram_window = 0x0;

  if (ld->disp_type == SKY_COLOR)
     reset_color(ld);
  else
     reset_mono(ld);

  /* Set Buffer pitch to 1280 (in units of 64 data bits) */
  *ld->io_idata = BUFFER_PITCH_LO | FIFTY ;
  *ld->io_idata = BUFFER_PITCH_HI;

  /* Turn on all bits in palette mask */
  *ld->io_idata = PALETTE_MASK | FF;


  /* Set palette index to 0 */
  *ld->io_idata = PALETTE_INDEX_LO | 0;
  *ld->io_idata = PALETTE_INDEX_HI | 0;

  *ld->io_idata = PALETTE_DATA | 0;
  *ld->io_idata = PALETTE_DATA | 0;
  *ld->io_idata = PALETTE_DATA | 0;

  /* Now that we have colors in the adapter we can clear the screen to
     a selected color by doing a PxBlt operation			   */
  /* Set up pixel map A as the screen */

  /* Set the colors */

  ld->colset = FALSE;
  ld->cop->fgd_color = 0;
  ld->cop->bgd_color = 0;
  ld->cop->plane_mask = 0x0000FFFF;
  ld->colset = FALSE;


  /* Set color compare as false */
  ld->cop->color_comp = COLOR_COMPARE_OFF; /* Set to enable update to mask */


  ld->cop->pix_index = PIX_MAP_A;	      /* Set up Pixel Map A	  */
  ld->cop->pixmap_base = (char *) ld->skymem; /* Set VRAM address */
  ld->cop->pix_hw = PEL1024HI | PEL1280LO; /*  Make pixel map 1280x1024 */
  ld->cop->pixmap_fmat = PIX_FMAT_4BPP;
  ld->cop->bgfgmix = 0x0303;

  /* Set up value for Pixel operation register to pass to the Blt routine */

  tempfull = POForeReg | POStepBlt | PODestA | POPatFore;



  /* This call to the PxBlt routine sets the pixel operation to be a	 */
  /* PxBlt using the foreground color register, Map A as the destination */
  /* map, the pattern is set to foreground fixed, the mask is disabled	 */
  /* and the octant value will cause filling from the upper left to	 */
  /* bottom right.							 */
  /*	    destx    xdim	     fgclr	 fgmix	    pixel op	 */
  /*	       desty	    ydim	     bgclr   bgmix  register	 */
  sky_pxblt( ld, 0, 0, 0 , 0 , 1279 , 1023 , 0, 0, 3 , 3 ,  tempfull);


  if (ld->poll_type)
     while(ld->cop->pollreg > 0x7f);
  else
     while(ld->cop->pi_control > 0x7f);
  /* Now that the screen memory is cleared ensure the adapter is pointing
     to the default map which we have just cleared out			 */

   /* Set start address to the first byte in the adapters VRAM. */
  *ld->io_idata = START_ADDRESS_LO; /* Set start address lo to 0 */
  *ld->io_idata = START_ADDRESS_MI; /* Set start address middle to 0 */
  *ld->io_idata = START_ADDRESS_HI; /* Set start address hi to 0 */


  ld->cop      =  (ulong)ld->cop & (~ base_addr );    /* base + offset - base = offset */
  ld->iop      =  (ulong)ld->iop & (~ base_addr );
  ld->io_idata =  (ulong)ld->io_idata & (~ base_addr );

  clrjmpx(&(jmpbuf));

}  /* end skyway_reset */


/*----------------------------------------------------------------*
*	    Start of entry points				  *
*----------------------------------------------------------------*/

/********************************************************************/
/*								    */
/* IDENTIFICATION: VTTACT					    */
/*								    */
/* DESCRIPTIVE name:  Activate virtual terminal,	            */
/*								    */
/* FUNCTION:							    */
/*	     Set the VDD state to active.			    */
/*								    */
/*	     If we are activating a Character (KSR) terminal:	    */
/*                                                                  */ 
/*		- Copy the contents of the presentation space into  */
/*		  the adapter frame buffer.			    */
/*		- Turn on cursor				    */
/*								    */
/*	     If user has requested something other than KSR         */
/*	     they are in error: 				    */
/*		- Log an error and return a Zero		    */
/*								    */
/*								    */
/* INPUTS:    Virtual terminal pointer				    */
/*								    */
/* OUTPUTS:							    */
/*								    */
/* CALLED BY: lftinit() and rcm (part of unmake_gp)		    */
/*	      vttsetm - dead code			    	    */
/*								    */
/* CALLS:							    */
/*	      vttdefc					            */
/*	      copy_ps						    */
/*								    */
/********************************************************************/


static long vttact(vp)
struct vtmstruc *vp;
{

    struct skyway_data *ld;
    struct sky_ddf *ddf;
    int rc=0;

    ld = (struct skyway_data *) vp->vttld;
    ddf = (struct sky_ddf *) vp->display->free_area;

    VDD_TRACE(ACT , TRACE_ENTRY, vp);


    /*--------------------------------*/
    /* set the VDD state to activated */
    /*--------------------------------*/

    ld->Vttenv.vtt_active = VTT_ACTIVE;

    switch (ld->Vttenv.vtt_mode)     /* on the current state of the VDD */
    {

       case KSR_MODE:



	       ddf->timerset = FALSE;

	/*************************************************************/
	/* copy the contents of the presentation space into the      */
	/* frame buffer establish the correct			     */
	/* position and shape of the hardware cursor.		     */
	/*							     */
	/* NOTE: A newly selected VDD has the following attributes:  */
	/* - the presentation space is initialized with all spaces   */
	/* - the shape of the hardware cursor is a double underscore */
	/* - the hw cursor is in the upper-left corner of the screen */
	/*************************************************************/

	      copy_ps(vp,TRUE);

	      /* Define the cursor */

	      vttdefc(vp, ld->Vttenv.cursor_select,1);   /* 1 == show cursor */

	      break;	      /* end of character mode */

	default:
        case GRAPHICS_MODE:

	      sky_err(vp,"SKYDD","vtt2drvr","invalid mode",SKY_INVALMODE, SKY_UNIQUE_7);
	      rc =-1;
    }

    VDD_TRACE(ACT , TRACE_EXIT, vp);
    return(rc);
}				 /* end  of  vttact		*/


/********************************************************************/
/*								    */
/* IDENTIFICATION: VTTCFL					    */
/*								    */
/* DESCRIPTIVE name:  Copy full lines of text.			    */
/*								    */
/* FUNCTION: Copies a sequence of one or more full lines	    */
/*	     up or down a specified number of lines (clipping	    */
/*	     at the presentation space boundaries). In addition,    */
/*	     the cursor can optionally be moved.		    */
/*								    */
/* INPUTS:    Virtual terminal pointer				    */
/*	      Source , Destination Row				    */
/*	      Number of rows to move				    */
/*	      Cursor show flag					    */
/*								    */
/* OUTPUTS:							    */
/*								    */
/* CALLED BY: Mode Processor					    */
/*								    */
/* CALLS:     copy_ps						    */
/*	      vttmovc						    */
/*								    */
/********************************************************************/


static long vttcfl(vp, s_row, d_row, num_rows,	cursor_show)
struct vtmstruc *vp;
long		   s_row,		   /* source row		   */
		   d_row,		   /* destination row		   */
		   num_rows;		   /* number of rows to copy	   */
ulong		   cursor_show; 	   /* if true cursor is moved to   */
					   /* the position given by *cp    */

{

ulong	       *buf_addr;	/* address of the buffer		*/
long	       s_offset,	/* starting offset of the source string */
	       d_offset,	/* starting offset of the target string */
	       save_off,	/* saving the original dest. offset	*/
	       num_words,	/* number of full words to be moved	*/
	       factor;		/* move direction indicator		*/
long           buf_offset,start,stop;

struct skyway_data *ld;

    ld = (struct skyway_data *) vp->vttld;

    VDD_TRACE(CFL , TRACE_ENTRY, vp);

    if (ld->Vttenv.vtt_mode != KSR_MODE){     /* not in character mode */
	sky_err(vp,"SKYDD","vtt2drvr",NULL,ld->Vttenv.vtt_mode, 
                SKY_NOTKSRMODE, SKY_UNIQUE_8);
	return(EIO);
    }

    if /************************************************************
       * the source address does not equal the destination address *
       ************************************************************/
	(s_row != d_row)
    {
       /************************************************
       * copy the indicated line segments as requested *
       ************************************************/

       /*********************************
       * update the presentation space *
       ********************************/

       buf_addr = ld->Vttenv.pse;

       /*********************************************************
	* calculate the starting offset of the first line to be *
	* moved in the source and and destination strings	*
	********************************************************/

       s_offset = ((s_row - 1) * ld->Vttenv.ps_size.wd);
       save_off = d_offset = ((d_row - 1) * ld->Vttenv.ps_size.wd);
       start = d_row - 1;
       stop = start + num_rows - 1;


       /*******************************************************************
	* calculate the number of full words to be copied		  *
	* The logic here is bound to need to be tuned for the SKYWAY	  *
	******************************************************************/

       num_words = (ld->Vttenv.ps_size.ht + 1) - d_row;
       if (num_words > num_rows)
	   num_words = num_rows;
       num_words = num_words * ld->Vttenv.ps_size.wd ;

       if /**************************************
	  * the destination is above the source *
	  **************************************/
	   (s_row > d_row)
       {
	  /************
	   * copy left *
	   ************/
	   factor = 1;
       }

       else
       {
	  /***************************************************************
	   * copy right to left avoiding source and dest overlap problems *
	   ***************************************************************/
	   factor = -1;
	   s_offset = s_offset + (num_words - 1);
	   d_offset = d_offset + (num_words - 1);
       }

       for /**********************************
	   * all lines that are to be copied *
	   **********************************/
	   (; num_words > 0; num_words--)
       {
	   /*****************************
	    * copy the next line segment *
	    *****************************/

	   *(buf_addr + ((d_offset + ld->Scroll_offset) % 
           ld->Vttenv.ps_words)) = *(buf_addr + ((s_offset + 
                                   ld->Scroll_offset) % ld->Vttenv.ps_words));
	   s_offset = s_offset + factor;
	   d_offset = d_offset + factor;
       }
    }

    ld->Vttenv.cursor_show = cursor_show;

    if (ld->Vttenv.vtt_active)	   /* If the virtual terminal is active */
    {
       buf_offset = (start * ld->Vttenv.ps_size.wd);
       for (;start <= stop; start++)
       {
	   draw_text(vp,(long)(ld->Vttenv.pse + ((buf_offset +
		     ld->Scroll_offset) % ld->Vttenv.ps_words)),
		     (ushort)ld->Vttenv.ps_size.wd,0,
		     (ushort)start,FALSE, TRUE);
	   buf_offset += ld->Vttenv.ps_size.wd;
	}
    }


    if (ld->Vttenv.vtt_active)	   /* If the cursor must be moved */
       vttmovc(vp);   /* Call movc to move it	     */

    VDD_TRACE(CFL , TRACE_EXIT, vp);
    return(0);
}				 /* end  of  vttcfl		*/


/********************************************************************/
/*								    */
/* IDENTIFICATION: VTTCLR					    */
/*								    */
/* DESCRIPTIVE name:  Clear rectangle				    */
/*								    */
/* FUNCTION: Clear a specified upright rectangular area of the	    */
/*	     frame buffer (if the Virtual Terminal is active)	    */
/*	     or presentation space (if the VT is not active).	    */
/*	     A space code is stored in each character position	    */
/*	     along with a specified attribute (e.g., normal,	    */
/*	     reverse video, underline, blink, etc.).		    */
/*								    */
/* INPUTS:    Virtual terminal pointer				    */
/*	      Structure indicating size of box to clear 	    */
/*	      Attribute to be stored into presentation space	    */
/*	      Cursor show flag					    */
/*								    */
/* OUTPUTS:   None.						    */
/*								    */
/* CALLED BY: Mode Processor					    */
/*								    */
/* CALLS:     sky_pxblt 					    */
/*	      vttmovc						    */
/*								    */
/********************************************************************/


static long vttclr(vp, sp, attr, cursor_show)
struct vtmstruc *vp;
struct vtt_box_rc_parms *sp;		/* upper-left and lower-right	*/
					/* corners of the rectangle	*/
ulong		  attr; 		/* character attribute		*/
ulong		  cursor_show;		/* if true cursor is moved to	*/
					/* pos specified in cp_parms	*/

{

ulong	     pcm_ch_attr,	 /* Color char and attr codes		 */
	     *buf_addr; 	 /* output buffer offset		 */
long	     buf_offset,	 /* offset into the output buffer	 */
	     buf_begin, 	 /* starting offset into the output buf  */
	     buf_end,		 /* ending offset into the output buf	 */
	     height,		 /* height of the rectangle		 */
	     i,tempfull;

long	     sx,sy,xdim,ydim;

ushort my_attrib, tmp_attr;	   /* attribute work area */
int parityrc;
struct skyway_data *ld;
label_t jmpbuf;
caddr_t base_addr;

    ld = (struct skyway_data *) vp->vttld;

    VDD_TRACE(CLR , TRACE_ENTRY, vp);

    if /*-----------------------*/
       /* not in character mode */
       /*-----------------------*/
	(ld->Vttenv.vtt_mode != KSR_MODE)
    {
       /*------------------------------------*/
       /* Only valid in character (KSR) mode */
       /*------------------------------------*/
       return(EIO);
    }

    /********************************************************************
     * calculate the attribute that should be stored with each character *
     * of the text string						 *
     ********************************************************************/
    SET_ATTRIBUTES(attr,tmp_attr);


    pcm_ch_attr = SPACE_A | tmp_attr ;

    /******************************************************
     * clear a rectangular area in the presentation space *
     *****************************************************/

    buf_addr = ld->Vttenv.pse;

    /***********************************************************
     * calculate beginning and ending offsets within each line *
     **********************************************************/
    buf_begin =  (sp->row_ul	- 1) * ld->Vttenv.ps_size.wd +
		 (sp->column_ul - 1) ;
    buf_end   = buf_begin + sp->column_lr - sp->column_ul;

    /*****************************************
     * calculate the height of the rectangle *
     ****************************************/
    height = (sp -> row_lr) - (sp -> row_ul) + 1;

    for /****************************
	* each row in the rectangle *
	****************************/

	(i=0; i<height; i++)
    {
	/****************
	* clear the row *
	****************/

       for /****************************
	   * each character in the row *
	   ****************************/

	   (buf_offset = buf_begin; buf_offset <= buf_end; buf_offset++)
       {
	   /*************************************************
	    * display the character as a space
	      with the specified attribute *
	    *************************************************/

	  *(buf_addr + ((buf_offset + ld->Scroll_offset) % 
          ld->Vttenv.ps_words)) = pcm_ch_attr;
       }

       /********************************
	* set up to clear the next row *
	*******************************/
       buf_begin = buf_begin + ld->Vttenv.ps_size.wd;
       buf_end	 = buf_end   + ld->Vttenv.ps_size.wd;
    }

    if /*--------------------------------*/
       /* the virtual terminal is active */
       /*--------------------------------*/
	(ld->Vttenv.vtt_active)
    {

       set_character_attributes(vp,pcm_ch_attr);

       /* Clear the rectangular area specified */
       ydim = (height * ld->Vttenv.character_box.height) - 1;
       xdim = ((sp->column_lr - sp->column_ul + 1) *
	      ld->Vttenv.character_box.width) - 1;
       sy = ((sp->row_ul-1) * ld->Vttenv.character_box.height) +
	     ld->center_y;
       sx = ((sp->column_ul-1) * ld->Vttenv.character_box.width) +
	     ld->center_x;

       /* Set parity handler */
       parityrc = setjmpx(&(jmpbuf));
       if (parityrc != 0)
       {
	  if (parityrc == EXCEPT_IO)
	  {
	     BUSMEM_DET(base_addr);
       	     ld->cop = (ulong) ld->cop & (~ (ulong) base_addr) ;  /* base + offset - base */

	     /* Log an error and return */
	     sky_err(vp,"SKYDD","vtt2drvr","setjmpx",parityrc, 
                     SKY_SETJMP, SKY_UNIQUE_9);
	     return(EIO);
	  }
	  else
	     longjmpx(parityrc);
       }

       base_addr = BUSMEM_ATT(BUS_ID,0);
       ld->cop = (ulong) ld->cop | (ulong) base_addr;  /* base + offset */

       /* wait for other operations to complete */
       if (ld->poll_type)
	  while(ld->cop->pollreg > 0x7f);
       else
	  while(ld->cop->pi_control > 0x7f);

       tempfull = POForeReg | POStepBlt | PODestA | POPatFore;


       ld->colset = FALSE;
       sky_pxblt( ld,ld->center_x, ld->center_y, sx, sy, xdim, ydim, 
                  ld->bgcol,  0,  3 , 3 , tempfull);
       ld->colset = FALSE;

       clrjmpx(&(jmpbuf));

       ld->cop = (ulong) ld->cop & (~ (ulong) base_addr) ;  /* base + offset - base */

       BUSMEM_DET(base_addr);
    }

    if /*--------------------------*/
       /* the cursor must be moved */
       /*--------------------------*/
	(ld->Vttenv.cursor_show = cursor_show /* assignment */)
    {
       /*-----------------*/
       /* move the cursor */
       /*-----------------*/
       vttmovc(vp);
    }
    VDD_TRACE(CLR , TRACE_EXIT, vp);
    return(0);
}				 /* end of vttclr		*/


/********************************************************************/
/*								    */
/* IDENTIFICATION: VTTCPL					    */
/*								    */
/* DESCRIPTIVE name:  Copy Line Segment 			    */
/*								    */
/* FUNCTION:  Provides a horizontal scroll function by copying a    */
/*	      portion of the specified line to another position     */
/*	      on that same line. This operation is repeated for     */
/*	      the number of consecutive lines requested.	    */
/*								    */
/* INPUTS:    Virtual terminal pointer				    */
/*	      Stucture indicating row/column information	    */
/*	      Cursor show flag					    */
/*								    */
/* OUTPUTS:   None.						    */
/*								    */
/* CALLED BY: Mode Processor					    */
/*								    */
/* CALLS:     copy_ps						    */
/*	      vttmovc						    */
/*								    */
/********************************************************************/

static long vttcpl(vp, rc, cursor_show)
struct vtmstruc *vp;
struct vtt_rc_parms *rc;		/* string position and length	*/
ulong		    cursor_show;	/* if true cursor is moved to	*/
					/* the pos given in cp_parms	*/
{

ulong		*buf_addr;	 /* address of the buffer		 */
long		s_offset,	 /* starting offset of the source string */
		d_offset,	 /* starting offset of the target string */
		length, 	 /* number of words to be moved 	 */
		slength,	 /* number of words to be moved 	 */
		no_lines_m1,	 /* number of lines to copy minus 1	 */
		factor; 	 /* move direction indicator		 */
long            buf_offset,start,stop;

struct skyway_data *ld;

    ld = (struct skyway_data *) vp->vttld;

    VDD_TRACE(CPL , TRACE_ENTRY, vp);

    if (ld->Vttenv.vtt_mode != KSR_MODE)  /* If not in character mode */
	return(EIO);

    if /************************************************************
       * the source address does not equal the destination address *
       ************************************************************/
	(rc -> dest_column != rc -> start_column)
    {
       /************************************************
       * copy the indicated line segments as requested *
       ************************************************/

       buf_addr = ld->Vttenv.pse;

       /**************************************************************
       * calculate the starting offset of the first character to be *
       * moved in the source and and destination strings	    *
       *************************************************************/
       s_offset = (rc->start_row - 1) * ld->Vttenv.ps_size.wd +
		  (rc->start_column - 1) ;
       d_offset = (rc->start_row - 1) * ld->Vttenv.ps_size.wd +
		  (rc->dest_column - 1) ;

       /****************************************************************/
       /* calculate the number of characters to be copied taking into  */
       /* account truncation when the destination is to the right of   */
       /* the source. NOTE: truncation is not possible when the        */
       /* destination is to the left of the source because the minimum */
       /* column number is 1					       */
       /****************************************************************/

       slength = ld->Vttenv.ps_size.wd - (rc -> dest_column) + 1;

       if (slength > (rc -> string_length))
	  slength = rc -> string_length;

       /**********************************************
	* calculate the number of lines to be copied *
	*********************************************/

       no_lines_m1 = (rc -> dest_row) - (rc -> start_row);
       start = rc->start_row - 1;
       stop = start + no_lines_m1;


       if /***********************************************
	  * the destination is to the left of the source *
	  ***********************************************/

	   ((rc -> dest_column) < (rc -> start_column))
       {
	  /************
	   * copy left *
	   ************/

	  factor = 1;

       }
       else
       {
	  /***************************************************************
	   * copy right to left avoiding source and dest overlap problems *
	   ***************************************************************/

	  factor = -1;
	  s_offset = s_offset + (slength - 1);
	  d_offset = d_offset + (slength - 1);
       }

       for /**********************************
	   * all lines that are to be copied *
	   **********************************/

	   (; no_lines_m1 >= 0; no_lines_m1--)
       {
	  /*****************************
	   * copy the next line segment *
	   *****************************/

	  for /**************************************
	      * all characters in the line segment  *
	      **************************************/

	      (length = slength; length > 0; length--)
	  {
	     /**********************************************************
	      * copy the character and its corresponding attribute code *
	      **********************************************************/

	     *(buf_addr + ((d_offset + ld->Scroll_offset)  %
						   ld->Vttenv.ps_words)) =
	     *(buf_addr + ((s_offset + ld->Scroll_offset)  %
						   ld->Vttenv.ps_words));
	     d_offset = (d_offset + factor) ;
	     s_offset = (s_offset + factor) ;
	  }

	  s_offset = s_offset + ld->Vttenv.ps_size.wd - (slength * factor);
	  d_offset = d_offset + ld->Vttenv.ps_size.wd - (slength * factor);

       }
    }

    if (ld->Vttenv.vtt_active)	 /* If the virtual terminal is active */
    {
       buf_offset = (start * ld->Vttenv.ps_size.wd);
       for (;start <= stop; start++)
       {
	   draw_text(vp,(long)(ld->Vttenv.pse + ((buf_offset +
		     ld->Scroll_offset) % ld->Vttenv.ps_words)),
		     (ushort)ld->Vttenv.ps_size.wd,0,(ushort)start,FALSE, TRUE);
	   buf_offset += ld->Vttenv.ps_size.wd;
       }
    }

    ld->Vttenv.cursor_show = cursor_show;

    if (cursor_show)	    /* If the cursor must be moved  */
       vttmovc(vp);	    /* call movc to move the cursor */

    VDD_TRACE(CPL , TRACE_EXIT, vp);
    return(0);
}				 /* end of vttcpl		 */


/**********************************************************************/
/*								      */
/*  IDENTIFICATION: VTTDACT					      */
/*								      */
/*  DESCRIPTIVE NAME:  Deactivate Command for the		      */
/*		       Virtual Display Driver (VDD)		      */
/*								      */
/*  FUNCTION:	Set the Vttenv mode to inactive.		      */
/*								      */
/*		If adapter has a FIFO interface wait for it to be     */
/*		empty.						      */
/*								      */
/*		If VT is currently in monitored mode:		      */
/*		  - Detach the paths from the Mode Processor and      */
/*		    SLIH					      */
/*								      */
/*  INPUTS:	None.						      */
/*								      */
/*  OUTPUTS:	DETACH_FAILED					      */
/*								      */
/*  CALLED BY:	Mode Processor					      */
/*								      */
/*  CALLS:	detach						      */
/*								      */
/*								      */
/**********************************************************************/

static long vttdact(vp)
struct vtmstruc *vp;

{

ushort i;
struct skyway_data *ld;
struct sky_ddf *ddf;

    ld = (struct skyway_data *) vp->vttld;
    ddf = (struct sky_ddf *) vp->display->free_area;

    VDD_TRACE(DACT , TRACE_ENTRY, vp);

    /*****************************************************
     * set the state of the virtual terminal to inactive *
     ****************************************************/

    ld->Vttenv.vtt_active = VTT_INACTIVE;
    ld->Vttenv.current_attr = 0xffff;
    ld->cur_font = -1;

    tstop(ddf->cdatime);
    ddf->jumpcount = 0;
    ddf->lastcount = 0;
    ddf->scrolltime = FALSE;
    ddf->jumpmode = FALSE;
    ddf->timerset = FALSE;

    VDD_TRACE(DACT , TRACE_EXIT, vp);
    return(0);
}				 /* end  of  vttdact		 */

/**********************************************************************/
/*								      */
/*   IDENTIFICATION: VTTDDF					      */
/*								      */
/*   DESCRIPTIVE NAME:	Device Dependent funtion support for SKYWAY   */
/*								      */
/*   FUNCTION:	Provide hardware dependent function to support	      */
/*		operations					      */
/*								      */
/*   INPUTS:	Virtual terminal pointer			      */
/*		Command 					      */
/*		Argument structure				      */
/*		Length						      */
/*								      */
/*   OUTPUTS:	None						      */
/*								      */
/*   CALLED BY: RCM						      */
/*								      */
/*   CALLS:	None						      */
/*								      */
/**********************************************************************/


static long vttddf(vp,cmd,arg,length)
struct vtmstruc *vp;	    /* Pointer to vtmstruct for request */
long cmd;		    /* Command requested */
struct dev_dep_fun *arg;    /* Arguments */
int length;		    /* Length of argument structure */

{
  struct skyway_data *ld;      /*pointer to local data area*/
  struct sky_ddf *ddf;
  int rc,oldlevel;
  ld = (struct skyway_data *) vp->vttld;

  VDD_TRACE(DDF , TRACE_ENTRY, vp);

  /* Use command to select work to do */
  switch ( cmd )

  {
     default:
	sky_err(vp,"SKYDD","vtt2drvr","badcmd","badcmd", 
                SKY_BADCMD, SKY_UNIQUE_10);
	rc = EIO;
	break;
  }  /* end of switch */

  VDD_TRACE(DDF , TRACE_EXIT, vp);
  return(rc);
}

/**********************************************************************/
/*								      */
/*   IDENTIFICATION: VTTDEFC					      */
/*								      */
/*   DESCRIPTIVE NAME:	Define Cursor to one of six shapes	      */
/*								      */
/*   FUNCTION:	If VT is not in character mode: 		      */
/*                - Return EIO                                        */
/*								      */
/*		Set selected cursor shape into Vttenv table.	      */
/*								      */
/*		If VT is active, change shape according to following  */
/*		selector values:				      */
/*								      */
/*		  0 - Invisible cursor				      */
/*		  1 - Single underscore 			      */
/*		  2 - Double underscore 			      */
/*		  3 - Half Blob 				      */
/*		  4 - Mid character double line 		      */
/*		  5 - Full blob 				      */
/*		  Any other value will get double underscore	      */
/*								      */
/*		If needed move the cursor by calling VTTMOVC	      */
/*								      */
/*   INPUTS:	Selector value					      */
/*		Vtt_cursor structure				      */
/*		Cursor_show (0 means don't show cursor) 	      */
/*		Top scan line					      */
/*		Bottom scan line				      */
/*								      */
/*   OUTPUTS:   EIO                                                   */
/*								      */
/*   CALLED BY:       		   			              */
/*		vttact						      */
/*		Mode Processor					      */
/*								      */
/*   CALLS:	Change_Cursor_shape				      */
/*		vttmovc 					      */
/*								      */
/**********************************************************************/


static long vttdefc(vp, selector, cursor_show)
struct vtmstruc *vp;
uchar		   selector;		/* shape selector		*/
ulong		   cursor_show; 	/* 0 ==> don't show the cursor	*/
{
 short	top,bottom;
 struct skyway_data *ld;

    ld = (struct skyway_data *) vp->vttld;

    VDD_TRACE(DEFC , TRACE_ENTRY, vp);

    if /*-----------------------*/
       /* not in character mode */
       /*-----------------------*/
	(ld->Vttenv.vtt_mode != KSR_MODE)
    {
       /*------------------------------------*/
       /* Only valid in character (KSR) mode */
       /*------------------------------------*/
       return(EIO);
    }

    /******************************************
    * Make sure global cursor shape is saved, *
    * it is used when new font is downloaded. *
    ******************************************/

    ld->Vttenv.cursor_select = selector;

    switch /**********************
	   * on the cursor shape *
	   **********************/
	(selector)
    {
       case /************
	    * invisible *
	    ************/
	 0:
	   /* code to set cursor to blank */
	    top = 255;
	    break;

       case /********************
	    * single underscore *
	    ********************/
	 1:
	    /*	set top and bottom to  char_box_ht-1 */
	    top = ld->Vttenv.character_box.height - 1;
	    bottom = ld->Vttenv.character_box.height - 1;
	    break;

       case /********************
	    * double underscore *
	    ********************/
	 2:
	    /*	set top to charbox_ht-2 and bottom to charbox_ht-1   */
	    top = ld->Vttenv.character_box.height - 2;
	    bottom = ld->Vttenv.character_box.height - 1;
	    break;

       case /************************************
	    * half blob lower half of character *
	    ************************************/
	 3:
	    /* insert code to to set top to charbox_ht/2 and bottom to	*/
	    /* charbox_ht-1						*/
	    top = ld->Vttenv.character_box.height / 2;
	    bottom = ld->Vttenv.character_box.height - 1;
	    break;

       case /****************************
	    * mid character double line *
	    ****************************/
	 4:
	    /* set top to (charbox_ht/2)-1 and bottom to		*/
	    /* charbox_hgt/2.						*/
	    top = (ld->Vttenv.character_box.height/2) ;
	    bottom = (ld->Vttenv.character_box.height/2) + 1;
	    break;

       case /************
	    * full blob *
	    ************/
	 5:
	    /* set top to 1 and bottom to charbox_hgt-1.  */
	    top = 1;
	    bottom = ld->Vttenv.character_box.height - 1;
	    break;

       default:
	   /***************************************************************
	   * Selectors 6-254 are reserved				  *
	   * any value other than 0-5 will get a double underscore!	  *
	   ***************************************************************/

	   top = ld->Vttenv.character_box.height-2;
	   bottom = ld->Vttenv.character_box.height-1;
       break;

    }		/* end of switch */

    if /*--------------------------------*/
       /* the virtual terminal is active */
       /*--------------------------------*/
	(ld->Vttenv.vtt_active)
    {
	/**********************************
	 * update adapter's cursor shape *
	 *********************************/
       change_cursor_shape (vp,top,bottom);
    }
    if /*--------------------------*/
       /* the cursor must be moved */
       /*--------------------------*/
	(ld->Vttenv.cursor_show = cursor_show /* assignment */)
       vttmovc(vp);

    VDD_TRACE(DEFC , TRACE_EXIT, vp);
    return(0);
}				 /* end  of vttdefc		 */

/**********************************************************************/
/*								      */
/*   IDENTIFICATION: VTTDMA					      */
/*								      */
/*   DESCRIPTIVE NAME:	Direct Memory access routine for skyway       */
/*								      */
/*   FUNCTION:	Provide hardware dependent function to support DMA    */
/*		operations					      */
/*								      */
/*   INPUTS:	RCM Device structure pointer			      */
/*		Pointer to device dependent arguments		      */
/*		Pointer to callback funtion			      */
/*								      */
/*								      */
/*   OUTPUTS:							      */
/*								      */
/*   CALLED BY: 						      */
/*								      */
/*   CALLS:							      */
/*								      */
/**********************************************************************/


static long vttdma(pdev,arg,callback,vp)
gscDev *pdev;
gscdma *arg;
int (*callback)();

struct vtmstruc *vp;
{
  struct skyway_data *ld;
  int oldlevel,rc,i;
  struct sky_ddf *ddf;
  int parityrc;
  struct ddsent *dds;
  label_t jmpbuf;
  caddr_t base_addr;

  dds = (struct ddsent *) pdev->devHead.display->odmdds;

  ld = (struct skyway_data *) vp->vttld;

   VDD_TRACE(DMA , TRACE_ENTRY, vp);


  ddf = (struct sky_ddf *) vp->display->free_area;

  /* This routine will check the parameters passed in and do the op  */
  /* accordingly. This code assumes all parameters passed in are     */
  /* correct and does not check the adapter for other operations     */



#ifdef NOWAIT
  if (arg->flags & DMA_WAIT)
#else
  if (1)
#endif
  {
     /* Here we have been asked to start the operation and wait until */
     /* it completes. To do this we will mask off system interrupts   */
     /* enable the operation complete interrupt on the adapter, issue */
     /* the operation and then go to sleep.			      */

     /* set up operation flags in the physical display */
     ddf->cmd = DMA_IN_PROGRESS | DMA_WAIT_REQ;
     parityrc = setjmpx(&(jmpbuf));
     if (parityrc != 0)
     {
	if (parityrc == EXCEPT_IO)
	{
	   BUSMEM_DET(base_addr);
           ld->iop = (ulong) ld->iop & (~(ulong) base_addr);
           ld->cop = (ulong) ld->cop & (~(ulong) base_addr);

	   /* Log an error and return */
	   sky_err(vp,"SKYDD","vtt2drvr","setjmpx", parityrc, 
                   SKY_SETJMP, SKY_UNIQUE_11);
	   return(EIO);
	}
	else
	   longjmpx(parityrc);
     }

     base_addr = BUSMEM_ATT(BUS_ID,0);
     ld->iop = (ulong) ld->iop | (ulong) base_addr;
     ld->cop = (ulong) ld->cop | (ulong) base_addr;

     oldlevel = i_disable(INTMAX);
     ld->iop->int_status = 0xff;
     ld->iop->int_enable |= OP_COMPLETE_MASK;

     /* Issue the operation */

     ld->cop->pixel_op_reg = *((ulong *)(arg->dma_cmd));
     clrjmpx(&(jmpbuf));

     ld->iop = (ulong) ld->iop & (~(ulong) base_addr);
     ld->cop = (ulong) ld->cop & (~(ulong) base_addr);

     BUSMEM_DET(base_addr);

     /* start timer */
     /* ddf->cdatime->timeout.it_value.tv_sec = 1;   */
     ddf->cdatime->timeout.it_value.tv_sec = 10;   /* change for 99062 */
     ddf->cdatime->timeout.it_value.tv_nsec = 0;
     ddf->timerset = TRUE;
     tstart(ddf->cdatime);

     rc = e_sleep(&(ddf->sleep_addr), EVENT_SHORT);
     i_enable(oldlevel);    /* turn interrupts back on */

     /* We have completed clean up and return */
     parityrc = setjmpx(&(jmpbuf));
     if (parityrc != 0)
     {
	if (parityrc == EXCEPT_IO)
	{
	   BUSMEM_DET(base_addr);
           ld->iop = (ulong) ld->iop & (~(ulong) base_addr);

	   /* Log an error and return */
	   sky_err(vp,"SKYDD","vtt2drvr","setjmpx", parityrc, 
                   SKY_SETJMP, SKY_UNIQUE_12);
	   return(EIO);
	}
	else
	   longjmpx(parityrc);
     }
     base_addr = BUSMEM_ATT(BUS_ID,0);
     ld->iop = (ulong) ld->iop | (ulong) base_addr;

     if ( (rc == EVENT_SUCC) && !(ddf->timerset))
     {
	ld->iop->int_status &= OP_COMPLETE_CLEAR;
	ddf->cmd = -1;
	ddf->sleep_addr = EVENT_NULL;

	/* Invoke callback routine */
	(*callback)(vp->display->pGSC->devHead.pProc);

	rc = 0;
     }
     else  /* Handle error conditions */
     {
	/* figure out error and set condition accordingly */
	ld->iop->int_enable &= OP_COMPLETE_CLEAR;
	tstop(ddf->cdatime);
	ddf->timerset = FALSE;
	rc = -1;
     }

     ld->iop = (ulong) ld->iop & (~(ulong) base_addr);
     BUSMEM_DET(base_addr);

     clrjmpx(&(jmpbuf));

  }  /* end of dma sync */
#ifdef NOWAIT
  else
  {  /* In this case we must have been called with NO_WAIT which */
     /* means we will have to issue the operation set the correct*/
     /* indicators in the ddf structure 			 */
     /* set up operation flags in the physical display */

     ddf->cmd = DMA_IN_PROGRESS;
     ddf->dcallback = callback;
     ddf->callbackarg = NULL;

     /* Issue the operation */

     parityrc = setjmpx(&(jmpbuf));
     if (parityrc != 0)
     {
	if (parityrc == EXCEPT_IO)
	{
           ld->iop = (ulong) ld->iop & ( ~ (ulong) base_addr );
           ld->cop = (ulong) ld->cop & ( ~ (ulong) base_addr );

	   BUSMEM_DET(base_addr);

	   /* Log an error and return */
	   sky_err(vp,"SKYDD","vtt2drvr","setjmpx", parityrc, 
                   SKY_SETJMP, SKY_UNIQUE_13);
	   return(EIO);
	}
	else
	   longjmpx(parityrc);
     }

     base_addr = BUSMEM_ATT(BUS_ID,0);
     ld->iop = (ulong) ld->iop | (ulong) base_addr ;
     ld->cop = (ulong) ld->cop | (ulong) base_addr ;

     oldlevel = i_disable(INTMAX);
     ld->iop->int_status = 0xff;
     ld->iop->int_enable |= OP_COMPLETE_MASK;

     /* Issue pixel op command */
     ld->cop->pixel_op_reg = *((ulong *)(arg->dma_cmd));
     clrjmpx(&(jmpbuf));
     i_enable(oldlevel);


     ld->iop = (ulong) ld->iop & ( ~ (ulong) base_addr );
     ld->cop = (ulong) ld->cop & ( ~ (ulong) base_addr );

     BUSMEM_DET(base_addr);
     rc = 0;

  }
#endif
 VDD_TRACE(DMA , TRACE_EXIT, vp);
 return(rc);
}

/*********************************************************************/
/*								     */
/*   IDENTIFICATION: VTTINIT					     */
/*								     */
/*   DESCRIPTIVE name:	Initialize SKYWAY  Virtual Display	     */
/*			Driver (VDD)				     */
/*								     */
/*   FUNCTION: Initialize the internal state of the device driver:   */
/*								     */
/*   These steps are done ONCE                              	     */
/*   ------------------------- 		                             */
/*								     */
/*   - Allocate space for local data structure                       */ 
/*   - Set up pointers to the various registers on theadapter        */
/*   - Set all entries in the Vttenv font structure to the default   */
/*     font. 			                                     */
/*   - Cursor shape and position			             */
/*   - Calculate presentation space centering offsets	             */
/*   - Allocate memory for presentation space                        */ 
/*   - Set character box values based on font size.	             */
/*   - Set cursor colors.				             */
/*   - load color talbe for KSR mode 			             */
/*								     */
/*   These steps are done every time vttinit is called        	     */
/*   ------------------------------------------------- 		     */
/*								     */
/*   - Initialize presentation space with blank and default attr.    */
/*   - Reset presentation virtual top                                */
/*   - Reset curren character attributes (font, fg, and bg)          */
/*   - reset the H/W                                                 */
/*   - download color map                                            */
/*								     */
/*  INPUTS:    Virtual terminal id				     */
/*	       Pointer to list of font ids			     */
/*	       Pointer to Presentation space size structure	     */
/*								     */
/*  OUTPUTS:  	                                                     */ 
/*								     */
/*  CALLED BY: lftinit() and rcm (part of unmake_gp)                 */ 
/*								     */
/*  CALLS:							     */
/*             skyway_reset()                                        */
/*             load_palette()                                        */
/*								     */
/*********************************************************************/

static long vttinit(vp, font_ids, ps_s)
struct vtmstruc *vp;
struct fontpal	*font_ids;	    /* real font ids		*/
struct t_ps_s *ps_s;		  /* presentation space size		   */

{

  long rc, i, font_w, font_h;
  struct skyway_data *ld;
  struct sky_ddf *ddf;
  struct ddsent *dds;
  int screen_width,screen_height;    /* variable for screen width in pixels */
  ulong base_addr;

  dds = (struct ddsent *) vp->display->odmdds;

  VDD_TRACE(INIT , TRACE_ENTRY, vp);

  if (vp->vttld == NULL)  
  {
     /* Allocate memory for local data area */

     ld = (struct skyway_data *)xmalloc(sizeof(struct skyway_data ),3,pinned_heap);
     if (ld == NULL)
     {
	sky_err(vp,"SKYDD","vtt2drvr","xmalloc",ld, SKY_XMALLOC, SKY_UNIQUE_14);
	return(ENOMEM);
     }

     vp->vttld = (char *) ld;
     bzero(ld,sizeof(struct skyway_data));

     ld->disp_type = vp->display->disp_devid[1];
     ddf = (struct sky_ddf *) vp->display->free_area;
     ld->poll_type = ddf->poll_type;
     vp->display->usage++;
     ld->comp_name = dds->component;

     ddf->jthreshold = 2;

     /* Establish pointers to the io and memory registers
        and the Vram on the adapter			 */

     ld->iop = dds->io_bus_addr_start ;    /* still need to OR in base addr when doing actual pio */
     ld->cop = dds->io_bus_mem_start ;
     ld->io_idata = dds->io_bus_addr_start + 10;
     ld->skymem = dds->vram_start;

     /**************************************************
      Load the font table with the primary font info
     ***************************************************/
     load_font_table(vp);

     font_w = ld->Vttenv.font_table[0].width;
     font_h = ld->Vttenv.font_table[0].height;

     /****************************************************
      * set the cursor to its default position and shape *
      * Upper left corner of screen and double underscore*
      ***************************************************/

     ld->Vttenv.cursor_select = 2;

     ld->Vttenv.cursor_pos.cursor_col = 1;
     ld->Vttenv.cursor_pos.cursor_row = 1;

     ld->Vttenv.cursor_shape.cursor_top = font_h-2;
     ld->Vttenv.cursor_shape.cursor_bot = font_h-1;

     /*******************************************************************
      * set the presentation space size to font_width/screen_width high *
      * and font_height/screen_height high 			     *
      *******************************************************************/

     ld->Vttenv.ps_size.wd = SCRN_WIDTH;
     ld->Vttenv.ps_size.ht = SCRN_HEIGHT;

     if ( (screen_width = SCRN_WIDTH * font_w) > X_RES ) 
        ld->center_x = 1;
     else
        ld->center_x = (X_RES - screen_width) >> 1;

     if ( (screen_height = SCRN_HEIGHT * font_h) > Y_RES ) 
        ld->center_y = font_h + 1;
     else
        ld->center_y = (Y_RES - screen_height) >> 1;


     /********************************************
      * allocate memory for the presention space *
      ********************************************/

     ld->Vttenv.ps_bytes = SCRN_WIDTH * SCRN_HEIGHT * sizeof(long) ;
     ld->Vttenv.ps_words = ld->Vttenv.ps_bytes / sizeof(long);

     ld->Vttenv.pse = (ulong *)xmalloc(ld->Vttenv.ps_bytes,3,pinned_heap);
     if (ld -> Vttenv.pse == NULL)
     {
        sky_err(vp,"SKYDD","vtt2drvr","xmalloc",ld->Vttenv.pse,SKY_XMALLOC, SKY_UNIQUE_15 );
        return(ENOMEM);
     }

     bzero(ld->Vttenv.pse,ld->Vttenv.ps_bytes);

     /******************************************************
      * set the character box values for cursor definition *
      *****************************************************/

     ld->fonthptr = (aixFontInfoPtr) ld->Vttenv.font_table[0].addr;
     ld->Vttenv.character_box.height = ld->fonthptr->minbounds.ascent +
                                    ld->fonthptr->minbounds.descent;
     ld->Vttenv.character_box.width = ld->fonthptr->minbounds.characterWidth;

     if (ld->Vttenv.character_box.width > 0x7)
        ld->Vttenv.font_box_width =
                (((ld->fonthptr->minbounds.characterWidth>>3)+1)<<3);
     else
        ld->Vttenv.font_box_width = 0x8;

     ld->Vttenv.font_box_width--;

     /************************
     * set the cursor colors *
     *************************/

     ld->Vttenv.cursor_color.fg = 15;
     ld->Vttenv.cursor_color.bg = 0;

     /****************************************/
     /* Initialize color table with defaults */
     /****************************************/

     for (i=0; i < 16; i++)
        ld->ct_k.rgbval[i] = dds->ksr_color_table[i];

     ld->ct_k.nent = 16;

     ld->Vttenv.vtt_mode = KSR_MODE;


  }

  ld = (struct skyway_data *) vp->vttld;
  ld->Scroll_offset = 0;

  ps_s -> ps_w = SCRN_WIDTH;
  ps_s -> ps_h = SCRN_HEIGHT;

  /**************************************************************
  * each word in the presentation spaces set character code to  * 
  * "spaces" and the attribute code to fg color 1               *
  ***************************************************************/
  for (i = 0; i < (ld->Vttenv.ps_words); i++)

  {
     ld->Vttenv.pse[i] = BLANK_CODE | ATTRIB_DEFAULT;
  }

  ld->Vttenv.current_attr = 0x2000;

  /* set the the current font to -1 to force the font to be loaded */
  ld->cur_font = -1;

  base_addr = BUSMEM_ATT(BUS_ID,0);

  skyway_reset(ld,base_addr);      /* Reset the adapter to a base state */

  BUSMEM_DET(base_addr);

  /************************************************************
   * Download the colors to the adapter.                       *
   ************************************************************/

   load_palette(vp);

   VDD_TRACE(INIT , TRACE_EXIT, vp);

   return(0);
}				 /* end  of  vttinit		 */



/*********************************************************************/
/*								     */
/*   IDENTIFICATION: VTTMOVC					     */
/*								     */
/*   DESCRIPTIVE name:	Move Cursor to requested location	     */
/*								     */
/*   FUNCTION:	Moves the cursor to the specified		     */
/*		row and column position on the screen. Row and	     */
/*		column numbers are unity based. 		     */
/*								     */
/*		If not in KSR mode:				     */
/*                - Return EIO                                       */
/*								     */
/*		Update the cursor position in environment structure  */
/*								     */
/*		If terminal is active:				     */
/*		  - Send commands to the adapter to move the cursor  */
/*								     */
/*   INPUTS:	Virtual terminal structure pointer containing cursor */
/*		position.					     */
/*								     */
/*   OUTPUTS:   EIO                                                  */
/*		INVALID_CURSOR_POSITION 			     */
/*								     */
/*   CALLED BY: Mode Processor					     */
/*		vttcfl						     */
/*		vttclr						     */
/*		vttcpl						     */
/*		vttdefc 					     */
/*		vttrep						     */
/*		vttscr						     */
/*								     */
/*   CALLS:	None.						     */
/*								     */
/*********************************************************************/

static long vttmovc(vp)
struct vtmstruc *vp;

{

union {
  ushort half;

  struct {
    char   hi;
    char   lo;
  } byte;
} addr;

 unsigned char c, *cptr;
 int i;
 short x,y;
 struct skyway_data *ld;
 int parityrc;
 struct sky_ddf *ddf;
 label_t jmpbuf;
 caddr_t base_addr;


    ld = (struct skyway_data *) vp->vttld;
    ddf = (struct sky_ddf *) vp->display->free_area;

    VDD_TRACE(MOVC , TRACE_ENTRY, vp);

    if /*-----------------------*/
       /* not in character mode */
       /*-----------------------*/
	(ld->Vttenv.vtt_mode != KSR_MODE)
    {
       /*------------------------------------*/
       /* Only valid in character (KSR) mode */
       /*------------------------------------*/
       return(EIO);
    }


    /******************************************************************
     * update the variable that tracks the virtual terminal's current *
     * cursor position						      *
     *****************************************************************/
    ld->Vttenv.cursor_pos.cursor_col = x = vp->mparms.cursor.x;
    ld->Vttenv.cursor_pos.cursor_row = y = vp->mparms.cursor.y;

    if /*--------------------------------*/
       /* the virtual terminal is active */
       /*--------------------------------*/
	(ld->Vttenv.vtt_active && !(ddf->jumpmode))
    {
       /*******************************************************
       * set the adapter's cursor position to requested value *
       ********************************************************/
       parityrc = setjmpx(&(jmpbuf));
       if (parityrc != 0)
       {
	  if (parityrc == EXCEPT_IO)
	  {
             ld->io_idata = (ulong)ld->io_idata & ( ~(ulong)base_addr) ; /* base + offset - base */
	     BUSMEM_DET(base_addr);
	     /* Log an error and return */
	     sky_err(vp,"SKYDD","vtt2drvr","setjmpx", parityrc, 
                     SKY_SETJMP, SKY_UNIQUE_16);
	     return(EIO);
	  }
	  else
	     longjmpx(parityrc);
       }

       x = (x-1) * ld->Vttenv.character_box.width + ld->center_x;
       y = (y-1) * ld->Vttenv.character_box.height + ld->center_y;

       base_addr = BUSMEM_ATT(BUS_ID,0);

       ld->io_idata = (ulong)ld->io_idata | (ulong)base_addr; /* base + offset */


       /* disable the cursor in the DAC 			       */
       *ld->io_idata = 0x5600;
       *ld->io_idata = 0x6c04;

       /* increment x and y by proper constants to achieve
	  desired results				       */
       x += ld->cursor_x;
       y += ld->cursor_y;

       cptr = (unsigned char *) &x+1;  /* get the high order bits      */
       *ld->io_idata = 0x5601;		   /* specify low  order X bits    */
       *ld->io_idata = 0x6c00 | *cptr;
       cptr--;
       *ld->io_idata = 0x6c00 | *cptr;

       cptr = (unsigned char *) &y+1; /*Order is important */
       /* DO y low order then high order */
       *ld->io_idata = 0x6c00 | *cptr;
       cptr--;
       *ld->io_idata = 0x6c00 | *cptr;

       /* enable the cursor in the DAC				       */
       *ld->io_idata = 0x5600;
       *ld->io_idata = 0x6c44;

       ld->io_idata = (ulong)ld->io_idata & ( ~(ulong)base_addr) ; /* base + offset - base */
       BUSMEM_DET(base_addr);

       clrjmpx(&(jmpbuf));
    }
    VDD_TRACE(MOVC , TRACE_EXIT, vp);
    return(0);

}				 /* end  of  vttmovc		 */


/*********************************************************************/
/*								     */
/*   IDENTIFICATION: VTTRDS					     */
/*								     */
/*   DESCRIPTIVE name:	Read Screen Segment			     */
/*								     */
/*   FUNCTION: Read the entire or partial content of the	     */
/*	       presentation space and convert each entry into a      */
/*	       two-byte display code and a two-byte attribute	     */
/*	       code.						     */
/*								     */
/*   INPUTS:	Virtual terminal structure pointer containing cursor */
/*		position.					     */
/*		Pointer and length of display symbol array	     */
/*		Pointer and length of display attributes array	     */
/*		Row and columns of requested area to read	     */
/*								     */
/*   OUTPUTS:   EIO                                                  */
/*		INVALID_CURSOR_POSITION 			     */
/*								     */
/*   CALLED BY: Mode Processor					     */
/*								     */
/*   CALLS:	None.						     */
/*								     */
/*********************************************************************/


static long vttrds(vp,ds,ds_size,attr,attr_size,rc)
struct vtmstruc *vp;
ushort		    *ds;	       /* array of display symbols     */
					/* returned by this procedure	*/
long		    ds_size;	       /* size of ds array	       */
ushort		    *attr;		/* array of attributes returned */
					 /* by this procedure		*/
long		    attr_size;		/* size of attr array		*/
struct vtt_rc_parms *rc;		/* string position and length	*/

{

ulong		 *buf_addr,tmp_full;
long		 i,
		 buf_offset;
ushort		 tmp_attr,tmp_char;

struct	 skyway_data *ld;

    ld = (struct skyway_data *) vp->vttld;


    if /*-----------------------*/
       /* not in character mode */
       /*-----------------------*/
	(ld->Vttenv.vtt_mode != KSR_MODE)
    {
       /*------------------------------------*/
       /* Only valid in character (KSR) mode */
       /*------------------------------------*/
       return(EIO);
    }

    VDD_TRACE(RDS , TRACE_ENTRY, vp);

    /******************************
     * read the presentation space *
     ******************************/

    buf_addr = ld->Vttenv.pse;

    /*****************************************************************
     * calculate the address of the first character that must be read *
     *****************************************************************/
    buf_offset = ((rc->start_row - 1) * ld->Vttenv.ps_size.wd +
		      (rc->start_column - 1));

    for /*********************************************************
	* all characters that must be read from the frame buffer *
	*********************************************************/

	(i=0; i < (rc->string_length); i++)
    {
	/*************************************************************
	 * read a character from the buffer and convert the character *
	 * code into a two-byte display code and the attribute into a *
	 * two-byte attribute code				      *
	 *************************************************************/

       tmp_full = *(ulong *)(buf_addr +
	       ((buf_offset + ld->Scroll_offset) % ld->Vttenv.ps_words));
       buf_offset++;

       /******************************************************
	* transform the pcm character code into a 2-byte code *
	******************************************************/

       /***************************************
	* the display code equals the pcm code *
	***************************************/
       ds[i] = tmp_full >> 16;

       /******************************
	* set the attribute bytes (2) *
	******************************/
       attr[i] = tmp_full & 0x0000ffff;
    }		/* end of for loop */

    VDD_TRACE(RDS , TRACE_EXIT, vp);
    return(0);
}				 /* end of vttrds		*/

/*********************************************************************/
/*								     */
/*   IDENTIFICATION: VTTTEXT					     */
/*								     */
/*   DESCRIPTIVE name:	Draw Text routine			     */
/*								     */
/*   FUNCTION: Draw a string of code base qualified ASCII	     */
/*	       characters into the refresh buffer of the	     */
/*	       display when a virtual terminal is active.	     */
/*	       If the terminal is inactive the characters are	     */
/*	       drawn into the presentation space that is managed     */
/*	       by the Virtual Display Driver.			     */
/*								     */
/*   INPUTS:	Virtual terminal structure pointer containing cursor */
/*		position.					     */
/*		Pointer to string to draw			     */
/*		Pointer and length of display attributes array	     */
/*		Row and columns of requested area to write	     */
/*		Flag for cursor show.				     */
/*								     */
/*   OUTPUTS:   EIO                                                  */
/*								     */
/*   CALLED BY: Mode Processor					     */
/*								     */
/*   CALLS:	draw_text					     */
/*		vttmovc 					     */
/*								     */
/*********************************************************************/

static long vtttext(vp, string, rc, cp, cursor_show)
struct vtmstruc *vp;
char		    *string;		      /* array of ascii characters    */
struct vtt_rc_parms *rc;		      /* string position and length   */
struct vtt_cp_parms *cp;		      /* code point base/mask and     */
					      /* new cursor position	      */
ulong		    cursor_show;	      /* if true cursor is moved to   */
					      /* the pos given in cp_parms    */
{

ushort		    tmp_attr,		/* Color character attribute	 */
		    len;
ulong		    *buf_addr;		     /* output buffer offset	      */
long		    buf_offset, 	     /* offset into the output buffer */
		    save_off,		     /* offset into the output buffer,*/
					     /* saved			      */
		    this_char,		     /* offset of the char to be      */
					     /* displayed		      */
		    last_char;		     /* offset of last char to be     */
					     /* displayed		      */
uchar		    temp_color; 	     /* temp area used for rev video  */
struct skyway_data  *ld;
struct sky_ddf *ddf;



    ld = (struct skyway_data *) vp->vttld;
    ddf = (struct sky_ddf *) vp->display->free_area;

     if /*-----------------------*/
	/* not in character mode */
	/*-----------------------*/
	(ld->Vttenv.vtt_mode != KSR_MODE)
     {
	/*------------------------------------*/
	/* Only valid in character (KSR) mode */
	/*------------------------------------*/
	return(EIO);
     }

     VDD_TRACE(TEXT , TRACE_ENTRY, vp);

     SET_ATTRIBUTES(cp->attributes,tmp_attr);

    /********************************
     * update the presentation space *
     ********************************/
     buf_addr = ld->Vttenv.pse;

    /*****************************************************************
     * calculate the address of the first character that must be read *
     *****************************************************************/
     save_off = buf_offset = ((rc->start_row - 1) * ld->Vttenv.ps_size.wd +
			     (rc->start_column - 1));

    /**********************************************************
     * calculate starting and ending offsets into input string *
     **********************************************************/
     this_char = (rc -> string_index) - 1;
     last_char = this_char + (rc -> string_length);

     for /*************************************
	 * all characters in the input string *
	 *************************************/

	(; this_char < last_char; this_char++)
     {
	/***********************************
	 * copy them into the output buffer *
	 ***********************************/

	*(buf_addr + ((buf_offset + ld->Scroll_offset) % 
        ld->Vttenv.ps_words)) = (ulong)
	(((string[this_char] & cp->cp_mask) + cp->cp_base) << 16) | tmp_attr;
	 buf_offset++;
     }

     if /*--------------------------------*/
	/* the virtual terminal is active */
	/*--------------------------------*/
	 (ld->Vttenv.vtt_active && !(ddf->jumpmode))
     {
	/*************************************
	 * draw the actual text on the screen *
	 *************************************/

	len = ((rc->string_length + rc->start_column-1)> ld->Vttenv.ps_size.wd)?
	ld->Vttenv.ps_size.wd - rc->start_column - 1 : rc->string_length ;
	draw_text(vp,(buf_addr + ((save_off + ld->Scroll_offset) %
					      ld->Vttenv.ps_words)),
	 len,(ushort)rc->start_column-1,(ushort)rc->start_row-1, TRUE,TRUE);
     }

     /* Update cursor show variable */
     ld->Vttenv.cursor_show = cursor_show;

     if (ld->Vttenv.cursor_show)      /* If the cursor must be moved */
	vttmovc(vp);   /* Call movc to move the cursor */

     VDD_TRACE(TEXT , TRACE_EXIT, vp);
     return(0);
}				 /* end  of  vtttext		 */


/**********************************************************************/
/*								      */
/*   IDENTIFICATION: VTTSCR					      */
/*								      */
/*   DESCRIPTIVE name:	Scroll screen up or down		      */
/*								      */
/*   FUNCTION:	Scrolls the entire contents of the	    screen    */
/*		(or presentation space if the Virtual Terminal is     */
/*		not active) up or down a specified number of lines.   */
/*								      */
/*		If not in character mode:			      */
/*                - Return EIO                                        */
/*								      */
/*		Set up attributes for area being scrolled out	      */
/*								      */
/*		If all the data has been scrolled off the screen:     */
/*		  - Update all entries in the pspace with blanks      */
/*		    and requested attributes.			      */
/*		  - Write contents of presentation space to screen    */
/*		    (clear the screen)				      */
/*								      */
/*		If data is scrolled up: 			      */
/*		  - Store a blank for every character in each line at */
/*		    bottom of screen				      */
/*								      */
/*		If data is scrolled down:			      */
/*		  - Store a blank for every character in each line at */
/*		    top of screen.				      */
/*								      */
/*		Write new Presentation space to the adapter	      */
/*								      */
/*		Move the cursor if requested			      */
/*								      */
/*   INPUTS:	Number of lines to scroll (>0 up, < 0 down)	      */
/*		Attribute for area being cleared		      */
/*		Cursor Structure containing new cursor position       */
/*		Flag to indicate whether to move cursor 	      */
/*								      */
/*   OUTPUTS:   EIO                                                   */
/*								      */
/*   CALLED BY: Mode Processor					      */
/*								      */
/*   CALLS:	sky_pxblt					      */
/*								      */
/**********************************************************************/


static long vttscr(vp, lines, attr, cursor_show)
struct vtmstruc *vp;
long		   lines;		/* number of lines to scroll	*/
					/*   >0 ==> scroll data up	*/
					/*   <0 ==> scroll data down	*/
ulong		   attr;		/* attribute associated with all*/
					/* characters of the new lines	*/
					/* that appear at either the top*/
					/* or bottom of the screen	*/
ulong		  cursor_show;		/* if true cursor is moved to	*/
					/* position given in cp_parms	*/
{

  long		    i,start;
  ulong 	    *buf_addr,		  /* address of the buffer	  */
		    tempfull,		  /* work variable		  */
		    blanks;		  /* blanks with the specifed	  */
					  /* attribute			  */

  ushort	    ysize,sx,sy,dx,dy,tmp_attr;



  union {
    ulong   full;
    struct {
      char  byte1;
      char  byte_h;
      char  byte_m;
      char  byte_l;
    } bytes;
  } startaddr;

  struct skyway_data *ld;
  struct sky_ddf *ddf;
  int parityrc,oldlev;
  label_t jmpbuf;
  caddr_t base_addr;

  ld = (struct skyway_data *) vp->vttld;
  ddf = (struct sky_ddf *) vp->display->free_area;


  if /*-----------------------*/
     /* not in character mode */
     /*-----------------------*/
      (ld->Vttenv.vtt_mode != KSR_MODE)
  {
     /*------------------------------------*/
     /* Only valid in character (KSR) mode */
     /*------------------------------------*/
     return(EIO);
  }

  VDD_TRACE(SCR , TRACE_ENTRY, vp);

  if (lines == 0 && ddf->jumpmode)
  {
     if (ld->Vttenv.vtt_active)  /* If terminal is active scroll screen */
     {
	/* In this case the timer has popped and we haven't recieved */
	/* any further scrolls to get the screen updated             */
	/* call copy_ps to refresh the screen                        */
	copy_ps(vp,TRUE);
	ddf->jumpcount = 0;
	ddf->lastcount = -1;
	ddf->scrolltime = FALSE;
	ddf->jumpmode = FALSE;
	ddf->timerset = FALSE;
	vttmovc(vp);
	return(0);
      }
      else
      {
	 /* Hot key has taken place before timer was stopped so do */
	 /* nothing!                                               */
	 return(0);
      }
   }




  /******************************************
   * will be updating the presentation space *
   ******************************************/
  buf_addr = ld->Vttenv.pse;

  /*******************************************************************
   * set up a word with one blank with the specified pc color attrib  *
   *******************************************************************/
  blanks = BLANKS_NULL_ATTR | attr ;

  /********************************
   * scroll the presentation space *
   ********************************/

  if /******************************************
     * all the data is scrolled off the screen *
     ******************************************/

      ((lines >= ld->Vttenv.ps_size.ht) || (lines <= -ld->Vttenv.ps_size.ht))
  {
     /* fill the screen with background color and the PS with blanks */

     for (i = 0; i < ld->Vttenv.ps_words; i++)
     {
	/* store a blank into the PS */
	ld->Vttenv.pse[i] = blanks;
     }

     /* If terminal is active clear the screen */

     if (ld->Vttenv.vtt_active == VTT_ACTIVE)
     {
	set_character_attributes(vp,blanks);

	parityrc = setjmpx(&(jmpbuf));
	if (parityrc != 0)
	{
	   if (parityrc == EXCEPT_IO)
	   {
	      ld->cop = (ulong) ld->cop & (~ (ulong) base_addr);
	      BUSMEM_DET(base_addr);
	      /* Log an error and return */
	      sky_err(vp,"SKYDD","vtt2drvr","setjmpx", parityrc, 
                      SKY_SETJMP, SKY_UNIQUE_17);
	      return(EIO);
	   }
	   else
	      longjmpx(parityrc);
	}

	base_addr = BUSMEM_ATT(BUS_ID,0);
	ld->cop = (ulong) ld->cop | (ulong) base_addr;   /* for sky_pxblt() */

	 /* Do a Blt to clear the screen */
	dx = (ld->Vttenv.character_box.width *
	       ld->Vttenv.ps_size.wd) - 1;
	start = (ld->Vttenv.ps_size.ht *
		 ld->Vttenv.character_box.height) - 1;
	tempfull = POForeReg | POStepBlt | PODestA | POPatFore;

	ld->colset = FALSE;
	sky_pxblt( ld, ld->center_x, ld->center_y, ld->center_x,
		   ld->center_y, dx, start,ld->bgcol , ld->bgcol,
		   3, 3, tempfull);
	ld->colset = FALSE;
	clrjmpx(&(jmpbuf));

	ld->cop = (ulong) ld->cop & (~ (ulong) base_addr);
	BUSMEM_DET(base_addr);
     }
     return(0);
  }
  else
  {
     /*********************************************************/
     /* only part of the data has been scrolled out of the PS */
     /*********************************************************/

     if /**************************
	* the data is scrolled up *
	**************************/
	 (lines > 0)
     {
	/************************************************************
	 * New method of scrolling is to simply increment a scroll   *
	 * pointer into the buffer to indicate where in the buffer   *
	 * the current upper left character is stored.		     *
	 ************************************************************/

	ld->Scroll_offset = ((ld->Scroll_offset + ld->Vttenv.ps_size.wd * lines)                              % ld->Vttenv.ps_words);

	for /**********************************************************
	    * every character in the new lines at the bottom of scree *
	    * n 						      *
	    **********************************************************/

	    (i = ((ld->Vttenv.ps_size.ht - lines) * ld->Vttenv.ps_size.wd);
	     i < ((ld->Vttenv.ps_size.ht	) * ld->Vttenv.ps_size.wd);
	     i++)
	{
	   /**********************************
	   * store a blank into the new line *
	   **********************************/

	   *(buf_addr + ((i + ld->Scroll_offset) % ld->Vttenv.ps_words))
		 = blanks;
	}
     }
     else    /* Scrolling text downward (lines < 0) */
     {
	/************************************************************
	 * New method of scrolling is to simply increment a scroll  *
	 * pointer into the buffer to indicate where in the buffer  *
	 * the current upper left character is stored.		    *
	 ************************************************************/

	ld->Scroll_offset += ld->Vttenv.ps_size.wd * lines ;

	/* Check to see if we have scrolled off top of p space */

	ld->Scroll_offset = (ld->Scroll_offset < 0 ) ?
	       ld->Vttenv.ps_words - ld->Scroll_offset : ld->Scroll_offset;


	for /**********************************************************
	    * every character in the new lines at the top of the      *
	    * screen.						      *
	    * NOTE: "lines" is negative.			      *
	    **********************************************************/

	    (i = 0; i < ((-lines) * ld->Vttenv.ps_size.wd); i++)
	{
	   /**********************************
	    * store a blank into the new line *
	    **********************************/

	   *(buf_addr + ((i + ld->Scroll_offset) % ld->Vttenv.ps_words))
		 = blanks;
	}	/* for */

     }	 /* data moved down */

  }	  /* end of updating p space */










  if (ld->Vttenv.vtt_active)  /* If terminal is active scroll screen */
  {
     if (ddf->jumpmode)
     {
	if (ddf->scrolltime || (ddf->jumpcount == ld->Vttenv.ps_size.ht))
	{
	   tstop(ddf->cdatime);
	   copy_ps(vp,TRUE);
	   ddf->jumpcount = 0;
	   ddf->lastcount = -1;
	   ddf->scrolltime = FALSE;
	   ddf->jumpmode = FALSE;
	   ddf->cdatime->timeout.it_value.tv_sec = 0;
	   ddf->cdatime->timeout.it_value.tv_nsec = 150000000;
	   ddf->timerset = TRUE;
	   tstart(ddf->cdatime);
	}
	ddf->jumpcount += lines;
	return(0);
     }
     else
     {
	 set_character_attributes(vp,blanks);
	 ddf->jumpcount += lines;

#ifdef JMPSCR
	 if (!(ddf->timerset))
	 {
	    ddf->cdatime->timeout.it_value.tv_sec = 0;
	    ddf->cdatime->timeout.it_value.tv_nsec = 150000000;
	    ddf->timerset = TRUE;
	    tstart(ddf->cdatime);
	 }
#endif

	 parityrc = setjmpx(&(jmpbuf));
	 if (parityrc != 0)
	 {
	    if (parityrc == EXCEPT_IO)
	    {
              ld->cop = (ulong) ld->cop & (~(ulong) base_addr);
	      BUSMEM_DET(base_addr);

	       /* Log an error and return */
	       sky_err(vp,"SKYDD","vtt2drvr","setjmpx", parityrc, 
                       SKY_SETJMP, SKY_UNIQUE_18);
	       return(EIO);
	    }
	    else
	       longjmpx(parityrc);
	 }



	 base_addr = BUSMEM_ATT(BUS_ID,0);
         ld->cop = (ulong) ld->cop | (ulong) base_addr;

	 dx = (ld->Vttenv.character_box.width *
	       ld->Vttenv.ps_size.wd ) - 1;

	 if (lines > 0)  /* scroll data up */
	 {
	    /* Calculate operation dimensions */
	    start = ((ld->Vttenv.ps_size.ht - lines)
		     * ld->Vttenv.character_box.height) - 1;
	    sx = ld->center_x;
	    sy = (lines * ld->Vttenv.character_box.height) + ld->center_y;
	    tempfull = POBackSrc | POForeSrc | POStepBlt | PODestA |
		     POSrcA ;

	    if (ld->poll_type)
	       while(ld->cop->pollreg > 0x7f);
	    else
	       while(ld->cop->pi_control > 0x7f);

	    sky_pxblt( ld, sx, sy, ld->center_x, ld->center_y, dx, start,
		      ld->fgcol , ld->bgcol, 3, 3, tempfull);

	    /* clear old lines out */
	    start = (lines * ld->Vttenv.character_box.height) - 1;
	    sy = ((ld->Vttenv.ps_size.ht - lines) *
		     ld->Vttenv.character_box.height) + ld->center_y;
	    sx = ld->center_x;

	    if (ld->poll_type)
	       while(ld->cop->pollreg > 0x7f);
	    else
	       while(ld->cop->pi_control > 0x7f);

	    tempfull = POBackReg | POForeReg | POStepBlt | PODestA |
		       POPatFore ;

	    ld->colset = FALSE;
	    sky_pxblt( ld, ld->center_x, ld->center_y, sx, sy, dx, start,
		       ld->bgcol, ld->bgcol, 3, 3, tempfull);
	    ld->colset = FALSE;

	 }
	 else            /* scroll data down */
	 {
	    /* make lines positive */
	    lines = -(lines);
	    /* Calculate operation dimensions */
	    start = (ld->Vttenv.ps_size.ht *
		    ld->Vttenv.character_box.height) + ld->center_y - 1;
	    sx = ld->center_x;
	    ysize = ((ld->Vttenv.ps_size.ht - lines) *
		     ld->Vttenv.character_box.height) - 1;
	    sy = ysize + ld->center_y;
	    tempfull = POBackSrc | POForeSrc | POStepBlt | PODestA |
		     POSrcA | POOct2;
	    if (ld->poll_type)
	       while(ld->cop->pollreg > 0x7f);
	    else
	       while(ld->cop->pi_control > 0x7f);


	     sky_pxblt( ld, ld->center_x, sy, ld->center_x, start, dx, ysize,
			ld->fgcol, ld->bgcol, 3, 3, tempfull);

	     /* clear old lines out */
	     sy = (lines * ld->Vttenv.character_box.height) - 1;
	     if (ld->poll_type)
		while(ld->cop->pollreg > 0x7f);
	     else
		while(ld->cop->pi_control > 0x7f);


	     tempfull = POBackReg | POForeReg | POStepBlt | PODestA |
			POPatFore ;

	     ld->colset = FALSE;
	     sky_pxblt( ld, ld->center_x, ld->center_y,ld->center_x,
			ld->center_y,  dx, sy,ld->bgcol , ld->bgcol,
			3, 3, tempfull);
	     ld->colset = FALSE;
	 }

         ld->cop = (ulong) ld->cop & (~(ulong) base_addr);
	 BUSMEM_DET(base_addr);

	 clrjmpx(&(jmpbuf));
     }                           /* end of jumpmode */

     if  /*--------------------------*/
	 /* the cursor must be moved */
	 /*--------------------------*/
       (ld->Vttenv.cursor_show = cursor_show /* assignment */)
     {
	 /*-----------------*/
	 /* move the cursor */
	 /*-----------------*/
	vttmovc(vp);
     }
  }				 /* end of active code */

  VDD_TRACE(SCR , TRACE_EXIT, vp);

  return(0);
}				 /* end of vttscr  */


/*********************************************************************/
/*								     */
/*   IDENTIFICATION: VTTSETM					     */
/*								     */
/*   DESCRIPTIVE name:	Setmode Routine 			     */
/*								     */
/*   FUNCTION: This command sets the mode of the VDD to either	     */
/*	       character or monitored mode.			     */
/*								     */
/*	       If the VDD is active and the mode is character	     */
/*	       this command:					     */
/*		  - initializes the adapter (if it was previously    */
/*		    in monitored mode or never previously	     */
/*		    initialized)				     */
/*		  - sets the VDD and adapter mode  to character      */
/*		  - copies the current contents of the		     */
/*		    VDD's presentation space into the frame buffer   */
/*		    of the CGA A/N adapter.			     */
/*		  - establishes the shape and position of the	     */
/*		    hardware cursor.				     */
/*								     */
/*	       If the VDD is active and the mode is monitored,	     */
/*	       this command:					     */
/*		  - copies the current contents of the		     */
/*		    frame buffer of the CGA A/N adapter into the     */
/*		    VDD's presentation space.			     */
/*		  - sets the adapter mode to monitored		     */
/*		  - detaches the Resource Controller from the	     */
/*		    CGA A/N SLIH				     */
/*								     */
/*   INPUTS:	Virtual terminal structure pointer		     */
/*		Mode to set terminal to 			     */
/*								     */
/*   OUTPUTS:   EIO                                                  */
/*								     */
/*   CALLED BY: Mode Processor					     */
/*								     */
/*   CALLS:	vttact						     */
/*								     */
/*       THIS IS DEAD CODE - LFT NEVER CALLS THIS FUNCTION	     */
/*								     */
/*********************************************************************/

static long vttsetm(vp,mode)
struct vtmstruc *vp;
long	 mode;				/* adapter mode: 0=> monitored,*/
					/* 1=> character, 2=> APA.     */

{
 struct skyway_data *ld;

    ld = (struct skyway_data *) vp->vttld;

    VDD_TRACE(SETM , TRACE_ENTRY, vp);

    if /************************************************
	* the requested mode is character or monitored *
	************************************************/
       ((mode == KSR_MODE) || (mode == GRAPHICS_MODE))
    {
       /*****************************************
       * set the VDD state to the specifed mode *
       ******************************************/

       ld->Vttenv.vtt_mode = mode;

       if /*--------------------------------*/
	  /* the virtual terminal is active */
	  /*--------------------------------*/
	   (ld->Vttenv.vtt_active)
       {
	  /************************
	  * Call activate routine *
	  ************************/
	  vttact(vp);
       }
    }
    VDD_TRACE(SETM , TRACE_EXIT, vp);
    return(0);
}				 /* end  of  set mode		  */

/***********************************************************************/
/*								       */
/* IDENTIFICATION: VTTSTCT					       */
/*								       */
/* DESCRIPTIVE NAME: Set color table routine			       */
/*								       */
/* FUNCTION:  This procedure sets the default color table to the color */
/*	      table passed into the routine.			       */
/*								       */
/*	      If not in character mode: 			       */
/*              - Return EIO                                           */
/*								       */
/*	      If the number of colors sent down is bigger than max     */
/*	      defined, only load what we can handle.		       */
/*								       */
/*	      Load colors onto adapter one at at time while loading    */
/*	      values into the default table.			       */
/*								       */
/* INPUTS:    Color table in the structure of vttcolt.		       */
/*	      Number of entries in the input table.		       */
/*								       */
/* OUTPUTS:   EIO                                                      */
/*								       */
/* CALLED BY: Mode processor					       */
/*								       */
/* CALLS:     load_palette					       */
/*								       */
/***********************************************************************/

static long vttstct (vp, color_table)
struct vtmstruc    *vp;
struct colorpal    *color_table;

{
 register long	   i;
 struct skyway_data *ld;
 long table_size;

    ld = (struct skyway_data *) vp->vttld;

    VDD_TRACE(STCT , TRACE_ENTRY, vp);

    if /*-----------------------*/
       /* not in character mode */
       /*-----------------------*/
	(ld->Vttenv.vtt_mode != KSR_MODE)
    {
       /*------------------------------------*/
       /* Only valid in character (KSR) mode */
       /*------------------------------------*/
       return(EIO);
    }

    table_size = color_table->numcolors;

    if (table_size > COLORTAB)	 /* Only load as many colors as the table */
       table_size = COLORTAB;	 /* can hold				  */

    ld->ct_k.nent = table_size;

    for ( i = 0; i < table_size; i++ )	/* for all of the colors in the */
					/* passed table.		*/
					/* reload default color table	*/
       ld->ct_k.rgbval[i] = color_table->rgbval[i];


    if (ld->Vttenv.vtt_active)	  /* the virtual terminal is active */
       load_palette(vp);

    VDD_TRACE(STCT , TRACE_EXIT, vp);
    return(0);
}				 /*  end  of  vttstct		 */

/***********************************************************************/
/*								       */
/* IDENTIFICATION: VTTTERM					       */
/*								       */
/* DESCRIPTIVE NAME: Terminate virtual terminal 		       */
/*								       */
/* FUNCTION:  This procedure deallocates the resources associated with */
/*	      a given virtual terminal. 			       */
/*								       */
/* INPUTS:    Virtual terminal pointer				       */
/*								       */
/* OUTPUTS:   None						       */
/*								       */
/* CALLED BY: Mode processor					       */
/*								       */
/* CALLS:     None.						       */
/*								       */
/***********************************************************************/

static long vttterm(vp)

struct vtmstruc *vp;
{
 struct skyway_data *ld;
 struct sky_ddf *ddf;

    ld = (struct skyway_data *) vp->vttld;
    ddf = (struct sky_ddf *) vp->display->free_area;

    VDD_TRACE(TERM , ENTRY, vp);

    tstop(ddf->cdatime);
    ddf->jumpcount = 0;
    ddf->lastcount = 0;
    ddf->scrolltime = FALSE;
    ddf->jumpmode = FALSE;

    /*******************************
     * free the presentation space *
     ******************************/
     xmfree(ld->Vttenv.pse, pinned_heap);

    /*******************************
     * free the local data area    *
     ******************************/
     xmfree(ld, pinned_heap);

     vp->vttld = NULL;
     vp->display->usage -= 1;

     vp->display->current_dpm_phase = 0;   /* for Display Power Management */

     VDD_TRACE(TERM , EXIT, vp);


    return(0);
}				 /* end  of  vttterm		 */



/*******************************************************/
/* Routines that Support the Rendering Context Manager */
/*******************************************************/


#ifdef EVENT_SUPPORT


/***********************************************************************/
/*								       */
/* IDENTIFICATION: ASYNC_MASK					       */
/*								       */
/* DESCRIPTIVE NAME: ASYNC_MASK - Async interrupt support code	       */
/*								       */
/* FUNCTION: Provide graphic processes the ability to recieve reports  */
/*	     on asyncronous events.				       */
/*								       */
/* INPUTS: Virtual terminal pointer				       */
/*	   Event request structure				       */
/*	   Callback function pointer				       */
/*								       */
/* OUTPUTS: None						       */
/*								       */
/* CALLED BY: Device independent rcm routine async_mask 	       */
/*								       */
/* CALLS: None							       */
/*								       */
/***********************************************************************/

static long async_mask(vp, event_req, callback)
struct vtmstruc *vp;
struct async_event *event_req;
int (*callback)();

{
  eventReport e_report;
  extern time_t time;
  struct skyway_data *ld;
  struct sky_ddf *ddf;
  int parityrc;
  label_t jmpbuf;
  caddr_t base_addr;

  ld = (struct skyway_data *) vp->vttld;

  /* Set parity handler */

  parityrc = setjmpx(&(jmpbuf));
  if (parityrc != 0)
  {
     if (parityrc == EXCEPT_IO)
     {
        ld->iop = (ulong)ld->iop * (~(ulong) base_addr);
	BUSMEM_DET(base_addr);

	/* Log an error and return */
	sky_err(vp,"SKYDD","vtt2drvr","setjmpx", parityrc, 
                SKY_SETJMP, SKY_UNIQUE_19);
	return(EIO);
     }
     else
	longjmpx(parityrc);
  }

  ddf = (struct sky_ddf *) vp->display->free_area;
  base_addr = BUSMEM_ATT(BUS_ID,0);
  ld->iop = (ulong)ld->iop | (ulong) base_addr;

  /* Check to see if this is a request to clear out interrupts */
  if (event_req->num_events == 0)
  {

     /* Leave sync interrupts on */
     ld->iop->int_enable = vp->display->pGSC->devHead.pProc->
			       procHead.pEvents->masks.s_mask;

     ld->iop = (ulong)ld->iop & (~(ulong) base_addr);
     BUSMEM_DET(base_addr);

     /* Clean out ddf structure */
     ddf->cmd &= ASYNC_OFF;

     ddf->a_event_mask = 0;
     if (ddf->cmd &= SYNC_REQ)	/* No sync requests */
	ddf->cmd &= EVENT_OFF;
     clrjmpx(&(jmpbuf));
     return(0);
  }

  /* Not a request to clear area so set up adapter as requested */
  /* set up housekeeping data					*/

  ddf->cmd |= ASYNC_REQ | EVENT_MODE;
  ddf->callback = callback;
  ddf->a_event_mask = event_req->mask;
  vp->display->cur_rcm = vp->display->pGSC->devHead.pProc;

  ld->iop->int_status = 0xff;
  ld->iop->int_enable |= ddf->a_event_mask;

  ld->iop = (ulong)ld->iop & (~(ulong) base_addr);
  BUSMEM_DET(base_addr);

  clrjmpx(&(jmpbuf));
  return(0);
}

/***********************************************************************/
/*								       */
/* IDENTIFICATION: ENABLE_EVENT 				       */
/*								       */
/* DESCRIPTIVE NAME: ENABLE_EVENT - Enable non reported events	       */
/*								       */
/* FUNCTION: Allows graphics process to turn on specific events on     */
/*	     the adapter without generating reports when an individual */
/*	     event takes place					       */
/*								       */
/* INPUTS:   Virtual terminal pointer				       */
/*	     Enable event structure				       */
/*								       */
/* OUTPUTS:  None.						       */
/*								       */
/* CALLED BY: Device independent rcm routine enable_event	       */
/*								       */
/* CALLS:    None.						       */
/*								       */
/***********************************************************************/

static long sky_enable_event(vp, event_req)
struct vtmstruc *vp;
struct enable_event *event_req;

{
   struct sky_ddf *ddf;
   struct phys_displays *pd;
   struct skyway_data *ld;
   int parityrc;
   label_t jmpbuf;
   caddr_t base_addr;

   pd = vp->display;
   ld = (struct skyway_data *) vp->vttld;

   /* Set parity handler */

   parityrc = setjmpx(&(jmpbuf));
   if (parityrc != 0)
   {
      if (parityrc == EXCEPT_IO)
      {
         ld->iop = (ulong) ld->iop  & (~ (ulong) base_addr) ;
	 BUSMEM_DET(base_addr);
	 /* Log an error and return */
	 sky_err(vp,"SKYDD","vtt2drvr","setjmpx",parityrc, 
                 SKY_SETJMP, SKY_UNIQUE_20);
	 return(EIO);
      }
      else
	 longjmpx(parityrc);
   }

   /* Take event mask and enable it on the card. Set the ddf area up */
   /* such that the event won't be reported.			     */


   ddf = (struct sky_ddf *) pd->free_area;
   ddf->cmd |= NORPT_EVENT | EVENT_MODE;
   ddf->e_event_mask = event_req->e_event;

   vp->display->cur_rcm = vp->display->pGSC->devHead.pProc;

   base_addr = BUMEM_ATT(BUS_ID,0);
   ld->iop = (ulong) ld->iop  | (ulong) base_addr;

   ld->iop->int_status = 0xff;
   ld->iop->int_enable |= event_req->e_event;

   ld->iop = (ulong) ld->iop  & (~ (ulong) base_addr) ;
   BUSMEM_DET(base_addr);

   clrjmpx(&(jmpbuf));
   return(0);
}

#endif

/***********************************************************************/
/*								       */
/* IDENTIFICATION:  MAKE_GP					       */
/*								       */
/* DESCRIPTIVE NAME: MAKE_GP - Graphics process support 	       */
/*								       */
/* FUNCTION: Return adapter addressing information to graphics	       */
/*	     process						       */
/*								       */
/* INPUTS:   RCM device structure				       */
/*	     RCM process pointer				       */
/*	     Device dependent mapping structure pointer and length     */
/*	     Trace pointer					       */
/*								       */
/* OUTPUTS:  None.						       */
/*								       */
/* CALLED BY: Device independent rcm routine make_gp.		       */
/*								       */
/* CALLS: None. 						       */
/*								       */
/***********************************************************************/

static long sky_make_gp(pdev,pproc,map,len,trace)
gscDev *pdev;
rcmProcPtr pproc;
struct sky_map	*map;
int len;
int *trace;
{
   struct ddsent *dds;

   /* This routine fills in the register mappings for this instance */
   /* of the skyway driver and returns to the gsc call		    */

   if (len < sizeof(struct sky_map))
       return -1;

   dds = (struct ddsent *) pdev->devHead.display->odmdds;
   map->io_addr = dds->io_bus_addr_start;
   map->cp_addr = dds->io_bus_mem_start;
   map->vr_addr = dds->vram_start;
   map->dma_addr[0] = dds->dma_range1_start;
   map->dma_addr[1] = dds->dma_range2_start;
   map->dma_addr[2] = dds->dma_range3_start;
   map->dma_addr[3] = dds->dma_range4_start;
   map->screen_height_mm = dds->screen_height_mm;
   map->screen_width_mm = dds->screen_width_mm;
   return(0);
}


/***********************************************************************/
/*								       */
/* IDENTIFICATION:  SYNC_MASK					       */
/*								       */
/* DESCRIPTIVE NAME: SYNC_MASK -  Sync interrupt support code	       */
/*								       */
/* FUNCTION: This procedure provides graphics processes the ability    */
/*	     to wait on a specific adapter event. The wait can be      */
/*	     syncronous (i.e. the process sleeps until the event       */
/*	     occurs) or asyncronous (i.e. the process is signalled     */
/*	     when the event occurs).				       */
/*								       */
/* INPUTS:  Virtual terminal pointer				       */
/*	    Wait event structure				       */
/*	    Callback function pointer				       */
/*								       */
/* OUTPUTS: None.						       */
/*								       */
/* CALLED BY: Device Independent RCM routine sync_mask		       */
/*								       */
/* CALLS: None. 						       */
/*								       */
/***********************************************************************/

static long sync_mask(vp, event_req, callback)
struct vtmstruc *vp;
struct wait_event *event_req;
int (*callback)();

{
   eventReport e_report;
   struct skyway_data *ld;
   int rc = 0;
   struct sky_ddf *ddf;
   int oldlevel;
   int parityrc;
   label_t jmpbuf;
   caddr_t base_addr;

   ld = (struct skyway_data *) vp->vttld;


   ddf = (struct sky_ddf *) vp->display->free_area;
   vp->display->cur_rcm = vp->display->pGSC->devHead.pProc;

   if ( event_req->wait )
   {
      /* Operation has been requested to be waited on */

      /* set up operation flags in the physical display */
      ddf->cmd |= SYNC_WAIT_REQ | SYNC_REQ | EVENT_MODE;
      ddf->callback = callback;

      /* Set parity handler */
      parityrc = setjmpx(&(jmpbuf));
      if (parityrc != 0)
      {
         if (parityrc == EXCEPT_IO)
         {
   	 	ld->iop = (ulong) ld->iop & (~ (ulong) base_addr);
   	 	ld->cop = (ulong) ld->cop & (~ (ulong) base_addr);

	 	BUSMEM_DET(base_addr);

	 	/* Log an error and return */
	 	sky_err(vp,"SKYDD","vtt2drvr","setjmpx",parityrc, 
                 	SKY_SETJMP, SKY_UNIQUE_21);
	 	return(EIO);
      	}
        else
	    longjmpx(parityrc);
      }

      base_addr = BUSMEM_ATT(BUS_ID,0);

      ld->iop = (ulong) ld->iop | (ulong) base_addr;
      ld->cop = (ulong) ld->cop | (ulong) base_addr;

      oldlevel = i_disable(INTMAX);
      ld->iop->int_status = 0xff;
      ld->iop->int_enable |= 0x80; /* enable cop complete */

      ld->cop->pixel_op_reg = event_req->mask;

      ld->iop = (ulong) ld->iop & (~ (ulong) base_addr);
      ld->cop = (ulong) ld->cop & (~ (ulong) base_addr);
      BUSMEM_DET(base_addr);

      clrjmpx(&(jmpbuf));

      /* start timer */
      ddf->cdatime->timeout.it_value.tv_sec = 1;
      ddf->cdatime->timeout.it_value.tv_nsec = 0;
      ddf->timerset = TRUE;
      tstart(ddf->cdatime);

      rc = e_sleep(&ddf->sleep_addr, EVENT_SHORT);

      i_enable(oldlevel);    /* turn interrupts back on */

      if ( (rc == EVENT_SUCC) && !(ddf->timerset))
      {
	 /* We have completed clean up and return */
	 ddf->sleep_addr = EVENT_NULL;

	 /* copy report into event structure */
	 event_req->report.event = ddf->report.event;
	 event_req->report.time =  ddf->report.time;
	 event_req->report.data[0] = ddf->report.data[0];
	 event_req->report.data[1] = ddf->report.data[1];
	 event_req->report.data[2] = ddf->report.data[2];
	 event_req->report.data[3] = ddf->report.data[3];

      }
      else  /* Handle error conditions */
      {
         /* Set parity handler */
         parityrc = setjmpx(&(jmpbuf));
         if (parityrc != 0)
         {
            if (parityrc == EXCEPT_IO)
            {
   	       ld->iop = (ulong) ld->iop & (~ (ulong) base_addr);

	       BUSMEM_DET(base_addr);

	       /* Log an error and return */
	       sky_err(vp,"SKYDD","vtt2drvr","setjmpx",parityrc, 
                               SKY_SETJMP, SKY_UNIQUE_21);
	       return(EIO);
            }
            else
	       longjmpx(parityrc);
         }

	 /* figure out error and set condition accordingly */
	 base_addr = BUSMEM_ATT(BUS_ID,0);
         ld->iop = (ulong) ld->iop | (ulong) base_addr;

	 ld->iop->int_enable &= OP_COMPLETE_CLEAR;

         ld->iop = (ulong) ld->iop & (~ (ulong) base_addr);
	 BUSMEM_DET(base_addr);

         clrjmpx(&(jmpbuf));

	 tstop(ddf->cdatime);
	 ddf->timerset = FALSE;
	 rc = -1;
      }
   }
   else
   {
      /* Set parity handler */
      parityrc = setjmpx(&(jmpbuf));
      if (parityrc != 0)
      {
         if (parityrc == EXCEPT_IO)
         {
   	    ld->iop = (ulong) ld->iop & (~ (ulong) base_addr);

	    BUSMEM_DET(base_addr);

	    /* Log an error and return */
	    sky_err(vp,"SKYDD","vtt2drvr","setjmpx",parityrc, 
                                      SKY_SETJMP, SKY_UNIQUE_21);
	    return(EIO);
         }
         else
	       longjmpx(parityrc);
      }

      /* Set up interrupt on adapter and leave */

      ddf->cmd |= SYNC_REQ | SYNC_NOWAIT_REQ | EVENT_MODE;
      ddf->callback = callback;
      vp->display->cur_rcm = vp->display->pGSC->devHead.pProc;

      base_addr = BUSMEM_ATT(BUS_ID,0);
      ld->iop = (ulong) ld->iop | (ulong) base_addr;

      ld->iop->int_status = 0xff;
      ld->iop->int_enable |= (char) event_req->mask;
	
      ld->iop = (ulong) ld->iop & (~ (ulong) base_addr);
      BUSMEM_DET(base_addr);

      clrjmpx(&(jmpbuf));

      rc = 0;
   }

 return(rc);
}



/***********************************************************************/
/*								       */
/* IDENTIFICATION: SKY_DEFINE					       */
/*								       */
/* DESCRIPTIVE NAME: SKY_DEFINE - Define device routine for Entry disp */
/*								       */
/* FUNCTION: Procedure performs all house keeping functions necessary  */
/*	     to get an adapter into the system. 		       */
/*								       */
/*	     - Allocates physical display entry 		       */
/*	       -- sets up function pointers			       */
/*	       -- initializes query information 		       */
/*								       */
/*	     - Sets POS registers on the adapter.		       */
/*								       */
/*	     - Checks to see if any other adapters have been configured*/
/*	       and adds this instance accordingly.		       */
/*								       */
/* INPUTS:   Device number					       */
/*	     Pointer ans length of dds. 			       */
/*								       */
/* OUTPUTS:  None.						       */
/*								       */
/* CALLED BY: sky_config					       */
/*								       */
/* CALLS:    None.						       */
/*								       */
/***********************************************************************/

long sky_define(devno,ddsptr,ddslen)
dev_t devno;
struct ddsent *ddsptr;
long ddslen;
{
  struct phys_displays *pd,*nxtpd;
  long i;
  union  {
   long  full;

   struct {
     char b1;
     char b2;
     char b3;
     char b4;
   } byte;
  } devdat;
  static char count;
  char slot;
  volatile char *pptr, *poscmd, *posdata, poll;
  struct sky_ddf *ddf;
  struct devsw sky_devsw;
  int rc,status;
  extern nodev();
  extern long sky_config();
  uchar ioda,ros;
  char pdat;
  label_t jmpbuf;
  caddr_t base_addr;

  /* This routine will be evoked by the configuration method which will */
  /* pass in the complete dds for the adapter. The routine will malloc	*/
  /* space for a physical display structure and fill in the function	*/
  /* pointer fields as well as the interrupt data, the display type and */
  /* the pointer to the dds structure. It then will look into the devsw */
  /* table using the iodn in the dds and determine if any other like	*/
  /* adapters have been put in the system. If none, it will invoke the	*/
  /* devswadd routine to get the driver in the system.			*/
  /* If the adapter is already in the devsw the code will walk the chain*/
  /* of struct and attach the struct to the end of the list.		*/

    pd = (struct phys_displays *) xmalloc(sizeof(struct phys_displays), 3,
                                          pinned_heap);
    if (pd == NULL)
    {
       sky_err(NULL,"SKYDD","vtt2drvr","xmalloc",pd, 
               SKY_XMALLOC, SKY_UNIQUE_22);
       return(ENOMEM);
    }

    bzero(pd, sizeof(struct phys_displays));


    count = minor(devno);
    count++;

    /* Initialize physical display structure */
    pd -> next = NULL;

    pd->interrupt_data.intr.next     = (struct intr *) NULL;
    pd->interrupt_data.intr.handler  = sky_intr;
    pd->interrupt_data.intr.bus_type = BUS_MICRO_CHANNEL;
    pd->interrupt_data.intr.flags    = 0;
    pd->interrupt_data.intr.level    = ddsptr->int_level;
    pd->interrupt_data.intr.priority = ddsptr->int_priority;
    pd->interrupt_data.intr.bid      = BUS_ID;

    /* Use the intr_args fields to get addressability to the pd table */
    /* and to set a unique value to determine if this was a skyway    */
    /* interrupt						      */


    pd->same_level = NULL;
    pd->dds_length = ddslen;
    pd->odmdds = (char *) ddsptr;

    /* Set up function pointers */

    pd->vttact =  vttact;   /*	Move the data from this
				    terminal to the display    */
    pd->vttcfl =  vttcfl;   /*	Move lines around	   */
    pd->vttclr =  vttclr;   /*	Clear a box on screen	   */
    pd->vttcpl =  vttcpl;   /*	Copy a part of the line    */
    pd->vttdact = vttdact;  /*	Mark the terminal as
				    being deactivated	       */
    pd->vttddf	= vttddf;  /*  Device Dependent stuff	 */
    pd->vttdefc = vttdefc;  /*	Change the cursor shape    */
    pd->vttdma	= vttdma;  /*  DMA Access routine    */
    pd->vttterm = vttterm;  /*	Free any resources used
				    by this VT		       */
    pd->vttinit = vttinit;  /*	setup new logical terminal */
    pd->vttmovc = vttmovc;  /*	Move the cursor to the
				    position indicated	       */
    pd->vttrds	= vttrds;   /*	Read a line segment	   */
    pd->vtttext = vtttext;  /*	Write a string of chars    */
    pd->vttscr	= vttscr;   /*	Scroll text on the VT	   */
    pd->vttsetm = vttsetm;  /*	Set mode to KSR or MOM	   */
    pd->vttstct = vttstct;  /*	Change color mappings	   */
    pd->vttpwrphase = vttpwrphase;  /*	Display power Management   */

    pd->make_gp   = sky_make_gp;    /*	Sync event support   */
    pd->sync_mask = sync_mask;    /*  Sync event support   */

#ifdef EVENT_SUPPORT
    pd->async_mask = async_mask;  /*  Async event support  */
    pd->enable_event = sky_enable_event; /*  Non report event support	*/
#endif

    pd->dev_init = sky_dev_init;
    pd->dev_term = sky_dev_term;

    pd->display_info.colors_total = 16777215;
    pd->display_info.colors_active = 16;
    pd->display_info.colors_fg = 16;
    pd->display_info.colors_bg = 16;
    for (i=0; i < 16; i++)
       pd->display_info.color_table[i] = ddsptr->ksr_color_table[i];
    pd->display_info.font_width = 12;
    pd->display_info.font_height = 30;
    pd->display_info.bits_per_pel =  1 ;
    pd->display_info.adapter_status = 0xC0000000;
    pd->display_info.apa_blink = 0x80000000;
    pd->display_info.screen_width_pel = 1280;
    pd->display_info.screen_height_pel = 1024;

    pd->display_info.screen_width_mm = ddsptr->screen_width_mm;
    pd->display_info.screen_height_mm = ddsptr->screen_height_mm;

    devdat.full = ddsptr->display_id;
    pd->disp_devid[0] = devdat.byte.b1;
    pd->disp_devid[1] = devdat.byte.b2;
    pd->disp_devid[2] = devdat.byte.b3;
    pd->disp_devid[3] = devdat.byte.b4;
				   /* Skyway physical device id */
    pd->usage = 0;		   /* Set to indicate no VTs open */
    pd->open_cnt = 0;		   /* Set to indicate driver is closed */
    pd->devno = devno;		   /* Set device number for lft use */

    /* allocate area for device dependent function problems */

    pd->free_area = (ulong *)xmalloc(sizeof(struct sky_ddf),3,pinned_heap);
    if (pd->free_area == NULL)
    {
       sky_err(NULL,"SKYDD","vtt2drvr","xmalloc",pd->free_area, 
               SKY_XMALLOC, SKY_UNIQUE_23 );
       return(ENOMEM);
    }

    /* Initialize device dependent data */
    ddf = (struct sky_ddf *) pd->free_area;
    bzero(ddf, sizeof(struct sky_ddf));
    ddf->sleep_addr = EVENT_NULL;

    if ((ddf->cdatime = (struct trb *) talloc()) == NULL)
    {
       sky_err(NULL,"SKYDD","vtt2drvr","talloc",pd->free_area, 
               SKY_TALLOC, SKY_UNIQUE_24);
       return(-1);
    }
    ddf->cdatime->flags = 0;
    ddf->cdatime->id = -1;
    ddf->cdatime->func = (void (*) ()) cda_timeout;
    ddf->cdatime->func_data = (ulong) pd;
    ddf->cdatime->ipri = ddsptr->int_priority;
    ddf->jumpmode = FALSE;


    /* Set up the pos regs for this adapter */
    if (ddsptr->slot_number == -1)
    {
       ddsptr->slot_number =
		       getport(pd->disp_devid[1],minor(devno));
       ddsptr->dma_channel = ddsptr->slot_number + 3;
    }


    if (ddsptr->slot_number == -1)
       return (-1);   /* No adapter detected */


     /* Set for use in calculating dds value */
    ioda = ((ddsptr->io_bus_addr_start - 0x2100)>>4);
    ros = (((ddsptr->io_bus_mem_start - 0xc0000)>>13)<<4);

    /* Adjust to point at coprocessor registers */
    ddsptr->io_bus_mem_start = ddsptr->io_bus_mem_start +
				0x1c00 + (ioda * 128);
     /* Now set ioda for use in pos register */
    ioda = ioda << 1;
    rc = setjmpx(&(jmpbuf));
    if (rc != 0)
    {
       if (rc == EXCEPT_IO)
       {
          IOCC_DET(base_addr);

	  /* Log an error and return */
	  sky_err(NULL,"SKYDD","vtt2drvr","setjmpx", rc, 
                  SKY_SETJMP, SKY_UNIQUE_25);
	  return(EIO);
       }
       else
	  longjmpx(rc);
    }

    slot = ddsptr->slot_number;
    base_addr = IOCC_ATT(IOCC_BID,0);      /* set up a seg. reg. to do i/o to iocc space */
    pptr = base_addr + POSREG(2,slot) + IO_IOCC;
     /* Set up pos registers for this instance of skyway based on the */
     /* count of adapters					      */
    *pptr = PosReg1 | ioda | ros;  /* Set up io and ros addresses*/
    pptr++;

    *pptr = PosReg2 | ( (ddsptr->dma_channel) << 3 ); /* set up dma level */
    pptr++;

    *pptr = PosReg3 |	   /* 4meg window values also set by posreg1 */
	    ((ddsptr->vram_start>>25)<<7);

    pptr++;

    *pptr = 0xD0;





    /* Determine if an old or new card */
    pptr = base_addr + POSREG(0,slot) + IO_IOCC;
    poscmd = pptr + 6;
    posdata = pptr + 3;
    pptr += 7;
    *poscmd = 0x47;
    poll = *posdata;

    /* If value is x then we have a new card so set flag on otherwise */
    /* leave flag at zero					      */
    if (poll == POLLVAL)
    {
       ddf->poll_type = NEW_POLL;
       pptr = base_addr + POSREG(5,slot) + IO_IOCC;
       *pptr = PosReg4;    /* Turn on Nibble mode */
       *poscmd = 0x46;
       poll = *posdata;
       if (poll & 0x01)      /* if card supports parity */
       {
	  /* ----------------
	     Parity error enable code
	     ----------------*/
	  pptr = base_addr + POSREG(7,slot) + IO_IOCC;
	  *pptr = 0x80; 		  /* Turn on extended address mode */
	  pptr = base_addr + POSREG(4,slot) + IO_IOCC;
	  *pptr |= 0xE0;		  /* Turn on Parity */
	  pptr = base_addr + POSREG(7,slot) + IO_IOCC;
	  *pptr = 0x00; 		  /* Turn off extended address mode */
       }
    }


    IOCC_DET(base_addr);
    clrjmpx(&jmpbuf);

    /* fill in dds fields according to pos configuration */
    pd->d_dma_area[0].bus_addr = ddsptr->dma_range1_start;
    pd->d_dma_area[0].length = 0x200000;
    pd->d_dma_area[1].bus_addr = ddsptr->dma_range2_start;
    pd->d_dma_area[1].length = 0x200000;
    pd->d_dma_area[2].bus_addr = ddsptr->dma_range3_start;
    pd->d_dma_area[2].length = 0x200000;
    pd->d_dma_area[3].bus_addr = ddsptr->dma_range4_start;
    pd->d_dma_area[3].length = 0x200000;
    pd->num_domains = 1;
    pd->dwa_device = 0;

    /* Set up bus authorization */
    pd->io_range = (ddsptr->io_bus_addr_start << 16) |
			   (ddsptr->io_bus_addr_start + 15);

    /* Now decide where to put the structure */

    devswqry(devno,&status,&nxtpd);

    if (nxtpd != NULL)
    {
      /* When we reach here at leat one other skyway has been defined in */
      /* the system so we need to calculate where to hook the display	 */
      /* structure into the chain of displays				 */

       while ( nxtpd->next != NULL )
       {
	  nxtpd = nxtpd->next;
       }
       /* When we fall out of the loop nxtpd will point to the structure */
       /* that the new display should be attached to.			 */
       nxtpd->next = pd;
     }
     else
     {	/* if this pointer is null then the card  is being configged for */
	/* the first time. In this case we must do a devswadd to get the */
	/* driver into the devswitch.					 */
	sky_devsw.d_open = sky_open;
	sky_devsw.d_close = sky_close;
	sky_devsw.d_read = nodev;
	sky_devsw.d_write = nodev;
	sky_devsw.d_ioctl = sky_ioctl;
	sky_devsw.d_strategy = nodev;
	sky_devsw.d_select = nodev;
	sky_devsw.d_config = sky_config;
	sky_devsw.d_print = nodev;
	sky_devsw.d_dump = nodev;
	sky_devsw.d_mpx = nodev;
	sky_devsw.d_revoke = nodev;
	sky_devsw.d_dsdptr = (char *) pd;
	sky_devsw.d_ttys = NULL;
	sky_devsw.d_selptr = NULL;
	sky_devsw.d_opts = 0;

	/* call devswadd to get us into the switch table */
	rc = devswadd(devno, &sky_devsw);
	if (rc != 0)
	{
	   /* Log an error */
	   sky_err(NULL,"SKYDD","vtt2drvr","devswadd",rc, 
                   SKY_DEVADDFAIL, SKY_UNIQUE_26 );
	   return(EFAULT);
	}


     }
    return(0);
}

/***********************************************************************/
/*								       */
/* IDENTIFICATION: SKY_IOCTL					       */
/*								       */
/* DESCRIPTIVE NAME: SKY_IOCTL- Interrupt handler for Entry Display    */
/*								       */
/* FUNCTION: Standard IOCTL entry point for AIX device driver.	       */
/*								       */
/* INPUTS: Device number					       */
/*	   Command						       */
/*	   Argument for command 				       */
/*	   Device flag (not used by this driver)		       */
/*	   Channel (ignored by this driver)			       */
/*	   Extension pratmeter (ignored by this driver) 	       */
/*								       */
/* OUTPUTS: None.						       */
/*								       */
/* CALLED BY: Hft						       */
/*								       */
/* CALLS: None. 						       */
/*								       */
/***********************************************************************/

long sky_ioctl(devno,cmd,arg,devflag,chan,ext)
dev_t devno;	 /* The device number of the adapter */
long cmd;	/* The command for the ioctl */
long arg;	/* The address of paramter block */
ulong devflag;	/* Flag indicating type of operation */
long chan;		/* Channel number ignored by this routine */
long ext;		/* Extended system parameter - not used   */
{

#ifdef LFTDIAG

struct hfquery tu_qry_buf;
struct hfqdiagc tu_cmd;
struct skyqdiagr tu_resp;
short i;
struct phys_displays *pd;
struct sky_ddf *ddf;
int status;
struct ddsent *dds;


 /* Get phys_display pointer THIS CODE ASSUMES FIRST skyway is minor 0 */
 i = minor(devno);
 devswqry(devno,&status,&pd);

 while ( pd != NULL )
 {
    if (pd->devno == devno)
       break;
    else
       pd = pd->next;
 }

 if (pd == NULL){
    sky_err(NULL,"SKYDD","vtt2drvr","devswqry", pd, 
            SKY_DEVQFAIL, SKY_UNIQUE_27);
    return(-1);      /* No display present */
    }

 ddf = (struct sky_ddf *) pd->free_area;
 dds = (struct ddsent *) pd->odmdds;

 switch (cmd)
 {
   case HFQUERY:    /* Request for diagnostic support */
   {

     if (copyin(arg, &tu_qry_buf, sizeof(struct hfquery)))
     {
	 /* error has occurred */
	 sky_err(NULL,"SKYDD","vtt2drvr","copyin",NULL, 
                 SKY_COPYINFAIL, SKY_UNIQUE_28 );
	 return(EFAULT);
     }

     if (copyin(tu_qry_buf.hf_cmd, &tu_cmd,tu_qry_buf.hf_cmdlen))
     {
	 /* error has occurred */
	 sky_err(NULL,"SKYDD","vtt2drvr","copyin",NULL, 
                 SKY_COPYINFAIL, SKY_UNIQUE_29 );
	 return(EFAULT);
     }


     if (( tu_cmd.hf_intro[7] == HFQDIAGCH) &&
	 ( tu_cmd.hf_intro[8] == HFQDIAGCL) &&
	 ( tu_cmd.hf_device == HF_DISPLAY))
     {
	   /* All parameters are correct proceed with diagnostic steps */
	switch (tu_cmd.hf_testnum)
	{
	   case SKY_START_DIAG:
	    /* Set up ddf structure in phys_display to count interrupts */

	      ddf->diaginfo[0] = 0; /* Frame flyback */
	      ddf->diaginfo[1] = 0; /* Vertical Blanking End */
	      ddf->diaginfo[2] = 0; /* Sprite End */
	      ddf->diaginfo[3] = 0; /* Cop_access_reject */
	      ddf->diaginfo[4] = 0; /* Cop_op_complete */

	      ddf->cmd = DIAG_MODE;
	   break;

	   case SKY_QUERY_DIAG:

	     /* Reset ddf structure in phys_display to count interrupts */
	     tu_resp.hf_intro[0] = HFINTROESC;
	     tu_resp.hf_intro[1] = HFINTROLBR;
	     tu_resp.hf_intro[2] = HFINTROEX;
	     tu_resp.hf_intro[3] = 0x0;
	     tu_resp.hf_intro[4] = 0x0;
	     tu_resp.hf_intro[5] = 0x0;
	     tu_resp.hf_intro[6] = 0xA;
	     tu_resp.hf_intro[7] = HFQDIAGRH;
	     tu_resp.hf_intro[8] = HFQDIAGRL;
	     tu_resp.hf_result[0] = ddf->diaginfo[0];
	     tu_resp.hf_result[1] = ddf->diaginfo[1];
	     tu_resp.hf_result[2] = ddf->diaginfo[2];
	     tu_resp.hf_result[3] = ddf->diaginfo[3];
	     tu_resp.hf_result[4] = ddf->diaginfo[4];


	     /* Reset counters */
	     ddf->diaginfo[0] = 0; /* Frame flyback */
	     ddf->diaginfo[1] = 0; /* Vertical Blanking End */
	     ddf->diaginfo[2] = 0; /* Sprite End */
	     ddf->diaginfo[3] = 0; /* Cop_access_reject */
	     ddf->diaginfo[4] = 0; /* Cop_op_complete */
	     if (copyout(&tu_resp,tu_qry_buf.hf_resp,tu_qry_buf.hf_resplen))
	     {
		sky_err(NULL,"SKYDD","vtt2drvr","copyout",NULL, 
                        SKY_COPYOUTFAIL, SKY_UNIQUE_30);
		return (EFAULT);
	     }
	   break;

	   case SKY_STOP_DIAG:
	    /* Clear up ddf structure in phys_display */

	     ddf->cmd = NULL;

	   break;

	 }  /* End of command switch */

     }
     else
     {
	sky_err(NULL,"SKYDD","vtt2drvr","bad query",NULL, 
                SKY_BADCMD, SKY_UNIQUE_31);
	return(EINVAL);
     }
   break;
   }

   case HOOKUP:    /* Request for diagnostic support */
   {
      ddf->sm_enq = arg;
      break;
   }

   default:
   {
      sky_err(NULL,"SKYDD","vtt2drvr","badcmd",NULL, 
              SKY_BADCMD, SKY_UNIQUE_32 );
      return(EINVAL);
   }

  } /* End of switch */

#endif

return (0);

} /* sky_ioctl */

/***********************************************************************/
/*								       */
/* IDENTIFICATION: SKY_OPEN					       */
/*								       */
/* DESCRIPTIVE NAME: SKY_OPEN - Open device routine for PC Mono        */
/*								       */
/* FUNCTION: This procedure is the device driver standard open routine */
/*	     It's function is to reset the adapter and allocate system */
/*	     resources necessary for the devide driver. 	       */
/*								       */
/* INPUTS:   Device number					       */
/*	     Flags for open type (ignored)			       */
/*	     Channel number (ignored)				       */
/*	     Extended parameter (ignored)			       */
/*								       */
/* OUTPUTS:  None.						       */
/*								       */
/* CALLED BY: lftconfig 					       */
/*								       */
/* CALLS:    skyway_reset					       */
/*								       */
/***********************************************************************/

long  sky_open(devno,flag,chan,ext)
dev_t devno;		/* Major and minor number of the device */
long flag;		/* flags for type of open.		*/
long chan;		/* Channel number ignored by this routine */
long ext;		/* Extended system parameter - not used   */

{
  long i,rc,status;
  struct phys_displays *pd;
  struct intr *intdat;	    /* interrupt level handler structure       */
  struct skyway_data lddummy;
  int channel_id;
  struct ddsent *dds;
  caddr_t base_addr;

  /* This routine is invoked by the lft config routine. Open	  */
  /* completes installing the driver into the system and	  */
  /* allocates the interrupt and dma channels according to what   */
  /* was passed in the dds. The routine also pins the interrupt   */
  /* handler and resets the adapter.				  */

  devswqry(devno,&status,&pd);

  while ( pd != NULL )
  {
     if (pd->devno == devno)
	break;
     else
	pd = pd->next;
  }

  if (pd == NULL)
  {
     sky_err(NULL,"SKYDD","vtt2drvr","devswqry", pd, 
             SKY_DEVQFAIL, SKY_UNIQUE_33);
     return(-1);      /* No display present */
  }

  dds = (struct ddsent *) pd->odmdds;

  if (pd->open_cnt == 0)
  {
     /* set the open_cnt field in the physical display entry */
     pd->open_cnt = 1;

     /* Pin driver code into memory */
     pincode((void *)sky_intr);

     /* Initialize DMA channel */

     dds = (struct ddsent *) pd->odmdds;
     pd->dma_chan_id =	d_init((dds->dma_channel),
			       MICRO_CHANNEL_DMA, BUS_ID);

     if (pd->dma_chan_id == DMA_FAIL)
     {
	sky_err(NULL,"SKYDD","vtt2drvr","d_init",pd->dma_chan_id, 
                SKY_DMAFAIL, SKY_UNIQUE_34);
	return(EIO);
     }


     if (i_init(&(pd->interrupt_data)) != INTR_SUCC)
     {
	sky_err(NULL,"SKYDD","vtt2drvr","i_init",NULL, 
                SKY_INTRFAIL, SKY_UNIQUE_35 );
	return(EIO);
     }

     /* Enable the interrupt level by calling i_unmask */
     i_unmask(&(pd->interrupt_data));

     /* Set up fake lddummy for call to reset routine */


     lddummy.fgcol = 5;
     lddummy.bgcol = 7;

     lddummy.iop = dds->io_bus_addr_start ;
     lddummy.io_idata = dds->io_bus_addr_start + 10 ;
     lddummy.cop = dds->io_bus_mem_start ;
     lddummy.skymem = dds->vram_start;
     lddummy.ct_k.rgbval[0] = 0x00000000;
     lddummy.disp_type = pd->disp_devid[1];
     /* reset the adapter */
     base_addr = BUSMEM_ATT(BUS_ID,0);    /* set up seg to do i/o to bus mem space */

/* BUG --> here ld.poll_type is not set, but it is used in skyway_reset */

     skyway_reset(&lddummy,base_addr);

     /* Set interrupt level on the adapter */
     /* The card supports levels 9-12, to get level 9 a 0 must be sent */
     /* to index register 6e to do this subtract 9 from the level set  */
     /* inthe dds.						      */

     lddummy.io_idata = (ulong)lddummy.io_idata + (ulong)base_addr;
     *lddummy.io_idata = 0x6e00 | (dds->int_level - 9);
     BUSMEM_DET(base_addr);
     return(0);
  }
  else	  /* Driver is already opened */
   return(-1);
}

/***********************************************************************/
/*								       */
/* IDENTIFICATION: SKY_CLOSE					       */
/*								       */
/* DESCRIPTIVE NAME: SKY_CLOSE - Close routine for SKYWAY	       */
/*								       */
/* FUNCTION: Standard device driver close routine - does opposite of   */
/*	     open in that it frees systems resources used by device    */
/*	     driver.						       */
/*								       */
/* INPUTS:   Device number					       */
/*	     Channel number (ignored)				       */
/*	     Extended paramter (ignored)			       */
/*								       */
/* OUTPUTS:  None.						       */
/*								       */
/* CALLED BY: lftconfig 					       */
/*								       */
/* CALLS:  None.						       */
/*								       */
/***********************************************************************/

long  sky_close(devno,chan,ext)
dev_t devno;
long chan;		/* Channel number ignored by this routine */
long ext;		/* Extended system parameter - not used   */
{
 struct phys_displays *pd;
 long status=-1;
 struct ddsent *dds;

 /* This routine will check the usage field in the Phys_display  struct */
 /* and see if it is non-zero. If so we can close the device if not then*/
 /* a -1 is return to the caller					*/

 /* Here will be the devsw traversal for devices which can have multiple*/
 /* instances.								*/

 devswqry(devno,&status,&pd);

 while ( pd != NULL )
 {
    if (pd->devno == devno) 
	break;
    pd = pd->next;
 }

 /* Check if it is open. If not dont do anything  */
 /* this is a bogus call to close */
 if (pd->open_cnt == 0)
    return(-1);

 if (pd == NULL) {
    sky_err(NULL,"SKYDD","vtt2drvr","devswqry", pd, 
            SKY_DEVQFAIL, SKY_UNIQUE_36);
    return(-1);      /* No display present */
  }

 /* test to see if the device is in use */
 if (pd->usage > 0){
    sky_err(NULL,"SKYDD","vtt2drvr",NULL, pd->usage, 
            SKY_DEVINUSE, SKY_UNIQUE_37);
    return(-1);        /* someone is still using it */
    }

 /* Clean up system resources */
 i_clear(&(pd->interrupt_data));
 dds = (struct ddsent *) pd->odmdds;
 d_clear(dds->dma_channel);

 pd->open_cnt = 0;  /* set the open count to 0 */
 /* unpin the code, since we pin the code each time we call sky_open()  */
 unpincode(sky_intr);

 return(0);
}

/***********************************************************************/
/*								       */
/* IDENTIFICATION:  DD_CONFIG					       */
/*								       */
/* DESCRIPTIVE NAME: DD_CONFIG - Close routine for Pc Mono	       */
/*								       */
/* FUNCTION: Standard device driver config routine - Configures,       */
/*	     unconfigures and returns vpd to calling config method.    */
/*								       */
/*	     Routine is passed in command and responds as follows:     */
/*								       */
/*	     CFG_INIT - copies in dds information and invokes	       */
/*			sky_define routine to get adapter instance     */
/*			into system.				       */
/*								       */
/*	     CFG_TERM - frees physical display structure thus	       */
/*			unconfiguring adapter instance. 	       */
/*								       */
/*	     CFG_QVPD - Returns vital product data off of adapter      */
/*								       */
/* INPUTS:   Device number					       */
/*	     Command ( as listed above )			       */
/*	     Uiop pointer to user space data (buffer to read vpd       */
/*	     into or dds information)				       */
/*								       */
/* OUTPUTS:  None.						       */
/*								       */
/* CALLED BY: Entry display config method			       */
/*								       */
/* CALLS:    sky_define 					       */
/*								       */
/***********************************************************************/

sky_config(devno, cmd, uiop)
dev_t devno;
long cmd;
struct uio *uiop;
{
  struct ddsent *dp;
  int rc,min_num,i,status;
  struct phys_displays *pd,*last;
  char pdat, *pptr,*poscmd,*posdata,vpddat[256];
  static short j,found[4],slot;
  caddr_t	base_addr;

  /* THis routine switches off of the command passed in and either inits */
  /* terminates or does a query of the VPD on the adapter.		 */

  switch (cmd)
  {
    case CFG_INIT: /* In this case we need to get our driver in to the */
		   /* system by allocating a phys display struct and   */
		   /* filling it in then adding our driver to the devsw*/
		   /* table.					       */
		   /* For this level of implementation we are compiled */
		   /* into the devsw so all we do is invoke dd_define  */
		   /* to build our phys display struct and hook it in  */
		   dp = (struct ddsent *) xmalloc(sizeof(struct ddsent), 3,
                        pinned_heap);
		   if (dp == NULL)
		   {
		      sky_err(NULL,"SKYDD","vtt2drvr","xmalloc",dp, 
                              SKY_XMALLOC, SKY_UNIQUE_38 );
		      return(ENOMEM);
		   }

		   rc = uiomove(dp,sizeof(struct ddsent), UIO_WRITE, uiop);
		   if (rc != 0)
		      return(rc);

		   rc = sky_define(devno,dp,sizeof(struct ddsent));


		   return(rc);

    case CFG_TERM: /* In this case we would remove all malloc'd data   */
		   /* and remove ourselves from the device switch table*/
		   /* if we are the last device of this type.	       */

		   rc = sky_undefine(devno);
		   return(rc);

    case CFG_QVPD: /* This function returns the Vital Product Data from*/
		   /* the adapter.				       */


		   devswqry(devno,&status,&pd);

		   while ( pd != NULL )
		   {
		      if (pd->devno == devno)
			 break;
		      else
			 pd = pd->next;
		   }

		   if (pd == NULL){
    		      sky_err(NULL,"SKYDD","vtt2drvr","devswqry", pd, 
                              SKY_DEVQFAIL, SKY_UNIQUE_39);
		      return(-1);      /* No display present */
		   }

		   base_addr = IOCC_ATT(IOCC_BID,0);  /* Gain access to iocc space */

		   dp = (struct ddsent *) pd->odmdds;
		   slot = dp->slot_number;
		   pptr = base_addr + POSREG(0,slot) + IO_IOCC;
		   poscmd = pptr + 6;
		   posdata = pptr + 3;
		   pptr += 7;
		   for (i=1; i<256; i++)
		   {
		      *poscmd = i;
		      vpddat[i] = *posdata;
		   }


		   /* Write data back out to user space */
		   rc = uiomove(vpddat, 256, UIO_READ, uiop);

		   if (rc != 0)
		   {
		      IOCC_DET(base_addr);
		      return(rc);
		   }

		   IOCC_DET(base_addr);
		   break;


    default:

    	sky_err(NULL,"SKYDD","vtt2drvr","case : default", 
                "switch(cmd)", SKY_BADCMD, SKY_UNIQUE_40);
	return(-1);

  }



  return (0);
}		    /* end of pcm_config */


long getport(type,minnum)
char type;
short minnum;
{
 /* This will walk all of the slots in the machine looking for an adapter */
 /* that matched the given id value.					  */
 char pdat, *pptr;
 static short i,j,found[4];
 caddr_t base_addr;

 if (minnum == 0)
 {
   for (i=0;i<4; i++)
      found[i] = 255;
 }

 base_addr = IOCC_ATT(IOCC_BID,0);  /* set up a segment reg to do i/o to iocc space */

 for ( i=0; i < 12; i++)
 {
   /* loop looking for a skyway adapter */

   pptr = base_addr + POSREG(0,i) + IO_IOCC;
   if (*pptr == (type+0xc3))
   {
     /* first byte matches see about second */
     pptr++;

     if (*pptr == 0x8e)
     {
       for (j=0; j<4; j++)
       {
	  if (found[j] == i)  /* we have found this card before */
	    break;
	  else		    /* New slot found */
	  {
	    if (i > found[j])
	       continue;
	    else
	    {
	      found[j] = i;
	      return(i);
	    }
	  }
       }
     }
   }
 }

 /* if we get here then we be in trouble cause there is no skyway */
 IOCC_DET(base_addr);
 return (-1);
}

/****************************************************************************

	Function Name: sky_dev_init

	Descriptive Name: Device dependent initialization

 * ------------------------------------------------------------------------*

	Function:

	This routine does the set up of the TCW's via the d_protect call.
	
	The bus authorization mask in the display structure is set up in
	the device independent dev_init function.


***************************************************************************/

sky_dev_init(pdev)
gscDevPtr	pdev;

{
	struct phys_displays *pd = pdev->devHead.display;
  	struct ddsent *ddsptr = (struct ddsent *) pd->odmdds;

    	d_protect(NULL, ddsptr->io_bus_mem_start, 128, 
                  pd->busmemr[0].auth_mask, BUS_ID);
    	d_protect(NULL, ddsptr->vram_start, 0x400000, 
                  pd->busmemr[0].auth_mask, BUS_ID);


        return 0;

}

/****************************************************************************

	Function Name: sky_dev_term

	Descriptive Name: Device dependent cleanup

 * ------------------------------------------------------------------------*

	Function:

	This function is currently a NULL function.  

***************************************************************************/

sky_dev_term(pdev)
gscDevPtr	pdev;
{
}

/***********************************************************************/
/*								       */
/* IDENTIFICATION: SKY_UNDEFINE 				       */
/*								       */
/* DESCRIPTIVE NAME: SKY_UNDEFINE  - Undefine phys disp for devno      */
/*								       */
/* FUNCTION: Procedure performs all functions necessary		       */
/*	     to unconfigure an adapter from the system.		       */
/*								       */
/*	     - Checks to see if multiple adapters have been configured */
/*	       and calls releasepd to do the right stuff               */
/*								       */
/* INPUTS:   Device number					       */
/*								       */
/* OUTPUTS:  error status.					       */
/*								       */
/* CALLED BY: sky_config					       */
/*								       */
/* CALLS:    sky_releasepd					       */
/*								       */
/***********************************************************************/

int sky_undefine(devno)
dev_t devno;
{
  struct phys_displays *pd,*curr_pd;
  struct phys_displays *prev_pd = NULL;
  int count=0,releasethis=0;
  int status;

  /*  Get all the info first */
  devswqry(devno,&status,&pd);

  if ( pd == NULL ) {
     sky_err(NULL,"SKYDD","vtt2drvr","devswqry", pd, SKY_DEVQFAIL,
            SKY_UNIQUE_41);
     return(-1);      /* No display present */
  }

  /*  Find out how many phys_disps are attached and flag the one that needs 
      to be released */
  while ( pd != NULL )
  {
     count++;
     if (pd->devno == devno) {
        releasethis=count;
        curr_pd = pd;	/* this is the pd we need to remove */
     }
     if ( ! releasethis )
        prev_pd = pd;
     pd = pd->next;
  }

  if(!releasethis) { /* we dont know what we are releasing */
     sky_err(NULL,"SKYDD","vtt2drvr",NULL, releasethis,
            SKY_DEVQFAIL, SKY_UNIQUE_42);
     return(-1);      /* No display present */
  }

  if (curr_pd->open_cnt > 0 ) 
     return(EBUSY);      /* display is being used */
   
  return(sky_releasepd(prev_pd, curr_pd,count,releasethis));
}

/***********************************************************************/
/*								       */
/* IDENTIFICATION: SKY_RELEASEPD 				       */
/*								       */
/* DESCRIPTIVE NAME: SKY_RELEASEPD - Release phys disp for devno passed*/
/*								       */
/* FUNCTION: Procedure performs all functions necessary		       */
/*	     to release a physical display from the system.	       */
/*								       */
/*	     - Calls devswdel if there is only one physical display    */
/*	       present and which has to be released.		       */
/*	     - Deallocates the specific physical display entry 	       */
/*	       whose devno matches with requested value if more than   */
/*	       one physical display is found by calling sky_releasepd  */
/*	     - Depending on which physical display has to be released  */
/*	       the necessary copying/releasing is done   	       */
/*								       */
/* INPUTS:  pd to be released, the prev pd, no of displays, releasethis*/
/*								       */
/* OUTPUTS:  error status.					       */
/*								       */
/* CALLED BY: sky_undefine					       */
/*								       */
/* CALLS:  							       */
/*								       */
/***********************************************************************/
int sky_releasepd(prev_pd,curr_pd,count,releasethis)
struct phys_displays *prev_pd, *curr_pd;
int count,releasethis;
{
  struct phys_displays *tmp_pd;
  int status,oldlevel;

  xmfree(curr_pd->odmdds,pinned_heap);
  xmfree(curr_pd->free_area,pinned_heap);

  if (releasethis == count) { /* this is the tail or the only one */

    if(count == 1) {
      /* need to remove from device swith table which also releases pd */
      status = devswdel(curr_pd->devno);
      if(status) {
           sky_err(NULL,"SKYDD","vtt2drvr","devswdel", status,
                    SKY_DEVDELFAIL, SKY_UNIQUE_43);
           return(status);      /* reason why devswdel failed */
      }
    } else {
      xmfree(curr_pd,pinned_heap);
      prev_pd->next = NULL;
    }
    return(0);
  } /* end of processing tail */
 
  if ((releasethis==1) && (count > 1)) { /* special copy case */
    
    tmp_pd = curr_pd->next;

    /* Copy the 2nd pd into the first pd */
    bcopy((char *)curr_pd->next,(char *)curr_pd,sizeof(struct phys_displays));

    xmfree(tmp_pd,pinned_heap);
    return(0);
  }
  else { /* must be a structure in the middle so remove it & reset pointers */
    prev_pd->next = curr_pd->next; /* link next with previous */
    xmfree(curr_pd,pinned_heap);
    return(0);
  }
}

  
/****************************************************************************

	Function Name: load_font_table

	Descriptive Name: Load local font table

 * ------------------------------------------------------------------------*

	Function:

	This function will load the local font table with font data stored
        in the lft font table. The lft font table may have from 1 to 8
        fonts stored in the table. Only one font is selected from the lft
        table and will be loaded into all eight indices of the local font 
        table. If a custom font is specified (font_index >= 0), then it will
        be used to determine the font otherwise the first font in the lft
        table will be used.

***************************************************************************/


void load_font_table(vp)
  struct vtmstruc *vp;
{
  int i, j;	/* temporary variables */
  struct skyway_data *ld;

  /* get the local data structure pointer */
  ld = (struct skyway_data *) vp->vttld;

  /* set the the current font to -1 to force the font to be loaded */
  ld->cur_font = -1; 

  /* select the custom font index if specified otherwise use first font */
  i = (vp->font_index >= 0) ? vp->font_index : 0;

  /* load the selected font into the entire font array */
  for (j = 0; j < ADAPT_MAX_FONTS; j++)
  {
     ld->Vttenv.font_table[j].index  = 0;
     ld->Vttenv.font_table[j].addr   = (char *) vp->fonts[i].font_ptr;
     ld->Vttenv.font_table[j].id     = vp->fonts[i].font_id;
     ld->Vttenv.font_table[j].height = vp->fonts[i].font_height;
     ld->Vttenv.font_table[j].width  = vp->fonts[i].font_width;
     ld->Vttenv.font_table[j].size   = vp->fonts[i].font_size >> 2;
  }
}

/*
 *  Display Power Management Function
 *  
 *      All we do is turned blanking on. 
 */
static long vttpwrphase(pd, phase)
struct phys_displays *pd;
int phase;                         /* 1=full-on, 2=standby, 3=suspend, 4=off */
{
	int parityrc, oldlvl;
	label_t jmpbuf;
	struct skyway_data *ld;
        caddr_t base_addr;
	uchar old_index_reg_val;
	uchar volatile * p_index_reg;

	ld = (struct skyway_data *) pd->visible_vt->vttld;
	/* Set parity handler */
	parityrc = setjmpx(&(jmpbuf));
	if (parityrc != 0)
	{
	   if (parityrc == EXCEPT_IO)
	   {
	      BUSMEM_DET(base_addr);
              ld->io_idata = (ulong)ld->io_idata & ( ~ (ulong)base_addr);

	      /* Log an error and return */
	      sky_err(pd->visible_vt,"SKYDD","vtt2drvr","setjmpx", 
	              parityrc, SKY_SETJMP, SKY_UNIQUE_1);
	      return;
	   }
	   else
	      longjmpx(parityrc);
	}

	base_addr = BUSMEM_ATT(BUS_ID,0);

	ld->io_idata = (ulong)ld->io_idata | (ulong)base_addr;

	/* 
         *  Don't know if I need to disable interrrupt here.
         *
         *  This function is called by lft and X for 
         *  display power management.  In lft, we run under 
         *  timer interrupt.  In X, we run under screen saver which calls loaddx which 
         *  calls us (dd) 
         *  
         */

	p_index_reg = ld->io_idata;          /* pointer to index register      */

	old_index_reg_val = *p_index_reg;    /* save content of index register */

	/* 
         * 1. if caller wants to turn on the display when it's off, we turn it on,
         *    and then update current DPM state 
         * 2. if caller wants to turn off the display when it's on, we turn it off.  
         *    and then update current DPM state 
         * 3. Other than that just update current DPM state 
         * 
	 *    Note initially current_dpm_phase is zero because the whole pd is bzeroed out
         *    Also, every lft is unconfigured, it calls vttterm.  In there current_dpm_phase
         *    is set to zero again.
         */  

	if ((phase == 1) && (phase != pd->current_dpm_phase))    /* case 1 */
	{

  	   if (ld->disp_type == SKY_COLOR)
		*ld->io_idata = DISPLAY_MODE_1 | MODE_1_DATA;
	   else
		*ld->io_idata = DISPLAY_MODE_1 | MODE_1_DATA_M;

             	*p_index_reg= old_index_reg_val ;     /* restore index register */ 
	}
	else if ((phase != 1) && (pd->current_dpm_phase < 2 ))    /* case 2  */
	{

	   *ld->io_idata = DISPLAY_MODE_1 | MODE_1_PRERESET;
           *p_index_reg= old_index_reg_val ;     /* restore index register */ 
	}

	pd->current_dpm_phase = phase;  /* update the state variable */

       	ld->io_idata = (ulong)ld->io_idata & ( ~ (ulong)base_addr);

       	BUSMEM_DET(base_addr);

       	clrjmpx(&(jmpbuf));

       	return(0);
}
