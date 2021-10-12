static char sccsid[] = "@(#)44  1.4  src/bos/diag/tu/sun/tscaler.c, tu_sunrise, bos411, 9437A411a 9/13/94 08:44:14";
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: init_tscaler
 *              tscaler
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************

Function(s) - Scaler Test

Module Name :  tscaler.c

Test the scaling function of pixel 2070.

*****************************************************************************/
#include <stdio.h>
#include "sun_tu_type.h"
#include "error.h"
#include "suntu.h"
#include "iodvp.h"
#include "px2070.h"

#undef DISPLAY          /* This is for engr debug purpose if want to see      */
                        /* the scaled image on the X-window:                  */
                        /* Need to have file vidinwin.c and define:           */
                        /*      #define CIF                                   */
                        /*      #undef  LINEREP                               */
#ifdef DISPLAY
#include "video_size.h"
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

/*****************************************************************************/
/*  init_tscaler()                                                           */
/*                                                                           */
/*  FUNCTION:                                                                */
/*    Move the image data from the frame buffer through IPU1, scaling by     */
/*    half, and store the resulting scaled image in the frame buffer.        */
/*                                                                           */
/*  INPUT:    None                                                           */
/*                                                                           */
/*  OUTPUT:   return value     - OK if successful, ERROR if NOT.             */
/*****************************************************************************/
int init_tscaler()
{
  int iXSize = 320;       /* size of scaled image */
  int iYSize = 240;
  int iXRef  = 320;
  int rc;

  /* Reset the 2070*/
  if (rc = SoftResetDVP()) return(rc);

  /* Disable Units*/
  if (rc = ioRDTWriteDVP(i_VPU_MCR,0)) return (rc);
  UpdateGen();

  /* halt sequencer*/
  if (rc=SeqHalt()) return(rc);

  /* Clear out the fifos*/
  if (rc=ResetFifo(ALL_FIFOS)) return(rc);

  /* Set up the MMU*/
  if (rc=SetUpMMU()) return(rc);

  /* Disable all objects*/
  if (rc=OBReset(ALL_OBJECTS)) return(rc);

  if (rc=UpdateGen()) return(rc);

  /* setup datapath controls*/
  if (rc = ioRDTWriteDVP(i_VIU_DPC1, IPU1_from_OP + OPU_LoopBack +VSU_LoopBack)) return (rc);
  if (rc = ioRDTWriteDVP(i_VIU_DPC2, IPU1_from_OP + OPU_LoopBack +VSU_LoopBack)) return (rc);

  /* OB0 - input */
  if (rc = Setup_OB (OBU0_MCR, (0x0400 + OB_Normal + OB_XBLT + OB_YBLT +
                     OB_No_Copy+ OB_SSM), iXRef*2, 0, 0,iXSize*2, iYSize*2, 0))
     return(rc);
  /* OB1 - output */
  if (rc = Setup_OB (OBU1_MCR, (0x0400 + OB_Normal  +  OB_XBLT + OB_YBLT +
                     OB_No_Copy+OB_SSM), iXRef, 5, 0, iXSize, iYSize, 0))
     return(rc);

  /* offset,mcrval,xbi,xei,ybi,yei*/
  if (rc = Setup_Ip1 (i_IPU1_F1_BASE,
                     (0x0 /*+FieldPol + Interlaced*/ + ColorConvert + Out332) , /* MCR*/
                      1+0,0,         /* XStart*/
                      iXSize*2+0,       /* XStop*/
                      2,0,
                      1+2,0,         /* YStart*/
                      (iYSize*2)+2,        /* YStop*/
                      2,0
                      ))
      return(rc);

  /* offset,mcrval,xbi,xei,ybi,yei*/
  if (rc = Setup_Ip1 (i_IPU1_F2_BASE,
                      (0x0 /*+FieldPol + Interlaced*/ + ColorConvert + Out332) , /* MCR*/
                      1+0,0,         /* XStart*/
                      iXSize*2+0,       /* XStop*/
                      2,0,
                      1+2,0,         /* YStart*/
                      (iYSize*2)+2,        /* YStop*/
                      2,0
                      ))
      return(rc);

  /* Setup the OPU */
  if (rc = ioRDTWriteDVP(OPU_MCR1, /*Interlaced +FieldPol+ */OP_BLANK_OUT)) return (rc);
  if (rc = ioRDTWriteDVP(OPU_YEI1,iYSize*2)) return (rc);
  if (rc = ioRDTWriteDVP(OPU_MCR2, /*Interlaced +FieldPol+ */OP_BLANK_OUT)) return (rc);
  if (rc = ioRDTWriteDVP(OPU_YEI2,iYSize*2)) return (rc);
  if (rc = ioRDTWriteDVP(OPU_XBI1,1)) return (rc);
  if (rc = ioRDTWriteDVP(OPU_XEI1,iXSize*2)) return (rc);
  if (rc = ioRDTWriteDVP(OPU_YBI1,1)) return (rc);
  if (rc = ioRDTWriteDVP(OPU_XBI2,1)) return (rc);
  if (rc = ioRDTWriteDVP(OPU_XEI2,iXSize*2)) return (rc);
  if (rc = ioRDTWriteDVP(OPU_YBI2,1)) return (rc);

  /* hsw,had,hap,hp, vsw,vad,vap,vp, vsu_disable */
  if (rc = Setup_VSU (4,9,iXSize*2+4,4+9+iXSize*2+10,1,1,iYSize*2,
                      iYSize*2+1+1+5, VSU_Enable)) return(rc);

  if (rc = EnableFifo(ALL_FIFOS)) return(rc);

  /* Sequencer start*/

  if (rc = ioRDTWriteDVP(SIU_SIM0,(OB0_to_OP + offset_1 ))) return (rc);
  if (rc = ioRDTWriteDVP(SIU_SIM1,(IPU1_to_OB1 + offset_M1 ))) return (rc);

  if (rc = ioRDTWriteDVP(SIU_SIM11,(OB0_to_OP + offset_1 ))) return (rc);
  if (rc = ioRDTWriteDVP(SIU_SIM12,(IPU1_to_OB1 + offset_M1 ))) return (rc);

  if (rc = ioRDTWriteDVP(i_SIU_MCR,(SIU_Start1 + SIU_No_Toggle /* SIU_SI1_to_Fld1 */
            + (SIU_SI1 * 0 )+ (SIU_SI2 * 11)))) return (rc);

  if (rc = ioRDTWriteDVP(i_VIU_WDT, VIU_MField_vsu)) return (rc);
  if (rc = ioOCSWriteDVP(0x0380)) return (rc);

  sleep(1);      /* Required:  to fix the scaling problem of crc=0 */

  if (rc = ioRDTWriteDVP(i_VPU_MCR, IP1_Start_Next + OP_Start_Next /*+ IP2_F1_Only*/)) return (rc);
  if (rc = ioOCSWriteDVP(0x0380)) return (rc);

  /* give time to scale it and stuff it into the frame buffer */
  sleep(1);

  SeqHalt();

  return(OK);
} /* End of init_tscaler() */

#define         BUFFSIZE        19200  /* Buffer for scaled data for 8-bit RGB */
                                       /* = 320x240/4 words                    */
#define         PATTERN_SIZE    8
#define         CRC_RESULT      0xceb7c502      /* CRC value of the scaled    */
                                                /* image, dependent on the    */
                                                /* given data image           */
/*****************************************************************************/
/*  tscaler()                                                                */
/*                                                                           */
/*  FUNCTION:                                                                */
/*    This will test the scaling function of 2070:                           */
/*    Write a given data image (640x480) to the top portion of the frame     */
/*    buffer, use the px2070 to scale the image by half, store the           */
/*    scaled image (320x240) to the bottom portion of the frame buffer,      */
/*    read the scaled image data and verify its checksum.                    */
/*                                                                           */
/*  INPUT:    None                                                           */
/*                                                                           */
/*  OUTPUT:   return value     - OK if successful, ERROR if NOT.             */
/*****************************************************************************/
int tscaler()
{
  int i, rc, zero_data=0;
  unsigned long *dma_block_ptr;
  unsigned long crcvalue;
  unsigned int test_pattern[8]={0xbe80be80,0xaa2caa8e,0x8a9a8a2a,0x7747773a,
                                0x58b758c7,0x466346d5,0x24d32472,0x00800080};
  ERROR_DETAILS error;    /* Structure used to load with error
                             information and passed to the
                             log_error() function in order to log an
                             error. */

  if (rc = pxinitialize())       /* Initialize the 2070 */
  {
     LOG_CRC_ERROR(ERRORSCALE_2070INIT,rc,0,0);
  }
  /* Write the top portion (start address 0) of frame buffer with given pattern */
  if (rc = DMAWrite(640,480,0x0,test_pattern,PATTERN_SIZE,40,WORD_BYTESWAP))
  {
    LOG_CRC_ERROR(ERRORSCALE_WRITEDATA,rc,0,0);
  }
  if (rc = pxinitialize())       /* Initialize the 2070 */
  {
     LOG_CRC_ERROR(ERRORSCALE_2070INIT,rc,0,0);
  }
  /* Write the rest of frame buffer (start address 0x4b000 with 0 */
  if (rc = DMAWrite(640,480,0x4b000,&zero_data,1,1,WORD_BYTESWAP))
  {
    LOG_CRC_ERROR(ERRORSCALE_WRITEZERO,rc,0,0);
  }

  /* Setup scaler to scale the image */
  if (rc = init_tscaler())
  {
    LOG_CRC_ERROR(ERRORSCALE_INIT,rc,0,0);
  }

  if ((dma_block_ptr = (unsigned long *) malloc(4*BUFFSIZE)) == NULL)
  {
    LOG_CRC_ERROR(ERRORSCALE_ALLOCATE,0,0,0);
  }
  /* Read scaled image data from the frame buffer */
  if (rc = DMAReadWithData(320,120,0x50000,dma_block_ptr,WORD_BYTESWAP))
  {
    free(dma_block_ptr);
    LOG_CRC_ERROR(ERRORSCALE_READDATA,rc,0,0);
  }

#ifdef DISPLAY
  initializeX();
  displayImage(dma_block_ptr);
#endif

  crcvalue = 0;
  for (i=0;i<BUFFSIZE;i++) {    /* initialize host buffer */
    crcvalue=cksum_32(dma_block_ptr[i],&crcvalue);        /* Accumulate 32 bit crc on 32bit(sz=4) value */
  } /* endfor */
  free(dma_block_ptr);

  DEBUG_1 ("crc=0x%x\n", crcvalue);
  if (crcvalue != CRC_RESULT)
  {
    LOG_CRC_ERROR(ERRORSCALE_CRCCHK,rc,CRC_RESULT,crcvalue);
  }

  return(OK);
} /* End of tscaler() */

