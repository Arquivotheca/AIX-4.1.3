static char sccsid[] = "@(#)32	1.3  src/bos/diag/tu/sky/p110.c, tu_sky, bos411, 9428A410j 2/15/94 20:42:08";
/*
 *   COMPONENT_NAME : (tu_sky) Color Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: copdone
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "skytu.h"
extern byte X_PLUS_2;
copdone()
{ volatile byte stat,pic,mam;
  volatile lword x,y=0;
  byte not_done = YES;

  mam=stat=pic=0xFFFFFFFF;

  while (TRUE)
  {
    if (X_PLUS_2)
    {
      /*****************new polling routine*********************/
      if ((mam=readbyte((byte *)(base+0x09)) & 0x80) != 0x80) break; 
      /*****************new polling routine*********************/
    }
    else
    {
      /*****************old polling routine*********************/
      if (
         (((stat=readbyte(INT_STAT_REG)) & 0x80) != 0x80) 
         || 
         (((pic =readbyte( PI_CNTL_REG)) & 0x80) == 0x80)
         ) continue;
      else break;
      /*****************old polling routine*********************/
    }

    for (x=0; x <= 1000; x++);
    if (y++ > 0x0F00)
    { sprintf(tinfo.info.msg_buff,"CoProcessor Will Not Finish its Operation") ;
      CatMsgs("\n--Interrupt Status    = 0x"); 
      CatMsgx(stat);
      CatMsgs("\n--PI Control Register = 0x"); 
      CatMsgx(pic);
      CatMsgs("\n--Memory Access Mode = 0x"); 
      CatMsgx(mam);
      writebyte(0xFF, INT_STAT_REG);
      CatErr(COPDONE | CANT_X_SKY);
      return(COPDONE | CANT_X_SKY);
    }
  }
  writebyte(0xFF, INT_STAT_REG);

  return(COPDONE | GOOD_OP);
}
