static char sccsid[] = "@(#)61	1.1  src/bos/diag/tu/gga/advtu.c, tu_gla, bos41J, 9515A_all 4/6/95 09:26:40";
/*
 *   COMPONENT_NAME: tu_gla
 *
 *   FUNCTIONS: CURSOR_REGION_YEND
 *              CURSOR_REGION_YSTART
 *              CURSOR_XPOS
 *              CURSOR_YPOS
 *              GRAY_YEND
 *              GRAY_YSTART
 *              SINGLE_PIXELS_YSTART
 *              THICK_COLOR_YEND
 *              THICK_COLOR_YSTART
 *              THIN_COLOR_YEND
 *              advanced_dply_tu
 *              draw_cursor
 *              draw_cursor_box
 *              draw_gray_scale
 *              draw_lines
 *              draw_pixel_block
 *              draw_quad_cmds
 *              init_palette
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <sys/types.h>
#include <stdio.h>
#include <math.h>

#include "exectu.h"
#include "tu_type.h"
#include "ggamisc.h"

#define FOREVER                    while (TRUE)


#define CURSOR_REGION_YLINES       150
#define CURSOR_REGION_YSTART(yres) (yres / 2 - CURSOR_REGION_YLINES)
#define CURSOR_REGION_YEND(yres)   (yres / 2)
#define CURSOR_BOX_PELS            100
#define SINGLE_PIXELS_YLINES       20
#define SINGLE_PIXELS_XCOLS        200
#define SINGLE_PIXELS_YSTART(yres) ((CURSOR_REGION_YSTART(yres) - SINGLE_PIXELS_YLINES) / 2)

#define THIN_COLOR_XSPACING        20
#define THIN_COLOR_XWIDTH          1
#define THIN_COLOR_XSTART          (SINGLE_PIXELS_XCOLS + 1)
#define THIN_COLOR_YSTART          0
#define THIN_COLOR_YEND(yres)      CURSOR_REGION_YSTART(yres)

#define NUM_THICK_COLOR_BARS       NUM_COLORS
#define THICK_COLOR_XSTART         0
#define THICK_COLOR_YSTART(yres)   CURSOR_REGION_YEND(yres)
#define THICK_COLOR_YEND(yres)     ((3 * yres) / 4)

#define GRAY_XSTART                0
#define GRAY_YSTART(yres)          (THICK_COLOR_YEND(yres) + 1)
#define GRAY_YEND(yres)            yres
#define NUM_GRAY_LEVELS            (PAL_LINES - NUM_COLORS)

#define GRAY_DELTA                 (PAL_LINES / NUM_GRAY_LEVELS)
#define GRAY_PALETTE_START         NUM_COLORS
#define NUM_GRAY_LINES             NUM_GRAY_LEVELS

/* Cursor hot spot coordinates */
#define CURSOR_XPOS(xres)          (xres / 2)
#define CURSOR_YPOS(yres)          (CURSOR_REGION_YSTART(yres) + CURSOR_REGION_YLINES / 2)


static void init_palette (void);
static int draw_pixel_block (ULONG);
static int draw_lines (ULONG, ULONG);
static int draw_quad_cmds (ULONG, ULONG);
static int draw_gray_scale (ULONG, ULONG);
static int draw_cursor_box (ULONG, ULONG);
static int draw_cursor (ULONG, ULONG);



/*
 * NAME : advanced_dply_tu
 *
 * DESCRIPTION :
 *
 *  The purpose of this test is to provide a comprehensive test of the
 *  graphic adapter in one visual screen using all of the QUAD commands
 *  (POINT, LINE, RECT, TRIANGLE and QUAD).
 *
 *  Creates an "advanced display pattern" as follows :
 *  In the top left section of the screen 20 x 200 single pixels
 *  are drawn followed by 1 pixel wide alternating white, red, green,
 *  and blue vertical lines spaced by 20 pixels. This section of the
 *  screen is useful for detecting GGA defects such as palette shifting
 *  and jitter.
 *  The next section contains a transparent/red/green/blue cursor on a
 *  white box. The box is drawn using the window origin function.
 *  The next section consists of vertical bars of the primary colors
 *  of red, green and blue plus the complimentary colors of white,
 *  yellow, magenta and cyan followed by brown and black.  This section
 *  of the pattern provides the ability to evaluate the system and monitors
 *  color fidelity and transient response from color to color and color
 *  hue (phase delay and phase shift). Window clipping is used in some
 *  of these patterns but not all.
 *  Below the color bars appear a Stair Step pattern of eight monochromatic
 *  blocks ranging from full amplitude black on the left to full white on the
 *  right side. The stair step must appear completely monochromatic (no
 *  color visible) if it aligns correctly.  Window clipping is used to
 *  produce this stair step pattern.
 *
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


int advanced_dply_tu (void)
{
  int     rc;
  ULONG xres, yres;

  TITLE("advanced_dply_tu");

  if ((rc = clear_screen ()) == SUCCESS)
  {
    init_palette ();                             /* initialize the palette   */
    get_screen_res (&xres, &yres);               /* get screen resolution    */

    if ((rc = draw_pixel_block (yres)) == SUCCESS)
      if ((rc = draw_lines (xres, yres)) == SUCCESS)
        if ((rc = draw_cursor_box (xres, yres)) == SUCCESS)
          if ((rc = draw_cursor (xres, yres)) == SUCCESS)
            if ((rc = draw_quad_cmds (xres, yres)) == SUCCESS)
              if ((rc = draw_gray_scale (xres, yres)) == SUCCESS)
              {
                while(!end_tu(get_dply_time()));
              };
  } /* endif */

  return (rc);
}



/*
 * NAME : init_palette
 *
 * DESCRIPTION :
 *
 *  Initializes the palette RAM
 *
 * INPUT PARAMETERS :
 *
 *  None.
 *
 * OUTPUT
 *
 *  Palette RAM initialized.
 *
 * RETURNS:
 *
 *  None.
 *
*/

static void init_palette (void)
{
  UCHAR  tone;
  UCHAR  i;
  ULONG  delta;

  TITLE ("init_palette");

  delta = (ULONG) uitrunc ((double) GRAY_DELTA);  /* gray scale delta      */

  /* set gray levels follow primary colors (NUM_COLORS) in palette RAM       */
  for(i = 0, tone = 0; i < NUM_GRAY_LEVELS; i++, tone += delta)
  {
    fill_palette (tone, tone, tone, (UCHAR) (GRAY_PALETTE_START + i),
                  (UCHAR) (GRAY_PALETTE_START + i));
  }

  return;
}



/*
 * NAME : draw_pixel_block
 *
 * DESCRIPTION :
 *
 *  Displays 20 x 200 single pixels on the top left portion of
 *  the screen.
 *
 * INPUT PARAMETERS :
 *
 *  1. y resolution.
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

static int draw_pixel_block (ULONG yres)
{
  POINT   point;
  ULONG x, y, xlimit, ylimit;
  int     rc;

  TITLE ("draw_pixel_block");

  rc = SUCCESS;
  ylimit = (SINGLE_PIXELS_YSTART(yres) + SINGLE_PIXELS_YLINES);
  xlimit = SINGLE_PIXELS_XCOLS;

  /* draw block of single pixels                                             */
  point.color = WHITE;
  for (y = SINGLE_PIXELS_YSTART(yres); rc == SUCCESS && y < ylimit; y++)
  {
    point.y = y;
    for (x = 0; rc == SUCCESS && x < xlimit; x += 2)
    {
      point.x = x;
      rc = draw_point (&point);
    } /* endfor */
  } /* endfor */

  return (rc);
}



/*
 * NAME : draw_lines
 *
 * DESCRIPTION :
 *
 *  Draws a thin vertical line (1 pixel wide) of alternating white,
 *  red, green and blue.  Each vertical lines spaced by 20 pixels portion
 *  of the pattern
 *
 * INPUT PARAMETERS :
 *
 *  1. x resolution.
 *  2. y resolution.
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

static int draw_lines (ULONG xres, ULONG yres)
{
  LINE    line;
  ULONG xdelta;
  UCHAR i;
  UCHAR color_bars[] = { WHITE, RED, GREEN, BLUE };
  UCHAR colors;
  int     rc;

  TITLE("draw_lines");

  colors = sizeof(color_bars) / sizeof(UCHAR);

  /* enable window clipping.                                                 */
  set_wclip (THIN_COLOR_XSTART - 1, THIN_COLOR_YSTART, xres - 1,
                                         THIN_COLOR_YEND(yres));

  /* initialize rectangle for thin color bars */
  xdelta   = THIN_COLOR_XSPACING + THIN_COLOR_XWIDTH;
  line.x1  = line.x2 = THIN_COLOR_XSTART;
  line.y1  = THIN_COLOR_YSTART;
  line.y2  = THIN_COLOR_YEND (yres);

  rc = SUCCESS;
  i  = 0;
  while (rc == SUCCESS && line.x1 < xres)
  {
    line.color = color_bars [i % colors];        /* setting up the color     */
    rc = draw_line (&line);
    line.x1 = line.x2 += xdelta;
    i++;
  }

  return (rc);
}




/*
 * NAME : draw_quad_cmds
 *
 * DESCRIPTION :
 *
 *  Draws the section consisting of nine vertical bars of the primary
 *  colors of red, green and blue plus the complimentary colors of yellow,
 *  magenta and cyan followed by black and white.
 *
 * INPUT PARAMETERS :
 *
 *  1. x resolution.
 *  2. y resolution.
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

#define NO_CLIP_THICK_BOXES          2
#define CLIP_THICK_BOXES             2

static int draw_quad_cmds (ULONG xres, ULONG yres)
{
  BOX      box;
  QUAD     quad;
  TRIANGLE triangle;
  ULONG  xdelta;
  ULONG  num_color_bars, i, j;
  ULONG  x_min, y_min, x_max, y_max;
  int      rc;
  UCHAR  color_bars [] = { WHITE, YELLOW, GREEN, CYAN, MAGENTA, RED, BLUE,
                             BROWN, BLACK
                           };

  TITLE ("draw_quad_cmds");

  num_color_bars = (ULONG) (sizeof(color_bars) / sizeof(UCHAR));
  xdelta = (ULONG) uitrunc ((double) (xres / num_color_bars));    /* width */

  set_wclip (0, 0, xres - 1, yres - 1);          /* disable window clipping  */

  /* draw first 2 color bars without clipping */
  box.xstart = (float) (THICK_COLOR_XSTART);
  box.ystart = (float) (THICK_COLOR_YSTART(yres));
  box.xend   = (float) xdelta;
  box.yend   = (float) (THICK_COLOR_YEND(yres));

  rc = SUCCESS;
  for(i = 0; rc == SUCCESS && i < NO_CLIP_THICK_BOXES; i++)
  {
    box.color = color_bars [i % num_color_bars];
    rc = draw_box (&box);
 /* box.xstart = trunc((double)box.xend) + 1.0; */
    box.xstart = uitrunc((double)box.xend) + 1.0;
    box.xend = box.xstart + (float) xdelta;
  }

  if (rc == SUCCESS)
  {
    /* use clipping for the rest of thick color bars by setting wind. bound. */
    x_min = (ULONG) uitrunc ((double)box.xstart) - 1;  /* x (offset from 0)*/
    y_min = (ULONG) uitrunc ((double)box.ystart);
    x_max = xres;
    y_max = (ULONG) uitrunc ((double)box.yend);

    /* turn on clipping for next 2 color bar */
    set_wclip (x_min - 1, y_min, xres - 1, y_max);

    /* setup everything out of bound so we can test the clipping algorithm.  */
    box.ystart = (float) (CURSOR_REGION_YSTART(yres));
    box.yend   = (float) yres;

    for(j = 0; rc == SUCCESS && j < CLIP_THICK_BOXES; j++, i++)
    {
      box.color = color_bars [i % num_color_bars];
      rc = draw_box(&box);

      box.xstart = box.xend + 1.0;
      box.xend = box.xstart + (float) xdelta;
    }

    if (rc == SUCCESS)
    {
      /* draw an unclipped hourglass quad */
      quad.x[0]  = box.xstart;
      quad.y[0]  = (float) (THICK_COLOR_YSTART(yres));
      quad.x[1]  = box.xend;
      quad.y[1]  = quad.y[0];
      quad.x[2]  = quad.x[0];
      quad.y[2]  = (float) (THICK_COLOR_YEND(yres));
      quad.x[3]  = box.xend;
      quad.y[3]  = quad.y[2];
      quad.color = color_bars [i++ % num_color_bars];
      set_wclip ((ULONG) uitrunc((double)quad.x[0]) - 1,
                 (ULONG) uitrunc((double)quad.y[0]),
                 xres - 1, (ULONG) uitrunc((double)quad.y[2]));

      if ((rc = draw_quad (&quad)) == SUCCESS)
      {
        /* draw a clipped hourglass quad */
        quad.x[0] = quad.x[1] + 1.0;
        quad.x[1] = quad.x[0] + (float) xdelta;
        quad.x[2] = quad.x[0];
        quad.x[3] = quad.x[1];
        quad.color = color_bars [i++ % num_color_bars];
        set_wclip ((ULONG) uitrunc((double)quad.x[0]) - 1,
                   (ULONG) uitrunc((double)(quad.y[0] + ((quad.y[2] - quad.y[0]) / 3))),
                   (ULONG) uitrunc((double)quad.x[1]),
                   (ULONG) uitrunc((double)(quad.y[3] - ((quad.y[3] - quad.y[1]) / 3))));

        if ((rc = draw_quad (&quad)) == SUCCESS)
        {
          /* draw unclipped triangles */
          triangle.x[0]  = (xdelta / 2) + quad.x[1];
          triangle.y[0]  = (float) (THICK_COLOR_YSTART(yres));
          triangle.x[1]  = quad.x[3] + (float) xdelta;
          triangle.y[1]  = quad.y[3];
          triangle.x[2]  = quad.x[3] + 1.0;
          triangle.y[2]  = (float) (THICK_COLOR_YEND(yres));
          triangle.color = color_bars [i++ % num_color_bars];
          set_wclip (0, 0, xres - 1, yres - 1);
          if ((rc = draw_triangle (&triangle)) == SUCCESS)
          {
            triangle.x[2]  = triangle.x[0] + (float) xdelta;
            triangle.y[2]  = triangle.y[0];
            triangle.color = BROWN;
            if ((rc = draw_triangle (&triangle)) == SUCCESS)
            {
              triangle.x[0] = triangle.x[1] + (float) xdelta;
              triangle.y[0] = triangle.y[1];
              triangle.color= WHITE;
              if (rc = draw_triangle (&triangle) == SUCCESS)
              {
                /* draw a quadrilateral */
                quad.x[0]  = triangle.x[2] + 1.0;
                quad.y[0]  = triangle.y[2];
                quad.x[1]  = xres;
                quad.y[1]  = quad.y[0];
                quad.x[2]  = xres;
                quad.y[2]  = triangle.y[0];
                quad.x[3]  = triangle.x[0];
                quad.y[3]  = quad.y[2];
                quad.color = YELLOW;
                if ((rc = draw_quad (&quad)) != SUCCESS)
                {
                  LOG_MSG ("Can't draw quadrilateral");
                }
              }
              else
                LOG_MSG ("Can't draw unclipped triangles");
            }
            else
              LOG_MSG ("Can't draw unclipped triangles");
          }
          else
            LOG_MSG ("Can't draw unclipped triangles");
        }
        else
          LOG_MSG ("Can't draw clipped hourglass");
      }
      else
        LOG_MSG ("Can't draw unclipped hourglass");
    }
    else
      LOG_MSG ("CLIP_THICK_BOXES failed");
  }
  else
    LOG_MSG ("NO_CLIP_THICK_BOXES failed");

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
 *  1. x resolution.
 *  2. y resolution.
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

static int draw_gray_scale(ULONG xres, ULONG yres)
{
  ULONG  xdelta;
  UCHAR  i;
  BOX      box;
  int      rc;

  TITLE("draw_gray_scale");

  xdelta     = (ULONG) uitrunc ((double)(xres / NUM_GRAY_LINES));
  box.xstart = GRAY_XSTART;
  box.ystart = GRAY_YSTART(yres);
  box.xend   = xdelta;
  box.yend   = GRAY_YEND(yres);

  /* set widow clipping */
  set_wclip (0, (ULONG) box.ystart, xres - 1, yres - 1);

  rc = SUCCESS;
  for (i = 0; i < NUM_GRAY_LINES; i++)
  {
    box.color = GRAY_PALETTE_START + i;
    rc = draw_box (&box);
    if (rc != SUCCESS)
    {
      LOG_MSG ("Can't draw gray scale");
      break;
    } /* endif */

    box.xstart = box.xend + 1.0;
    box.xend = box.xstart + (float) xdelta;
  }

  return (rc);
}


/*
 * NAME : draw_cursor_box
 *
 * DESCRIPTION :
 *
 *  Draws the box where cursor will be positioned.
 *
 * INPUT PARAMETERS :
 *
 *  1. x resolution.
 *  2. y resolution.
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

static int draw_cursor_box(ULONG xres, ULONG yres)
{
  BOX   box;
  int   rc;

  set_wclip (0, 0, xres - 1, yres - 1);
  box.xstart = (float) CURSOR_XPOS(xres) - (CURSOR_BOX_PELS / 2);
  box.ystart = (float) CURSOR_YPOS(yres) - (CURSOR_BOX_PELS / 2);
  box.xend   = box.xstart + CURSOR_BOX_PELS;
  box.yend   = box.ystart + CURSOR_BOX_PELS;
  box.color  = WHITE;
  rc = draw_box(&box);
  if (rc != SUCCESS)
  {
    LOG_MSG ("Can't draw cursor box");
  } /* endif */

  return (rc);
}



/*
 * NAME : draw_cursor
 *
 * DESCRIPTION :
 *
 *  Draws the cursor.
 *
 * INPUT PARAMETERS :
 *
 *  1. x resolution.
 *  2. y resolution.
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

#define CRAM_SIZE                  64            /* cursor RAM is 64x64x2    */
#define BITS_PER_PIXEL             2
#define PIXELS_IN_BYTE             (BITS_IN_BYTE / BITS_PER_PIXEL)

static int draw_cursor (ULONG xres, ULONG yres)
{
  static struct
  {
    UCHAR red, green, blue;
    UCHAR reg;
  } color_regs [NUM_CURSOR_COLORS] =
               {
                  0x00, 0x00, 0x00, CURCOL_TRANS_COLOR, /* transparent        */
                  0xff, 0x00, 0x00, CURCOL1,            /* color 1 = red      */
                  0x00, 0xff, 0x00, CURCOL2,            /* color 2 = green    */
                  0x00, 0x00, 0xff, CURCOL3,            /* color 3 = blue     */
               };
  static  UCHAR cursor_color [NUM_CURSOR_COLORS];
  int     rc, i, x, y, num_pixels_per_color, pixels_wrote;
  UCHAR tempc;
  ULONG ltemp;

  TITLE("draw_cursor");

  /* setting up the cursor color registers (RGB)                           */
  WriteIBM525(RGB525_CURSOR_1_RED,   color_regs[CURCOL1].red  );
  WriteIBM525(RGB525_CURSOR_1_GREEN, color_regs[CURCOL1].green);
  WriteIBM525(RGB525_CURSOR_1_BLUE,  color_regs[CURCOL1].blue );
  WriteIBM525(RGB525_CURSOR_2_RED,   color_regs[CURCOL2].red  );
  WriteIBM525(RGB525_CURSOR_2_GREEN, color_regs[CURCOL2].green);
  WriteIBM525(RGB525_CURSOR_2_BLUE,  color_regs[CURCOL2].blue );
  WriteIBM525(RGB525_CURSOR_3_RED,   color_regs[CURCOL3].red  );
  WriteIBM525(RGB525_CURSOR_3_GREEN, color_regs[CURCOL3].green);
  WriteIBM525(RGB525_CURSOR_3_BLUE,  color_regs[CURCOL3].blue );

  /* initialize all colors for the cursor                                  */
  for (i = 1; i < NUM_CURSOR_COLORS; i++)
  {
    cursor_color [i] = color_regs[i].reg;
  } /* endfor */

  /* Make sure the cursor will be 64x64 and auto-increment is enabled */

  tempc = ReadIBM525(RGB525_CURSOR_CTL);

  if ((tempc & RGB525_CURSOR_SIZE_MASK) != RGB525_CURSOR_SIZE_64x64)
    {
      tempc &= ~RGB525_CURSOR_SIZE_MASK;
      tempc |= RGB525_CURSOR_SIZE_64x64;
      WriteIBM525(RGB525_CURSOR_CTL, tempc);
    }

  ltemp = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
  ltemp = RL(W9100_INDEXCONTR);
  tempc = ltemp >> 8;

  if ((tempc && INDEX_CONTROL_MASK) != INDEX_CONTROL_INC_ON)
    tempc |= INDEX_CONTROL_INC_ON;

  ltemp = tempc;
  ltemp |= (ltemp << 24) | (ltemp << 16) | (ltemp << 8);
  dac_workaround();
  ltemp = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
  WL(ltemp, W9100_INDEXCONTR);

  /* draw the cursor offset with the screen coordinate system                */

  rc = set_cursor_pos (CURSOR_XPOS(xres) - 1, CURSOR_YPOS(yres) - 1);

  if (rc == SUCCESS)
  {
    num_pixels_per_color = CRAM_SIZE / (NUM_CURSOR_COLORS - 1);

    /* set up the cursor ram pattern */

    dac_workaround();
    ltemp = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
    WL(0x00000000, W9100_INDEXLOW);
    dac_workaround();
    ltemp = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
    WL(0x01010101, W9100_INDEXHIGH);
    for(y = 0; y < CRAM_SIZE; y++)
    {
      pixels_wrote = 0;
      for(i = 1; i < NUM_CURSOR_COLORS; i++)
      {
        for (x = 0; x < num_pixels_per_color / PIXELS_IN_BYTE;
                x++, pixels_wrote += PIXELS_IN_BYTE)
        {
          dac_workaround();
          ltemp = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
          ltemp = cursor_color[i];
          ltemp |= (ltemp << 24) | (ltemp << 16) | (ltemp << 8);
          WL(ltemp, W9100_INDEXDATA);
        } /* endfor */
      }  /* end for */

      /* for the rest of cursor pixels, we turn it off (do not display)      */
      for (i = pixels_wrote; i < CRAM_SIZE; i += PIXELS_IN_BYTE)
      {
        dac_workaround();
        ltemp = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
        ltemp = cursor_color[0];
        ltemp |= (ltemp << 24) | (ltemp << 16) | (ltemp << 8);
        WL(ltemp, W9100_INDEXDATA);
      } /* endfor */
    } /* endfor */
  } /* endif */

  enable_cursor ();

  return (rc);
}

