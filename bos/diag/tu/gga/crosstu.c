static char sccsid[] = "@(#)64	1.1  src/bos/diag/tu/gga/crosstu.c, tu_gla, bos41J, 9515A_all 4/6/95 09:26:45";
/*
 *   COMPONENT_NAME: tu_gla
 *
 *   FUNCTIONS: bw128_tu
 *		bw160_tu
 *		bw64_tu
 *		bw9x11_dots_tu
 *		cross_hatch
 *		dot_patterns
 *		wb128_tu
 *		wb160_tu
 *		wb64_tu
 *		wb9x11_dots_tu
 *		wb9x11_tu
 *		wb9x9_tu
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


typedef struct
{
  UCHAR bkg_color;
  UCHAR line_color;
  ULONG delta_x;
  ULONG delta_y;
  BOOL draw_cursor;
  ULONG x_cursor_pos;
  ULONG y_cursor_pos;

} CROSS_HATCH_PARMS;


static int cross_hatch(CROSS_HATCH_PARMS *);
static int dot_patterns(CROSS_HATCH_PARMS *);


/*
 * NAME : bw64_tu
 *
 * DESCRIPTION :
 *
 * Displays a pattern consisting of black lines drawn every 64
 * pixels, vertically and  horizontally on a white background.
 * A transparent/red/green/blue 64x64 cursor is placed at the
 * (64,64) location (one square from the top, one square from the left).
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

int bw64_tu(void)
{
  CROSS_HATCH_PARMS cp;

  cp.bkg_color = WHITE;
  cp.line_color = BLACK;
  cp.draw_cursor = TRUE;
  cp.delta_x = cp.delta_y = 64;
  cp.x_cursor_pos = cp.y_cursor_pos = 93;

  return (cross_hatch(&cp));
}


/*
 * NAME : wb64_tu
 *
 * DESCRIPTION :
 *
 * Displays a pattern consisting of white lines drawn every 64
 * pixels, vertically and  horizontally on a black background.
 * A transparent/red/green/blue 64x64 cursor is placed at the
 * (64,64) location (one square from the top, one square from the left).
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

int wb64_tu(void)
{
  CROSS_HATCH_PARMS cp;

  cp.bkg_color = BLACK;
  cp.line_color = WHITE;
  cp.draw_cursor = TRUE;
  cp.delta_x = cp.delta_y = 64;
  cp.x_cursor_pos = cp.y_cursor_pos = 93;

  return(cross_hatch(&cp));
}


/*
 * NAME : bw128_tu
 *
 * DESCRIPTION :
 *
 * Displays a pattern consisting of black lines drawn every 128
 * pixels, vertically and  horizontally on a white background.
 * A transparent/red/green/blue 64x64 cursor is placed at the
 * (64,64) location.
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

int bw128_tu(void)
{
  CROSS_HATCH_PARMS cp;

  cp.bkg_color = WHITE;
  cp.line_color = BLACK;
  cp.delta_x = cp.delta_y = 128;
  cp.draw_cursor = TRUE;
  cp.x_cursor_pos = cp.y_cursor_pos = 160;

  return(cross_hatch(&cp));
}


/*
 * NAME : wb128_tu
 *
 * DESCRIPTION :
 *
 * Displays a pattern consisting of white lines drawn every 128
 * pixels, vertically and  horizontally on a back background.
 * A transparent/red/green/blue 64x64 cursor is placed at the
 * (64,64) location (one square from the top, one square from the left).
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

int wb128_tu(void)
{
  CROSS_HATCH_PARMS cp;

  cp.bkg_color = BLACK;
  cp.line_color = WHITE;
  cp.delta_x = cp.delta_y = 128;
  cp.draw_cursor = TRUE;
  cp.x_cursor_pos = cp.y_cursor_pos = 160;

  return(cross_hatch(&cp));
}


/*
 * NAME : bw160_tu
 *
 * DESCRIPTION :
 *
 * Displays a pattern consisting of black lines drawn every 160
 * pixels horizontally, and 128 pixels vertically on a white background.
 * A transparent/red/green/blue 64x64 cursor is placed at the
 * (160,128) location.
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

int bw160_tu(void)
{
  CROSS_HATCH_PARMS cp;

  cp.bkg_color = WHITE;
  cp.line_color = BLACK;
  cp.delta_x = 160;
  cp.delta_y = 128;
  cp.draw_cursor = TRUE;
  cp.x_cursor_pos = 192;
  cp.y_cursor_pos = 160;

  return(cross_hatch(&cp));
}


/*
 * NAME : wb160_tu
 *
 * DESCRIPTION :
 *
 * Displays a pattern consisting of white lines drawn every 160
 * pixels horizontally, and 128 pixels vertically on a black background.
 * A transparent/red/green/blue 64x64 cursor is placed at the
 * (160,128) location.
 *
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

int wb160_tu(void)
{
  CROSS_HATCH_PARMS cp;

  cp.bkg_color = BLACK;
  cp.line_color = WHITE;
  cp.delta_x = 160;
  cp.delta_y = 128;
  cp.draw_cursor = TRUE;
  cp.x_cursor_pos = 192;
  cp.y_cursor_pos = 160;

  return(cross_hatch(&cp));
}



/*
 * NAME : wb9x9_tu
 *
 * DESCRIPTION :
 *
 *  A grid is drawn of 9 horizontal x 9 vertical white lines on a
 *  black background. Cursor is disabled.
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

int wb9x9_tu(void)
{
  CROSS_HATCH_PARMS cp;
  ULONG xres, yres;

  get_screen_res(&xres, &yres);
  cp.bkg_color = BLACK;
  cp.line_color = WHITE;
  cp.delta_x = xres / 8;                         /* 9 vertical (9 - 1)       */
  cp.delta_y = yres / 8;                         /* 9 horizontal (9 - 1)     */
  cp.draw_cursor = FALSE;

  return(cross_hatch(&cp));
}


/*
 * NAME : wb9x11_tu
 *
 * DESCRIPTION :
 *
 *  A grid is drawn of 9 horizontal x 11 vertical white lines on a
 *  black background. Cursor is disabled.
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

int wb9x11_tu(void)
{
  CROSS_HATCH_PARMS cp;
  ULONG xres, yres;

  get_screen_res(&xres, &yres);
  cp.bkg_color = BLACK;
  cp.line_color = WHITE;
  cp.delta_x = xres / 10;                        /* 11 vertical (11 - 1)     */
  cp.delta_y = yres / 8;                         /* 9 horizontal (9 - 1)     */
  cp.draw_cursor = FALSE;

  return(cross_hatch(&cp));
}


/*
 * NAME : wb9x11_dots_tu
 *
 * DESCRIPTION :
 *
 *  A grid is drawn of 9 horizontal x 11 vertical white dots on a
 *  black background. Cursor is disabled.
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

int wb9x11_dots_tu(void)
{
  CROSS_HATCH_PARMS cp;
  ULONG xres, yres;

  get_screen_res(&xres, &yres);
  cp.bkg_color = BLACK;
  cp.line_color = WHITE;
  cp.delta_x = xres / 8;                         /* except the last dot      */
  cp.delta_y = yres / 10;                        /* except the last dot      */
  cp.draw_cursor = FALSE;

  return(dot_patterns(&cp));
}


/*
 * NAME : bw9x11_dots_tu
 *
 * DESCRIPTION :
 *
 *  A grid is drawn of 9 horizontal x 11 vertical black dots on a
 *  white background. Cursor is disabled.
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

int bw9x11_dots_tu(void)
{
  CROSS_HATCH_PARMS cp;
  ULONG xres, yres;

  get_screen_res(&xres, &yres);
  cp.bkg_color = WHITE;
  cp.line_color = BLACK;
  cp.delta_x = xres / 8;                         /* except the last dot      */
  cp.delta_y = yres / 10;                        /* except the last dot      */
  cp.draw_cursor = FALSE;

  return(dot_patterns(&cp));
}



/*
 * NAME : cross_hatch
 *
 * DESCRIPTION :
 *
 *  Draws a cross-hatch pattern as specified by input parameters.
 *
 * INPUT PARAMETERS :
 *
 *  1. Pointer to CROSS_HATCH_PARMS struct.
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

int cross_hatch(CROSS_HATCH_PARMS *cp)
{
  ULONG x, y, xres, yres;
  LINE    line;
  int     rc;

  TITLE ("cross_hatch");

  rc = full_color (cp-> bkg_color, FALSE);
  if (rc == SUCCESS)
  {
    get_screen_res (&xres, &yres);
    set_wclip (0, 0, xres - 1, yres - 1);

    /* draw horizontal lines except last one:  */
    line.x1 = 0;           line.x2 = xres - 1;
    line.color = cp->line_color;
    for(y = 0; rc == SUCCESS && y <= (yres - cp->delta_y); y += cp->delta_y)
    {
      line.y1 = line.y2 = y;
      rc = draw_line (&line);
    }

    /* draw last horizontal line */
    if (rc == SUCCESS)
    {
      line.y1 = line.y2 = yres - 1;
      rc = draw_line (&line);
    } /* endif */

    /* draw vertical lines except last one:     */
    line.y1 = 0;           line.y2 = yres - 1;
    for(x = 0; rc == SUCCESS && x <= (xres - cp->delta_x); x += cp->delta_x)
    {
      line.x1 = line.x2 = x;
      rc = draw_line (&line);
    }

    /* draw last vertical line */
    if (rc == SUCCESS)
    {
      line.x1 = line.x2 = xres - 1;
      rc = draw_line (&line);
    } /* endif */


    if(rc == SUCCESS && cp->draw_cursor)
    {
      disable_cursor ();
      rc = set_cursor_pos (cp->x_cursor_pos, cp->y_cursor_pos);
      if (rc == SUCCESS)
        draw_rgb_cursor ();
      enable_cursor ();
    }

    if (rc == SUCCESS)
      while(!end_tu(get_dply_time()));
  }
  else
    rc = CLEAR_BKGROUND_ERR;

  return(rc);
}



/*
 * NAME : dot_patterns
 *
 * DESCRIPTION :
 *
 *  Draws a dot pattern as specified by input parameters.
 *
 * INPUT PARAMETERS :
 *
 *  1. Pointer to CROSS_HATCH_PARMS struct.
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

int dot_patterns(CROSS_HATCH_PARMS *cp)
{
  POINT   point;
  ULONG x, y, xres, yres;
  int     rc;

  TITLE ("dot_patterns");

  rc = full_color (cp-> bkg_color, FALSE);
  if (rc == SUCCESS)
  {
    get_screen_res (&xres, &yres);
    set_wclip (0, 0, xres - 1, yres - 1);

    /* draw horizontal dots except the last line.                            */
    point.color = cp -> line_color;
    for(y = 0; rc == SUCCESS && y <= (yres - cp-> delta_y); y += cp-> delta_y)
    {
      point.y  = y;
      for (x = 0; rc == SUCCESS && x <= xres - cp-> delta_x; x += cp-> delta_x)
      {
        point.x  = x;
        rc = draw_point (&point);
      } /* endfor */

      /* complete the last point on the line                                 */
      if (rc == SUCCESS)
      {
        point.x = xres - 1;                      /* last location on a line  */
        point.y = y;
        rc = draw_point (&point);
      }
    }

    /* draw the last line                                                    */
    point.y = yres - 1;                          /* last location on a line  */
    for (x = 0; rc == SUCCESS && x <= xres - cp-> delta_x; x += cp-> delta_x)
    {
      point.x = x;
      rc = draw_point (&point);
    } /* endfor */

    /* draw the last point of the last line                                  */
    point.x = xres - 1;                          /* last location on a line  */
    if (rc == SUCCESS)
    {
      rc = draw_point (&point);
    } /* endif */

    /* if requested to draw a cursor, then do it                             */
    if(rc == SUCCESS && cp->draw_cursor)
    {
      disable_cursor ();
      rc = set_cursor_pos (cp->x_cursor_pos, cp->y_cursor_pos);
      if (rc == SUCCESS)
        draw_rgb_cursor ();
      enable_cursor ();
    }

    if (rc == SUCCESS)
      while(!end_tu(get_dply_time()));
  }
  else
    rc = CLEAR_BKGROUND_ERR;

  return (rc);
}
