static char sccsid[] = "@(#)87	1.3  src/bos/diag/tu/swmono/pe10.c, tu_swmono, bos411, 9428A410j 6/16/94 17:28:50";
/*
 *   COMPONENT_NAME : (tu_swmono) Grayscale Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: COMBINE
 *		len_crc
 *		map_crc
 *		set_ret
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include "skytu.h"
/*************************************************
* Name     : set_ret()                           *
* Function : test rc, do a crc, and set the      *
*            return code                         *
*                                                *
*************************************************/
set_ret(tucode,crccode,rc,map)
lword tucode;
word crccode;
word  rc;
byte  map;
{ word  crc;


  if (rc > ERR_LEVEL)
  { CatErr(tucode | SUBTU_FAIL);
    return(tucode | SUBTU_FAIL);
  }
  else if ((crc=map_crc(map)) != crccode)
  { sprintf(tinfo.info.msg_buff,"CRC Failure in %X: \n--CRC Expected=0x%X\n--CRC Received=0x%X\n",
    tucode/0x10000, crccode, crc);
    CatErr(tucode | THISTUFAIL);
    return(tucode | THISTUFAIL);
  }
  else
  { CatErr(tucode | GOOD_OP);
    return(tucode | GOOD_OP);
  }
}
/*************************************************
* Name     : map_crc()                           *
* Function : do a crc on a given map             *
*                                                *
*************************************************/
map_crc(map)
byte  map;   /* set to MAPA,MAPB,or MAPC */
{ word  crc;
  word  num_lines,wth;
  lword length, *start_addr; 
  byte  fmt;

  GETVBASE;
  start_addr=MapStart(map);
  start_addr=(lword *)((lword)start_addr - (vbase & 0xFFFFFFF));
  /* now have difference between map start addr. and vbase */
  start_addr=(lword *)(((lword)start_addr / 1280)*1280+vbase);
  /* now have the start address of the first byte of the screen line */

  num_lines = Mapht(map);
  wth       = Mapwth(map);
  fmt       = Mapfmt(map);
  length = (num_lines + 1) * (wth + 1);
  if (fmt == BPP4)  length /= 2;
  if (fmt == BPP1)  length /= 8;
  /* now have the number of bytes to that must go into the crc */

  crc = len_crc(((byte *) start_addr), length);
  return(crc);
}
/*************************************************
* Name     : len_crc()                           *
* Function : do a crc on a section of memory,    *
*            given the start address and length  *
*                                                *
*************************************************/
#define crc_mask 0xff07
#define COMBINE(x, y) (((x) << 8) | (y))

struct bites {
   char msb;
   char lsb;
};

len_crc(pbuff, length)
char *pbuff;
int length;
{
    union accum {
	short whole;
	struct bites bite;
    } avalue, dvalue;

    short ret_crc;
    char datav;
    int i;

    dvalue.whole = 0xffff;
    for(i=0; length > 0; i++, length--) 
    {
	if (i == i/0x10000) sleep(1);
	datav = *(pbuff+i);
	avalue.bite.lsb = (datav ^ dvalue.bite.lsb);
	dvalue.bite.lsb = avalue.bite.lsb;
	avalue.whole = ((avalue.whole * 16) ^ dvalue.bite.lsb);
	dvalue.bite.lsb = avalue.bite.lsb;
	avalue.whole <<= 8;

	avalue.whole >>= 1;
	avalue.bite.lsb ^= dvalue.bite.lsb;

	avalue.whole >>= 4;

	avalue.whole = COMBINE(avalue.bite.lsb, avalue.bite.msb);
	avalue.whole = ((avalue.whole & crc_mask) ^ dvalue.bite.lsb);
	avalue.whole = COMBINE(avalue.bite.lsb, avalue.bite.msb);
	avalue.bite.lsb ^= dvalue.bite.msb;
	dvalue.whole = avalue.whole;
    }
    ret_crc = dvalue.whole;
    return (ret_crc & 0xffff);
}
