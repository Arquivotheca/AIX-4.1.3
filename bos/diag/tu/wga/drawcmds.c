static char sccsid[] = "@(#)68  1.2  src/bos/diag/tu/wga/drawcmds.c, tu_wga, bos411, 9428A410j 4/23/93 15:28:38";
/*
 *   COMPONENT_NAME: TU_WGA
 *
 *   FUNCTIONS: draw_box
 *              draw_line
 *              draw_point
 *              draw_quad
 *              draw_triangle
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
  ulong_t packed_xy, status;
  ulong_t x, y;
  int     i, rc;
#ifdef LOGMSGS
  char    msg[80];
#endif

  TITLE ("draw_box");
  rc = SUCCESS;
  wait_for_wtkn_ready ();                        /* wait for WTKN chip ready */
  set_foreground_color ((uchar_t) box -> color);

  /* set up xy coordinate for top left corner                                */
  x = (ulong_t) uitrunc ((double)box -> xstart);
  y = (ulong_t) uitrunc ((double)box -> ystart);
  packed_xy = IGM_PACK (x, y);
  igc_write_2D_meta_coord (IGM_RECT, IGM_ABS, IGM_XY, packed_xy);

  /* set up xy coordinate for bottom right corner                            */
  x = (ulong_t) uitrunc ((double)box -> xend);
  y = (ulong_t) uitrunc ((double)box -> yend);

  packed_xy = IGM_PACK (x, y);
  igc_write_2D_meta_coord (IGM_RECT, IGM_ABS, IGM_XY, packed_xy);

  igc_draw_quad ();                              /* initiate QUAD draw cmd   */

  /* check the status of draw QUAD command                                   */
  get_igc_reg ((uchar_t) STATUS_REG, &status);
  if (status & QUAD_EXCEPTIONS)
  {
#ifdef LOGMSGS
    sprintf (msg, "Draw box exception (QUAD command) = 0x%08x",
                                 status & QUAD_EXCEPTIONS);
    LOG_MSG (msg);
#endif
    rc = QUAD_CMD_FAILED;
  } /* endif */

#ifdef DEBUG_WGA
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
  ulong_t packed_xy, status;
  int     i, rc;
#ifdef LOGMSGS
  char    msg[80];
#endif

  TITLE ("draw_quad");

  rc = SUCCESS;
  wait_for_wtkn_ready ();                        /* wait for WTKN chip ready */
  set_foreground_color ((uchar_t) quadralateral -> color);

  /* set up for all 4 coordinates                                            */
  for (i = 0; i < 4; i++)
  {
    packed_xy = IGM_PACK ((ulong_t) uitrunc ((double) quadralateral -> x[i]),
                          (ulong_t) uitrunc ((double) quadralateral -> y[i]));
    igc_write_2D_meta_coord (IGM_QUAD, IGM_ABS, IGM_XY, packed_xy);
  } /* endfor */

  igc_draw_quad ();                              /* initiate QUAD draw cmd   */

  get_igc_reg ((uchar_t) STATUS_REG, &status);
  if (status & QUAD_EXCEPTIONS)
  {
#ifdef LOGMSGS
    sprintf (msg, "Draw quad exception (QUAD command) = 0x%08x",
                                 status & QUAD_EXCEPTIONS);
    LOG_MSG (msg);
#endif
    rc = QUAD_CMD_FAILED;
  } /* endif */

#ifdef DEBUG_WGA
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
  ulong_t packed_xy, status;
  int     i, rc;
#ifdef LOGMSGS
  char    msg[80];
#endif

  TITLE ("draw_triangle");

  rc = SUCCESS;
  wait_for_wtkn_ready ();                        /* wait for WTKN chip ready */
  set_foreground_color ((uchar_t) triangle -> color);

  /* set up for all 3 coordinates                                            */
  for (i = 0; i < 3; i++)
  {
    packed_xy = IGM_PACK ((ulong_t) uitrunc ((double) triangle -> x[i]),
                          (ulong_t) uitrunc ((double) triangle -> y[i]));
    igc_write_2D_meta_coord (IGM_TRIANGLE, IGM_ABS, IGM_XY, packed_xy);
  } /* endfor */

  igc_draw_quad ();                              /* initiate QUAD draw cmd   */

  get_igc_reg ((uchar_t) STATUS_REG, &status);
  if (status & QUAD_EXCEPTIONS)
  {
#ifdef LOGMSGS
    sprintf (msg, "Draw triangle exception (QUAD command) = 0x%08x",
                                       status & QUAD_EXCEPTIONS);
    LOG_MSG (msg);
#endif
    rc = QUAD_CMD_FAILED;
  } /* endif */

#ifdef DEBUG_WGA
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
  ulong_t packed_xy, status;
  int     rc;
#ifdef LOGMSGS
  char    msg[80];
#endif

  TITLE ("draw_point");

  rc = SUCCESS;
  wait_for_wtkn_ready ();                        /* wait for WTKN chip ready */
  set_foreground_color ((uchar_t) point -> color);

  packed_xy = IGM_PACK ((ulong_t) uitrunc ((double) point -> x),
                        (ulong_t) uitrunc ((double) point -> y));
  igc_write_2D_meta_coord (IGM_POINT, IGM_ABS, IGM_XY, packed_xy);

  igc_draw_quad ();                              /* initiate QUAD draw cmd   */

  get_igc_reg ((uchar_t) STATUS_REG, &status);
  if (status & QUAD_EXCEPTIONS)
  {
#ifdef LOGMSGS
    sprintf (msg, "Draw point exception (QUAD command) = 0x%08x",
                                       status & QUAD_EXCEPTIONS);
    LOG_MSG (msg);
#endif
    rc = QUAD_CMD_FAILED;
  } /* endif */

#ifdef DEBUG_WGA
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
  ulong_t packed_xy, status;
  int     rc;
#ifdef LOGMSGS
  char    msg[80];
#endif

  TITLE ("draw_line");

  rc = SUCCESS;
  wait_for_wtkn_ready ();                        /* wait for WTKN chip ready */
  set_foreground_color ((uchar_t) line -> color);

  /* initialize the first vertex                                             */
  packed_xy = IGM_PACK ((ulong_t) uitrunc ((double) line -> x1),
                        (ulong_t) uitrunc ((double) line -> y1));
  igc_write_2D_meta_coord (IGM_LINE, IGM_ABS, IGM_XY, packed_xy);

  /* initialize the second vertex                                            */
  packed_xy = IGM_PACK ((ulong_t) uitrunc ((double) line -> x2),
                        (ulong_t) uitrunc ((double) line -> y2));
  igc_write_2D_meta_coord (IGM_LINE, IGM_ABS, IGM_XY, packed_xy);

  igc_draw_quad ();                              /* initiate QUAD draw cmd   */

  get_igc_reg ((uchar_t) STATUS_REG, &status);
  if (status & QUAD_EXCEPTIONS)
  {
#ifdef LOGMSGS
    sprintf (msg, "Draw line exception (QUAD command) = 0x%08x",
                                       status & QUAD_EXCEPTIONS);
    LOG_MSG (msg);
#endif
    rc = QUAD_CMD_FAILED;
  } /* endif */

#ifdef DEBUG_WGA
  printf ("\nDraw line status = 0x%08x", status);  fflush(stdout);
#endif

  return (rc);
}
