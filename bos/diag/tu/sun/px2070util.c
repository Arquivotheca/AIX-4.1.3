static char sccsid[] = "@(#)34	1.1  src/bos/diag/tu/sun/px2070util.c, tu_sunrise, bos411, 9437A411a 3/28/94 17:46:16";
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: EnableFifo
 *		GetHostPorts
 *		IsFifoBlastReady
 *		IsFifoEmpty
 *		IsFifoFull
 *		OBReset
 *		ResetFifo
 *		SeqHalt
 *		SeqManStart
 *		SeqWait
 *		SetUpMMU
 *		Set_ALU_Constants
 *		Set_ALU_Logic_Function
 *		SetupDMA
 *		Setup_DW
 *		Setup_Ip1
 *		Setup_Ip2
 *		Setup_OB
 *		Setup_OP
 *		Setup_VSU
 *		SoftResetDVP
 *		UpdateGen
 *		V1FieldIn
 *		V1FieldOut
 *		ioLutReadDVP
 *		ioLutWriteDVP
 *		ioMDTReadDVP
 *		ioMDTShortReadDVP
 *		ioMDTShortWriteDVP
 *		ioMDTWriteDVP
 *		ioOCSReadDVP
 *		ioOCSWriteDVP
 *		ioRDTBlastReadDVP1141
 *		ioRDTBlastWriteDVP1083
 *		ioRDTReadDVP
 *		ioRDTWriteDVP
 *		ioRINDataReadDVP
 *		ioRINDataWriteDVP
 *		ioRINReadDVP
 *		ioRINWriteDVP
 *		pxinitialize
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
Module Name :  px2070util.c
*****************************************************************************/
/* #includes --------------------------------------------*/
#include <stdio.h>

#include "px2070.h"
#include "iodvp.h"
#include "suntu.h"
#include "error.h"

int  ioMDTShortWriteDVP(unsigned int DATA)
{
     return (pio_write(HIU_4,(DATA),1));
}

int  ioMDTShortReadDVP(unsigned int *DATA)
{
     int rc;
     if (rc = pio_read(HIU_4, DATA, 1))
       return (rc);
     *DATA = *DATA & 0xFFFF;
     return (OK);
}

int  ioMDTReadDVP(unsigned int *DATA)
{
     unsigned int val1, val2;
     int rc;
     if (rc = pio_read(HIU_4, &val1, 1))
       return (rc);
     if (rc = pio_read(HIU_4, &val2, 1))
       return (rc);
     *DATA = ((val1 <<16) & 0xffff0000) | (val2 & 0xffff);
     return (OK);
}

int  ioMDTWriteDVP(unsigned int DATA)
{
     int rc;
     if (rc = pio_write(HIU_4,((DATA) & 0xffff0000) >> 16,1))
       return (rc);
     if (rc = pio_write(HIU_4,((DATA) & 0xffff),1))
       return (rc);
     return (OK);
}

int  ioRDTWriteDVP(unsigned int ADDR, unsigned int DATA)
{
     int rc;
     if (rc = pio_write(HIU_2,ADDR,1))
       return (rc);
     if (rc = pio_write(HIU_3,DATA,1))
       return (rc);
     return (OK);

}

int  ioOCSWriteDVP(unsigned int DATA)
{
     return (pio_write(HIU_1,DATA,1));
}

int  ioOCSReadDVP(unsigned int *data)
{
     return (pio_read(HIU_1,data,1));
}

int  ioRINWriteDVP(unsigned int ADDR)
{
     return (pio_write(HIU_2,ADDR,1));
}

int  ioRINReadDVP(unsigned int *DATA)
{
     return (pio_read(HIU_2,DATA,1));
}

int  ioRINDataWriteDVP(unsigned int DATA)
{
     return (pio_write(HIU_3,DATA,1));
}

int  ioRINDataReadDVP(unsigned int *DATA)
{
     return (pio_read(HIU_3,DATA,1));
}

int  ioRDTReadDVP(unsigned int ADDR, unsigned int *DATA)
{
     int rc;
     if (rc = pio_write(HIU_2,(unsigned int) ADDR,1))
           return (rc);
     if (rc = pio_read(HIU_3,(unsigned int *)DATA,1))
           return (rc);
     return (OK);
}


int pxinitialize()
{
  int rc;
  if (rc = SoftResetDVP()) return (rc);
  if (rc = SeqHalt()) return (rc);
  if (rc = ResetFifo(ALL_FIFOS)) return (rc);
  if (rc = SetUpMMU()) return (rc);
  if (rc = OBReset(ALL_OBJECTS)) return (rc);
  if (rc = ioRDTWriteDVP(DWU_MCR,0)) return (rc);
  if (rc = UpdateGen()) return (rc);
}

/**********************************************************
*
*       File Name:  T2070UTL.c
*
*       Module Abstract: This module assigns fifo F and fifo D
*               to the host.
*
*
*       Calling Sequence:
*       GetHostPorts()
*
*       Input Parameters:
*       none
*       Output Parameters:
*       none
*
*       Global Variables:
*       none
***********************************************************
*       Author: Kyle Pratt
*       Date:   11/23/92
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/
int GetHostPorts()
{
  int rc;
  /* reset datapath controls to flush any bogus state  */
  if (rc = ioRDTWriteDVP(VIU_DPC1, 0)) return (rc);
  if (rc = ioRDTWriteDVP(VIU_DPC2, 0)) return (rc);
  if (rc=UpdateGen())  return(rc);

  /* setup datapath controls for the Host  */
  if (rc = ioRDTWriteDVP(VIU_DPC1, OPU_to_Host+IPU2_from_Host)) return (rc);
  if (rc = ioRDTWriteDVP(VIU_DPC2, OPU_to_Host+IPU2_from_Host)) return (rc);
  if (rc=UpdateGen())  return(rc);
  return(OK);
} /* End GetHost Ports  */

int SeqWait(iStartField)
{
  int rc;
  int iLastFieldId,iNewFieldId,iEnableStart;
  unsigned int val;

  iEnableStart  = 0;
  if (rc = ioRDTReadDVP(i_VIU_TEST, &val)) return (rc);
  iLastFieldId =   val & 0x4000;

  while (iEnableStart == 0)
  {
    iNewFieldId =   val & 0x4000;
    if (iLastFieldId != iNewFieldId)
    {
      if (iStartField == SIU_Start1)    /* Wait for trailing edge of mfldid  */
      {
        if (iNewFieldId == 0x4000)
          iLastFieldId = iNewFieldId;
        else
          iEnableStart =1;
      } /* if SIU_Start1  */

      else                            /* Wait for leading edge of mfldid  */
      {
        if (iNewFieldId == 0)
          iLastFieldId = iNewFieldId;
        else
          iEnableStart =1;
      } /* if !SIU_Start1  */
    } /* if iLastField != iNewField  */
  } /* While  */
  return(OK);

} /* End SeqWait  */

/**********************************************************
*
*       File Name:  T2070UTL.c
*
*       Module Abstract: This module sets the MMU for the standard
*       eval board configuration.
*
*       Calling Sequence:
*       SetUpMMU();
*
*       Input Parameters:
*       none
*
*       Output Parameters:
*       none
*
*       Global Variables:
*
***********************************************************
*       Author: Kyle Pratt
*       Date:   11/24/92
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/
int SetUpMMU()
{
  int rc;
  if (rc = ioRDTWriteDVP(MMU_MCR,(FBWidth32 + FBType256K))) return (rc);
  return(OK);
} /* End SetUpMMU  */

/**********************************************************
*
*       File Name:  T2070UTL.c
*
*       Module Abstract: This module halts the sequencer.
*
*       Calling Sequence:
*       SeqHalt();
*
*       Input Parameters:
*       none
*
*       Output Parameters:
*       none
*
*       Global Variables:
*
***********************************************************
*       Author: Kyle Pratt
*       Date:   11/23/92
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/
int SeqHalt()
{
  int rc;
/* halt sequencer  */
  if (rc = ioRDTWriteDVP(i_SIU_MCR,SIU_Halt)) return (rc);
  if (rc=UpdateGen())  return(rc);
  return(OK);
} /* End SeqHalt  */

/**********************************************************
*
*       File Name:  T2070UTL.c
*
*       Module Abstract: This module manually starts the sequencer.
*
*       Calling Sequence:
*       SeqManStart();
*
*       Input Parameters:
*       none
*
*       Output Parameters:
*       none
*
*       Global Variables:
*
***********************************************************
*       Author: Kyle Pratt
*       Date:   11/23/92
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/
int SeqManStart()
{
  int rc;
/* Generate Newfield to start sequencer --  */
  if (rc = ioRDTWriteDVP(i_VIU_WDT,VIU_MField_man)) return (rc);
  if (rc=UpdateGen())  return(rc);
  if (rc = ioRDTWriteDVP(i_VIU_WDT,VIU_MField_man+VIU_Man_Start)) return (rc);
  if (rc=UpdateGen())  return(rc);
  return(OK);
} /* End SeqManStart  */

/**********************************************************
*
*       File Name:  T2070UTL.c
*
*       Module Abstract: This module disables the specified object buffers.
*
*       Calling Sequence:
*       iError = OBReset(E_WhichObject)
*
*       Input Parameters:
*       E_WhichObject           object to be reset
*
*       Output Parameters:
*       iError          return code 0=success -1=fail
*
*       Global Variables:
*
***********************************************************
*       Author: Kyle Pratt
*       Date:   11/23/92
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/
int OBReset(WhichObject E_WhichObject)

{
int rc;
int iReturn=0;
switch (E_WhichObject)
  {
  case ALL_OBJECTS:
    /*Disable all objex  */
    if (rc = ioRDTWriteDVP(OBU0_MCR,0)) return (rc);
    if (rc = ioRDTWriteDVP(OBU1_MCR,0)) return (rc);
    if (rc = ioRDTWriteDVP(OBU2_MCR,0)) return (rc);
    if (rc = ioRDTWriteDVP(OBU3_MCR,0)) return (rc);
    if (rc = ioRDTWriteDVP(OBU4_MCR,0)) return (rc);
    if (rc = ioRDTWriteDVP(OBU5_MCR,0)) return (rc);
    if (rc = ioRDTWriteDVP(OBU6_MCR,0)) return (rc);
    if (rc = ioRDTWriteDVP(OBU7_MCR,0)) return (rc);
    if (rc=UpdateGen())  return(rc);
    break;
  case OBJECT0:
    if (rc = ioRDTWriteDVP(OBU0_MCR,0)) return (rc);
    if (rc=UpdateGen())  return(rc);
    break;
  case OBJECT1:
    if (rc = ioRDTWriteDVP(OBU1_MCR,0)) return (rc);
    if (rc=UpdateGen())  return(rc);
    break;
  case OBJECT2:
    if (rc = ioRDTWriteDVP(OBU2_MCR,0)) return (rc);
    if (rc=UpdateGen())  return(rc);
    break;
  case OBJECT3:
    if (rc = ioRDTWriteDVP(OBU3_MCR,0)) return (rc);
    if (rc=UpdateGen())  return(rc);
    break;
  case OBJECT4:
    if (rc = ioRDTWriteDVP(OBU4_MCR,0)) return (rc);
    if (rc=UpdateGen())  return(rc);
    break;
  case OBJECT5:
    if (rc = ioRDTWriteDVP(OBU5_MCR,0)) return (rc);
    if (rc=UpdateGen())  return(rc);
    break;
  case OBJECT6:
    if (rc = ioRDTWriteDVP(OBU6_MCR,0)) return (rc);
    if (rc=UpdateGen())  return(rc);
    break;
  case OBJECT7:
    if (rc = ioRDTWriteDVP(OBU7_MCR,0)) return (rc);
    if (rc=UpdateGen())  return(rc);
    break;
  default:
    iReturn = -1;
  }
return(iReturn);
} /* End OBReset  */

/**********************************************************
*
*       File Name:  T2070UTL.c
*
*       Module Abstract: This module resets the specified fifo(s).
*
*       Calling Sequence:
*       ResetFifo(wData)
*
*       Input Parameters:
*       wData           fifo(s) to be reset
*
*       Output Parameters:
*       none
*
*       Global Variables:
*
***********************************************************
*       Author: Kyle Pratt
*       Date:   11/23/92
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/
int ResetFifo(unsigned int wData)
{
  int rc;
  if (rc = ioRDTWriteDVP(SIU_FCS,wData)) return (rc);
  return(OK);
} /* End ResetFifo  */

/**********************************************************
*
*       File Name:  T2070UTL.c
*
*       Module Abstract: This module enables the specified fifo(s).
*
*       Calling Sequence:
*       EnableFifo(wData)
*
*       Input Parameters:
*       wData           fifo(s) to be enabled
*
*       Output Parameters:
*       none
*
*       Global Variables:
*
***********************************************************
*       Author: Kyle Pratt
*       Date:   11/23/92
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/
int EnableFifo(unsigned int wData)
{
  int rc;
  if (rc = ioRDTWriteDVP(SIU_FCS,~wData)) return (rc);
  return(OK);
} /* End EnableFifo  */

/**********************************************************
*
*       File Name:  T2070UTL.c
*
*       Module Abstract: This module checks the full flag of
*                the specified fifo.
*
*       Calling Sequence:
*       iRetVal = IsFifoFull(E_WhichFifo)
*
*       Input Parameters:
*       E_WhichFifo             fifo to be checked
*
*       Output Parameters:
*       iRetVal         return code 0=not full
*                                   1=full
*                                  -1=fail
*
*       Global Variables:
*
***********************************************************
*       Author: Kyle Pratt
*       Date:   11/23/92
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/
int IsFifoFull(WhichFifo E_WhichFifo, int *FifoStatus)

{
int rc;
unsigned int val;

*FifoStatus=0;

switch (E_WhichFifo)
  {
  case FIFO_A:
    if (rc = ioRDTReadDVP(SIU_FCS, &val)) return (rc);
    *FifoStatus = val & ALUA_FIFO;
    break;
  case FIFO_B:
    if (rc = ioRDTReadDVP(SIU_FCS, &val)) return (rc);
    *FifoStatus = val & ALUB_FIFO;
    break;
  case FIFO_C:
       if (rc = ioRDTReadDVP(SIU_FCS, &val)) return (rc);
    *FifoStatus = val & ALUC_FIFO;
    break;
  case FIFO_D:
       if (rc = ioRDTReadDVP(SIU_FCS, &val)) return (rc);
    *FifoStatus = val & OPU_FIFO;
    break;
  case FIFO_E:
       if (rc = ioRDTReadDVP(SIU_FCS, &val)) return (rc);
    *FifoStatus = val & ALUE_FIFO;
    break;
  case FIFO_F:
       if (rc = ioRDTReadDVP(SIU_FCS, &val)) return (rc);
    *FifoStatus = val & IPU2_FIFO;
    break;
  case FIFO_G:
       if (rc = ioRDTReadDVP(SIU_FCS, &val)) return (rc);
    *FifoStatus = val & IPU1_FIFO;
    break;
  default:
    return(-1);
  }
return(OK);
} /* End IsFifoFull  */

/**********************************************************
*
*       File Name:  T2070UTL.c
*
*       Module Abstract: This module checks the empty flag of
*                the specified fifo.
*
*       Calling Sequence:
*       iRetVal = IsFifoEmpty(E_WhichFifo)
*
*       Input Parameters:
*       E_WhichFifo             fifo to be checked
*
*       Output Parameters:
*       iRetVal         return code 0=not empty
*                                   1=empty
*                                  -1=fail
*
*       Global Variables:
*
***********************************************************
*       Author: Kyle Pratt
*       Date:   11/23/92
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/
int IsFifoEmpty(WhichFifo E_WhichFifo, int *FifoStatus)

{
int rc;
unsigned int val;

*FifoStatus=0;
switch (E_WhichFifo)
  {
  case FIFO_A:
       if (rc = ioRDTReadDVP(SIU_FCS, &val)) return (rc);
    *FifoStatus = val & ALUA_FIFO>>1;
    break;
  case FIFO_B:
       if (rc = ioRDTReadDVP(SIU_FCS, &val)) return (rc);
    *FifoStatus = val & ALUB_FIFO>>1;
    break;
  case FIFO_C:
       if (rc = ioRDTReadDVP(SIU_FCS, &val)) return (rc);
    *FifoStatus = val & ALUC_FIFO>>1;
    break;
  case FIFO_D:
       if (rc = ioRDTReadDVP(SIU_FCS, &val)) return (rc);
    *FifoStatus = val & OPU_FIFO>>1;
    break;
  case FIFO_E:
       if (rc = ioRDTReadDVP(SIU_FCS, &val)) return (rc);
    *FifoStatus = val & ALUE_FIFO>>1;
    break;
  case FIFO_F:
       if (rc = ioRDTReadDVP(SIU_FCS, &val)) return (rc);
    *FifoStatus = val & IPU2_FIFO>>1;
    break;
  case FIFO_G:
       if (rc = ioRDTReadDVP(SIU_FCS, &val)) return (rc);
    *FifoStatus = val & IPU1_FIFO>>1;
    break;
  default:
    return(-1);
  }
return(OK);
} /* End IsFifoEmpty  */

/**********************************************************
*
*       File Name:  T2070UTL.c
*
*       Module Abstract: This module checks the almost empty flag of
*                fifo D or the almost full flag of fifo F.
*
*       Calling Sequence:
*       iRetVal = IsFifoBlastReady(E_WhichFifo)
*
*       Input Parameters:
*       E_WhichFifo             fifo to be checked
*
*       Output Parameters:
*       iRetVal         return code 0=not ready for blast
*                                   1=ready for blast
*                                  -1=fail
*
*       Global Variables:
*
***********************************************************
*       Author: Kyle Pratt
*       Date:   11/23/92
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/
int IsFifoBlastReady(WhichFifo E_WhichFifo)

{
int rc, iReturnVal=0;
unsigned int val;
switch (E_WhichFifo)
  {
  case FIFO_D:
       if (rc = ioOCSReadDVP(&val)) return (rc);
    iReturnVal = val & FIFO_ALMOST_EMPTY;
    break;
  case FIFO_F:
       if (rc = ioOCSReadDVP(&val)) return (rc);
    iReturnVal = val & FIFO_ALMOST_FULL;
    break;
  default:
    iReturnVal = -1;
  }
return(iReturnVal);
} /* End IsFifoBlastReady  */

/**********************************************************
*
*       File Name:  T2070UTL.c
*
*       Module Abstract: This module performs a soft reset of the 2070.
*
*       Calling Sequence:
*       SoftResetDVP();
*
*       Input Parameters:
*       none
*
*       Output Parameters:
*       none
*
*       Global Variables:
*
***********************************************************
*       Author: Kyle Pratt
*       Date:   11/23/92
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/
int SoftResetDVP()

{
  int rc;
  if (rc = ioOCSWriteDVP(SW_RESET)) return (rc);
  if (rc = ioOCSWriteDVP(0)) return (rc);
  return(OK);
} /* End SoftResetDVP  */

/* ----------------------------------------------------  */
int Setup_OB(unsigned int offset, unsigned int mc, unsigned int refx,
              unsigned int lah, unsigned int lal, unsigned int xsz,
              unsigned int ysz, unsigned int decm)
{
  int rc;
  if (rc = ioRDTWriteDVP(offset+0,mc)) return (rc);
  if (rc = ioRDTWriteDVP(offset+1,refx)) return (rc);
  if (rc = ioRDTWriteDVP(offset+2,lal)) return (rc);
  if (rc = ioRDTWriteDVP(offset+3,lah)) return (rc);
  if (rc = ioRDTWriteDVP(offset+4,xsz)) return (rc);
  if (rc = ioRDTWriteDVP(offset+5,ysz)) return (rc);
  if (rc = ioRDTWriteDVP(offset+6,decm)) return (rc);
  return(OK);
} /* Setup_OB;  */

    /*- ----------------------------------------------------  */
int Setup_DW (unsigned int offset, unsigned int refx, unsigned int lah,
               unsigned int lal, unsigned int xsz, unsigned int xst,
               unsigned int xzm, unsigned int ysz, unsigned int yst,
               unsigned int yzm)
{
  int rc;
  if (rc = ioRDTWriteDVP(offset+i_DWU_RFX,refx)) return (rc);
  if (rc = ioRDTWriteDVP(offset+i_DWU_LSH,lah)) return (rc);
  if (rc = ioRDTWriteDVP(offset+i_DWU_LSL,lal)) return (rc);
  if (rc = ioRDTWriteDVP(offset+i_DWU_WSX,xsz)) return (rc);
  if (rc = ioRDTWriteDVP(offset+i_DWU_DSX,xst)) return (rc);
  if (rc = ioRDTWriteDVP(offset+i_DWU_WSY,ysz)) return (rc);
  if (rc = ioRDTWriteDVP(offset+i_DWU_DSY,yst)) return (rc);
  if (rc = ioRDTWriteDVP(offset+i_DWU_DWZ,((yzm*256)+xzm))) return (rc);
  return(OK);
} /* Setup_DW;  */

     /*-- ----------------------------------------------------  */

int Setup_VSU (unsigned int hsw, unsigned int had, unsigned int hap,
                unsigned int hp, unsigned int vsw, unsigned int vad,
                unsigned int vap, unsigned int vp, unsigned int vsumode)
{
  int rc;
  if (rc = ioRDTWriteDVP(i_VSU_HSW, hsw)) return (rc);
  if (rc = ioRDTWriteDVP(i_VSU_HAD, had)) return (rc);
  if (rc = ioRDTWriteDVP(i_VSU_HAP, hap)) return (rc);
  if (rc = ioRDTWriteDVP(i_VSU_HP,  hp)) return (rc);
  if (rc = ioRDTWriteDVP(i_VSU_VSW, vsw)) return (rc);
  if (rc = ioRDTWriteDVP(i_VSU_VAD, vad)) return (rc);
  if (rc = ioRDTWriteDVP(i_VSU_VAP, vap)) return (rc);
  if (rc = ioRDTWriteDVP(i_VSU_VP,  vp+vsumode)) return (rc);
  return(OK);
} /* Setup_VSU;  */

     /*-- ----------------------------------------------------  */
int Set_ALU_Constants (unsigned int cay, unsigned int cau,
                        unsigned int cav, unsigned int cby,
                        unsigned int cbu, unsigned int cbv,
                        unsigned int ccy, unsigned int ccu,
                        unsigned int ccv)
{
  int rc;
  if (rc = ioRDTWriteDVP(i_ALU_CAY,cay)) return (rc);
  if (rc = ioRDTWriteDVP(i_ALU_CAU,cau)) return (rc);
  if (rc = ioRDTWriteDVP(i_ALU_CAV,cav)) return (rc);
  if (rc = ioRDTWriteDVP(i_ALU_CBY,cby)) return (rc);
  if (rc = ioRDTWriteDVP(i_ALU_CBU,cbu)) return (rc);
  if (rc = ioRDTWriteDVP(i_ALU_CBV,cbv)) return (rc);
  if (rc = ioRDTWriteDVP(i_ALU_CCY,ccy)) return (rc);
  if (rc = ioRDTWriteDVP(i_ALU_CCU,ccu)) return (rc);
  if (rc = ioRDTWriteDVP(i_ALU_CCV,ccv)) return (rc);
  return(OK);
} /* Set_ALU_constants;  */

     /*-- ----------------------------------------------------  */

 int Set_ALU_Logic_Function(unsigned int lopy, unsigned int lopu,
                             unsigned int lopv)
{
  int rc;
  if (rc = ioRDTWriteDVP(i_ALU_LOPY,lopy)) return (rc);
  if (rc = ioRDTWriteDVP(i_ALU_LOPU,lopu)) return (rc);
  if (rc = ioRDTWriteDVP(i_ALU_LOPV,lopv)) return (rc);
  return(OK);
} /* Set_ALU_Logic_Function;  */

    /* -- ----------------------------------------------------  */

int Setup_Ip1 (unsigned int offset, unsigned int mcrval,
                unsigned int xbi, unsigned int xbf, unsigned int xei,
                unsigned int xsi, unsigned int xsf, unsigned int ybi,
                unsigned int ybf, unsigned int yei, unsigned int ysi,
                unsigned int ysf)
{
  int rc;
  if (rc = ioRDTWriteDVP(offset+i_MCR_offset,mcrval)) return (rc);
  if (rc = ioRDTWriteDVP(offset+i_XBI_offset,xbi)) return (rc);
  if (rc = ioRDTWriteDVP(offset+i_XBF_offset,xbf)) return (rc);
  if (rc = ioRDTWriteDVP(offset+i_XEI_offset,xei)) return (rc);
  if (rc = ioRDTWriteDVP(offset+i_XSI_offset,xsi)) return (rc);
  if (rc = ioRDTWriteDVP(offset+i_XSF_offset,xsf)) return (rc);

  if (rc = ioRDTWriteDVP(offset+i_YBI_offset,ybi)) return (rc);
  if (rc = ioRDTWriteDVP(offset+i_YBF_offset,ybf)) return (rc);
  if (rc = ioRDTWriteDVP(offset+i_YEI_offset,yei)) return (rc);
  if (rc = ioRDTWriteDVP(offset+i_YSI_offset,ysi)) return (rc);
  if (rc = ioRDTWriteDVP(offset+i_YSF_offset,ysf)) return (rc);
  return(OK);
} /* Setup_Ip1;  */

     /*-- ----------------------------------------------------  */

int Setup_OP (unsigned int offset, unsigned int mcrval,
               unsigned int xbi, unsigned int xei, unsigned int ybi,
               unsigned int yei)
{
  int rc;
  if (rc = ioRDTWriteDVP(offset+i_MCR_offset,mcrval)) return (rc);
  if (rc = ioRDTWriteDVP(offset+i_XBI_offset,xbi)) return (rc);
  if (rc = ioRDTWriteDVP(offset+i_XEI_offset,xei)) return (rc);
  if (rc = ioRDTWriteDVP(offset+i_YBI_offset,ybi)) return (rc);
  if (rc = ioRDTWriteDVP(offset+i_YEI_offset,yei)) return (rc);
  return(OK);
} /* Setup_OP;  */

int Setup_Ip2 (unsigned int offset, unsigned int mcrval,
                unsigned int xbi, unsigned int xei, unsigned int ybi,
                unsigned int yei)
{
  int rc;
  if (rc = ioRDTWriteDVP(offset+i_MCR_offset,mcrval)) return (rc);
  if (rc = ioRDTWriteDVP(offset+i_XBI_offset,xbi)) return (rc);
  if (rc = ioRDTWriteDVP(offset+i_XEI_offset,xei)) return (rc);
  if (rc = ioRDTWriteDVP(offset+i_YBI_offset,ybi)) return (rc);
  if (rc = ioRDTWriteDVP(offset+i_YEI_offset,yei)) return (rc);
  return(OK);
} /* Setup_OP;  */

      /*-- ---------------------------------------------------  */

int UpdateGen()
{
  int rc;
  unsigned int wReadVal;

/*  if (rc = ioOCSReadDVP()) return (rc);*/
  if (rc = ioOCSWriteDVP(0x0380)) return (rc);
  return(OK);
} /* UpdateGen  */


int SetupDMA(int xsize, int ysize, int OBJ, int direction)
{
  int i, rc;

  /* Set up port V2  and datapath controls */
  switch (direction) {
    case DMAOUT:
/*
           if (rc = ioRDTWriteDVP(i_VIU_MCR2,VIU_oss_vsu + VIU_sme + VIU_oblt + VIU_OutOnly)) return (rc);
*/
           if (rc = ioRDTWriteDVP(i_VIU_MCR2,0x2101)) return (rc);
           if (rc = ioRDTWriteDVP(i_VIU_DPC1,VSU_to_Vp2 + OPU_to_Vp2)) return (rc);
           break;

    case DMAIN:
           if (rc = ioRDTWriteDVP(i_VIU_MCR2,VIU_oss_vsu + VIU_imss + VIU_InOnly)) return (rc);
           if (rc = ioRDTWriteDVP(i_VIU_DPC1,VSU_to_Vp2 + IPU2_from_Vp2 + OPU_LoopBack)) return (rc);
           break;
  }

  /* Set up IPU2 or OPU */
  switch (direction) {
    case DMAIN:
           if (rc = Setup_Ip2(i_IPU2_MCR1,0,0,xsize,0,ysize)) return (rc);
           if (rc = Setup_Ip2(i_IPU2_MCR2,0,0,xsize,0,ysize)) return (rc);
           break;
    case DMAOUT:
           if (rc = Setup_OP(i_OPU_MCR1,OP_BLANK_OUT,0,xsize,0,ysize)) return (rc);
           if (rc = Setup_OP(i_OPU_MCR2,OP_BLANK_OUT,0,xsize,0,ysize)) return (rc);
           break;
  }

  /* Set up VSU */
  for(i=1024;((xsize*ysize)%i)!=0; i--);

/*
  Setup_VSU( 0, 5, i,280,1,2,xsize*ysize/i,4,VSU_Enable + 24 + VSU_SingleSweep);
*/
  if (rc = Setup_VSU(16,45,352,1,373,2,6,240,VSU_Enable+248)) return (rc);

/*
  if (rc = ioRDTWriteDVP(i_VSU_HAP,i)) return (rc);
  if (rc = ioRDTWriteDVP(i_VSU_VAP,xsize*ysize) return (rc);
*/

  /* Enable FIFOs */
  if (rc= EnableFifo(ALL_FIFOS)) return (rc);

  /* Set up Sequencer */
  if (direction==DMAOUT) {
    switch (OBJ) {
      case 1:  if (rc = ioRDTWriteDVP(SIU_SIM0,OB1_to_OP + offset_0 + SIU_EXIT)) return (rc);
               break;

      case 2:  if (rc = ioRDTWriteDVP(SIU_SIM0,OB2_to_OP + offset_0 + SIU_EXIT)) return (rc);
               break;

      case 3:  if (rc = ioRDTWriteDVP(SIU_SIM0,OB3_to_OP + offset_0 + SIU_EXIT)) return (rc);
               break;

      case 4:  if (rc = ioRDTWriteDVP(SIU_SIM0,OB4_to_OP + offset_0 + SIU_EXIT)) return (rc);
               break;

      case 5:  if (rc = ioRDTWriteDVP(SIU_SIM0,OB5_to_OP + offset_0 + SIU_EXIT)) return (rc);
               break;

      case 6:  if (rc = ioRDTWriteDVP(SIU_SIM0,OB6_to_OP + offset_0 + SIU_EXIT)) return (rc);
               break;

      case 7:  if (rc = ioRDTWriteDVP(SIU_SIM0,OB7_to_OP + offset_0 + SIU_EXIT)) return (rc);
               break;

      default: if (rc = ioRDTWriteDVP(SIU_SIM0,OB0_to_OP + offset_0 + SIU_EXIT)) return (rc);
               break;
    }
  }
  else {
    switch (OBJ) {
      case 1:  if (rc = ioRDTWriteDVP(SIU_SIM0,IPU2_to_OB1 + offset_0 + SIU_EXIT)) return (rc);
               break;

      case 2:  if (rc = ioRDTWriteDVP(SIU_SIM0,IPU2_to_OB2 + offset_0 + SIU_EXIT)) return (rc);
               break;

      case 3:  if (rc = ioRDTWriteDVP(SIU_SIM0,IPU2_to_OB3 + offset_0 + SIU_EXIT)) return (rc);
               break;

      case 4:  if (rc = ioRDTWriteDVP(SIU_SIM0,IPU2_to_OB4 + offset_0 + SIU_EXIT)) return (rc);
               break;

      case 5:  if (rc = ioRDTWriteDVP(SIU_SIM0,IPU2_to_OB5 + offset_0 + SIU_EXIT)) return (rc);
               break;

      case 6:  if (rc = ioRDTWriteDVP(SIU_SIM0,IPU2_to_OB6 + offset_0 + SIU_EXIT)) return (rc);
               break;

      case 7:  if (rc = ioRDTWriteDVP(SIU_SIM0,IPU2_to_OB7 + offset_0 + SIU_EXIT)) return (rc);
               break;

      default: if (rc = ioRDTWriteDVP(SIU_SIM0,IPU2_to_OB0 + offset_0 + SIU_EXIT)) return (rc);
               break;
    }
  }
  if (rc = ioRDTWriteDVP(i_SIU_MCR,SIU_Start1 + SIU_No_Toggle)) return (rc);
  if (rc= SeqManStart()) return (rc);

  /* Enable IP2  or OPU */
  switch(direction) {
    case DMAIN:  if (rc = ioRDTWriteDVP(i_VPU_MCR,IP2_F1_Only)) return (rc);
                 break;
    case DMAOUT: if (rc = ioRDTWriteDVP(i_VPU_MCR,OP_F1_Only)) return (rc);
                 break;
  }
/*
  if (rc = ioRDTWriteDVP(i_VSU_VP, 5+VSU_Enable + VSU_SingleSweep)) return (rc);
*/
  if (rc=UpdateGen())  return(rc);
  return(OK);
}

int V1FieldIn(int xsize, int ysize, int csc, int gamma,
          int xscale, int yscale, int OBJ, int outform)
{
  int rc;
  /* set up V1 */
  if (rc = ioRDTWriteDVP(i_VIU_MCR1,VIU_oss_vp + VIU_ihsp + VIU_ivsp + VIU_InOnly)) return (rc);

  /* set up datapath controls */
  if (rc = ioRDTWriteDVP(i_VIU_DPC1, IPU1_from_Vp1)) return (rc);

  if (rc= Setup_Ip1(i_IPU1_F1_BASE, csc + gamma + outform,
            0,0,        /* XBI, XBF */
            xsize,      /* XEI */
            xscale,0,   /* XSI, XSF */
            0,0,        /* YBI, YBF */
            ysize,      /* YEI */
            yscale,0))  /* XSI, XSF */
        return (rc);

  /* Enable FIFOs */
  if (rc= EnableFifo(ALL_FIFOS)) return (rc);

  switch (OBJ) {
    case 1:  if (rc = ioRDTWriteDVP(SIU_SIM0,IPU1_to_OB1 + offset_0)) return (rc);
             break;

    case 2:  if (rc = ioRDTWriteDVP(SIU_SIM0,IPU1_to_OB2 + offset_0)) return (rc);
             break;

    case 3:  if (rc = ioRDTWriteDVP(SIU_SIM0,IPU1_to_OB3 + offset_0)) return (rc);
             break;

    case 4:  if (rc = ioRDTWriteDVP(SIU_SIM0,IPU1_to_OB4 + offset_0)) return (rc);
             break;

    case 5:  if (rc = ioRDTWriteDVP(SIU_SIM0,IPU1_to_OB5 + offset_0)) return (rc);
             break;

    case 6:  if (rc = ioRDTWriteDVP(SIU_SIM0,IPU1_to_OB6 + offset_0)) return (rc);
             break;

    case 7:  if (rc = ioRDTWriteDVP(SIU_SIM0,IPU1_to_OB7 + offset_0)) return (rc);
             break;

    default: if (rc = ioRDTWriteDVP(SIU_SIM0,IPU1_to_OB0 + offset_0)) return (rc);
             break;
  }
  if (rc = ioRDTWriteDVP(i_SIU_MCR,SIU_Start1 + SIU_No_Toggle +(SIU_SI1 * 0))) return (rc);
  if (rc= SeqManStart()) return (rc);
  if (rc=UpdateGen())  return(rc);

  /* Enable IP1 */
  if (rc = ioRDTWriteDVP(i_VPU_MCR,IP1_F1_Only)) return (rc);
  if (rc=UpdateGen())  return(rc);
  return(OK);
}

int V1FieldOut(int xsize, int ysize, int OBJ, int zoom, int mode)
{
  int rc;
  /* set up V1 */
  if (rc = ioRDTWriteDVP(i_VIU_MCR1,VIU_oss_vp + VIU_ihsp + VIU_ivsp + VIU_OutOnly)) return (rc);

  /* set up datapath controls */
  if (rc = ioRDTWriteDVP(i_VIU_DPC1, OPU_to_Vp1)) return (rc);

  /* set up OPU */
  if (rc= Setup_OP(i_OPU_F1_BASE, zoom + mode,
           0,xsize,  /* XBI, XEI */
           0,ysize)) /* YBI, YEI */
         return (rc);
  /* Enable FIFOs */
  if (rc= EnableFifo(ALL_FIFOS)) return (rc);

  switch (OBJ) {
    case 1:  if (rc = ioRDTWriteDVP(SIU_SIM0,OB1_to_OP + SIU_EXIT)) return (rc);
             break;

    case 2:  if (rc = ioRDTWriteDVP(SIU_SIM0,OB2_to_OP + SIU_EXIT)) return (rc);
             break;

    case 3:  if (rc = ioRDTWriteDVP(SIU_SIM0,OB3_to_OP + SIU_EXIT)) return (rc);
             break;

    case 4:  if (rc = ioRDTWriteDVP(SIU_SIM0,OB4_to_OP + SIU_EXIT)) return (rc);
             break;

    case 5:  if (rc = ioRDTWriteDVP(SIU_SIM0,OB5_to_OP + SIU_EXIT)) return (rc);
             break;

    case 6:  if (rc = ioRDTWriteDVP(SIU_SIM0,OB6_to_OP + SIU_EXIT)) return (rc);
             break;

    case 7:  if (rc = ioRDTWriteDVP(SIU_SIM0,OB7_to_OP + SIU_EXIT)) return (rc);
             break;

    default: if (rc = ioRDTWriteDVP(SIU_SIM0,OB0_to_OP + SIU_EXIT)) return (rc);
             break;
  }
  if (rc = ioRDTWriteDVP(i_SIU_MCR,SIU_Start1 + SIU_No_Toggle)) return (rc);
  if (rc= SeqManStart()) return (rc);
  if (rc=UpdateGen())  return(rc);

  /* Enable OPU */
  if (rc = ioRDTWriteDVP(i_VPU_MCR,OP_F1_Only)) return (rc);
  if (rc=UpdateGen())  return(rc);
  return(OK);
}

/**********************************************************
*
*       Module Abstract:  This module provides the ability to
*               write the indexed registers of the 2070. This routine
*               accepts the base index, a pointer to a list of data
*               words, and the number of words to be written.
*
*       Calling Sequence:
*         ioRDTBlastWriteDVP(wIndex, pwList, iCount)
*
*       Input Parameters:
*       unsigned int  wIndex          Starting index value.
*       unsigned int  *pwList Pointer to a list of data words.
*       int             iCount          Number of words to be written.
*
*       Output Parameters:
*       none
*
*       Global Variables:
*       none
***********************************************************
*       Author: Kyle Pratt
*       Date:   11/2/92
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/

int  ioRDTBlastWriteDVP(wIndex, pwList, iCount)
unsigned int wIndex;
unsigned int *pwList;
int iCount;

{
  int rc;
/* Output the address with the address autoincrement feature turned on. */
if (rc = ioRINWriteDVP(wIndex | 0x8000)) return (rc);
/* lpOutWordFuncs[G_Px2070PortType](S_pCurrentDVPPorts->PX2070Port2,(wIndex | 0x8000)); */

#ifdef _WINDOWS
#ifdef DEBUG
if (rc= dbgDisplayBlastWrite (wIndex, pwList, iCount)) return (rc);
#endif
#endif

/* Write all the data in the list. */
while (iCount > 0)
{
  if (rc = ioRINDataWriteDVP(*pwList)) return (rc);
/*  lpOutWordFuncs[G_Px2070PortType](S_pCurrentDVPPorts->PX2070Port3, *pwList); */
  pwList++;
  iCount--;
  }
  return(OK);
} /* End ioRDTBlastWriteDVP */

/**********************************************************
*
*       Module Abstract:  This module provides the ability to
*               read a block of the indexed registers of the 2070.
*               This routine accepts the base index, a pointer to
*               a list of data words, and the number of words to be read.
*
*       Calling Sequence:
*        ioRDTBlastReadDVP(wIndex, pwList, iCount)
*
*       Input Parameters:
*       unsigned int  wIndex          Starting index value.
*       unsigned int  *pwList Pointer to a list of data words.
*       int             iCount          Number of words to be written.
*
*       Output Parameters:
*       none
*
*       Global Variables:
*       none
***********************************************************
*       Author: Kyle Pratt
*       Date:   11/2/92
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/

int  ioRDTBlastReadDVP(wIndex, pwList, iCount)
unsigned int wIndex;
unsigned int *pwList;
int iCount;
{
  int rc;
/* Output the address with the address autoincrement feature turned on. */
if (rc = ioRINWriteDVP(wIndex | 0x8000)) return (rc);
/*lpOutWordFuncs[G_Px2070PortType](S_pCurrentDVPPorts->PX2070Port2,(wIndex | 0x8000)); */

/* Read all the data to the list. */
while (iCount > 0)
{
     if (rc = ioRINDataReadDVP(pwList)) return (rc);
/*  *pwList = lpInWordFuncs[G_Px2070PortType](S_pCurrentDVPPorts->PX2070Port3); */
  pwList++;
  iCount--;
  }
  return(OK);
}
/**********************************************************
*
*       Module Abstract:  This module provides the ability to
*               write the lut in IPU1 of the 2070. This routine
*               accepts the base index, a pointer to a list of data
*               words, and the number of words to be written.
*
*       Calling Sequence:
*        ioLutWriteDVP(wIndex, pbList, iCount)
*
*       Input Parameters:
*       unsigned int  wIndex          Starting index value.
*       unsigned char   *pbList Pointer to a list of data words.
*       int             iCount          Number of words to be written.
*
*       Output Parameters:
*       none
*
*       Global Variables:
*       none
***********************************************************
*       Author: Kyle Pratt
*       Date:   11/11/92
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/

int ioLutWriteDVP(wIndex, pbList, iCount)
unsigned int wIndex;
unsigned char *pbList;
int iCount;

{
int rc;
int iError=0;
unsigned int wWriteVal;

/* If input parms are ok */
if (((wIndex * 3 + iCount) <= MAX_IPU1_LUT))
  {

  /* Output the index of the lut. */
  if (rc = ioRDTWriteDVP(IPU1_LRB,wIndex)) return (rc);

  /* Write all the data in the list. */
  while (iCount > 0)
    {
    wWriteVal = (unsigned int) *pbList;
    if (rc = ioRDTWriteDVP(IPU1_LRD,wWriteVal)) return (rc);
    pbList++;
    iCount--;
    }
  } /* if parms ok */
else
  iError = -1;
return(iError);
}
/**********************************************************
*
       File Name:  if (rc = ioDVP) return (rc);
*
*       Module Abstract:  This module provides the ability to
*               read a block of the IPU1 lut in the  2070.
*               This routine accepts the base index, a pointer to
*               a list of data words, and the number of words to be read.
*
*       Calling Sequence:
*        ioLutReadDVP(wIndex, pbList, iCount)
*
*       Input Parameters:
*       unsigned int  wIndex          Starting index value.
*       unsigned char   *pbList Pointer to a list of data words.
*       int             iCount          Number of words to be written.
*
*       Output Parameters:
*       none
*
*       Global Variables:
*       none
***********************************************************
*       Author: Kyle Pratt
*       Date:   11/2/92
*
*       Revision History:
*       WHO             WHEN            WHAT/WHY/HOW
*
*
*********************************************************/

int  ioLutReadDVP(wIndex, pbList, iCount)
unsigned int wIndex;
unsigned char  *pbList;
int iCount;

{
int rc, iError=0;
unsigned int wReadVal;

/* If input parms are ok */
if (((wIndex * 3 + iCount) <= MAX_IPU1_LUT))
  {
  /* Output the index of the lut. */
  if (rc = ioRDTWriteDVP(IPU1_LRB,wIndex)) return (rc);

  /* Read all the data to the list. */
  while (iCount > 0)
    {
  if (rc = ioRDTReadDVP(IPU1_LRD, &wReadVal)) return (rc);
    *pbList = (unsigned char) (wReadVal & IPU1_LRD_MASK);
    pbList++;
    iCount--;
    }
  } /* if parms ok */
else
  iError = -1;
return(iError);
}
