static char sccsid[] = "@(#)33  1.2  src/bos/diag/tu/sun/pixeldma.c, tu_sunrise, bos411, 9437A411a 5/27/94 14:52:58";
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: pxDMARead
 *              pxDMAWrite
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
Module Name :  pixeldma.c
*****************************************************************************/

#include "sun_tu_type.h"
#include "error.h"
#include "suntu.h"
#include "iodvp.h"
#include "px2070.h"

/*****************************************************************************/
/*  pxDMARead()                                                              */
/*                                                                           */
/*  FUNCTION:                                                                */
/*    Setup the 2070 to do DMA read from the frame buffer.                   */
/*                                                                           */
/*  INPUT:    iXsize    -  Row size of the image data                        */
/*            iYsize    -  Col size of the image data                        */
/*            fb_startaddr - Starting address of data in frame buffer        */
/*                                                                           */
/*  OUTPUT:   return value     - OK if sucessful, ERROR if NOT.              */
/*****************************************************************************/
int pxDMARead(int iXSize, int iYSize, unsigned fb_startaddr)
{
int rc;
int iXRef,iPort,iVidProc;
int iWinXStart,iWinYStart,iWinWidth,iWinHeight;
int itestval;
unsigned int fb_startaddr_low, fb_startaddr_high;

   iXRef  = iXSize;
   fb_startaddr_low = fb_startaddr & 0xffff;
   fb_startaddr_high= (fb_startaddr & 0xffff0000) >> 16;

   /* Reset the 2070 */
   if (rc=SoftResetDVP()) return(rc);

   /* Disable Units */
   if (rc = ioRDTWriteDVP(i_VPU_MCR,0)) return (rc);
   if (rc=UpdateGen()) return(rc);

   /* halt sequencer */
   if (rc=SeqHalt()) return(rc);

   /* Clear out the fifos */
   if (rc=ResetFifo(ALL_FIFOS)) return(rc);

   /* Set up the MMU */
   if (rc=SetUpMMU()) return(rc);

   /* Disable all objects */
   if (rc=OBReset(ALL_OBJECTS)) return(rc);

   if (rc=UpdateGen()) return(rc);

   /* setup objects */
   if (rc=Setup_OB (OBU1_MCR, (OB_Normal + OB_XBLT + OB_YBLT + OB_SSM + OB_No_Copy),
       iXRef, fb_startaddr_high, fb_startaddr_low,iXSize, iYSize, 0)) return(rc);

   /* Setup the OPU */
   if (rc = ioRDTWriteDVP(OPU_MCR1, /*Interlaced +*/ OP_BLANK_OUT /*+ ZoomEn*/)) return (rc);
   if (rc = ioRDTWriteDVP(OPU_XBI1,1)) return (rc);
   if (rc = ioRDTWriteDVP(OPU_XEI1,iXSize)) return (rc);
   if (rc = ioRDTWriteDVP(OPU_YBI1,1)) return (rc);
   if (rc = ioRDTWriteDVP(OPU_YEI1,iYSize)) return (rc);
   if (rc = ioRDTWriteDVP(OPU_MCR2,/*Interlaced +*/ OP_BLANK_OUT)) return (rc);
   if (rc = ioRDTWriteDVP(OPU_XBI2,1)) return (rc);
   if (rc = ioRDTWriteDVP(OPU_XEI2,iXSize)) return (rc);
   if (rc = ioRDTWriteDVP(OPU_YBI2,1)) return (rc);
   if (rc = ioRDTWriteDVP(OPU_YEI2,iYSize)) return (rc);

   if (rc=EnableFifo(ALL_FIFOS)) return(rc);

   /* hsw,had,hap,hp, vsw,vad,vap,vp, vsu_disable */
   if (rc=Setup_VSU (1,4,iXSize,1+4+iXSize+10,1,4,iYSize,iYSize+1+4, VSU_Enable))
        return(rc);
   /*****************************************
   * Now that we have an interlaced object,
   * configure things for output.
   ******************************************/

   /* Setup Video Port 2 */
   if (rc = ioRDTWriteDVP(i_VIU_MCR2,/*VIU_ovsp + VIU_ohsp +*/ VIU_OutOnly + VIU_oblt
                  + VIU_oss_vsu_op + VIU_sme)) return (rc);

   /* setup datapath controls */
   if (rc = ioRDTWriteDVP(i_VIU_DPC1,  OPU_to_Vp2 + VSU_to_Vp2 /*+ IPU1_from_Vp1_Phase*/)) return (rc);
   if (rc = ioRDTWriteDVP(i_VIU_DPC2,  OPU_to_Vp2 + VSU_to_Vp2 /*+ IPU1_from_Vp1_Phase*/)) return (rc);

   if (rc=EnableFifo(ALL_FIFOS))return(rc);

   /* Sequencer start */
   if (rc = ioRDTWriteDVP(SIU_SIM0,(OB1_to_OP  + offset_1 /*+ SIU_EXIT*/))) return (rc);
   if (rc = ioRDTWriteDVP(SIU_SIM1,(OB1_to_OP  + offset_M1 /*+ SIU_EXIT*/))) return (rc);
   if (rc = ioRDTWriteDVP(i_SIU_MCR,(SIU_Start1 + SIU_No_Toggle)
               + (SIU_SI1 * 0 )/*+ (SIU_SI2 * 0)*/)) return (rc);

   if (rc = ioRDTWriteDVP(i_VIU_WDT,VIU_MField_V2o)) return (rc);
   if (rc = ioOCSWriteDVP(0x380)) return (rc);

   /* Enable IP1, OP */
   if (rc = ioRDTWriteDVP(i_VPU_MCR, /*IP1_F1_Only +*/ OP_Start_Next)) return (rc);
   if (rc = ioOCSWriteDVP(0x380)) return (rc);

   return(OK);

} /* End pxDMARead() */

/*****************************************************************************/
/*  pxDMAWrite()                                                             */
/*                                                                           */
/*  FUNCTION:                                                                */
/*    Setup the 2070 to do DMA write to the frame buffer.                    */
/*                                                                           */
/*  INPUT:    iXsize    -  Row size of the image data                        */
/*            iYsize    -  Col size of the image data                        */
/*            fb_startaddr - Starting address of data in frame buffer        */
/*                                                                           */
/*  OUTPUT:   return value     - OK if sucessful, ERROR if NOT.              */
/*****************************************************************************/
int pxDMAWrite(int iXSize, int iYSize, unsigned fb_startaddr)
{
   int rc, fb_startaddr_high, fb_startaddr_low;
   int iXRef,iPort,iVidProc;
   int iWinXStart,iWinYStart,iWinWidth,iWinHeight;
   int itestval;

   iXRef  = iXSize;
   fb_startaddr_low = fb_startaddr & 0xffff;
   fb_startaddr_high= (fb_startaddr & 0xffff0000) >> 16;

   /* Disable Units */
   if (rc = ioRDTWriteDVP(i_VPU_MCR,0)) return (rc);
   if (rc=UpdateGen())return(rc);

   /* halt sequencer */
   if (rc=SeqHalt())return(rc);

   /* Clear out the fifos */
   if (rc=ResetFifo(ALL_FIFOS))return(rc);

   /* Set up the MMU */
   if (rc=SetUpMMU())return(rc);

   /* Disable all objects */
   if (rc=OBReset(ALL_OBJECTS))return(rc);

   if (rc=UpdateGen())return(rc);

   /* setup objects */
   if (rc=Setup_OB (OBU1_MCR, (OB_Normal + OB_XBLT + OB_YBLT + OB_SSM + OB_No_Copy),
             iXRef, fb_startaddr_high, fb_startaddr_low, iXSize, iYSize, 0))
          return(rc);

   if (rc=Setup_Ip2(i_IPU2_F1_BASE,0,1,iXSize,1,iYSize))return(rc);
   if (rc=Setup_Ip2(i_IPU2_F2_BASE,0,1,iXSize,1,iYSize))return(rc);

   if (rc=EnableFifo(ALL_FIFOS))return(rc);

   /* hsw,had,hap,hp, vsw,vad,vap,vp, vsu_disable */
   if (rc=Setup_VSU (1,4,iXSize+1,1+4+iXSize+10,1,1,iYSize,iYSize+1+1, VSU_Enable))
      return(rc);

   /*****************************************
   * Now that we have an interlaced object,
   * configure things for output.
   ******************************************/

   /* Setup Video Port 2 */
   if (rc = ioRDTWriteDVP(i_VIU_MCR2,/*VIU_ovsp + VIU_ohsp +*/ VIU_InOnly + VIU_oblt
                  + VIU_oss_vsu /*_op*/ + VIU_iblt + VIU_imss + VIU_sme)) return (rc);

   /* setup datapath controls */
   if (rc = ioRDTWriteDVP(i_VIU_DPC1,  IPU2_from_Vp2 + VSU_to_Vp2 /*+ IPU1_from_Vp1_Phase*/)) return (rc);
   if (rc = ioRDTWriteDVP(i_VIU_DPC2,  IPU2_from_Vp2 + VSU_to_Vp2 /*+ IPU1_from_Vp1_Phase*/)) return (rc);

   if (rc=EnableFifo(ALL_FIFOS))return(rc);

   /* Enable IP2 */
   if (rc = ioRDTWriteDVP(i_VPU_MCR, /*IP1_F1_Only +*/ IP2_Start_Next)) return (rc);

   /* Sequencer start */
   if (rc = ioRDTWriteDVP(SIU_SIM0,(IPU2_to_OB1  + offset_0 + SIU_EXIT))) return (rc);
   if (rc = ioRDTWriteDVP(i_SIU_MCR,(SIU_Start1 + SIU_No_Toggle)
               + (SIU_SI1 * 0 )/*+ (SIU_SI2 * 0)*/)) return (rc);

   if (rc = ioOCSWriteDVP(0x300)) return (rc);

   if (rc = ioRDTWriteDVP(i_VIU_WDT,VIU_MField_V2i)) return (rc);
   UpdateGen();

   return(OK);

} /* End pxDMAWrite() */
