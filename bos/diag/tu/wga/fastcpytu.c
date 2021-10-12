static char sccsid[] = "@(#)41  1.3  src/bos/diag/tu/wga/fastcpytu.c, tu_wga, bos411, 9428A410j 1/3/94 17:19:36";
/*
 *   COMPONENT_NAME: TU_WGA
 *
 *   FUNCTIONS: fast_copy_tu
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

#include "wga_regval.h"
#include "wga_reg.h"
#include "exectu.h"
#include "tu_type.h"
#include "wgamisc.h"
#include "wgareg.h"


#define  TEST_LOOPS           5                  /* test 5 times             */

#ifdef TRASH
static CHAR_BITMAP message [] =
  {
   { 0x00000000, 0xFFDB9918, 0x18181818, 0x1818183C, 0x00000000 },    /* 'T' */
   { 0x00000000, 0x00000000, 0x3C66C6FE, 0xC0C0623C, 0x00000000 },    /* 'e' */
   { 0x00000000, 0x00000000, 0x7AC6C270, 0x1C86C6BC, 0x00000000 },    /* 's' */
   { 0x00000000, 0x10103030, 0xFE303030, 0x3030321C, 0x00000000 },    /* 't' */
   { 0x00000000, 0x10381000, 0x78181818, 0x1818187E, 0x00000000 },    /* 'i' */
   { 0x00000000, 0x00000000, 0xDC666666, 0x666666EF, 0x00000000 },    /* 'n' */
   { 0x00000000, 0x00000000, 0x7BCDCCCC, 0x7880FC7E, 0x82C67C00 },    /* 'g' */
   { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },    /* ' ' */
   { 0x00000000, 0x10381000, 0x78181818, 0x1818187E, 0x00000000 },    /* 'i' */
   { 0x00000000, 0x00000000, 0xDC666666, 0x666666EF, 0x00000000 },    /* 'n' */
   { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },    /* ' ' */
   { 0x00000000, 0x00000000, 0xDC766363, 0x6363667C, 0x6060F000 },    /* 'p' */
   { 0x00000000, 0x00000000, 0xDE736360, 0x606060F8, 0x00000000 },    /* 'r' */
   { 0x00000000, 0x00000000, 0x3C66C3C3, 0xC3C3663C, 0x00000000 },    /* 'o' */
   { 0x00000000, 0x00000000, 0x7BCDCCCC, 0x7880FC7E, 0x82C67C00 },    /* 'g' */
   { 0x00000000, 0x00000000, 0xDE736360, 0x606060F8, 0x00000000 },    /* 'r' */
   { 0x00000000, 0x00000000, 0x3C66C6FE, 0xC0C0623C, 0x00000000 },    /* 'e' */
   { 0x00000000, 0x00000000, 0x7AC6C270, 0x1C86C6BC, 0x00000000 },    /* 's' */
   { 0x00000000, 0x00000000, 0x7AC6C270, 0x1C86C6BC, 0x00000000 },    /* 's' */
   { 0x00000000, 0x00000000, 0x00000000, 0x00001038, 0x10000000 },    /* '.' */
   { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },    /* ' ' */
   { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },    /* ' ' */
   { 0x00000000, 0xFE676763, 0x63677E60, 0x606060F0, 0x00000000 },    /* 'P' */
   { 0x00000000, 0x78181818, 0x18181818, 0x1818187E, 0x00000000 },    /* 'l' */
   { 0x00000000, 0x00000000, 0x3C66C6FE, 0xC0C0623C, 0x00000000 },    /* 'e' */
   { 0x00000000, 0x00000000, 0x7CC6C61E, 0x66C6CE73, 0x00000000 },    /* 'a' */
   { 0x00000000, 0x00000000, 0x7AC6C270, 0x1C86C6BC, 0x00000000 },    /* 's' */
   { 0x00000000, 0x00000000, 0x3C66C6FE, 0xC0C0623C, 0x00000000 },    /* 'e' */
   { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 },    /* ' ' */
   { 0x00000000, 0xC3C3C3C3, 0xDBDBDB7E, 0x6E666642, 0x00000000 },    /* 'W' */
   { 0x00000000, 0x00000000, 0x7CC6C61E, 0x66C6CE73, 0x00000000 },    /* 'a' */
   { 0x00000000, 0x10381000, 0x78181818, 0x1818187E, 0x00000000 },    /* 'i' */
   { 0x00000000, 0x10103030, 0xFE303030, 0x3030321C, 0x00000000 },    /* 't' */
   { 0x00000000, 0x00000000, 0x00000000, 0x00001038, 0x10000000 },    /* '.' */
   { 0x00000000, 0x00000000, 0x00000000, 0x00001038, 0x10000000 },    /* '.' */
   { 0x00000000, 0x00000000, 0x00000000, 0x00001038, 0x10000000 },    /* '.' */
  };
#endif

/*
 * NAME : fast_copy_tu
 *
 * DESCRIPTION :
 *
 *   This TU is used to verify the fixes for the "hash" problem seen on
 *   the WIZARD chip.  This problem occurs on any write operation to the VRAM
 *   immediately after a read operation, which would cause the write not to
 *   be processed.  Refer to feature # 82823 for more details.
 *   This TU will first clear the entire screen (with 0s), paints the top half
 *   with some color, and then copy the bottom half to top half using
 *   fast copy method.  To verify, the TU will read the top half of screen
 *   using NATIVE read method (we can use any method) to verify that all pixels
 *   are set to 0 (BLACK).
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

int fast_copy_tu (void)
{
  int     rc, words_per_scan_line, num_scan_lines, loop, scale;
  ulong_t xres, yres, *source_addr, *target_addr;
  ulong_t x, y, data;
  BOX     box;

  TITLE("fast_copy_tu");

  if ((rc = clear_screen ()) == SUCCESS)
  {
    get_screen_res (&xres, &yres);               /* screen size in pixels     */

#ifdef TRASH
    scale = 2;                                   /* blow up the letter size   */
    display_message (352, yres - (scale * CHARACTER_HEIGHT_IN_PIXEL), scale,
                     scale, message, (uchar_t) YELLOW,
                     sizeof (message) / sizeof (CHAR_BITMAP));

    /* we need to re-adjust the screen height so that we can skip the wait msg*/
    yres -= (scale * CHARACTER_HEIGHT_IN_PIXEL);
#endif

    /* We know that we are going to copy from the bottom 1/2 of screen  to    */
    /* the top, therefore, we can do some set up in advance (i.e. number of   */
    /* words per screen width (scan line), source address to be copied from   */
    /* (start from bottom half of screen), target address to be copied to     */
    /* (start from top of screen), and number scan lines to be copied).       */
    /* Otherwise, the following set up must be done in the 'for' loop.        */

    /* calculate the amount of memory for each scan line (bits per scan line) */
    words_per_scan_line = (xres * BPP) / BITS_IN_WORD;     /* words per line  */

    /* set up the location of the box to be drawn in the top half of screen   */
    box.xstart = 0;                              /*start at left corner of scr*/
    box.ystart = 0;                              /*start at left corner of scr*/
    box.xend   = (float) (xres - 1);             /*right corner, offset from 0*/
    box.yend   = (float) (trunc ((yres - 1) / 2));  /*1/2 of scr, offset fr. 0*/

    num_scan_lines = (ulong_t) (box.yend - box.ystart + 1);   /* lines to cpy */

    /* calculate the from/to addresses to copy (using NATIVE addressing)      */
    source_addr =                                /* addr of scr to copy from  */
       W8720ADDR((ulong_t) box.xstart, (ulong_t) (box.yend + 1));
    target_addr =                                /* addr of scr to copy to    */
       W8720ADDR((ulong_t) box.xstart, (ulong_t) box.ystart);


    /* now, we will display the box in different color accept for BLACK, and  */
    /* then verify the VRAM.  If an error occur, we will return to the caller */
    /* immediately.  Otherwise, we need to test several more times to make    */
    /* sure that there is no error.                                           */
    for (loop = 1; (loop <= TEST_LOOPS) && (rc == SUCCESS); loop++)
    {
      box.color  = (uchar_t) ((loop % NUM_COLORS) + 1);      /* skip if BLACK */
      rc = draw_box (&box);                      /* display the box with color*/

      if (rc == SUCCESS)
      {
        /* copy the bottom half of screen to top half using fast copy method  */
        moveit (target_addr, source_addr, words_per_scan_line,
                   BYTES_IN_WORD, WTKN_YSHIFT, num_scan_lines);

        /* verify the VRAM just copied.  Every words should become to BLACK   */
        /* (0) since we just finished copied from the bottom 1/2 screen to top*/
        for (y = box.ystart; (rc == SUCCESS) && (y <= box.yend); y++)
          for (x = box.xstart; (rc == SUCCESS) && (x <= box.xend);
                                                      x += BYTES_IN_WORD)
          {           /* increment x by BYTES_IN_WORD because we R/W 4 pixels */
            data = W8720_NATIVE_READ(x, y);      /* get the data from adapter */
            if (data != BLACK)                   /* error if data != BLACK (0)*/
            {                                    /*because bottom of scr = BLK*/
              rc = VRAM_FAST_COPY_ERR;
              set_mem_info (W8720ADDR(x, y), BLACK, data, rc,
                            "Fast Copy Test Failed", "");
              break;
            } /* end if */
          } /* endfor */
      } /* end if */
    } /* endfor */

  } /* endif */

  return (rc);
}
