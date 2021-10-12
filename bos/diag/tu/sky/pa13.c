static char sccsid[] = "@(#)91	1.2  src/bos/diag/tu/sky/pa13.c, tu_sky, bos411, 9428A410j 10/29/93 13:41:33";
/*
 *   COMPONENT_NAME : (tu_sky) Color Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: colxpxblt
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
/***********************************************************
* Name      : colxpxblt()                                  *
* Function  : Source to Destination color expanding PxBlt  *
*                                                          *
***********************************************************/
colxpxblt(smap, sxp, syp, owth, oht,dmap, dxp, dyp, pmap, pxp, pyp,fcol,bcol,fmix,bmix)
word pmap,smap,dmap;/*pattern,source,destination maps: Set to MAPA, MAPB, MAPC*/
word sxp, syp, dxp, dyp, pxp, pyp; /* source/destination x and y pointers */
word owth, oht;  /* Blt size: operation width and height */
lword fcol, bcol;/* Fore/Back ground colors to expand pattern bit to */
byte fmix, bmix; /* Fore/Back ground mixes to use in Blt operation */
{
  lword white,black,red,green,blue,yellow;
  word  ht,wth;
  word  sdx,sdy,dpx,dpy,dir;
  word  pfmt,sfmt, dfmt;  /* patt/source/destination formats */
  word  pmht,smht, dmht,pmwth,smwth, dmwth;  /* patt/source/destination heights/widths */
  word  rc;
  if ((smap > MAPC) || (dmap > MAPC) || (pmap > MAPC))
  { sprintf(tinfo.info.msg_buff,"Illegal Map Sent to colxpxblt()");
    CatMsgs("\n--Pattern Map     = 0x");
    CatMsgx(pmap);
    CatMsgs("\n--Source Map      = 0x");
    CatMsgx(smap);
    CatMsgs("\n--Destination Map = 0x");
    CatMsgx(dmap);
    CatErr(COLXPXB | THISTUFAIL);
    return(COLXPXB | THISTUFAIL);
  }
  
  
  pfmt = Mapfmt(pmap);
  sfmt = Mapfmt(smap);
  dfmt = Mapfmt(dmap);
  if (sfmt != dfmt)
  { sprintf(tinfo.info.msg_buff,"Attempt to PxBlt was made using two different map formats");
    CatMsgs("\n--Source Format      = 0x");
    CatMsgx(sfmt);
    CatMsgs("\n--Destination Format = 0x");
    CatMsgx(dfmt);
    CatErr(COLXPXB | THISTUFAIL);
    return(COLXPXB | THISTUFAIL);
  }
  
  if (pfmt != BPP1)
  { sprintf(tinfo.info.msg_buff,"Attempt to color expand from a non 1 bit/Pel format was made" );
    CatMsgs("\n--Pattern Format      = 0x");
    CatMsgx(pfmt);
    CatErr(COLXPXB | THISTUFAIL);
    return(COLXPXB | THISTUFAIL);
  }
  
  pmht = Mapht(pmap);
  smht = Mapht(smap);
  dmht = Mapht(dmap);
  pmwth= Mapwth(pmap);
  smwth= Mapwth(smap);
  dmwth= Mapwth(dmap);
  if ( (smht < syp+oht) || (smwth < sxp+owth) ||
       (dmht < dyp+oht) || (dmwth < dxp+owth) ||
       (pmht < pyp+oht) || (pmwth < pxp+owth)  )
  { sprintf(tinfo.info.msg_buff,"Operation Dimension is Greater that Map Dimension");
    CatMsgs("\n--Pattern Height       = 0x");
    CatMsgx(pmht);
    CatMsgs("\n--Source Height        = 0x");
    CatMsgx(smht);
    CatMsgs("\n--Destination Height   = 0x");
    CatMsgx(dmht);
    CatMsgs("\n--Source Y Pointer     = 0x");
    CatMsgx(syp);
    CatMsgs("\n--Operation Height     = 0x");
    CatMsgx(oht);
    CatMsgs("\n--Source Width         = 0x");
    CatMsgx(smwth);
    CatMsgs("\n--Destination Width    = 0x");
    CatMsgx(dmwth);
    CatMsgs("\n--Destination X Pointer = 0x");
    CatMsgx(sxp);
    CatMsgs("\n--Operation Width       = 0x");
    CatMsgx(owth);
    CatErr(COLXPXB | THISTUFAIL);
    return(COLXPXB | THISTUFAIL);
  }
  
  /* All parameters appear to be valid, do pxblt */  

  writebyte(sfmt, MEM_ACCESS_MODE);
  writeword(dxp, DESTINATION_X_PTR);
  writeword(dyp, DESTINATION_Y_PTR);
  writeword(sxp, SOURCE_X_PTR);
  writeword(syp, SOURCE_Y_PTR);
  writeword(owth, OP_DIM1);         /* width of pxblt  */  /* -2 is for test*/
  writeword(oht,  OP_DIM2);         /* height of PxBlt */  /* -2 is for test*/
  writelword(fcol, FG_COLOR);
  writebyte(fmix, FGD_MIX);
  writelword(bcol, BG_COLOR);
  writebyte(bmix, BGD_MIX);
  writeword(COL_CMP_OFF, COLOR_CMP_COND); /* allow update */
  set_por(MMD, DAP, 0x00, pmap, smap, dmap, BGC, FGC, PXBLT);
  /* pattern = 1Bit/Pel Pattern Map - uses foreground function for Patt=1
                                    - uses background function for Patt=0
     FG Funct= FGC(foreground color)--\___  
               FGM(FG Mix)------------/    destination
               (destination color)---/   
     BG Funct= BGC(background color)--\___  
               BGM(BG Mix)------------/    destination
               (destination color)---/   
  */
  
  rc=copdone();
  if (rc > ERR_LEVEL)
  { CatErr(COLXPXB | SUBTU_FAIL);
    return(COLXPXB | SUBTU_FAIL);
  }
  else
  { CatErr(COLXPXB | GOOD_OP);
    return(COLXPXB | GOOD_OP);
  }
} 
