static char sccsid[] = "@(#)95	1.2  src/bos/diag/tu/sky/e030.c, tu_sky, bos411, 9428A410j 10/29/93 13:40:17";
/*
 *   COMPONENT_NAME : (tu_sky) Color Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: e030
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
* Name     : e030()                                                 *
* Function : Kingston EMC Standard Patterns - Static                *
********************************************************************/
e030()
{ 
  byte stat,pic;
  word  rc;
  lword mapht,mapwth,dec,x;
  byte grey1,grey2, grey3,grey4, grey5,grey6, grey7,grey8,magenta,cyan;

  initcop();

  do
  {
    rc = set_pal(black  =0x00, 0x00, 0x00, 0x00);FUSE;
    rc = set_pal(white  =0x01, 0xFF, 0xFF, 0xFF);FUSE;
    rc = set_pal(green  =0x02, 0x00, 0xFF, 0x00);FUSE;
    rc = set_pal(yellow =0x03, 0xFF, 0xFF, 0x00);FUSE;
    rc = set_pal(red    =0x04, 0xFF, 0x00, 0x00);FUSE;
    rc = set_pal(magenta=0x05, 0xFF, 0x00, 0xFF);FUSE;
    rc = set_pal(blue   =0x06, 0x00, 0x00, 0xFF);FUSE;
    rc = set_pal(cyan   =0x07, 0x00, 0xFF, 0xFF);FUSE;
    rc = set_pal(black  =0x08, 0x00, 0x00, 0x00);FUSE;
    rc = set_pal(white  =0x09, 0xFF, 0xFF, 0xFF);FUSE;
    rc = set_pal(grey1  =0x0A, 0xFF, 0xFF, 0xFF);FUSE;
    rc = set_pal(grey2  =0x0B, 0xDB, 0xDB, 0xDB);FUSE;
    rc = set_pal(grey3  =0x0C, 0xB6, 0xB6, 0xB6);FUSE;
    rc = set_pal(grey4  =0x0D, 0x92, 0x92, 0x92);FUSE;
    rc = set_pal(grey5  =0x0E, 0x6D, 0x6D, 0x6D);FUSE;
    rc = set_pal(grey6  =0x0F, 0x49, 0x49, 0x49);FUSE;
    rc = set_pal(grey7  =0x10, 0x25, 0x25, 0x25);FUSE;
    rc = set_pal(grey8  =0x11, 0x00, 0x00, 0x00);FUSE;

    /* Black / White Pattern at top */
    rc = make_map(MAPA, skyvbase, 1024-1, 1280-1, BPP8); FUSE;
    rc = clrscr(MAPA, black); FUSE;
    rc = make_map(MAPC, skyvbase+0x140000, mapht=64-1, mapwth=480-1, BPP8);FUSE;
    rc = clrscr(MAPC, white); FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,0x0,0x0);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,1279-mapwth,192-1);FUSE;


    /* 128 * 512  Color Bars */
    rc = make_map(MAPC, skyvbase+0x140000, mapht=512, mapwth=160-1, BPP8);FUSE;
    for (dec=0;dec<8; dec++)
    { rc = clrscr(MAPC, dec+2); FUSE;
      rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,160*dec,255);FUSE;
    } FUSE;
    strcpy(tinfo.info.err_buff,"E0300000:");

    /* 128 * 128  Grey Scale Squares */
    rc = make_map(MAPC, skyvbase+0x140000, mapht=127, mapwth=160-1, BPP8);FUSE;
    for (dec=0;dec<8; dec++)
    { rc = clrscr(MAPC, dec+0x0A); FUSE;
      rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,160*dec,767);FUSE;
    } FUSE;
    strcpy(tinfo.info.err_buff,"E0300000:");

    /* Black / White Frequency Burst */
    rc = make_map(MAPC,skyvbase+0x140000,mapht=127,mapwth=95,BPP8);FUSE;
    rc = clrscr(MAPC, white); FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,0,895);FUSE;
    rc = clrscr(MAPC, black); FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,96,895);FUSE;
    strcpy(tinfo.info.err_buff,"E0300000:");

    rc = make_map(MAPC,skyvbase+0x140000,mapht=127,mapwth=42,BPP8);FUSE;
    rc = clrscr(MAPC, white); FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,192,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,276,895);FUSE;
    rc = clrscr(MAPC, black); FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,234,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,318,895);FUSE;
    strcpy(tinfo.info.err_buff,"E0300000:");

    rc = make_map(MAPC,skyvbase+0x140000,mapht=127,mapwth=32,BPP8);FUSE;
    rc = clrscr(MAPC, white); FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,360,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,424,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,488,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,552,895);FUSE;
    rc = clrscr(MAPC, black); FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,392,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,456,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,520,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,584,895);FUSE;
    strcpy(tinfo.info.err_buff,"E0300000:");

    rc = make_map(MAPC,skyvbase+0x140000,mapht=127,mapwth=16,BPP8);FUSE;
    rc = clrscr(MAPC, white); FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,616,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,648,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,680,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,712,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,744,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,776,895);FUSE;
    rc = clrscr(MAPC, black); FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,632,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,664,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,696,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,728,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,760,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,792,895);FUSE;
    strcpy(tinfo.info.err_buff,"E0300000:");

    rc = make_map(MAPC,skyvbase+0x140000,mapht=127,mapwth=8,BPP8);FUSE;
    rc = clrscr(MAPC, white); FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,808,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,824,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,840,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,856,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,872,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,888,895);FUSE;
    rc = clrscr(MAPC, black); FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,816,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,832,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,848,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,864,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,880,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,896,895);FUSE;
    strcpy(tinfo.info.err_buff,"E0300000:");

    rc = make_map(MAPC,skyvbase+0x140000,mapht=127,mapwth=4,BPP8);FUSE;
    rc = clrscr(MAPC, white); FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,904,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,912,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,920,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,928,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,936,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,944,895);FUSE;
    rc = clrscr(MAPC, black); FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,908,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,916,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,924,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,932,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,940,895);FUSE;
    rc = make_map(MAPC,skyvbase+0x140000,mapht=127,mapwth=12,BPP8);FUSE;
    rc = clrscr(MAPC, black); FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,948,895);FUSE;
    strcpy(tinfo.info.err_buff,"E0300000:");

    rc = make_map(MAPC,skyvbase+0x140000,mapht=127,mapwth=2,BPP8);FUSE;
    rc = clrscr(MAPC, white); FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,960,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,964,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,968,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,972,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,976,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,980,895);FUSE;
    rc = clrscr(MAPC, black); FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,962,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,966,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,970,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,974,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,978,895);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,982,895);FUSE;
    strcpy(tinfo.info.err_buff,"E0300000:");

    /* ensure lower left corner is black */
    rc = make_map(MAPC, skyvbase+0x140000, mapht=127, mapwth=295, BPP8);FUSE;
    rc = clrscr(MAPC, black); FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,984,895);FUSE;

    /* Lower Left Corner Single Pixels */
    rc = make_map(MAPC, skyvbase+0x140000, mapht=0, mapwth=0, BPP8);FUSE;
    rc = clrscr(MAPC, white); FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,1160,928);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,1160,960);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,1160,992);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,1200,928);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,1200,960);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,1200,992);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,1240,928);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,1240,960);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,mapwth,mapht,MAPA,1240,992);FUSE;

       
  } while (FALSE);  /* do once, breaking on any error */
  return(GOOD_OP) ;
}
