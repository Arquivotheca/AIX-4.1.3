static char sccsid[] = "@(#)67  1.3  src/bos/diag/tu/wga/cursortu.c, tu_wga, bos411, 9428A410j 1/3/94 17:14:40";
/*
 *   COMPONENT_NAME: TU_WGA
 *
 *   FUNCTIONS: cursor_ram_tu
 *              dply_cursor_tu
 *              draw_rgb_cursor
 *              full_cursor_ram_color
 *              get_cursor_color
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

#define CURSOR_RAM_SIZE              0x100       /* cursor RAM size in words */
#define GET_RAND_CHAR                ((uchar_t) ((rand() / MAX_RAND) * (double) 255))
#define CURSOR_RES                   64          /* cursor RAM is 64 * 64 * 2*/
#define CURSOR_RAM_SIZE_IN_BYTE      0x3FF
#define ITERATION                    100




static ulong_t cursor_colors [NUM_CURSOR_COLOR_REGS];
static struct
{
  uchar_t red, green, blue;
  uchar_t reg;
} color_regs [NUM_CURSOR_COLOR_REGS] =
             {
                0x00, 0x00, 0x00, CURSOR_TRANS_COLOR,   /* transparent color */
                0xff, 0x00, 0x00, CURCOL1_REG,     /* color reg 1 = red      */
                0x00, 0xff, 0x00, CURCOL2_REG,     /* color reg 2 = green    */
                0x00, 0x00, 0xff, CURCOL3_REG,     /* color reg 3 = blue     */
              };


static void full_cursor_ram_color (ulong_t);


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
   ulong_t   scratch[CURSOR_RAM_SIZE], i;
   ulong_t   data, xres, yres, color, addr;
   uchar_t   tmp_char;
   static BOOL initialize = FALSE;

ulong_t  x, y;


   if (!initialize)
   {
     /* setting up the cursor color registers (RGB)                          */
     load_dac_reg ((ulong_t) CURSOR_COLOR_REGISTERS_OFFSET);
     for(i = 1; i < NUM_CURSOR_COLOR_REGS; i++)
     {
        write_igc_reg ((uchar_t) CURCMND_REG, (ulong_t) color_regs [i].red);
        write_igc_reg ((uchar_t) CURCMND_REG, (ulong_t) color_regs [i].green);
        write_igc_reg ((uchar_t) CURCMND_REG, (ulong_t) color_regs [i].blue);
     }  /* end for */

     initialize = TRUE;                          /* not to execute it again  */
   } /* endif */

   if ((rc = clear_screen ()) == SUCCESS)
   {
     enable_cursor ();
     if ((get_cursor_status () & CURSOR_ON) == CURSOR_ON)
     {
       get_screen_res (&xres, &yres);
       for (j = 0; j < ITERATION && rc == SUCCESS; j++)
       {
         for (i = 0; i < NUM_CURSOR_COLOR_REGS && rc == SUCCESS; i++)
         {

x = GET_RAND_ULONG % xres;
y = GET_RAND_ULONG % yres;
rc = set_cursor_pos (x, y);

/*           rc = set_cursor_pos (GET_RAND_ULONG % xres, GET_RAND_ULONG % yres); */
           if (rc == SUCCESS)
           {
             color = get_cursor_color (i);
             full_cursor_ram_color (color);      /* write the test pattern   */

             /* verify the cursor RAM with the pattern just wrote            */
             load_dac_reg ((ulong_t) CURSOR_RAM_OFFSET);
             for (k = 0; k <= CURSOR_RAM_SIZE_IN_BYTE && rc == SUCCESS; k++)
             {
                get_igc_reg ((uchar_t) CURCMND_REG, &data);  /* get 4 pixels */
                if (data != (color & 0xFF))      /*cmp last 8 bits for RAMDAC*/
                {
                  rc = CRAM_RW_ERR;
                  addr = CURSOR_RAM_OFFSET + k;
                  set_mem_info (addr, color & 0xFF, data, rc, "", "");
                } /* endif */
             } /* endfor */
           } /* endif */

else
{
  printf ("\nCursor position  x = %d     y = %d    j = %d    i = %d", x, y, j, i);
}

         } /* endfor */
       } /* endfor */

       if (rc == SUCCESS)
       {
         /* Second test, decay test. Write all 1's, delay, and then verify   */
         /* the entire cursor RAM                                            */
         color = 0xffffffff;                 /* color to fill cursor RAM     */

         /* fill up the entire cursor RAM with color                         */
         full_cursor_ram_color (color);

         sleep (DECAY_MEMORY_TIME);              /* check for possible decay */

         /* Read and verify the entire cursor RAM */
         load_dac_reg ((ulong_t) CURSOR_RAM_OFFSET);
         for(i = 0; i < CURSOR_RAM_SIZE && rc == SUCCESS; i++)
         {
           get_igc_reg ((uchar_t) CURCMND_REG, &data);
           if (data != (color & 0xFF))           /* if does not match then   */
           {                                     /* set return code and addr.*/
             rc = CRAM_DECAY_ERR;
             addr = CURSOR_RAM_OFFSET + i;
             set_mem_info (addr, color & 0xFF, data, rc, "", "");
           }
         }
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
  ulong_t  x, y, xres, yres;
  int      rc;

  TITLE ("dply_cursor_tu");

  if ((rc = clear_screen ()) == SUCCESS)
  {
    get_screen_res(&xres, &yres);
    disable_cursor ();
    draw_rgb_cursor ();
    rc = set_cursor_pos (0, 0);
    enable_cursor ();

    if (rc == SUCCESS)
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
  ulong_t  y, x;
  int rc, num_pixels_per_color, i;
  static  BOOL initialize = FALSE;               /* default, NOT init. yet   */


  TITLE ("draw_rgb_cursor");

  if (!initialize)
  {
    /* setting up the cursor color registers (RGB)                           */
    load_dac_reg ((ulong_t) CURSOR_COLOR_REGISTERS_OFFSET);
    for(i = 1; i < NUM_CURSOR_COLOR_REGS; i++)
    {
      write_igc_reg ((uchar_t) CURCMND_REG, (ulong_t) color_regs [i].red);
      write_igc_reg ((uchar_t) CURCMND_REG, (ulong_t) color_regs [i].green);
      write_igc_reg ((uchar_t) CURCMND_REG, (ulong_t) color_regs [i].blue);
    }  /* end for */

    /* initialize all colors for the cursor                                  */
    for (i = 0; i < NUM_CURSOR_COLOR_REGS; i++)
    {
      cursor_colors [i] = get_cursor_color (color_regs [i].reg);
    } /* endfor */

    initialize = TRUE;                           /* DO NOT initialize again  */
  } /* end if */

  num_pixels_per_color = CURSOR_RES / 2;         /* for 2 colors width       */

  /* cursor image is represented as 64x64 pixels. Each pixel is represented  */
  /* by 2 bits.  So we can map the cursor image as 64x128 pixels.            */

  /* set the color for first half of the cursor (32x128 pixels) with         */
  /* cursor color register 1 & 0 (RED & BLACK)                               */
  load_dac_reg ((ulong_t) CURSOR_RAM_OFFSET);    /* load cursor RAM address  */
  for(y = 0; y < CURSOR_RES / 2; y++)
  {
    for (x = 0; x < num_pixels_per_color / PIXELS_IN_BYTE; x++)
    {
      write_igc_reg ((uchar_t) CURCMND_REG, cursor_colors [1]);
    } /* endfor */

    for (x = 0; x < num_pixels_per_color / PIXELS_IN_BYTE; x++)
    {
      write_igc_reg ((uchar_t) CURCMND_REG, cursor_colors [0]);
    } /* endfor */
  }

  /* set the color for second half of the cursor (32x128 pixels)             */
  /* with cursor color register 1 & 2 (GREEN & BLACK)                        */
  for(; y < CURSOR_RES; y++)
  {
    for (x = 0; x < num_pixels_per_color / PIXELS_IN_BYTE; x++)
    {
      write_igc_reg ((uchar_t) CURCMND_REG, cursor_colors [2]);
    } /* endfor */

    for (x = 0; x < num_pixels_per_color / PIXELS_IN_BYTE; x++)
    {
      write_igc_reg ((uchar_t) CURCMND_REG, cursor_colors [3]);
    } /* endfor */
  } /* endfor */

  return;
}



/*
 * NAME : get_cursor_color
 *
 * DESCRIPTION :
 *
 *  Returns a word of cursor color formatted data.
 *
 * INPUT PARAMETERS :
 *
 *  1. Cursor color.
 *
 * OUTPUT
 *
 *  None.
 *
 * RETURNS:
 *
 *  Word of cursor color formatted data.
 *
*/

#define CURSOR_PIXEL_RES         2

ulong_t get_cursor_color(ulong_t color)
{
  ulong_t i, cc;

  cc = 0;
  color &= 0x3;

  for(i = 1; i <= BITS_IN_WORD / CURSOR_PIXEL_RES; i++)
  {
    cc |= color;
    color <<= CURSOR_PIXEL_RES;
  }

  return(cc);
}



/*
 * NAME : full_cursor_ram_color
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

static void full_cursor_ram_color (ulong_t color)
{
  int i;
  TITLE("full_cursor_ram_color");

  load_dac_reg ((ulong_t) CURSOR_RAM_OFFSET);    /* load cursor RAM address  */
  for (i = 0; i <= CURSOR_RAM_SIZE_IN_BYTE; i++)
  {
    write_igc_reg ((uchar_t) CURCMND_REG, color);          /* write 4 pixels */
  }

  return;
}
