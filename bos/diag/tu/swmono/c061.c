static char sccsid[] = "@(#)51	1.2  src/bos/diag/tu/swmono/c061.c, tu_swmono, bos411, 9428A410j 10/29/93 14:18:22";
/*
 *   COMPONENT_NAME : (tu_swmono) Grayscale Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: c061
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
* Name     :  c061()                             *
* Function :  Tests the color compare function   *
*             by using all 8 color compare       *
*             conditions, 8 color compare values,*
*             and 8 resulting destination colors *
*                                                *
*************************************************/

c061()
{
  word  ht,wth;
  word  rc;
  lword col,ccv;
  word  ccc;
  

  do
  { 
    initcop();

    /* make the screen  map  */
    ht = ((wth = 1279) * 1023/1279);
    writebyte(BPP4, MEM_ACCESS_MODE);
    rc = make_map(MAPA, skyvbase, ht, wth, BPP4);FUSE;
    rc = clrscr(MAPA, black);FUSE;  

    /* fill in the horizontal color pattern */
    for (col=0; col<8; col++)
    { writebyte(BPP4, MEM_ACCESS_MODE);
      writeword(COL_CMP_OFF, COLOR_CMP_COND);
      writebyte(SRC, FGD_MIX);
      writelword(col, FG_COLOR); 
      writeword(wth, OP_DIM1);
      writeword(ht/8, OP_DIM2);
      writeword(0, DESTINATION_X_PTR);
      writeword( (col * ht/8), DESTINATION_Y_PTR);
      rc=set_por(MMD,DAP,NWP,FIXED_PATTERN,MAPA,MAPA,BGC,FGC,PXBLT); FUSE;
      rc=copdone(); FUSE;
    }

    /* run vertical colors up and down, using color compares */
    for (ccv=0; ccv<8; ccv++)
    { for (ccc=0; ccc<8; ccc++)
      { for (col=0; col<8; col++)
        { writebyte(BPP4, MEM_ACCESS_MODE);
          writeword(ccc, COLOR_CMP_COND);
          writelword(ccv, COLOR_CMP_VAL);
          writebyte(SRC, FGD_MIX);
          writelword(col, FG_COLOR); 
          writeword(wth/512, OP_DIM1);
          writeword(ht, OP_DIM2);
          writeword(((col*wth/512)+(ccc*wth/64)+(ccv*wth/8)),DESTINATION_X_PTR);
          writeword(0, DESTINATION_Y_PTR);
          rc=set_por(MMD,DAP,NWP,FIXED_PATTERN,MAPA,MAPA,BGC,FGC,PXBLT); FUSE;
          rc=copdone(); FUSE;
        } FUSE; /* end for color, col */
      } FUSE; /* end for color compare condition, ccc */
    } FUSE; /* end for color compare value, ccv */
  } while (FALSE);  /* do once allowing for break on err */
                      /* and redo if we were interrupted   */

  rc = set_ret(CCC_TEST, C061CRC, rc, MAPA);
  return(rc);
}
