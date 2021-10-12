static char sccsid[] = "@(#)74	1.2  src/bos/diag/tu/swmono/p220.c, tu_swmono, bos411, 9428A410j 10/29/93 14:20:11";
/*
 *   COMPONENT_NAME : (tu_swmono) Grayscale Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: clrscr
 *		clrscrint
 *		mclrscr
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
/*****************************************************************
* Name     : clrscr()                                            *
* Function : clears the given map to the given color             *
* PreReqs  : map must have been made using make_map()            *
*            color palette must have been set up for palcol      *
*                                                                *
*****************************************************************/
clrscr(map,palcol)
byte map;      /* Map to be cleared         : set to MAPA, MAPB, MAPC */
byte palcol;   /* Palette Color to Clear to : set to (0x00..0xFF) */
{
  word dim,rc;
  GETIOBASE;
  GETBASE;
  GETVBASE;

  if (map > MAPC)
  { CatErr( CLRSCR | CPF_INVDAT);
    sprintf(tinfo.info.msg_buff,"Illegal MAP Index Sent to clrscr() routine: 0x%X", map);
    return( CLRSCR  | CPF_INVDAT);
  }

  /*  execute a PxBlt Operation  */
  do
  {
  writeword(map, PIX_MAP_INDEX);
  writeword(0x0000, DESTINATION_X_PTR);
  writeword(0x0000, DESTINATION_Y_PTR);
  writeword(0x0000, SOURCE_X_PTR);
  writeword(0x0000, SOURCE_Y_PTR);
  writebyte(SRC, BGD_MIX);
  writebyte(SRC, FGD_MIX);
  writelword(0x0000, BG_COLOR);
  writelword(palcol, FG_COLOR);
  dim = Mapwth(map);               /* width of map    */
  writeword(dim, OP_DIM1);         /* width of pxblt  */
  dim = Mapht(map);                /* height of map   */
  writeword(dim, OP_DIM2);         /* height of PxBlt */
  dim = Mapfmt(map);
  writeword(dim, MEM_ACCESS_MODE);
  writeword(COL_CMP_OFF, COLOR_CMP_COND); /* allow update */
  rc = set_por(MMD, DAP, 0x00, FIXED_PATTERN, map, map, BGC, FGC, PXBLT);FUSE;
  rc=copdone(); FUSE;
  } while (FALSE); /* do once, allowing for break on err */
  if (rc > ERR_LEVEL)
  { return( CLRSCR | SUBTU_FAIL);
  }
  else
  {
  return( CLRSCR  | GOOD_OP);
  }
} 
/*****************************************************************
* Name     : clrscrint()                                         *
* Function : clears the given map to the given color             *
*            and does not check for CoP done                     *.
* PreReqs  : map must have been made using make_map()            *
*            color palette must have been set up for palcol      *
*            interrupts should be enabled to determine if done   *
*****************************************************************/
clrscrint(map,palcol)
byte map;      /* Map to be cleared         : set to MAPA, MAPB, MAPC */
byte palcol;   /* Palette Color to Clear to : set to (0x00..0xFF) */
{
  word dim,rc;
  GETIOBASE;
  GETBASE;
  GETVBASE;

  if (map > MAPC)
  { CatErr( CLRSCR | CPF_INVDAT);
    sprintf(tinfo.info.msg_buff,"Illegal MAP Index Sent to clrscr() routine: 0x%X", map);
    return( CLRSCR  | CPF_INVDAT);
  }

  /*  execute a PxBlt Operation  */
  do
  {
  writeword(map, PIX_MAP_INDEX);
  writeword(0x0000, DESTINATION_X_PTR);
  writeword(0x0000, DESTINATION_Y_PTR);
  writeword(0x0000, SOURCE_X_PTR);
  writeword(0x0000, SOURCE_Y_PTR);
  writebyte(SRC, BGD_MIX);
  writebyte(SRC, FGD_MIX);
  writelword(0x0000, BG_COLOR);
  writelword(palcol, FG_COLOR);
  dim = Mapwth(map);               /* width of map    */
  writeword(dim, OP_DIM1);         /* width of pxblt  */
  dim = Mapht(map);                /* height of map   */
  writeword(dim, OP_DIM2);         /* height of PxBlt */
  dim = Mapfmt(map);
  writeword(dim, MEM_ACCESS_MODE);
  writeword(COL_CMP_OFF, COLOR_CMP_COND); /* allow update */
  rc = set_por(MMD, DAP, 0x00, FIXED_PATTERN, map, map, BGC, FGC, PXBLT);FUSE;
  } while (FALSE); /* do once, allowing for break on err */
  if (rc > ERR_LEVEL)
  { return( CLRSCR | SUBTU_FAIL);
  }
  else
  { return( CLRSCR  | GOOD_OP);
  }
} 
/*****************************************************************
* Name     : mclrscr()                                           *
* Function : clears the given map to the given color             *
* PreReqs  : map must have been made using make_map()            *
*            color palette must have been set up for palcol      *
*                                                                *
*****************************************************************/
mclrscr(map,palcol,mask)
byte map;      /* Map to be cleared         : set to MAPA, MAPB, MAPC */
byte palcol;   /* Palette Color to Clear to : set to (0x00..0xFF) */
byte mask;     /* mask                      : set to MMD, MMBE, MME */
{
  word dim,rc;
  GETIOBASE;
  GETBASE;
  GETVBASE;

  if (map > MAPC)
  { CatErr( MCLRSCR | CPF_INVDAT);
    sprintf(tinfo.info.msg_buff,"Illegal MAP Index Sent to clrscr() routine: 0x%X", map);
    return( MCLRSCR  | CPF_INVDAT);
  }
  writeword(0x6004, INDX_ADDR); /*Access the Palette Read Mask */
  if (mask > MME)
  { CatErr( MCLRSCR | CPF_INVDAT);
    sprintf(tinfo.info.msg_buff,"Illegal MASK Index Sent to clrscr() routine: 0x%X", mask);
    return( MCLRSCR  | CPF_INVDAT);
  }
  writeword((0x6000 | palcol), INDX_ADDR); /*Access palcol-th palette color */

  /*  execute a PxBlt Operation  */
  do
  {
  writeword(map, PIX_MAP_INDEX);
  writeword(0x0000, DESTINATION_X_PTR);
  writeword(0x0000, DESTINATION_Y_PTR);
  writeword(0x0000, SOURCE_X_PTR);
  writeword(0x0000, SOURCE_Y_PTR);
  writebyte(SRC, BGD_MIX);
  writebyte(SRC, FGD_MIX);
  writelword(0x0000, BG_COLOR);
  writelword(palcol, FG_COLOR);
  dim = Mapwth(map);               /* width of map    */
  writeword(dim, OP_DIM1);         /* width of pxblt  */
  dim = Mapht(map);                /* height of map   */
  writeword(dim, OP_DIM2);         /* height of PxBlt */
  writeword(COL_CMP_OFF, COLOR_CMP_COND); /* allow update */
  rc = set_por(mask, DAP, 0x00, FIXED_PATTERN, map, map, BGC, FGC, PXBLT);FUSE;
  rc=copdone(); FUSE;
  } while (FALSE); /* do once, allowing for break on err */
  if (rc > ERR_LEVEL)
  { return( MCLRSCR | SUBTU_FAIL);
  }
  else
  {
  return( MCLRSCR  | GOOD_OP);
  }
} 
