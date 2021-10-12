static char sccsid[] = "@(#)65  1.3  src/bos/diag/tu/wga/colorbartu.c, tu_wga, bos411, 9428A410j 1/3/94 17:05:12";
/*
 *   COMPONENT_NAME: TU_WGA
 *
 *   FUNCTIONS: color_init
 *              colorbar_tu
 *              draw_color_bars
 *              draw_grey_scale
 *              draw_piano_bars
 *              draw_single_pixels
 *              draw_white_boxes
 *              restore_default_colors
 *              save_default_colors
 *              get_color_pattern
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


#include <sys/types.h>
#include <stdio.h>
#include <math.h>

#include "exectu.h"
#include "tu_type.h"
#include "wgamisc.h"
#include "wga_reg.h"
#include "wga_regval.h"


#define NUM_BARS          9                           /* 9 color bars on scr */

/* This measures (inches) were taken from reference screen (approx.) */
#define REF_SCREEN_WIDTH  9.5
#define REF_SCREEN_HEIGHT (9.0 + 2.5/16.0)
#define BAR_WIDTH         (REF_SCREEN_WIDTH / (float) NUM_BARS)
#define BAR_YSTART        (2.0 + 5.5/16.0)
#define BAR_YEND          (7.0 + 0.5/16.0)
#define DOTS_XSTART       (8.0 + 11.0 / 16.0)
#define DOTS_YSTART       (8.0 + 7.0 / 16.0)

#define GRAY_TONES        NUM_BARS - 1                /*except last color bar*/
#define GRAY_DELTA        255 / GRAY_TONES
#define BW_BAR_GROUPS     5
#define BW_BARS           12
#define NUMBER_DOTS       3


static struct RGB
{
  uchar_t red;
  uchar_t green;
  uchar_t blue;
};

#ifndef SET_COLORS
static struct RGB default_colors [PAL_LINES];
#else
static struct RGB default_colors [PAL_LINES] =   /* default color for palette*/
                              { { 0x00, 0x00, 0x00 },
                                { 0xff, 0x00, 0x00 },
                                { 0x00, 0xff, 0x00 },
                                { 0xf0, 0xf0, 0x00 },
                                { 0x00, 0x00, 0xf0 },
                                { 0xb0, 0x00, 0xb0 },
                                { 0x00, 0xf0, 0xf0 },
                                { 0xc8, 0xc8, 0xc8 },
                                { 0x78, 0x78, 0x78 },
                                { 0xf0, 0x80, 0x20 },
                                { 0x50, 0xe0, 0x30 },
                                { 0xc0, 0x80, 0x20 },
                                { 0x30, 0x60, 0xff },
                                { 0xe0, 0x30, 0xe0 },
                                { 0x70, 0xf0, 0xd0 },
                                { 0xff, 0xff, 0xff },
                                { 0xf9, 0x2f, 0x5d },
                                { 0xdf, 0x7e, 0x0a },
                                { 0xfe, 0x47, 0xd6 },
                                { 0xf5, 0xbe, 0x9f },
                                { 0xbc, 0xcf, 0x07 },
                                { 0xe3, 0x6f, 0x76 },
                                { 0xde, 0xeb, 0xef },
                                { 0xdf, 0xed, 0x77 },
                                { 0x5e, 0x77, 0xfa },
                                { 0xf7, 0xaa, 0xe2 },
                                { 0x7b, 0x4a, 0x31 },
                                { 0x9d, 0xbd, 0x9f },
                                { 0x78, 0xd5, 0xdb },
                                { 0xfe, 0xfd, 0xf9 },
                                { 0x6f, 0xcf, 0xdc },
                                { 0x35, 0xf0, 0xea },
                                { 0x5c, 0xaf, 0xfb },
                                { 0x6d, 0xd3, 0x8f },
                                { 0xff, 0x3f, 0x14 },
                                { 0xff, 0x65, 0x6c },
                                { 0xd6, 0xbd, 0xfd },
                                { 0xff, 0xaf, 0x17 },
                                { 0xbd, 0x3f, 0xdf },
                                { 0x77, 0xee, 0x73 },
                                { 0xbe, 0x1f, 0xf5 },
                                { 0xef, 0x09, 0x56 },
                                { 0xdb, 0xcb, 0xdf },
                                { 0x7a, 0xa6, 0xbb },
                                { 0x9f, 0x5a, 0x9f },
                                { 0x7f, 0xbf, 0xf4 },
                                { 0xec, 0xf2, 0x7f },
                                { 0xe6, 0x13, 0xfb },
                                { 0xff, 0x77, 0x50 },
                                { 0xb8, 0x67, 0xf6 },
                                { 0xea, 0xc4, 0xfb },
                                { 0xfd, 0xa7, 0xce },
                                { 0xfd, 0xe7, 0xfb },
                                { 0xc8, 0xbd, 0x1b },
                                { 0xb8, 0x7b, 0xfd },
                                { 0xfb, 0x5e, 0x99 },
                                { 0xfd, 0x76, 0xfd },
                                { 0xaf, 0xb1, 0x6f },
                                { 0x1d, 0x57, 0x5f },
                                { 0xb2, 0x47, 0x77 },
                                { 0xde, 0x39, 0xe9 },
                                { 0xdf, 0xd7, 0x1b },
                                { 0x3f, 0x4b, 0x7f },
                                { 0x79, 0x16, 0xe3 },
                                { 0x97, 0xbb, 0xfd },
                                { 0xd7, 0xbb, 0xf6 },
                                { 0xef, 0x5e, 0x4c },
                                { 0x0f, 0xe2, 0x7e },
                                { 0x7d, 0xdf, 0xf3 },
                                { 0xfe, 0xff, 0xb7 },
                                { 0xff, 0x79, 0x2f },
                                { 0xb5, 0x42, 0xec },
                                { 0xa5, 0x79, 0xf9 },
                                { 0xdc, 0x58, 0x7b },
                                { 0x7b, 0xc0, 0xfd },
                                { 0x35, 0xff, 0x2b },
                                { 0x24, 0x8f, 0x62 },
                                { 0x5c, 0x7a, 0x7f },
                                { 0x7f, 0xf3, 0xee },
                                { 0xbb, 0x36, 0x47 },
                                { 0xcb, 0x6a, 0xdf },
                                { 0xff, 0xf7, 0x9b },
                                { 0x33, 0xeb, 0xfb },
                                { 0x5f, 0xfd, 0x73 },
                                { 0xef, 0xe7, 0x6b },
                                { 0xf5, 0x6a, 0x2f },
                                { 0xe9, 0x6e, 0xbf },
                                { 0xff, 0xff, 0x8e },
                                { 0x47, 0xa2, 0x2b },
                                { 0xd5, 0xff, 0xab },
                                { 0x5b, 0x7e, 0xd8 },
                                { 0x3e, 0xf3, 0x87 },
                                { 0x7c, 0x27, 0xff },
                                { 0x8b, 0xec, 0xd9 },
                                { 0xf2, 0x8b, 0xd4 },
                                { 0xb4, 0xf7, 0xb7 },
                                { 0xe7, 0x95, 0xbb },
                                { 0xf7, 0x56, 0x6b },
                                { 0xdf, 0xcb, 0xf8 },
                                { 0xbf, 0xf4, 0xd6 },
                                { 0xfb, 0xde, 0xc5 },
                                { 0xb7, 0xe7, 0xef },
                                { 0x39, 0xff, 0xf4 },
                                { 0x93, 0x7a, 0xfa },
                                { 0x7d, 0xd8, 0xd3 },
                                { 0x72, 0xa4, 0x5f },
                                { 0xdb, 0xde, 0xbe },
                                { 0xe6, 0xb7, 0xb0 },
                                { 0xef, 0xef, 0x1a },
                                { 0x71, 0x8a, 0xcb },
                                { 0xbd, 0x34, 0x13 },
                                { 0x3b, 0x87, 0xfe },
                                { 0xfe, 0xfa, 0xbe },
                                { 0x37, 0xbe, 0x92 },
                                { 0xfb, 0xd1, 0xc3 },
                                { 0x37, 0x77, 0x84 },
                                { 0xb7, 0x4d, 0xd7 },
                                { 0xff, 0xbf, 0x1f },
                                { 0x5a, 0xf1, 0xd2 },
                                { 0x69, 0xd7, 0x79 },
                                { 0xb4, 0xfe, 0xca },
                                { 0xed, 0xee, 0xba },
                                { 0xac, 0x7e, 0x32 },
                                { 0xfb, 0x9f, 0xb7 },
                                { 0xaf, 0xe7, 0x5f },
                                { 0x6d, 0xb7, 0xa4 },
                                { 0x3f, 0xeb, 0xbd },
                                { 0xd9, 0xff, 0x3a },
                                { 0xf5, 0x79, 0xed },
                                { 0xf7, 0x6d, 0xfb },
                                { 0xf7, 0xe9, 0xbd },
                                { 0xdd, 0xbf, 0xc5 },
                                { 0xf5, 0xfb, 0x9d },
                                { 0xfe, 0x7b, 0xf1 },
                                { 0x96, 0x3f, 0xf5 },
                                { 0x4f, 0x5a, 0xaa },
                                { 0x7f, 0xfe, 0xfa },
                                { 0xf5, 0x35, 0xde },
                                { 0xb3, 0xc2, 0xb5 },
                                { 0x3d, 0x7e, 0x7b },
                                { 0x6e, 0xef, 0xf3 },
                                { 0x3f, 0xb7, 0x43 },
                                { 0x3f, 0x33, 0xdf },
                                { 0x1b, 0xfd, 0x94 },
                                { 0xfe, 0xcb, 0xfd },
                                { 0xd6, 0x3a, 0xff },
                                { 0xac, 0xbc, 0x72 },
                                { 0x53, 0x29, 0x02 },
                                { 0x65, 0xf1, 0xcc },
                                { 0x39, 0xdf, 0xed },
                                { 0x0b, 0xbb, 0x97 },
                                { 0xfd, 0xff, 0x7e },
                                { 0xe8, 0x5a, 0x57 },
                                { 0x77, 0x76, 0xe7 },
                                { 0xfa, 0x6f, 0x2d },
                                { 0xc1, 0xfd, 0xbb },
                                { 0xf6, 0xff, 0xd3 },
                                { 0xd4, 0xf6, 0xff },
                                { 0xbe, 0xd2, 0x7b },
                                { 0xee, 0xf3, 0xef },
                                { 0xbf, 0xff, 0xbc },
                                { 0x66, 0x7d, 0x2b },
                                { 0x6b, 0xed, 0xba },
                                { 0xb9, 0xab, 0xc7 },
                                { 0xf7, 0xb5, 0x7f },
                                { 0xa5, 0xa7, 0xba },
                                { 0x71, 0x56, 0xb7 },
                                { 0x16, 0xd6, 0xf7 },
                                { 0x5b, 0x47, 0x8e },
                                { 0xed, 0x1c, 0xb0 },
                                { 0xff, 0x3e, 0xfa },
                                { 0x9b, 0x25, 0xd3 },
                                { 0x17, 0xef, 0x57 },
                                { 0x56, 0xf6, 0x7a },
                                { 0xd5, 0x4f, 0xfa },
                                { 0x4f, 0xdf, 0xff },
                                { 0xf7, 0x35, 0xb3 },
                                { 0x2f, 0x4b, 0xcb },
                                { 0xa9, 0x59, 0xdd },
                                { 0x42, 0xdb, 0x3f },
                                { 0xab, 0x2c, 0xfe },
                                { 0x56, 0xd4, 0x7f },
                                { 0xbc, 0xf8, 0xef },
                                { 0x78, 0x86, 0xea },
                                { 0x9f, 0xcd, 0xf1 },
                                { 0x7e, 0xb1, 0x6f },
                                { 0x7e, 0x35, 0x5b },
                                { 0x5b, 0xfc, 0xbf },
                                { 0xeb, 0xec, 0x7d },
                                { 0xd6, 0xfa, 0xfb },
                                { 0xfb, 0xf1, 0xfe },
                                { 0x9f, 0xe7, 0xf8 },
                                { 0xe2, 0xdf, 0xff },
                                { 0xde, 0xed, 0xde },
                                { 0x29, 0xd5, 0x9a },
                                { 0x87, 0xf5, 0x77 },
                                { 0x95, 0xd4, 0xbf },
                                { 0xe6, 0xff, 0xc3 },
                                { 0xe9, 0xe4, 0x5d },
                                { 0xef, 0x65, 0x6a },
                                { 0x7a, 0xed, 0x09 },
                                { 0xfb, 0x71, 0x7e },
                                { 0x4d, 0xc5, 0xb4 },
                                { 0xe2, 0x9b, 0xb7 },
                                { 0xbc, 0x7d, 0x2a },
                                { 0xec, 0x58, 0xee },
                                { 0xc7, 0x08, 0x9b },
                                { 0x9a, 0x6f, 0x5d },
                                { 0xbb, 0xbf, 0xfc },
                                { 0x9d, 0xce, 0xef },
                                { 0xbb, 0xda, 0x7f },
                                { 0x85, 0x67, 0xf3 },
                                { 0xfd, 0xfb, 0x15 },
                                { 0x1c, 0x3f, 0xf8 },
                                { 0x7d, 0x4f, 0xee },
                                { 0xee, 0xfd, 0xbf },
                                { 0x67, 0xfe, 0x4d },
                                { 0x7c, 0x9e, 0x37 },
                                { 0xdb, 0xb5, 0xbf },
                                { 0xfe, 0x7e, 0x76 },
                                { 0x5f, 0xd6, 0x57 },
                                { 0xbb, 0x17, 0x5c },
                                { 0xc9, 0xcf, 0xf7 },
                                { 0x5e, 0xcf, 0x7f },
                                { 0xfd, 0x9f, 0xff },
                                { 0xef, 0x7a, 0xde },
                                { 0xfb, 0xbe, 0xd7 },
                                { 0x51, 0x64, 0xdf },
                                { 0xf3, 0xdd, 0x72 },
                                { 0xee, 0xad, 0xbe },
                                { 0xbc, 0x79, 0xde },
                                { 0x7c, 0xbf, 0xd6 },
                                { 0x57, 0x3b, 0xd9 },
                                { 0xf3, 0x9b, 0xf4 },
                                { 0xea, 0xfb, 0x5b },
                                { 0x8b, 0x17, 0xfa },
                                { 0xa4, 0xce, 0xcf },
                                { 0x8a, 0x45, 0xfd },
                                { 0xdd, 0x4d, 0xd7 },
                                { 0xfc, 0xa7, 0xd3 },
                                { 0xff, 0x67, 0x97 },
                                { 0xff, 0x9a, 0xac },
                                { 0xc7, 0x72, 0x7f },
                                { 0xfe, 0x90, 0xf7 },
                                { 0x86, 0xff, 0xd7 },
                                { 0xcb, 0xbf, 0x3f },
                                { 0xe6, 0x97, 0xe9 },
                                { 0xf3, 0x4d, 0xbd },
                                { 0xb3, 0xbb, 0x9f },
                                { 0x7e, 0xf7, 0x45 },
                                { 0xbf, 0xfb, 0xdd },
                                { 0x74, 0xa0, 0xa3 },
                                { 0xd3, 0xb7, 0x9f },
                                { 0xfe, 0x9b, 0x77 },
                                { 0x9e, 0x36, 0x49 },
                                { 0xb6, 0xb6, 0x81 } };
#endif



static void save_default_colors (void);
static int  draw_white_boxes(float, float);
static int  draw_color_bars(float, float);
static int  draw_grey_scale(float, float);
static int  draw_piano_bars(float, float);
static int  draw_single_pixels(uchar_t, float, float);



/*
 * NAME : colorbar_tu
 *
 * DESCRIPTION :
 *
 * Creates a pattern that is a combination of four component
 * patterns for the evaluation of the performance of the WGA and
 * monitor. The top section consists of low frequency components of
 * black and full amplitude white. This is used to evaluate the low
 * frequency performance of the system for video smear or low frequency
 * tilt (shading).
 * The mid section consists of eight vertical bars of the primary colors
 * of red, green and blue plus the complimentary colors of yellow, magenta
 * and cyan followed by black and white.  The complimentary colors are a mix
 * of two of the primaries such as green and red to produce yellow.
 * This section of the pattern provides the ability to evaluate the system
 * and monitors color fidelity and transient response from color to color
 * and color hue (phase delay and phase shift).
 * Below the color bars appear a Stair Step pattern of eight monochromatic
 * blocks ranging from full amplitude white on the left to full black on the
 * right side.  This stair step pattern is used to verify the correct
 * alignment of the video amplifier in the monitor.  The stair step must
 * appear completely monochromatic (no color visible) if aligned correctly.
 * The bottom section contains a frequency burst pattern of short vertical
 * bars of black and white of varying width plus a group of single pixels.
 * This pattern provides a visible indication of the system video bandwidth
 * including the frame buffers, digital to analog converters, video cable
 * drivers, the interconnecting video coaxial cable and the video amplifiers
 * within the monitor.
 *
 * INPUT PARAMETERS :
 *
 *   None.
 *
 * OUTPUT
 *
 *  None.
 *
 * RETURNS:
 *
 *  Error code or SUCCESS.
 *
*/

int colorbar_tu (void)
{
  ulong_t xres, yres, tone;
  float   xscale, yscale;
  int     rc;
  uchar_t i;

  TITLE ("colorbar_tu");

  if ((rc = clear_screen ()) == SUCCESS)
  {
    /* setting up the color for monochromatic blocks (full amplitude from    */
    /* white to black                                                        */
    tone = 0xff;
    for (i = NUM_COLORS; i < NUM_COLORS + GRAY_TONES; i++, tone -= GRAY_DELTA)
    {
      fill_palette (tone, tone, tone,
                    i, i, (BOOL) (i == NUM_COLORS ? TRUE : FALSE));
    } /* endfor */

    get_screen_res (&xres, &yres);
    xscale = (float) (xres / REF_SCREEN_WIDTH);    /* x pixels per inch      */
    yscale = (float) (yres / REF_SCREEN_HEIGHT);   /* y pixels per inch      */

    if ((rc = draw_white_boxes(xscale, yscale)) == SUCCESS)
     if ((rc = draw_color_bars(xscale, yscale)) == SUCCESS)
      if ((rc = draw_grey_scale(xscale, yscale)) == SUCCESS)
       if ((rc = draw_piano_bars(xscale, yscale)) == SUCCESS)
        rc = draw_single_pixels((uchar_t) WHITE, xscale, yscale);
  } /* endif */

  return (rc);
}


/*
 * NAME : draw_white_boxes
 *
 * DESCRIPTION :
 *
 *  Displays the white boxes that make up the pattern.
 *
 * INPUT PARAMETERS :
 *
 *  1. X scale.
 *  2. Y scale.
 *
 * OUTPUT
 *
 *  None.
 *
 * RETURNS:
 *
 *  Error code or SUCCESS
 *
*/

static int draw_white_boxes(float xscale, float yscale)
{
  ulong_t i;
  BOX     box;
  int     rc;

  static BOX white_box[] =
         { { 0,    0,              3.63, (9.5/16.0), WHITE },
           { 5 * BAR_WIDTH, 1.0 + 11.0/16.0, REF_SCREEN_WIDTH, BAR_YSTART, WHITE }
         };

  TITLE ("draw_white_boxes");

  rc = SUCCESS;
  for(i = 0; rc == SUCCESS && i < sizeof(white_box) / sizeof(BOX); i++)
  {
    box.xstart = white_box[i].xstart * xscale;
    box.xend = white_box[i].xend * xscale;
    box.ystart = white_box[i].ystart * yscale;
    box.yend = white_box[i].yend * yscale;
    box.color = white_box[i].color;

    rc = draw_box(&box);
  }

  return (rc);
}



/*
 * NAME : draw_color_bars
 *
 * DESCRIPTION :
 *
 *  Draws the color bars section of the pattern.
 *
 * INPUT PARAMETERS :
 *
 *  1. X scale.
 *  2. Y scale.
 *
 * OUTPUT
 *
 *  None.
 *
 * RETURNS:
 *
 *  Error code or SUCCESS
 *
*/

static int draw_color_bars(float xscale, float yscale)
{
  uchar_t i;
  BOX     box;
  float   xdelta;
  int     rc;

  uchar_t color_bars [] = { GREEN, YELLOW, RED, MAGENTA, BLUE,
                            CYAN, BROWN, BLACK, WHITE
                          };

  TITLE ("draw_color_bars");

  xdelta = BAR_WIDTH * xscale;
  box.xstart = 0;
  box.ystart = BAR_YSTART * yscale;
  box.xend = xdelta;
  box.yend = BAR_YEND * yscale;

  rc = SUCCESS;
  for (i = 0; rc == SUCCESS && i < NUM_BARS; i++)
  {
    box.color = color_bars[i];
    rc = draw_box(&box);
    box.xstart = box.xend;
    box.xend += xdelta;
  }

  return (rc);
}



/*
 * NAME : draw_gray_scale
 *
 * DESCRIPTION :
 *
 *  Draws the  stair step pattern of monochromatic blocks.
 *
 * INPUT PARAMETERS :
 *
 *  1. X scale.
 *  2. Y scale.
 *
 * OUTPUT
 *
 *  None.
 *
 * RETURNS:
 *
 *  Error code or SUCCESS
 *
*/

static int draw_grey_scale(float xscale, float yscale)
{
  uchar_t  i;
  BOX      box;
  float    xdelta;
  int      rc;

  TITLE ("draw_gray_scale");

  xdelta = BAR_WIDTH * xscale;
  box.xstart = 0;
  box.ystart = BAR_YEND * yscale;
  box.xend = xdelta;
  box.yend = REF_SCREEN_HEIGHT * yscale;

  rc = SUCCESS;
  for(i = NUM_COLORS; rc == SUCCESS && i < NUM_COLORS + GRAY_TONES; i++)
  {
    box.color = i;
    rc = draw_box(&box);
    box.xstart = box.xend;
    box.xend += xdelta;
  }

  return (rc);
}



/*
 * NAME : draw_piano_bars
 *
 * DESCRIPTION :
 *
 *  Draws the alternating black and white bars section.
 *
 * INPUT PARAMETERS :
 *
 *  1. X scale.
 *  2. Y scale.
 *
 * OUTPUT
 *
 *  None.
 *
 * RETURNS:
 *
 *  Error code or SUCCESS
 *
*/

static int draw_piano_bars(float xscale, float yscale)
{
  ulong_t i, j;
  uchar_t tone;
  BOX     box;
  float   xdelta;
  int     rc;

  TITLE ("draw_piano_bars");

  box.xstart = (11.0/16.0) * xscale;
  box.ystart = 8.0 * yscale;
  box.xend = box.xstart + (11.0/16.0) * xscale;
  box.yend = REF_SCREEN_HEIGHT * yscale;
  box.color = BLACK;
  rc = draw_box(&box);

  xdelta = (4.0/16.0) * xscale;

  for(j = 0, tone = WHITE; rc == SUCCESS && j < BW_BAR_GROUPS;
                                               j++, xdelta /= 2.0)
  {
    if(j == (BW_BAR_GROUPS - 1))
      tone = BLACK;

    for(i = 0; rc == SUCCESS && i < BW_BARS; i++)
    {
      box.xstart = box.xend;
      box.xend += xdelta;
      box.color = tone;
      rc = draw_box(&box);

      if((j == (BW_BAR_GROUPS - 1)) && i < 2)
        continue;
      else
        tone = (tone == WHITE) ? BLACK : WHITE;
    }
  }

  return (rc);
}


/*
 * NAME : draw_single_pixels
 *
 * DESCRIPTION :
 *
 *  Draws the 9 dots in the lower right corner.
 *
 * INPUT PARAMETERS :
 *
 *  1. Pixel color.
 *  2. X scale.
 *  3. Y scale.
 *
 * OUTPUT
 *
 *  None.
 *
 * RETURNS:
 *
 *  Error code or SUCCESS
 *
*/

static int draw_single_pixels(uchar_t color, float xscale, float yscale)
{
  POINT    point;
  ulong_t  x, y, i, j, status, packed_xy;
  float    xdelta, ydelta;
  int      rc;

  TITLE ("draw_single_pixels");

  xdelta = trunc ((4.0/16.0) * xscale);
  ydelta = trunc ((4.0/16.0) * yscale);

  rc = SUCCESS;
  point.color = color;
  for(j = 0, y = (ulong_t) uitrunc (DOTS_YSTART * yscale);
        rc == SUCCESS && j < NUMBER_DOTS; j++, y += (ulong_t) ydelta)
  {
    point.y = y;
    for(i = 0, x = (ulong_t) uitrunc (DOTS_XSTART * xscale);
          rc == SUCCESS && i < NUMBER_DOTS; i++, x += (ulong_t) xdelta)
    {
      point.x = x;
      rc = draw_point (&point);
    }
  }

  return (rc);
}


/*
 * NAME : save_default_colors
 *
 * DESCRIPTION :
 *
 *   Save the initial palette RAM (256 colors).  The palette RAM
 *   should be set up by the device driver.
 *
 * INPUT :
 *
 *   None.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   None.
 *
*/

static void save_default_colors (void)
{
  int     i;
  ulong_t addr, color;
  static BOOL  saved_colors = FALSE;

  TITLE("save_default_colors");

#ifdef SET_COLORS                                /* for debugging only       */
  saved_colors = TRUE;                           /* do not read from palette */
#endif

  if (!saved_colors)
  {
#ifdef DUMP_REGS
    printf ("\nDumping PALETTE RAM ......");  fflush (stdout);
#endif

    load_dac_reg ((ulong_t) COLOR_PALETTE_RAM_OFFSET);

    for(i = 0; i < PAL_LINES; i++)
    {
      get_igc_reg ((uchar_t) COLOR_PALETTE_RAM, &color);       /* red cycle  */
      default_colors[i].red = (uchar_t) color;
      get_igc_reg ((uchar_t) COLOR_PALETTE_RAM, &color);       /* green cycle*/
      default_colors[i].green = (uchar_t) color;
      get_igc_reg ((uchar_t) COLOR_PALETTE_RAM, &color);       /* blue cycle */
      default_colors[i].blue = (uchar_t) color;

#ifdef DUMP_REGS
      printf ("\nIndex = %3d     red = 0x%02x       green = 0x%02x       blue = 0x%02x",
              i, default_colors[i].red, default_colors[i].green,
              default_colors[i].blue);
      fflush (stdout);
#endif
    }
    saved_colors = TRUE;
  }

  return;
}


#ifdef TRASH
/*
 * NAME : restore_default_colors
 *
 * DESCRIPTION :
 *
 *   Restore the default palette RAM (256 colors) from the saved area.
 *
 * INPUT :
 *
 *   None.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   None.
 *
*/

void restore_default_colors (void)
{
  int     i;

  TITLE("restore_default_colors");

  load_dac_reg ((ulong_t) COLOR_PALETTE_RAM_OFFSET);

  for(i = 0; i < PAL_LINES; i++)
  {
    write_igc_reg ((uchar_t)COLOR_PALETTE_RAM, (ulong_t) default_colors[i].red);
    write_igc_reg ((uchar_t)COLOR_PALETTE_RAM, (ulong_t) default_colors[i].green);
    write_igc_reg ((uchar_t)COLOR_PALETTE_RAM, (ulong_t) default_colors[i].blue);
  }

  return;
}
#endif


/*
 * NAME : color_init
 *
 * DESCRIPTION :
 *
 *   Initialize the primary colors for all of the TUs.
 *
 * INPUT :
 *
 *   None.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   None.
 *
*/


void color_init (void)
{
  int    i;
  static struct RGB colors [NUM_COLORS] =
                      { 0,       0,    0,     /* BLACK         */
                        0xFF,    0,    0,     /* RED           */
                        0,    0xFF,    0,     /* GREEN         */
                        0xF0, 0xF0,    0,     /* YELLOW        */
                        0,       0, 0xF0,     /* BLUE          */
                        0xB0,    0, 0xB0,     /* MAGENTA       */
                        0,    0xF0, 0xF0,     /* BABY BLUE     */
                        0xC8, 0xC8, 0xC8,     /* LIGHT GRAY    */
                        0x78, 0x78, 0x78,     /* GRAY          */
                        0xF0, 0x80, 0x20,     /* ORANGE        */
                        0x50, 0xE0, 0x30,     /* DARK GREEN    */
                        0xC0, 0x80, 0x20,     /* BROWN         */
                        0x30, 0x60, 0xFF,     /* LIGHT BLUE    */
                        0xE0, 0x30, 0xE0,     /* LIGHT MAGENTA */
                        0x70, 0xF0, 0xD0,     /* CYAN          */
                        0xFF, 0xFF, 0xFF,     /* WHITE         */
                        0xF9, 0x2F, 0x5D,     /* BRICK RED     */
                        0xDF, 0x7E, 0x0A,     /* RUST          */
                        0XFE, 0x47, 0xD6,     /* PINK          */
                        0xF5, 0xBE, 0x9F,     /* BEIGE         */
                        0xBC, 0xCF, 0x07,     /* OLIVE GREEN   */
                        0xE3, 0x6F, 0x76,     /* SALMON        */
                        0xDE, 0xEB, 0xEF,     /* OFF WHITE     */
                        0xDF, 0xED, 0x77,     /* LIGHT GREEN   */
                        0x5E, 0x77, 0xFA,     /* SKY BLUE      */
                        0xF7, 0xAA, 0xE2,     /* PALE PINK     */
                        0x7B, 0x4A, 0x31      /* DARK BROWN    */
                      };

  TITLE("color_init");

  save_default_colors ();                        /* save the DD palette RAM  */

  load_dac_reg ((ulong_t) COLOR_PALETTE_RAM_OFFSET);

  /* fill up the palette RAM with all of the default primary colors          */
  for(i = 0; i < NUM_COLORS; i++)
  {
    write_igc_reg ((uchar_t)COLOR_PALETTE_RAM, (ulong_t) colors[i].red);
    write_igc_reg ((uchar_t)COLOR_PALETTE_RAM, (ulong_t) colors[i].green);
    write_igc_reg ((uchar_t)COLOR_PALETTE_RAM, (ulong_t) colors[i].blue);
  }

  return;
}



/*
 * NAME : get_color_pattern
 *
 * DESCRIPTION :
 *
 *  Returns a word of color pattern formatted data.
 *
 * INPUT PARAMETERS :
 *
 *  1. color.
 *
 * OUTPUT
 *
 *  None.
 *
 * RETURNS:
 *
 *  Word of color pattern formatted data.
 *
*/

ulong_t get_color_pattern (ulong_t color)
{
  ulong_t i, color_pattern;

  color_pattern = 0;
  for(i = 1; i <= BYTES_IN_WORD; i++)
  {
    color_pattern |= color;
    color <<= BITS_IN_BYTE;
  }

  return (color_pattern);
}

