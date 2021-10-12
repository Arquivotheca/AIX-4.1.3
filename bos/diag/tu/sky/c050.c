static char sccsid[] = "@(#)11	1.2  src/bos/diag/tu/sky/c050.c, tu_sky, bos411, 9428A410j 10/29/93 13:39:23";
/*
 *   COMPONENT_NAME : (tu_sky) Color Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: c050
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
#define mmm for(rc=1; rc < 0xFFF; rc++); rc = 0;
/*************************************************
* Name : c050                                    *
* Function : Area Fill Test                      *
*                                                *
*************************************************/
c050()
{
  word  ht,wth;
  word  sdx,sdy,dpx,dpy,dir;
  word  rc;

  do
  { 
    initcop();

    /* make the screen  map clear to green */
    ht = ((wth = 1279) * 1024/1280);
    writebyte(BPP8, MEM_ACCESS_MODE);
    rc = make_map(MAPA, skyvbase, ht, wth, BPP8);FUSE;
    rc = clrscr(MAPA, green);FUSE;

    /* make the pattern map = area boundry map and clear to zeros */
    ht = ((wth = 0x300) * 1024/1280);
    writebyte(BPP1, MEM_ACCESS_MODE);
    rc = make_map(MAPC, skyvbase+0x140000, ht, wth, BPP1);FUSE;
    rc = clrscr(MAPC, black);FUSE;
    /*source map, background info, and fs are don't cares due to fixed pattern*/
    /* foreground color is a don't care due to fg mix = zeros */
    /* one bit per pel map made and cleared to color 0 */
    dpx = (1280-wth)/2;
    dpy = (1024-ht)/2;
    rc=colxpxblt(MAPA,dpx,dpy,wth,ht,MAPA,dpx,dpy,
		 MAPC,0x00,0x00,purple, white,SRC,SRC);FUSE;
    /* Source Map A is a don't care since its pels are combined with the FG   */
    /* color under SRC mix if the pattern is a one  bit, or Map A's pels are  */
    /* combined with the BG color under SRC Mix if the pattern pel is zero    */

    /* make an octagon boundry on the pattern map */
    sdx = (wth - wth*3/5)/2;    /* octagon will take up 3/5 of the map */
    sdy = (ht  - wth*3/5)/2;    /* octagon will take up 3/5 of the map */
    dpy = sdy*3+sdx*2;          /* set dest ptr at lowest side: 4/5 map ht */
    dpx = sdx * 2;              /* set dest ptr at hrz start  : 2/5 map wth*/
    writebyte(BPP1, MEM_ACCESS_MODE);
    writeword(dpx, DESTINATION_X_PTR);
    writeword(dpy, DESTINATION_Y_PTR);
    writelword(white, FG_COLOR);
    writebyte(SRCxorDEST, FGD_MIX);
    writeword(COL_CMP_OFF, COLOR_CMP_COND);

    for (dir=0; dir < 8; dir++)
    { rc=set_por(MMD, DAB, 0x00, FIXED_PATTERN, MAPC, MAPC, BGC, FGC, DSW);FUSE;
      rc=set_dsr(dir, DRAW, sdx);
    }
    FUSE;
    /* Blt pattern map to screen to view - use color expansion */
    /* foreground = green,     background = yellow */ 
    dpx = (1280-wth)/2;
    dpy = (1024-ht)/2;
    rc=colxpxblt(MAPA,dpx,dpy,wth,ht,MAPA,dpx,dpy,
		 MAPC,0x00,0x00,brown,blue,SRC,SRC);FUSE;
    
    /* fill the pattern maps's octagon boundry */
    /* and Blt pattern map to screen to view - use color expansion */
    writebyte(BPP8, MEM_ACCESS_MODE);
    writebyte(SRC, FGD_MIX);
    writebyte(SRC, BGD_MIX);
    writelword(red, FG_COLOR);
    writelword(yellow, BG_COLOR);
    writeword(0, PATTERN_X_PTR);
    writeword(0, PATTERN_Y_PTR);
    writeword(dpx, DESTINATION_X_PTR);
    writeword(dpy, DESTINATION_Y_PTR);
    rc=set_por(MMD, DAP, 0x00, MAPC, MAPC ,MAPA, BGC, FGC, FPXBLT);FUSE;
    rc=copdone(); FUSE;

  } while(FALSE); /* do once, allowing for break on err */

  rc = set_ret(AREA_FILL, C050CRC, rc, MAPA);
  return(rc);
}
