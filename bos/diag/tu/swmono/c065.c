static char sccsid[] = "@(#)55	1.2  src/bos/diag/tu/swmono/c065.c, tu_swmono, bos411, 9428A410j 10/29/93 14:18:36";
/*
 *   COMPONENT_NAME : (tu_swmono) Grayscale Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: c065
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
* Name : c065                                    *
* Function : Test the 4 Bit Per Pel Function of  *
*            a map, including a 4BPP color expand*
*                                                *
*************************************************/
c065()
{
  word  ht,wth;
  word  sdx,sdy,dpx,dpy,dir;
  word  rc;

  do
  { 
    writebyte(BPP4, MEM_ACCESS_MODE);
    initcop();
    writeword(0x6004, INDX_ADDR);
    writeword(0x640F, INDX_ADDR);
    writeword(0x4350, INDX_ADDR);
    writeword(0x5102, INDX_ADDR);

    /* make the screen  map clear to red */
    ht = ((wth = 1280-1) * 1023/1279)
    ;
    writebyte(BPP4, MEM_ACCESS_MODE);
    rc = make_map(MAPA, skyvbase, ht, wth, BPP4);FUSE;
    rc = clrscr(MAPA, red);FUSE;

    /* make the pattern map = area boundry map and clear to zeros */
    ht = ((wth = 0x400) * 1024/1280);
    writebyte(BPP1, MEM_ACCESS_MODE);
    rc = make_map(MAPC, skyvbase+0x0A0000, ht, wth, BPP1);FUSE;
    rc = clrscr(MAPC, black);FUSE;
    /*source map, background info, and fs are don't cares due to fixed pattern*/
    /* foreground color is a don't care due to fg mix = zeros */
    /* one bit per pel map made and cleared to color 0 */
    dpx = (1280-wth)/2;
    dpy = (1024-ht)/2;
    /* Source Map A is a don't care since its pels are combined with the FG   */
    /* color under SRC mix if the pattern is a one  bit, or Map A's pels are  */
    /* combined with the BG color under SRC Mix if the pattern pel is zero    */

    /* make a diamond boundry on the pattern map */

 
      /* make the pattern map and put a diamond pattern on it */
      writebyte(BPP1, MEM_ACCESS_MODE);
      writelword(white, FG_COLOR);
      writebyte(SRCxorDEST, FGD_MIX);
      writeword(COL_CMP_OFF, COLOR_CMP_COND);
      rc=bres(wth/2,0,wth,ht/2,MMD,DAB,FIXED_PATTERN,MAPC,MAPC,BGC,FGC,LDW);FUSE;
      rc=bres(wth,ht/2,wth/2,ht,MMD,DAB,FIXED_PATTERN,MAPC,MAPC,BGC,FGC,LDW);FUSE;
      rc=bres(wth/2,ht,0,ht/2,MMD,DAB,FIXED_PATTERN,MAPC,MAPC,BGC,FGC,LDW);FUSE;
      rc=bres(0,ht/2,wth/2,0,MMD,DAB,FIXED_PATTERN,MAPC,MAPC,BGC,FGC,LDW);FUSE;
      rc=bres(0,0,wth,0,MMD,DAB,FIXED_PATTERN,MAPC,MAPC,BGC,FGC,LDW);FUSE;
      rc=bres(wth,0,wth,ht,MMD,DAB,FIXED_PATTERN,MAPC,MAPC,BGC,FGC,LDW);FUSE;
      rc=bres(wth,ht,0,ht,MMD,DAB,FIXED_PATTERN,MAPC,MAPC,BGC,FGC,LDW);FUSE;
      rc=bres(0,ht,0,0,MMD,DAB,FIXED_PATTERN,MAPC,MAPC,BGC,FGC,LDW);FUSE;

      /* area fill the mask map with the pattern's diamond */
      rc=pattfill(MAPA,dpx,dpy,wth,ht,MAPC,0,0,yellow,black,BPP4); FUSE;

  } while (FALSE);  /* do once allowing for break on err */
                      /* and redo if we were interrupted   */

  rc = set_ret(BPP4_TEST, C065CRC, rc, MAPA);
  return(rc);
}
