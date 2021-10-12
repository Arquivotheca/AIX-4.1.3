static char sccsid[] = "@(#)38  1.3  src/bos/diag/tu/sun/t560reg.c, tu_sunrise, bos411, 9437A411a 6/1/94 09:29:46";
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: Test560Regs
 *              TestFifo
 *              TestHuffman
 *              TestQuantizer
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
*       File Name:  t560reg.c
*
*       Module Abstract: This module is the high level test routine
*               that enables the testing of the registers of the 560.
*
*       Calling Sequence:
*       Test560Regs()
*
*       Input Parameters:
*       none
*       Output Parameters:
*       none
*
*       Global Variables:
*       TestPattern[MAXSHORTPATS]
***********************************************************
*       Author: Dzung Vu
*       Date:   11/10/93
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/
/* #includes --------------------------------------------*/
#include <stdio.h>

#include "error.h"
#include "sun_tu_type.h"
#include "suntu.h"
#include "pc5xx.h"

#define MAXSHORTPATS       20
#define MAXLONGPATS        36
#define HuffMask           0x1ff
#define QuantMask          0xffff
#define FIFOTESTSIZE       30

 /*  Global test pattern array */
unsigned int TestPattern[MAXLONGPATS] = {
                                         0x0000,0x0001,0x0002,0x0004,0x0008,
                                         0x0010,0x0020,0x0040,0x0080,0x0100,
                                         0x0200,0x0400,0x0800,0x1000,0x2000,
                                         0x4000,0x8000,0x1111,0x2222,0x3333,
                                         0x4444,0x5555,0x6666,0x7777,0x8888,
                                         0x9999,0xaaaa,0xbbbb,0xcccc,0xdddd,
                                         0xeeee,0xffff,0xefff,0xdfff,0xbfff,
                                         0x7fff };

int Test560Regs()
{
  int rc;

  /* Test the HUFFMAN table */
  /* Enable Huffman table loading */
  WriteCL5xxRegister(_Huff_Enable, 0x1, 0);

  if (rc=TestHuffman(CODECHUFFYACMIN, CODECHUFFYACMAX))
  {
       error.register_test.error_code = ERROR560_HUFFYACREGS;
       error.register_test.sub_error_code = rc;
       log_error(error);
       return(ERROR560_HUFFYACREGS);
  }
  if (rc=TestHuffman(CODECHUFFYDCMIN, CODECHUFFYDCMAX))
  {
       error.register_test.error_code = ERROR560_HUFFYDCREGS;
       error.register_test.sub_error_code = rc;
       log_error(error);
       return(ERROR560_HUFFYDCREGS);
  }
  if (rc=TestHuffman(CODECHUFFCACMIN, CODECHUFFCACMAX))
  {
       error.register_test.error_code = ERROR560_HUFFCACREGS;
       error.register_test.sub_error_code = rc;
       log_error(error);
       return(ERROR560_HUFFCACREGS);
  }
  if (rc=TestHuffman(CODECHUFFCDCMIN, CODECHUFFCDCMAX))
  {
       error.register_test.error_code = ERROR560_HUFFCDCREGS;
       error.register_test.sub_error_code = rc;
       log_error(error);
       return(ERROR560_HUFFCDCREGS);
  }
  /* Disable Huffman table loading */
  WriteCL5xxRegister(_Huff_Enable, 0x0, 0);

  /* Test the QUANTIZER table */
  /* Enable Quantizer table loading */
  WriteCL5xxRegister(_QuantSync, 0x400, 0);
  WriteCL5xxRegister(_QuantABSelect, 00, 0);
  if (rc=TestQuantizer())
  {
       error.register_test.error_code = ERROR560_QUANTREGS;
       error.register_test.sub_error_code = rc;
       log_error(error);
       return(ERROR560_QUANTREGS);
  }

#ifdef CL560B
  /* Test the FIFO registers */
  if (rc=TestFifo())
  {
       error.register_test.error_code = ERROR560_FIFOREGS;
       error.register_test.sub_error_code = rc;
       log_error(error);
       return(ERROR560_FIFOREGS);
  }
#endif

  return (OK);
}  /*  End Test560Regs */

/**********************************************************
*
*       File Name:  Test560.c
*
*       Module Abstract: This module performs the diagnostic
*               testing of the registers in the Huffman table.
*
*       Calling Sequence:
*       TestHuffman()
*
*       Input Parameters:
*       none
*       Output Parameters:
*       none
*
*       Global Variables:
*       G_iTestPats[MAXPATTERNS]
***********************************************************
*       Author: Dzung Vu
*       Date:   11/10/93
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/
int TestHuffman(unsigned int startaddr, unsigned int endaddr)
{
int rc;
int iTestArrayIndex;
unsigned int wWriteVal,wReadVal,wIndex;


  /*  Do an address dependent read/write/compare test  */

  /*  Write the data */
  for (wWriteVal = startaddr; wWriteVal <= endaddr;wWriteVal+=4)
    { if (rc = WriteCL5xxRegister(wWriteVal, wWriteVal & HuffMask, 0)) return (rc);
    }

  /*  Read the data back and test it */
  for (wWriteVal = startaddr; wWriteVal <= endaddr;wWriteVal+=4)
  {
    if (rc = ReadCL5xxRegister(wWriteVal, &wReadVal, 0)) return (rc);
    if ((wReadVal & HuffMask) != (wWriteVal & HuffMask))
    {
      error.register_test.expected_data = wWriteVal & HuffMask;
      error.register_test.actual_data = wReadVal & HuffMask;
      error.register_test.address = wWriteVal;
      printf("HUFFMAN ADDRESS PATTERN REG TEST COMPARE ERROR Expected %04x Actual %04x \n",wWriteVal & HuffMask,wReadVal & HuffMask);
      return (ERROR560_ADDRTEST);
    }
  }  /*  for wWriteVal */

  /*  Test each register in the HUFFMAN with the test array */
  /*  For each register */
  for (wIndex = startaddr; wIndex <= endaddr;wIndex+=4)
    {
     /*  Write the contents of the test array and test it */
    for (iTestArrayIndex = 0;iTestArrayIndex < MAXSHORTPATS;iTestArrayIndex++)
    {
      wWriteVal = TestPattern[iTestArrayIndex];
      if (rc = WriteCL5xxRegister(wIndex, wWriteVal & HuffMask, 0)) return (rc);
      if (rc = ReadCL5xxRegister(wIndex, &wReadVal, 0)) return (rc);
      if ((wReadVal & HuffMask) != (wWriteVal & HuffMask))
        {
        error.register_test.expected_data = wWriteVal & HuffMask;
        error.register_test.actual_data = wReadVal & HuffMask;
        error.register_test.address = wIndex;
        printf("HUFFMAN ARRAY PATTERN REG TEST COMPARE ERROR Index %04x Expected %04x Actual %04x \n",
           wIndex,wWriteVal & HuffMask, wReadVal & HuffMask);
        return (ERROR560_PATTEST);
        }
      }  /*  for iTestIndex */
    }  /*  for wIndex */

  return (OK);
}  /*  End TestHuffman */

/**********************************************************
*
*       File Name:  Test560.c
*
*       Module Abstract: This module performs the diagnostic
*               testing of the registers in the Quantizer table.
*
*       Calling Sequence:
*       TestQuantizer()
*
*       Input Parameters:
*       none
*       Output Parameters:
*       none
*
*       Global Variables:
*       G_iTestPats[MAXPATTERNS]
***********************************************************
*       Author: Dzung Vu
*       Date:   11/10/93
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/
int TestQuantizer()
{
int rc;
int iTestArrayIndex;
unsigned int wWriteVal,wReadVal,wIndex;


  /*  Do an address dependent read/write/compare test  */

  /*  Write the data */
  for (wWriteVal = CODECQUANTMIN; wWriteVal <= CODECQUANTMAX;wWriteVal+=4)
    { if (rc = WriteCL5xxRegister(wWriteVal, wWriteVal & QuantMask, 0)) return (rc);
    }

  /*  Read the data back and test it */
  for (wWriteVal = CODECQUANTMIN; wWriteVal <= CODECQUANTMAX;wWriteVal+=4)
  {
    /* For Quantizer, must do 2 consecutive reads to get valid data */
    if (rc = ReadCL5xxRegister(wWriteVal, &wReadVal, 0)) return (rc);
    if (rc = ReadCL5xxRegister(wWriteVal, &wReadVal, 0)) return (rc);
    if ((wReadVal & QuantMask) != (wWriteVal & QuantMask))
    {
      error.register_test.expected_data = wWriteVal & QuantMask;
      error.register_test.actual_data = wReadVal & QuantMask;
      error.register_test.address = wWriteVal;
      printf("QUANTIZER ADDRESS PATTERN REG TEST COMPARE ERROR Expected %04x Actual %04x \n",wWriteVal & QuantMask,wReadVal & QuantMask);
      return (ERROR560_ADDRTEST);
    }
  }  /*  for wWriteVal */

  /*  Test each register in the HUFFMAN with the test array */
  /*  For each register */
  for (wIndex = CODECQUANTMIN; wIndex <= CODECQUANTMAX;wIndex+=4)
    {
     /*  Write the contents of the test array and test it */
    for (iTestArrayIndex = 0;iTestArrayIndex < MAXLONGPATS;iTestArrayIndex++)
    {
      wWriteVal = TestPattern[iTestArrayIndex];
      if (rc = WriteCL5xxRegister(wIndex, wWriteVal & QuantMask, 0)) return (rc);
    /* For Quantizer, must do 2 consecutive reads to get valid data */
      if (rc = ReadCL5xxRegister(wIndex, &wReadVal, 0)) return (rc);
      if (rc = ReadCL5xxRegister(wIndex, &wReadVal, 0)) return (rc);
      if ((wReadVal & QuantMask) != (wWriteVal & QuantMask))
        {
        error.register_test.expected_data = wWriteVal & QuantMask;
        error.register_test.actual_data = wReadVal & QuantMask;
        error.register_test.address = wIndex;
        printf("QUANTIZER ARRAY PATTERN REG TEST COMPARE ERROR Index %04x Expected %04x Actual %04x \n",
           wIndex,wWriteVal & QuantMask, wReadVal & QuantMask);
        return (ERROR560_PATTEST);
        }
      }  /*  for iTestIndex */
    }  /*  for wIndex */

  return (OK);
}  /*  End TestQuantizer */

/*****************************************************************************/
/*  TestFifo()                                                               */
/*                                                                           */
/*  FUNCTION:                                                                */
/*    Test the first 30 words (about 1/4) of the 560 FIFO (128 words) by     */
/*    writing, reading back, and verifying the data                          */
/*    The 560 needs to be set in DECOMPRESS mode to write the FIFO, and      */
/*    COMPRESS mode to read the FIFO.                                        */
/*                                                                           */
/*  INPUT:    NONE                                                           */
/*                                                                           */
/*  OUTPUT:   return value     - OK if successful, ERROR if NOT.             */
/*****************************************************************************/
int TestFifo ()
{
int rc;
int iTestArrayIndex;
unsigned int wWriteVal,wReadVal,wIndex;

  /*  Do an address dependent read/write/compare test  */
  /* Reset the 560 chip */
  WriteCL5xxRegister(_S_Reset, 0xff, 0);
  /* Setup 560 in decompress mode in order to write the FIFO */
  if (rc = WriteCL5xxRegister(_Config, 0x118, 0)) return (rc);
/*  DecompSetup(640,240,20);*/

  /*  Write the data */
  for (wWriteVal = 0; wWriteVal < FIFOTESTSIZE; wWriteVal++)
    { if (rc = WriteCL5xxRegister(_Codec, wWriteVal, 0)) return (rc);
    }
  usleep(10);

/* Test to make sure have the correct number of 32-bit words (FIFOTESTSIZE/2) */
/* written into the FIFO                                                      */
  if (rc = ReadCL5xxRegister(_FIFO_LevelL, &wReadVal, 0)) return (rc);
  if (wReadVal != (FIFOTESTSIZE/2)) {
      if (rc = ReadCL5xxRegister(_FIFO_LevelL, &wReadVal, 0)) return (rc);
      if (wReadVal != (FIFOTESTSIZE/2)) {
        error.register_test.expected_data = FIFOTESTSIZE/2;
        error.register_test.actual_data = wReadVal;
        error.register_test.address = _FIFO_LevelL;
        printf("FIFO COUNT ERROR Expected %04x Actual %04x \n",(FIFOTESTSIZE/2),wReadVal);
        return (ERROR560_FIFOCOUNT);
      } /* endif */
  } /* endif */

  /* Setup 560 in compress mode in order to read the FIFO */
  if (rc = WriteCL5xxRegister(_Config, 0x18, 0)) return (rc);
/*  CompSetup(640,240,20,0); */

  /*  Read the data back and test it */
  for (wWriteVal = 0; wWriteVal < FIFOTESTSIZE; wWriteVal++)
  {
    if (rc = ReadCL5xxRegister(_Codec, &wReadVal, 0)) return (rc);
    wReadVal = wReadVal & 0xffff;
    if (wReadVal  != wWriteVal)
    {
      error.register_test.expected_data = wWriteVal;
      error.register_test.actual_data = wReadVal;
      error.register_test.address = wWriteVal;
      printf("FIFO ADDRESS PATTERN REG TEST COMPARE ERROR Expected %04x Actual %04x \n",wWriteVal,wReadVal);
      return (ERROR560_ADDRTEST);
    }
  }  /*  for wWriteVal */

  /*  Do data patterns write/read/compare test  */
  /* Reset the 560 chip */
  WriteCL5xxRegister(_S_Reset, 0xff, 0);
  /* Setup 560 in decompress mode in order to write the FIFO */
  if (rc = WriteCL5xxRegister(_Config, 0x118, 0)) return (rc);
/*  DecompSetup(640,240,20);*/
  /*  Write the data */
  for (wIndex = 0; wIndex < FIFOTESTSIZE; wIndex++)
    { if (rc = WriteCL5xxRegister(_Codec, TestPattern[wIndex], 0)) return (rc);
    }

  usleep(10);

/* Test to make sure have the correct number of 32-bit words (FIFOTESTSIZE/2) */
/* written into the FIFO                                                      */
  if (rc = ReadCL5xxRegister(_FIFO_LevelL, &wReadVal, 0)) return (rc);
  if (wReadVal != (FIFOTESTSIZE/2)) {
        error.register_test.expected_data = FIFOTESTSIZE/2;
        error.register_test.actual_data = wReadVal;
        error.register_test.address = _FIFO_LevelL;
        printf("FIFO COUNT ERROR Expected %04x Actual %04x \n",(FIFOTESTSIZE/2),wReadVal);
        return (ERROR560_FIFOCOUNT);
  } /* endif */

  /* Setup 560 in compress mode in order to read the FIFO */
    if (rc = WriteCL5xxRegister(_Config, 0x18, 0)) return (rc);
/*    CompSetup(640,240,20,0); */
      /*  Read the data back and test it */
    for (wIndex = 0; wIndex < FIFOTESTSIZE; wIndex++)
    {
      if (rc = ReadCL5xxRegister(_Codec, &wReadVal, 0)) return (rc);
      if (wReadVal  != TestPattern[wIndex])
      {
        error.register_test.expected_data = TestPattern[wIndex];
        error.register_test.actual_data = wReadVal;
        error.register_test.address = wIndex;
        printf("FIFO ARRAY PATTERN REG TEST COMPARE ERROR Expected %04x Actual %04x \n",wWriteVal,wReadVal);
        return (ERROR560_PATTEST);
      }
    }  /*  for wWriteVal */

  return (OK);

}  /*  End TestFifo */
