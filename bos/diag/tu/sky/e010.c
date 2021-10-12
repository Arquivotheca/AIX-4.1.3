static char sccsid[] = "@(#)31	1.1.1.2  src/bos/diag/tu/sky/e010.c, tu_sky, bos411, 9428A410j 2/16/94 10:38:00";
/*
 *   COMPONENT_NAME : (tu_sky) Color Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: copdoneqwik
 *		e010
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
/********************************************************************
* Name     : e010()                                                 *
* Function : Scrolling H's                                          *
********************************************************************/
#define SCRL 10
e010()
{
  word  blkht,  fldht  = 30;
  word  blkwth, fldwth = 12;
  lword fcol;
  lword bcol;
  byte stat,pic;
  word  rc;
  lword x,y;

  do
  {
    initcop();
    /* reverse original f and b for new FCC req'mnts 11/12/92*/
    fcol = white;
    bcol = black;
    rc = make_map(MAPA, skyvbase, 1024, 1279, BPP8); FUSE;
    rc = clrscr(MAPA, white); FUSE;
    rc = make_map(MAPC, skyvbase+0x140000, fldht-1, fldwth-1, BPP8); FUSE;
    rc = clrscr(MAPC, bcol); FUSE;

    writebyte(SRC, FGD_MIX);
    writelword(fcol, FG_COLOR);
    /* Top of Leg horizontal hashes */
    rc = bres(2,8,6,8, MMD, DAP, FIXED_PATTERN, MAPC, MAPC, BGC, FGC, LDW);FUSE;
    rc = bres(8,8,12,8,MMD, DAP, FIXED_PATTERN, MAPC, MAPC, BGC, FGC, LDW);FUSE;
    /* Bottom of Leg horizontal hashes */
    rc = bres(2,24,6,24,MMD,DAP, FIXED_PATTERN, MAPC, MAPC, BGC, FGC, LDW);FUSE;
    rc = bres(8,24,12,24,MMD,DAP,FIXED_PATTERN, MAPC, MAPC, BGC, FGC, LDW);FUSE;
    /* Middle of H horizontal bar */
    rc = bres(3,15,11,15,MMD,DAP,FIXED_PATTERN, MAPC, MAPC, BGC, FGC, LDW);FUSE;
    rc = bres(3,16,11,16,MMD,DAP,FIXED_PATTERN, MAPC, MAPC, BGC, FGC, LDW);FUSE;
    /* Left Leg */
    rc = bres(3,8,3,24,MMD,DAP,FIXED_PATTERN, MAPC, MAPC, BGC, FGC, LDW);FUSE;
    rc = bres(4,8,4,24,MMD,DAP,FIXED_PATTERN, MAPC, MAPC, BGC, FGC, LDW);FUSE;
    rc = bres(5,8,5,24,MMD,DAP,FIXED_PATTERN, MAPC, MAPC, BGC, FGC, LDW);FUSE;
    /* Right Leg */
    rc = bres( 9,8, 9,24,MMD,DAP,FIXED_PATTERN, MAPC, MAPC, BGC, FGC, LDW);FUSE;
    rc = bres(10,8,10,24,MMD,DAP,FIXED_PATTERN, MAPC, MAPC, BGC, FGC, LDW);FUSE;
    rc = bres(11,8,11,24,MMD,DAP,FIXED_PATTERN, MAPC, MAPC, BGC, FGC, LDW);FUSE;

    /* make a row of 'H' blocks */
    rc = make_map(MAPB, skyvbase, fldht-1, 1280-1, BPP8);FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,fldwth-1,fldht-1,MAPB,0x00,0x00);
    for(blkwth=fldwth; blkwth*2 <= 1280; blkwth *= 2)
    { rc=s2dpxblt(MAPB, 0x00, 0x00, blkwth-1, fldht-1, MAPB, blkwth, 0x00);FUSE;
    }
    FUSE; 
    for(; blkwth <= 1280-fldwth; blkwth += fldwth)
    { rc=s2dpxblt(MAPC, 0x00, 0x00, fldwth-1, fldht-1, MAPB, blkwth, 0x00);FUSE;
    }
    FUSE; 
   
    /* fill the screen with rows of 'H' blocks */
    rc = make_map(MAPC, skyvbase, 1024-1+SCRL, 1280-1, BPP8);FUSE;
    for(blkht=fldht; blkht*2 <= 1024; blkht *= 2)
    { rc = s2dpxblt(MAPC, 0x00, 0x00, 1280-1, blkht-1, MAPC, 0x00, blkht);FUSE;
    }
    FUSE; 
    for(; blkht <= 1024-fldht; blkht += fldht)
    { rc = s2dpxblt(MAPB, 0x00, 0x00, 1280-1, fldht-1, MAPC, 0x00, blkht);FUSE;
    }
    FUSE; 


    /* scroll the puppy */
    /* MapC = the Screen,
       MapC   has it's second row shifted up to the top
    */
    *PIX_MAP_INDEX=MAPC;
    *SOURCE_X_PTR=0x00;
    *DESTINATION_X_PTR=0x00;
    *OP_DIM1=1279;
    *FGD_MIX=SRC;
    *COLOR_CMP_COND=COL_CMP_OFF;
    for (y=0; y < 3*1024/SCRL; y++)
    { 
      copdoneqwik(5000);
      *DESTINATION_Y_PTR=1024;
      *SOURCE_Y_PTR=0x00;
      *OP_DIM2=SCRL-1;
      *PIX_OP_REG = 0x8033A8;
	   /* picked up the top SCRL lines and copied them to the bottom */
      copdoneqwik(500);
      *DESTINATION_Y_PTR=0x00;
      *SOURCE_Y_PTR=SCRL;
      *OP_DIM2=1023;
      *PIX_OP_REG = 0x8033A8;
	   /* moved lines SCRL thru 1024 to lines 0 thru 1023+SCRL */
    } /* for */
  } while (FALSE);  /* do once allowing for break on err */
                      /* and redo if we were interrupted   */
  copdoneqwik(5000);
  return(GOOD_OP) ;
}
/*********************************************************
*                     copdoneqwik                        *
*********************************************************/
extern byte X_PLUS_2;
copdoneqwik(wait)
int wait;
{ volatile byte stat,pic,mam;
  volatile lword x;

  while (TRUE)
  {
    if (X_PLUS_2){if ((mam=readbyte((byte *)(base+0x09)) & 0x80) != 0x80)break;}
    else if (
              (((stat=readbyte(INT_STAT_REG)) & 0x80) != 0x80) 
              || 
              (((pic =readbyte( PI_CNTL_REG)) & 0x80) == 0x80)
            ) { for (x=0;x<wait;x++); continue; }
    else break;
  }
  *INT_STAT_REG=0xFF;
  return(COPDONE | GOOD_OP);
}
