static char sccsid[] = "@(#)36	1.2  src/bos/diag/tu/sky/c0f1.c, tu_sky, bos411, 9428A410j 10/29/93 13:40:06";
/*
 *   COMPONENT_NAME : (tu_sky) Color Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: c0f1
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
#define mmm for(rc=1; rc < 0xFFF; rc++); rc = 0;
#define RSTCOL if (col++ > MAXCOL) col = 0;
#define MAXCOL 16
#define X30 0x400
#define Y30 0x000
#define X40 0x400
#define Y40 0x100
#define X50 0x000
#define Y50 0x200
#define X64 0x000
#define Y64 0x300
#define X61 0x300
#define Y61 0x200
#define X62 0x200
#define Y62 0x000
#define XB2 0x100
#define YB2 0x200
#define XB3 0x000
#define YB3 0x000
/********************************************************
* Name       : c0f1()                                   *
* Function   : Test Bressenham Line Draw                *
*                                                       *
********************************************************/
c0f1()
{
  word rc;
  byte color;
  word  sdx,sdy,dpx,dpy,dir;
  word  ht,wth;
  lword col,ccv,pm;
  int  framex,framey,framew,frameh;
  word  dim;
  word xoff, yoff;
  float msz; /* was word, made float to satisfy flakey compiler */

do
{
  do
  {
    initcop();
    /*set mapA to the screen*/
    rc = make_map(MAPA, skyvbase, 1024-1, 1280-1, BPP8); FUSE;
    rc = clrscr(MAPA, blue); FUSE;/* MapA is cleared to Palette color 0 */
  } while (FALSE);

  do /* C030 */
  {
    /*set mapB to the design sub screen */
    rc = make_map(MAPB, skyvbase+0x140000, 0xFF, 0xFF, BPP8); FUSE;
    rc = clrscr(MAPB, red); FUSE;

    rc = bres_h304560v(MAPB, white); FUSE;
    rc = bresbox(MAPB, white); FUSE;

    rc = make_map(MAPC, skyvbase+0x140000+0x10000, 0xA0, 0xA0, BPP8);FUSE;
    rc = clrscr(MAPC, white); FUSE;

    rc = bres_h304560v(MAPC, green); FUSE;
    rc = bresbox(MAPC, black); FUSE;

    rc = s2dpxblt(MAPC,0x00,0x00,0xA0,0xA0,MAPB,(256-0xA0)/2,(256-0xA0)/2);FUSE;
    rc = s2dpxblt(MAPB,0x00,0x00,0xFF,0xFF,MAPA,X30,Y30);FUSE;
  } while (FALSE);  /* do once allowing for break on err */
  FUSE;
  rc = strlen(tinfo.info.err_buff);
  if (rc + 600 > BUFF_LEN)
  { sprintf(tinfo.info.msg_buff,"This Error String is continued below...");
    rc = 0;
    rc_msg(MASKTEST,rc,CPTR);
  }

  do /* C040 */
  {
    rc = make_map(MAPB, skyvbase+0x140000, 0xFF, 0xFF, BPP8); FUSE;
    rc = clrscr(MAPB, brown); FUSE;/* MapA is cleared to Palette color 0 */
    rc = stepdrawoct(MAPB, yellow); FUSE;
    rc = stepdrawbox(MAPB, yellow); FUSE;

    rc = make_map(MAPC, skyvbase+0x140000+0x10000, 0xA0, 0xA0, BPP8);FUSE;
    rc = clrscr(MAPC, yellow); FUSE;
    rc = stepdrawoct(MAPC, black);     FUSE;
    rc = stepdrawbox(MAPC, red);     FUSE;
    rc = s2dpxblt(MAPC,0x00,0x00,0xA0,0xA0,MAPB,(256-0xA0)/2,(256-0xA0)/2);FUSE;
    rc = s2dpxblt(MAPB,0x00,0x00,0xFF,0xFF,MAPA,X40,Y40);FUSE;
  } while (FALSE);  /* do once allowing for break on err */
  FUSE;
  rc = strlen(tinfo.info.err_buff);
  if (rc + 600 > BUFF_LEN)
  { sprintf(tinfo.info.msg_buff,"This Error String is continued below...");
    rc = 0;
    rc_msg(MASKTEST,rc,CPTR);
  }

  do /* C050 */
  { 

    rc = make_map(MAPB, skyvbase+0x140000, 0xFF, 0xFF, BPP8); FUSE;
    rc = clrscr(MAPB, green);FUSE;
    rc = s2dpxblt(MAPB,0x00,0x00,0xFF,0xFF,MAPA,X50,Y50);FUSE;


    /* make the pattern map = area boundry map and clear to zeros */
    ht=wth=0xA0;
    writebyte(BPP1, MEM_ACCESS_MODE);
    rc = make_map(MAPC, skyvbase+0x140000+0x10000, ht, wth, BPP1);FUSE;
    rc = clrscr(MAPC, black);FUSE;
    /*source map, background info, and fs are don't cares due to fixed pattern*/
    /* foreground color is a don't care due to fg mix = zeros */
    /* one bit per pel map made and cleared to color 0 */
    dpx = (0xFF-wth)/2;
    dpy = (0xFF-ht)/2;
    rc=colxpxblt(MAPB,dpx,dpy,wth,ht,MAPB,dpx,dpy,MAPC,0x00,0x00,purple, white,SRC,SRC);FUSE;
    rc = s2dpxblt(MAPB,0x00,0x00,0xFF,0xFF,MAPA,X50,Y50);FUSE;

    /* Source Map A is a don't care since its pels are combined with the FG   */
    /* color under SRC mix if the pattern is a one  bit, or Map A's pels are  */
    /* combined with the BG color under SRC Mix if the pattern pel is zero    */

    /* make an octagon boundry on the pattern map */
    sdx = wth*1/5;    /* octagon will take up 3/5 of the map */
    sdy = ht*1/5;     /* octagon will take up 3/5 of the map */
    dpy = sdy*4;      /* set dest ptr at lowest side: 4/5 map ht */
    dpx = sdx*2;      /* set dest ptr at hrz start  : 2/5 map wth*/
    writebyte(BPP1, MEM_ACCESS_MODE);
    writeword(dpx, DESTINATION_X_PTR);
    writeword(dpy, DESTINATION_Y_PTR);
    writelword(white, FG_COLOR);
    writebyte(SRCxorDEST, FGD_MIX);
    writeword(COL_CMP_OFF, COLOR_CMP_COND);

    for (dir=0; dir < 8; dir++)
    { rc=set_por(MMD, DAB, 0x00, FIXED_PATTERN, MAPC, MAPC, BGC, FGC, DSW);FUSE;
      rc=set_dsr(dir, DRAW, sdx);
    }
    FUSE;
    /* Blt pattern map to screen to view - use color expansion */
    /* foreground = green,     background = yellow */ 
    dpx = (0xFF-wth)/2;
    dpy = (0xFF-ht)/2;
    rc=colxpxblt(MAPB,dpx,dpy,wth,ht,MAPB,dpx,dpy,MAPC,0x00,0x00,brown,blue,SRC,SRC);FUSE;
    rc = s2dpxblt(MAPB,0x00,0x00,0xFF,0xFF,MAPA,X50,Y50);FUSE;
    
    /* fill the pattern maps's octagon boundry */
    /* and Blt pattern map to screen to view - use color expansion */
    writebyte(BPP8, MEM_ACCESS_MODE);
    writebyte(SRC, FGD_MIX);
    writebyte(SRC, BGD_MIX);
    writelword(red, FG_COLOR);
    writelword(yellow, BG_COLOR);
    writeword(0, PATTERN_X_PTR);
    writeword(0, PATTERN_Y_PTR);
    writeword(0xA0, OP_DIM1);
    writeword(0xA0, OP_DIM2);
    writeword(dpx, DESTINATION_X_PTR);
    writeword(dpy, DESTINATION_Y_PTR);
    rc=set_por(MMD, DAP, 0x00, MAPC, MAPC ,MAPB, BGC, FGC, FPXBLT);FUSE;
    rc=copdone(); FUSE;
    rc = s2dpxblt(MAPB,0x00,0x00,0xFF,0xFF,MAPA,X50,Y50);FUSE;
  } while(FALSE); /* do once, allowing for break on err */
  FUSE;
  rc = strlen(tinfo.info.err_buff);
  if (rc + 600 > BUFF_LEN)
  { sprintf(tinfo.info.msg_buff,"This Error String is continued below...");
    rc = 0;
    rc_msg(MASKTEST,rc,CPTR);
  }

  do /* C064 */
  {
    rc = make_map(MAPB, skyvbase+0x140000, 0xFF, 0xFF, BPP8); FUSE;
    rc = clrscr(MAPB, red);FUSE;  

    /* fill in the frame */
    frameh = framew = 0xFF;
    framey = framex = (0xFF - framew)/2;
    writebyte(BPP8, MEM_ACCESS_MODE);
    writeword(COL_CMP_OFF, COLOR_CMP_COND);
    writebyte(SRC, FGD_MIX);
    col=0;
    for (ccc=0; ccc<framew/2; ccc++)
    { writelword(col, FG_COLOR); RSTCOL;
      writeword(2-1, OP_DIM1);
      writeword(frameh, OP_DIM2);
      writeword((ccc*2+framex),DESTINATION_X_PTR);
      writeword(framey, DESTINATION_Y_PTR);
      rc=set_por(MMD,DAP,NWP,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,PXBLT); FUSE;
      rc=copdone(); FUSE;
    }
    FUSE;
    rc = s2dpxblt(MAPB,0x00,0x00,0xFF,0xFF,MAPA,X64,Y64);FUSE;

    /* put the low box in place */
    /* the entire low box will be green */
    frameh = framew = 0x100;
    framey = framex = (0x100 - framew)/2;
    writebyte(BPP8, MEM_ACCESS_MODE);
    writeword(COL_CMP_OFF, COLOR_CMP_COND);
    writebyte(SRC, FGD_MIX);
    writelword(green, FG_COLOR); 
    writeword(6*framew/16, OP_DIM1);
    writeword(6 * frameh/16, OP_DIM2);
    writeword((5 * framew/16 + framex + 1), DESTINATION_X_PTR);
    writeword((10 * frameh/16 + framey), DESTINATION_Y_PTR);
    rc=set_por(MMD,DAP,NWP,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,PXBLT); FUSE;
    rc=copdone(); FUSE;
    rc = s2dpxblt(MAPB,0x00,0x00,0xFF,0xFF,MAPA,X64,Y64);FUSE;

    /* blt the low box to the left box */
    /* the left box will be XORed partially with the frame, and 
         partially with the low box:
         (red=02)   xor (green=03) = (white=01)
         (green=03) xor (green=03) = (black=00)
    */
    writebyte(BPP8, MEM_ACCESS_MODE);
    writeword(COL_CMP_OFF, COLOR_CMP_COND);
    writebyte(SRCxorDEST, FGD_MIX);
    writeword(6 * framew/16, OP_DIM1);
    writeword(6 * frameh/16, OP_DIM2);
    writeword(5 * framew/16 + framex + 1, SOURCE_X_PTR);
    writeword(10 * frameh/16 + framey, SOURCE_Y_PTR);
    writeword(0 + framex, DESTINATION_X_PTR);
    writeword(5 * frameh/16 + framey, DESTINATION_Y_PTR);
    rc=set_por(MMD,DAP,NWP,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,PXBLT); FUSE;
    rc=copdone(); FUSE;
    rc = s2dpxblt(MAPB,0x00,0x00,0xFF,0xFF,MAPA,X64,Y64);FUSE;

    /* blt the left box to the high box */
    /* the high box will be ANDed partially with the frame, and 
         partially with the left box:
         (red=02)   and (white=01) = (black=00)
         (red=02)   and (black=00) = (black=00)
         (white=01) and (white=01) = (white=01)
    */
    writebyte(BPP8, MEM_ACCESS_MODE);
    writeword(COL_CMP_OFF, COLOR_CMP_COND);
    writebyte(SRCandDEST, FGD_MIX);
    writeword(6 * framew/16, OP_DIM1);
    writeword(6 * frameh/16, OP_DIM2);
    writeword(6 * framew/16 + framex, SOURCE_X_PTR);
    writeword(5 * frameh/16 + framey, SOURCE_Y_PTR);
    writeword(11 * framew/16 + framex + 1, DESTINATION_X_PTR);
    writeword(0 + framey, DESTINATION_Y_PTR);
    rc=set_por(MMD,DAP,NEP,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,PXBLT); FUSE;
    rc=copdone(); FUSE;
    rc = s2dpxblt(MAPB,0x00,0x00,0xFF,0xFF,MAPA,X64,Y64);FUSE;

    /* blt the high box to the right box */
    /* the right box will be ORed partially with the frame, and 
         partially with the high box:
         (red=02)   or (black=00) = (red=02)
         (red=02)   or (black=00) = (red=02)
         (red=02)   or (black=00) = (red=02)
         (red=02)   or (white=00) = (green=03)
    */
    writebyte(BPP8, MEM_ACCESS_MODE);
    writeword(COL_CMP_OFF, COLOR_CMP_COND);
    writebyte(SRCorDEST, FGD_MIX);
    writeword(6 * framew/16, OP_DIM1);
    writeword(6 * frameh/16, OP_DIM2);
    writeword(11 * framew/16 + framex + 1, SOURCE_X_PTR);
    writeword(6  * frameh/16 + framey, SOURCE_Y_PTR);
    writeword(16 * framew/16 + framex, DESTINATION_X_PTR);
    writeword(11 * frameh/16 + framey, DESTINATION_Y_PTR);
    rc=set_por(MMD,DAP,SEP,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,PXBLT); FUSE;
    rc=copdone(); FUSE;
    rc = s2dpxblt(MAPB,0x00,0x00,0xFF,0xFF,MAPA,X64,Y64);FUSE;

    /* blt the right box to the low box */
    /* the low box will be XORed partially with the frame, and 
         partially with the right box:
         (red=02)   xor (green=03) = (white=01)
         (red=02)   xor (green=03) = (white=01) 
         (red=02)   xor (black=00) = (red=02)
         (green=03) xor (green=03) = (black=00)
    */
    writebyte(BPP8, MEM_ACCESS_MODE);
    writeword(COL_CMP_OFF, COLOR_CMP_COND);
    writebyte(SRCxorDEST, FGD_MIX);
    writeword(6 * framew/16, OP_DIM1);
    writeword(6 * frameh/16, OP_DIM2);
    writeword(10 * framew/16 + framex, SOURCE_X_PTR);
    writeword(11 * frameh/16 + framey, SOURCE_Y_PTR);
    writeword(5  * framew/16 + framex + 1, DESTINATION_X_PTR);
    writeword(16 * frameh/16 + framey, DESTINATION_Y_PTR);
    rc=set_por(MMD,DAP,SWP,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,PXBLT); FUSE;
    rc=copdone(); FUSE;
    rc = s2dpxblt(MAPB,0x00,0x00,0xFF,0xFF,MAPA,X64,Y64);FUSE;
  } while(FALSE); /* do once, allowing for break on err */
  FUSE;
  rc = strlen(tinfo.info.err_buff);
  if (rc + 600 > BUFF_LEN)
  { sprintf(tinfo.info.msg_buff,"This Error String is continued below...");
    rc = 0;
    rc_msg(MASKTEST,rc,CPTR);
  }

  do /* C0b2 */
  {
    col = 0;
    rc = make_map(MAPB, skyvbase+0x140000, 0x1FF, 0x1FF, BPP8);FUSE;

    /*make incrementally smaller mask rectangles - area inside will be updated*/
    for (msz=0x1FF, xoff=yoff=0; msz > 2; yoff=xoff=(0x200-(msz/=1.3))/2)
    {
      /* make the mask map */
      ht = wth = msz;
      writeword(xoff, MASK_MAP_XOFFSET);
      writeword(yoff, MASK_MAP_YOFFSET);
      writebyte(BPP1, MEM_ACCESS_MODE);
      writeword(COL_CMP_OFF, COLOR_CMP_COND);
      rc = make_map(MASK, skyvbase+0x140000+0x40000, ht, wth, BPP1);FUSE;
      writebyte(BPP1, MEM_ACCESS_MODE);

      /* update mapB thru the mask */
      rc = mclrscr(MAPB, col, MMBE);FUSE; RSTCOL;
      rc = s2dpxblt(MAPB,0x00,0x00,0x1FF,0x1FF,MAPA,XB2,YB2);FUSE;
 
    } /* end for msz */ 
    FUSE;
  } while(FALSE); /* do once, allowing for break on err */
  FUSE;
  rc = strlen(tinfo.info.err_buff);
  if (rc + 600 > BUFF_LEN)
  { sprintf(tinfo.info.msg_buff,"This Error String is continued below...");
    rc = 0;
    rc_msg(MASKTEST,rc,CPTR);
  }

  do /* C061 */
  {

    /* make the screen  map  */
    writebyte(BPP8, MEM_ACCESS_MODE);
    ht=wth=512;
    rc = make_map(MAPB, skyvbase+0x140000, 0x1FF, 0x1FF, BPP8);FUSE;
    rc = clrscr(MAPB, black);FUSE;  

    /* fill in the horizontal color pattern */
    for (col=0; col<8; col++)
    { writebyte(BPP8, MEM_ACCESS_MODE);
      writeword(COL_CMP_OFF, COLOR_CMP_COND);
      writebyte(SRC, FGD_MIX);
      writelword(col, FG_COLOR); 
      writeword(wth, OP_DIM1);
      writeword(ht/8-1, OP_DIM2);
      writeword(0, DESTINATION_X_PTR);
      writeword( (col * ht/8)-1, DESTINATION_Y_PTR);
      rc=set_por(MMD,DAP,NWP,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,PXBLT); FUSE;
      rc=copdone(); FUSE;
    }
    rc = s2dpxblt(MAPB,0x00,0x00,0x1FF,0x1FF,MAPA,X61,Y61);FUSE;

    /* run vertical colors up and down, using color compares */
    for (ccv=0; ccv<8; ccv++)
    { for (ccc=0; ccc<8; ccc++)
      { for (col=0; col<8; col++)
        { writebyte(BPP8, MEM_ACCESS_MODE);
          writeword(ccc, COLOR_CMP_COND);
          writelword(ccv, COLOR_CMP_VAL);
          writebyte(SRC, FGD_MIX);
          writelword(col, FG_COLOR); 
          writeword(wth/512-1, OP_DIM1);
          writeword(ht, OP_DIM2);
          writeword(((col*wth/512)+(ccc*wth/64)+(ccv*wth/8))-1,DESTINATION_X_PTR);
          writeword(0, DESTINATION_Y_PTR);
          rc=set_por(MMD,DAP,NWP,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,PXBLT); FUSE;
          rc=copdone(); FUSE;
        }
      }
    }
    rc = s2dpxblt(MAPB,0x00,0x00,0x1FF,0x1FF,MAPA,X61,Y61);FUSE;
  } while(FALSE); /* do once, allowing for break on err */
  FUSE;
  rc = strlen(tinfo.info.err_buff);
  if (rc + 600 > BUFF_LEN)
  { sprintf(tinfo.info.msg_buff,"This Error String is continued below...");
    rc = 0;
    rc_msg(MASKTEST,rc,CPTR);
  }

  do /* C062 */
  { 

    writebyte(BPP8, MEM_ACCESS_MODE);
    rc = make_map(MAPB, skyvbase+0x140000, 0x1FF, 0x1FF, BPP8);FUSE;
    rc = clrscr(MAPB, black);FUSE;  

    /* fill in the horizontal color pattern */
    for (col=0; col<8; col++)
    { writebyte(BPP8, MEM_ACCESS_MODE);
      writeword(COL_CMP_OFF, COLOR_CMP_COND);
      writebyte(SRC, FGD_MIX);
      writelword(col, FG_COLOR); 
      writeword(wth, OP_DIM1);
      writeword(ht/8-1, OP_DIM2);
      writeword(0, DESTINATION_X_PTR);
      writeword( (col * ht/8)-1, DESTINATION_Y_PTR);
      rc=set_por(MMD,DAP,NWP,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,PXBLT); FUSE;
      rc=copdone(); FUSE;
    }
    rc = s2dpxblt(MAPB,0x00,0x00,0x1FF,0x1FF,MAPA,X62,Y62);FUSE;

    /* run vertical colors up and down, using plane masking */
    for (pm=0; pm<8; pm++)
    { for (col=0; col<8; col++)
      { writebyte(BPP8, MEM_ACCESS_MODE);
        writeword(COL_CMP_OFF, COLOR_CMP_COND);
        writelword(pm, PLANE_MASK);
        writebyte(SRC, FGD_MIX);
        writelword(col, FG_COLOR); 
        writeword(wth/64-1, OP_DIM1);
        writeword(ht-1, OP_DIM2);
        writeword(((col*wth/64)+(pm*wth/8))-1,DESTINATION_X_PTR);
        writeword(0, DESTINATION_Y_PTR);
        rc=set_por(MMD,DAP,NWP,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,PXBLT); FUSE;
        rc=copdone(); FUSE;
      }
    }
    rc = s2dpxblt(MAPB,0x00,0x00,0x1FF,0x1FF,MAPA,X62,Y62);FUSE;
  } while(FALSE); /* do once, allowing for break on err */
  FUSE;
  rc = strlen(tinfo.info.err_buff);
  if (rc + 600 > BUFF_LEN)
  { sprintf(tinfo.info.msg_buff,"This Error String is continued below...");
    rc = 0;
    rc_msg(MASKTEST,rc,CPTR);
  }


  do /* C0B3 */
  { 
    col = 0;

    writebyte(BPP8, MEM_ACCESS_MODE);
    rc = make_map(MAPA, skyvbase+0x140000, 0x1FF, 0x1FF, BPP8);FUSE;

    /* make the mask map - define as MAPC */
    ht = wth = 0x1FF ;
    writebyte(BPP1, MEM_ACCESS_MODE);
    writeword(0, MASK_MAP_XOFFSET);
    writeword(0, MASK_MAP_YOFFSET);
    rc = make_map(MASK, skyvbase+0x140000+0x40000, ht, wth, BPP1);FUSE;
    rc = make_map(MAPC, skyvbase+0x140000+0x40000, ht, wth, BPP1);FUSE;
    rc = clrscr(MAPC, white);FUSE;  /* Enable the entire mask */


    /* put diamonds in squares in diamonds .... */
    /*
    for (msz=0x1FF, xoff=yoff=0; msz > 2; yoff=xoff+=(msz/=2.0)/2)
    --altered for flaky compiler,
    ----rest of increment section of for loop at end of the for loop, below.
    */
    for (msz=0x1FF, xoff=yoff=0; msz > 2; msz/=2.0)
    {
      /* make the pattern map and put a square in it */
      ht = wth = msz;
      if (wth == 0x1FF) ht++;
      writebyte(BPP1, MEM_ACCESS_MODE);
      writeword(COL_CMP_OFF, COLOR_CMP_COND);
      rc = make_map(MAPB, skyvbase+0x140000+0x40000+0x40000/8,ht,wth,BPP1);FUSE;
      rc = clrscr(MAPB, black);FUSE;  /* set entire map to background */
      writebyte(BPP1, MEM_ACCESS_MODE);
      writelword(white, FG_COLOR);
      writebyte(SRCxorDEST, FGD_MIX);
      writeword(COL_CMP_OFF, COLOR_CMP_COND);
      rc=bres(0,ht,0,0,MMD,DAB,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,LDW);FUSE;
      rc=bres(0,0,wth,0,MMD,DAB,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,LDW);FUSE;
      if (wth != 0x1FF)
      { rc=bres(wth,0,wth,ht,MMD,DAB,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,LDW);FUSE;
        rc=bres(wth,ht,0,ht,MMD,DAB,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,LDW);FUSE;
      } /* allows full screen mask to be full screen, not one pell short */

      /* area fill the mask map with the pattern's square*/
      rc=pattfill(MAPC,xoff,yoff,wth,ht,MAPB,0,0,white,black,BPP1); FUSE;/* square is enabled */

      /* update mapA thru the mask */
      rc = mclrscr(MAPA, col, MME);FUSE; RSTCOL;
      rc = clrscr(MAPB, black);FUSE;  /* Remove the entire Pattern */
      rc = clrscr(MAPC, black);FUSE;  /* Enable the entire mask */

      /* move mapA to visable screen */
      rc = make_map(MAPB, skyvbase+0x140000, 0x1FF, 0x1FF, BPP8);FUSE;
      rc = make_map(MAPA, skyvbase, 1024-1, 1280-1, BPP8); FUSE;
      rc = s2dpxblt(MAPB,0x00,0x00,0x1FF,0x1FF,MAPA,XB3,YB3);FUSE;
      /* return to original map assignment */
      rc = make_map(MAPA, skyvbase+0x140000, 0x1FF, 0x1FF, BPP8);FUSE;
      rc = make_map(MAPB, skyvbase+0x140000+0x40000+0x40000/8,ht,wth,BPP1);FUSE;

      /* make the pattern map and put a diamond pattern on it */
      ht = wth = msz;
      writebyte(BPP1, MEM_ACCESS_MODE);
      writeword(COL_CMP_OFF, COLOR_CMP_COND);
      rc = clrscr(MAPB, black); FUSE; /* set entire map to background */
      writebyte(BPP1, MEM_ACCESS_MODE);
      writelword(white, FG_COLOR);
      writebyte(SRCxorDEST, FGD_MIX);
      writeword(COL_CMP_OFF, COLOR_CMP_COND);
      rc=bres(wth/2,0,wth,ht/2,MMD,DAB,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,LDW);FUSE;
      rc=bres(wth,ht/2,wth/2,ht,MMD,DAB,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,LDW);FUSE;
      rc=bres(wth/2,ht,0,ht/2,MMD,DAB,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,LDW);FUSE;
      rc=bres(0,ht/2,wth/2,0,MMD,DAB,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,LDW);FUSE;
      rc=bres(0,0,wth,0,MMD,DAB,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,LDW);FUSE;
      rc=bres(wth,0,wth,ht,MMD,DAB,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,LDW);FUSE;
      rc=bres(wth,ht,0,ht,MMD,DAB,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,LDW);FUSE;
      rc=bres(0,ht,0,0,MMD,DAB,FIXED_PATTERN,MAPB,MAPB,BGC,FGC,LDW);FUSE;

      /* area fill the mask map with the pattern's diamond */
      rc=pattfill(MAPC,xoff,yoff,wth,ht,MAPB,0,0,white,black,BPP1); FUSE;/* square is enabled */

      /* update mapA thru the mask  */
      rc = mclrscr(MAPA, col, MME);FUSE; RSTCOL;
      rc = clrscr(MAPB, black);FUSE;  /* Remove the entire Pattern */
      rc = clrscr(MAPC, black);FUSE;  /* Disable the entire mask */
 
      /* move mapA to visable screen */
      rc = make_map(MAPB, skyvbase+0x140000, 0x1FF, 0x1FF, BPP8);FUSE;
      rc = make_map(MAPA, skyvbase, 1024-1, 1280-1, BPP8); FUSE;
      rc = s2dpxblt(MAPB,0x00,0x00,0x1FF,0x1FF,MAPA,XB3,YB3);FUSE;
      /* return to original map assignment */
      rc = make_map(MAPA, skyvbase+0x140000, 0x1FF, 0x1FF, BPP8);FUSE;
      rc = make_map(MAPB, skyvbase+0x140000+0x40000+0x40000/8,ht,wth,BPP1);FUSE;

      rc = strlen(tinfo.info.err_buff);
      if (rc + 600 > BUFF_LEN)
      { sprintf(tinfo.info.msg_buff,"This Error String is continued below...");
        rc = 0;
        rc_msg(MASKTEST,rc,CPTR);
      }
      ht = msz / 2.0;  /* ht just converts float to int for flaky compiler */
      xoff += ht/2;    /* -- and all the rest of this stuff should be in the */
      yoff  = xoff;    /* surrounding for loop - see commented for above loop */
    } /* end for msz */ 
  } while(FALSE); /* do once, allowing for break on err */

  rc = make_map(MAPA, skyvbase, 1024-1, 1280-1, BPP8); FUSE;
  rc = set_ret(0xC0F10000,0xD1D2 , rc, MAPA);

} while (FALSE);  /* do once allowing for break on err */
                    /* and redo if we were interrupted   */
return(rc);
}
