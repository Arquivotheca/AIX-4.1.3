static char sccsid[] = "@(#)19  1.2  src/bos/diag/tu/wga/pixelcmdstu.c, tu_wga, bos411, 9428A410j 1/3/94 17:26:00";
/*
 *   COMPONENT_NAME: TU_WGA
 *
 *   FUNCTIONS: pixel1_tu
 *              pixel8_tu
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
#include "wga_reg.h"
#include "wga_regval.h"


#define SPACES_BETWEEN_2_LETTERS          30
#define N_CHARS                           26



static int read_native_space(ulong_t, ulong_t, ulong_t,ulong_t,uchar_t);
extern void CopyString (ulong_t *, ulong_t *, ulong_t);



static CHAR_BITMAP characters [N_CHARS] =
  {
   { 0x00000000, 0x3818183C, 0x2C2C267E, 0x4643C3E7, 0x00000000 },    /* 'A' */
   { 0x00000000, 0xFC666666, 0x667C6663, 0x636367FE, 0x00000000 },    /* 'B' */
   { 0x00000000, 0x1D376361, 0x61606060, 0x6161331E, 0x00000000 },    /* 'C' */
   { 0x00000000, 0xFC666363, 0x63636363, 0x636366FC, 0x00000000 },    /* 'D' */
   { 0x00000000, 0xFF636160, 0x647C6460, 0x606163FF, 0x00000000 },    /* 'E' */
   { 0x00000000, 0xFF636160, 0x647C6460, 0x606060F0, 0x00000000 },    /* 'F' */
   { 0x00000000, 0x3A6EC6C2, 0xC0C0CFC7, 0xC6C66E3A, 0x00000000 },    /* 'G' */
   { 0x00000000, 0xEF666666, 0x667E6666, 0x666666EF, 0x00000000 },    /* 'H' */
   { 0x00000000, 0x7E181818, 0x18181818, 0x1818187E, 0x00000000 },    /* 'I' */
   { 0x00000000, 0x3E0C0C0C, 0x0C0C0C0C, 0xCCCC8C78, 0x00000000 },    /* 'J' */
   { 0x00000000, 0xF766646C, 0x7870786C, 0x6C6666F7, 0x00000000 },    /* 'K' */
   { 0x00000000, 0xF0606060, 0x60606060, 0x606163FF, 0x00000000 },    /* 'L' */
   { 0x00000000, 0xC3426666, 0x7E7E5A5A, 0x424242E7, 0x00000000 },    /* 'M' */
   { 0x00000000, 0xC7626272, 0x725A5A4E, 0x4E4646E2, 0x00000000 },    /* 'N' */
   { 0x00000000, 0x3C66C3C3, 0xC3C3C3C3, 0xC3C3663C, 0x00000000 },    /* 'O' */
   { 0x00000000, 0xFE676763, 0x63677E60, 0x606060F0, 0x00000000 },    /* 'P' */
   { 0x00000000, 0x3C66C3C3, 0xC3C3C3C3, 0xDBCB6E3C, 0x0C070000 },    /* 'Q' */
   { 0X00000000, 0xFC666666, 0x666C786C, 0x666663F3, 0x00000000 },    /* 'R' */
   { 0x00000000, 0x7ACE86C2, 0xE0783C0E, 0x86C2E6BC, 0x00000000 },    /* 'S' */
   { 0x00000000, 0xFFDB9918, 0x18181818, 0x1818183C, 0x00000000 },    /* 'T' */
   { 0x00000000, 0xF7626262, 0x62626262, 0x6262623C, 0x00000000 },    /* 'U' */
   { 0x00000000, 0xF7636272, 0x3634341C, 0x1C180808, 0x00000000 },    /* 'V' */
   { 0x00000000, 0xC3C3C3C3, 0xDBDBDB7E, 0x6E666642, 0x00000000 },    /* 'W' */
   { 0x00000000, 0xE7636634, 0x3418182C, 0x2C66C6E7, 0x00000000 },    /* 'X' */
   { 0x00000000, 0xF7626224, 0x341C1818, 0x1818183C, 0x00000000 },    /* 'Y' */
   { 0x00000000, 0xFEC68C8C, 0x18183030, 0x6262C6FE, 0x00000000 }     /* 'Z' */
  };



/*
 * NAME : pixel1_tu
 *
 * DESCRIPTION :
 *
 *   This TU displays 26 alphabet characters in upper case with different
 *   colors across the screen using pixel1 command.  Each letter is a bitmap
 *   of 9x20 pixels which are made of 5 words.  Each pixel of the character
 *   will be enlarged 4 times bigger to make up a 36x80 pixels wide before
 *   sending to the pixel1 command.
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

int pixel1_tu (void)
{
  uchar_t x_scale, y_scale;
  int     rc, i, j, n_chars;
  ulong_t x0;                                    /* left edge of block       */
  ulong_t x1;                                    /* current point for xfer   */
  ulong_t y1;
  ulong_t x2;                                    /* right hand edge of block */
  ulong_t y3;                                    /* increment                */
  ulong_t raster, packed_xy, minterms, bitmap, xres, yres, color;


  TITLE ("pixel1_tu");

  if ((rc = clear_screen ()) == SUCCESS)
  {
    get_screen_res (&xres, &yres);
    x_scale  = 4;                                /* zoom rate per pixel      */
    y_scale  = 4;                                /* zoom rate per pixel      */

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
    igc_write_2D_coord_reg (IGM_ABS, IGM_X, 0, x0);
    packed_xy = IGM_PACK (x1, y1);
    igc_write_2D_coord_reg (IGM_ABS, IGM_XY, 1, packed_xy);
    igc_write_2D_coord_reg (IGM_ABS, IGM_X, 2, x2);
    igc_write_2D_coord_reg (IGM_ABS, IGM_Y, 3, y3);

    /* setup the minterms to write foreground when source is 1 and background*/
    /* when source is 0.                                                     */
    minterms = IGC_S_MASK & IGC_F_MASK;
    raster = (raster & 0x30000) | minterms | OVER_SIZED;
    write_igc_reg ((uchar_t) RASTER_REG, raster);

    i = 0;                                       /* start with first char    */
    /* set up foreground pseudo-color index for each character.  If the    */
    /* color is BLACK (color = 0) then skip to the next color              */
    set_foreground_color ((uchar_t) ((i % NUM_COLORS) + 1));

    draw_character (characters [i],            /* display the character    */
                    (uchar_t) PIXEL1_CMD, x_scale, y_scale, color);

    /* calculate the next position for the character to be displayed       */
    set_new_position (&x0, &x1, &y1, &x2, xres, yres, x_scale, y_scale,
                      SPACES_BETWEEN_2_LETTERS);

    /* set up the next coordinate for pixel1 command                       */
    igc_write_2D_coord_reg (IGM_ABS, IGM_X, 0, x0);
    packed_xy = IGM_PACK (x1, y1);
    igc_write_2D_coord_reg (IGM_ABS, IGM_XY, 1, packed_xy);
    igc_write_2D_coord_reg (IGM_ABS, IGM_X, 2, x2);

    if (i < (N_CHARS - 1))                     /* index to next character  */
      i++;
    else
      i = 0;
  } /* endif */

  return (rc);
}



/*
 * NAME : pixel8_tu
 *
 * DESCRIPTION :
 *
 *   This TU will display 1 character at a time ('A' through 'Z') on
 *   the center of the screen using pixel8 command.  Each character is
 *   a bitmap of 9x20 pixels which are make of 5 words.  Each pixel of
 *   the character will be enlarged 32 times bigger to make up a 288x640
 *   pixels bitmap before sending to the pixel8 command.  Each character
 *   will be displayed in different color.
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

int pixel8_tu (void)
{
  uchar_t x_scale, y_scale;
  int     rc, i, j, n_chars;
  ulong_t x0;                                    /* left edge of block       */
  ulong_t x1;                                    /* current point for xfer   */
  ulong_t y1;
  ulong_t x2;                                    /* right hand edge of block */
  ulong_t raster, packed_xy, minterms, bitmap, xres, yres;
  ulong_t color;

  TITLE ("pixel8_tu");

  if ((rc = clear_screen ()) == SUCCESS)
  {
    get_screen_res (&xres, &yres);
    x_scale  = 32;                               /* blow up 32 times bigger  */
    y_scale  = 32;                               /* blow up 32 times bigger  */

    /* setting up registers (coordinations) for pixel8 command               */
    x0 = (xres - (CHARACTER_WIDTH_IN_PIXEL * x_scale)) / 2;
    x1 = (xres - (CHARACTER_WIDTH_IN_PIXEL * x_scale)) / 2;      /* x coord. */
    y1 = (yres - (CHARACTER_HEIGHT_IN_PIXEL * y_scale)) / 2;     /* y coord. */
    x2 = x0 + x_scale * CHARACTER_WIDTH_IN_PIXEL;  /*right x corner of bitmap*/

    /* setup the minterms to write source color                              */
    minterms = IGC_S_MASK;
    get_igc_reg ((uchar_t) RASTER_REG, &raster);
    raster = (raster & 0x30000) | minterms | OVER_SIZED;

    wait_for_wtkn_ready ();
    write_igc_reg ((uchar_t) PLANE_MASK_REG, (ulong_t) 0xff);
    write_igc_reg ((uchar_t) RASTER_REG, raster);

    i = 0;                                       /* start with first char    */
    /* set up the coordinate for pixel8 command                            */
    igc_write_2D_coord_reg (IGM_ABS, IGM_X, 0, x0);
    packed_xy = IGM_PACK (x1, y1);
    igc_write_2D_coord_reg (IGM_ABS, IGM_XY, 1, packed_xy);
    igc_write_2D_coord_reg (IGM_ABS, IGM_X, 2, x2);
    igc_write_2D_coord_reg (IGM_ABS, IGM_Y, 3, 1);

    /* set up the color pattern (8 bits/pixels) for each letter.  If the   */
    /* color is BLACK (color = 0) then skip to next color                  */
    color = get_color_pattern ((i % NUM_COLORS) + 1);

    draw_character (characters [i],
                    (uchar_t) PIXEL8_CMD, x_scale, y_scale, color);
    if (i < (N_CHARS) - 1)                     /* index to next character  */
      i++;
    else
      i = 0;
  } /* endif */

  return (rc);
}



/*
 * NAME : draw_character
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

void draw_character (CHAR_BITMAP letter, uchar_t cmd,
                     uchar_t x_scale, uchar_t y_scale, ulong_t color)
{
  ulong_t  bitmap, data, pattern;
  int      i, j, k, l, bitpos;

  TITLE ("draw_character");

  x_scale %= (BITS_IN_WORD + 1);                 /* max zoom rate = 0 .. 32  */
  if (x_scale == 0)                              /* minimum zoom rate = 1    */
    x_scale = 1;

  wait_for_wtkn_ready ();                        /* wait for WTKN chip ready */

  for (i = 0; i < BIT_MAP_SIZE; i++)             /* for every word of bitmap */
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

          switch (cmd)
          {
            /* for pixel1 cmd, we enlarge each bit (0/1) into a word (left   */
            /* justify) and then send that word to pixel1 command            */
            case PIXEL1_CMD : data = data == 0 ? BLACK :
                                             ~(~(ulong_t) 0 >> x_scale);
                              igc_write_pixel1 (x_scale, data);
                              break;

            /* for pixel8 cmd, we enlarge each bit (0/1) into a multiple of 4*/
            case PIXEL8_CMD : for (l = 0; l < (x_scale / BYTES_IN_WORD); l++)
                              {
                                pattern = data == 0 ? BLACK : color;
                                igc_write_pixel8 (pattern);  /* for 4 pixels */
                              }
                              break;

            default         : break;
          } /* endswitch */
        } /* endfor */
      } /* endfor */
    } /* endfor */
  } /* endfor */

  return;
}




/*
 * NAME : set_new_position
 *
 * DESCRIPTION :
 *
 *   Find a next avaliable position on the screen for a character to
 *   be displayed (for pixel 1 command only).
 *
 * INPUT PARAMETERS :
 *
 *   1. location of left corner of bitmap
 *   2. location of x coordinate on the screen
 *   3. location of y coordinate on the screen
 *   4. location of right corner of bitmap
 *   5. x resolution of the screen
 *   6. y resolution of the screen
 *   7. x scale to be enlarged (1..32)
 *   8. y scale to be enlarged (1..32)
 *
 * OUTPUT
 *
 *   1. location of left corner of bitmap
 *   2. location of x coordinate on the screen
 *   3. location of y coordinate on the screen
 *   4. location of right corner of bitmap
 *
 * RETURNS:
 *
 *  None.
 *
*/

void set_new_position (ulong_t *x0, ulong_t *x1, ulong_t *y1,
                       ulong_t *x2, ulong_t xres, ulong_t yres,
                       uchar_t x_scale, uchar_t y_scale, ulong_t spaces)
{
  *x0 = *x2 + spaces;
  *x2 = *x0 + (x_scale * CHARACTER_WIDTH_IN_PIXEL);
  if (*x2 > xres)                                /* if out of bound then wrap*/
  {
    *x0 = 0;                                     /* restart with column 0    */
    *x2 = *x0 + (x_scale * CHARACTER_WIDTH_IN_PIXEL);  /*last col. of bitmap */
    *y1 += y_scale * CHARACTER_HEIGHT_IN_PIXEL;  /* y coord. on screen       */
  } /* endif */
  *x1 = *x0;                                     /* left coordinate on screen*/

  if ((*y1 + (y_scale * CHARACTER_HEIGHT_IN_PIXEL)) > yres)
  {                                              /* if out of bound then wrap*/
      *y1 = 0;
  }

  return;
}


#ifdef DISP_MSG
/*
 * NAME : display_wait_message
 *
 * DESCRIPTION :
 *
 *   Display a wait message on the screen at X, Y location.
 *
 * INPUT PARAMETERS :
 *
 *   1) X coordinate
 *   2) Y coordinate
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

void display_message (ulong_t x_coord, ulong_t y_coord,
                      uchar_t x_scale, uchar_t y_scale,
                      CHAR_BITMAP *message, uchar_t color, int n_chars)
{
  int     i;
  ulong_t x0;                                    /* left edge of block       */
  ulong_t x1;                                    /* current point for xfer   */
  ulong_t y1;
  ulong_t x2;                                    /* right hand edge of block */
  ulong_t y3;                                    /* increment                */
  ulong_t raster, old_raster, packed_xy, bitmap, xres, yres;

  TITLE("display_wait_message");

  /* setup to use foreground color mask for PIXEL 1 command                  */
  wait_for_wtkn_ready ();                        /* must wait for WTKN chip  */
  get_igc_reg ((uchar_t) RASTER_REG, &raster);

  /* setup the minterms to write foreground when source is 1 and background  */
  /* when source is 0.                                                       */
  old_raster = raster;               /* save default raster to restore later */
  raster = (raster & 0x30000) | IGC_S_MASK & IGC_F_MASK | OVER_SIZED;
  write_igc_reg ((uchar_t) RASTER_REG, raster);

  set_foreground_color ((uchar_t) color);        /* set foreground color     */

  /* setting up registers for pixel1 command                                 */
  x0 = x_coord;                                  /* left corner of bitmap    */
  x1 = x0;                                       /* x coord map to screen    */
  y1 = y_coord;                                  /* y coord map to screen    */
  x2 = x0 + x_scale * CHARACTER_WIDTH_IN_PIXEL;  /*right corner of bitmap    */
  y3 = 1;                                        /* increment                */

  get_screen_res (&xres, &yres);
  for (i = 0; i < n_chars; i++)
  {
    /* set up the parameter engine for the pixel1 command                    */
    igc_write_2D_coord_reg (IGM_ABS, IGM_X, 0, x0);
    packed_xy = IGM_PACK (x1, y1);
    igc_write_2D_coord_reg (IGM_ABS, IGM_XY, 1, packed_xy);
    igc_write_2D_coord_reg (IGM_ABS, IGM_X, 2, x2);
    igc_write_2D_coord_reg (IGM_ABS, IGM_Y, 3, y3);

    draw_character (*(message + i),            /* display the character      */
                    (uchar_t) PIXEL1_CMD, x_scale, y_scale, color);
    set_new_position (&x0, &x1, &y1, &x2, xres, yres, x_scale, y_scale, 1);
  };

  wait_for_wtkn_ready ();                        /* must wait for WTKN chip  */
  write_igc_reg ((uchar_t) RASTER_REG, raster);

  return;
}
#endif




/*
*  NAME  :  Pixel8_str_tst_tu 
*
*  DESCRIPTION :
*
*   This TU writes the whole VRAM buffer to RED color.  Then writes a 500X500 
*   rectangular buffer in Black (0) in the middle of the screen by writing 
*   a string of 104 bytes of data at a time to the PIXEL8 command. Then reads
*   the entire VRAM buffer to make sure the data read equals data written to
*   the VRAM.  If something wrong with the PIXEL8 command of the WEITEK chip,
*   black box will not be drawn in the center of the screen and error
*   'VRAM_RW_TEST_ERR' will be returned to the calling application.
*
*  INPUT PARAMETERS:
*
*     None.
*
*  OUTPUT
*
*     None.
*
*  RETURNS:
*
*     Error code or SUCCESS.
*
*/

#define   SIZE  500

int   pixel8_str_tst_tu (void)
  {
  int     rc;
  ulong_t x0;                                    /* left edge of block       */
  ulong_t x1;                                    /* current point for xfer   */
  ulong_t y1;
  ulong_t x2;                                    /* right hand edge of block */
  ulong_t raster, packed_xy, minterms, xres, yres;
  BOX     box;
  ulong_t  *black_box;



  TITLE ("pixel8_str_tst_tu");

  if ((rc = color_full_vram((uchar_t) RED)) == SUCCESS)
  {


   /* allocate memory space for the black box  */
   black_box = (ulong_t *) malloc ((size_t) (SIZE * SIZE));

   rc = (black_box == NULL) ?
                OUT_OFF_MEMORY_ERR : SUCCESS;


   if (rc == SUCCESS)
   { 
   get_screen_res (&xres, &yres);

   /* setting up registers (coordinates) for pixel8 command               */
   x0 = (xres - SIZE) / 2;    /* Left edge of the block to be transferred */
   x1 = (xres - SIZE) / 2;    /* Point at which to begin the transfer     */
   y1 = (yres - SIZE) / 2;    /* y coord. */
   x2 = x0 + (SIZE - 1);      /* right edge of the block to be xfer       */

   /* now, we must make sure that the coordinates are on word boundary  */
   x0 = (x0 / BYTES_IN_WORD) * BYTES_IN_WORD;
   x1 = (x1 / BYTES_IN_WORD) * BYTES_IN_WORD;
   x2 = (x2 / BYTES_IN_WORD) * BYTES_IN_WORD;

   /* setup the minterms to write source color                            */
   wait_for_wtkn_ready ();
   minterms = IGC_S_MASK;
   get_igc_reg ((uchar_t) RASTER_REG, &raster);
   raster = (raster & 0x30000) | minterms | OVER_SIZED;

   write_igc_reg ((uchar_t) PLANE_MASK_REG, (ulong_t) 0xff);
   write_igc_reg ((uchar_t) RASTER_REG, raster);


    /* set up the coordinates for pixel8 command                         */
    igc_write_2D_coord_reg (IGM_ABS, IGM_X, 0, x0);
    packed_xy = IGM_PACK (x1, y1);
    igc_write_2D_coord_reg (IGM_ABS, IGM_XY, 1, packed_xy);
    igc_write_2D_coord_reg (IGM_ABS, IGM_X, 2, x2);
    igc_write_2D_coord_reg (IGM_ABS, IGM_Y, 3, 1);


    /* initialize black box to zeroes */
    memset (&black_box[0], BLACK, SIZE*SIZE);


    CopyString (PIXEL8_REG_ADDR, black_box, SIZE*SIZE);

    free (black_box);      /* return the allocated memory space */



   /* read and verify the data written to the VRAM in native addressing mode */ 

  if ((rc = read_native_space(0, 0, (xres-1), y1 - 1, (uchar_t)RED)) == SUCCESS)
  if ((rc = read_native_space(0, y1, x0 - 1, y1 + (SIZE - 1),(uchar_t) RED))
                                                == SUCCESS)
   if ((rc= read_native_space (x2 + 1, y1, (xres-1), y1 + (SIZE - 1),
                               (uchar_t) RED))== SUCCESS)
    if ((rc = read_native_space(0, y1+SIZE, (xres-1), (yres-1),
                               (uchar_t) RED))== SUCCESS)
      if (rc = read_native_space(x0, y1, x2, y1 + (SIZE - 1),(uchar_t) BLACK));
    }    /* endif */

  }
  return (rc);
   }




/*
*  NAME  :  read_native_space
* 
*  DESCRIPTION :
*    Reads the specified portion of the VRAM space in native addressing mode
*    and makes sure data read is equal to the value of the color passed
*    as a parameter.
*
*
*  INPUT PARAMETERS:
*     
*     1.  left X coordinate      
*     2.  left Y coordinate
*     3.  right x coordinate
*     4.  bottom left y coordinate       
*     5.  color           
*
*  OUTPUT
*
*    None.
*
*  RETURNS :
*
*    Error code or SUCCESS. 
*
*/


 static int read_native_space (ulong_t x_begin, ulong_t y_begin,
                                ulong_t x_end, ulong_t y_end,
                                        uchar_t color)
   {
     ulong_t   data, pattern, x, y, *addr;
     int       rc;

     TITLE ("read_native_space");

     /* word of color pattern formatted data */
     pattern = get_color_pattern (color);    
     rc = SUCCESS;

     /* read and verify VRAM in native addressing mode */   
     for (y = y_begin; y <= y_end && rc== SUCCESS; y++)
     {
      addr = (ulong_t) W8720ADDR (x_begin, y);
      for (x = x_begin; x < x_end && rc== SUCCESS; x +=BYTES_IN_WORD, addr++)
         {
           data = *addr;
           if (data != pattern)
           {
             rc = VRAM_RW_TEST_ERR;
             set_mem_info (addr, pattern, data, rc,
                                "", "Read NATIVE FMT");
           }
         }  /* end for */
      } /* end for */
    return (rc);
   }

