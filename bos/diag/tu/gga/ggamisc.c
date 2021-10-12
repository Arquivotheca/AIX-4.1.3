static char sccsid[] = "@(#)72	1.1  src/bos/diag/tu/gga/ggamisc.c, tu_gla, bos41J, 9515A_all 4/6/95 09:27:00";
/*
 *   COMPONENT_NAME: tu_gla
 *
 *   FUNCTIONS: FLWDCHARS
 *              HFWDCHARS
 *              VTD
 *              dump_BT485_regs
 *              dump_wtk_draw_eng_regs
 *              dump_wtk_param_eng_regs
 *              dump_wtk_sys_cntl_regs
 *              dump_wtk_video_cntl_regs
 *              gga_initialize
 *              reset_palette
 *              set_background_color
 *              set_foreground_color
 *              sign_extend
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
#include <sys/termio.h>

#include <stdio.h>

#include "exectu.h"
#include "tu_type.h"
#include "ggareg.h"
#include "ggamisc.h"
#include "ggapci.h"


/*---------------------------------------------------------------------------*/
/* GLOBAL CONSTANTS:                                                         */
/*---------------------------------------------------------------------------*/

#define  SGR_FOREGROUND_OFFSET              30
#define  SGR_BRIGHT_FOREGROUND_OFFSET       90
#define  SGR_BACKGROUND_OFFSET              40
#define  SGR_BRIGHT_BACKGROUND_OFFSET       100

#define  HFWDCHARS(n)    (n) >> 8  & 0xff, (n) & 0xff
#define  FLWDCHARS(n)    (n) >> 24 & 0xff, (n) >> 16 & 0xff, (n) >> 8 & 0xff, (n) & 0xff
#define  VTD(n)          HFINTROESC, HFINTROLBR, HFINTROEX, FLWDCHARS(n)

#define  PROTOCOLVTD     VTD(sizeof(struct hfprotocol) -3)
#define  HFINTRO         HFINTROESC, HFINTROLBR, HFINTROEX



/*---------------------------------------------------------------------------*/
/* FUNCTIONS:                                                                */
/*---------------------------------------------------------------------------*/

void dump_wtk_sys_cntl_regs (void)
{
  ULONG  cntlreg;

  printf ("\nWeitek System Control registers"); fflush(stdout);
  printf ("\n-------------------------------"); fflush(stdout);

  get_igc_reg ((UCHAR) SYSCONFIG_REG, &cntlreg);
  printf ("\n>> SYSCONFIG_REG = %8x", cntlreg); fflush(stdout);
  get_igc_reg ((UCHAR) INTERRUPT_REG, &cntlreg);
  printf ("\n>> INTERRUPT_REG = %8x", cntlreg); fflush(stdout);
  get_igc_reg ((UCHAR) INTERRUPT_ENBL_REG, &cntlreg);
  printf ("\n>> INTERRUPT_ENBL_REG = %8x \n\n", cntlreg); fflush(stdout);

  return;
}

void dump_wtk_param_eng_regs (void)
{
  ULONG  cntlreg;

  printf ("\nWeitek Parameter Engine registers"); fflush(stdout);
  printf ("\n---------------------------------"); fflush(stdout);

  get_igc_reg ((UCHAR) STATUS_REG, &cntlreg);
  printf ("\n>> STATUS_REG = %8x", cntlreg); fflush(stdout);
  get_igc_reg ((UCHAR) CINDEX_REG, &cntlreg);
  printf ("\n>> CINDEX_REG = %8x", cntlreg); fflush(stdout);
  get_igc_reg ((UCHAR) W_OFFSET_REG, &cntlreg);
  printf ("\n>> W_OFFSET_REG = %8x \n\n", cntlreg); fflush(stdout);

  return;
}

void dump_wtk_draw_eng_regs (void)
{
  ULONG  cntlreg;

  printf ("\nWeitek Drawing Engine registers"); fflush(stdout);
  printf ("\n-------------------------------"); fflush(stdout);

  get_igc_reg ((UCHAR) FGROUND_REG, &cntlreg);
  printf ("\n>> FGROUND_REG = %8x", cntlreg); fflush(stdout);
  get_igc_reg ((UCHAR) BGROUND_REG, &cntlreg);
  printf ("\n>> BGROUND_REG = %8x", cntlreg); fflush(stdout);
  get_igc_reg ((UCHAR) PLANE_MASK_REG, &cntlreg);
  printf ("\n>> PLANE_MASK_REG = %8x", cntlreg); fflush(stdout);
  get_igc_reg ((UCHAR) RASTER_REG, &cntlreg);
  printf ("\n>> RASTER_REG = %8x \n\n", cntlreg); fflush(stdout);

  return;
}

void dump_wtk_video_cntl_regs (void)
{
  ULONG  cntlreg;

  printf ("\nWeitek Video Control registers"); fflush(stdout);
  printf ("\n------------------------------"); fflush(stdout);

  get_igc_reg ((UCHAR) HRZT_REG, &cntlreg);
  printf ("\n>> HRZT_REG = %8x", cntlreg); fflush(stdout);
  get_igc_reg ((UCHAR) HRZSR_REG, &cntlreg);
  printf ("\n>> HRZSR_REG = %8x", cntlreg); fflush(stdout);
  get_igc_reg ((UCHAR) HRZBR_REG, &cntlreg);
  printf ("\n>> HRZBR_REG = %8x", cntlreg); fflush(stdout);
  get_igc_reg ((UCHAR) HRZBF_REG, &cntlreg);
  printf ("\n>> HRZBF_REG = %8x", cntlreg); fflush(stdout);
  get_igc_reg ((UCHAR) VRTT_REG, &cntlreg);
  printf ("\n>> VRTT_REG = %8x", cntlreg); fflush(stdout);
  get_igc_reg ((UCHAR) VRTSR_REG, &cntlreg);
  printf ("\n>> VRTSR_REG = %8x", cntlreg); fflush(stdout);
  get_igc_reg ((UCHAR) VRTBR_REG, &cntlreg);
  printf ("\n>> VRTBR_REG = %8x", cntlreg); fflush(stdout);
  get_igc_reg ((UCHAR) VRTBF_REG, &cntlreg);
  printf ("\n>> VRTBF_REG = %8x", cntlreg); fflush(stdout);
  get_igc_reg ((UCHAR) SRTCTL_REG, &cntlreg);
  printf ("\n>> SRTCTL_REG = %8x \n\n", cntlreg); fflush(stdout);
  get_igc_reg ((UCHAR) SRADDR_REG, &cntlreg);
  printf ("\n>> SRADDR_REG = %8x \n\n", cntlreg); fflush(stdout);
  get_igc_reg ((UCHAR) SRTCTL2_REG, &cntlreg);
  printf ("\n>> SRTCTL2_REG = %8x \n\n", cntlreg); fflush(stdout);

  return;
}

/*
 * NAME : gga_initialize
 *
 * DESCRIPTION :
 *
 *   Initialize and setup the global GGA registers.
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

void gga_initialize (void)
{
  ULONG     raster, syscnfg, temp;
  ULONG     xres, yres;

  TITLE("gga_initialize");

  if (initialize_palette)                       /* initialize color palette */
  {
    color_init ();                              /* initialize primary color */
    initialize_palette = FALSE;
  }

  /* initialize clipping window to full size, no clipping                    */
  get_screen_res (&xres, &yres);
  set_w_offset (0, 0);
  set_wclip (0, 0, xres - 1, yres - 1);
  raster = IGC_F_MASK | OVER_SIZED;
  wait_for_wtkn_ready ();
  write_igc_reg ((UCHAR) RASTER_REG, raster);
  write_igc_reg ((UCHAR) PLANE_MASK_REG, (ULONG) 0xffffffff);

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
 * NAME : set_color_1
 *
 * DESCRIPTION :
 *   Sets color[1] register with desired color
 *
 * INPUT :
 *
 *   color to be set.
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

void set_color_1 (ULONG color)
{
  TITLE("set_color_1");

  write_igc_reg ((UCHAR) COLOR1_REG, (ULONG) color);

  return;
}



/*
 * NAME : set_color_0
 *
 * DESCRIPTION :
 *
 *   Sets color[0] register with desired color
 *
 * INPUT :
 *
 *   color to be set.
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

void set_color_0 (ULONG color)
{
  TITLE("set_color_0");

  write_igc_reg ((UCHAR) COLOR0_REG, (ULONG) color);

  return;
}


/*
 * NAME : set_foreground_color
 *
 * DESCRIPTION :
 *   Sets foreground color register with desired color
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

void set_foreground_color (ULONG color)
{
  TITLE("set_foreground_color");

  write_igc_reg ((UCHAR) FGROUND_REG, (ULONG) color);

  return;
}



/*
 * NAME : set_background_color
 *
 * DESCRIPTION :
 *
 *   Sets background color register with desired color
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

void set_background_color (ULONG color)
{
  TITLE("set_background_color");

  write_igc_reg ((UCHAR) BGROUND_REG, (ULONG) color);

  return;
}


/*
 * NAME : get_end_of_frame_buffer
 *
 * DESCRIPTION :
 *
 *   Determines if this is a 2Mb or 4Mb adapter. This test will attempt to fill
 *   a 4Mb card with LONG values which are the address of each LONG in memory.
 *   Since the addressing scheme wraps on a 2Mb card this will cause a large
 *   number of failures when the addresses are read back for verification.
 *
 * INPUT :
 *
 * OUTPUT :
 *
 * RETURNS :
 *
 *   End of frame buffer address (i.e. 0x9fffff or 0xbfffff)
 *
*/

ULONG get_end_of_frame_buffer(void)
  {
    int   error_count = 0;
    ULONG i, *pbuffl, templ = 0;

    pbuffl = (ULONG *) (FRAME_BUFFER); /* Fill 4Mb VRAM with LONG addresses */
    for (i=0; i<0x100000; i++)
      {
        *pbuffl = (ULONG) pbuffl;  /* Each LONG will contain its own address */
        templ = *pbuffl; /* debug use */
        pbuffl++;
      }

    pbuffl = (ULONG *) (FRAME_BUFFER); /* Test 4MB VRAM with expected address */
    for (i=0; i<0x100000; i++)
      {
        templ = *pbuffl; /* debug use */
        if (templ != (ULONG) pbuffl)
          error_count++;
        if (error_count >= 1000)
          break;
        pbuffl++;
      }

    if (error_count >= 1000)           /* A large number of errors == 2Mb */
      return(0x9fffff);
    else
      return(0xbfffff);
  }

