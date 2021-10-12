static char sccsid[] = "@(#)79	1.1  src/htx/usr/lpp/htx/lib/hga/util.c, tu_hga, htx410 6/2/94 11:37:12";
/*
 *   COMPONENT_NAME: tu_hga
 *
 *   FUNCTIONS: MAX
 *		MIN
 *		SetScissorSize
 *		blt_sc_sc
 *		bt485_goto_xy
 *		color_xform
 *		draw_H
 *		draw_line
 *		draw_rec
 *		full_scrn_chr
 *		lines_test
 *		s3_GMTest
 *		s3_clear_screen
 *		s3_putchar
 *		s3_rect_fill
 *		scr_h
 *		scrl_scrn
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <stdio.h>
#include <sys/types.h>

#include "hga_extern.h"
#include "s3regs.h"
#include "hgautils.h"
#include "colors.h"
#include "screen.h"
#include "isofont.h"
#include "hxihtx.h"

extern SCR_INFO screen;

# define MAX( a, b )   ((( a ) > ( b )) ? ( a ) : ( b ))
# define MIN( a, b )   ((( a ) < ( b )) ? ( a ) : ( b ))

#define NUM_COLORS	15

static line_color[NUM_COLORS] = {
	CLR_BLUE,CLR_RED, CLR_PINK, CLR_GREEN, CLR_CYAN, CLR_YELLOW,
	CLR_NEUTRAL, CLR_DARKGRAY, CLR_DARKPINK, CLR_DARKGREEN,
	CLR_DARKCYAN, CLR_BROWN, CLR_PALEGRAY, CLR_WHITE, CLR_BLACK };

static colorbars[] = {
	CLR_BLACK, CLR_BLUE, CLR_GREEN, CLR_YELLOW, CLR_BROWN, CLR_CYAN, CLR_RED, 
	CLR_DARKGRAY};

static int odf_fix = 0;
static int draw_color;
static int f_color = CLR_YELLOW;
static int b_color = CLR_RED;

static color_xform();
static void bt485_goto_xy();


static uchar	x_fudge[4] = {
	8, 	/* xfudge for Monitor = 0 */
	8,	/* xfudge for Monitor = 1 */
	8,	/* xfudge for Monitor = 2 */
	40	/* xfudge for Monitor = 3 */
};
static uchar	y_fudge[4] = {
	3, 	/* yfudge for Monitor = 0 */
	3,	/* yfudge for Monitor = 1 */
	5,	/* yfudge for Monitor = 2 */
	1	/* yfudge for Monitor = 3 */
};

static void draw_line(int x1,int y1,int x2,int y2) {
	int     min_v;
	int     max_v;
	unsigned short draw_cmd;

	max_v = MAX(abs(x2-x1),abs(y2-y1));
	min_v = MIN(abs(x2-x1),abs(y2-y1));
	draw_cmd = 0x2013;
	if ( y1 <= y2 )
		draw_cmd |= 0x0080;

	if ( abs(x2-x1) < abs(y2-y1) )
		draw_cmd |= 0x0040;

	HGAWaitForFiFo(ThreeSlots);

	S3WriteRegisterSwappedShort (S3ForegroundMix, 0x0027);
	S3WriteRegisterSwappedShort (S3ForegroundColor,draw_color);
	S3WriteRegisterSwappedShort (S3MultiFuncControl,
	    (MF_PIX_CNTL | 0x0000));

	HGAWaitForFiFo(SevenSlots);

	S3WriteRegisterSwappedShort (S3CurrentX, x1);
	S3WriteRegisterSwappedShort (S3CurrentY,y1);
	S3WriteRegisterSwappedShort (S3MajorAxisPixelCount, max_v);
	S3WriteRegisterSwappedShort (S3DestinationX,
	    (2 * (min_v - max_v)) & 0x3fff);
	S3WriteRegisterSwappedShort (S3DestinationY, (2 * min_v) & 0x3fff);

	if (x1<x2) {
		S3WriteRegisterSwappedShort (S3ErrorTerm, 
		    (2 * min_v - max_v) & 0x3fff);
		draw_cmd |= 0x0020;
	} else 
		S3WriteRegisterSwappedShort (S3ErrorTerm, 
		    (2 * min_v - max_v - 1) & 0x3fff);

	S3WriteRegisterSwappedShort (S3DrawingCommand, draw_cmd);

	return;
}
/*----------------------------------------------------------------------------*/
/* lines_test()  This routine displays a test pattern on the display        */
/*----------------------------------------------------------------------------*/
lines_test(void)
{
	int             rc = 0;
	int             i;
	int             j;
	int             xstart,xincr;
	int             ystart,yincr;
	int             max_res;

	max_res = NUM_COLORS;

	s3_clear_screen(CLR_BLACK);
	for (i=0;(i<=max_res) && (rc == 0);i++) {

		draw_color = line_color[i];

		xincr = xstart = screen.width  / 4;
		yincr = ystart = screen.height / 4;

		draw_line(2, 2, screen.width-3, 2);
		draw_line(screen.width-3, 2, screen.width-3, screen.height-3);
		draw_line(screen.width-3, screen.height-3, 2, screen.height-3);
		draw_line(2, screen.height-3, 2, 2);
		draw_line(3, 3, screen.width-4, 3);
		draw_line(screen.width-4, 3, screen.width-4, screen.height-4);
		draw_line(screen.width-4, screen.height-4, 3, screen.height-4);
		draw_line(3, screen.height-4, 3, 3);

		for (j=0;j<4;j++) {
			draw_line(4, ystart, screen.width-5, ystart);
			draw_line(4, ystart+1, screen.width-5, ystart+1);
			draw_line(xstart, 4, xstart,screen.height-5);
			draw_line(xstart+1, 4, xstart+1, screen.height-5);
			xstart += xincr;
			ystart += yincr;
		} /* endfor */

		sleep(1);
	}

	s3_clear_screen(CLR_BLACK);
	return rc;
}


SetScissorSize()
{

	S3WriteRegister (S3MultiFuncControl, (short)(0x1));
	S3WriteRegisterSwappedShort (S3MultiFuncControl, (short)0x1000);
	S3WriteRegister (S3MultiFuncControl, (short)(0x2));
	S3WriteRegisterSwappedShort (S3MultiFuncControl, (short)0x1000);
	S3WriteRegister (S3MultiFuncControl, (short)(0x3));
	S3WriteRegisterSwappedShort (S3MultiFuncControl, (short)0x32FF);
	S3WriteRegister (S3MultiFuncControl, (short)(0x4));
	S3WriteRegisterSwappedShort (S3MultiFuncControl, (short)0x43FF);

	return 0;
}

/*---------------------------------------------------------------------------*/
/*      s3_putchar()                                                         */
/*                                                                           */
/*      BitBlt a a character from a particular font onto the display         */
/*---------------------------------------------------------------------------*/
s3_putchar(int    char_code,    /* The character to place on the screen */
int    color_f,      /* in this color                        */
int    color_b,      /* on this background                   */
int    x,            /* at this x position (0 = left)        */
int    y,            /* at this y position (0 = top)         */
FONT_TABLE * f)	     /* The font table                       */
{

	int             i;
	unsigned char  *t;
	int             font_w;
	int             font_h;
	int             cur_row;
	int             cur_column;
	int             color_f_use;
	int             color_b_use;
	unsigned char   u;

	if (odf_fix == 1)
		odf_fix = 2;
	else if(odf_fix == 2)
		odf_fix = 3;
	else if(odf_fix == 3) {
		odf_fix = 0;
		S3WriteRegister(S3CRTControlIndex,0x45);
		S3ReadRegister(S3CRTControlData, u);
		u &= ~0x20;
		S3WriteRegister(S3CRTControlData, u);
	}

	font_w = f->width;
	font_h = f->height;

	HGAWaitForFiFo(FiveSlots);
	S3WriteRegisterSwappedShort (S3ForegroundMix, 0x0027);
	if (color_f == -1)
		color_f_use = f_color;
	else
		color_f_use = color_f;
	if (color_b == -1)
		color_b_use = b_color;
	else
		color_b_use = color_b;
	S3WriteRegisterSwappedShort (S3ForegroundColor, 
	    color_xform(color_f_use));
	S3WriteRegisterSwappedShort (S3BackgroundMix, 0x0007);
	S3WriteRegisterSwappedShort (S3BackgroundColor,
	    color_xform(color_b_use));
	S3WriteRegister (S3MultiFuncControl,(MF_PIX_CNTL | 0x0080));

	HGAWaitForFiFo(FiveSlots);
	S3WriteRegisterSwappedShort (S3CurrentX, x);
	S3WriteRegisterSwappedShort (S3CurrentY, y);
	S3WriteRegisterSwappedShort (S3MajorAxisPixelCount,font_w-1);
	S3WriteRegisterSwappedShort (S3MultiFuncControl, 
	    (MF_MIN_AXIS_PCNT | (font_h-1)));

	HGAProcessorIdle();

	/* Write a Character using byte mode.    */
	S3WriteRegisterSwappedShort (S3DrawingCommand,0x51b3);

	/* For larger fonts, this could be       */
	/* optimized to word (16 bits) mode      */

	t = f->font+char_code*font_h;
	font_w = 16;
	font_h = 16;

	i = ((font_w+7)/8) * font_h;
	while (i--)
		S3WriteRegisterSwappedShort (S3PixelDataTransfer,
		    (unsigned short) *t++);

	return;
}

/*---------------------------------------------------------------------------*/
/*      color_xform()                                                        */
/*                                                                           */
/*      Convert from 256 color index into an RGB for 16, 24, or 32 bit color */
/*---------------------------------------------------------------------------*/
static color_xform(int index)
{
	char   red,blue,green;
	char   *s;
	int    color_use;
	extern char user_palette[];

	if (pixelDepth == 8)
		color_use = index;
	else {
		s = user_palette + (3 * index);
		red   = *s++;
		green = *s++;
		blue  = *s;

		if (pixelDepth == 16) {
			color_use = ((red   << 8) & 0xf800) |
			((green << 3) & 0x07e0) |
			(blue  >> 3);
		} else {
			/* 24 or 32 bit color   */
			color_use = (red   << 16) | (green <<  8) | blue;
		} /* endif */
	} /* endif */
	return color_use;
}

/*---------------------------------------------------------------------------*/
/* s3_GMTest()  Test the different graphics modes                            */
/*---------------------------------------------------------------------------*/
int s3_GMTest() 
{
	int rows;
	int cols;
	int rc = 0;
	int i;
	int j;
	int max_res;
	extern SCR_INFO screen;

	rows = screen.height / current_font->height;
	cols = screen.width  / current_font->width;

	s3_clear_screen(CLR_BLACK);
	bt485_goto_xy(15,7);

	for (j=1;j<=cols;j++) {
		int x,n;

		x = j;
		n = 5;
		do {
			if ((j<9) || (j>12))
				s3_putchar('a' + x % 10,
				    CLR_YELLOW,
				    CLR_WHITE,
				    (j-1) * current_font->width,
				    n-- * current_font->height,
				    current_font);
			x /= 10;
		} while ((x != 0) && ((j % 10) == 0));
	} /* endfor */

	for (j=1;j<=rows;j++) {
		int x,n;

		x = j;
		n = 10;
		do {
			if ((j<4) || (j>7))
				s3_putchar('a' + x % 10,
				    CLR_YELLOW,
				    CLR_WHITE,
				    n-- * current_font->width,   
				    (j-1)  * current_font->height,  
				    current_font);
			x /= 10;
		} while ((x != 0) && ((j % 10) == 0));
	} /* endfor */

	sleep(2);

	s3_clear_screen(CLR_BLACK);

	return rc;
}

/*---------------------------------------------------------------------------*/
/*      bt485_xy()                                                           */
/*                                                                           */
/*      Move the cursor to the appropriate location on the display           */
/*---------------------------------------------------------------------------*/
static void bt485_goto_xy(int x,int y)
{
	int     u;
	int     xfudge,yfudge;
	extern int monitor;

	S3WriteRegister (S3CRTControlIndex, 0x45);
	S3ReadRegister  (S3CRTControlData, u);
	u |= 0x20;
	S3WriteRegister (S3CRTControlData, u);

	/* Access cursor RAM array Data         */
	S3WriteRegister (S3CRTControlIndex, 0x55);
	S3ReadRegister  (S3CRTControlData, u);
	u |= 0x03;
	S3WriteRegister (S3CRTControlData, u);

	/* The S3/PCI does not present CDE to the DAC.  This code computes a
	 fudge that should correct this defect. */
	if (pixelDepth == 8) {
		xfudge = x_fudge[monitor];
		yfudge = y_fudge[monitor];
	} else {
		xfudge = 0;
		yfudge = 0;
	} /* endif */

	/* Using 32x32 cursor */
	x += 32 + xfudge; 
	y += 32 + yfudge;

	S3WriteRegister (DAC_X_LOW,   (x & 0xff) );
	S3WriteRegister (DAC_X_HIGH, ((x >> 8) & 0xff) );
	S3WriteRegister (DAC_Y_LOW,  (y & 0xff) );
	S3WriteRegister (DAC_Y_HIGH, ((y >> 8) & 0xff) );

	/* Restore access to Palette data       */
	S3WriteRegister (S3CRTControlIndex, 0x55); 
        S3ReadRegister  (S3CRTControlData, u);
	u &= ~0x03;
	S3WriteRegister (S3CRTControlData, u);

	odf_fix = 1;
}

/*---------------------------------------------------------------------------*/
/*      s3_clear_screen()                                                    */
/*                                                                           */
/*      BitBlt the entire display to white.                                  */
/*---------------------------------------------------------------------------*/
s3_clear_screen(int col) 
{


	HGAWaitForFiFo(SixSlots);

	S3WriteRegisterSwappedShort (S3MultiFuncControl, (MF_SCISSORS_T | 0));
	S3WriteRegisterSwappedShort (S3MultiFuncControl, (MF_SCISSORS_L | 0));
	S3WriteRegisterSwappedShort (S3MultiFuncControl, 
				(MF_SCISSORS_B | screen.height-1));
	S3WriteRegisterSwappedShort (S3MultiFuncControl, 
				(MF_SCISSORS_R | screen.width-1));

	S3WriteRegisterSwappedShort (S3ShortStrokeVectorTransfer, 0x0000);
	S3WriteRegisterSwappedShort (S3MultiFuncControl, MF_MULT_MISC|0x0000);

	HGAWaitForFiFo(ThreeSlots);
	S3WriteRegisterSwappedShort (S3WriteMask, 0xffff);
	S3WriteRegisterSwappedShort (S3ReadMask, 0x0001);
	S3WriteRegisterSwappedShort (S3ColorCompare, 0x0000);

  	s3_rect_fill(0, 0, screen.width-1, screen.height-1, col);

  return;
}

/*---------------------------------------------------------------------------*/
/*      s3_rect_fill()                                                       */
/*                                                                           */
/*      BitBlt a rectangle to the desired color                              */
/*---------------------------------------------------------------------------*/
s3_rect_fill(int ul_x,
                  int ul_y,
                  int lr_x,
                  int lr_y,
                  int color) 
{


	HGAWaitForFiFo(ThreeSlots);
	S3WriteRegisterSwappedShort(S3ForegroundMix, 0x0027);
	S3WriteRegisterSwappedShort (S3ForegroundColor, color_xform(color));
	S3WriteRegisterSwappedShort (S3MultiFuncControl, 
							(MF_PIX_CNTL | 0x0000));
	

	HGAWaitForFiFo(SixSlots);
	HGAFillBlockSolid (ul_x, ul_y, (lr_x - ul_x), (lr_y - ul_y));

	HGAProcessorIdle();

  	return;
}

draw_rec()
{
	int x1, x2, y1, y2, col;
	int rows, cols;
	int	incr;

	int i;

	s3_clear_screen(CLR_BLACK);
	rows = screen.height / current_font->height;
        cols = screen.width  / current_font->width;

	x1 = 0; y1 = 0; /*x2 = 1024; y2=760; */
	x2 = 640; y2=480; 
	
	incr =  screen.width / 8;

	for(i=0; i<8; i++) {
		s3_rect_fill(x1, y1, x1+incr, y2, line_color[i]);
		x1 = x1+incr;
		sleep(1);
	}

	sleep(3);
	s3_clear_screen(CLR_BLACK);
	return(0);
}

draw_H()
{
	/* Draw character 'H' as a polygon */
	draw_line(2,8,6,8);
	draw_line(8,8,12,8);
	s3_rect_fill(3,15,11,16, CLR_WHITE);
	draw_line(2,24,6,24);
	draw_line(8,24,12,24);
	s3_rect_fill(3,8,5,24, CLR_WHITE);
	s3_rect_fill(9,8, 11,24, CLR_WHITE);

	return 0;
}


/* Constants used in this file */
#define SMALL_SCRN_SCR_CNT      0x200 /* 1024x768 screen scroll cycle count*/
#define BIG_SCRN_SCR_CNT        0x100 /* 1280x1024 screen scroll cycle count*/
#define SCR_H_FGC_INDEX         0xff  /* scroll H foreground color index   */
#define SCR_H_BGC_INDEX         0x00  /* scroll H background color index   */
#define SCR_AMT                 0x1   /* vertical pixels to scroll each cycle */
#define CHAR_BOX_WIDTH          0x10  /* scroll character box width        */
#define CHAR_BOX_HEIGHT         0x20  /* scroll character box height       */
#define SMALL_SCRN_WIDTH        0x400 /* 1024x768 screen width             */
#define SMALL_SCRN_HEIGHT       0x300 /* 1024x768 screen height            */
#define BIG_SCRN_WIDTH          0x500 /* 1280x1024 screen width            */
#define BIG_SCRN_HEIGHT         0x400 /* 1280x1024 screen height           */

scr_h()
{
	int scrn_w, scrn_h, scr_cnt, chr_w, chr_h;
	int scr_amt = SCR_AMT;
	int j, rc;
		
	scrn_w = screen.width;
	scrn_h = screen.height;
	scr_cnt = SMALL_SCRN_SCR_CNT;
	chr_w = CHAR_BOX_WIDTH;
	chr_h = CHAR_BOX_HEIGHT;

	s3_clear_screen(CLR_BLACK);	
	draw_color = CLR_WHITE;
	draw_H();
	full_scrn_chr(chr_w, chr_h, scrn_w, scrn_h);

	/*if ( (strcmp(htx_s.run_type, "EMC") == 0) ||
		(strcmp(htx_s.run_type, "emc") == 0) ) {
		  while (1) {
			rc = scrl_scrn( scr_amt, chr_h, scrn_w, scrn_h );
			if ( rc )
				return rc;
		  }
	} else {
		for(j=0; j<= scr_cnt; j++) {
			rc = scrl_scrn( scr_amt, chr_h, scrn_w, scrn_h );
			if ( rc ) 
				return rc;
		}
	}
	*/
	s3_clear_screen(CLR_BLACK);	
	return 0;
}

/*
 *  NAME: full_scrn_chr
 *
 *  FUNCTION:      This function copies a character box at 0,0 to fill the
 *                 full screen.
 *
 *  ALGORITHM:     This function performs screen to screen blts in frame
 *                 buffer A to copy a character box at 0,0 with dimensions
 *                 chr_w by chr_h to fill a screen with dimensions scrn_w by
 *                 scrn_h.
 *
 * EXECUTION ENVIRONMENT: Process only
 *
 * NOTES: None
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: This function returns zero if successful, else returns error code.
 */
/*
 * INPUT:          chr_w        character box width
 *
 *                 chr_h        character box height
 *
 *                 scrn_w       width of the visible screen in pixels
 *
 *                 scrn_h       height of the visible screen in pixels
 *
 * OUTPUT: Refer to RETURNS above.
 */

int full_scrn_chr(int chr_w, int chr_h, int scrn_w, int scrn_h)
{
  int j;                                  /* loop counter */
  int rc = 0;
  int chrs_line;                          /* characters per line */
  int lines_scrn;                         /* lines per scrn height */
  int sx,sy,dx,dy,w,h;

  chrs_line = scrn_w / chr_w;

   sx = 0;
   sy = 0;
   dy = 0;
   h = chr_h;
   w = chr_w;
   dx = chr_w;
   rc = blt_sc_sc(sx, sy, dx, dy, w, h);    /* screen to screen copy direct */
   if (rc) {
     return(rc);
   } /* endif */

   w = 2*chr_w;
   dx = 2*chr_w;
   rc = blt_sc_sc(sx, sy, dx, dy, w, h);    /* screen to screen copy direct */
   if (rc) {
     return(rc);
   } /* endif */

   w = 4*chr_w;
   dx = 4*chr_w;
   rc = blt_sc_sc(sx, sy, dx, dy, w, h);    /* screen to screen copy direct */
   if (rc) {
     return(rc);
   } /* endif */

   w = 8 * chr_w;
   for (j = 2 ; j <= (chrs_line / 8) ; j++) {
     dx = (j - 1) * w;
     rc = blt_sc_sc(sx, sy, dx, dy, w, h);
     if (rc) {
       return(rc);
     } /* endif */
   } /* endfor */

   w = chr_w;
   for (j = (j*8) ; j <= chrs_line ; j++) {
     dx = (j - 1) * chr_w;
     rc = blt_sc_sc(sx, sy, dx, dy, w, h);
     if (rc) {
       return(rc);
     } /* endif */
   } /* endfor */

   lines_scrn = scrn_h / chr_h;

   sx = 0;
   sy = 0;
   dx = 0;
   h = chr_h;
   w = scrn_w;

   dy = chr_h;
   rc = blt_sc_sc( sx, sy, dx, dy, w, h );
   if (rc) {
     return(rc);
   } /* endif */

   h  = 2*chr_h;
   dy = 2*chr_h;
   rc = blt_sc_sc( sx, sy, dx, dy, w, h );
   if (rc) {
     return(rc);
   } /* endif */

   h  = 4 * chr_h;
   dy = 4 * chr_h;
   rc = blt_sc_sc( sx, sy, dx, dy, w, h );
   if (rc) {
     return(rc);
   } /* endif */

   h = 8 * chr_h;
   j = 0;
   for (j = 2 ; j <= (lines_scrn / 8) ; j++) {
     dy = (j - 1) * h;
     rc = blt_sc_sc( sx, sy, dx, dy, w, h );
     if (rc) {
       return(rc);
     } /* endif */
   } /* endfor */

   h  = chr_h;
   for (j = j*8 ; j <= lines_scrn ; j++) {
     dy = ( j-1 ) * chr_h;
     rc = blt_sc_sc( sx, sy, dx, dy, w, h );
     if (rc) {
       return(rc);
     } /* endif */
   } /* end for */

   return(0);

} /* end full_scrn_chr() */

blt_sc_sc(int sx, int sy, int dx, int dy, int w, int h)
{

	unsigned short draw_cmd;

	draw_cmd = 0xc0b3;
#if 0
	if ( sx <= dx ) 
		draw_cmd |= 0x0020;
	if ( sy <= dy )
		draw_cmd |= 0x0080;
#endif
	HGAWaitForFiFo(TwoSlots);
	S3WriteRegisterSwappedShort (S3MultiFuncControl, 
						(MF_PIX_CNTL | 0x00C0));
	S3WriteRegisterSwappedShort (S3ForegroundMix, 0x0067);


	/* Draw operation */
	HGAWaitForFiFo(SevenSlots);

	S3WriteRegisterSwappedShort (S3CurrentX, sx);
	S3WriteRegisterSwappedShort (S3CurrentY,sy);
	S3WriteRegisterSwappedShort (S3DestinationX,dx);
	S3WriteRegisterSwappedShort (S3DestinationY,dy);
	S3WriteRegisterSwappedShort (S3MajorAxisPixelCount, w-1);
	S3WriteRegisterSwappedShort (S3MinorAxisPixelCount, h-1);

	S3WriteRegisterSwappedShort (S3DrawingCommand, draw_cmd);

	HGAProcessorIdle();
	return 0;
}

/*
 *  NAME: scrl_scrn
 *
 *  FUNCTION:      This function scrolls a buffer A screen vertically.
 *
 *
 * ALGORITHM:      This function performs 2 screen to screen blts to scroll
 *                 down by scr_amt vertical pixels a
 *                 frame buffer A screen of dimensions scrn_w by scrn_h
 *                 full of character lines of height chr_h.
 *                 This function assumes that the data repeats every
 *                 chr_h pixels vertically.
 *
 * EXECUTION ENVIRONMENT: Process only
 *
 * NOTES: None
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: This function returns zero if successful, else returns error code.
 */
/*
 * INPUT:          scr_amt      number of pixel lines to scroll vertically on
 *                              each cycle.
 *
 *                 chr_h        character box height
 *
 *                 scrn_w       width of the visible screen in pixels
 *
 *                 scrn_h       height of the visible screen in pixels
 *
 * OUTPUT: Refer to RETURNS above.
 */

int scrl_scrn(int scr_amt, int chr_h, int scrn_w, int scrn_h)
{
  int rc;
  int sx,sy,dx,dy,w,h;

   /* blt full scrn minus scr_amt to top of screen */

   sx = 0;
   sy = scr_amt;
   dx = 0;
   dy = 0;
   h = scrn_h - scr_amt;
   w = scrn_w;

   rc = blt_sc_sc( sx, sy, dx, dy, w, h );
   if (rc) {
     return(rc);
   } /* endif */

   /* blt scrn_w x scr_amt from identical data to bottom of screen */

   sx = 0;
   sy = chr_h-scr_amt;
   dx = 0;
   dy = scrn_h-scr_amt;
   h = scr_amt;
   w = scrn_w;

   rc = blt_sc_sc( sx, sy, dx, dy, w, h );
   if (rc) {
     return(rc);
   } /* endif */

   return(0);

} /* end scrl_scrn() */

