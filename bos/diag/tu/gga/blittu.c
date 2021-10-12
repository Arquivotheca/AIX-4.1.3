static char sccsid[] = "@(#)62	1.1  src/bos/diag/tu/gga/blittu.c, tu_gla, bos41J, 9515A_all 4/6/95 09:26:42";
/*
 *   COMPONENT_NAME: tu_gla
 *
 *   FUNCTIONS: blit_tu
 *		move_to
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
#include "ggamisc.h"
#include "tu_type.h"


static void move_to (ULONG *, ULONG *, ULONG *, ULONG *,
                     BOX *, ULONG, ULONG);

/*
 * NAME : blit_tu
 *
 * DESCRIPTION :
 *
 *   This function is used to exercise the BLIT command of the GGA.  The
 *   function will display a yellow square box on the center of the screen,
 *   copy (blit) the image to another random location and then clear out
 *   the previous image.
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

#define BOX_SIZE                     64
#define SLEEP_TIME                   (100 * 1000)           /* in usec       */

int blit_tu (void)
{
  BOX     box;
  ULONG xres, yres, minterms, raster, x1, y1, x2, y2, status;
  int     rc;
#ifdef LOGMSGS
  char    msg[80];
#endif

  TITLE("bitblt_tu");

  if ((rc = clear_screen ()) == SUCCESS)
  {
    get_screen_res (&xres, &yres);
    /* display a yellow square box in the middle of the screen               */
    box.xstart = (xres - BOX_SIZE) / 2;
    box.ystart = (yres - BOX_SIZE) / 2;
    box.xend   = box.xstart + BOX_SIZE;
    box.yend   = box.ystart + BOX_SIZE;
    box.color  = YELLOW;
    rc = draw_box (&box);

    if (rc == SUCCESS)
    {
      do
      {
        /* change the raster operation to use the image's color              */
        raster = IGC_S_MASK | OVER_SIZED;
        wait_for_wtkn_ready ();
        write_igc_reg ((UCHAR) RASTER_REG, raster);

        /* calculate random location to display                              */
        move_to (&x1, &y1, &x2, &y2, &box, xres, yres);

        /* setting up the blit registers for the area to be blit             */
        igc_write_2D_coord_reg (IGM_ABS, IGM_X, 0,
                                 (ULONG) uitrunc ((double) box.xstart));
        igc_write_2D_coord_reg (IGM_ABS, IGM_Y, 0,
                                  (ULONG) uitrunc ((double) box.ystart));
        igc_write_2D_coord_reg (IGM_ABS, IGM_X, 1,
                                  (ULONG) uitrunc ((double) box.xend));
        igc_write_2D_coord_reg (IGM_ABS, IGM_Y, 1,
                                  (ULONG) uitrunc ((double) box.yend));

        /* setting up the coordinates for the new location                   */
        igc_write_2D_coord_reg (IGM_ABS, IGM_X, 2, x1);
        igc_write_2D_coord_reg (IGM_ABS, IGM_Y, 2, y1);
        igc_write_2D_coord_reg (IGM_ABS, IGM_X, 3, x2);
        igc_write_2D_coord_reg (IGM_ABS, IGM_Y, 3, y2);

        igc_do_blit ();

        get_igc_reg ((UCHAR) STATUS_REG, &status);
        if (status & BLIT_EXCEPTIONS)
        {
#ifdef LOGMSGS
          sprintf (msg, "Blit Exception = 0x%08x", status);
          LOG_MSG (msg);
#endif
          rc = BLIT_CMD_FAILED;
        } /* endif */

        if (rc == SUCCESS)
        {
          /* restore the raster to use foreground mask for drawing           */
          raster = IGC_F_MASK | OVER_SIZED;
          wait_for_wtkn_ready ();
          write_igc_reg ((UCHAR) RASTER_REG, raster);

          box.color = BLACK;
          rc = draw_box (&box);                  /* clear out previous image */
          if (rc == SUCCESS)
          {                                      /* save new location so we  */
            box.xstart = x1;                     /* can clear out next time  */
            box.ystart = y1;
            box.xend   = x2;
            box.yend   = y2;

            usleep (SLEEP_TIME);                 /* slow down the GGA        */
          } /* endif */
          else
            rc = CLEAR_IMAGE_ERR;
        } /* endif */
      }
      while((!end_tu(get_dply_time())) && (rc == SUCCESS)) ;
    } /* endif */
  } /* endif */

  return(rc);
}


/*
 * NAME : move_to
 *
 * DESCRIPTION :
 *
 *   Find a next suitable location (randomly) on the screen.
 *
 * INPUT PARAMETERS :
 *
 *   1) pointer to new position for x1
 *   2) pointer to new position for y1
 *   3) pointer to new position for x2
 *   4) pointer to new position for y2
 *   5) pointer to a structure of a box image
 *   6) x screen resolution
 *   7) y screen resolution
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
static void move_to (ULONG *x1, ULONG *y1, ULONG *x2, ULONG *y2,
                     BOX *box, ULONG xres, ULONG yres)
{
  TITLE("move_to");

  do
  {
    do
    {
      *x1 = random () % xres;                    /* get another random loc.  */
      *y1 = random () % yres;                    /* get another random loc.  */
    }            /* making sure the new image will not overlay the old image */
    while ((*x1 >= box-> xstart && *x1 <= box->xend) ||
           (*x1 + BOX_SIZE >= box->xstart && *x1 + BOX_SIZE <= box->xend) ||
           (*y1 >= box-> ystart && *y1 <= box->yend) ||
           (*y1 + BOX_SIZE >= box->ystart && *y1 + BOX_SIZE <= box->yend));

    *x2 = *x1 + BOX_SIZE;
    *y2 = *y1 + BOX_SIZE;
  }                               /* continue to loop until not out of bound */
  while (*x2 >= xres || *y2 >= yres);

  return;
}

