static char sccsid[] = "@(#)82	1.1  src/bos/diag/tu/gga/palettetu.c, tu_gla, bos41J, 9515A_all 4/6/95 09:27:17";
/*
 *   COMPONENT_NAME: tu_gla
 *
 *   FUNCTIONS: black_tu
 *              blue_tu
 *              clear_screen
 *              color_full_vram
 *              fill_palette
 *              full_color
 *              green_tu
 *              palette_tu
 *              red_tu
 *              verify_ram
 *              white_tu
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
/* #include <sys/adspace.h> */ /*@ support for io_att() and io_det(), better */
                               /*@ known as PCI_ATTACH() and PCI_DETACH()    */
#include "ggareg.h"
#include "exectu.h"
#include "ggamisc.h"
#include "tu_type.h"

#define PAL_LINES   256            /* Palette RAM words. (256 x 24 bit words) */
#define RETRY_LIMIT 10

/* Local prototypes */
static BOOL verify_ram(UCHAR *, UCHAR *, UCHAR *, int);


int palettecheck_tu(void)
  {
    UCHAR rbit[3] = {0x49, 0x92, 0x24};
    UCHAR gbit[3] = {0x92, 0x24, 0x49};
    UCHAR bbit[3] = {0x24, 0x49, 0x92};
    ULONG value, bit_r, bit_g, bit_b, templ;
    int   retry, r_num_retries, g_num_retries, b_num_retries;
    int   i, j, rc=SUCCESS;

    for (i = 0 ; i <= 255 ; i++)  /* For each palette entry... */
      {
        for (j = 0 ; j < 3 ; j++)   /* For each of three test values... */
          {
            r_num_retries = 0;
            g_num_retries = 0;
            b_num_retries = 0;
            do                        /* Execute R/W test against each byte */
              {                       /* of the current palette entry...    */
                retry = FALSE;

                /* Set palette entry to write (three bytes) */
                value = (i) | (i << 8) | (i << 16) | (i << 24);
                templ = RL(FRAME_BUFFER+RAMDAC_HW_BUG);
                WL(value, W9100_PALCURRAMW);
                dac_workaround();

                /* Write to red byte of palette entry */
                value = rbit[j];
                value = (value) | (value << 8) | (value << 16) | (value << 24);
                templ = RL(FRAME_BUFFER+RAMDAC_HW_BUG);
                WL(value, W9100_PALDATA);
                dac_workaround();

                /* Write to green byte of palette entry */
                value = gbit[j];
                value = (value) | (value << 8) | (value << 16) | (value << 24);
                templ = RL(FRAME_BUFFER+RAMDAC_HW_BUG);
                WL(value, W9100_PALDATA);
                dac_workaround();

                /* Write to blue byte of palette entry */
                value = bbit[j];
                value = (value) | (value << 8) | (value << 16) | (value << 24);
                templ = RL(FRAME_BUFFER+RAMDAC_HW_BUG);
                WL(value, W9100_PALDATA);
                dac_workaround();


                /* Set palette entry to read (three bytes) */
                value = (i) | (i << 8) | (i << 16) | (i << 24);
                templ = RL(FRAME_BUFFER+RAMDAC_HW_BUG);
                WL(value, W9100_PALCURRAMR);
                dac_workaround();

                /* Read the red byte of palette entry */
                templ = RL(FRAME_BUFFER+RAMDAC_HW_BUG);
                bit_r = RL(W9100_PALDATA) >> 8;
                dac_workaround();

                /* Read the green byte of palette entry */
                templ = RL(FRAME_BUFFER+RAMDAC_HW_BUG);
                bit_g = RL(W9100_PALDATA) >> 8;
                dac_workaround();

                /* Read the blue byte of palette entry */
                templ = RL(FRAME_BUFFER+RAMDAC_HW_BUG);
                bit_b = RL(W9100_PALDATA) >> 8;
                dac_workaround();

                if (bit_r != rbit[j])
                  {
                    retry = TRUE;
                    r_num_retries++;
                  }
                if (bit_g != gbit[j])
                  {
                    retry = TRUE;
                    g_num_retries++;
                  }
                if (bit_b != bbit[j])
                  {
                    retry = TRUE;
                    b_num_retries++;
                  }
              } while ((retry == TRUE) && (r_num_retries < RETRY_LIMIT));

            /*** Check for retry warnings/errors ***/
            if (r_num_retries != 0)
              {
                if (r_num_retries >= RETRY_LIMIT)
                  {
                    rc = PRAM_RW_ERR;
                    set_mem_info ((ULONG) i, rbit[j], bit_r, rc, "Palette R/W error", "Red byte failed");
                  }
                else
                    set_mem_info ((ULONG) i, rbit[j], bit_r, rc, "Palette R/W warning", "Red byte had to retry");
              }
            if (g_num_retries != 0)
              {
                if (g_num_retries >= RETRY_LIMIT)
                  {
                    rc = PRAM_RW_ERR;
                    set_mem_info ((ULONG) i, gbit[j], bit_g, rc, "Palette R/W error", "Green byte failed");
                  }
                else
                    set_mem_info ((ULONG) i, gbit[j], bit_g, rc, "Palette R/W warning", "Green byte had to retry");
              }
            if (b_num_retries != 0)
              {
                if (b_num_retries >= RETRY_LIMIT)
                  {
                    rc = PRAM_RW_ERR;
                    set_mem_info ((ULONG) i, bbit[j], bit_b, rc, "Palette R/W error", "Blue byte failed");
                  }
                else
                    set_mem_info ((ULONG) i, bbit[j], bit_b, rc, "Palette R/W warning", "Blue byte had to retry");
              }
          }
      }

    /* now, we must re-initialize the color table for other TUs */
    color_init ();

    return(rc);
  }


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


int palette_tu(void)
{
  UCHAR rscratch[PAL_LINES], gscratch[PAL_LINES], bscratch[PAL_LINES];
  UCHAR pal_entry[3];
  int rc, i, j;

  rc = SUCCESS;

  for (j = 0; rc == SUCCESS && j < PAL_LINES; j++)
  {
    /* write the entire palette RAM with color (j)                           */
    for(i = 0; i < PAL_LINES; i++)
    {
      pal_entry[0] = rscratch[i] = j;
      pal_entry[1] = gscratch[i] = j;
      pal_entry[2] = bscratch[i] = j;
      WritePalette(i, 1, pal_entry);
    }

    /* Now, read and verify the palette RAM                                  */
    if(!verify_ram(&rscratch[0], &gscratch[0], &bscratch[0], PRAM_RW_ERR))
      rc = PRAM_RW_ERR;
  }

  for (j = 0; rc == SUCCESS && j < PAL_LINES; j++)
  {
    /* write the red palette RAM with color (j)                              */
    for(i = 0; i < PAL_LINES; i++)
    {
      pal_entry[0] = rscratch[i] = j;
      pal_entry[1] = gscratch[i] = 0;
      pal_entry[2] = bscratch[i] = 0;
      WritePalette(i, 1, pal_entry);
    }

    /* Now, read and verify the palette RAM                                  */
    if(!verify_ram(&rscratch[0], &gscratch[0], &bscratch[0], PRAM_RW_ERR))
      rc = PRAM_RW_ERR;
  }

  for (j = 0; rc == SUCCESS && j < PAL_LINES; j++)
  {
    /* write the green palette RAM with color (j)                            */
    for(i = 0; i < PAL_LINES; i++)
    {
      pal_entry[0] = rscratch[i] = 0;
      pal_entry[1] = gscratch[i] = j;
      pal_entry[2] = bscratch[i] = 0;
      WritePalette(i, 1, pal_entry);
    }

    /* Now, read and verify the palette RAM                                  */
    if(!verify_ram(&rscratch[0], &gscratch[0], &bscratch[0], PRAM_RW_ERR))
      rc = PRAM_RW_ERR;
  }

  for (j = 0; rc == SUCCESS && j < PAL_LINES; j++)
  {
    /* write the blue palette RAM with color (j)                             */
    for(i = 0; i < PAL_LINES; i++)
    {
      pal_entry[0] = rscratch[i] = 0;
      pal_entry[1] = gscratch[i] = 0;
      pal_entry[2] = bscratch[i] = j;
      WritePalette(i, 1, pal_entry);
    }

    /* Now, read and verify the palette RAM                                  */
    if(!verify_ram(&rscratch[0], &gscratch[0], &bscratch[0], PRAM_RW_ERR))
      rc = PRAM_RW_ERR;
  }

  if(rc == SUCCESS)
  {
    /* Decay test */
    for(i = 0; i < PAL_LINES; i++)
    {
      pal_entry[0] = rscratch [i] = 0x7F;
      pal_entry[1] = gscratch [i] = 0x7F;
      pal_entry[2] = bscratch [i] = 0x7F;
      WritePalette(i, 1, pal_entry);
    }

    sleep (DECAY_MEMORY_TIME);

    if(!verify_ram(&rscratch[0], &gscratch[0], &bscratch[0], PRAM_DECAY_ERR))
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


static BOOL verify_ram(UCHAR *rref, UCHAR *gref, UCHAR *bref, int err_code)
{
  int i;
  BOOL rc;
  UCHAR red, green, blue, pal_entry[3];

  rc = TRUE;

  for(i = 0; i < PAL_LINES && rc; i++)
  {
    ReadPalette(i, 1, pal_entry);
    red   = pal_entry[0];
    green = pal_entry[1];
    blue  = pal_entry[2];

    /* verify the data */
    if (rref[i] != red)
    {
      /* set up the address that failed the test                             */
      set_mem_info((ULONG) i, (ULONG) rref[i], (ULONG) red, err_code, "verify palette RAM", "");
      rc = FALSE;
    }
    else
      if (gref[i] != green)
      {
        /* set up the address that failed the test                           */
        set_mem_info((ULONG) i, (ULONG) gref[i], (ULONG) green, err_code, "verify palette RAM", "");
        rc = FALSE;
      }
      else
        if (bref[i] != blue)
        {
          /* set up the address that failed the test                         */
          set_mem_info((ULONG) i, (ULONG) bref[i], (ULONG) blue, err_code, "verify palette RAM", "");
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

  rc = full_color((UCHAR) BLACK, TRUE);
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

  rc = full_color((UCHAR) WHITE, TRUE);
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

  rc = full_color((UCHAR) RED, TRUE);
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

  rc = full_color((UCHAR) GREEN, TRUE);
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

  rc = full_color((UCHAR) BLUE, TRUE);
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
 *   1. color intensity.
 *   2. wait for keytouch ? (boolean)
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

#define DELAY_TIME 1

int full_color (UCHAR color, BOOL wait)
{
  ULONG status, packed_xy, xres, yres;
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

  if (rc == SUCCESS && wait)
    sleep (DELAY_TIME);
    /*@   OR   */
    /*@ while(!end_tu(get_dply_time())) ; */

  return (rc);
}



/*
 * NAME : clear_screen
 *
 * DESCRIPTION :
 *
 * Clears video to black for MOM mode.
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
  ULONG  xres, yres;

  TITLE("clear_screen");

  rc = full_color ((UCHAR) BLACK, FALSE);    /* clear the screen to black*/
  if ((rc != SUCCESS) && (rc != SCREEN_RES_ERR))
    rc = CLEAR_BKGROUND_ERR;

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

void fill_palette (UCHAR red, UCHAR green, UCHAR blue,
                   UCHAR start_line, UCHAR end_line)
{
  ULONG addr;
  ULONG i, x, num;
  UCHAR palette_data[768];

  TITLE("fill_palette");

  start_line %= PAL_LINES;                       /* make sure of inputs      */
  end_line %= PAL_LINES;
  num = (start_line - end_line) + 1;

  for (x=0, i=start_line; i<=end_line; x++, i++)
  {
    palette_data[x] = red;
    palette_data[x+1] = green;
    palette_data[x+2] = blue;
  }

  WritePalette((int) start_line, num, palette_data);

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

int color_full_vram (UCHAR color)

{
   BOX   box;
   int   rc ;

   TITLE ("full_screen");

   set_wclip (0,0,(MAX_PHYSICAL_PIX_PER_LINE-1),(MAX_PHYSICAL_SCAN_LINES-1));

   box.xstart = box.ystart = (float) 0;
   box.xend = (float) MAX_PHYSICAL_PIX_PER_LINE - 1;
   box.yend = (float) MAX_PHYSICAL_SCAN_LINES - 1;
   box.color = color;

   rc = draw_box (&box);

   return (rc);
}


