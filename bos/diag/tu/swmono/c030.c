static char sccsid[] = "@(#)49	1.2  src/bos/diag/tu/swmono/c030.c, tu_swmono, bos411, 9428A410j 10/29/93 14:18:10";
/*
 *   COMPONENT_NAME : (tu_swmono) Grayscale Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: c030
 *		c031
 *		c032
 *		c040
 *		c041
 *		c042
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
* Name       : c030()                                   *
* Function   : Test Bressenham Line Draw                *
*                                                       *
********************************************************/
c030()
{
  word rc;
  word ht;
  word wth;
  byte color;

  do
  {
    initcop();
    /*set mapA to the screen, set mapB to the test Destination map on screen*/
    rc = make_map(MAPA, skyvbase, 1024-1, 1280-1, BPP4); FUSE;
    rc = clrscr(MAPA, blue); FUSE;
    rc = bres_h304560v(MAPA, white); FUSE;
    rc = bresbox(MAPA, white); FUSE;

    ht = ((wth = 0x3DE) * 1023/1279);
    rc = make_map(MAPB, skyvbase+0x0A0000, ht, wth, BPP4);FUSE;
    rc = clrscr(MAPB, white); FUSE;
    /* center test map on the screen */
    rc = s2dpxblt(MAPB,0x00,0x00,wth,ht,MAPA,(1280-wth)/2,(1024-ht)/2);FUSE;

    rc = bres_h304560v(MAPB, green); FUSE;
    rc = bresbox(MAPB, red); FUSE;

    rc=s2dpxblt(MAPB,0x00,0x00,wth,ht,MAPA,(1280-wth)/2,(1024-ht)/2);FUSE;
    rc = set_ret(BRESDRAW0, C030CRC, rc, MAPA);
  } while (FALSE);  /* do once allowing for break on err */
                      /* and redo if we were interrupted   */
  return(rc);
}
/********************************************************
* Name       : c031()                                   *
* Function   : Test Screen Length Bressenham Line Draw  *
*                                                       *
********************************************************/
c031()
{
  word rc;
  word ht;
  word wth;
  byte color;


  do
  {
    initcop();
    /*set mapA to the screen, set mapB to the test Destination map on screen*/
    rc = make_map(MAPA, skyvbase, 1024-1, 1280-1, BPP4); FUSE;
    rc = clrscr(MAPA, blue); FUSE;
    rc = brescrs(MAPA, yellow); FUSE;
    rc = bresbox(MAPA, yellow); FUSE;

    ht = ((wth = 0x3DE) *1023/1279); /* make MapB  (ht+1) x (wth+1) */
    rc = make_map(MAPB, skyvbase+0x0A0000, ht, wth, BPP4);FUSE;
    rc = clrscr(MAPB, yellow); FUSE;/* MapB is cleared to Palette color 1 */
    /* center test map on the screen */
    rc = s2dpxblt(MAPB,0x00,0x00,wth,ht,MAPA,(1280-wth)/2,(1024-ht)/2);FUSE;

    rc = brescrs(MAPB, black); FUSE;
    rc = bresbox(MAPB, brown); FUSE;

    rc=s2dpxblt(MAPB,0x00,0x00,wth,ht,MAPA,(1280-wth)/2,(1024-ht)/2);FUSE;
  } while (FALSE);  /* do once allowing for break on err */

  rc = set_ret(BRESDRAW1, C031CRC, rc, MAPA);
  return(rc);
}
/********************************************************
* Name       : c030()                                   *
* Function   : Test Bressenham Line Draw                *
*              (horz,vert,30,45,60 degree)              *
*                                                       *
********************************************************/
c032()
{
  word rc;
  word ht;
  word wth;
  byte color;
  byte white;

  do
  {
    initcop();
    /*set mapA to the screen, set mapB to the test Destination map on screen*/
    rc = make_map(MAPA, skyvbase, 1024-1, 1280-1, BPP4);
    rc = clrscr(MAPA, red); FUSE;/* MapA is cleared to Palette color 0 */
    rc = bres_h304560v(MAPA, black); FUSE;

    ht = ((wth = 0x3DE) * 1023/1279); /* make MapB  (ht+1) x (wth+1) */
    rc = make_map(MAPB, skyvbase+0x0A0000, ht, wth, BPP4);FUSE;
    rc = clrscr(MAPB, black); FUSE;/* MapB is cleared to Palette color 1 */

    rc = bres_h304560v(MAPB, red); FUSE;

    /* center test map on the screen */
    rc=s2dpxblt(MAPB,0x00,0x00,wth,ht,MAPA,(1280-wth)/2,(1024-ht)/2);FUSE;
  } while (FALSE);  /* do once allowing for break on err */

  rc = set_ret(BRESDRAW2, C032CRC, rc, MAPA);
  return(rc);
}
/********************************************************
* Name       : c040()                                   *
* Function   :  Step and Draw
*                                                       *
********************************************************/
c040()
{
  word rc;
  word ht;
  word wth;
  byte color;
  byte dir;


  do
  {
    initcop();
    /*set mapA to the screen, set mapB to the test Destination map on screen*/
    rc = make_map(MAPA, skyvbase, 1024-1, 1280-1, BPP4); FUSE;
    rc = clrscr(MAPA, brown); FUSE;/* MapA is cleared to Palette color 0 */
    rc = stepdrawoct(MAPA, yellow); FUSE;
    rc = stepdrawbox(MAPA, yellow); FUSE;

    ht = ((wth = 0x3DE) * 1023/1279); /* make MapB  (ht+1) x (wth+1) */
    /* center test map on the screen */
    rc = make_map(MAPB, skyvbase+0x0A0000, ht, wth, BPP4);FUSE;
    rc = clrscr(MAPB, yellow); FUSE;/* MapB is cleared to Palette color 1 */
    rc = s2dpxblt(MAPB,0x00,0x00,wth,ht,MAPA,(1280-wth)/2,(1024-ht)/2);FUSE;


    rc = stepdrawoct(MAPB, black);     FUSE;
    rc = stepdrawbox(MAPB, white);     FUSE;
    rc=s2dpxblt(MAPB,0x00,0x00,wth,ht,MAPA,(1280-wth)/2,(1024-ht)/2);FUSE;
  } while (FALSE);  /* do once allowing for break on err */

  rc = set_ret(STEPDRAW0, C040CRC, rc, MAPA);
  return(rc);
}
/********************************************************
* Name       : c041()                                   *
* Function   : Test Screen length Line Draw             *
*                                                       *
********************************************************/
c041()
{
  word rc;
  word ht;
  word wth;
  byte color;
  byte dir;

    do
    {
      initcop();
      /*set mapA to the screen, set mapB to the test Destination map on screen*/
      rc = make_map(MAPA, skyvbase, 1024-1, 1280-1, BPP4); FUSE;
      rc = clrscr(MAPA, purple); FUSE;/* MapA is cleared to Palette color 0 */
      rc = stepdrawcrs(MAPA, yellow); FUSE;
      rc = stepdrawbox(MAPA, yellow); FUSE;

      ht = ((wth = 0x3DE) * 1023/1279); /* make MapB  (ht+1) x (wth+1) */
      /* center test map on the screen */
      rc = make_map(MAPB, skyvbase+0x0A0000, ht, wth, BPP4);FUSE;
      rc = clrscr(MAPB, yellow); FUSE;/* MapB is cleared to Palette color 1 */
      rc = s2dpxblt(MAPB,0x00,0x00,wth,ht,MAPA,(1280-wth)/2,(1024-ht)/2);FUSE;

      rc = stepdrawcrs(MAPB, purple);     FUSE;
      rc = stepdrawbox(MAPB, red);     FUSE;

      rc=s2dpxblt(MAPB,0x00,0x00,wth,ht,MAPA,(1280-wth)/2,(1024-ht)/2);FUSE;
    } while (FALSE);  /* do once allowing for break on err */
    rc = set_ret(STEPDRAW1, C041CRC, rc, MAPA);
    return(rc);
}
/********************************************************
* Name       : c042()                                   *
* Function   : Test All Octants of Step and Draw        *
*                                                       *
********************************************************/
c042()
{
  word rc;
  word ht;
  word wth;
  byte color;
  byte dir;

    do
    {
      initcop();
      /*set mapA to the screen, set mapB to the test Destination map on screen*/
      rc = make_map(MAPA, skyvbase, 1024-1, 1280-1, BPP4); FUSE;
      rc = clrscr(MAPA, blue); FUSE;/* MapA is cleared to Palette color 0 */
      rc = stepdrawoct(MAPA, white); FUSE;

      ht = ((wth = 0x3DE) * 1023/1279); /* make MapB  (ht+1) x (wth+1) */
      /* center test map on the screen */
      rc = make_map(MAPB, skyvbase+0x0A0000, ht, wth, BPP4);FUSE;
      rc = clrscr(MAPB, white); FUSE;/* MapB is cleared to Palette color 1 */
      rc = s2dpxblt(MAPB,0x00,0x00,wth,ht,MAPA,(1280-wth)/2,(1024-ht)/2);FUSE;

      rc = stepdrawoct(MAPB, blue);     FUSE;

      rc=s2dpxblt(MAPB,0x00,0x00,wth,ht,MAPA,(1280-wth)/2,(1024-ht)/2);FUSE;
      } while (FALSE);  /* do once allowing for break on err */

    rc = set_ret(STEPDRAW2, C042CRC, rc, MAPA);
    return(rc);
} 
