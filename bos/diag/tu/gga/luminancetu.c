static char sccsid[] = "@(#)78	1.1  src/bos/diag/tu/gga/luminancetu.c, tu_gla, bos41J, 9515A_all 4/6/95 09:27:10";
/*
 *   COMPONENT_NAME: tu_gla
 *
 *   FUNCTIONS: display_pattern
 *		luminance_fall_off_tu
 *		luminance_tu_1
 *		luminance_tu_10
 *		luminance_tu_11
 *		luminance_tu_12
 *		luminance_tu_2
 *		luminance_tu_3
 *		luminance_tu_4
 *		luminance_tu_5
 *		luminance_tu_6
 *		luminance_tu_7
 *		luminance_tu_8
 *		luminance_tu_9
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



#define DAYTONA_WIDTH      300                   /* mm                       */
#define DAYTONA_HEIGHT     225

/* This measures (mm) were taken from reference screen (approx.) */
#define BOX_DIMENSION      50                    /* default 50mm in width    */


static int display_pattern (BOOL, UCHAR, UCHAR, UCHAR);


/*
 * NAME : luminance_fall_off_tu
 *
 * DESCRIPTION :
 *
 *   Luminance Fall Off tu.  Display 9 square boxes on the screen
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

#define NO_BOXES   3

int luminance_fall_off_tu (void)
{
  ULONG xres, yres, i, j;
  float   width, height, xdelta, ydelta, x, y;
  BOX     box;
  int     rc;

  if((rc = clear_screen ()) == SUCCESS)
  {
   /* scale factors : pixels / mm                                            */
    get_screen_res(&xres, &yres);

    /* width & height are in pixels                                          */
    width = ((float) xres / DAYTONA_WIDTH) * BOX_DIMENSION;
    height = ((float) yres / DAYTONA_HEIGHT) * BOX_DIMENSION;
    xdelta = (xres - (NO_BOXES * width)) / (NO_BOXES + 1);
    ydelta = (yres - (NO_BOXES * height)) / (NO_BOXES + 1);
    xdelta = (xdelta < 0.0) ? 0.0 : xdelta;
    ydelta = (ydelta < 0.0) ? 0.0 : ydelta;

    box.color = WHITE;

    /* display 9 white boxes on the screen                                   */
    for (y = ydelta, i = 0; rc == SUCCESS && i < NO_BOXES;
                                 i++, y += ydelta + height)
    {
      box.ystart = y;
      box.yend = box.ystart + height;
      for (x = xdelta, j = 0; rc == SUCCESS && j < NO_BOXES;
                                 j++, x += xdelta + width)
      {
        box.xstart = x;
        box.xend = box.xstart + width;
        rc = draw_box (&box);
      }
    }

    if (rc == SUCCESS)
      while(!end_tu(get_dply_time()));           /* wait for a key pressed   */
  }

  return(rc);
}


/*
 * NAME : luminance_tu_1
 *
 * DESCRIPTION :
 *
 *   Display a vertical pattern of X0 where X = pixel on, 0 = pixel off
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
 *  Error code or SUCCESS
 *
*/

int luminance_tu_1 (void)
{
  return (display_pattern (TRUE, (UCHAR) GREEN, (UCHAR) 0xAA, (UCHAR) 8));
}



/*
 * NAME : luminance_tu_2
 *
 * DESCRIPTION :
 *
 *   Display a horizontal pattern of X0 where X = pixel on, 0 = pixel off
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
 *  Error code or SUCCESS
 *
*/

int luminance_tu_2 (void)
{
  return (display_pattern (FALSE, (UCHAR) GREEN, (UCHAR) 0xAA, (UCHAR) 8));
}



/*
 * NAME : luminance_tu_3
 *
 * DESCRIPTION :
 *
 *   Display a vertical pattern of XX0 where X = pixel on, 0 = pixel off
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
 *  Error code or SUCCESS
 *
*/

int luminance_tu_3 (void)
{
  return (display_pattern (TRUE, (UCHAR) GREEN, (UCHAR) 0xD8, (UCHAR) 6));
}



/*
 * NAME : luminance_tu_4
 *
 * DESCRIPTION :
 *
 *   Display a horizontal pattern of XX0 where X = pixel on, 0 = pixel off
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
 *  Error code or SUCCESS
 *
*/

int luminance_tu_4 (void)
{
  return (display_pattern (FALSE, (UCHAR) GREEN, (UCHAR) 0xD8, (UCHAR) 6));
}



/*
 * NAME : luminance_tu_5
 *
 * DESCRIPTION :
 *
 *   Display a vertical pattern of X00 where X = pixel on, 0 = pixel off
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
 *  Error code or SUCCESS
 *
*/

int luminance_tu_5 (void)
{
  return (display_pattern (TRUE, (UCHAR) GREEN, (UCHAR) 0x90, (UCHAR) 6));
}



/*
 * NAME : luminance_tu_6
 *
 * DESCRIPTION :
 *
 *   Display a horizontal pattern of X00 where X = pixel on, 0 = pixel off
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
 *  Error code or SUCCESS
 *
*/

int luminance_tu_6 (void)
{
  return (display_pattern (FALSE, (UCHAR) GREEN, (UCHAR) 0x90, (UCHAR) 6));
}



/*
 * NAME : luminance_tu_7
 *
 * DESCRIPTION :
 *
 *   Display a vertical pattern of XX00 where X = pixel on, 0 = pixel off
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
 *  Error code or SUCCESS
 *
*/

int luminance_tu_7 (void)
{
  return (display_pattern (TRUE, (UCHAR) GREEN, (UCHAR) 0xCC, (UCHAR) 8));
}



/*
 * NAME : luminance_tu_8
 *
 * DESCRIPTION :
 *
 *   Display a horizontal pattern of XX00 where X = pixel on, 0 = pixel off
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
 *  Error code or SUCCESS
 *
*/

int luminance_tu_8 (void)
{
  return (display_pattern (FALSE, (UCHAR) GREEN, (UCHAR) 0xCC, (UCHAR) 8));
}



/*
 * NAME : luminance_tu_9
 *
 * DESCRIPTION :
 *
 *   Display a vertical pattern of X0XX0 where X = pixel on, 0 = pixel off
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
 *  Error code or SUCCESS
 *
*/

int luminance_tu_9 (void)
{
  return (display_pattern (TRUE, (UCHAR) GREEN, (UCHAR) 0xB0, (UCHAR) 5));
}



/*
 * NAME : luminance_tu_10
 *
 * DESCRIPTION :
 *
 *   Display a horizontal pattern of X0XX0 where X = pixel on, 0 = pixel off
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
 *  Error Code or SUCCESS
 *
*/

int luminance_tu_10 (void)
{
  return (display_pattern (FALSE, (UCHAR) GREEN, (UCHAR) 0xB0, (UCHAR) 5));
}



/*
 * NAME : luminance_tu_11
 *
 * DESCRIPTION :
 *
 *   Display a vertical pattern of X00X0 where X = pixel on, 0 = pixel off
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
 *  Error code or SUCCESS
 *
*/

int luminance_tu_11 (void)
{
  return (display_pattern (TRUE, (UCHAR) GREEN, (UCHAR) 0x90, (UCHAR) 5));
}



/*
 * NAME : luminance_tu_12
 *
 * DESCRIPTION :
 *
 *   Display a horizontal pattern of X00X0 where X = pixel on, 0 = pixel off
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
 *  Error code or SUCCESS
 *
*/

int luminance_tu_12 (void)
{
  return (display_pattern (FALSE, (UCHAR) GREEN, (UCHAR) 0x90, (UCHAR) 5));
}



/*
 * NAME : display_pattern
 *
 * DESCRIPTION :
 *
 *  Displays luminance patterns.
 *
 * INPUT PARAMETERS :
 *  1) vertical / horizontal drawing (boolean)
 *  2) line color
 *  3) 8 bits pattern to be displayed
 *  4) number of bits to check in the pattern
 *
 * OUTPUT
 *
 *  None.
 *
 * RETURNS:
 *
 *  Error code or SUCCESS
 *
 * NOTE:
 *  The caller MUST packed the pattern into a left justify string (UCHAR
 *  type).  If the pattern doesn't fit into a UCHAR, then 0s should be
 *  packed at the end of the string.  Otherwise, multiple patterns should
 *  be packed into the string.
*/

static int display_pattern (BOOL vertical, UCHAR color,
                            UCHAR input_pattern, UCHAR pattern_size)
{
  int     x, y, rc;
  UCHAR i, pattern;
  ULONG xres, yres;
  LINE    line;

  if ((rc = clear_screen ()) == SUCCESS)
  {
    get_screen_res (&xres, &yres);
    line.color = color;

    if (vertical)                                /* for vertical pattern     */
    {
      line.y1 = 0;       line.y2 = yres - 1;
      for (x = 0; rc == SUCCESS && x < xres - 1;)
      {                                          /* loop for entire screen   */
        pattern = input_pattern;
        for (i = 0; rc == SUCCESS && x < xres - 1 && i < pattern_size; i++, x++)
        {                                        /* loop for each pattern    */
          if (pattern & 0x80)                    /* draw line if bit is on   */
          {
            line.x1 = line.x2 = x;
            rc = draw_line (&line);
          } /* endif */
          pattern <<= 1;                         /* shifting the next bit up */
        } /* endfor */
      } /* endfor */
    }
    else                                         /* for horizontal pattern   */
    {
      line.x1 = 0;       line.x2 = xres - 1;
      for (y = 0; rc == SUCCESS && y < yres - 1;)
      {                                          /* loop for entire screen   */
        pattern = input_pattern;
        for (i = 0; rc == SUCCESS && y < yres - 1 && i < pattern_size; i++, y++)
        {                                        /* loop for each pattern    */
          if (pattern & 0x80)                    /* draw line if bit is on   */
          {
            line.y1 = line.y2 = y;
            rc = draw_line (&line);
          } /* endif */
          pattern <<= 1;                         /* shifting the next bit up */
        } /* endfor */
      } /* endfor */
    } /* endif */

    if (rc == SUCCESS)
      while(!end_tu(get_dply_time()));           /* wait for a key pressed   */
  } /* endif */

  return (rc);
}

