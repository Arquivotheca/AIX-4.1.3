static char sccsid[] = "@(#)54	1.2  src/bos/diag/tu/swmono/c064.c, tu_swmono, bos411, 9428A410j 10/29/93 14:18:32";
/*
 *   COMPONENT_NAME : (tu_swmono) Grayscale Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: c064
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
#define RSTCOL if (col++ > MAXCOL) col = 0;
#define MAXCOL 16
/*************************************************
* Name     :  c064()                             *
* Function :  Tests the octant in the pixel      *
*             operation register by blting       *
*             blocks onto octant corners         *
*                                                *
*                                                *
*************************************************/

c064()
{
  word  ht,wth;
  int  framex,framey,framew,frameh;
  word  dim, rc;
  lword xspot;
  lword col;
  

  do
  { 
    initcop();

    /* make the screen  map  */
    ht = ((wth = 1279) * 1023/1279);
    writebyte(BPP4, MEM_ACCESS_MODE);
    rc = make_map(MAPA, skyvbase, ht, wth, BPP4);FUSE;
    rc = clrscr(MAPA, blue);FUSE;  

    /* fill in the frame */
    frameh = ((framew = 0x400) * 1023/1279);
    framey = ((framex = (wth - framew)/2) * 1023/1279);
    writebyte(BPP4, MEM_ACCESS_MODE);
    writeword(COL_CMP_OFF, COLOR_CMP_COND);
    writebyte(SRC, FGD_MIX);
    col=0;
    for (xspot=0; xspot<framew/2; xspot++)
    { writelword(col, FG_COLOR); RSTCOL;
      writeword(2-1, OP_DIM1);
      writeword(frameh, OP_DIM2);
      writeword((xspot*2+framex),DESTINATION_X_PTR);
      writeword(framey, DESTINATION_Y_PTR);
      rc=set_por(MMD,DAP,NWP,FIXED_PATTERN,MAPA,MAPA,BGC,FGC,PXBLT); FUSE;
      rc=copdone(); FUSE;
    }

    /* put the low box in place */
    /* the entire low box will be green */
    frameh = ((framew = 0x400) * 1023/1279);
    framey = ((framex = (wth - framew)/2) * 1023/1279);
    writebyte(BPP4, MEM_ACCESS_MODE);
    writeword(COL_CMP_OFF, COLOR_CMP_COND);
    writebyte(SRC, FGD_MIX);
    writelword(green, FG_COLOR); 
    writeword(6*framew/16, OP_DIM1);
    writeword(6 * frameh/16, OP_DIM2);
    writeword((5 * framew/16 + framex + 1), DESTINATION_X_PTR);
    writeword((10 * frameh/16 + framey), DESTINATION_Y_PTR);
    rc=set_por(MMD,DAP,NWP,FIXED_PATTERN,MAPA,MAPA,BGC,FGC,PXBLT); FUSE;
    rc=copdone(); FUSE;

    /* blt the low box to the left box */
    writebyte(BPP4, MEM_ACCESS_MODE);
    writeword(COL_CMP_OFF, COLOR_CMP_COND);
    writebyte(SRCxorDEST, FGD_MIX);
    writeword(6 * framew/16, OP_DIM1);
    writeword(6 * frameh/16, OP_DIM2);
    writeword(5 * framew/16 + framex + 1, SOURCE_X_PTR);
    writeword(10 * frameh/16 + framey, SOURCE_Y_PTR);
    writeword(0 + framex, DESTINATION_X_PTR);
    writeword(5 * frameh/16 + framey, DESTINATION_Y_PTR);
    rc=set_por(MMD,DAP,NWP,FIXED_PATTERN,MAPA,MAPA,BGC,SPM,PXBLT); FUSE;
    rc=copdone(); FUSE;

    /* blt the left box to the high box */
    writebyte(BPP4, MEM_ACCESS_MODE);
    writeword(COL_CMP_OFF, COLOR_CMP_COND);
    writebyte(SRCandDEST, FGD_MIX);
    writeword(6 * framew/16, OP_DIM1);
    writeword(6 * frameh/16, OP_DIM2);
    writeword(5 * framew/16 + framex, SOURCE_X_PTR);
    writeword(5 * frameh/16 + framey, SOURCE_Y_PTR);
    writeword(11 * framew/16 + framex + 1, DESTINATION_X_PTR);
    writeword(0 + framey, DESTINATION_Y_PTR);
    rc=set_por(MMD,DAP,NEP,FIXED_PATTERN,MAPA,MAPA,BGC,SPM,PXBLT); FUSE;
    rc=copdone(); FUSE;

    /* blt the high box to the right box */
    writebyte(BPP4, MEM_ACCESS_MODE);
    writeword(COL_CMP_OFF, COLOR_CMP_COND);
    writebyte(SRCorDEST, FGD_MIX);
    writeword(6 * framew/16, OP_DIM1);
    writeword(6 * frameh/16, OP_DIM2);
    writeword(11 * framew/16 + framex + 1, SOURCE_X_PTR);
    writeword(6  * frameh/16 + framey, SOURCE_Y_PTR);
    writeword(16 * framew/16 + framex, DESTINATION_X_PTR);
    writeword(11 * frameh/16 + framey, DESTINATION_Y_PTR);
    rc=set_por(MMD,DAP,SEP,FIXED_PATTERN,MAPA,MAPA,BGC,SPM,PXBLT); FUSE;
    rc=copdone(); FUSE;

    /* blt the right box to the low box */
    writebyte(BPP4, MEM_ACCESS_MODE);
    writeword(COL_CMP_OFF, COLOR_CMP_COND);
    writebyte(SRCxorDEST, FGD_MIX);
    writeword(6 * framew/16, OP_DIM1);
    writeword(6 * frameh/16, OP_DIM2);
    writeword(10 * framew/16 + framex, SOURCE_X_PTR);
    writeword(11 * frameh/16 + framey, SOURCE_Y_PTR);
    writeword(5  * framew/16 + framex + 1, DESTINATION_X_PTR);
    writeword(16 * frameh/16 + framey, DESTINATION_Y_PTR);
    rc=set_por(MMD,DAP,SWP,FIXED_PATTERN,MAPA,MAPA,BGC,SPM,PXBLT); FUSE;
    rc=copdone(); FUSE;
  } while (FALSE);  /* do once allowing for break on err */
                      /* and redo if we were interrupted   */

  rc = set_ret(OCT_TEST, C064CRC, rc, MAPA);
  return(rc);
}
