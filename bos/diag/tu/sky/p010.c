static char sccsid[] = "@(#)27	1.2  src/bos/diag/tu/sky/p010.c, tu_sky, bos411, 9428A410j 10/29/93 13:40:28";
/*
 *   COMPONENT_NAME : (tu_sky) Color Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: initcop
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
initcop()
{
  word rc;

  GETBASE;
  GETVBASE;
  GETIOBASE;
  
  do
  { 

  do
  { /* initialize skyway registers - just like TU A000 - only faster */
    /*
    testdadcr();
    testmcsrr();
    testcrtchorz();
    testcrtcvert();
    testmisc();
    testspr_pal();
    initsprite();
    */
    a001();
  } while (FALSE);  /* executes only once */

    rc = set_pal(black =0x00, 0x00, 0x00, 0x00);FUSE;
    rc = set_pal(white =0x01, 0xFF, 0xFF, 0xFF);FUSE;
    rc = set_pal(red   =0x02, 0xFF, 0x00, 0x00);FUSE;
    rc = set_pal(green =0x03, 0x00, 0xA0, 0x00);FUSE;
    rc = set_pal(blue  =0x04, 0x00, 0x00, 0xFF);FUSE;
    rc = set_pal(grey  =0x05, 0x70, 0x70, 0x70);FUSE;
    rc = set_pal(purple=0x06, 0x80, 0x00, 0x80);FUSE;
    rc = set_pal(ltcyan=0x07, 0x70, 0x00, 0xD0);FUSE;
    rc = set_pal(brown =0x08, 0x80, 0x40, 0x00);FUSE;
    rc = set_pal(yellow=0x09, 0xF0, 0xF0, 0x10);FUSE;
    rc = set_pal(ltgren=0x0A, 0x00, 0xFF, 0x1F);FUSE;
    rc = set_pal(       0x0B, 0x00, 0xA0, 0x00);FUSE;
    rc = set_pal(       0x0C, 0x50, 0x00, 0x00);FUSE;
    rc = set_pal(       0x0D, 0xFF, 0x80, 0x80);FUSE;
    rc = set_pal(       0x0E, 0x00, 0x00, 0xFF);FUSE;
    rc = set_pal(       0x0F, 0x00, 0xFF, 0x00);FUSE;
    rc = set_pal(       0x10, 0xFF, 0x00, 0x00);FUSE;
    rc = spritecol(sred=0x01, 0xFF, 0x00, 0x00);FUSE;
    rc = spritecol(sblue=0x02, 0x00, 0xFF, 0x00);FUSE;
    rc = spritecol(sgreen=0x03, 0x00, 0x00, 0xFF);FUSE;
    writeword(COL_CMP_OFF, COLOR_CMP_COND);
    writebyte(BPP8, MEM_ACCESS_MODE);
    writelword(0xFF, PLANE_MASK);
    writeword(0x6004, INDX_ADDR);  /* access the palette mask */
    writeword(0x64FF, INDX_ADDR);  /* set the mask to 8BBP mode */
    writeword(0x43A0, INDX_ADDR);  /* set the buffer pitch low to 8BBP mode */
    writeword(0x5103, INDX_ADDR);  /* set the display mode to 8BBP mode */

       
  } while(FALSE); /* do once, allowing for break on err */
  if (rc > ERR_LEVEL)
  { CatErr(INIT_COP | SUBTU_FAIL);
    return(INIT_COP | SUBTU_FAIL);
  }
  else
  { CatErr(INIT_COP | GOOD_OP);
    return(INIT_COP | GOOD_OP);
  }
}
