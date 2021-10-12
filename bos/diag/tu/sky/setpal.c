static char sccsid[] = "@(#)04	1.2  src/bos/diag/tu/sky/setpal.c, tu_sky, bos411, 9428A410j 10/29/93 13:42:07";
/*
 *   COMPONENT_NAME : (tu_sky) Color Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: set_pal
 *		spritecol
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
/**********************************************************************
* Name      : set_pal()                                               *
* Function  : given the palette location and color, this routine will *
*             set the location to the color                           *
*                                                                     *
**********************************************************************/
set_pal(loc, red,grn,blu)
byte loc,red,grn,blu;
{
  GETIOBASE; /* get the IO Base Address to access the Display Controller Regs */
  
  writebyte(0x60, INDX_ADDR);  /* access indexed reg 60: Palette Index Lo */
  writebyte(loc, DATAB_ADDR); /*set Palette Index Lo to desired location*/
  /* NOTE: for Skyway: Brooktree Palette DAC, don't need Palette Index Hi */

  writebyte(0x65, INDX_ADDR);  /* access indexed reg 65: Palette Data */
  writebyte(red, DATAB_ADDR); /* write the palette RED data */
  writebyte(grn, DATAB_ADDR); /* write the palette GREEN data */
  writebyte(blu, DATAB_ADDR); /* write the palette BLUE data */
}
/**********************************************************************
* Name      : spritecol()                                             *
* Function  : given the sprite  location and color, this routine will *
*             set the location to the color                           *
*                                                                     *
**********************************************************************/
spritecol(loc, red,grn,blu)
byte loc,red,grn,blu;
{
  GETIOBASE; /* get the IO Base Address to access the Display Controller Regs */
  
  writebyte(0x60, INDX_ADDR);  /* access indexed reg 60: Palette Index Lo */
  writebyte(loc, DATAB_ADDR); /*set Palette Index Lo to desired location*/
  /* NOTE: for Skyway: Brooktree Palette DAC, don't need Palette Index Hi */

  writebyte(0x38, INDX_ADDR);  /* access indexed reg 38: Sprite Color Data */
  writebyte(red, DATAB_ADDR); /* write the palette RED data */
  writebyte(grn, DATAB_ADDR); /* write the palette GREEN data */
  writebyte(blu, DATAB_ADDR); /* write the palette BLUE data */
}
