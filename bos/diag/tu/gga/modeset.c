static char sccsid[] = "@(#)79	1.1  src/bos/diag/tu/gga/modeset.c, tu_gla, bos41J, 9515A_all 4/6/95 09:27:12";
/*
 *   COMPONENT_NAME: tu_gla
 *
 *   FUNCTIONS:
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

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/devinfo.h>
#include <sys/ioctl.h>
#include <sys/mode.h>
#include <sys/mdio.h>
#include <ggadds.h>
#include "ggapci.h"
#include "p9_video.h"
#include "p9rgb525.h"
#include "exectu.h"

char  p9_device     = WEITEK_P9100;
char  p9_dac_device = RGB_525;
char  p9_device_mem = 4;
char  firstt = TRUE;
PSH   *psh_ptr;
PSH2  *psh_ptr2;

/* External variables declared in ggamisc.h */

extern uint   display_mode;
extern ushort machine_type;
extern ulong  BE_prefix, LE_prefix, GGA_pcispace;
extern uint   gga_x_max, gga_y_max;

/*------------------------------------------------------------------------------*/
/*                                                                              */
/*      Initialize Weitek video controller                                      */
/*                                                                              */
/*      Input:                                                                  */
/*                                                                              */
/*           requested_mode = 201h ==>> 640x480x8                               */
/*           requested_mode = 205h ==>> 1024x768x8                              */
/*           requested_mode = 107h ==>> 1280x1024x8                             */
/*                                                                              */
/*------------------------------------------------------------------------------*/

uint modeset(int requested_mode)
  {
    VRB vrb, *vrb_ptr=&vrb;
    int   curDACIndex;
    uchar curColor;
    uchar shift4RGB;
    uchar currentDacVal;
    uint  Temp;
    int   j,i,s;
    char  *pData;
    ulong raster;
    ulong minterms, ltemp;
    uchar oldCD, saveFWc, rc;
    char ucpdata;
    uint  pdata;

    /* initialize base registers */
    if (init_P9100_base() == FAIL)
      return(FAIL);

    switch(requested_mode)
      {
        case 0x201:
          out32(P9100_SRTCTL2, 0x05000000);        /* set polarity to -,- */
          out32(P9100_SYSCNFG, DP9100_NHres640);    /* 640 x 480 for now */
          /* Setup the rgb525 pixel clock for 640x480  @ 72.81Hz 31.57Mhz MAX  VIDEO CLK */
          p9_rgb525_write( RGB525_F0, 0x0000003d);
          p9_rgb525_write( RGB525_PIXEL_FMT, 0x00000003);
          p9_rgb525_write( RGB525_MISC_CLK, 0x00000027);
          /* Wait for Weitek controller not busy */
          wait_for_wtkn_ready();
          out32(P9100_WIN_MAX, W640x480);
          out32(P9100_B_WIN_MAX, W640x480);
          out32(P9100_P_WIN_MAX, W640x480);

          /* set video controller timing registers  for 640 x 480 at 72.81z mode N*/
          out32(P9100_HRZSR, 0x11000000); /* vesa 0x04000000 */
          out32(P9100_HRZBR, 0x13000000);
          out32(P9100_HRZBF, 0x63000000);
          out32(P9100_HRZT,  0x67000000);
          out32(P9100_VRTSR, 0x03000000);
          out32(P9100_VRTBR, 0x17000000);
          out32(P9100_VRTBF, 0xf7010000);
          out32(P9100_VRTT,  0x08020000);
          enableRGB525();
          gga_x_max = 640;
          gga_y_max = 480;
          for (j=0; j<0x1000; j++)
            ltemp = in32(P9100_FRAME_BUF); /* delay */
          break;
        case 0x202:
          out32(P9100_SRTCTL2, 0x05000000);        /* set polarity to -,- */
          out32(P9100_SYSCNFG, DP9100_NHres640);    /* 640 x 480 for now */
          /* Setup the rgb525 pixel clock for 640x480  @ 60Hz 25.17Mhz MAX  VIDEO CLK */
          p9_rgb525_write( RGB525_F0, 0x00000024);
          p9_rgb525_write( RGB525_PIXEL_FMT, 0x00000003);
          p9_rgb525_write( RGB525_MISC_CLK, 0x00000027);
          /* Wait for Weitek controller not busy */
          wait_for_wtkn_ready();
          out32(P9100_WIN_MAX, W640x480);
          out32(P9100_B_WIN_MAX, W640x480);
          out32(P9100_P_WIN_MAX, W640x480);

          /* set video controller timing registers  for 640 x 480 at 25.17Mhz mode C*/
          out32(P9100_HRZSR, 0x0b000000); /* vesa 0x04000000 */
          out32(P9100_HRZBR, 0x0c000000);
          out32(P9100_HRZBF, 0x5c000000);
          out32(P9100_HRZT,  0x63000000);  /* vs 61 as per calcs  */
          out32(P9100_VRTSR, 0x02000000);
          out32(P9100_VRTBR, 0x22000000);  /* vs 1b as per calcs  */
          out32(P9100_VRTBF, 0x02020000);  /* vs 1fb as per calcs */
          out32(P9100_VRTT,  0x14020000);  /* vs 211 as per calcs */
          enableRGB525();
          gga_x_max = 640;
          gga_y_max = 480;
          for (j=0; j<0x1000; j++)
            ltemp = in32(P9100_FRAME_BUF); /* delay */
          break;

        case 0x205:
          in32(P9100_FRAME_BUF+0x00000100);         /* hw bug           */
          out32(P9100_SRTCTL2, 0x00000000);         /* set polarity to +,+ */
          in32(P9100_FRAME_BUF+0x00000100);          /* hw bug           */
          out32(P9100_SYSCNFG, P9100_NHres1024);    /* 1024 x 768       */
          in32(P9100_FRAME_BUF+0x00000100);          /* hw bug           */

          /* Setup the pixel clock for 1024x768  @ 75.6Hz 80Mhz  MAX  VIDEO CLK */
          p9_rgb525_write( RGB525_F0, 0x0000008f);

          /* If its the 2Mb card adjust bank size and blank delay */
          if (p9_device_mem==2)
            out32(P9100_MEMCNFG,0x3d0008c8);        /* rey 8/25 */

          /* If its the 2Mb card at 50Mhz forget spec and fudge */
          if ((p9_device_mem==2) && (in32le(P9100_PU_CONFIG) & 0x00000200))
            p9_rgb525_write(RGB525_F0, 0x000000a0);

          p9_rgb525_write( RGB525_PIXEL_FMT, 0x00000003);
          p9_rgb525_write( RGB525_MISC_CLK, 0x00000027);
          /* Wait for Weitek controller not busy */
          wait_for_wtkn_ready();
          out32(P9100_WIN_MAX, W1024x768);
          out32(P9100_B_WIN_MAX, W1024x768);
          out32(P9100_P_WIN_MAX, W1024x768);

          /* set video controller timing registers for 1024 x 768 at 72 Hz NI */
          out32(P9100_HRZSR   , 0x23000000);
          out32(P9100_HRZBR   , 0x25000000);
          out32(P9100_HRZBF   , 0xa5000000);
          out32(P9100_HRZT    , 0xab000000);
          out32(P9100_VRTSR   , 0x08000000);
          out32(P9100_VRTBR   , 0x26000000);
          out32(P9100_VRTBF   , 0x25030000);
          out32(P9100_VRTT    , 0x26030000);
          enableRGB525();
          gga_x_max = 1024;
          gga_y_max = 768;
          for (j=0; j<0x1000; j++)
            ltemp = in32(P9100_FRAME_BUF); /* delay */
          break;
        case 0x107:
          out32(P9100_SRTCTL2, 0x05000000);        /* set polarity to -,- */
          in32(P9100_FRAME_BUF+0x00000100);
          out32le(P9100_SYSCNFG, 0x28403000);   /* weitek    */
          in32(P9100_FRAME_BUF+0x00000100);          /* hw bug           */

          /* Setup the pixel clock for 1280x1024  @ 60Hz 111.5Mhz  MAX  VIDEO CLK */
          p9_rgb525_write( RGB525_F0, 0x000000af);

          /* If its a 2Mb card - fudge. Also adjust for the blank delay */
          if (p9_device_mem==2)
            {
            p9_rgb525_write( RGB525_F0, 0x000000b5);
            out32(P9100_MEMCNFG,0x3d0008c8);        /* rey 8/25 */
            }
          p9_rgb525_write(RGB525_F0, 0x000000af);
          p9_rgb525_write(RGB525_F0, 0x000000ac);

          p9_rgb525_write( RGB525_PIXEL_FMT, 0x00000003);
          p9_rgb525_write( RGB525_MISC_CLK, 0x00000027);
          /* Wait for Weitek controller not busy */
          wait_for_wtkn_ready();
          out32(P9100_WIN_MAX,   W1280x1024);
          out32(P9100_B_WIN_MAX, W1280x1024);
          out32(P9100_P_WIN_MAX, W1280x1024);

          /* set video controller timing registers for 1280 x 1024 at 60 Hz NI mode MM */
          if (p9_device_mem == 4)
            {
              out32(P9100_HRZSR   , 0x18000000);
              out32(P9100_HRZBR   , 0x3a000000);
              out32(P9100_HRZBF   , 0xda000000);
              out32(P9100_HRZT    , 0xdb000000);
              out32(P9100_VRTSR   , 0x03000000);
              out32(P9100_VRTBR   , 0x1d000000);
              out32(P9100_VRTBF   , 0x1d040000);
              out32(P9100_VRTT    , 0x20040000);
            }
          else   /* p9_device_mem == 2 */
            {
              out32(P9100_HRZSR   , 0x18000000);
              out32(P9100_HRZBR   , 0x34000000);
              out32(P9100_HRZBF   , 0xd4000000);
              out32(P9100_HRZT    , 0xdc000000);
              out32(P9100_VRTSR   , 0x03000000);
              out32(P9100_VRTBR   , 0x1d000000);
              out32(P9100_VRTBF   , 0x1d040000);
              out32(P9100_VRTT    , 0x1e040000);
            }
          enableRGB525();
          gga_x_max = 1280;
          gga_y_max = 1024;
          for (j=0; j<0x1000; j++)
            ltemp = in32(P9100_FRAME_BUF); /* delay */
          break;
        case 0x274:
          out32(P9100_SRTCTL2, 0x00000000);        /* set polarity to +,+ */
          in32(P9100_FRAME_BUF+0x00000100);
          out32le(P9100_SYSCNFG, 0x086a8000);      /* weitek    */
          in32(P9100_FRAME_BUF+0x00000100);        /* hw bug           */

          /* set clock as per spec */
          p9_rgb525_write( RGB525_F0, 0x000000d4);
          p9_rgb525_write( RGB525_PIXEL_FMT, 0x00000003);
          p9_rgb525_write( RGB525_MISC_CLK, 0x00000027);

          /* Wait for Weitek controller not busy */
          wait_for_wtkn_ready();
          out32(P9100_WIN_MAX,   W1600x1280);
          out32(P9100_B_WIN_MAX, W1600x1280);
          out32(P9100_P_WIN_MAX, W1600x1280);

          /* set video controller timing registers for 1600 x 1280 at 60 Hz NI mode MM */
          /*  based on memory size  */
          out32(P9100_HRZSR   , 0x17000000);
          out32(P9100_HRZBR   , 0x38000000);
          out32(P9100_HRZBF   , 0x00010000);
          out32(P9100_HRZT    , 0x05010000);
          out32(P9100_VRTSR   , 0x0a000000);
          out32(P9100_VRTBR   , 0x25000000);
          out32(P9100_VRTBF   , 0x25050000);
          out32(P9100_VRTT    , 0x26050000);
          enableRGB525();
          gga_x_max = 1600;
          gga_y_max = 1280;
          for (j=0; j<0x1000; j++)
            ltemp = in32(P9100_FRAME_BUF); /* delay */
          break;
      }

    /* Wait for Weitek controller not busy */
    wait_for_wtkn_ready();

    Temp = in32(P9100_SRTCTL) | enableVid;
    ltemp = in32(P9100_FRAME_BUF+0x00000100);          /* hw bug  */
    out32((P9100_SRTCTL), Temp); /* turn video on */

    Temp  = in32le(P9100_MEMCNFG);
    Temp |= 0x00100000;
    ltemp = in32(P9100_FRAME_BUF+0x00000100);
    out32le(P9100_MEMCNFG,Temp);

    display_mode = requested_mode;
  }


uint init_P9100_base(void)
  {
    int   j, i, jjj;
    uint  ui;
    int   raster, rc, fd;
    ulong ltemp;

    if (firstt==TRUE)  
      {
        /*** Test for 4Mb card ***/
        ltemp = get_end_of_frame_buffer();
        if (ltemp == 0x9fffff)
          p9_device_mem = 2;
        else
          p9_device_mem = 4;

        firstt = FALSE;
      }

    i = 0;
    while (rgb525_init_tab[i].addr != 0)
      {
        p9_rgb525_write(rgb525_init_tab[i].addr,
                        rgb525_init_tab[i].data);
        i++;
      } /* endwhile */


    ltemp = in32(P9100_FRAME_BUF+0x00000100);          /* hw bug         */
    out32(P9100_SRTCTL, 0xC4030000);           /* diamond 1 e403 */
    ltemp = in32(P9100_FRAME_BUF+0x00000100);          /* hw bug         */
    out32(P9100_MEMCNFG,0x3f0048d1);           /* DIAMON 1       */
    if (in32le(P9100_PU_CONFIG) & 0x00000200)
      out32(P9100_MEMCNFG,0x3f0048c9);         /* rey  8/23   */

    /* initialize more video control registers */
    ltemp = in32(P9100_FRAME_BUF+0x00000000);          /* hw bug  */
    out32(P9100_INT_EN, 0x80000000);     /* disable P9100 interrupts       */
    ltemp = in32(P9100_FRAME_BUF+0x00000100);          /* hw bug  */
    out32(P9100_PREHRZC, 0x00000000);    /* set horizontal counter preload */
    ltemp = in32(P9100_FRAME_BUF+0x00000100);          /* hw bug  */
    out32(P9100_PREVRTC, 0x00000000);    /* set vertical counter preload   */

    /* initialize vram control registers */
    out32(P9100_RFPER, 0x88010000);      /* set refresh period             */
    out32(P9100_RLMAX, 0xfa000000);      /* set max time  for RAS- asserted */

    if (p9_device_mem==2)
      {
        ltemp = in32(P9100_FRAME_BUF+0x00000100);          /* hw bug           */
        out32(P9100_SRTCTL, 0xc3010000);           /* diamond 1 7/13   */
        ltemp = in32(P9100_FRAME_BUF+0x00000100);          /* hw bug           */
        out32(P9100_MEMCNFG,0x3d0008c8);           /* DIAMON    7/13   */
        if (in32le(P9100_PU_CONFIG) & 0x00000200)
          out32(P9100_MEMCNFG,0x3d0008c8);        /* rey  8/23   */
      }
    /* Wait for Weitek controller not busy */
    wait_for_wtkn_ready();

    /* initialize raster register for non-transperancy */
    raster =  PatrnDisable | 0xf0000100;

    /* Wait for Weitek controller not busy */
    wait_for_wtkn_ready();

    out32(P9100_RASTER, raster);

    /* Wait for Weitek controller not busy */
    wait_for_wtkn_ready();

    out32(P9100_PLANE_MASK, 0xffffffff);

    /* Wait for Weitek controller not busy */
    wait_for_wtkn_ready();

    out32(P9100_Draw_Mode, 0xa0000000);

    out32(P9100_COOR_OFFS, 0x00000000);

    /* Wait for Weitek controller not busy */
    wait_for_wtkn_ready();

    out32(P9100_WIN_MIN, 0x00000000);
    out32(P9100_B_WIN_MIN, 0x00000000);
    out32(P9100_P_WIN_MIN, 0x00000000);

    return(PASS);
  }


void p9_rgb525_write(ushort index, uchar data)
  {
    uint  RepdData;
    ulong ltemp;

    RepdData  = index & 0x00ff;
    RepdData |= (RepdData << 8) | (RepdData << 16) | (RepdData << 24);
    ltemp = in32(P9100_FRAME_BUF+0x00000200);          /* hw bug  */
    out32(P9100_IndexLow, RepdData);
    ltemp = in32(P9100_PU_CONFIG); /* ensure 5 clocks between DAC accesses */

    RepdData  = ( index & 0xff00 ) >> 8;
    RepdData |= (RepdData << 8) | (RepdData << 16) | (RepdData << 24);
    ltemp = in32(P9100_FRAME_BUF+0x00000200);          /* hw bug  */
    out32(P9100_IndexHigh, RepdData);
    ltemp = in32(P9100_PU_CONFIG); /* ensure 5 clocks between DAC accesses */

    RepdData  = data;
    RepdData |= (RepdData << 8) | (RepdData << 16) | (RepdData << 24);
    ltemp = in32(P9100_FRAME_BUF+0x00000200);          /* hw bug  */
    out32(P9100_IndexData, RepdData);
    ltemp = in32(P9100_PU_CONFIG); /* ensure 5 clocks between DAC accesses */

    return;
  }


uint p9_rgb525_read(ushort index)
  {
    uint  RepdData;
    uchar data;
    int idata;
    ulong ltemp;

    RepdData  = index & 0x00ff;
    RepdData |= (RepdData << 8) | (RepdData << 16) | (RepdData << 24);
    ltemp = in32(P9100_FRAME_BUF+0x00000200);          /* hw bug  */
    out32(P9100_IndexLow, RepdData);
    ltemp = in32(P9100_PU_CONFIG); /* ensure 5 clocks between DAC accesses */

    RepdData  = ( index & 0xff00 ) >> 8;
    RepdData |= (RepdData << 8) | (RepdData << 16) | (RepdData << 24);
    ltemp = in32(P9100_FRAME_BUF+0x00000200);          /* hw bug  */
    out32(P9100_IndexHigh, RepdData);
    ltemp = in32(P9100_PU_CONFIG); /* ensure 5 clocks between DAC accesses */

    ltemp = in32(P9100_FRAME_BUF+0x00000200);          /* hw bug  */
    idata = in32(P9100_IndexData);
    return ((uchar) (idata>>8));
  }


void SetRGB525PixelClk(void)
  {
    return;
  }


void enableRGB525(void)
  {
    ulong ltemp;

    ltemp = in32(P9100_FRAME_BUF+0x00000200);          /* hw bug  */
    out32le(P9100_PixelMask,   0xffffffff);
  }

