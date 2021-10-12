static char sccsid[] = "@(#)88	1.1  src/bos/diag/tu/gga/scrolltu.c, tu_gla, bos41J, 9515A_all 4/6/95 09:27:27";
/*
 *   COMPONENT_NAME: tu_gla
 *
 *   FUNCTIONS: scroll_gmode
 *		scroll_h_gmode_tu0
 *		scroll_h_gmode_tu1
 *		scroll_h_draw
 *		scroll_screen
 *		write_bottom_lines
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
#include <fcntl.h>
#include <errno.h>

#include "exectu.h"
#include "tu_type.h"
#include "ggamisc.h"

#define FOREVER     while(TRUE)

/* all dimensions in pixels */
#define VERT_SIZE_0           12
#define HOR_SIZE_0            8
#define VERT_SPACING_0        4
#define HOR_SPACING_0         4
#define SCROLL_LINES_0        1

#define VERT_SIZE_1           20
#define HOR_SIZE_1            12
#define VERT_SPACING_1        4
#define HOR_SPACING_1         4
#define SCROLL_LINES_1        (VERT_SIZE_1 + VERT_SPACING_1)

#define BKG_COLOR_ADDR        NUM_COLORS
#define FG_COLOR_ADDR         (NUM_COLORS + 1)

static void write_bottom_lines(ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG);
static void scroll_screen(ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG);
static int scroll_gmode(ULONG, ULONG, ULONG, ULONG, ULONG);

/*
 * NAME : scroll_h_draw
 *
 * DESCRIPTION :
 *
 *  Displays scrolling Hs in graphics mode (using draw and blit engine)
 *
 * INPUT :
 *
 *  None.
 *
 * OUTPUT :
 *
 *  None.
 *
 * RETURNS :
 *
 *  Error code or SUCCESS.
 *
*/

#define CHAR_WIDTH   13
#define CHAR_HEIGHT  25

int scroll_h_draw(void)
{
  BOX     box;
  LINE    line;
  int     i, j, rc;
  ULONG xres, yres, raster, status;

  TITLE("scroll_h_draw");

  rc = SUCCESS;

  if ((rc = clear_screen ()) == SUCCESS)
  {
    get_screen_res (&xres, &yres);
    line.color = box.color = WHITE;
    do
      {
        /***** Draw an 'H' *****/

        /* Modify raster to use foreground mask for drawing */
        get_igc_reg ((UCHAR) RASTER_REG, &raster);    
        raster = (raster & 0x30000) | IGC_F_MASK | OVER_SIZED;
        wait_for_wtkn_ready ();
        write_igc_reg ((UCHAR) RASTER_REG, raster);

        line.x1 = 2; line.y1 = (yres-CHAR_HEIGHT)+7; line.x2 = 6; line.y2 = (yres-CHAR_HEIGHT)+7;
        rc = draw_line(&line);
        if (rc != SUCCESS) break;
        line.x1 = 8; line.y1 = (yres-CHAR_HEIGHT)+7; line.x2 = 12; line.y2 = (yres-CHAR_HEIGHT)+7;
        rc = draw_line(&line);
        if (rc != SUCCESS) break;
        box.xstart = 3; box.ystart = (yres-CHAR_HEIGHT)+14; box.xend = 11; box.yend = (yres-CHAR_HEIGHT)+15;
        rc = draw_box(&box);
        if (rc != SUCCESS) break;
        line.x1 = 2; line.y1 = (yres-CHAR_HEIGHT)+23; line.x2 = 6; line.y2 = (yres-CHAR_HEIGHT)+23;
        rc = draw_line(&line);
        if (rc != SUCCESS) break;
        line.x1 = 8; line.y1 = (yres-CHAR_HEIGHT)+23; line.x2 = 12; line.y2 = (yres-CHAR_HEIGHT)+23;
        rc = draw_line(&line);
        if (rc != SUCCESS) break;
        box.xstart = 3; box.ystart = (yres-CHAR_HEIGHT)+7; box.xend = 5; box.yend = (yres-CHAR_HEIGHT)+23;
        rc = draw_box(&box);
        if (rc != SUCCESS) break;
        box.xstart = 9; box.ystart = (yres-CHAR_HEIGHT)+7; box.xend = 11; box.yend = (yres-CHAR_HEIGHT)+23;
        rc = draw_box(&box);
        if (rc != SUCCESS) break;

        /***** Copy it across screen *****/

        /* Modify the raster to use the image's color */
        wait_for_wtkn_ready();
        get_igc_reg ((UCHAR) RASTER_REG, &raster);    
        raster = (raster & 0x30000) | IGC_S_MASK | OVER_SIZED;
        write_igc_reg ((UCHAR) RASTER_REG, raster);

        box.xstart = 0;
        box.ystart = (yres-1)-CHAR_HEIGHT;
        box.xend = CHAR_WIDTH;
        box.yend = (yres-1);
        for (i = 1; i <= (xres / CHAR_WIDTH); i++)
          {
            /* setting up the blit registers for the blit source */
            igc_write_2D_coord_reg (IGM_ABS, IGM_X, 0, 
                                    (ULONG) uitrunc ((double) box.xstart));
            igc_write_2D_coord_reg (IGM_ABS, IGM_Y, 0, 
                                    (ULONG) uitrunc ((double) box.ystart));
            igc_write_2D_coord_reg (IGM_ABS, IGM_X, 1, 
                                    (ULONG) uitrunc ((double) box.xend));
            igc_write_2D_coord_reg (IGM_ABS, IGM_Y, 1, 
                                    (ULONG) uitrunc ((double) box.yend));
           
            /* setting up the coordinates for the blit target */
            igc_write_2D_coord_reg (IGM_ABS, IGM_X, 2, 
                                    (ULONG) uitrunc ((double) box.xstart+(i*CHAR_WIDTH)));
            igc_write_2D_coord_reg (IGM_ABS, IGM_Y, 2, 
                                    (ULONG) uitrunc ((double) box.ystart));
            igc_write_2D_coord_reg (IGM_ABS, IGM_X, 3, 
                                    (ULONG) uitrunc ((double) box.xend+(i*CHAR_WIDTH)));  
            igc_write_2D_coord_reg (IGM_ABS, IGM_Y, 3, 
                                    (ULONG) uitrunc ((double) box.yend));  
           
            igc_do_blit ();
           
            wait_for_wtkn_ready();
            get_igc_reg ((UCHAR) STATUS_REG, &status);
            if (status & BLIT_EXCEPTIONS)
              {
                rc = BLIT_CMD_FAILED;
                return(rc);
              } 
          } 

        /***** Scroll screen up by one line *****/
        for (i=0; i < CHAR_HEIGHT; i++) 
          {
            /***** Scroll screen up by one pixel *****/
                /* setting up the blit registers for the blit source*/
                igc_write_2D_coord_reg (IGM_ABS, IGM_X, 0, 
                                        (ULONG) uitrunc ((double) 0));
                igc_write_2D_coord_reg (IGM_ABS, IGM_Y, 0, 
                                        (ULONG) uitrunc ((double) 1));
                igc_write_2D_coord_reg (IGM_ABS, IGM_X, 1, 
                                        (ULONG) uitrunc ((double) (xres-1)));
                igc_write_2D_coord_reg (IGM_ABS, IGM_Y, 1, 
                                        (ULONG) uitrunc ((double) (yres-1)));
             
                /* setting up the coordinates for the blit target */
                igc_write_2D_coord_reg (IGM_ABS, IGM_X, 2, 
                                        (ULONG) uitrunc ((double) 0));
                igc_write_2D_coord_reg (IGM_ABS, IGM_Y, 2, 
                                        (ULONG) uitrunc ((double) 0));
                igc_write_2D_coord_reg (IGM_ABS, IGM_X, 3, 
                                        (ULONG) uitrunc ((double) (xres-1)));
                igc_write_2D_coord_reg (IGM_ABS, IGM_Y, 3, 
                                        (ULONG) uitrunc ((double) (yres-2)));
             
                igc_do_blit ();
             
                wait_for_wtkn_ready();
                get_igc_reg ((UCHAR) STATUS_REG, &status);
                if (status & BLIT_EXCEPTIONS)
                  {
                    rc = BLIT_CMD_FAILED;
                    return(rc);
                  } 
          }
      } while ((!end_tu(get_dply_time())) && (rc == SUCCESS));

  return (rc);
  }
}


/*
 * NAME : scroll_h_gmode_tu0
 *
 * DESCRIPTION :
 *
 *  Displays scrolling Hs in graphics mode.
 *
 * INPUT :
 *
 *  None.
 *
 * OUTPUT :
 *
 *  None.
 *
 * RETURNS :
 *
 *  Error code or SUCCESS.
 *
*/

int scroll_h_gmode_tu0(void)
{
  TITLE("scroll_h_gmode_tu0");

  return(scroll_gmode(VERT_SIZE_0, HOR_SIZE_0, VERT_SPACING_0, HOR_SPACING_0,
                      SCROLL_LINES_0));
}


/*
 * NAME : scroll_h_gmode_tu1
 *
 * DESCRIPTION :
 *
 *  Displays scrolling Hs in graphics mode.
 *
 * INPUT :
 *
 *  None.
 *
 * OUTPUT :
 *
 *  None.
 *
 * RETURNS :
 *
 *  Error code or SUCCESS.
 *
*/

int scroll_h_gmode_tu1(void)
{
  TITLE("scroll_h_gmode_tu1");

  return(scroll_gmode(VERT_SIZE_1, HOR_SIZE_1, VERT_SPACING_1, HOR_SPACING_1,
                      SCROLL_LINES_1));
}



static int scroll_gmode(ULONG vsize, ULONG hsize, ULONG vspace,
                        ULONG hspace, ULONG lines)
{
  ULONG xres, yres, i;
  int rc;

  TITLE("scroll_gmode");

  if ((rc = clear_screen()) == SUCCESS)
  {
    disable_cursor ();
    get_screen_res(&xres, &yres);

    /* setup the foreground and background color */
    fill_palette (0xff, 0xff, 0xff, (UCHAR) FG_COLOR_ADDR, (UCHAR) FG_COLOR_ADDR);
    fill_palette (0, 0, 0, (UCHAR) BKG_COLOR_ADDR, (UCHAR) BKG_COLOR_ADDR);

    write_bottom_lines (xres, yres, vsize, hsize, vspace, hspace, lines);
    FOREVER
    {
      scroll_screen(xres, yres, vsize, hsize, vspace, hspace, lines);

      if(end_tu(get_dply_time()))
         break; 
    }
  } /* endif */

  return(rc);
}



/*
 * NAME : scroll_screen
 *
 * DESCRIPTION :
 *
 *  Scrolls screen in 8 bpp.
 *
 * INPUT :
 *
 *  1. X resolution.
 *  2. Y resolution.
 *
 * OUTPUT :
 *
 *  None.
 *
 * RETURNS :
 *
 *  None.
 *
*/

static void scroll_screen (ULONG xres, ULONG yres,
                           ULONG vsize, ULONG hsize,
                           ULONG vspace, ULONG hspace, ULONG lines)
{
  ULONG x, y, z, tmp, ylim, xlim, pixels_in_word;

  TITLE("scroll_screen");

  xlim = xres * BPP / BITS_IN_WORD;
  pixels_in_word = BITS_IN_WORD / BPP;

  /* Do for all lines : get group of pixel lines and copy them to previous   */
  /* group.                                                                  */

  for(y = 0; y < yres - lines; y += lines)
  {

    ylim = y + 2 * lines;
    ylim = (ylim < yres) ? lines : (lines - (ylim - yres));

    for(z = 0; z < ylim; z++)
    {
      for(x = 0; x < xlim; x++)
      {

        /*@*** Old XYADDR-based stuff ****/
        /* tmp = VRAM_XY_READ(x * pixels_in_word, y + z + lines, FULL_R); */
        /* Clear word if it's not a blank line                            */
        /* if(tmp != 0)                                                   */
        /*  VRAM_XY_WRITE(x * pixels_in_word, y + z + lines, FULL_W, 0);  */
        /* VRAM_XY_WRITE(x * pixels_in_word, y + z, FULL_W, tmp);         */

        /*@*** New NATIVE-based stuff ****/
        tmp = W9100_NATIVE_READ(x * pixels_in_word, y + z + lines);
        /* Clear word if it's not a blank line */
        if(tmp != 0)
          W9100_NATIVE_WRITE(x * pixels_in_word, y + z + lines, 0);
        W9100_NATIVE_WRITE(x * pixels_in_word, y + z, tmp);
      }
    }
  }

  write_bottom_lines (xres, yres, vsize, hsize, vspace, hspace, lines);

  return;
}




/*
 * NAME : write_bottom_lines
 *
 * DESCRIPTION :
 *
 *  Writes next lines of pixels. Next lines are always displayed at the bottom
 *  of the screen.
 *
 * INPUT :
 *
 *  1. X resolution.
 *  2. Y resolution.
 *
 * OUTPUT :
 *
 *  None.
 *
 * RETURNS :
 *
 *  None.
 *
*/

static void write_bottom_lines (ULONG xres, ULONG yres,
                                ULONG vsize, ULONG hsize, ULONG vspace,
                                ULONG hspace, ULONG num_lines)
{
  ULONG x, z, lines, i, xlimit, pixels_in_word;

  static ULONG idx = 0;                        /* index into line of Hs    */

/* Next buffer used to save 2 lines of pixels : one where the H's horizontal */
/* bar is drawn and one without this bar, so after these lines are saved we  */
/* can use FULL_W instead of PART_W to provide a more efficient way of       */
/* writing lines.                                                            */

  static struct
  {
    BOOL saved;
    ULONG buff[(MAX_NUM_COLS * BPP) / BITS_IN_WORD][2]; /*2 lines of pixels*/
    ULONG idx;
  } line = { FALSE };

  TITLE("write_bottom_lines");

  pixels_in_word = BITS_IN_WORD / BPP;
  xlimit = xres * BPP / BITS_IN_WORD;

  for(i = 0; i < num_lines; i++)
  {
    idx = idx % (vsize + vspace) + 1;

    if(idx <= vsize)                            /*otherwise it's line spacing*/
    {
      if(!line.saved)
      {
        for(z = 0; z < xres; z += (hsize + hspace))
        {
          for(x = z; x < z + hsize; x++)
          {
            if(idx == vsize / 2 || x == z || x == (z + hsize - 1))
              /*@ VRAM_XY_WRITE(x, yres + i - num_lines, PART_W, */
              /*@              get_part_xy (FG_COLOR_ADDR, 1));  */
              W9100_NATIVE_WRITE(x, yres + i - num_lines,
                            get_part_xy (FG_COLOR_ADDR, 1));
          }
        }

        line.idx = (idx == vsize / 2) ? 1 : 0;

        for(x = 0; x < xlimit; x++)
          line.buff[x][line.idx] =
            /*@ VRAM_XY_READ (x * pixels_in_word, yres + i - num_lines, FULL_R); */
            W9100_NATIVE_READ (x * pixels_in_word, yres + i - num_lines);

        if(idx == vsize / 2)
          line.saved = TRUE;
      }

      else                                       /* retrieve line from buffer*/
      {
        line.idx = (idx == vsize / 2) ? 1 : 0;

        for(x = 0; x < xlimit; x++)
          /*@ VRAM_XY_WRITE(x * pixels_in_word, yres + i - num_lines, FULL_W, */
          /*@                   line.buff[x][line.idx]);                      */
          W9100_NATIVE_WRITE(x * pixels_in_word, yres + i - num_lines,
                             line.buff[x][line.idx]);
      }
    }
  }

  return;
}
