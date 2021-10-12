static char sccsid[] = "@(#)55  1.3  src/bos/diag/tu/sun/560ToFrom2070.c, tu_sunrise, bos411, 9437A411a 5/27/94 14:52:34";
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: c560From2070
 *              c560To2070
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
/**************************************************************************/
/*  File:  560ToFrom2070.C                                                */
/**************************************************************************/
#include <stdio.h>
#include "px2070.h"
#include "iodvp.h"
#include "suntu.h"
#include "error.h"
#include "sun_tu_type.h"

/*****************************************************************************/
/*  c560To2070()                                                             */
/*                                                                           */
/*  FUNCTION:                                                                */
/*    This test will setup the Pixel 2070 to receive transfered              */
/*    data from Cube 550.                                                    */
/*                                                                           */
/*  INPUT:    None                                                           */
/*                                                                           */
/*  OUTPUT:   return value     - OK if successful, ERROR if NOT.             */
/*****************************************************************************/
int c560To2070()
{
 int iTestCnt,iXSize, iYSize,iLoopCount, FifoFull, FifoEmpty, rc;
 unsigned int bExpected;
 unsigned int wReadVal,wWriteVal,wRowCount,wColCount,wExpected;
 unsigned int dwReadVal,dwExpected;
 long lTimeCount;

iXSize = 1024;
iYSize = 512;

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
 if (rc=ioRDTWriteDVP(SIU_SIM0,(IPU2_to_OB0 + offset_0 /*+ SIU_EXIT*/))) return(rc);

 /*  Setup Object (object */
 /*     normal addressing */

 if (rc=Setup_OB (OBU0_MCR, (OB_Normal +OB_SSM +  OB_XBLT + OB_YBLT + OB_No_Copy),
          iXSize, 0, 0,  iXSize, iYSize, 0)) return(rc);

 /*  Enable sequencer start */
 if (rc=ioRDTWriteDVP(SIU_MCR,(SIU_Start1 + SIU_No_Toggle + (SIU_SI1 * 0)))) return(rc);

 /*  Generate Newfield to start sequencer  */
 if (rc=SeqManStart()) return(rc);

 /*  write to memory port  */

  return(OK);
}

/*****************************************************************************/
/*  c560From2070()                                                           */
/*                                                                           */
/*  FUNCTION:                                                                */
/*    This test will setup the Pixel 2070 to transfer data to Cube 550.      */
/*                                                                           */
/*  INPUT:    None                                                           */
/*                                                                           */
/*  OUTPUT:   return value     - OK if successful, ERROR if NOT.             */
/*****************************************************************************/
int c560From2070()
{
 int iTestCnt,iXSize, iYSize,iLoopCount, FifoFull, FifoEmpty, rc;
 unsigned int bExpected;
 unsigned int wReadVal,wWriteVal,wRowCount,wColCount,wExpected;
 unsigned int dwReadVal,dwExpected;
 long lTimeCount;

iXSize = 1024;
iYSize = 512;

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
 if (rc=ioRDTWriteDVP(SIU_SIM0,(OB0_to_OP + offset_0 /*+ SIU_EXIT*/))) return(rc);

 /*  Setup Object (object */
 /*     normal addressing */

 if (rc=Setup_OB (OBU0_MCR, (OB_Normal +OB_SSM +  OB_XBLT + OB_YBLT + OB_No_Copy),
          iXSize, 0, 0,  iXSize, iYSize, 0)) return(rc);

 /*  Enable sequencer start */
 if (rc=ioRDTWriteDVP(SIU_MCR,(SIU_Start1 + SIU_No_Toggle + (SIU_SI1 * 0)))) return(rc);

 /*  Generate Newfield to start sequencer  */
 if (rc=SeqManStart()) return(rc);

 /*  write to memory port  */

  return(OK);
}
