static char sccsid[] = "@(#)41  1.2  src/bos/diag/tu/sun/tframebuf.c, tu_sunrise, bos411, 9437A411a 4/5/94 10:49:44";
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: FBTest1
 *              FBTest2
 *              FBTest3
 *              Test2070MemIO
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

Function(s) - Frame Buffer Test

Module Name :  tframebuf.c

Test writes/reads/compares byte values to check all bit positions
in the entire frame buffer.

*****************************************************************************/

#include <stdio.h>
#include "px2070.h"
#include "iodvp.h"
#include "suntu.h"
/*#include "px2070util.h"*/
#include "error.h"
#include "sun_tu_type.h"

#define MAXSHORTPATS  37

unsigned int G_wTestShorts[MAXSHORTPATS] = {0x0000,0xffff,0xaaaa,0x5555,
                                0x0001,0x0002,0x0004,0x0008,
                                0x0010,0x0020,0x0040,0x0080,
                                0x0100,0x0200,0x0400,0x0800,
                                0x1000,0x2000,0x4000,0x8000,
                                0xfffe,0xfffd,0xfffb,0xfff7,
                                0xffef,0xffdf,0xffbf,0xff7f,
                                0xfeff,0xfdff,0xfbff,0xf7ff,
                                0xefff,0xdfff,0xbfff,0x7fff,
                                0};

int Test2070MemIO()
{
  int rc;
  /* Clear the error code */
  error.fb_test.sub_error_code = 0;
  error.fb_test.expected_data = 0;
  error.fb_test.actual_data = 0;
  error.fb_test.row = 0;
  error.fb_test.column = 0;

  if (rc = pxinitialize())       /* Initialize the 2070 */
  {
       error.fb_test.error_code = ERRORFB_2070INIT;
       error.fb_test.sub_error_code = rc;
       log_error(error);
       return(ERRORFB_2070INIT);
  }

  if (rc=FBTest1())
  {
       error.fb_test.error_code = ERRORFB_TEST1;
       error.fb_test.sub_error_code = rc;
       log_error(error);
       return(ERRORFB_TEST1);
  }

  if (rc=FBTest2())
  {
       error.fb_test.error_code = ERRORFB_TEST2;
       error.fb_test.sub_error_code = rc;
       log_error(error);
       return(ERRORFB_TEST2);
  }

  if (rc=FBTest3())
  {
       error.fb_test.error_code = ERRORFB_TEST3;
       error.fb_test.sub_error_code = rc;
       log_error(error);
       return(ERRORFB_TEST3);
  }

  return(OK);
}  /*  End T2070MemIo */


int FBTest1()
/**************************************************************
   Test the a small portion of  memory:
   Write data to fifo F  then OB0 and read it back from fifo D
***************************************************************/
{
 int iTestCnt,iXSize, iYSize,iLoopCount, FifoFull, FifoEmpty, rc;
 unsigned int bExpected;
 unsigned int wReadVal,wWriteVal,wRowCount,wColCount,wExpected;
 unsigned int dwReadVal,dwExpected;
 long lTimeCount;

 iXSize = 64;
 iYSize = 2;

 /*  halt sequencer */
 if (rc=SeqHalt()) return(rc);

 /*  setup datapath controls */
 if (rc=GetHostPorts()) return(rc);

 /*  Clear out the fifos */
 if (rc=ResetFifo(ALL_FIFOS)) return(rc);

 /*  Enable the fifos */
 if (rc=EnableFifo(ALL_FIFOS)) return(rc);

 /*  Set up the MMU */
 if (rc=SetUpMMU()) return(rc);

 /*  Disable all objects */
 if (rc=OBReset(ALL_OBJECTS)) return(rc);

 /*  Setup sequencer */
  /*  Seq Inst 0  */
 if (rc=ioRDTWriteDVP(SIU_SIM0,(IPU2_to_OB0 + offset_0))) return(rc);

 /*  Setup Object (object */
 /*     normal addressing */

 if (rc=Setup_OB (OBU0_MCR, (0x00 + OB_Normal +OB_SSM +  OB_XBLT + OB_YBLT + OB_No_Copy),
          iXSize, 0, 0,  iXSize, iYSize, 0)) return(rc);

 /*  Enable sequencer start */
 if (rc=ioRDTWriteDVP(SIU_MCR,(SIU_Start1 + SIU_No_Toggle + (SIU_SI1 * 0)))) return(rc);

 /*  Generate Newfield to start sequencer  */
 if (rc=SeqManStart()) return(rc);

 /*  write */

 /*  Make sure the fifo is ready */
lTimeCount = 0;
if (rc=IsFifoFull(FIFO_F, &FifoFull)) return(rc);
while (FifoFull)
  {
  lTimeCount++;
  if (lTimeCount == TIMEOUT)
    {
    DEBUG_0("TIMEOUT ERROR Fifos Not Responding\n");
    return (ERRORFB_TEST1+1);
    }
  if (rc=IsFifoFull(FIFO_F, &FifoFull)) return(rc);
  }
for (iTestCnt=0;iTestCnt<(iXSize*iYSize);iTestCnt++)
  {
  if (rc=ioMDTShortWriteDVP(iTestCnt |0x5540)) return(rc);
  iTestCnt++;
  if (rc=ioMDTShortWriteDVP(iTestCnt |0x5540)) return(rc);
  iTestCnt++;
  if (rc=ioMDTShortWriteDVP(iTestCnt |0xaa80)) return(rc);
  iTestCnt++;
  if (rc=ioMDTShortWriteDVP(iTestCnt |0xaa80)) return(rc);
  }

 /*  Might need a delay here before halting */
 /*  Read the fifo flags */
 if (rc=ioRDTReadDVP(SIU_FCS, &wReadVal)) return(rc);

 /*  halt sequencer */
 if (rc=SeqHalt()) return(rc);

 /*  set ob0 to read from memory */
 if (rc=Setup_OB (OBU0_MCR, (0x00 + OB_Normal + OB_SSM + OB_XBLT + OB_YBLT + OB_No_Copy),
                  iXSize, 0, 0,  iXSize, iYSize, 0)) return(rc);

 /*  reprogram seqencer */
  /*  Seq Inst 0  */
 if (rc=ioRDTWriteDVP(SIU_SIM0,(OB0_to_OP + offset_0))) return(rc);

 /*  Enable sequencer start */
 if (rc=ioRDTWriteDVP(SIU_MCR,(SIU_Start1 + SIU_No_Toggle + (SIU_SI1 * 0)))) return(rc);

 /*  Generate Newfield to start sequencer  */
 if (rc=SeqManStart()) return(rc);

iTestCnt = lTimeCount =0;
while (iTestCnt < iXSize*iYSize)
  {
  lTimeCount++;
  if (lTimeCount == TIMEOUT)
    {
    DEBUG_0("TIMEOUT ERROR Fifos Not Responding\n");
    return (ERRORFB_TEST1+1);
    }
  if (rc=IsFifoEmpty(FIFO_D, &FifoEmpty)) return(rc);
  if (!FifoEmpty)
  {
    lTimeCount = 0;
    if (rc=ioMDTShortReadDVP(&wReadVal)) return(rc);
    if (wReadVal != (iTestCnt|0x5540))
      {
      DEBUG_2("ERROR memory read test Expected %x Actual %x \n",
              iTestCnt|0x5540,wReadVal);
      error.fb_test.expected_data = iTestCnt|0x5540;
      error.fb_test.actual_data = wReadVal;
      return (ERRORFB_TEST1+2);
      }
    iTestCnt++;
    if (rc=ioMDTShortReadDVP(&wReadVal)) return(rc);
    if (wReadVal != (iTestCnt|0x5540))
      {
      DEBUG_2("ERROR memory read test Expected %x Actual %x \n",
              iTestCnt|0x5540,wReadVal);
      error.fb_test.expected_data = iTestCnt|0x5540;
      error.fb_test.actual_data = wReadVal;
      return (ERRORFB_TEST1+2);
      }
    iTestCnt++;
    if (rc=ioMDTShortReadDVP(&wReadVal)) return(rc);
    if (wReadVal != (iTestCnt|0xaa80))
      {
      DEBUG_2("ERROR memory read test Expected %x Actual %x \n",
              iTestCnt|0xaa80,wReadVal);
      error.fb_test.expected_data = iTestCnt|0xaa80;
      error.fb_test.actual_data = wReadVal;
      return (ERRORFB_TEST1+2);
      }
    iTestCnt++;
    if (rc=ioMDTShortReadDVP(&wReadVal)) return(rc);
    if (wReadVal != (iTestCnt|0xaa80))
      {
      DEBUG_2("ERROR memory read test Expected %04x Actual %04x \n",
              iTestCnt|0xaa80,wReadVal);
      error.fb_test.expected_data = iTestCnt|0xaa80;
      error.fb_test.actual_data = wReadVal;
      return (ERRORFB_TEST1+2);
      }
    iTestCnt++;
    }  /*  If fifo not empty */
  }  /*  While */
  return(OK);

}

int FBTest2()
/**************************************************************
 Now test the entire memory array with address dependent data
***************************************************************/
{
 int iTestCnt,iXSize, iYSize,iLoopCount, FifoFull, FifoEmpty, rc;
 unsigned int bExpected;
 unsigned int wReadVal,wWriteVal,wRowCount,wColCount,wExpected;
 unsigned int dwReadVal,dwExpected;
 long lTimeCount;
 unsigned int fb_startaddr_high, fb_startaddr_low;

#ifdef WHOLE_FRAME
 iXSize = 512;           /* For testing entire frame buffer */
 iYSize = 1024;          /* too long of a test for HTX */
 fb_startaddr_high = 0;
 fb_startaddr_low = 0;
#else
 iXSize = 128;           /* To shorten the HTX test time */
 iYSize = 128;
 fb_startaddr_high = 0x4;  /* Start somewhere middle of frame buffer */
 fb_startaddr_low = 0xb000;
#endif

 /*  halt sequencer */
 if (rc=SeqHalt()) return(rc);

 /*  setup datapath controls */
 if (rc=GetHostPorts()) return(rc);

 /*  Clear out the fifos */
 if (rc=ResetFifo(ALL_FIFOS)) return(rc);

 /*  Enable the fifos */
 if (rc=EnableFifo(ALL_FIFOS)) return(rc);

 /*  Set up the MMU */
 if (rc=SetUpMMU()) return(rc);

 /*  Disable all objects */
 if (rc=OBReset(ALL_OBJECTS)) return(rc);

 /*  Setup sequencer */
  /*  Seq Inst 0  */
 if (rc=ioRDTWriteDVP(SIU_SIM0,(IPU2_to_OB0 + offset_0))) return(rc);

 /*  Setup Object (object */
 /*     normal addressing */

 if (rc=Setup_OB (OBU0_MCR, (OB_Normal +OB_SSM +  OB_XBLT + OB_YBLT + OB_No_Copy),
          iXSize, fb_startaddr_high, fb_startaddr_low,  iXSize, iYSize, 0)) return(rc);

 /*  Enable sequencer start */
 if (rc=ioRDTWriteDVP(SIU_MCR,(SIU_Start1 + SIU_No_Toggle + (SIU_SI1 * 0)))) return(rc);

 /*  Generate Newfield to start sequencer  */
 if (rc=SeqManStart()) return(rc);

 /*  write to memory port  */

DEBUG_0("   Memory Address Test Writing Data\n\r");

lTimeCount = 0;
for (wRowCount = 0;wRowCount < iYSize;wRowCount++)
  {
  for (wColCount = 0;wColCount < iXSize;wColCount+=2)
    {
     /*  While fifo F is full spin */
    if (rc=IsFifoFull(FIFO_F, &FifoFull)) return(rc);
    while (FifoFull)
      {
       /*                 wRowCount */
      lTimeCount++;
      if (lTimeCount == TIMEOUT)
        {
        DEBUG_0("TIMEOUT ERROR Fifos Not Responding\n");
        return (ERRORFB_TEST2+1);
        }
      if (rc=IsFifoFull(FIFO_F, &FifoFull)) return(rc);
      }
    if (rc=ioMDTWriteDVP(((unsigned int)wColCount/2) | (((unsigned int)wRowCount)<<16))) return(rc);
    lTimeCount = 0;
    }   /*  for ColCount */
  }   /*  for RowCount */

 /*  Might need a delay here before halting */
 /*  Read the fifo flags */
 if (rc=ioRDTReadDVP(SIU_FCS,&wReadVal)) return(rc);

 /*  halt sequecer */
 if (rc=SeqHalt()) return(rc);

 /*  set ob0 to read from memory */
 if (rc=Setup_OB (OBU0_MCR, (OB_Normal + OB_SSM + OB_XBLT + OB_YBLT + OB_No_Copy),
                  iXSize, fb_startaddr_high, fb_startaddr_low,  iXSize, iYSize, 0)) return(rc);

 /*  reprogram seqencer */
  /*  Seq Inst 0  */
 if (rc=ioRDTWriteDVP(SIU_SIM0,(OB0_to_OP + offset_0))) return(rc);

 /*  Enable sequencer start */
 if (rc=ioRDTWriteDVP(SIU_MCR,(SIU_Start1 + SIU_No_Toggle + (SIU_SI1 * 0)))) return(rc);

 /*  Generate Newfield to start sequencer  */
 if (rc=SeqManStart()) return(rc);

DEBUG_0("   Memory Address Test Reading Data\n\r");

lTimeCount = 0;
 /*  Read back the data */
for (wRowCount = 0;wRowCount < iYSize;wRowCount++)
  {
  for (wColCount = 0;wColCount < iXSize;wColCount+=2)
    {
     /*  If fifo D is empty spin */
    if (rc=IsFifoEmpty(FIFO_D, &FifoEmpty)) return(rc);
    while (FifoEmpty)
      {
       /*                 wRowCount */
      lTimeCount++;
      if (lTimeCount == TIMEOUT)
        {
        DEBUG_0("TIMEOUT ERROR Fifos Not Responding\n");
        return (ERRORFB_TEST2+1);
        }
      if (rc=IsFifoEmpty(FIFO_D, &FifoEmpty)) return(rc);
      }
    lTimeCount = 0;
    if (rc=ioMDTReadDVP(&dwReadVal)) return(rc);
    if (dwReadVal != (((unsigned int)wColCount/2) | (((unsigned int)wRowCount)<<16)))
      {
        DEBUG_2("ERROR Memory Address Test Expected %08lx Actual %08lx \n",
             (((unsigned int)wColCount/2) | (((unsigned int)wRowCount)<<16)) ,dwReadVal);
        error.fb_test.expected_data = ((unsigned int)wColCount/2) | (((unsigned int)wRowCount)<<16);
        error.fb_test.actual_data = dwReadVal;
        error.fb_test.row = wRowCount;
        error.fb_test.column = wColCount;
        return (ERRORFB_TEST2+2);
      }
    }  /*  For ColCount */
  }  /*  For RowCount */
  return(OK);
}

int FBTest3()
/**************************************************************
 Now test the entire memory array with test array data
***************************************************************/
{
 int iTestCnt,iXSize, iYSize,iLoopCount, FifoEmpty, rc;
 unsigned int bExpected;
 unsigned int wReadVal,wWriteVal,wRowCount,wColCount,wExpected;
 unsigned int dwReadVal,dwExpected;
 long lTimeCount;
 unsigned int fb_startaddr_high, fb_startaddr_low;

#ifdef WHOLE_FRAME
 iXSize = 512;           /* For testing entire frame buffer */
 iYSize = 1024;          /* too long of a test for HTX */
 fb_startaddr_high = 0;
 fb_startaddr_low = 0;
#else
 iXSize = 128;           /* To shorten the HTX test time */
 iYSize = 128;
 fb_startaddr_high = 0x4;        /* Start somewhere in the middle of fr. buffer*/
 fb_startaddr_low = 0xc000;
#endif

iTestCnt = 1;

for (iLoopCount = 0;iLoopCount < iTestCnt;iLoopCount++)
  {
   /*  Disable the ALU */
  if (rc = ioRDTWriteDVP(VPU_MCR,ALU_Disable)) return (rc);
  if (rc=UpdateGen()) return(rc);

   /*  halt sequencer */
  if (rc=SeqHalt()) return(rc);

   /*  setup datapath controls */
  if (rc=GetHostPorts()) return(rc);

   /*  Clear out the fifos */
  if (rc=ResetFifo(ALL_FIFOS)) return(rc);

   /*  Enable the fifos */
  if (rc=EnableFifo(ALL_FIFOS)) return(rc);

   /*  Set up the MMU */
  if (rc=SetUpMMU()) return(rc);

   /*  Disable all objects */
  if (rc=OBReset(ALL_OBJECTS)) return(rc);

   /*  Setup sequencer */
  /*  Seq Inst 0  */
  if (rc = ioRDTWriteDVP(SIU_SIM0,(ALUE_to_OB0 + offset_0))) return (rc);

   /*  Setup Object (object */
   /*     normal addressing */

  if (rc=Setup_OB (OBU0_MCR, (OB_Normal +OB_SSM +  OB_XBLT + OB_YBLT + OB_No_Copy),
            iXSize, fb_startaddr_high, fb_startaddr_low,  iXSize, iYSize, 0)) return(rc);

   /*  Setup the ALU A constant registers with the test array data */
  if (rc = ioRDTWriteDVP(ALU_CAY,G_wTestShorts[iLoopCount])) return (rc);
  if (rc = ioRDTWriteDVP(ALU_CAU,G_wTestShorts[iLoopCount])) return (rc);
  if (rc = ioRDTWriteDVP(ALU_CAV,G_wTestShorts[iLoopCount])) return (rc);

   /*  Set up the ALU to pass constant reg A to fifo E */
   /*  ALU MCR regs */
  if (rc = ioRDTWriteDVP(ALU_MCR1,ALU_Asrc_Reg + ALU_Vout_Logic + ALU_Uout_Logic +
                ALU_Yout_Logic)) return (rc) ;
  if (rc = ioRDTWriteDVP(ALU_MCR2,ALU_Asrc_Reg + ALU_Vout_Logic + ALU_Uout_Logic +
                ALU_Yout_Logic)) return (rc) ;

   /*  ALU logic operation select */
  if (rc = ioRDTWriteDVP(ALU_LOPY,ALU_PassA)) return (rc);
  if (rc = ioRDTWriteDVP(ALU_LOPU,ALU_PassA)) return (rc);
  if (rc = ioRDTWriteDVP(ALU_LOPV,ALU_PassA)) return (rc);
  if (rc = UpdateGen()) return(rc);

   /*  Enable the ALU */
  if (rc = ioRDTWriteDVP(VPU_MCR,ALU_Enable)) return (rc);
  if (rc = UpdateGen()) return(rc);


   /*  Enable sequencer start */
  if (rc = ioRDTWriteDVP(SIU_MCR,(SIU_Start1 + SIU_No_Toggle + (SIU_SI1 * 0)))) return (rc);

   /*  Generate Newfield to start sequencer  */
  if (rc=SeqManStart()) return(rc);


  DEBUG_1("   Memory Data Test %d Writing Data\n\r",iLoopCount);

   /*  Spin here until write done */
   do    /* spin */
   {
      if (rc = ioRDTReadDVP(OBU0_MCR,&wReadVal)) return (rc);
   }  while (wReadVal & OPM_MASK);

   /*  halt sequencer */
  if (rc=SeqHalt()) return(rc);

   /*  set ob0 to read from memory */
  if (rc=Setup_OB (OBU0_MCR, (OB_Normal + OB_SSM + OB_XBLT + OB_YBLT + OB_No_Copy),
                  iXSize, fb_startaddr_high, fb_startaddr_low,  iXSize, iYSize, 0)) return(rc);

   /*  reprogram seqencer */
  /*  Seq Inst 0  */
  if (rc = ioRDTWriteDVP(SIU_SIM0,(OB0_to_OP + offset_0))) return (rc);

   /*  Enable sequencer start */
  if (rc = ioRDTWriteDVP(SIU_MCR,(SIU_Start1 + SIU_No_Toggle + (SIU_SI1 * 0)))) return (rc);

   /*  Generate Newfield to start sequencer  */
  if (rc=SeqManStart()) return(rc);

  DEBUG_1("   Memory Data Test %d Reading Data\n\r",iLoopCount);

  lTimeCount = 0;
   /*  Read back the data */
  for (wRowCount = 0;wRowCount < iYSize;wRowCount++)
    {
    for (wColCount = 0;wColCount < iXSize;wColCount+=2)
      {
       /*  If fifo D is empty spin */
      if (rc=IsFifoEmpty(FIFO_D, &FifoEmpty)) return(rc);
      while (FifoEmpty)
        {
        lTimeCount++;
        if (lTimeCount == TIMEOUT)
          {
          DEBUG_0("TIMEOUT ERROR Fifos Not Responding\n");
          return (ERRORFB_TEST3+1);
          }
        if (rc=IsFifoEmpty(FIFO_D, &FifoEmpty)) return(rc);
        }
      lTimeCount = 0;
      if (rc = ioMDTReadDVP(&dwReadVal)) return (rc);
      bExpected = (unsigned int) (G_wTestShorts[iLoopCount] &0xff);
      wExpected = (((unsigned int) bExpected) &0xff) | (((unsigned int) bExpected)<<8);
      dwExpected = (((unsigned int) wExpected) &0xffff) | (((unsigned int) wExpected)<<16);
      if (dwReadVal != dwExpected)
        {
        DEBUG_4("ERROR Memory Data Test Expected %08lx Actual %08lx Row %d Col %d\n",
                dwExpected,dwReadVal,wRowCount,wColCount);
        error.fb_test.expected_data = dwExpected;
        error.fb_test.actual_data = dwReadVal;
        error.fb_test.row = wRowCount;
        error.fb_test.column = wColCount;
        return (ERRORFB_TEST3+2);
        }
      }  /*  For ColCount */
    }  /*  For RowCount */
  }  /*  For iLoopCount */
  return(OK);
}
