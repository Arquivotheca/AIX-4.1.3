static char sccsid[] = "@(#)01	1.2  src/bos/diag/tu/sky/c0b3.c, tu_sky, bos411, 9428A410j 10/29/93 13:39:47";
/*
 *   COMPONENT_NAME : (tu_sky) Color Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: c0b3
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
/****************************************************
* Name     : c0b3                                   * 
* Function : Tests the function of Mask Map Enabled *
*                                                   *
*                                                   *
*                                                   *
*                                                   *
****************************************************/
#define RSTCOL if (col++ > MAXCOL) col = 0;
#define MAXCOL 16

c0b3()
{
  word  ht,wth;
  word  sdx,sdy,dpx,dpy,dir;
  word  rc;
  byte col;
  word xoff, yoff;
  word msz;

  do
  { 
    initcop();
    col = 0;

    /* make the screen  map  */
    ht = ((wth = 1279) * 1023/1279);
    writebyte(BPP8, MEM_ACCESS_MODE);
    rc = make_map(MAPA, skyvbase, ht, wth, BPP8);FUSE;

    /* make the mask map - define as MAPC */
    ht = ((wth = 1279) * 1023/1279);
    writebyte(BPP1, MEM_ACCESS_MODE);
    writeword(0, MASK_MAP_XOFFSET);
    writeword(0, MASK_MAP_YOFFSET);
    rc = make_map(MASK, skyvbase+0x168000, ht, wth, BPP1);FUSE;
    rc = make_map(MAPC, skyvbase+0x168000, ht, wth, BPP1);FUSE;
    rc = clrscr(MAPC, white);FUSE;  /* Enable the entire mask */


    /* put diamonds in squares in diamonds .... */
    for (msz=1279, xoff=yoff=0; msz > 2; yoff=(xoff+=(msz/=2)/2)*1023/1279)
    {
      /* make the pattern map and put a square in it */
      ht = ((wth = msz) * 1023/1279);
      if (wth == 1279) ht++;
      writebyte(BPP1, MEM_ACCESS_MODE);
      writeword(COL_CMP_OFF, COLOR_CMP_COND);
      rc = make_map(MAPB, skyvbase+0x140000, ht, wth, BPP1);FUSE;
      rc = clrscr(MAPB, black);FUSE;  /* set entire map to background */
      writebyte(BPP1, MEM_ACCESS_MODE);
      writelword(white, FG_COLOR);
      writebyte(SRCxorDEST, FGD_MIX);
      writeword(COL_CMP_OFF, COLOR_CMP_COND);
      rc=bres(0,ht,0,0,MMD,DAB,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,LDW);FUSE;
      rc=bres(0,0,wth,0,MMD,DAB,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,LDW);FUSE;
      if (wth != 1279)
      { rc=bres(wth,0,wth,ht,MMD,DAB,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,LDW);FUSE;
        rc=bres(wth,ht,0,ht,MMD,DAB,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,LDW);FUSE;
      } /* allows full screen mask to be full screen, not one pell short */

      /* area fill the mask map with the pattern's square*/
      rc=pattfill(MAPC,xoff,yoff,wth,ht,MAPB,0,0,white,black,BPP1); FUSE;
      /* square is enabled */

      /* update mapA thru the mask */
      rc = mclrscr(MAPA, col, MME);FUSE; RSTCOL;
      rc = clrscr(MAPB, black);FUSE;  /* Remove the entire Pattern */
      rc = clrscr(MAPC, black);FUSE;  /* Enable the entire mask */

 
      /* make the pattern map and put a diamond pattern on it */
      ht = ((wth=msz) * 1023/1279);
      writebyte(BPP1, MEM_ACCESS_MODE);
      writeword(COL_CMP_OFF, COLOR_CMP_COND);
      rc = make_map(MAPB, skyvbase+0x140000, ht, wth, BPP1);FUSE;
      rc = clrscr(MAPB, black); FUSE; /* set entire map to background */
      writebyte(BPP1, MEM_ACCESS_MODE);
      writelword(white, FG_COLOR);
      writebyte(SRCxorDEST, FGD_MIX);
      writeword(COL_CMP_OFF, COLOR_CMP_COND);
      rc=bres(wth/2,0,wth,ht/2,MMD,DAB,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,LDW);FUSE;
      rc=bres(wth,ht/2,wth/2,ht,MMD,DAB,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,LDW);FUSE;
      rc=bres(wth/2,ht,0,ht/2,MMD,DAB,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,LDW);FUSE;
      rc=bres(0,ht/2,wth/2,0,MMD,DAB,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,LDW);FUSE;
      rc=bres(0,0,wth,0,MMD,DAB,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,LDW);FUSE;
      rc=bres(wth,0,wth,ht,MMD,DAB,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,LDW);FUSE;
      rc=bres(wth,ht,0,ht,MMD,DAB,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,LDW);FUSE;
      rc=bres(0,ht,0,0,MMD,DAB,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,LDW);FUSE;

      /* area fill the mask map with the pattern's diamond */
      rc=pattfill(MAPC,xoff,yoff,wth,ht,MAPB,0,0,white,black,BPP1); FUSE;
      /* square is enabled */

      /* update mapA thru the mask  */
      rc = mclrscr(MAPA, col, MME);FUSE; RSTCOL;
      rc = clrscr(MAPB, black);FUSE;  /* Remove the entire Pattern */
      rc = clrscr(MAPC, black);FUSE;  /* Disable the entire mask */
 
      rc = strlen(tinfo.info.err_buff);
      if (rc + 600 > BUFF_LEN)
      { sprintf(tinfo.info.msg_buff,"This Error String is continued below...");
        rc = 0;
        rc_msg(MASKTEST,rc,CPTR);
      }
    } /* end for msz */ 
    FUSE;

  } while (FALSE);  /* do once allowing for break on err */
                      /* and redo if we were interrupted   */

  rc = set_ret(MASKTEST,C0B3CRC,rc,MAPA); 
  return(rc);
}
