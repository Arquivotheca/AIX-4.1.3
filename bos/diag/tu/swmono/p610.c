static char sccsid[] = "@(#)79	1.2  src/bos/diag/tu/swmono/p610.c, tu_swmono, bos411, 9428A410j 10/29/93 14:20:28";
/*
 *   COMPONENT_NAME : (tu_swmono) Grayscale Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: stepdrawbox
 *		stepdrawcrs
 *		stepdrawoct
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
/**********************************************************
*                      stepdrawbox                        *
**********************************************************/
stepdrawbox(map, color)
word map;          /* set to MAPA, MAPB, MAPC */
word color;        
{
   word wth, ht, len;
   byte dy;
   word rc;
 
   if (map > MAPC)
   { sprintf(tinfo.info.msg_buff,"Illegal Map sent to stepdraw()");
     CatErr(STEPDRAWBOX | THISTUFAIL);
     return(STEPDRAWBOX | THISTUFAIL);
   }
   wth = Mapwth(map);
   ht  = Mapht(map);

   do
   { 
     *PIX_MAP_INDEX     = map;
     *FGD_MIX           = SRC;
     *FG_COLOR          = color;
     *COLOR_CMP_COND    = COL_CMP_OFF; /* allow update */

     *DESTINATION_X_PTR = 0;
     *DESTINATION_Y_PTR = 0;
     rc=set_por(MMD, DAP, 0x00, FIXED_PATTERN, map, map, BGC, FGC,DSW); FUSE;
     rc=set_dsr(EASTS, DRAW, wth+1);FUSE;

     *DESTINATION_X_PTR = wth ;
     *DESTINATION_Y_PTR = 0;
     rc=set_por(MMD, DAP, 0x00, FIXED_PATTERN, map, map, BGC, FGC,DSW); FUSE;
     rc=set_dsr(SOUTHS, DRAW, ht+1);FUSE;

     *DESTINATION_X_PTR = wth;
     *DESTINATION_Y_PTR = ht;
     rc=set_por(MMD, DAP, 0x00, FIXED_PATTERN, map, map, BGC, FGC,DSW); FUSE;
     rc=set_dsr(WESTS, DRAW, wth+1);FUSE;

     *DESTINATION_X_PTR = 0;
     *DESTINATION_Y_PTR = ht;
     rc=set_por(MMD, DAP, 0x00, FIXED_PATTERN, map, map, BGC, FGC,DSW); FUSE;
     rc=set_dsr(NORTHS, DRAW, ht+1);  FUSE;

   } while (FALSE); /* do once, allowing for break on error */
   if (rc > ERR_LEVEL)
   { CatErr(STEPDRAWBOX || SUBTU_FAIL);
     return(STEPDRAWBOX || SUBTU_FAIL);
   }
   else 
   { CatErr(STEPDRAWBOX || GOOD_OP);
     return(STEPDRAWBOX || GOOD_OP);
   }
} 
/**********************************************************
*                      stepdrawoct                        *
**********************************************************/
stepdrawoct(map,color)
word color;
{
   word wth, ht, len;
   byte dy;
   word rc;
 
   if (map > MAPC)
   { sprintf(tinfo.info.msg_buff,"Illegal Map sent to stepdraw()");
     CatErr(STEPDRAWOCT | THISTUFAIL);
     return(STEPDRAWOCT | THISTUFAIL);
   }
   wth = Mapwth(map);
   ht  = Mapht(map);
   len = ((ht > wth) ? ht : wth); 

   for (dy=0; dy < 8; dy++)
   { 
     *PIX_MAP_INDEX     = map;
     *DESTINATION_X_PTR = wth/2;
     *DESTINATION_Y_PTR = ht/2;
     *FGD_MIX           = SRC;
     *FG_COLOR          = color;
     *COLOR_CMP_COND    = COL_CMP_OFF; /* allow update */
     rc = set_por(MMD, DAP, 0x00, FIXED_PATTERN, map, map, BGC, FGC,DSW); FUSE; 
     rc = set_dsr(dy, 0x01, len/2);  FUSE;
   } 
   if (rc > ERR_LEVEL)
   { CatErr(STEPDRAWOCT || THISTUFAIL);
     return(STEPDRAWOCT || THISTUFAIL);
   }
   else
   { CatErr(STEPDRAWOCT || GOOD_OP);
     return(STEPDRAWOCT || GOOD_OP);
   }
} 
/**********************************************************
*                      stepdrawcrs                        *
**********************************************************/
stepdrawcrs(map, color)
word map;          /* set to MAPA, MAPB, MAPC */
word color;        
{
   word wth, ht, len;
   byte dy;
   word rc;
 
   if (map > MAPC)
   { sprintf(tinfo.info.msg_buff,"Illegal Map sent to stepdraw()");
     CatErr(STEPDRAWCRS | THISTUFAIL);
     return(STEPDRAWCRS | THISTUFAIL);
   }
   wth = Mapwth(map);
   ht  = Mapht(map);

   do
   { 
     *PIX_MAP_INDEX     = map;
     *FGD_MIX           = SRC;
     *FG_COLOR          = color;
     *COLOR_CMP_COND    = COL_CMP_OFF; /* allow update */

     *DESTINATION_X_PTR = wth/2;
     *DESTINATION_Y_PTR = 0;
     rc=set_por(MMD, DAP, 0x00, FIXED_PATTERN, map, map, BGC, FGC,DSW); FUSE;
     rc=set_dsr(SOUTHS, DRAW, ht+1);FUSE;

     *DESTINATION_X_PTR = wth/2;
     *DESTINATION_Y_PTR = ht;
     rc=set_por(MMD, DAP, 0x00, FIXED_PATTERN, map, map, BGC, FGC,DSW); FUSE;
     rc=set_dsr(NORTHS, DRAW, ht+1);  FUSE;

     *DESTINATION_X_PTR = wth ;
     *DESTINATION_Y_PTR = ht/2;
     rc=set_por(MMD, DAP, 0x00, FIXED_PATTERN, map, map, BGC, FGC,DSW); FUSE;
     rc=set_dsr(WESTS, DRAW, wth+1);FUSE;

     *DESTINATION_X_PTR = 0 ;
     *DESTINATION_Y_PTR = ht/2;
     rc=set_por(MMD, DAP, 0x00, FIXED_PATTERN, map, map, BGC, FGC,DSW); FUSE;
     rc=set_dsr(EASTS, DRAW, wth+1);FUSE;

   } while (FALSE); /* do once, allowing for break on error */
   if (rc > ERR_LEVEL)
   { CatErr(STEPDRAWCRS || SUBTU_FAIL);
     return(STEPDRAWCRS || SUBTU_FAIL);
   }
   else 
   { CatErr(STEPDRAWCRS || GOOD_OP);
     return(STEPDRAWCRS || GOOD_OP);
   }
} 
