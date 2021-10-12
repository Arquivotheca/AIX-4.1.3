static char sccsid[] = "@(#)65	1.1  src/bos/diag/tu/gga/cursortu.c, tu_gla, bos41J, 9515A_all 4/6/95 09:26:47";
/*
 *   COMPONENT_NAME: tu_gla
 *
 *   FUNCTIONS: cursor_ram_tu
 *              dply_cursor_tu
 *              draw_rgb_cursor
 *              fill_cursor_ram_color
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

#define CURSOR_RAM_SIZE              0x100       /* cursor RAM size in words */
#define GET_RAND_CHAR                ((UCHAR) ((rand() / MAX_RAND) * (double) 255))
#define CURSOR_RES                   64          /* cursor RAM is 64 * 64 * 2*/
#define CURSOR_RAM_SIZE_IN_BYTE      0x3FF
#define ITERATION                    100




static UCHAR cursor_colors [NUM_CURSOR_COLORS];
static struct
{
  UCHAR red, green, blue, reg;
} color_regs [NUM_CURSOR_COLORS] =
             {
                0x00, 0x00, 0x00, CURCOL_TRANS_COLOR,  /* transparent color   */
                0xff, 0x00, 0x00, CURCOL1,             /* color reg 1 = red   */
                0x00, 0xff, 0x00, CURCOL2,             /* color reg 2 = green */
                0x00, 0x00, 0xff, CURCOL3,             /* color reg 3 = blue  */
              };

static void fill_cursor_ram_color (UCHAR);


/*
 * NAME : cursor_ram_tu
 *
 * DESCRIPTION :
 *
 *  Performs several function such as write/read/verify tests on the
 *  64x64 cursor RAM.  Two different tests will be performed.  The first
 *  test run for 100 times.  Durring each iteration of the first test,
 *  it moves the cursor to a random location, fill the cursor RAM with
 *  different colors (primary colors, RGB) and then read/verify every
 *  location of the cursor RAM for the color it just read.  For the second
 *  test, a decay test is performed by writing all 1s, waiting for
 *  approx. 5 secs before reading and verifying the cursor RAM.
 *
 * INPUT PARAMETERS :
 *
 *  None.
 *
 * OUTPUT
 *
 *  Cursor RAM altered.
 *
 * RETURNS:
 *
 *  Error code or SUCCESS.
 *
*/

int cursor_ram_tu (void)
{
   int       j, k, rc;
   ULONG   scratch[CURSOR_RAM_SIZE], i, ltemp, value;
   ULONG   xres, yres, addr, x, y;
   UCHAR   tmp_char, color, data;

   /* setting up the cursor color registers (RGB)                          */
   WriteIBM525(RGB525_CURSOR_1_RED,   color_regs[CURCOL1].red  );
   WriteIBM525(RGB525_CURSOR_1_GREEN, color_regs[CURCOL1].green);
   WriteIBM525(RGB525_CURSOR_1_BLUE,  color_regs[CURCOL1].blue );
   WriteIBM525(RGB525_CURSOR_2_RED,   color_regs[CURCOL2].red  );
   WriteIBM525(RGB525_CURSOR_2_GREEN, color_regs[CURCOL2].green);
   WriteIBM525(RGB525_CURSOR_2_BLUE,  color_regs[CURCOL2].blue );
   WriteIBM525(RGB525_CURSOR_3_RED,   color_regs[CURCOL3].red  );
   WriteIBM525(RGB525_CURSOR_3_GREEN, color_regs[CURCOL3].green);
   WriteIBM525(RGB525_CURSOR_3_BLUE,  color_regs[CURCOL3].blue );

   if ((rc = clear_screen ()) == SUCCESS)
   {
     enable_cursor ();
     if ((get_cursor_status() & RGB525_CURSOR_MODE_MASK) != RGB525_CURSOR_MODE_OFF)
     {
       get_screen_res (&xres, &yres);
       for (j = 0; j < ITERATION && rc == SUCCESS; j++)
       {
         for (i = 1; i < NUM_CURSOR_COLORS && rc == SUCCESS; i++)
         {
           x = GET_RAND_ULONG % xres;
           y = GET_RAND_ULONG % yres;
           rc = set_cursor_pos (x, y);
           if (rc == SUCCESS)
           {
             color = color_regs[i].reg;
             fill_cursor_ram_color (color);      /* write the test pattern   */

             /* verify the cursor RAM with the pattern just wrote            */

             dac_workaround();
             ltemp = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
             WL(0x00000000, W9100_INDEXLOW);
             dac_workaround();
             ltemp = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
             WL(0x01010101, W9100_INDEXHIGH);

             for (k = 0; k <= CURSOR_RAM_SIZE_IN_BYTE && rc == SUCCESS; k++)
             {
               dac_workaround();
               ltemp = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
               ltemp = RL(W9100_INDEXDATA);
               data = ltemp >> 8;
               data &= 0x03; /* color in low 2 bits */
               if (data != (color & 0x03))
               {
                 rc = CRAM_RW_ERR;
                 addr = RGB525_CURSOR_ARRAY + k;
                 set_mem_info (addr, (ULONG) color, (ULONG) data, rc, "", "");
               } /* endif */
             } /* endfor */
           }
           else
           {
             printf ("\nCursor position x = %d  y = %d  j = %d  i = %d", x, y, j, i);
           } /* endif */
         } /* endfor */
       } /* endfor */

       if (rc == SUCCESS)
       {
         /* Second test, decay test. Write all 1's, delay, and then verify   */
         /* the entire cursor RAM                                            */
         color = 0xff;                        /* data to fill cursor RAM     */

         /* fill up the entire cursor RAM with color                         */
         fill_cursor_ram_color (color);

         sleep (DECAY_MEMORY_TIME);              /* check for possible decay */

         /* Read and verify the entire cursor RAM */
         dac_workaround();
         ltemp = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
         WL(0x00000000, W9100_INDEXLOW);
         dac_workaround();
         ltemp = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
         WL(0x01010101, W9100_INDEXHIGH);
         for (k = 0; k <= CURSOR_RAM_SIZE && rc == SUCCESS; k++)
         {
           dac_workaround();
           ltemp = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
           ltemp = RL(W9100_INDEXDATA);
           data = ltemp >> 8;
           data &= 0x03; /* color in low 2 bits */
           if (data != (color & 0x03))
           {
             rc = CRAM_RW_ERR;
             addr = RGB525_CURSOR_ARRAY + k;
             set_mem_info (addr, (ULONG) color, (ULONG) data, rc, "", "");
           } /* endif */
         } /* endfor */
       } /* endif */
     } /* endif */
     else
       rc = SET_CURSOR_ERR;
   } /* endif */

   return (rc);
}



/*
 * NAME : dply_cursor_tu
 *
 * DESCRIPTION :
 *
 *  Displays a moving cursor horizontally and vertically.
 *
 * INPUT PARAMETERS :
 *
 *  None.
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

#define SLEEP_TIME (50 * 1000)  /* usec */

int dply_cursor_tu(void)
{
  ULONG  x, y, xres, yres;
  int      rc;

  TITLE ("dply_cursor_tu");

  if ((rc = clear_screen ()) == SUCCESS)
  {
    get_screen_res(&xres, &yres);
  /*  disable_cursor (); */
    enable_cursor ();
    draw_rgb_cursor ();
    rc = set_cursor_pos (0, 0);
    enable_cursor ();

    if (rc == SUCCESS)
    {
      do
      {
        /* move cursor vertically */
        for(x = 0; x < xres && rc == SUCCESS; x += CURSOR_RES)
        {
          for(y = 0; y < yres && rc == SUCCESS; y += CURSOR_RES)
          {
            rc = set_cursor_pos (x, y);
            usleep(SLEEP_TIME);
          }
        }

        /* move cursor horizontally */
        for(y = 0; y < yres && rc == SUCCESS; y += CURSOR_RES)
        {
          for(x = 0; x < xres && rc == SUCCESS; x += CURSOR_RES)
          {
            rc = set_cursor_pos (x, y);
            usleep(SLEEP_TIME);
          }
        }
      } while(!end_tu(get_dply_time()) && rc == SUCCESS);
    } /* endif */
  } /* endif */

  return(rc);
}



/*
 * NAME : draw_rgb_cursor
 *
 * DESCRIPTION :
 *
 *  Draws a red/green/blue cursor in the cursor RAM
 *
 * INPUT PARAMETERS :
 *
 *  None.
 *
 * OUTPUT
 *
 *  None.
 *
 * RETURNS:
 *
 *  None.
 *
*/

#define BITS_PER_PIXEL             2
#define PIXELS_IN_BYTE             (BITS_IN_BYTE / BITS_PER_PIXEL)

void draw_rgb_cursor (void)
{
  ULONG  y, x, ltemp, hwbug, lcolor, lindex, hindex, index;
  int rc, num_pixels_per_color, i;

  TITLE ("draw_rgb_cursor");

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
  for (i = 0; i < NUM_CURSOR_COLORS; i++)
  {
    cursor_colors [i] = color_regs[i].reg;
  } /* endfor */

  num_pixels_per_color = CURSOR_RES / 2;         /* for 2 colors width       */

  /* set the color for first half of the cursor (32x128 pixels) with         */
  /* cursor color register 1 & 0 (RED & BLACK)                               */
  index = 0x100;
  for(y = 0; y < CURSOR_RES / 2; y++)
  {
    lcolor = cursor_colors[1];
    lcolor |= (lcolor << 6) | (lcolor << 4) | (lcolor << 2);
    lcolor |= (lcolor << 24) | (lcolor << 16) | (lcolor << 8);
    for (x = 0; x < num_pixels_per_color / PIXELS_IN_BYTE; x++)
      {
      lindex = index & 0xff;
      lindex |= (lindex << 24) | (lindex << 16) | (lindex << 8);
      hindex = (index >> 8) & 0xff;
      hindex |= (hindex << 24) | (hindex << 16) | (hindex << 8);
      dac_workaround();
      hwbug = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
      WL(lindex, W9100_INDEXLOW);
      dac_workaround();
      hwbug = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
      WL(hindex, W9100_INDEXHIGH);
      dac_workaround();
      hwbug = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
      WL(lcolor, W9100_INDEXDATA);
      index++;
      }

    lcolor = cursor_colors[0];
    lcolor |= (lcolor << 6) | (lcolor << 4) | (lcolor << 2);
    lcolor |= (lcolor << 24) | (lcolor << 16) | (lcolor << 8);
    for (x = 0; x < num_pixels_per_color / PIXELS_IN_BYTE; x++)
      {
      lindex = index & 0xff;
      lindex |= (lindex << 24) | (lindex << 16) | (lindex << 8);
      hindex = (index >> 8) & 0xff;
      hindex |= (hindex << 24) | (hindex << 16) | (hindex << 8);
      dac_workaround();
      hwbug = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
      WL(lindex, W9100_INDEXLOW);
      dac_workaround();
      hwbug = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
      WL(hindex, W9100_INDEXHIGH);
      dac_workaround();
      hwbug = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
      WL(lcolor, W9100_INDEXDATA);
      index++;
      }
  }

  /* set the color for second half of the cursor (32x128 pixels)             */
  /* with cursor color register 2 & 3 (GREEN & BLUE)                         */
  for(y = CURSOR_RES / 2; y < CURSOR_RES; y++)
  {
    lcolor = cursor_colors[2];
    lcolor |= (lcolor << 6) | (lcolor << 4) | (lcolor << 2);
    lcolor |= (lcolor << 24) | (lcolor << 16) | (lcolor << 8);
    for (x = 0; x < num_pixels_per_color / PIXELS_IN_BYTE; x++)
      {
      lindex = index & 0xff;
      lindex |= (lindex << 24) | (lindex << 16) | (lindex << 8);
      hindex = (index >> 8) & 0xff;
      hindex |= (hindex << 24) | (hindex << 16) | (hindex << 8);
      dac_workaround();
      hwbug = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
      WL(lindex, W9100_INDEXLOW);
      dac_workaround();
      hwbug = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
      WL(hindex, W9100_INDEXHIGH);
      dac_workaround();
      hwbug = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
      WL(lcolor, W9100_INDEXDATA);
      index++;
      }

    lcolor = cursor_colors[3];
    lcolor |= (lcolor << 6) | (lcolor << 4) | (lcolor << 2);
    lcolor |= (lcolor << 24) | (lcolor << 16) | (lcolor << 8);
    for (x = 0; x < num_pixels_per_color / PIXELS_IN_BYTE; x++)
      {
      lindex = index & 0xff;
      lindex |= (lindex << 24) | (lindex << 16) | (lindex << 8);
      hindex = (index >> 8) & 0xff;
      hindex |= (hindex << 24) | (hindex << 16) | (hindex << 8);
      dac_workaround();
      hwbug = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
      WL(lindex, W9100_INDEXLOW);
      dac_workaround();
      hwbug = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
      WL(hindex, W9100_INDEXHIGH);
      dac_workaround();
      hwbug = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
      WL(lcolor, W9100_INDEXDATA);
      index++;
      }
  } /* endfor */

  return;
}



/*
 * NAME : fill_cursor_ram_color
 *
 * DESCRIPTION :
 *
 *  Paint the entire cursor with color
 *
 * INPUT PARAMETERS :
 *
 *  Cursor color
 *
 * OUTPUT
 *
 *  None.
 *
 * RETURNS:
 *
 *  None.
 *
*/

static void fill_cursor_ram_color (UCHAR color)
{
  ULONG lcolor, hwbug, lindex, hindex, index;
  int i;

  TITLE("fill_cursor_ram_color");

  lcolor = color;
  /* Spread color across byte. Two bits per cursor pixel. */
  lcolor |= (lcolor << 6) | (lcolor << 4) | (lcolor << 2);
  /* Spread byte across long. 9100 bug workaraound. */
  lcolor |= (lcolor << 24) | (lcolor << 16) | (lcolor << 8);
  index = 0x100;
  for (i = 0; i <= CURSOR_RAM_SIZE_IN_BYTE; i++)
  {
    /* Auto index not working or not enabled. Manual increment implemented */
    lindex = index & 0xff;
    lindex |= (lindex << 24) | (lindex << 16) | (lindex << 8);
    hindex = (index >> 8) & 0xff;
    hindex |= (hindex << 24) | (hindex << 16) | (hindex << 8);
    dac_workaround();
    hwbug = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
    WL(lindex, W9100_INDEXLOW);
    dac_workaround();
    hwbug = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
    WL(hindex, W9100_INDEXHIGH);
    dac_workaround();
    hwbug = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
    WL(lcolor, W9100_INDEXDATA);
    index++;
  }

  return;
}



/*
 * NAME : set_cursor_pos
 *
 * DESCRIPTION :
 *
 *  Sets cursor position.  The cursor position x & y values are
 *  calculated as follow:
 *    xpos = desired display screen (xpos) position + [64 or 32]
 *    ypos = desired display screen (ypos) position + [64 or 32]
 *
 *  '64' is used if 64x64 cursor is active, '32' if 32x32 cursor.
 *
 * INPUT :
 *
 *   1. X coordinate.
 *   2. Y coordinate.
 *
 * OUTPUT :
 *
 *   Cursor position register updated.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS;
 *
*/

int set_cursor_pos(ULONG xpos, ULONG ypos)
{
  ULONG xval, yval, x, y, offset;
  UCHAR tempc;
  int     rc;

  TITLE("set_cursor_pos");

  rc = SET_CURSOR_ERR;

  /* choose correct offset value based on current cursor size */

  tempc = ReadIBM525(RGB525_CURSOR_CTL);

  offset = (ULONG) ((tempc && RGB525_CURSOR_SIZE_MASK) == RGB525_CURSOR_SIZE_64x64) ? 64 : 32;

  /* calculate the cursor x value */
  x = xpos + offset;

  /* calculated the cursor y value */
  y = ypos + offset;

  /* start with cursor x low register, we write x & y coordinate to RAMDAC */
  WriteIBM525((UCHAR) RGB525_CURSOR_X_LOW,  (UCHAR) x       ) ; /* Do X_LOW  */
  WriteIBM525((UCHAR) RGB525_CURSOR_X_HIGH, (UCHAR) (x >> 8)) ; /* Do X_HIGH */
  WriteIBM525((UCHAR) RGB525_CURSOR_Y_LOW,  (UCHAR) y       ) ; /* Do Y_LOW  */
  WriteIBM525((UCHAR) RGB525_CURSOR_Y_HIGH, (UCHAR) (y >> 8)) ; /* Do Y_HIGH */

  get_cursor_pos (&xval, &yval, offset);
  rc = ((xval == xpos) && (yval == ypos)) ? SUCCESS : SET_CURSOR_ERR;

  return (rc);
}



/*
 * NAME : get_cursor_pos
 *
 * DESCRIPTION :
 *
 *  Get cursor position.  The cursor position x & y values are
 *  calculated as follow:
 *    xpos = displayed screen (xpos) position + [32 or 64]
 *    ypos = displayed screen (ypos) position + [32 or 64]
 *
 *  '64' is used if 64x64 cursor is active, '32' if 32x32 cursor.
 *
 * INPUT :
 *
 * 1. Address of X coordinate variable
 * 2. Address of Y coordinate variable
 *
 * OUTPUT :
 *
 * 1. X coordinate.
 * 2. Y coordinate.
 *
 * RETURNS :
 *
 *   None.
 *
*/

void get_cursor_pos (ULONG *xpos, ULONG *ypos, ULONG offset)
{
  UCHAR  x_low, x_high, y_low, y_high;

  TITLE ("get_cursor_pos");

  /* start with cursor x low register, we read x & y coordinate from RAMDAC */
  x_low  = ReadIBM525((UCHAR) RGB525_CURSOR_X_LOW);
  x_high = ReadIBM525((UCHAR) RGB525_CURSOR_X_HIGH);
  y_low  = ReadIBM525((UCHAR) RGB525_CURSOR_Y_LOW);
  y_high = ReadIBM525((UCHAR) RGB525_CURSOR_Y_HIGH);

  *xpos = ((x_high << 8) + x_low) - offset;
  *ypos = ((y_high << 8) + y_low) - offset;

  return;
}

