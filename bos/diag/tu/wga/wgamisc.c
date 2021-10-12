static char sccsid[] = "@(#)84  1.5  src/bos/diag/tu/wga/wgamisc.c, tu_wga, bos411, 9428A410j 1/3/94 17:31:44";
/*
 *   COMPONENT_NAME: TU_WGA
 *
 *   FUNCTIONS: dump_wga_regs
 *              set_background_color
 *              set_foreground_color
 *              sign_extend
 *              wga_initialize
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
#include <sys/termio.h>
#include <stdio.h>

#include "exectu.h"
#include "tu_type.h"
#include "wgareg.h"
#include "wgamisc.h"
#include "vpd.h"



/*---------------------------------------------------------------------------*/
/* FUNCTIONS:                                                                */
/*---------------------------------------------------------------------------*/

#ifdef DUMP_CONFIG_REGS
void dump_wga_regs (void)
{
  ulong_t  cntlreg;

  printf ("\n\nDumping the Weitek registers");  fflush(stdout);

  get_igc_reg ((uchar_t) SYSCNFG_REG, &cntlreg);
  printf ("\n>> SYSCNFG = %8x", cntlreg);  fflush(stdout);
  get_igc_reg ((uchar_t) INT_EN_REG, &cntlreg);
  printf ("\n>> INT_EN = %8x", cntlreg);  fflush(stdout);
  get_igc_reg ((uchar_t) SRTCTL_REG, &cntlreg);
  printf ("\n>> SRTCTL = %8x", cntlreg);  fflush(stdout);
  get_igc_reg ((uchar_t) MEMCNFG_REG, &cntlreg);
  printf ("\n>> MEMCNFG = %8x", cntlreg);  fflush(stdout);
  get_igc_reg ((uchar_t) RFPER_REG, &cntlreg);
  printf ("\n>> RFPER = %8x", cntlreg);  fflush(stdout);
  get_igc_reg ((uchar_t) RLMAX_REG, &cntlreg);
  printf ("\n>> RLMAX = %8x", cntlreg);  fflush(stdout);
  get_igc_reg ((uchar_t) PREHRZC_REG, &cntlreg);
  printf ("\n>> PREHRZC = %8x", cntlreg);  fflush(stdout);
  get_igc_reg ((uchar_t) PREVRTC_REG, &cntlreg);
  printf ("\n>> PREVRTC = %8x", cntlreg);  fflush(stdout);

  return;
}
#endif



/*
 * NAME : wga_initialize
 *
 * DESCRIPTION :
 *
 *   Initialize and setup the global WGA registers.
 *
 * INPUT :
 *
 *   None.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   None.
 *
*/



static BOOL initialize_palette = TRUE;

void wga_initialize (void)
{
  ulong_t     raster;
  ulong_t     xres, yres;

  TITLE("wga_initialize");

#ifdef DUMP_CONFIG_REGS
  dump_wga_regs ();
#endif

#ifdef SET_COLORS
  initialize_palette = TRUE;                                  /*override to restore for DD*/
#endif

  if (initialize_palette)                       /* initialize color palette */
  {
    color_init ();                               /* initialize primary color */
    set_machine_model (DEFAULT_MACHINE_MODEL);

    initialize_palette = FALSE;
  }

  /* initialize clipping window to full size, no clipping                    */
  get_screen_res (&xres, &yres);
  set_w_offset (0, 0);
  set_wclip (0, 0, xres - 1, yres - 1);

  get_igc_reg ((uchar_t) RASTER_REG, &raster);
  raster = (raster & 0x30000) | IGC_F_MASK | OVER_SIZED;
  wait_for_wtkn_ready ();                        /*we must wait for WTKN chip*/
  write_igc_reg ((uchar_t) RASTER_REG, raster);
  write_igc_reg ((uchar_t) PLANE_MASK_REG, (ulong_t) 0xff);

  return;
}




/*
 * NAME : reset_palette
 *
 * DESCRIPTION :
 *
 *    Sets up the variable for color palette
 *    initialization.
 *
 * INPUT :
 *
 *   None.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   None.
 *
*/


void reset_palette(void)

  {
    initialize_palette = TRUE;
    return;
  }


/*
 * NAME : set_foreground_color
 *
 * DESCRIPTION :
 *
 *  Causes the next characters received in the data stream or from the
 *  keyboard to have the display attributes by the parameter string.  Whether
 *  the characters really have the requested attributes on the display
 *  depends on the capabilities of the physical display device used by the
 *  virtual terminal.  For more information, refer to Set Graphic Rendition
 *  (SGR).
 *
 * INPUT :
 *
 *   Foreground color to be set.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   None.
 *
*/

void set_foreground_color (uchar_t color)
{

  TITLE("set_foreground_color");
  write_igc_reg ((uchar_t) FGROUND_REG, (ulong_t) color);

  return;
}



/*
 * NAME : set_background_color
 *
 * DESCRIPTION :
 *
 *  Causes the next characters received in the data stream or from the
 *  keyboard to have the display attributes by the parameter string.  Whether
 *  the characters really have the requested attributes on the display
 *  depends on the capabilities of the physical display device used by the
 *  virtual terminal.  For more information, refer to Set Graphic Rendition
 *  (SGR).
 *
 * INPUT :
 *
 *   Background color to be set.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   None.
 *
*/

void set_background_color (uchar_t color)
{

  TITLE("set_background_color");
  write_igc_reg ((uchar_t) BGROUND_REG, (ulong_t) color);

  return;
}



/*
 * NAME : sign_extend
 *
 * DESCRIPTION :
 *
 *   This function is used to extend the sign bit for a word.
 *
 * INPUT :
 *
 *   1. word to be extended.
 *   2. sign bit position.  The bit position is ordered from right to left.
 *      The LSB bit is bit 0
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   Extend the sign bit.
 *
*/

int sign_extend (ulong_t x, uchar_t sign_bit_pos)
{
  return ((x & 1 << sign_bit_pos) ?
             x | (~0 << sign_bit_pos) : x & ~(~0 << sign_bit_pos));
}
