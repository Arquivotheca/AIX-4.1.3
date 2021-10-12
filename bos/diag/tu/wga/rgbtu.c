static char sccsid[] = "@(#)77  1.3  src/bos/diag/tu/wga/rgbtu.c, tu_wga, bos411, 9428A410j 1/3/94 17:27:13";
/*
 *   COMPONENT_NAME: TU_WGA
 *
 *   FUNCTIONS: get_full_xy
 *              get_part_xy
 *              rgb_tu
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

#include "exectu.h"
#include "tu_type.h"
#include "wgamisc.h"




/*
 * NAME : rgb_tu
 *
 * DESCRIPTION :
 *
 *  Clears the top 1/3 of the screen to red, the middle
 *  1/3 of the screen to green, and the bottom 1/3 of the screen to blue.
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

int rgb_tu(void)
{
  ulong_t xres, yres, ydelta;
  BOX     box;
  int     rc;
  uchar_t i;
  uchar_t color_attr [3] = { RED, GREEN, BLUE };

  TITLE ("rgb_tu");

  get_screen_res (&xres, &yres);
  ydelta = yres / 3;
  box.xstart = box.ystart = 0;
  box.xend = xres;
  box.yend = yres / 3;

  for (i = 0; rc == SUCCESS && i < 3; i++)
  {
    box.color = color_attr [i];
    rc = draw_box (&box);
    box.ystart = box.yend + 1;
    box.yend = box.ystart + ydelta;
  } /* endfor */


  return (rc);
}


/*
 * NAME : get_full_xy
 *
 * DESCRIPTION :
 *
 * Returns a word filled with data in FULL_W format.
 *
 * INPUT :
 *
 *  1. Input data
 *
 * OUTPUT :
 *
 *  None.
 *
 * RETURNS :
 *
 *  FULL_W formatted word
 *
*/

ulong_t get_full_xy (ulong_t data)
{
  ulong_t i, pixels, cw;

  pixels = BITS_IN_WORD / BPP;
  cw = data;

  for(i = 1; i <= pixels; i++)
  {
    cw <<= BPP;
    cw |= data;
  }

  return(cw);
}



/*
 * NAME : get_part_xy
 *
 * DESCRIPTION :
 *
 * Returns a word filled with data in PART_W format.
 *
 * INPUT :
 *
 *  1. Input data
 *  2. Number of pixels filled with input data
 *
 * OUTPUT :
 *
 *  None.
 *
 * RETURNS :
 *
 *  PART_W formatted word
 *
*/

ulong_t get_part_xy (ulong_t data, ulong_t pixels)
{
  ulong_t i, cw, tmp, max_pixels;

  max_pixels = (3 * BITS_IN_BYTE) / BPP;

  if(pixels > max_pixels)
    pixels = max_pixels;

  tmp = data << (BITS_IN_WORD - BPP);

  for(i = 1, cw = 0; i <= pixels; i++)
  {
    cw |= tmp;
    tmp >>= BPP;
  }

  return((cw & 0xffffff00) | (pixels - 1));
}
