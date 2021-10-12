static char sccsid[] = "@(#)85	1.2  src/bos/diag/tu/swmono/pa80.c, tu_swmono, bos411, 9428A410j 10/29/93 14:21:00";
/*
 *   COMPONENT_NAME : (tu_swmono) Grayscale Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: pattfill
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
* Name : pattfill                                *
* Function : Pattern Fill                        *
*                                                *
*************************************************/
pattfill(dmap,dx,dy,ox,oy,pmap,px,py,fcol,bcol,bpp)
word dmap,pmap;   /* set to MAPA,MAPB,MAPC */
word dx,dy,px,py; /* xy pointers           */
word ox,oy;       /* xy operational dimensions */
lword fcol,bcol;  /* fg bg colors          */
byte bpp;         /* set to BPP4,BPP4,BPP1 */
{
  word rc;

  do
  { 
    /* fill the dmap using the pmap's boundry */
    bpp = Mapfmt(dmap);
    writebyte(bpp, MEM_ACCESS_MODE);
    writebyte(SRC, FGD_MIX);
    writebyte(SRC, BGD_MIX);
    writelword(fcol, FG_COLOR);
    writelword(bcol, BG_COLOR);
    writeword(px, PATTERN_X_PTR);
    writeword(py, PATTERN_Y_PTR);
    writeword(dx, DESTINATION_X_PTR);
    writeword(dy, DESTINATION_Y_PTR);
    writeword(ox, OP_DIM1);
    writeword(oy, OP_DIM2);
    rc=set_por(MMD, DAP, 0x00, pmap, pmap ,dmap, BGC, FGC, FPXBLT);FUSE;
    rc=copdone(); FUSE;

  } while(FALSE); /* do once, allowing for break on err */
  if (rc > ERR_LEVEL)
  { CatErr(PATT_FILL | SUBTU_FAIL);
    return(PATT_FILL | SUBTU_FAIL);
  }
  else
  { CatErr(PATT_FILL | GOOD_OP);
    return(PATT_FILL | GOOD_OP);
  }
}
