static char sccsid[] = "@(#)25	1.2  src/bos/diag/tu/sky/c062.c, tu_sky, bos411, 9428A410j 10/29/93 13:39:31";
/*
 *   COMPONENT_NAME : (tu_sky) Color Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: c062
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
* Name     :  c062()                             *
* Function :  Tests the plane mask function by   *
*             blting 8 colors against 8 colors   *
*             8 times, each time with another    *
*             plane masked                       *
*                                                *
*************************************************/

c062()
{
  word  ht,wth;
  word  rc;
  lword col,pm;
  

  do
  { 
    initcop();

    /* make the screen  map  */
    ht = ((wth = 1279) * 1023/1279);
    writebyte(BPP8, MEM_ACCESS_MODE);
    rc = make_map(MAPA, skyvbase, ht, wth, BPP8);FUSE;
    rc = clrscr(MAPA, black);FUSE;  

    /* fill in the horizontal color pattern */
    for (col=0; col<8; col++)
    { writebyte(BPP8, MEM_ACCESS_MODE);
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

    /* run vertical colors up and down, using plane masking */
    for (pm=0; pm<8; pm++)
    { for (col=0; col<8; col++)
      { writebyte(BPP8, MEM_ACCESS_MODE);
        writeword(COL_CMP_OFF, COLOR_CMP_COND);
        writelword(pm, PLANE_MASK);
        writebyte(SRC, FGD_MIX);
        writelword(col, FG_COLOR); 
        writeword(wth/64, OP_DIM1);
        writeword(ht, OP_DIM2);
        writeword(((col*wth/64)+(pm*wth/8)),DESTINATION_X_PTR);
        writeword(0, DESTINATION_Y_PTR);
        rc=set_por(MMD,DAP,NWP,FIXED_PATTERN,MAPA,MAPA,BGC,FGC,PXBLT); FUSE;
        rc=copdone(); FUSE;
      }
    }
  } while (FALSE);  /* do once allowing for break on err */
                      /* and redo if we were interrupted   */

  rc = set_ret(PLANEMASK, C062CRC, rc, MAPA);
  return(rc);
}
