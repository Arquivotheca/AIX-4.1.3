static char sccsid[] = "@(#)66	1.1  src/bos/diag/tu/gga/drawcmds.c, tu_gla, bos41J, 9515A_all 4/6/95 09:26:49";
/*
 *   COMPONENT_NAME: tu_gla
 *
 *   FUNCTIONS: draw_box
 *              draw_line
 *              draw_point
 *              draw_quad
 *              draw_triangle
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


/*
 * NAME : draw_box
 *
 * DESCRIPTION :
 *
 *  Draws a box as specified by input parameters.
 *
 * INPUT PARAMETERS :
 *
 *  Pointer to BOX structure.
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

int draw_box (BOX *box)
{
  ULONG packed_xy, status;
  ULONG x, y, color;
  int     i, rc;
#ifdef LOGMSGS
  char    msg[80];
#endif

  TITLE ("draw_box");
  rc = SUCCESS;
  wait_for_wtkn_ready ();                        /* wait for WTKN chip ready */

  color = gga_Get_Color(box->color);
  set_foreground_color(color);

  WL((ULONG) 0xFFFFFFFF, W9100_PLANE_MASK);
  wait_for_wtkn_ready();
  WL((FMASKOVER), W9100_RASTER);
  wait_for_wtkn_ready();

  /* set up xy coordinate for top left corner                                */
  x = (ULONG) uitrunc ((double)box -> xstart);
  y = (ULONG) uitrunc ((double)box -> ystart);
  packed_xy = IGM_PACK (x, y);
  igc_write_2D_meta_coord (IGM_RECT, IGM_ABS, IGM_XY, packed_xy);

  /* set up xy coordinate for bottom right corner                            */
  x = (ULONG) uitrunc ((double)box -> xend);
  y = (ULONG) uitrunc ((double)box -> yend);

  packed_xy = IGM_PACK (x, y);
  igc_write_2D_meta_coord (IGM_RECT, IGM_ABS, IGM_XY, packed_xy);

  igc_draw_quad ();                              /* initiate QUAD draw cmd   */

  /* check the status of draw QUAD command                                   */
  get_igc_reg ((UCHAR) STATUS_REG, &status);
  if (status & QUAD_EXCEPTIONS)
  {
#ifdef LOGMSGS
    sprintf (msg, "Draw box exception (QUAD command) = 0x%08x",
                                 status & QUAD_EXCEPTIONS);
    LOG_MSG (msg);
#endif
    rc = QUAD_CMD_FAILED;
  } /* endif */

#ifdef DEBUG_GGA
  printf ("\nDraw box status = 0x%08x", status);  fflush(stdout);
#endif

  return (rc);
}



/*
 * NAME : draw_quad
 *
 * DESCRIPTION :
 *
 *  Draws a quadrilateral as specified by input parameters.
 *
 * INPUT PARAMETERS :
 *
 *  Pointer to QUAD structure.
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

int draw_quad (QUAD *quadralateral)
{
  ULONG packed_xy, status, color;
  int     i, rc;
#ifdef LOGMSGS
  char    msg[80];
#endif

  TITLE ("draw_quad");

  rc = SUCCESS;
  wait_for_wtkn_ready ();                        /* wait for WTKN chip ready */

  color = gga_Get_Color(quadralateral->color);
  set_foreground_color(color);

  /* set up for all 4 coordinates                                            */
  for (i = 0; i < 4; i++)
  {
    packed_xy = IGM_PACK ((ULONG) uitrunc ((double) quadralateral -> x[i]),
                          (ULONG) uitrunc ((double) quadralateral -> y[i]));
    igc_write_2D_meta_coord (IGM_QUAD, IGM_ABS, IGM_XY, packed_xy);
  } /* endfor */

  igc_draw_quad ();                              /* initiate QUAD draw cmd   */

  get_igc_reg ((UCHAR) STATUS_REG, &status);
  if (status & QUAD_EXCEPTIONS)
  {
#ifdef LOGMSGS
    sprintf (msg, "Draw quad exception (QUAD command) = 0x%08x",
                                 status & QUAD_EXCEPTIONS);
    LOG_MSG (msg);
#endif
    rc = QUAD_CMD_FAILED;
  } /* endif */

#ifdef DEBUG_GGA
  printf ("\nDraw quad status = 0x%08x", status);  fflush(stdout);
#endif

  return (rc);
}



/*
 * NAME : draw_triangle
 *
 * DESCRIPTION :
 *
 *  Draws a triangle as specified by input parameters.
 *
 * INPUT PARAMETERS :
 *
 *  Pointer to TRIANGLE structure.
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

int draw_triangle (TRIANGLE *triangle)
{
  ULONG packed_xy, status, color;
  int     i, rc;
#ifdef LOGMSGS
  char    msg[80];
#endif

  TITLE ("draw_triangle");

  rc = SUCCESS;
  wait_for_wtkn_ready ();                        /* wait for WTKN chip ready */

  color = gga_Get_Color(triangle->color);
  set_foreground_color(color);

  /* set up for all 3 coordinates                                            */
  for (i = 0; i < 3; i++)
  {
    packed_xy = IGM_PACK ((ULONG) uitrunc ((double) triangle -> x[i]),
                          (ULONG) uitrunc ((double) triangle -> y[i]));
    igc_write_2D_meta_coord (IGM_TRIANGLE, IGM_ABS, IGM_XY, packed_xy);
  } /* endfor */

  igc_draw_quad ();                              /* initiate QUAD draw cmd   */

  get_igc_reg ((UCHAR) STATUS_REG, &status);
  if (status & QUAD_EXCEPTIONS)
  {
#ifdef LOGMSGS
    sprintf (msg, "Draw triangle exception (QUAD command) = 0x%08x",
                                       status & QUAD_EXCEPTIONS);
    LOG_MSG (msg);
#endif
    rc = QUAD_CMD_FAILED;
  } /* endif */

#ifdef DEBUG_GGA
  printf ("\nDraw triangle status = 0x%08x", status);  fflush(stdout);
#endif

  return (rc);
}



/*
 * NAME : draw_point
 *
 * DESCRIPTION :
 *
 *  Draws a point at (x, y) with color
 *
 * INPUT PARAMETERS :
 *
 *  Pointer to POINT structure
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

int draw_point (POINT *point)
{
  ULONG packed_xy, status, color;
  int     rc;
#ifdef LOGMSGS
  char    msg[80];
#endif

  TITLE ("draw_point");

  rc = SUCCESS;
  wait_for_wtkn_ready ();                        /* wait for WTKN chip ready */

  color = gga_Get_Color(point->color);
  set_foreground_color(color);

  packed_xy = IGM_PACK ((ULONG) uitrunc ((double) point -> x),
                        (ULONG) uitrunc ((double) point -> y));
  igc_write_2D_meta_coord (IGM_POINT, IGM_ABS, IGM_XY, packed_xy);

  igc_draw_quad ();                              /* initiate QUAD draw cmd   */

  get_igc_reg ((UCHAR) STATUS_REG, &status);
  if (status & QUAD_EXCEPTIONS)
  {
#ifdef LOGMSGS
    sprintf (msg, "Draw point exception (QUAD command) = 0x%08x",
                                       status & QUAD_EXCEPTIONS);
    LOG_MSG (msg);
#endif
    rc = QUAD_CMD_FAILED;
  } /* endif */

#ifdef DEBUG_GGA
  printf ("\nDraw point status = 0x%08x", status);  fflush(stdout);
#endif

  return (rc);
}




/*
 * NAME : draw_line
 *
 * DESCRIPTION :
 *
 *  Draws a line as specified by input parameters.
 *
 * INPUT PARAMETERS :
 *
 *  Pointer to LINE structure.
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

int draw_line (LINE *line)
{
  ULONG packed_xy, status, color;
  int     rc;
#ifdef LOGMSGS
  char    msg[80];
#endif

  TITLE ("draw_line");

  rc = SUCCESS;
  wait_for_wtkn_ready ();                        /* wait for WTKN chip ready */

  color = gga_Get_Color(line->color);
  set_foreground_color(color);

  /* initialize the first vertex                                             */
  packed_xy = IGM_PACK ((ULONG) uitrunc ((double) line -> x1),
                        (ULONG) uitrunc ((double) line -> y1));
  igc_write_2D_meta_coord (IGM_LINE, IGM_ABS, IGM_XY, packed_xy);

  /* initialize the second vertex                                            */
  packed_xy = IGM_PACK ((ULONG) uitrunc ((double) line -> x2),
                        (ULONG) uitrunc ((double) line -> y2));
  igc_write_2D_meta_coord (IGM_LINE, IGM_ABS, IGM_XY, packed_xy);

  igc_draw_quad ();                              /* initiate QUAD draw cmd   */

  get_igc_reg ((UCHAR) STATUS_REG, &status);
  if (status & QUAD_EXCEPTIONS)
  {
#ifdef LOGMSGS
    sprintf (msg, "Draw line exception (QUAD command) = 0x%08x",
                                       status & QUAD_EXCEPTIONS);
    LOG_MSG (msg);
#endif
    rc = QUAD_CMD_FAILED;
  } /* endif */

#ifdef DEBUG_GGA
  printf ("\nDraw line status = 0x%08x", status);  fflush(stdout);
#endif

  return (rc);
}
