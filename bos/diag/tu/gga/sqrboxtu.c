static char sccsid[] = "@(#)90	1.1  src/bos/diag/tu/gga/sqrboxtu.c, tu_gla, bos41J, 9515A_all 4/6/95 09:27:30";
/*
 *   COMPONENT_NAME: tu_gla
 *
 *   FUNCTIONS: sqr_box_50mm_tu
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



/*
 * NAME : sqr_box_50mm_tu
 *
 * DESCRIPTION :
 *   Display a 50mm square box, white on black on the center
 *   of the screen.
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

int sqr_box_50mm_tu(void)
{
  ULONG xres, yres;
  float   width, height;
  BOX     box;
  int     rc;

  TITLE("sqr_box_50mm_tu");

  if ((rc = clear_screen ()) == SUCCESS)
  {
    get_screen_res(&xres, &yres);

    /* width & height are in pixels                                          */
    width = ((float) xres / DAYTONA_WIDTH) * BOX_DIMENSION;
    height = ((float) yres / DAYTONA_HEIGHT) * BOX_DIMENSION;

    /* initialize the size and color for the box to be displayed             */
    box.xstart = ((float)xres - width) / 2.0;
    box.ystart = ((float)yres - height) / 2.0;
    box.xend   = box.xstart + width;
    box.yend   = box.ystart + height;
    box.color  = WHITE;
    rc = draw_box (&box);

    if (rc == SUCCESS)
      while(!end_tu(get_dply_time()));
  }

  return (rc);
}

