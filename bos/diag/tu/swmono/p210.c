static char sccsid[] = "@(#)72	1.2  src/bos/diag/tu/swmono/p210.c, tu_swmono, bos411, 9428A410j 10/29/93 14:20:02";
/*
 *   COMPONENT_NAME : (tu_swmono) Grayscale Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: make_map
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
* Name      : make_map                                    *
* Function  : use specifications to make a Pixel Map      *
*                                                         *
**********************************************************/
make_map(map, org, ht, wth, fmt) 
word map;           /* set to MAPA, MAPB, MAPC, or MASK index values */
lword *org;         /* map base address (origin) */
word ht;            /* map height */
word wth;           /* map width */
word fmt;           /* map format */
{
word rc;

   GETBASE          /* macro to get the base address */
  
   if (map > MAPC)
   { CatErr( MAKEMAP | CPF_INVDAT);
     sprintf(tinfo.info.msg_buff,"Illegal MAP Index Sent to make_map routine: 0x%X", map);
     return( MAKEMAP | CPF_INVDAT);
   }
   if (wth > 0x0FFF)
   { CatErr( MAKEMAP | CPF_INVDAT);
     sprintf(tinfo.info.msg_buff,"Illegal Pixel Map Width Sent to make_map routine: 0x%X", wth);
     return( MAKEMAP | CPF_INVDAT);
   }
   if (ht > 0x0FFF)
   { CatErr( MAKEMAP | CPF_INVDAT);
     sprintf(tinfo.info.msg_buff,"Illegal Pixel Map Height Sent to make_map routine: 0x%X", ht);
     return( MAKEMAP | CPF_INVDAT);
   }
   if ((fmt != BPP4) && (fmt != BPP4) && (fmt != BPP1))
   { CatErr( MAKEMAP | CPF_INVDAT);
     sprintf(tinfo.info.msg_buff,"Illegal Pixel Map Format Sent to make_map routine: 0x%X", fmt);
     return( MAKEMAP | CPF_INVDAT);
   }


   /* all data appears valid, define the map */

   if (map == MAPA)              /* first, set the external map variables */
   {
     MapAht  = ht;               /* for future reference */
     MapAwth = wth;
     MapAfmt = fmt;
     MapAorg = org;
   }
   else if (map == MAPB)
   {
     MapBht  = ht;
     MapBwth = wth;
     MapBfmt = fmt;
     MapBorg = org;
   }
   else if (map == MAPC)
   {
     MapCht  = ht;
     MapCwth = wth;
     MapCfmt = fmt;
     MapCorg = org;
   }
   else
   {
   }

   /* now set the coprocessor registers, * /
   /* which are not readable for future use */
   writeword(map, PIX_MAP_INDEX); 
   writelword(org, PIX_MAP_BASE);  
   writeword(ht,  PIX_MAP_HT);
   writeword(wth, PIX_MAP_WTH);
   writeword(fmt, PIX_MAP_FMT);
   CatErr(MAKEMAP | GOOD_OP);
   return(MAKEMAP | GOOD_OP);

}

