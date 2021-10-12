static char sccsid[] = "@(#)94	1.2  src/bos/diag/tu/sky/c0b2.c, tu_sky, bos411, 9428A410j 10/29/93 13:39:42";
/*
 *   COMPONENT_NAME : (tu_sky) Color Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: c0b2
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
/*************************************************
* Name     : c0b2() 
* Function : Tests the function of Mask Map      *
*            Boundry Enabled                     *
*                                                *
*************************************************/
#define RSTCOL if (col++ > MAXCOL) col = 0;
#define MAXCOL 16

c0b2()
{
  word  ht,wth;
  word  sdx,sdy,dpx,dpy,dir;
  word  rc;
  byte col;
  word xoff, yoff;
  float msz;

  do
  { 
    initcop();
    col = 0;

    /* make the screen  map  */
    ht = ((wth = 1279) * 1023/1279);
    writebyte(BPP8, MEM_ACCESS_MODE);
    rc = make_map(MAPA, skyvbase, ht, wth, BPP8);FUSE;

    /*make incrementally smaller mask rectangles - area inside will be updated*/
    for (msz=1279,xoff=yoff=0; msz>2; yoff=(xoff=(1279-(msz/=1.3))/2)*1023/1279)
    {
      /* make the mask map */
      ht = ((wth = msz) * 1023/1279);
      writeword(xoff, MASK_MAP_XOFFSET);
      writeword(yoff, MASK_MAP_YOFFSET);
      writebyte(BPP1, MEM_ACCESS_MODE);
      writeword(COL_CMP_OFF, COLOR_CMP_COND);
      rc = make_map(MASK, skyvbase+0x140000, ht, wth, BPP1);FUSE;
      writebyte(BPP1, MEM_ACCESS_MODE);

      /* update mapA thru the mask */
      rc = mclrscr(MAPA, col, MMBE);FUSE; RSTCOL;
 
    } /* end for msz */ 
    FUSE;

  } while (FALSE);  /* do once allowing for break on err */
                      /* and redo if we were interrupted   */

  rc = set_ret(BMSKTEST, C0B2CRC, rc, MAPA);
  return(rc);
}
