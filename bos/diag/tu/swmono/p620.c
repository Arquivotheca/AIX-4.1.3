static char sccsid[] = "@(#)80	1.2  src/bos/diag/tu/swmono/p620.c, tu_swmono, bos411, 9428A410j 10/29/93 14:20:34";
/*
 *   COMPONENT_NAME : (tu_swmono) Grayscale Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: bres_h304560v
 *		bresbox
 *		brescrs
 *		centerdraw
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
/********************************************************
*                      bresbox                          *
********************************************************/
bresbox(map, color)
word map;          /* set to MAPA, MAPB, MAPC */
word color;        
{
   word wth, ht, len;
   byte dy;
   word rc;
 
   if (map > MAPC)
   { sprintf(tinfo.info.msg_buff,"Illegal Map sent to stepdraw()");
     CatErr(BRESBOX | THISTUFAIL);
     return(BRESBOX | THISTUFAIL);
   }
   wth = Mapwth(map);
   ht  = Mapht(map);

   do
   { 
     writebyte(SRC,FGD_MIX);
     writelword(color,FG_COLOR);
     writeword(COL_CMP_OFF,COLOR_CMP_COND);

     rc=bres(0,0,wth,0,MMD,DAP,FIXED_PATTERN,map,map,BGC,FGC,LDW); FUSE;

     rc=bres(wth,0,wth,ht,MMD,DAP,FIXED_PATTERN,map,map,BGC,FGC,LDW); FUSE;

     rc=bres(wth,ht,0,ht,MMD,DAP,FIXED_PATTERN,map,map,BGC,FGC,LDW); FUSE;

     rc=bres(0,ht,0,0,MMD,DAP,FIXED_PATTERN,map,map,BGC,FGC,LDW); FUSE;

   } while (FALSE); /* do once, allowing for break on error */
   if (rc > ERR_LEVEL)
   { CatErr(BRESBOX || SUBTU_FAIL);
     return(BRESBOX || SUBTU_FAIL);
   }
   else 
   { CatErr(BRESBOX || GOOD_OP);
     return(BRESBOX || GOOD_OP);
   }
} 
/***********************************************************
* Name       : bres_h304560v                               *
* Function   : Uses Bressenham line Draw to draw           *
*              0,30,45,60,90 degree lines                  *
* Calls      : make_map, set_dsr, writeword, readword      *
* Called by  : c000, run_tu                                *
***********************************************************/
word centerx, centery,Map;

bres_h304560v(map,col) 
word map;
lword col;
{ 
  word wth,ht,ptx,pty;

  GETBASE
  Map = map;
  
  writebyte(SRC, FGD_MIX);
  writelword(col, FG_COLOR); 
  writeword(COL_CMP_OFF, COLOR_CMP_COND);
  wth = Mapwth(map);
  ht  = Mapht(map);
  centerx = wth/2;
  centery = ht/2;

  ptx = tan30(centerx);
  pty = tan30(centery);

  centerdraw(centerx*2,centery);                            /* 0 degrees */
  centerdraw(centerx*2,centery - ptx);           /* 30 degrees */
  centerdraw(centerx*2,0);                                  /* 45 degrees */
  centerdraw(centerx + pty, 0);                  /* 60 degrees */

  centerdraw(centerx,0);                                    /* 90 degrees */
  centerdraw(centerx - pty, 0);                  /* 120 degrees */
  centerdraw(0,0);                                          /* 135 degrees */
  centerdraw(0,centery - ptx);                   /* 150 degrees */

  centerdraw(0,centery);                                    /* 180 degrees */
  centerdraw(0,centery + ptx);                   /* 210 degrees */
  centerdraw(0,centery*2);                                  /* 225 degrees */
  centerdraw(centerx - pty, centery*2);          /* 240 degrees */

  centerdraw(centerx, centery*2);                           /* 270 degrees */
  centerdraw(centerx + pty, centery*2);          /* 300 degrees */
  centerdraw(centerx*2, centery*2);                         /* 315 degrees */
  centerdraw(centerx*2,centery + ptx);           /* 330 degrees */

  CatErr(HV304560 | GOOD_OP); 
  return(HV304560 | GOOD_OP); 
}
 

/***********************************************************
* Name       : centerdraw                                  *
* Function   : Uses Bressenham line Draw to draw           *
*              lines from the center of a map (given       *
*              centerx and centery are preset external     *
*              variables) to the specified points          *
* Calls      : bres                                        * 
* Called by  : c033                                        *
***********************************************************/
centerdraw(endx,endy)
word endx,endy;
{
  bres(centerx,centery,endx,endy,MMD,DAP,FIXED_PATTERN,Map,Map,BGC,FGC,LDW); 
}

/********************************************************
*                      brescrs                          *
********************************************************/
brescrs(map, color)
word map;          /* set to MAPA, MAPB, MAPC */
word color;        
{
   word wth, ht, len;
   byte dy;
   word rc;
 
   if (map > MAPC)
   { sprintf(tinfo.info.msg_buff,"Illegal Map sent to stepdraw()");
     CatErr(BRESCRS | THISTUFAIL);
     return(BRESCRS | THISTUFAIL);
   }
   wth = Mapwth(map);
   ht  = Mapht(map);

   do
   { 
     writebyte(SRC,FGD_MIX);
     writelword(color,FG_COLOR);
     writeword(COL_CMP_OFF,COLOR_CMP_COND);

     rc=bres(wth/2,0,wth/2,ht,MMD,DAP,FIXED_PATTERN,map,map,BGC,FGC,LDW); FUSE;
     rc=bres(wth/2,ht,wth/2,0,MMD,DAP,FIXED_PATTERN,map,map,BGC,FGC,LDW); FUSE;

     rc=bres(wth,ht/2,0,ht/2,MMD,DAP,FIXED_PATTERN,map,map,BGC,FGC,LDW); FUSE;

     rc=bres(0,ht/2,wth,ht/2,MMD,DAP,FIXED_PATTERN,map,map,BGC,FGC,LDW); FUSE;

   } while (FALSE); /* do once, allowing for break on error */
   if (rc > ERR_LEVEL)
   { CatErr(BRESCRS || SUBTU_FAIL);
     return(BRESCRS || SUBTU_FAIL);
   }
   else 
   { CatErr(BRESCRS || GOOD_OP);
     return(BRESCRS || GOOD_OP);
   }
} 
