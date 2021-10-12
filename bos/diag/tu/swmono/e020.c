static char sccsid[] = "@(#)65	1.2  src/bos/diag/tu/swmono/e020.c, tu_swmono, bos411, 9428A410j 10/29/93 14:19:19";
/*
 *   COMPONENT_NAME : (tu_swmono) Grayscale Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: e020
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
/********************************************************************
* Name     : e020()                                                 *
* Function : IBM EMC Standard Patterns - Static                     *
********************************************************************/
e020()
{ 
  byte stat,pic;
  word  rc;
  lword mapht,mapwth,dec;

  initcop();

  do
  {
    writebyte(SRC, FGD_MIX);

    rc = make_map(MAPA, skyvbase, 1024-1, 1280-1, BPP4); FUSE;
    rc = clrscr(MAPA, black=0x00); FUSE;
    rc = make_map(MAPC, skyvbase+0x0A0000, mapht=(1024*.15)-1, mapwth=(1280*.80)-1, BPP4); FUSE;

    for (dec=0;dec<mapwth; dec++)
    { 
      writelword(white=0x0F, FG_COLOR);
      rc=bres(dec,0,dec,mapht,MMD,DAP,FIXED_PATTERN,MAPC,MAPC,BGC,FGC,LDW);FUSE;
      writelword(black=0x00, FG_COLOR); dec++;
      rc=bres(dec,0,dec,mapht,MMD,DAP,FIXED_PATTERN,MAPC,MAPC,BGC,FGC,LDW);FUSE;
      strcpy(tinfo.info.msg_buff,""); strcpy(tinfo.info.err_buff,"");
    } FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,(1280-mapwth)/2,(1024-mapht*5)/2);FUSE;

    for (dec=0;dec<mapwth; dec++)
    { writelword(white=0x0F, FG_COLOR);
      rc=bres(dec,0,dec,mapht,MMD,DAP,FIXED_PATTERN,MAPC,MAPC,BGC,FGC,LDW);FUSE;
      dec++;
      rc=bres(dec,0,dec,mapht,MMD,DAP,FIXED_PATTERN,MAPC,MAPC,BGC,FGC,LDW);FUSE;
      writelword(black=0x00, FG_COLOR);
      dec++;
      rc=bres(dec,0,dec,mapht,MMD,DAP,FIXED_PATTERN,MAPC,MAPC,BGC,FGC,LDW);FUSE;
      strcpy(tinfo.info.msg_buff,""); strcpy(tinfo.info.err_buff,"");
    }FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,(1280-mapwth)/2,(1024-mapht*5)/2+mapht*2);FUSE;

    for (dec=0;dec<mapwth; dec++)
    { writelword(white=0x0F, FG_COLOR);
      rc=bres(dec,0,dec,mapht,MMD,DAP,FIXED_PATTERN,MAPC,MAPC,BGC,FGC,LDW);FUSE;
      dec++;
      rc=bres(dec,0,dec,mapht,MMD,DAP,FIXED_PATTERN,MAPC,MAPC,BGC,FGC,LDW);FUSE;
      dec++;
      rc=bres(dec,0,dec,mapht,MMD,DAP,FIXED_PATTERN,MAPC,MAPC,BGC,FGC,LDW);FUSE;
      writelword(black=0x00, FG_COLOR);
      dec++;
      rc=bres(dec,0,dec,mapht,MMD,DAP,FIXED_PATTERN,MAPC,MAPC,BGC,FGC,LDW);FUSE;
      dec++;
      rc=bres(dec,0,dec,mapht,MMD,DAP,FIXED_PATTERN,MAPC,MAPC,BGC,FGC,LDW);FUSE;
      strcpy(tinfo.info.msg_buff,""); strcpy(tinfo.info.err_buff,"");
    }FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,(1280-mapwth)/2,(1024-mapht*5)/2+mapht*4);FUSE;

  } while (FALSE);  /* do once, breaking on any error */
  return(GOOD_OP) ;
}
