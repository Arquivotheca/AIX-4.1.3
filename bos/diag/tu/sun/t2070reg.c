static char sccsid[] = "@(#)37	1.1  src/bos/diag/tu/sun/t2070reg.c, tu_sunrise, bos411, 9437A411a 3/28/94 17:46:31";
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: Test2070AluRegs
 *		Test2070DwuRegs
 *		Test2070HiuRegs
 *		Test2070Ipu1Regs
 *		Test2070Ipu2Regs
 *		Test2070MmuRegs
 *		Test2070ObuRegs
 *		Test2070OpuRegs
 *		Test2070Regs
 *		Test2070SiuRegs
 *		Test2070ViuRegs
 *		Test2070VpuRegs
 *		Test2070VsuRegs
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
*       File Name:  t2070reg.c
*
*       Module Abstract: This module is the high level test routine
*               that enables the testing of the registers of the 2070.
*
*       Calling Sequence:
*       Test2070Regs()
*
*       Input Parameters:
*       none
*       Output Parameters:
*       none
*
*       Global Variables:
*       G_wTestShorts[MAXSHORTPATS]
*       G_iErrorCount
*       G_iErrorLimit
*       G_pvConfigArray[]
***********************************************************
*       Author: Kyle Pratt
*       Date:   11/3/92
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/
/* #includes --------------------------------------------*/
#include <stdio.h>

#include "iodvp.h"
#include "px2070.h"
#include "suntu.h"
#include "error.h"
#include "sun_tu_type.h"

#define MAXSHORTPATS            37

extern G_iErrorCount;
extern G_iErrorLimit;
extern G_pvConfigArray[];
extern G_Px2070PortType;
 /*  Global test pattern array */
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
unsigned int wTestResultsArray[MAX_SIU_INDX2];

int Test2070Regs()
{
  int rc;
  /*  Do a soft reset */
  if (rc = ioOCSWriteDVP(SW_RESET)) return (rc);

  if (rc=Test2070HiuRegs())
  {
       error.register_test.error_code = ERROR2070_HIUREGS;
       error.register_test.sub_error_code = rc;
       log_error(error);
       return(ERROR2070_HIUREGS);
  }
  if (rc=Test2070ViuRegs())
  {
       error.register_test.error_code = ERROR2070_VIUREGS;
       error.register_test.sub_error_code = rc;
       log_error(error);
       return(ERROR2070_VIUREGS);
  }
  if (rc=Test2070VsuRegs())
  {
       error.register_test.error_code = ERROR2070_VSUREGS;
       error.register_test.sub_error_code = rc;
       log_error(error);
       return(ERROR2070_VSUREGS);
  }
  if (rc=Test2070VpuRegs())
  {
       error.register_test.error_code = ERROR2070_VPUREGS;
       error.register_test.sub_error_code = rc;
       log_error(error);
       return(ERROR2070_VPUREGS);
  }
  if (rc=Test2070Ipu1Regs())
  {
       error.register_test.error_code = ERROR2070_IPU1REGS;
       error.register_test.sub_error_code = rc;
       log_error(error);
       return(ERROR2070_IPU1REGS);
  }
  if (rc=Test2070Ipu2Regs())
  {
       error.register_test.error_code = ERROR2070_IPU2REGS;
       error.register_test.sub_error_code = rc;
       log_error(error);
       return(ERROR2070_IPU2REGS);
  }
  /* Test2070AluRegs(); */
  if (rc=Test2070OpuRegs())
  {
       error.register_test.error_code = ERROR2070_OPUREGS;
       error.register_test.sub_error_code = rc;
       log_error(error);
       return(ERROR2070_OPUREGS);
  }
  if (rc=Test2070MmuRegs())
  {
       error.register_test.error_code = ERROR2070_MMUREGS;
       error.register_test.sub_error_code = rc;
       log_error(error);
       return(ERROR2070_MMUREGS);
  }
  if (rc=Test2070ObuRegs())
  {
       error.register_test.error_code = ERROR2070_OBUREGS;
       error.register_test.sub_error_code = rc;
       log_error(error);
       return(ERROR2070_OBUREGS);
  }
  /* Test2070DwuRegs(); */
  if (rc=Test2070SiuRegs())
  {
       error.register_test.error_code = ERROR2070_SIUREGS;
       error.register_test.sub_error_code = rc;
       log_error(error);
       return(ERROR2070_SIUREGS);
  }

   /*  Do a soft reset */
  if (rc = ioOCSWriteDVP(SW_RESET)) return (rc);
  if (rc = UpdateGen())  return(rc);
  return (OK);
}  /*  End Test2070Regs */
/**********************************************************
*
*       File Name:  t2070reg.c
*
*       Module Abstract: This module performs the diagnostic
*               testing of the registers in the HIU.
*
*       Calling Sequence:
*       Test2070HiuRegs()
*
*       Input Parameters:
*       none
*       Output Parameters:
*       none
*
*       Global Variables:
*       G_iTestPats[MAXPATTERNS]
*       G_iErrorCount
*       G_iErrorLimit
***********************************************************
*       Author: Kyle Pratt
*       Date:   11/3/92
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/
int Test2070HiuRegs()
{
int rc;
unsigned int wReadVal,wWriteVal,wCSUVal;
int i,iCurrentDVP;
PX2070Ports *pPX2070AddrStruct;

 /*  Skip the the HIU_CSU register test (a read only register) */

 /*  DBG and DRD registers aren't really testable so skip'em */

 /*  Test the HIU_OCS register */
for (i=0; i < MAXSHORTPATS; i++)
  {
   /*  Get the test value and write it */
  wWriteVal = G_wTestShorts[i] & HIU_OCS_MASK;
  if (rc = ioOCSWriteDVP(wWriteVal))  return (rc);

   /*  Is what you read what you wrote? */
  if (rc = ioOCSReadDVP(&wReadVal))  return (rc);
  wReadVal = wReadVal & HIU_OCS_MASK;
  if (wReadVal != wWriteVal)
    {
    error.register_test.expected_data = wWriteVal;
    error.register_test.actual_data = wReadVal;
    DEBUG_2("1 HIU_OCS READ ERROR Expected %04x Actual %04x \n",wWriteVal,wReadVal);
    return (ERROR2070_HIUREGS+1);
    }
  }  /*  for */
   /*  Reset the HIU_OCS register */
  if (rc = ioOCSWriteDVP(0)) return (rc);

 /*  Can't generate interrupts yet so skip IRQ register test */

 /*  Test the HIU_RIN (index) register */
for (i=0; i < MAXSHORTPATS; i++)
  {
   /*  Get the test value and write it */
  wWriteVal = G_wTestShorts[i] & HIU_RIN_MASK;
  if (rc = ioRINWriteDVP(wWriteVal)) return (rc);

   /*  Is what you read what you wrote? */
  if (rc = ioRINReadDVP(&wReadVal)) return (rc);
  wReadVal = wReadVal & HIU_RIN_MASK;
  if (wReadVal != wWriteVal)
    {
    error.register_test.expected_data = wWriteVal;
    error.register_test.actual_data = wReadVal;
    DEBUG_2("HIU_RIN READ ERROR Expected %04x Actual %04x \n",wWriteVal,wReadVal);
    return (ERROR2070_HIUREGS+2);
    }
  }  /*  for */

 /*  Test the HIU_ISU  register */
for (i=0; i < MAXSHORTPATS; i++)
  {
   /*  Get the test value and write it */
  wWriteVal = G_wTestShorts[i] & HIU_ISU_MASK;
  if (rc = ioRDTWriteDVP(HIU_ISU,wWriteVal)) return (rc);

   /*  Is what you read what you wrote? */
  if (rc = ioRDTReadDVP(HIU_ISU, &wReadVal)) return (rc);
  wReadVal = wReadVal & HIU_ISU_MASK;
  if (wReadVal != wWriteVal)
    {
    error.register_test.expected_data = wWriteVal;
    error.register_test.actual_data = wReadVal;
    DEBUG_2("HIU_ISU READ ERROR Expected %04x Actual %04x \n",wWriteVal,wReadVal);
    return (ERROR2070_HIUREGS+3);
    }
  }  /*  for */
  /*  Reset all registers */
  if (rc = ioOCSWriteDVP(SW_RESET)) return (rc);
  if (rc = ioOCSWriteDVP(0)) return (rc);
return (OK);
}  /*  End Test2070HiuRegs */

/**********************************************************
*
*       File Name:  t2070reg.c
*
*       Module Abstract: This module performs the diagnostic
*               testing of the registers in the VIU.
*
*       Calling Sequence:
*       Test2070ViuRegs()
*
*       Input Parameters:
*       none
*       Output Parameters:
*       none
*
*       Global Variables:
*       G_iTestPats[MAXPATTERNS]
*       G_iErrorCount
*       G_iErrorLimit
***********************************************************
*       Author: Kyle Pratt
*       Date:   11/3/92
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/
int Test2070ViuRegs()
{
int rc;
int iTestArrayIndex;
unsigned int wWriteVal,wReadVal,wIndex;
unsigned int wMaskArray[MAX_VIU_INDX1] = {VIU_MCR1_MASK,VIU_MCR2_MASK,
                                             VIU_DPC1_MASK,VIU_DPC2_MASK,
                                             VIU_WDT_MASK};

 /*  Do an address dependent test with out auto increment */

 /*  Write the data */
for (wWriteVal = VIU_INDEX_1; wWriteVal < VIU_INDEX_1 + MAX_VIU_INDX1;wWriteVal++)
   { if (rc = ioRDTWriteDVP(wWriteVal,(wWriteVal & wMaskArray[wWriteVal - VIU_INDEX_1]))) return (rc);
   }

 /*  Post 'em */
if (rc = UpdateGen())  return(rc);

 /*  Read the data back and test it */
for (wWriteVal = VIU_INDEX_1; wWriteVal < VIU_INDEX_1 + MAX_VIU_INDX1;wWriteVal++)
  {
  if (rc = ioRDTReadDVP(wWriteVal, &wReadVal)) return (rc);
  wReadVal = wReadVal & wMaskArray[wWriteVal - VIU_INDEX_1];
  if (wReadVal != (wWriteVal & wMaskArray[wWriteVal - VIU_INDEX_1]))
    {
    error.register_test.expected_data = wWriteVal & wMaskArray[wWriteVal - VIU_INDEX_1];
    error.register_test.actual_data = wReadVal;
    DEBUG_2("VIU READ ERROR Expected %04x Actual %04x \n",wWriteVal & wMaskArray[wWriteVal - VIU_INDEX_1],wReadVal);
    return (ERROR2070_VIUREGS+1);
    }
  }  /*  for wWriteVal */

 /*  Test each register in the VIU with the test array */

 /*  For each register */
for (wIndex = VIU_INDEX_1; wIndex < VIU_INDEX_1 + MAX_VIU_INDX1;wIndex++)
  {
   /*  Write the contents of the test array and test it */
  for (iTestArrayIndex = 0;iTestArrayIndex < MAXSHORTPATS;iTestArrayIndex++)
    {
    wWriteVal = G_wTestShorts[iTestArrayIndex];
    if (rc = ioRDTWriteDVP(wIndex,wWriteVal & wMaskArray[wIndex - VIU_INDEX_1])) return (rc);
    if (rc = UpdateGen())  return(rc);
    if (rc = ioRDTReadDVP(wIndex, &wReadVal)) return (rc);
    wReadVal = wReadVal & wMaskArray[wIndex - VIU_INDEX_1];
    if (wReadVal != (wWriteVal & wMaskArray[wIndex - VIU_INDEX_1]))
      {
      error.register_test.expected_data = wWriteVal & wMaskArray[wIndex - VIU_INDEX_1];
      error.register_test.actual_data = wReadVal;
      error.register_test.address = wIndex;
      DEBUG_3("VIU READ ERROR Index %04x Expected %04x Actual %04x \n",
           wIndex,wWriteVal & wMaskArray[wIndex - VIU_INDEX_1],wReadVal);
      return (ERROR2070_VIUREGS+2);
      }
    }  /*  for iTestIndex */
  }  /*  for wIndex */

 /*  Test the auto increment feature for the VIU registers */

 /*  Write out a bunch of words */
if (rc = ioRDTBlastWriteDVP(VIU_INDEX_1,G_wTestShorts,MAX_VIU_INDX1)) return (rc);

 /*  Post 'em */
if (rc = UpdateGen())  return(rc);

 /*  Read in a bunch of words */
if (rc = ioRDTBlastReadDVP(VIU_INDEX_1,wTestResultsArray,MAX_VIU_INDX1)) return (rc);

 /*  Test the data */
for (iTestArrayIndex = 0;iTestArrayIndex < MAX_VIU_INDX1;iTestArrayIndex++)
  {
  if ((G_wTestShorts[iTestArrayIndex] & wMaskArray[iTestArrayIndex]) !=
        (wTestResultsArray[iTestArrayIndex] & wMaskArray[iTestArrayIndex]))
    {
    DEBUG_2("3 VIU READ ERROR Expected %04x Actual %04x \n",
      G_wTestShorts[iTestArrayIndex] & wMaskArray[iTestArrayIndex],
      wTestResultsArray[iTestArrayIndex] & wMaskArray[iTestArrayIndex]);
    error.register_test.expected_data = G_wTestShorts[iTestArrayIndex] & wMaskArray[iTestArrayIndex];
    error.register_test.actual_data =  wTestResultsArray[iTestArrayIndex] & wMaskArray[iTestArrayIndex];
    return (ERROR2070_VIUREGS+3);
    }
  }  /*  for iTestIndex */
return (OK);
}  /*  End Test2070ViuRegs */

/**********************************************************
*
*       File Name:  t2070reg.c
*
*       Module Abstract: This module performs the diagnostic
*               testing of the registers in the VSU.
*
*       Calling Sequence:
*       Test2070VsuRegs()
*
*       Input Parameters:
*       none
*       Output Parameters:
*       none
*
*       Global Variables:
*       G_iTestPats[MAXPATTERNS]
*       G_iErrorCount
*       G_iErrorLimit
***********************************************************
*       Author: Kyle Pratt
*       Date:   11/3/92
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/
int Test2070VsuRegs()
{
int rc;
int iTestArrayIndex;
unsigned int wWriteVal,wReadVal,wIndex;
unsigned int wMaskArray[MAX_VSU_INDX1] = {VSU_HSW_MASK,VSU_HAD_MASK,
                                             VSU_HAP_MASK,VSU_HP_MASK,
                                             VSU_VSW_MASK,VSU_VAD_MASK,
                                             VSU_VAP_MASK,VSU_VP_MASK};
 /*  Do an address dependent test with out auto increment */

 /*  Write the data */
for (wWriteVal = VSU_INDEX_1; wWriteVal < VSU_INDEX_1 + MAX_VSU_INDX1;wWriteVal++)
   { if (rc = ioRDTWriteDVP(wWriteVal,wWriteVal)) return (rc);
   }
 /*  Post 'em */
if (rc = UpdateGen())  return(rc);

 /*  Read the data back and test it */
for (wWriteVal = VSU_INDEX_1; wWriteVal < VSU_INDEX_1 + MAX_VSU_INDX1;wWriteVal++)
  {
 if (rc = ioRDTReadDVP(wWriteVal, &wReadVal)) return (rc);
  wReadVal = wReadVal & wMaskArray[wWriteVal - VSU_INDEX_1];
  if (wReadVal != (wWriteVal & wMaskArray[wWriteVal - VSU_INDEX_1]))
    {
    DEBUG_2("VSU READ ERROR Expected %04x Actual %04x \n",
           wWriteVal & wMaskArray[wWriteVal - VSU_INDEX_1],wReadVal);
    error.register_test.expected_data =  wWriteVal & wMaskArray[wWriteVal - VSU_INDEX_1];
    error.register_test.actual_data =  wReadVal;
    return (ERROR2070_VSUREGS+1);
    }
  }  /*  for wWriteVal */

 /*  Test each register in the VSU with the test array */

 /*  For each register */
for (wIndex = VSU_INDEX_1; wIndex < VSU_INDEX_1 + MAX_VSU_INDX1;wIndex++)
  {
   /*  Write the contents of the test array and test it */
  for (iTestArrayIndex = 0;iTestArrayIndex < MAXSHORTPATS;iTestArrayIndex++)
    {
    wWriteVal = G_wTestShorts[iTestArrayIndex];
    if (rc = ioRDTWriteDVP(wIndex,wWriteVal)) return (rc);
     /*  Post 'em */
    if (rc = UpdateGen())  return(rc);
    if (rc = ioRDTReadDVP(wIndex, &wReadVal)) return (rc);
    wReadVal = wReadVal & wMaskArray[wIndex - VSU_INDEX_1];
    if (wReadVal != (wWriteVal & wMaskArray[wIndex - VSU_INDEX_1]))
      {
      DEBUG_3("VSU READ ERROR Index %04x Expected %04x Actual %04x \n",
             wIndex,wWriteVal & wMaskArray[wIndex - VSU_INDEX_1],wReadVal);
      error.register_test.expected_data =  wWriteVal & wMaskArray[wIndex - VSU_INDEX_1];
      error.register_test.actual_data =  wReadVal;
      error.register_test.address =  wIndex;
      return (ERROR2070_VSUREGS+2);
      }
    }  /*  for iTestIndex */
  }  /*  for wIndex */

 /*  Test the auto increment feature for the VSU registers */

 /*  Write out a bunch of words */
if (rc = ioRDTBlastWriteDVP(VSU_INDEX_1,G_wTestShorts,MAX_VSU_INDX1)) return (rc);

 /*  Post 'em */
if (rc = UpdateGen())  return(rc);

 /*  Read in a bunch of words */
if (rc = ioRDTBlastReadDVP(VSU_INDEX_1,wTestResultsArray,MAX_VSU_INDX1)) return (rc);

 /*  Test the data */
for (iTestArrayIndex = 0;iTestArrayIndex < MAX_VSU_INDX1;iTestArrayIndex++)
  {
  if ((G_wTestShorts[iTestArrayIndex] & wMaskArray[iTestArrayIndex]) !=
        (wTestResultsArray[iTestArrayIndex] & wMaskArray[iTestArrayIndex]))
    {
    DEBUG_2("VSU READ ERROR Expected %04x Actual %04x \n",
          G_wTestShorts[iTestArrayIndex] & wMaskArray[iTestArrayIndex],
          wTestResultsArray[iTestArrayIndex] & wMaskArray[iTestArrayIndex]);
    error.register_test.expected_data =   G_wTestShorts[iTestArrayIndex] & wMaskArray[iTestArrayIndex];
    error.register_test.actual_data =   wTestResultsArray[iTestArrayIndex] & wMaskArray[iTestArrayIndex];
    return (ERROR2070_VSUREGS+3);
    }
  }  /*  for iTestIndex */
  return (OK);
}  /*  End Test2070VsuRegs */

/**********************************************************
*
*       File Name:  t2070reg.c
*
*       Module Abstract: This module performs the diagnostic
*               testing of the registers in the VPU.
*
*       Calling Sequence:
*       Test2070VpuRegs()
*
*       Input Parameters:
*       none
*       Output Parameters:
*       none
*
*       Global Variables:
*       G_iTestPats[MAXPATTERNS]
*       G_iErrorCount
*       G_iErrorLimit
***********************************************************
*       Author: Kyle Pratt
*       Date:   11/3/92
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/
int Test2070VpuRegs()
{
int rc;
int iTestArrayIndex;
unsigned int wWriteVal,wReadVal,wIndex;
unsigned int wMaskArray[MAX_VPU_INDX1] = {VPU_MCR_MASK};

 /*  Do an address dependent test with out auto increment */

 /*  Write the data */
for (wWriteVal = VPU_INDEX_1; wWriteVal < VPU_INDEX_1 + MAX_VPU_INDX1;wWriteVal++)
  { if (rc = ioRDTWriteDVP(wWriteVal,wWriteVal)) return (rc);
  }
 /*  Post 'em */
if (rc = UpdateGen())  return(rc);

 /*  Read the data back and test it */
for (wWriteVal = VPU_INDEX_1; wWriteVal < VPU_INDEX_1 + MAX_VPU_INDX1;wWriteVal++)
  {
  if (rc = ioRDTReadDVP(wWriteVal, &wReadVal)) return (rc);
  wReadVal = wReadVal & wMaskArray[0];
  if (wReadVal != (wWriteVal & wMaskArray[0]))
    {
    DEBUG_2("VPU READ ERROR Expected %04x Actual %04x \n",
           wWriteVal & wMaskArray[0],wReadVal);
    error.register_test.expected_data =  wWriteVal & wMaskArray[0];
    error.register_test.actual_data = wReadVal;
    return (ERROR2070_VPUREGS+1);
    }
  }  /*  for wWriteVal */

 /*  Test each register in the VPU with the test array */

 /*  For each register */
for (wIndex = VPU_INDEX_1; wIndex < VPU_INDEX_1 + MAX_VPU_INDX1;wIndex++)
  {
   /*  Write the contents of the test array and test it */
  for (iTestArrayIndex = 0;iTestArrayIndex < MAXSHORTPATS;iTestArrayIndex++)
    {
    wWriteVal = G_wTestShorts[iTestArrayIndex];
    if (rc = ioRDTWriteDVP(wIndex,wWriteVal)) return (rc);
     /*  Post 'em */
    if (rc = UpdateGen())  return(rc);
    if (rc = ioRDTReadDVP(wIndex, &wReadVal)) return (rc);
    wReadVal = wReadVal & wMaskArray[0];
    if (wReadVal != (wWriteVal & wMaskArray[0]))
      {
      DEBUG_3("VPU READ ERROR Index %04x Expected %04x Actual %04x \n",
              wIndex,wWriteVal & wMaskArray[0],wReadVal);
      error.register_test.expected_data =  wWriteVal & wMaskArray[0];
      error.register_test.actual_data = wReadVal;
      error.register_test.address = wIndex;
      return (ERROR2070_VPUREGS+2);
      }
    }  /*  for iTestIndex */
  }  /*  for wIndex */

 /*  Test the auto increment feature for the VPU registers */

 /*  Write out a bunch of words */
if (rc = ioRDTBlastWriteDVP(VPU_INDEX_1,G_wTestShorts,MAX_VPU_INDX1)) return (rc);

 /*  Post 'em */
if (rc = UpdateGen())  return(rc);

 /*  Read in a bunch of words */
if (rc = ioRDTBlastReadDVP(VPU_INDEX_1,wTestResultsArray,MAX_VPU_INDX1)) return (rc);

 /*  Test the data */
for (iTestArrayIndex = 0;iTestArrayIndex < MAX_VPU_INDX1;iTestArrayIndex++)
  {
  if ((G_wTestShorts[iTestArrayIndex]  & wMaskArray[0]) !=
      (wTestResultsArray[iTestArrayIndex] & wMaskArray[0]))
    {
    DEBUG_2("VPU READ ERROR Expected %04x Actual %04x \n",G_wTestShorts[iTestArrayIndex],wTestResultsArray[iTestArrayIndex]);
    error.register_test.expected_data = G_wTestShorts[iTestArrayIndex];
    error.register_test.actual_data = wTestResultsArray[iTestArrayIndex];
    return (ERROR2070_VPUREGS+3);
    }
  }  /*  for iTestIndex */
  return (OK);
}  /*  End Test2070VpuRegs */

/**********************************************************
*
*       File Name:  t2070reg.c
*
*       Module Abstract: This module performs the diagnostic
*               testing of the registers in the IPU1.
*
*       Calling Sequence:
*       Test2070Ipu1Regs()
*
*       Input Parameters:
*       none
*       Output Parameters:
*       none
*
*       Global Variables:
*       G_iTestPats[MAXPATTERNS]
*       G_iErrorCount
*       G_iErrorLimit
***********************************************************
*       Author: Kyle Pratt
*       Date:   11/3/92
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/
int Test2070Ipu1Regs()
{
int rc;
int iTestArrayIndex,iIndexCount,i;
int iMaskIndexA=0,iMaskIndexB=0,iMaskIndexC=0,iMaskIndexD=0;
unsigned int wWriteVal,wReadVal,wIndex;
int iMaxArray[4] = {MAX_IPU1_INDX1-3,
                   MAX_IPU1_INDX2,
                   MAX_IPU1_INDX3,
                   MAX_IPU1_INDX4};
unsigned int wIndexArray[4] = {IPU1_INDEX_1 + 3,
                                  IPU1_INDEX_2,
                                  IPU1_INDEX_3,
                                  IPU1_INDEX_4};
unsigned char bLutInArray[MAX_IPU1_LUT];
unsigned char bLutOutArray[MAX_IPU1_LUT] ={0};
unsigned int wMaskArray[MAX_IPU1_INDX1-3 + MAX_IPU1_INDX3 + MAX_IPU1_INDX4] =
                          {/*IPU1_PIX_MASK,IPU1_LIC_MASK,IPU1_FLC_MASK,*/
                           IPU1_LIR_MASK,IPU1_FIR_MASK,/*IPU1_LRB_MASK,
                           IPU1_LRD_MASK,*/IPU1_MCR1_MASK,IPU1_XBF1_MASK,
                           IPU1_XBI1_MASK,IPU1_XEI1_MASK,IPU1_XSF1_MASK,
                           IPU1_XSI1_MASK,IPU1_YBF1_MASK,IPU1_YBI1_MASK,
                           IPU1_YEI1_MASK,IPU1_YSF1_MASK,IPU1_YSI1_MASK,
                           IPU1_KFC1_MASK,IPU1_MMY1_MASK,IPU1_MMU1_MASK,
                           IPU1_MMV1_MASK,IPU1_MCR2_MASK,IPU1_XBF2_MASK,
                           IPU1_XBI2_MASK,IPU1_XEI2_MASK,IPU1_XSF2_MASK,
                           IPU1_XSI2_MASK,IPU1_YBF2_MASK,IPU1_YBI2_MASK,
                           IPU1_YEI2_MASK,IPU1_YSF2_MASK,IPU1_YSI2_MASK,
                           IPU1_KFC2_MASK,IPU1_MMY2_MASK,IPU1_MMU2_MASK,
                           IPU1_MMV2_MASK};
 /*  Setup the VIU to source a clock  for ipu1 */
if (rc = ioRDTWriteDVP(VIU_DPC1,IPU1_from_OP + IPU2_from_Host + OPU_to_Host)) return (rc);
if (rc = ioRDTWriteDVP(VIU_DPC2,IPU1_from_OP + IPU2_from_Host + OPU_to_Host)) return (rc);

 /*  Test each of the four index bases of IPU1 */
for (iIndexCount = 0;iIndexCount < 4;iIndexCount++)
  {
   /*  If this is the LUT index (1) skip it */
  if (iIndexCount == 1) continue;

   /*  Do an address dependent test with out auto increment */

   /*  Write the data */
  for (wWriteVal = wIndexArray[iIndexCount];
       wWriteVal < wIndexArray[iIndexCount] + iMaxArray[iIndexCount];
       wWriteVal++)
    {
    if (rc = ioRDTWriteDVP(wWriteVal,wWriteVal & wMaskArray[iMaskIndexA])) return (rc);
    iMaskIndexA++;
    }

   /*  Post 'em */
  if (rc = UpdateGen())  return(rc);

   /*  Read the data back and test it */
  for (wWriteVal = wIndexArray[iIndexCount];
       wWriteVal < wIndexArray[iIndexCount] + iMaxArray[iIndexCount];
       wWriteVal++)
    {
    if (rc = ioRDTReadDVP(wWriteVal, &wReadVal)) return (rc);
    wReadVal = wReadVal & wMaskArray[iMaskIndexD];
    if (wReadVal != (wWriteVal & wMaskArray[iMaskIndexD]))
      {
      DEBUG_2("IPU1 READ ERROR Expected %04x Actual %04x \n",
              wWriteVal & wMaskArray[iMaskIndexD],wReadVal);
      error.register_test.expected_data =  wWriteVal & wMaskArray[iMaskIndexD];
      error.register_test.actual_data = wReadVal;
      return (ERROR2070_IPU1REGS+1);
      }
    iMaskIndexD++;
    }  /*  for wWriteVal */

   /*  Test each register in the IPU1 with the test array */

   /*  For each register */
  for (wIndex = wIndexArray[iIndexCount];
       wIndex < wIndexArray[iIndexCount] + iMaxArray[iIndexCount];
       wIndex++)
    {
     /*  Write the contents of the test array and test it */
     /* for (iTestArrayIndex  */
    for (iTestArrayIndex = 0;iTestArrayIndex < 25;iTestArrayIndex++)
      {
      wWriteVal = G_wTestShorts[iTestArrayIndex];
      if (rc = ioRDTWriteDVP(wIndex,wWriteVal & wMaskArray[iMaskIndexB])) return (rc);
       /*  Post 'em */
      if (rc = UpdateGen())  return(rc);
      if (rc = ioRDTReadDVP(wIndex, &wReadVal)) return (rc);
      wReadVal = wReadVal & wMaskArray[iMaskIndexB];
      if (wReadVal != (wWriteVal & wMaskArray[iMaskIndexB]))
        {
        DEBUG_3("IPU1 READ ERROR Index %04x Expected %04x Actual %04x \n",
                wIndex,wWriteVal & wMaskArray[iMaskIndexB],wReadVal);
      error.register_test.expected_data =  wWriteVal & wMaskArray[iMaskIndexB];
      error.register_test.actual_data = wReadVal;
      error.register_test.address = wIndex;
      return (ERROR2070_IPU1REGS+2);
        }
      }  /*  for iTestIndex */
    iMaskIndexB++;
    }  /*  for wIndex */

   /*  Test the auto increment feature for the IPU1 registers */

   /*  Write out a bunch of words */
  if (rc = ioRDTBlastWriteDVP(wIndexArray[iIndexCount],G_wTestShorts,iMaxArray[iIndexCount])) return (rc);

   /*  Post 'em */
  if (rc = UpdateGen())  return(rc);

   /*  Read in a bunch of words */
  if (rc = ioRDTBlastReadDVP(wIndexArray[iIndexCount],wTestResultsArray,iMaxArray[iIndexCount])) return (rc);

   /*  Test the data */
  for (iTestArrayIndex = 0;
       iTestArrayIndex < iMaxArray[iIndexCount];
       iTestArrayIndex++)
    {
    if ((G_wTestShorts[iTestArrayIndex] & wMaskArray[iMaskIndexC]) !=
        (wTestResultsArray[iTestArrayIndex] & wMaskArray[iMaskIndexC]))
      {
      DEBUG_2("IPU1 READ ERROR Expected %04x Actual %04x \n",
             G_wTestShorts[iTestArrayIndex] & wMaskArray[iMaskIndexC],
             wTestResultsArray[iTestArrayIndex] & wMaskArray[iMaskIndexC]);
      error.register_test.expected_data =  G_wTestShorts[iTestArrayIndex] & wMaskArray[iMaskIndexC];
      error.register_test.actual_data = wTestResultsArray[iTestArrayIndex] & wMaskArray[iMaskIndexC];
      return (ERROR2070_IPU1REGS+3);
      }
    iMaskIndexC++;
    }  /*  for iTestIndex */
  }  /*  for iIndexCount */

 /*  Test the gamma lut in IPU1 */

 /*  Build the test data */
for (i = 0;i < MAX_IPU1_LUT;i++)
  {
  bLutInArray[i] = i/3;
  }

 /*  Write the data */
if (rc = ioLutWriteDVP(0,bLutInArray,MAX_IPU1_LUT)) return (rc);
/*  DEBUG_0("ERROR Unable to load IPU1 LUT\n");       */
else
  {
   /*  Read the data */
if (rc = ioLutReadDVP(0,bLutOutArray,MAX_IPU1_LUT)) return (rc);
/*    DEBUG_0("ERROR Unable to read IPU1 LUT\n");     */
  else
    {
     /*  Is what you read what you wrote? */
    for (i = 0;i < MAX_IPU1_LUT;i++)
      {
      if (bLutInArray[i] != bLutOutArray[i])
        {
        DEBUG_3("IPU1 LUT ERROR Index %02x Expected %02x Actual %02x\n",i,bLutInArray[i],bLutOutArray[i]);
        error.register_test.expected_data =  bLutInArray[i];
        error.register_test.actual_data = bLutOutArray[i];
        error.register_test.address = i;
        return (ERROR2070_IPU1REGS+4);
        }
      }  /*  for i */
    }  /*  else */
  }  /*  else   */
  return (OK);
}  /*  End Test2070Ipu1Regs */

/**********************************************************
*
*       File Name:  t2070reg.c
*
*       Module Abstract: This module performs the diagnostic
*               testing of the registers in the IPU2.
*
*       Calling Sequence:
*       Test2070Ipu2Regs()
*
*       Input Parameters:
*       none
*       Output Parameters:
*       none
*
*       Global Variables:
*       G_iTestPats[MAXPATTERNS]
*       G_iErrorCount
*       G_iErrorLimit
***********************************************************
*       Author: Kyle Pratt
*       Date:   11/3/92
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/
int Test2070Ipu2Regs()
{
int rc;
int iTestArrayIndex,iIndexCount;
int iMaskIndexA=0,iMaskIndexB=0,iMaskIndexC=0;
unsigned int wMaskArray[MAX_IPU2_INDX1-3 + MAX_IPU2_INDX2 + MAX_IPU2_INDX3] =
                          {/*IPU2_PIX_MASK,IPU2_LIC_MASK,IPU2_FLC_MASK,*/
                           IPU2_LIR_MASK,IPU2_FIR_MASK,IPU2_MCR1_MASK,
                           IPU2_XBI1_MASK,IPU2_XEI1_MASK,IPU2_YBI1_MASK,
                           IPU2_YEI1_MASK,IPU2_MCR2_MASK,IPU2_XBI2_MASK,
                           IPU2_XEI2_MASK,IPU2_YBI2_MASK,IPU2_YEI2_MASK};

unsigned int wWriteVal,wReadVal,wIndex;
int iMaxArray[3] = {MAX_IPU2_INDX1-3,
                   MAX_IPU2_INDX2,
                   MAX_IPU2_INDX3};
unsigned int wIndexArray[3] = {IPU2_INDEX_1+3,
                                  IPU2_INDEX_2,
                                  IPU2_INDEX_3};

 /*  Test each of the three index bases of IPU2 */
for (iIndexCount = 0;iIndexCount < 3;iIndexCount++)
 {
   /*  Do an address dependent test with out auto increment */

   /*  Write the data */
  for (wWriteVal = wIndexArray[iIndexCount];
       wWriteVal < wIndexArray[iIndexCount] + iMaxArray[iIndexCount];
       wWriteVal++)
    {
     /*  Skip over unwed addresses */
    if ((iIndexCount != 0) && (((wWriteVal & 0xf) == 1) ||
                               ((wWriteVal & 0xf) == 4) ||
                               ((wWriteVal & 0xf) == 5) ||
                               ((wWriteVal & 0xf) == 6))) continue;
    if (rc = ioRDTWriteDVP(wWriteVal,wWriteVal & wMaskArray[iMaskIndexA])) return (rc);
    iMaskIndexA++;
    }

   /*  Post 'em */
  if (rc = UpdateGen())  return(rc);

   /* iMaskIndexA */
   /*  Read the data back and test it */
  for (wWriteVal = wIndexArray[iIndexCount];
       wWriteVal < wIndexArray[iIndexCount] + iMaxArray[iIndexCount];
       wWriteVal++)
    {
     /*  Skip over unwed addresses */
    if ((iIndexCount != 0) && (((wWriteVal & 0xf) == 1) ||
                               ((wWriteVal & 0xf) == 4) ||
                               ((wWriteVal & 0xf) == 5) ||
                               ((wWriteVal & 0xf) == 6))) continue;
 if (rc = ioRDTReadDVP(wWriteVal, &wReadVal)) return (rc);
    wReadVal = wReadVal & wMaskArray[iMaskIndexC];
    if (wReadVal != (wWriteVal & wMaskArray[iMaskIndexC]))
      {
      DEBUG_2("IPU2 READ ERROR Expected %04x Actual %04x \n",
              wWriteVal & wMaskArray[iMaskIndexA],wReadVal);
      error.register_test.expected_data =  wWriteVal & wMaskArray[iMaskIndexA];
      error.register_test.actual_data = wReadVal;
      return (ERROR2070_IPU2REGS+1);
      }
    iMaskIndexC++;
    }  /*  for wWriteVal */

   /*  Test each register in the IPU2 with the test array */

   /*  For each register */
  for (wIndex = wIndexArray[iIndexCount];
       wIndex < wIndexArray[iIndexCount] + iMaxArray[iIndexCount];
       wIndex++)
    {
     /*  Skip over unwed addresses */
    if ((iIndexCount != 0) && (((wIndex & 0xf) == 1) ||
                               ((wIndex & 0xf) == 4) ||
                               ((wIndex & 0xf) == 5) ||
                               ((wIndex & 0xf) == 6))) continue;

     /*  Write the contents of the test array and test it */
    for (iTestArrayIndex = 0;iTestArrayIndex < MAXSHORTPATS;iTestArrayIndex++)
      {
      wWriteVal = G_wTestShorts[iTestArrayIndex];
      if (rc = ioRDTWriteDVP(wIndex,wWriteVal & wMaskArray[iMaskIndexB])) return (rc);
      if (rc = UpdateGen())  return(rc);
      if (rc = ioRDTReadDVP(wIndex, &wReadVal)) return (rc);
      wReadVal = wReadVal & wMaskArray[iMaskIndexB];
      if (wReadVal != (wWriteVal & wMaskArray[iMaskIndexB]))
        {
        DEBUG_3("IPU2 READ ERROR Index %04x Expected %04x Actual %04x \n",
                wIndex,wWriteVal & wMaskArray[iMaskIndexB],wReadVal);
        error.register_test.expected_data = wWriteVal & wMaskArray[iMaskIndexB];
        error.register_test.actual_data = wReadVal;
        error.register_test.address = wIndex;
        return (ERROR2070_IPU2REGS+2);
        }
      }  /*  for iTestIndex */
    iMaskIndexB++;
    }  /*  for wIndex */
  }  /*  for iIndexCount */
  return (OK);
}  /*  End Test2070Ipu2Regs */

/**********************************************************
*
*       File Name:  t2070reg.c
*
*       Module Abstract: This module performs the diagnostic
*               testing of the registers in the OPU.
*
*       Calling Sequence:
*       Test2070OpuRegs()
*
*       Input Parameters:
*       none
*       Output Parameters:
*       none
*
*       Global Variables:
*       G_iTestPats[MAXPATTERNS]
*       G_iErrorCount
*       G_iErrorLimit
***********************************************************
*       Author: Kyle Pratt
*       Date:   11/3/92
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/
int Test2070OpuRegs()
{
int rc;
int iTestArrayIndex,iIndexCount;
int iMaskIndexA=0,iMaskIndexB=0;
unsigned int wWriteVal,wReadVal,wIndex;
int iMaxArray[2] = {MAX_OPU_INDX1,
                   MAX_OPU_INDX2};
unsigned int wIndexArray[2] = {OPU_INDEX_1,
                                  OPU_INDEX_2};
unsigned int wMaskArray[MAX_OPU_INDX1 + MAX_OPU_INDX2] =
                          {OPU_MCR1_MASK,OPU_XBI1_MASK,OPU_XEI1_MASK,
                          OPU_YBI1_MASK,OPU_YEI1_MASK,OPU_MCR2_MASK,
                          OPU_XBI2_MASK,OPU_XEI2_MASK,OPU_YBI2_MASK,
                          OPU_YEI2_MASK};

 /*  Test each of the two index bases of OPU */
for (iIndexCount = 0;iIndexCount < 2;iIndexCount++)
 {
   /*  Do an address dependent test with out auto increment */

   /*  Write the data */
  for (wWriteVal = wIndexArray[iIndexCount];
       wWriteVal < wIndexArray[iIndexCount] + iMaxArray[iIndexCount];
       wWriteVal++)
    {
     /*  Skip over unwed addresses */
    if (((wWriteVal & 0xf) == 1) ||
                               ((wWriteVal & 0xf) == 4) ||
                               ((wWriteVal & 0xf) == 5) ||
                               ((wWriteVal & 0xf) == 6)) continue;
    if (rc = ioRDTWriteDVP(wWriteVal,wWriteVal)) return (rc);
    }
   /*  Post 'em */
  if (rc = UpdateGen())  return(rc);

   /*  Read the data back and test it */
  for (wWriteVal = wIndexArray[iIndexCount];
       wWriteVal < wIndexArray[iIndexCount] + iMaxArray[iIndexCount];
       wWriteVal++)
    {
     /*  Skip over unwed addresses */
    if (((wWriteVal & 0xf) == 1) ||
                               ((wWriteVal & 0xf) == 4) ||
                               ((wWriteVal & 0xf) == 5) ||
                               ((wWriteVal & 0xf) == 6)) continue;
 if (rc = ioRDTReadDVP(wWriteVal, &wReadVal)) return (rc);
    wReadVal = wReadVal & wMaskArray[iMaskIndexA];
    if (wReadVal != (wWriteVal & wMaskArray[iMaskIndexA]))
      {
      DEBUG_2("OPU READ ERROR Expected %04x Actual %04x \n",
             wWriteVal & wMaskArray[iMaskIndexA],wReadVal);
      error.register_test.expected_data =   wWriteVal & wMaskArray[iMaskIndexA];
      error.register_test.actual_data = wReadVal;
      return (ERROR2070_OPUREGS+1);
      }
    iMaskIndexA++;
    }  /*  for wWriteVal */

   /*  Test each register in the OPU with the test array */

   /*  For each register */
  for (wIndex = wIndexArray[iIndexCount];
       wIndex < wIndexArray[iIndexCount] + iMaxArray[iIndexCount];
       wIndex++)
    {
     /*  Skip over unwed addresses */
    if (((wIndex & 0xf) == 1) ||
                               ((wIndex & 0xf) == 4) ||
                               ((wIndex & 0xf) == 5) ||
                               ((wIndex & 0xf) == 6)) continue;
     /*  Write the contents of the test array and test it */
    for (iTestArrayIndex = 0;iTestArrayIndex < MAXSHORTPATS;iTestArrayIndex++)
      {
      wWriteVal = G_wTestShorts[iTestArrayIndex];
      if (rc = ioRDTWriteDVP(wIndex,wWriteVal)) return (rc);
       /*  Post 'em */
      if (rc = UpdateGen())  return(rc);
      if (rc = ioRDTReadDVP(wIndex, &wReadVal)) return (rc);
      wReadVal = wReadVal & wMaskArray[iMaskIndexB];
      if (wReadVal != (wWriteVal & wMaskArray[iMaskIndexB]))
        {
        DEBUG_3("OPU READ ERROR Index %04x Expected %04x Actual %04x \n",
               wIndex,wWriteVal & wMaskArray[iMaskIndexB],wReadVal);
        error.register_test.expected_data =   wWriteVal & wMaskArray[iMaskIndexB];
        error.register_test.actual_data = wReadVal;
        error.register_test.address = wIndex;
        return (ERROR2070_OPUREGS+2);
        }
      }  /*  for iTestIndex */
    iMaskIndexB++;
    }  /*  for wIndex */
  }  /*  for iIndexCount */
  return (OK);
}  /*  End Test2070OpuRegs */

/**********************************************************
*
*       File Name:  t2070reg.c
*
*       Module Abstract: This module performs the diagnostic
*               testing of the registers in the MMU.
*
*       Calling Sequence:
*       Test2070MmuRegs()
*
*       Input Parameters:
*       none
*       Output Parameters:
*       none
*
*       Global Variables:
*       G_iTestPats[MAXPATTERNS]
*       G_iErrorCount
*       G_iErrorLimit
***********************************************************
*       Author: Kyle Pratt
*       Date:   11/3/92
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/
int Test2070MmuRegs()
{
int rc;
int iTestArrayIndex;
unsigned int wWriteVal,wReadVal,wIndex;
 /*  Do an address dependent test with out auto increment */

 /*  Write the data */
for (wWriteVal = MMU_INDEX_1; wWriteVal < MMU_INDEX_1 + MAX_MMU_INDX1;wWriteVal++)
  { if (rc = ioRDTWriteDVP(wWriteVal,wWriteVal)) return (rc);
  }
 /*  Read the data back and test it */
for (wWriteVal = MMU_INDEX_1; wWriteVal < MMU_INDEX_1 + MAX_MMU_INDX1;wWriteVal++)
  {
 if (rc = ioRDTReadDVP(wWriteVal, &wReadVal)) return (rc);
  wReadVal = wReadVal & MMU_MCR_MASK;
  if (wReadVal != (wWriteVal & MMU_MCR_MASK))
    {
    DEBUG_2("MMU READ ERROR Expected %04x Actual %04x \n",
            wWriteVal & MMU_MCR_MASK,wReadVal);
        error.register_test.expected_data = wWriteVal & MMU_MCR_MASK;
        error.register_test.actual_data = wReadVal;
        return (ERROR2070_MMUREGS+1);
    }
  }  /*  for wWriteVal */

 /*  Test each register in the MMU with the test array */

 /*  For each register */
for (wIndex = MMU_INDEX_1; wIndex < MMU_INDEX_1 + MAX_MMU_INDX1;wIndex++)
  {
   /*  Write the contents of the test array and test it */
  for (iTestArrayIndex = 0;iTestArrayIndex < MAXSHORTPATS;iTestArrayIndex++)
    {
    wWriteVal = G_wTestShorts[iTestArrayIndex];
    if (rc = ioRDTWriteDVP(wIndex,wWriteVal)) return (rc);
    if (rc = ioRDTReadDVP(wIndex, &wReadVal)) return (rc);
    wReadVal = wReadVal & MMU_MCR_MASK;
    if (wReadVal != (wWriteVal & MMU_MCR_MASK))
      {
      DEBUG_3("MMU READ ERROR Index %04x Expected %04x Actual %04x \n",
              wIndex,wWriteVal & MMU_MCR_MASK,wReadVal);
        error.register_test.expected_data = wWriteVal & MMU_MCR_MASK;
        error.register_test.actual_data = wReadVal;
        error.register_test.address = wIndex;
        return (ERROR2070_MMUREGS+2);
      }
    }  /*  for iTestIndex */
  }  /*  for wIndex */
  return (OK);
}  /*  End Test2070MmuRegs */

/**********************************************************
*
*       File Name:  t2070reg.c
*
*       Module Abstract: This module performs the diagnostic
*               testing of the registers in the OBU.
*
*       Calling Sequence:
*       Test2070ObuRegs()
*
*       Input Parameters:
*       none
*       Output Parameters:
*       none
*
*       Global Variables:
*       G_iTestPats[MAXPATTERNS]
*       G_iErrorCount
*       G_iErrorLimit
***********************************************************
*       Author: Kyle Pratt
*       Date:   11/3/92
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/
int Test2070ObuRegs()
{
int rc;
int iTestArrayIndex,iIndexCount;
int iMaskIndexA=0,iMaskIndexB=0,iMaskIndexC=0;
unsigned int wWriteVal,wReadVal,wIndex;
int iMaxArray[1] = {MAX_OBU_INDX1};
unsigned int wIndexArray[8] = {OBU0_INDEX_1,OBU1_INDEX_1,
                                  OBU2_INDEX_1,OBU3_INDEX_1,
                                  OBU4_INDEX_1,OBU5_INDEX_1,
                                  OBU6_INDEX_1,OBU7_INDEX_1};
unsigned int wMaskArray[MAX_OBU_INDX1] =
                            {OBU_MCR_MASK,OBU_RFX_MASK,OBU_LSL_MASK,
                             OBU_LSH_MASK,OBU_BSX_MASK,OBU_BSY_MASK,
                             OBU_DEC_MASK};

 /*  Test each of the seven index bases of OBU */
for (iIndexCount = 0;iIndexCount < 8;iIndexCount++)
 {
 iMaskIndexA = iMaskIndexB = iMaskIndexC=0;
   /*  Do an address dependent test with out auto increment */

   /*  Write the data */
  for (wWriteVal = wIndexArray[iIndexCount];
       wWriteVal < wIndexArray[iIndexCount] + iMaxArray[0];
       wWriteVal++)
     { if (rc = ioRDTWriteDVP(wWriteVal,wWriteVal)) return (rc);
     }
   /* Post 'em */
  if (rc = UpdateGen())  return(rc);
   /*  Read the data back and test it */
  for (wWriteVal = wIndexArray[iIndexCount];
       wWriteVal < wIndexArray[iIndexCount] + iMaxArray[0];
       wWriteVal++)
    {
    if (rc = ioRDTReadDVP(wWriteVal, &wReadVal)) return (rc);
    wReadVal = wReadVal & wMaskArray[iMaskIndexA];
    if (wReadVal != (wWriteVal & wMaskArray[iMaskIndexA]))
      {
      DEBUG_2("OBU READ ERROR Expected %04x Actual %04x \n",
               wWriteVal & wMaskArray[iMaskIndexA],wReadVal);
      error.register_test.expected_data =wWriteVal & wMaskArray[iMaskIndexA];
      error.register_test.actual_data = wReadVal;
      return (ERROR2070_OBUREGS+1);
      }
    iMaskIndexA++;
    }  /*  for wWriteVal */

   /*  Test each register in the OBU with the test array */

   /*  For each register */
  for (wIndex = wIndexArray[iIndexCount];
       wIndex < wIndexArray[iIndexCount] + iMaxArray[0];
       wIndex++)
    {
     /*  Write the contents of the test array and test it */
    for (iTestArrayIndex = 0;iTestArrayIndex < MAXSHORTPATS;iTestArrayIndex++)
      {
      wWriteVal = G_wTestShorts[iTestArrayIndex];
      if (rc = ioRDTWriteDVP(wIndex,wWriteVal)) return (rc);
       /* Post 'em */
      if (rc = UpdateGen())  return(rc);
      if (rc = ioRDTReadDVP(wIndex, &wReadVal)) return (rc);
      wReadVal = wReadVal & wMaskArray[iMaskIndexB];
      if (wReadVal != (wWriteVal & wMaskArray[iMaskIndexB]))
        {
        DEBUG_3("OBU READ ERROR Index %04x Expected %04x Actual %04x \n",
                wIndex,wWriteVal & wMaskArray[iMaskIndexB],wReadVal);
        error.register_test.expected_data =wWriteVal & wMaskArray[iMaskIndexB];
        error.register_test.actual_data = wReadVal;
        error.register_test.address = wIndex;
        return (ERROR2070_OBUREGS+2);
        }
      }  /*  for iTestIndex */
    iMaskIndexB++;
    }  /*  for wIndex */

   /*  Test the auto increment feature for the OBU registers */

   /*  Write out a bunch of words */
  if (rc = ioRDTBlastWriteDVP(wIndexArray[iIndexCount],G_wTestShorts,iMaxArray[0])) return (rc);

   /* Post 'em */
  if (rc = UpdateGen())  return(rc);
   /*  Read in a bunch of words */
  if (rc = ioRDTBlastReadDVP(wIndexArray[iIndexCount],wTestResultsArray,iMaxArray[0])) return (rc);

   /*  Test the data */
  for (iTestArrayIndex = 0;
       iTestArrayIndex < iMaxArray[0];
       iTestArrayIndex++)
    {
    if ((G_wTestShorts[iTestArrayIndex] & wMaskArray[iMaskIndexC]) !=
          (wTestResultsArray[iTestArrayIndex] & wMaskArray[iMaskIndexC]))
      {
      DEBUG_2("OBU READ ERROR Expected %04x Actual %04x \n",
             G_wTestShorts[iTestArrayIndex] & wMaskArray[iMaskIndexC],
             wTestResultsArray[iTestArrayIndex] & wMaskArray[iMaskIndexC]);
        error.register_test.expected_data =  G_wTestShorts[iTestArrayIndex] & wMaskArray[iMaskIndexC];
        error.register_test.actual_data =  wTestResultsArray[iTestArrayIndex] & wMaskArray[iMaskIndexC];
        return (ERROR2070_OBUREGS+3);
      }
    iMaskIndexC++;
    }  /*  for iTestIndex */
  }  /*  for iIndexCount */
  return (OK);
}  /*  End Test2070ObuRegs */

/**********************************************************
*
*       File Name:  t2070reg.c
*
*       Module Abstract: This module performs the diagnostic
*               testing of the registers in the SIU.
*
*       Calling Sequence:
*       Test2070SiuRegs()
*
*       Input Parameters:
*       none
*       Output Parameters:
*       none
*
*       Global Variables:
*       G_iTestPats[MAXPATTERNS]
*       G_iErrorCount
*       G_iErrorLimit
***********************************************************
*       Author: Kyle Pratt
*       Date:   11/3/92
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/
int Test2070SiuRegs()
{
int rc;
int iTestArrayIndex,iIndexCount;
int iMaskIndexA=0,iMaskIndexB=0,iMaskIndexC=0;
unsigned int wWriteVal,wReadVal,wIndex;
int iMaxArray[2] = {MAX_SIU_INDX1,
                   MAX_SIU_INDX2};
unsigned int wIndexArray[2] = {SIU_INDEX_1-2,
                                  SIU_INDEX_2};
unsigned int wMaskArray[MAX_SIU_INDX1 + MAX_SIU_INDX2] =
                          {SIU_MCR_MASK,SIU_FCS_MASK,SIU_FOU_MASK,
                           SIU_SIM0_MASK,SIU_SIM1_MASK,SIU_SIM2_MASK,
                           SIU_SIM3_MASK,SIU_SIM4_MASK,SIU_SIM5_MASK,
                           SIU_SIM6_MASK,SIU_SIM7_MASK,SIU_SIM8_MASK,
                           SIU_SIM9_MASK,SIU_SIM10_MASK,SIU_SIM11_MASK,
                           SIU_SIM12_MASK,SIU_SIM13_MASK,SIU_SIM14_MASK,
                           SIU_SIM15_MASK,SIU_SIM16_MASK,SIU_SIM17_MASK,
                           SIU_SIM18_MASK,SIU_SIM19_MASK,SIU_SIM20_MASK,
                           SIU_SIM21_MASK,SIU_SIM22_MASK,SIU_SIM23_MASK,
                           SIU_SIM24_MASK,SIU_SIM25_MASK,SIU_SIM26_MASK,
                           SIU_SIM27_MASK,SIU_SIM28_MASK,SIU_SIM29_MASK,
                           SIU_SIM30_MASK,SIU_SIM31_MASK};

 /*  Test each of the two index bases of SIU */
for (iIndexCount = 0;iIndexCount < 2;iIndexCount++)
 {
   /*  Do an address dependent test with out auto increment */

   /*  Write the data */
  for (wWriteVal = wIndexArray[iIndexCount];
       wWriteVal < wIndexArray[iIndexCount] + iMaxArray[iIndexCount];
       wWriteVal++)
  {  if (rc = ioRDTWriteDVP(wWriteVal,wWriteVal)) return (rc);
  }
   /*  Post 'em */
  if (rc = UpdateGen())  return(rc);

   /*  Read the data back and test it */
  for (wWriteVal = wIndexArray[iIndexCount];
       wWriteVal < wIndexArray[iIndexCount] + iMaxArray[iIndexCount];
       wWriteVal++)
    {
    if (rc = ioRDTReadDVP(wWriteVal, &wReadVal)) return (rc);
    wReadVal = wReadVal & wMaskArray[iMaskIndexA];
    if (wReadVal != (wWriteVal & wMaskArray[iMaskIndexA]))
      {
      DEBUG_2("SIU READ ERROR Expected %04x Actual %04x \n",
              wWriteVal & wMaskArray[iMaskIndexA],wReadVal);
      error.register_test.expected_data = wWriteVal & wMaskArray[iMaskIndexA];
      error.register_test.actual_data = wReadVal;
      return (ERROR2070_SIUREGS+1);
      }
    iMaskIndexA++;
    }  /*  for wWriteVal */

   /*  Test each register in the SIU with the test array */

   /*  For each register */
  for (wIndex = wIndexArray[iIndexCount];
       wIndex < wIndexArray[iIndexCount] + iMaxArray[iIndexCount];
       wIndex++)
    {
     /*  Write the contents of the test array and test it */
    for (iTestArrayIndex = 0;iTestArrayIndex < MAXSHORTPATS;iTestArrayIndex++)
     /* for (iTestArrayIndex  */
      {
      wWriteVal = G_wTestShorts[iTestArrayIndex];
      if (rc = ioRDTWriteDVP(wIndex,wWriteVal)) return (rc);
       /*  Post 'em */
      if (rc = UpdateGen())  return(rc);

      if (rc = ioRDTReadDVP(wIndex, &wReadVal)) return (rc);
      wReadVal = wReadVal & wMaskArray[iMaskIndexB];
      if (wReadVal != (wWriteVal & wMaskArray[iMaskIndexB]))
        {
        DEBUG_3("SIU READ ERROR Index %04x Expected %04x Actual %04x \n",
                wIndex,wWriteVal & wMaskArray[iMaskIndexB],wReadVal);
        error.register_test.expected_data = wWriteVal & wMaskArray[iMaskIndexB];
        error.register_test.actual_data = wReadVal;
        error.register_test.address = wIndex;
        return (ERROR2070_SIUREGS+2);
        }
      }  /*  for iTestIndex */
    iMaskIndexB++;
    }  /*  for wIndex */

   /*  Test the auto increment feature for the SIU registers */

   /*  Write out a bunch of words */
  if (rc = ioRDTBlastWriteDVP(wIndexArray[iIndexCount],G_wTestShorts,iMaxArray[iIndexCount])) return (rc);

   /*  Post 'em */
  if (rc = UpdateGen())  return(rc);

   /*  Read in a bunch of words */
  if (rc = ioRDTBlastReadDVP(wIndexArray[iIndexCount],wTestResultsArray,iMaxArray[iIndexCount])) return (rc);

   /*  Test the data */
  for (iTestArrayIndex = 0;
       iTestArrayIndex < iMaxArray[iIndexCount];
       iTestArrayIndex++)
    {
    if ((G_wTestShorts[iTestArrayIndex] & wMaskArray[iMaskIndexC]) !=
          (wTestResultsArray[iTestArrayIndex] & wMaskArray[iMaskIndexC]))
      {
      DEBUG_2("SIU READ ERROR Expected %04x Actual %04x \n",
             G_wTestShorts[iTestArrayIndex] & wMaskArray[iMaskIndexC],
             wTestResultsArray[iTestArrayIndex] & wMaskArray[iMaskIndexC]);
      error.register_test.expected_data =  G_wTestShorts[iTestArrayIndex] & wMaskArray[iMaskIndexC];
      error.register_test.actual_data =  wTestResultsArray[iTestArrayIndex] & wMaskArray[iMaskIndexC];
      return (ERROR2070_SIUREGS+3);
      }
    }  /*  for iTestIndex */
  iMaskIndexC++;
  }  /*  for iIndexCount */
  return (OK);
}  /*  End Test2070SiuRegs */

/**********************************************************
*
*       File Name:  t2070reg.c
*
*       Module Abstract: This module performs the diagnostic
*               testing of the registers in the ALU.
*
*       Calling Sequence:
*       Test2070AluRegs()
*
*       Input Parameters:
*       none
*       Output Parameters:
*       none
*
*       Global Variables:
*       G_iTestPats[MAXPATTERNS]
*       G_iErrorCount
*       G_iErrorLimit
***********************************************************
*       Author: Kyle Pratt
*       Date:   11/3/92
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/
int Test2070AluRegs()
{
int rc;
int iTestArrayIndex;
int iMaskIndex=0;
unsigned int wMaskArray[MAX_ALU_INDX1] =
                          {ALU_MCR1_MASK,ALU_MCR2_MASK,ALU_TOP_MASK,
                           ALU_AV_MASK,ALU_LOPY_MASK,ALU_LOPU_MASK,
                           ALU_LOPV_MASK,ALU_CAY_MASK,ALU_CAU_MASK,
                           ALU_CAV_MASK,ALU_CBY_MASK,ALU_CBU_MASK,
                           ALU_CBV_MASK,ALU_CCY_MASK,ALU_CCU_MASK,
                           ALU_CCV_MASK};
unsigned int wWriteVal,wReadVal,wIndex;
 /*  Do an address dependent test with out auto increment */

 /*  Write the data */
for (wWriteVal = ALU_INDEX_1; wWriteVal < ALU_INDEX_1 + MAX_ALU_INDX1;wWriteVal++)
  { if (rc = ioRDTWriteDVP(wWriteVal,wWriteVal & wMaskArray[iMaskIndex])) return (rc);
  }
   /*  Post 'em */
  if (rc = UpdateGen())  return(rc);

 /*  Read the data back and test it */
for (wWriteVal = ALU_INDEX_1; wWriteVal < ALU_INDEX_1 + MAX_ALU_INDX1;wWriteVal++)
  {
 if (rc = ioRDTReadDVP(wWriteVal, &wReadVal)) return (rc);
  wReadVal = wReadVal & wMaskArray[iMaskIndex];
  if (wReadVal != (wWriteVal & wMaskArray[iMaskIndex]))
    {
    DEBUG_2("ALU READ ERROR Expected %04x Actual %04x \n",
            wWriteVal & wMaskArray[iMaskIndex],wReadVal);
    return (ERROR2070_ALUREGS);
    }
  iMaskIndex++;
  }  /*  for wWriteVal */

 /*  Test each register in the ALU with the test array */

 /*  For each register */
iMaskIndex=0;
for (wIndex = ALU_INDEX_1; wIndex < ALU_INDEX_1 + MAX_ALU_INDX1;wIndex++)
  {
   /*  Write the contents of the test array and test it */
   /* for (iTestArrayIndex  */
  for (iTestArrayIndex = 0;iTestArrayIndex < 25;iTestArrayIndex++)
    {
    wWriteVal = G_wTestShorts[iTestArrayIndex];
    if (rc = ioRDTWriteDVP(wIndex,wWriteVal & wMaskArray[iMaskIndex])) return (rc);
     /*  Post 'em */
    if (rc = UpdateGen())  return(rc);

 if (rc = ioRDTReadDVP(wIndex, &wReadVal)) return (rc);
    wReadVal = wReadVal & wMaskArray[iMaskIndex];
    if (wReadVal != (wWriteVal & wMaskArray[iMaskIndex]))
      {
      DEBUG_3("ALU READ ERROR Index %04x Expected %04x Actual %04x \n",
              wIndex,wWriteVal & wMaskArray[iMaskIndex],wReadVal);
      return (ERROR2070_ALUREGS);
      }
    }  /*  for iTestIndex */
  iMaskIndex++;
  }  /*  for wIndex */

 /*  Test the auto increment feature for the ALU registers */
iMaskIndex=0;
 /*  Write out a bunch of words */
if (rc = ioRDTBlastWriteDVP(ALU_INDEX_1,G_wTestShorts,MAX_ALU_INDX1)) return (rc);

 /*  Post 'em */
if (rc = UpdateGen())  return(rc);


 /*  Read in a bunch of words */
if (rc = ioRDTBlastReadDVP(ALU_INDEX_1,wTestResultsArray,MAX_ALU_INDX1)) return (rc);

 /*  Test the data */
for (iTestArrayIndex = 0;iTestArrayIndex < MAX_ALU_INDX1;iTestArrayIndex++)
  {
  if ((G_wTestShorts[iTestArrayIndex]  & wMaskArray[iMaskIndex]) !=
      (wTestResultsArray[iTestArrayIndex] & wMaskArray[iMaskIndex]))
    {
    DEBUG_2("ALU READ ERROR Expected %04x Actual %04x \n",
           G_wTestShorts[iTestArrayIndex] & wMaskArray[iMaskIndex],
           wTestResultsArray[iTestArrayIndex] & wMaskArray[iMaskIndex]);
    return (ERROR2070_ALUREGS);
    }
  iMaskIndex++;
  }  /*  for iTestIndex */
  return (OK);
}  /*  End Test2070AluRegs */



/**********************************************************
*
*       File Name:  t2070reg.c
*
*       Module Abstract: This module performs the diagnostic
*               testing of the registers in the DWU.
*
*       Calling Sequence:
*       Test2070DwuRegs()
*
*       Input Parameters:
*       none
*       Output Parameters:
*       none
*
*       Global Variables:
*       G_iTestPats[MAXPATTERNS]
*       G_iErrorCount
*       G_iErrorLimit
***********************************************************
*       Author: Kyle Pratt
*       Date:   11/3/92
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/
int Test2070DwuRegs()
{
int rc;
int iTestArrayIndex,iIndexCount;
int iMaskIndexA,iMaskIndexB,iMaskIndexC;
unsigned int wWriteVal,wReadVal,wIndex;
int iMaxArray[5] = {MAX_DWU_INDX1,
                    MAX_DWU_INDX2,
                    MAX_DWU_INDX2,
                    MAX_DWU_INDX2,
                    MAX_DWU_INDX2,};
unsigned int wIndexArray[5] = {DWU_INDEX_1,
                                  DWU0_INDEX_2,
                                  DWU1_INDEX_2,
                                  DWU2_INDEX_2,
                                  DWU3_INDEX_2};
unsigned int wMaskArray[MAX_DWU_INDX1 + MAX_DWU_INDX2] =
                                {DWU_MCR_MASK,DWU_HCR_MASK,
                                 DWU_DZF_MASK,DWU_RFX_MASK,
                                 DWU_LSL_MASK,DWU_LSH_MASK,
                                 DWU_WSX_MASK,DWU_WSY_MASK,
                                 DWU_DSX_MASK,DWU_DSY_MASK};

 /*  Test each of the five index bases of DWU */
for (iIndexCount = 0;iIndexCount < 5;iIndexCount++)
 {
 if (iIndexCount == 0)
   iMaskIndexA = iMaskIndexB = iMaskIndexC=0;
 else
   iMaskIndexA = iMaskIndexB = iMaskIndexC=MAX_DWU_INDX1;
   /*  Do an address dependent test with out auto increment */

   /*  Write the data */
  for (wWriteVal = wIndexArray[iIndexCount];
       wWriteVal < wIndexArray[iIndexCount] + iMaxArray[iIndexCount];
       wWriteVal++)
    {if (rc = ioRDTWriteDVP(wWriteVal,wWriteVal)) return (rc);
    }
   /* Post 'em */
  if (rc = UpdateGen())  return(rc);

   /*  Read the data back and test it */
  for (wWriteVal = wIndexArray[iIndexCount];
       wWriteVal < wIndexArray[iIndexCount] + iMaxArray[iIndexCount];
       wWriteVal++)
    {
 if (rc = ioRDTReadDVP(wWriteVal, &wReadVal)) return (rc);
    wReadVal = wReadVal & wMaskArray[iMaskIndexA];
    if (wReadVal != (wWriteVal & wMaskArray[iMaskIndexA]))
      {
      DEBUG_2("DWU READ ERROR Expected %04x Actual %04x \n",
              wWriteVal & wMaskArray[iMaskIndexA],wReadVal);
      return (ERROR2070_DWUREGS);
      }
    iMaskIndexA++;
    }  /*  for wWriteVal */

   /*  Test each register in the DWU with the test array */

   /*  For each register */
  for (wIndex = wIndexArray[iIndexCount];
       wIndex < wIndexArray[iIndexCount] + iMaxArray[iIndexCount];
       wIndex++)
    {
     /*  Write the contents of the test array and test it */
    for (iTestArrayIndex = 0;iTestArrayIndex < MAXSHORTPATS;iTestArrayIndex++)
      {
      wWriteVal = G_wTestShorts[iTestArrayIndex];
      if (rc = ioRDTWriteDVP(wIndex,wWriteVal)) return (rc);
       /* Post 'em */
      if (rc = UpdateGen())  return(rc);
 if (rc = ioRDTReadDVP(wIndex, &wReadVal)) return (rc);
      wReadVal = wReadVal & wMaskArray[iMaskIndexB];
      if (wReadVal != (wWriteVal & wMaskArray[iMaskIndexB]))
        {
        DEBUG_3("DWU READ ERROR Index %04x Expected %04x Actual %04x \n",
               wIndex,wWriteVal & wMaskArray[iMaskIndexB],wReadVal);
      return (ERROR2070_DWUREGS);
        }
      }  /*  for iTestIndex */
    iMaskIndexB++;
    }  /*  for wIndex */

   /*  Test the auto increment feature for the DWU registers */

   /*  Write out a bunch of words */
  if (rc = ioRDTBlastWriteDVP(wIndexArray[iIndexCount],G_wTestShorts,iMaxArray[iIndexCount])) return (rc);
   /*  Post 'em */
  if (rc = UpdateGen())  return(rc);
   /*  Read in a bunch of words */
  if (rc = ioRDTBlastReadDVP(wIndexArray[iIndexCount],wTestResultsArray,iMaxArray[iIndexCount])) return (rc);

   /*  Test the data */
  for (iTestArrayIndex = 0;
       iTestArrayIndex < iMaxArray[iIndexCount];
       iTestArrayIndex++)
    {
    if ((G_wTestShorts[iTestArrayIndex] & wMaskArray[iMaskIndexC]) !=
         (wTestResultsArray[iTestArrayIndex] & wMaskArray[iMaskIndexC]))
      {
      DEBUG_2("DWU READ ERROR Expected %04x Actual %04x \n",
              G_wTestShorts[iTestArrayIndex] & wMaskArray[iMaskIndexC],
              wTestResultsArray[iTestArrayIndex] & wMaskArray[iMaskIndexC]);
      return (ERROR2070_DWUREGS);
      }
    iMaskIndexC++;
    }  /*  for iTestIndex */
  }  /*  for iIndexCount */
  return (OK);
}  /*  End Test2070DwuRegs */

