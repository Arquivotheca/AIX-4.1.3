static char sccsid[] = "@(#)86	1.1  src/bos/diag/tu/gga/rgbtu.c, tu_gla, bos41J, 9515A_all 4/6/95 09:27:24";
/*
 *   COMPONENT_NAME: tu_gla
 *
 *   FUNCTIONS: get_full_xy
 *		get_part_xy
 *		rgb_tu
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

#include "exectu.h"
#include "tu_type.h"
#include "ggamisc.h"




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
  ULONG xres, yres, ydelta;
  BOX     box;
  int     rc = SUCCESS;
  UCHAR i;
  UCHAR color_attr [3] = { RED, GREEN, BLUE };

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

  if (rc == SUCCESS)
    while(!end_tu(get_dply_time()));

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

ULONG get_full_xy (ULONG data)
{
  ULONG i, pixels, cw;

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

ULONG get_part_xy (ULONG data, ULONG pixels)
{
  ULONG i, cw, tmp, max_pixels;

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
