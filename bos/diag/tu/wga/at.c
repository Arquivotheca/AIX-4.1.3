static char sccsid[] = "@(#)06  1.2  src/bos/diag/tu/wga/at.c, tu_wga, bos411, 9428A410j 3/4/94 17:47:55";
/*
 *   COMPONENT_NAME: TU_WGA
 *
 *   FUNCTIONS: display_AT_tu 
 *
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
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
#include "wga_reg.h"
#include "wga_regval.h"

#define CHAR_HEIGHT_IN_PIXEL               16
#define SPACES_BETWEEN_2_LETTERS           5 
#define NCHARS                             1
#define BITMAP_SIZE                        3
typedef ulong_t BITMAP [BITMAP_SIZE];

static BITMAP character[NCHARS] =
{
  { 0x3E63C19D, 0xBDB5B5B7, 0x9AC0613E }                     /* '@' */
};

static void draw_char (BITMAP letter, uchar_t x_scale, uchar_t y_scale, 
                      ulong_t color);

/*
 * NAME : display_AT_tu
 *
 * DESCRIPTION :
 *   This TU displays @ sign full screen.  Each @ is a bitmap of 9x20 pixels
 *   which is 5 words.  Each pixel of the character will be enlarged 1 times
 *   horizontally and 1 times vertically to make up a 9x20  pixels before
 *   displaying on the screen.
 *
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

int display_AT_tu (void)
{
  uchar_t x_scale, y_scale;
  int     rc, n_chars;
  int     row, col;
  ulong_t x0;                                    /* left edge of block       */
  ulong_t x1;                                    /* current point for xfer   */
  ulong_t y1;
  ulong_t x2;                                    /* right hand edge of block */
  ulong_t y3;                                    /* increment                */
  ulong_t raster, packed_xy, minterms, bitmap, xres, yres, color;


  TITLE ("display_AT_tu");

  if ((rc = clear_screen ()) == SUCCESS)
  {
    get_screen_res (&xres, &yres);
    x_scale  = 1;                                /* zoom rate per pixel      */
    y_scale  = 2;                                /* zoom rate per pixel      */

    /* setup to use foreground color mask                                    */
    get_igc_reg ((uchar_t) RASTER_REG, &raster);
    raster = (raster & 0x30000) | IGC_F_MASK | OVER_SIZED;
    wait_for_wtkn_ready ();
    write_igc_reg ((uchar_t) RASTER_REG, raster);
    write_igc_reg ((uchar_t) PLANE_MASK_REG, (ulong_t) 0xff);

    /* do it again so we can turn the pattern use off                        */
    write_igc_reg ((uchar_t) RASTER_REG, raster);

    /* setting up registers for pixel1 command                               */
    x0 = 0;                                      /* left corner of bitmap    */
    x1 = x0;                                     /* x coord map to screen    */
    y1 = 0;                                      /* y coord map to screen    */
    x2 = x0 + x_scale * CHARACTER_WIDTH_IN_PIXEL;/*right corner of bitmap    */
    y3 = 1;                                      /* increment                */

    /* set up the parameter engine for the pixel1 command                    */
    igc_write_2D_coord_reg (IGM_ABS, IGM_Y, 3, y3);

    /* setup the minterms to write foreground when source is 1 and background*/
    /* when source is 0.                                                     */
    minterms = IGC_S_MASK & IGC_F_MASK;
    raster = (raster & 0x30000) | minterms | OVER_SIZED;
    write_igc_reg ((uchar_t) RASTER_REG, raster);

    set_foreground_color ((uchar_t) WHITE);  

	for (row = 0; row <= (yres - (y_scale * CHAR_HEIGHT_IN_PIXEL));)
    {
	  for (col = 0; col <= (xres - (x_scale * CHARACTER_WIDTH_IN_PIXEL));)
	  {
        igc_write_2D_coord_reg (IGM_ABS, IGM_X, 0, col);
        packed_xy = IGM_PACK ((ulong_t) col, (ulong_t) row);
        igc_write_2D_coord_reg (IGM_ABS, IGM_XY, 1, packed_xy);
        igc_write_2D_coord_reg (IGM_ABS, IGM_X, 2, 
								   (col + x_scale * CHARACTER_WIDTH_IN_PIXEL));

        draw_char (character[0], x_scale, y_scale, WHITE);

		col += (x_scale * CHARACTER_WIDTH_IN_PIXEL) + SPACES_BETWEEN_2_LETTERS;
      }	
	  row +=(y_scale * CHAR_HEIGHT_IN_PIXEL); /* update row position*/
    }  /* end for */
  } /* endif */

  return (rc);
}


/*
 * NAME : draw_char
 *
 * DESCRIPTION :
 *
 *   Enlarge and display a character bitmap at specify color.
 *
 * INPUT PARAMETERS :
 *
 *   1. character bitmap (5 words)
 *   2. command to be displayed (pixel1 / pixel8)
 *   3. x scale to be enlarged (1 .. 32)
 *   4. y scale to be enlarged (1 .. 32)
 *   5. color to be displayed
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

static void draw_char (BITMAP letter, uchar_t x_scale, uchar_t y_scale, 
  					   ulong_t color)
{
  ulong_t  bitmap, data, pattern;
  int      i, j, k, l, bitpos;

  TITLE ("draw_char");

  x_scale %= (BITS_IN_WORD + 1);                 /* max zoom rate = 0 .. 32  */
  if (x_scale == 0)                              /* minimum zoom rate = 1    */
    x_scale = 1;

  wait_for_wtkn_ready ();                        /* wait for WTKN chip ready */

  for (i = 0; i < BITMAP_SIZE; i++)              /* for every word of bitmap */
  {

    /* starting from the left (MSB) byte, we zoom every bits of each word    */
    /* in the bitmap.                                                        */
    for (j = 24; j >= 0; j -= 8)
    {
      bitmap = (letter[i] >> j) & 0xFF;          /*strip off 1 byte of bitmap*/

      /* for the byte that we stripped off, enlarge every bit.               */
      for (k = y_scale; k > 0; k--)
      {
        /* enlarge every bits of the byte we stripped off                    */
        for (bitpos = BITS_IN_BYTE - 1; bitpos >= 0; bitpos--)
        {
          data = ((bitmap >> bitpos) & 0x01);    /* get a bit to be enlarged */

          /* for pixel1 cmd, we enlarge each bit (0/1) into a word (left   */
          /* justify) and then send that word to pixel1 command            */
          data = data == 0 ? BLACK : ~(~(ulong_t) 0 >> x_scale);
          igc_write_pixel1 (x_scale, data);
        } /* endfor */
      } /* endfor */
    } /* endfor */
  } /* endfor */

  return;
}

