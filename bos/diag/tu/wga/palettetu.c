static char sccsid[] = "@(#)74  1.3  src/bos/diag/tu/wga/palettetu.c, tu_wga, bos411, 9428A410j 1/3/94 17:22:28";
/*
 *   COMPONENT_NAME: TU_WGA
 *
 *   FUNCTIONS: black_tu
 *              blue_tu
 *              clear_screen
 *              fill_palette
 *              full_color
 *              green_tu
 *              palette_tu
 *              red_tu
 *              verify_ram
 *              white_tu
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

#include "wgareg.h"
#include "exectu.h"
#include "tu_type.h"
#include "wgamisc.h"
#include "wga_reg.h"
#include "wga_regval.h"



/*
 * NAME : palette_tu
 *
 * DESCRIPTION :
 *
 *  This TU performs several functions such as write, read and verify
 *  to test the color palette RAM contained in the BT 459 RAMDAC.
 *  It performs 2 different kind of tests.  The first test run for 256
 *  times.  Durring each iteration of the first test, it increments
 *  each color of the palette RAM color by 1.  After each iteration,
 *  the palette RAM will be read and verified.  By incrementing the
 *  palette RAM colors, the screen will gradually change from black
 *  to white.  For the second test, a decay test is performed by
 *  waiting for approx. 5 secs before reading and verifying the palette
 *  RAM.  THis test will execute for 1 time.
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
 *   Error code or SUCCESS.
 *
*/

#define PAL_LINES   256            /* Palette RAM words. (256 x 24 bit words) */

static BOOL verify_ram(ulong_t *, int);


int palette_tu(void)
{
  ulong_t scratch[PAL_LINES];
  int rc, i, j;

  rc = SUCCESS;

  for (j = 0; rc == SUCCESS && j < PAL_LINES; j++)
  {
    /* write the entire palette RAM with color (j)                           */
    load_dac_reg ((ulong_t) COLOR_PALETTE_RAM_OFFSET);
    for(i = 0; i < PAL_LINES; i++)
    {
      scratch[i] = j;
      write_igc_reg ((uchar_t)COLOR_PALETTE_RAM, scratch [i]);
      write_igc_reg ((uchar_t)COLOR_PALETTE_RAM, scratch [i]);
      write_igc_reg ((uchar_t)COLOR_PALETTE_RAM, scratch [i]);
    }

    /* Now, read and verify the palette RAM                                  */
    if(!verify_ram(&scratch[0], PRAM_RW_ERR))
      rc = PRAM_RW_ERR;

  }

  if(rc == SUCCESS)
  {
    /* Decay test */
    load_dac_reg ((ulong_t) COLOR_PALETTE_RAM_OFFSET);
    for(i = 0; i < PAL_LINES; i++)
    {
      scratch [i] = 0x7F;
      write_igc_reg ((uchar_t)COLOR_PALETTE_RAM, scratch [i]);
      write_igc_reg ((uchar_t)COLOR_PALETTE_RAM, scratch [i]);
      write_igc_reg ((uchar_t)COLOR_PALETTE_RAM, scratch [i]);
    }

    sleep (DECAY_MEMORY_TIME);

    if(!verify_ram(&scratch[0], PRAM_DECAY_ERR))
      rc = PRAM_DECAY_ERR;
  }

  /* now, we must re-initialize the color table for other TUs                */
  color_init ();

  return(rc);
}



/*
 * NAME : verify_ram
 *
 * DESCRIPTION :
 *
 *  Reads palette RAM and compares it with input buffer.
 *
 * INPUT :
 *
 *   1. Reference buffer.
 *   2. Error code if verification fails.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   TRUE if data compared OK, else FALSE.
 *
*/


static BOOL verify_ram(ulong_t *ref, int err_code)
{
  int i;
  BOOL rc;
  ulong_t red, green, blue;

  rc = TRUE;

  load_dac_reg ((ulong_t) COLOR_PALETTE_RAM_OFFSET);
  for(i = 0; i < PAL_LINES && rc; i++)
  {
    get_igc_reg ((uchar_t) COLOR_PALETTE_RAM, &red);
    get_igc_reg ((uchar_t) COLOR_PALETTE_RAM, &green);
    get_igc_reg ((uchar_t) COLOR_PALETTE_RAM, &blue);

    /* verify the data */
    if (ref[i] != red)
    {
      /* set up the address that failed the test                             */
      set_mem_info((ulong_t) i, ref[i], red, err_code, "verify palette RAM", "");
      rc = FALSE;
    }
    else
      if (ref[i] != green)
      {
        /* set up the address that failed the test                           */
        set_mem_info((ulong_t) i, ref[i], green, err_code, "verify palette RAM", "");
        rc = FALSE;
      }
      else
        if (ref[i] != blue)
        {
          /* set up the address that failed the test                         */
          set_mem_info((ulong_t) i, ref[i], blue, err_code, "verify palette RAM", "");
          rc = FALSE;
        }
  }

  return(rc);
}




/*
 * NAME : black_tu
 *
 * DESCRIPTION :
 *
 *  Clears screen to black.
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
 *   Error code or SUCCESS.
 *
*/

int black_tu(void)
{
  int  rc;

  TITLE("black_tu");

  rc = full_color((uchar_t) BLACK);
  rc = (rc == SUCCESS || rc == SCREEN_RES_ERR) ? rc : SET_FOREGROUND_ERR;

  return (rc);
}



/*
 * NAME : white_tu
 *
 * DESCRIPTION :
 *
 *  Clears screen to white.
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
 *   Error code or SUCCESS.
 *
*/

int white_tu(void)
{
  int  rc;

  TITLE("white_tu");

  rc = full_color((uchar_t) WHITE);
  rc = (rc == SUCCESS || rc == SCREEN_RES_ERR) ? rc : SET_FOREGROUND_ERR;

  return (rc);
}



/*
 * NAME : red_tu
 *
 * DESCRIPTION :
 *
 *  Clears screen to red.
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
 *   Error code or SUCCESS.
 *
*/

int red_tu(void)
{
  int  rc;

  TITLE("red_tu");

  rc = full_color((uchar_t) RED);
  rc = (rc == SUCCESS || rc == SCREEN_RES_ERR) ? rc : SET_FOREGROUND_ERR;

  return (rc);
}



/*
 * NAME : green_tu
 *
 * DESCRIPTION :
 *
 *  Clears screen to green.
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
 *   Error code or SUCCESS.
 *
*/

int green_tu(void)
{
  int  rc;

  TITLE("green_tu");

  rc = full_color((uchar_t) GREEN);
  rc = (rc == SUCCESS || rc == SCREEN_RES_ERR) ? rc : SET_FOREGROUND_ERR;

  return (rc);
}




/*
 * NAME : blue_tu
 *
 * DESCRIPTION :
 *
 *  Clears screen to blue.
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
 *   Error code or SUCCESS.
 *
*/

int blue_tu(void)
{
  int  rc;

  TITLE("blue_tu");

  rc = full_color((uchar_t) BLUE);
  rc = (rc == SUCCESS || rc == SCREEN_RES_ERR) ? rc : SET_FOREGROUND_ERR;

  return (rc);
}



/*
 * NAME : full_color
 *
 * DESCRIPTION :
 *
 *  Displays a solid color as specified by input parameters.
 *
 * INPUT :
 *
 *    color intensity.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS.
 *
*/

int full_color (uchar_t color)
{
  ulong_t status, packed_xy, xres, yres;
  BOX     box;
  int     rc;

  TITLE("full_color");

  get_screen_res (&xres, &yres);
  set_wclip (0, 0, xres - 1, yres - 1);          /* make it a full screen    */

  box.xstart = box.ystart = (float) 0;
  box.xend = (float) xres - 1;
  box.yend = (float) yres - 1;
  box.color = color;

  rc = draw_box (&box);                          /* display the box on scr.  */


  return (rc);
}



/*
 * NAME : clear_screen
 *
 * DESCRIPTION :
 *
 * Clears video to black.  
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
 *   Error code or SUCCESS
 *
*/

int clear_screen(void)
{
  int      rc;
  BOX      box;
  ulong_t  xres, yres;

  TITLE("clear_screen");

  if (in_graphics())                                  /* for GRAPHICS mode only        */
  {
    rc = full_color ((uchar_t) BLACK);    /* clear the screen to black*/
    if ((rc != SUCCESS) && (rc != SCREEN_RES_ERR))
      rc = CLEAR_BKGROUND_ERR;
  }

  return (rc);
}



/*
 * NAME : fill_palette
 *
 * DESCRIPTION :
 *
 *  Initializes palette.
 *
 * INPUT PARAMETERS :
 *
 *  1. red intensity
 *  2. green intensity
 *  3. blue intensity
 *  4. from palette line
 *  5. to palette line
 *  6. setup the address for palette RAM ? (boolean)
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

void fill_palette (ulong_t red, ulong_t green, ulong_t blue,
                   uchar_t start_line, uchar_t end_line, BOOL set_up)
{
  ulong_t addr;
  ulong_t i;

  TITLE("fill_palette");

  start_line %= PAL_LINES;                       /* make sure of inputs      */
  end_line %= PAL_LINES;

  if (set_up)
    load_dac_reg ((ulong_t) COLOR_PALETTE_RAM_OFFSET + (ulong_t) start_line);

  for (i = start_line; i <= end_line; i++)
  {
    write_igc_reg ((uchar_t)COLOR_PALETTE_RAM, red);
    write_igc_reg ((uchar_t)COLOR_PALETTE_RAM, green);
    write_igc_reg ((uchar_t)COLOR_PALETTE_RAM, blue);
  }

  return;
}



/*
*   NAME: color_full_vram
*
*    DESCRIPTION:
*
*        Writes BLACK(0) to 2048 X 1024 VRAM. 
*
*    INPUT PARAMETERS :
*
*        None.
*
*    OUTPUT
*
*       None.
*
*    RETURNS :
*
*      Error code or SUCCESS
*
*/

int color_full_vram(uchar_t color)

  {
    BOX   box;
    int   rc ;

    TITLE ("full_screen");
    set_wclip (0,0,(MAX_PHYSICAL_PIX_PER_LINE-1),(MAX_PHYSICAL_SCAN_LINES-1));

    box.xstart = box.ystart = (float) 0;
    box.xend = (float) MAX_PHYSICAL_PIX_PER_LINE - 1;
    box.yend = (float) MAX_PHYSICAL_SCAN_LINES - 1;
    box.color = color;

    rc = draw_box(&box);
    return(rc);
  }

   
