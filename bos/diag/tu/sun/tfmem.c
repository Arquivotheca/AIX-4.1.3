static char sccsid[] = "@(#)40  1.8  src/bos/diag/tu/sun/tfmem.c, tu_sunrise, bos411, 9437A411a 9/13/94 08:44:04";
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: TestFieldMem
 *              V1In
 *              V1Out
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

/**********************************************************
*
*       File Name:  tfmem.c
*
*       Module Abstract: This module is the high level test
*               of the field memory
*
*********************************************************/

/* #includes --------------------------------------------*/
#include <stdio.h>

#include "iodvp.h"
#include "px2070.h"
#include "suntu.h"
#include "error.h"
#include "sun_tu_type.h"

#define XSIZE           640
#define YSIZE           480
#define PATTERN_SIZE    4

/*****************************************************************************/
/*  TestFieldMem()                                                           */
/*                                                                           */
/*  FUNCTION:                                                                */
/*    This test does the followings:                                         */
/*      - Write the top portion (start address 0) of frame buffer with a     */
/*        given pattern (640x480) image by DMA.                              */
/*      - Output the image from the buffer to the pixel bus thru V1 OUTPUT   */
/*        port of 2070.  Capture the image to field memory.                  */
/*      - Output the image from field memory back to the pixel bus.          */
/*      - Set the V1 port as input and store the image back to the frame     */
/*        buffer at the bottom half (start address 0x4c000).                 */
/*      - DMA Read the bottom half of frame buffer and verify the data.      */
/*                                                                           */
/*  INPUT:    None                                                           */
/*                                                                           */
/*  OUTPUT:   return value     - OK if DMA verify sucessful, ERROR if NOT.   */
/*****************************************************************************/
int TestFieldMem()
{
  int rc, zero_data=0;
  int i, j, k, dma_block_size, BuffSize, line_adjust, size_adjust;
  unsigned long *DMAReadData;
  int xsize=640;      /* Setup to test the entire field memory 640x240 */
  int ysize=240;
  int number_rep=1;

/*  Alternate pattern to use if want to .....
  unsigned int test_pattern[PATTERN_SIZE]={0x11223344,0x55667788,0x99aabbcc,
                                           0xddeeff00,0xffffffff,0x00000000,
                                           0x11111111,0x22222222,0x33333333,
                                           0x44444444,0x55555555,0x66666666,
                                           0x77777777,0x88888888,0x99999999,
                                           0xaaaaaaaa};
*/

  unsigned int test_pattern[PATTERN_SIZE]={0x11223344,0x55667788,0x99aabbcc,
                                           0xddeeff00};

  /* Setup front-end video */
  if (rc = vidsetup(NTSC_NOINPUT))
  {
     LOG_MEM_ERROR(ERRORFMEM_VIDSETUP,rc,0,0,0);
  }

  if (rc = pxinitialize())      /* Initialize the 2070 */
  {
     LOG_MEM_ERROR(ERRORFMEM_2070INIT,rc,0,0,0);
  }

  /* Write the top portion (start address 0) of frame buffer with given pattern */
  if (rc = DMAWrite(640,480,0x0,test_pattern,PATTERN_SIZE,number_rep,WORD_BYTESWAP))
  {
     LOG_MEM_ERROR(ERRORFMEM_WRITEDATA,rc,0,0,0);
  }
  if (rc = pxinitialize())       /* Initialize the 2070 */
  {
     LOG_MEM_ERROR(ERRORFMEM_2070INIT,rc,0,0,0);
  }
  /* Write the rest of frame buffer (start address 0x4b000 with 0 */
  if (rc = DMAWrite(640,480,0x4b000,&zero_data,1,1,WORD_BYTESWAP))
  {
     LOG_MEM_ERROR(ERRORFMEM_WRITEZERO,rc,0,0,0);
  }

  /* Move stored data from top portion of frame buffer to the pixel bus */
  if (rc=V1Out(0x0))
  {
     LOG_MEM_ERROR(ERRORFMEM_V1OUT,rc,0,0,0);
  }
  sleep(1);        /* wait for pixel data ready */

  /* Store data to field memory */
#ifdef CIF
  pio_write(LBControl,0x4000002a,1); /* for CIF */
#endif
  if (rc = pio_write(LBControl,0x4000ac0a,1))
  {
     LOG_MEM_ERROR(ERRORFMEM_STOREFIELD,rc,0,0,0);
  }
  usleep(500000);        /* wait for data to store to field memory */

  /* Freeze the field memory */
#ifdef CIF
  pio_write(LBControl,0x4000003a,1); /* for CIF */
#endif
  if (rc = pio_write(LBControl,0x4000ac18,1))
  {
     LOG_MEM_ERROR(ERRORFMEM_FREEZEFIELD,rc,0,0,0);
  }
  usleep(500000);        /* wait for field memory being freezed */

  /* Display field memory to output port which transfer field memory */
  /* data to the pixel bus */
  if (pio_write(LBControl,0x4000ac40,1))
  {
     LOG_MEM_ERROR(ERRORFMEM_DISPLAYFIELD,rc,0,0,0);
  }
#ifdef CIF
  pio_write(LBControl,0x40000062,1); /* for CIF */
#endif
  usleep(500000);

  /* Transfer the data from pixel bus (which was in the field memory) */
  /* back to the bottom portion of frame buffer for data verification */
  if (rc=V1In(0x4c000))
  {
     LOG_MEM_ERROR(ERRORFMEM_V1IN,rc,0,0,0);
  }
  sleep(1);        /* wait for data to store to field memory */

  /* Setup the DMA buffer */
  BuffSize = (xsize*ysize)/2;
  dma_block_size = 4*BuffSize;
  if ((DMAReadData = (unsigned long *)malloc(dma_block_size)) == NULL)
  {
     LOG_MEM_ERROR(ERRORFMEM_ALLOCATE,rc,0,0,0);
  }

  /* DMA read back the data from the frame buffer */
  if (rc = DMAReadWithData(xsize,ysize,0x4c000,DMAReadData,WORD_BYTESWAP))
  {
     free(DMAReadData);
     LOG_MEM_ERROR(ERRORFMEM_READDATA,rc,0,0,0);
  }

  /* Find the valid start of field memory data */
  i = PATTERN_SIZE*number_rep;
  for (j=0;j<(PATTERN_SIZE*number_rep) ;j++ ) {
    if (DMAReadData[i++] == test_pattern[0]) {
      i--;      /* i contains the valid starting point of field memory */
      j = PATTERN_SIZE*number_rep; /* exit for loop */
    }
  } /* endfor */

  size_adjust = BuffSize - (14*xsize/2); /* This is due to the way the 2070  */
                                         /* setup to transfer data that the  */
                                         /* last 14 lines will have invalid  */
                                         /* data (see Setup_Ip1 call in      */
                                         /* V1In() routine)                  */

  /* Verify data */
  while (i<size_adjust) {
     for (j=0;j<PATTERN_SIZE ;j++ ) {
       for (k=0;k<number_rep ;k++ ) {
          if (DMAReadData[i++] != test_pattern[j]) {
            /* Need to do line adjustment due to 2070 not writing 6 pixels    */
            /* at start of line during v1out() routine.  This is due to       */
            /* the time it takes to move data from fb to the pixel bus        */
            /* since using the sync from the 7191 and NOT the 2070            */
            line_adjust = i%(xsize/2);
            if ((line_adjust!=1) && (line_adjust!=2) && (line_adjust!=3)) {
              pio_write(LBControl,0x4000ac40,1);  /* display field memory for */
                                                   /* screen verifying */
              DEBUG_3 ("Failed compare test: expected=0x%x, received=0x%x, with i=%d\n",
                       test_pattern[j], DMAReadData[i-1], i-1);
              free(DMAReadData);
              LOG_MEM_ERROR(ERRORFMEM_COMPARE,0,test_pattern[j],
                            DMAReadData[i-1],i-1);
            } /* endif */
          }
          if (i>=BuffSize) {              /* check to make sure 'i' not       */
                                          /* overrun the 'DMAReadData'        */
            k=number_rep; j=PATTERN_SIZE; /* if yes, exit                     */
          } /* endif */
       } /* endfor */
     } /* endfor */
  } /* endwhile */

  free(DMAReadData);

  if (rc = pxinitialize())      /* Initialize the 2070 */
  {
     LOG_MEM_ERROR(ERRORFMEM_2070INIT,rc,0,0,0);
  }

  return (OK);
}  /*  End TestFieldMem */

/*****************************************************************************/
/*  V1In()                                                                   */
/*                                                                           */
/*  FUNCTION:                                                                */
/*    Set the V1 port of 2070 as INPUT.                                      */
/*                                                                           */
/*  INPUT:    fb_startaddr     - start address of frame buffer to store data */
/*                                                                           */
/*  OUTPUT:   return value     - OK if successful, ERROR if NOT.             */
/*****************************************************************************/
int V1In(unsigned int fb_startaddr)
{
  int rc;
  int iXSize,iYSize,iXRef;
  unsigned int fb_startaddr_high, fb_startaddr_low;

  iXSize = XSIZE;
  iYSize = YSIZE;
  iXRef  = XSIZE;

  fb_startaddr_low = fb_startaddr & 0xffff;
  fb_startaddr_high= (fb_startaddr & 0xffff0000) >> 16;

  /*  halt sequencer */
  if (rc=SeqHalt()) return(rc);

  /*  Clear out the fifos */
  if (rc=ResetFifo(ALL_FIFOS)) return(rc);

  /*  Enable the fifos */
  if (rc=EnableFifo(ALL_FIFOS)) return(rc);

  /*  Set up the MMU */
  if (rc=SetUpMMU()) return(rc);

  /*  Disable all objects */
  if (rc=OBReset(ALL_OBJECTS)) return(rc);

  if (rc = ioRDTWriteDVP(i_VPU_MCR,0)) return (rc);
  if (rc = UpdateGen()) return(rc);

  /*****************************************
  * Now that we have an interlaced object,
  * configure things for input.
  ******************************************/

  /* Setup Video Port 1 as input */
  if (rc = ioRDTWriteDVP(i_VIU_MCR1,VIU_oss_vp + VIU_ihsp + VIU_ivsp + VIU_InOnly)) return (rc);

  /* setup datapath controls */
  if (rc = ioRDTWriteDVP(i_VIU_DPC1, IPU1_from_Vp1_Phase)) return (rc);
  if (rc = ioRDTWriteDVP(i_VIU_DPC2, IPU1_from_Vp1_Phase)) return (rc);

  /* Setup Object */
  if (rc = Setup_OB (OBU0_MCR, (OB_Normal + OB_XBLT + OB_YBLT +OB_No_Copy+ OB_SSM),
                     iXRef, fb_startaddr_high, fb_startaddr_low, iXSize,iYSize, 0))
     return(rc);

  /* offset,mcrval,xbi,xei,ybi,yei*/
   if (rc = Setup_Ip1 (i_IPU1_F1_BASE,
             (FieldPol + Interlaced + /*Gamma +*/ Out422NoTag) , /* MCR*/
              1,0,         /* XStart*/
              iXSize,       /* XStop*/
              1,0,
              1+23,0,         /* YStart*/
              iYSize+23,        /* YStop*/
#ifdef NEEDED
              1,0,         /* YStart*/
              iYSize,        /* YStop*/
#endif
              1,0
              ))
     return(rc);
  /* offset,mcrval,xbi,xei,ybi,yei*/
  if (rc = Setup_Ip1 (i_IPU1_F2_BASE,
             (FieldPol + Interlaced /*+ Gamma*/ + Out422NoTag) , /* MCR*/
             1,0,         /* XStart*/
             iXSize,       /* XStop*/
             1,0,
             1+23,0,         /* YStart*/
             iYSize+23,        /* YStop*/
#ifdef NEEDED
              1,0,         /* YStart*/
              iYSize,        /* YStop*/
#endif
             1,0
             ))
     return(rc);

  /* Sequencer start*/
  if (rc = ioRDTWriteDVP(SIU_SIM0,(IPU1_to_OB0 + offset_0 + SIU_EXIT ))) return (rc);

  if (rc = ioRDTWriteDVP(i_SIU_MCR,(SIU_Start1 + SIU_No_Toggle
            + (SIU_SI1 * 0 )))) return (rc);

  if (rc = ioRDTWriteDVP(i_VIU_WDT,VIU_MField_V1i)) return (rc);
  if (rc = UpdateGen()) return(rc);

  /* Enable IP1 */
  if (rc = ioRDTWriteDVP(i_VPU_MCR,IP1_F1_Only)) return (rc);
  if (rc=UpdateGen())  return(rc);

  return(OK);
} /* End of V1In */

/*****************************************************************************/
/*  V1Out()                                                                  */
/*                                                                           */
/*  FUNCTION:                                                                */
/*    Set the V1 port of 2070 as OUTPUT.                                     */
/*                                                                           */
/*  INPUT:    fb_startaddr     - start address of frame buffer to output data*/
/*                                                                           */
/*  OUTPUT:   return value     - OK if successful, ERROR if NOT.             */
/*****************************************************************************/
int V1Out(unsigned int fb_startaddr)
{
  int rc;
  int iXSize,iYSize,iXRef;
  unsigned int fb_startaddr_high, fb_startaddr_low;

  iXSize = XSIZE;
  iYSize = YSIZE;
  iXRef  = XSIZE;

  fb_startaddr_low = fb_startaddr & 0xffff;
  fb_startaddr_high= (fb_startaddr & 0xffff0000) >> 16;

  /*  halt sequencer */
  if (rc=SeqHalt()) return(rc);

  /*  Clear out the fifos */
  if (rc=ResetFifo(ALL_FIFOS)) return(rc);

  /*  Enable the fifos */
  if (rc=EnableFifo(ALL_FIFOS)) return(rc);

  /*  Set up the MMU */
  if (rc=SetUpMMU()) return(rc);

  /*  Disable all objects */
  if (rc=OBReset(ALL_OBJECTS)) return(rc);

  if (rc = ioRDTWriteDVP(i_VPU_MCR,0)) return (rc);
  if (rc = UpdateGen()) return(rc);

  /*****************************************
  * Now that we have an interlaced object,
  * configure things for output.
  ******************************************/

  /* Set control reg for V1 output */
  if (rc = pio_write(LBControl,0x4000a008,1)) return(rc);

  /* Setup Video Port 1*/
  if (rc = ioRDTWriteDVP(i_VIU_MCR1,VIU_oss_vp /*_op*/ + VIU_ohsp + VIU_ovsp + VIU_oblt + VIU_OutOnly)) return (rc);

  /*  setup datapath controls */
  if (rc = ioRDTWriteDVP(i_VIU_DPC1, OPU_to_Vp1)) return (rc);
  if (rc = ioRDTWriteDVP(i_VIU_DPC2, OPU_to_Vp1)) return (rc);

  /*  Setup Object  ??? OB_SSM*/
  if (rc = Setup_OB (OBU0_MCR, (OB_Normal + OB_XBLT + OB_YBLT +OB_No_Copy),
                     iXRef, fb_startaddr_high, fb_startaddr_low, iXSize, iYSize, 0))
     return(rc);

  /* Setup the OPU */
  if (rc = ioRDTWriteDVP(OPU_MCR1, FieldPol + Interlaced + In422NoTag/*+ OP_BLANK_OUT*/)) return (rc);
  if (rc = ioRDTWriteDVP(OPU_XBI1,1+1)) return (rc);
  if (rc = ioRDTWriteDVP(OPU_XEI1,iXSize+1)) return (rc);
  if (rc = ioRDTWriteDVP(OPU_YBI1,1)) return (rc);
  if (rc = ioRDTWriteDVP(OPU_YEI1,iYSize)) return (rc);

  if (rc = ioRDTWriteDVP(OPU_MCR2,Interlaced + In422NoTag/* + OP_BLANK_OUT*/)) return (rc);
  if (rc = ioRDTWriteDVP(OPU_XBI2,1+1)) return (rc);
  if (rc = ioRDTWriteDVP(OPU_XEI2,iXSize+1)) return (rc);
  if (rc = ioRDTWriteDVP(OPU_YBI2,1)) return (rc);
  if (rc = ioRDTWriteDVP(OPU_YEI2,iYSize)) return (rc);

  /*  Setup sequencer */
  /*  Seq Inst 0  */
  if (rc = ioRDTWriteDVP(SIU_SIM0,(OB0_to_OP + offset_0 + SIU_EXIT))) return (rc);

  /*  Enable sequencer start */
  if (rc = ioRDTWriteDVP(SIU_MCR,(SIU_Start1 + SIU_No_Toggle + (SIU_SI1 * 0)))) return (rc);

  /*  Generate Newfield to start sequencer  */
  if (rc = ioRDTWriteDVP(i_VIU_WDT,VIU_MField_V1o)) return (rc);
  if (rc = UpdateGen()) return(rc);
  if (rc = ioRDTWriteDVP(i_VPU_MCR, OP_Start_Next /*IP2_F1_Only*/)) return (rc);
  if (rc = UpdateGen()) return(rc);

  return(OK);
} /* End of V1Out */

