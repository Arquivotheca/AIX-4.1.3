static char sccsid[] = "@(#)29	1.2  src/bos/diag/tu/sky/pa10.c, tu_sky, bos411, 9428A410j 10/29/93 13:41:28";
/*
 *   COMPONENT_NAME : (tu_sky) Color Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: s2dpxblt
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
* Name      : s2dpxblt()                                   *
* Function  : Source to Destination PxBlt                  *
*                                                          *
***********************************************************/
s2dpxblt(smap, sxp, syp, owth, oht, dmap, dxp, dyp)
word smap, dmap; /* source,destination maps: Set to MAPA, MAPB, MAPC */
word sxp, syp, dxp, dyp; /* source/destination x and y pointers */
word owth, oht;  /* Blt size: operation width and height */
{
  word sfmt, dfmt;  /* source/destination formats */
  word smht, dmht,smwth, dmwth;  /* source/destination heights/widths */
  word rc;

  if ((smap > MAPC) || (dmap > MAPC))
  { sprintf(tinfo.info.msg_buff,"Illegal Map Sent to s2dpxblt()");
    CatMsgs("\n--Source Map      = 0x");
    CatMsgx(smap);
    CatMsgs("\n--Destination Map = 0x");
    CatMsgx(dmap);
    CatErr(S2DPXB | THISTUFAIL);
    return(S2DPXB | THISTUFAIL);
  }
  
  
  sfmt = Mapfmt(smap);
  dfmt = Mapfmt(dmap);
  if (sfmt != dfmt)
  { sprintf(tinfo.info.msg_buff,"Attempt to PxBlt was made using two different map formats");
    CatMsgs("\n--Source Format      = 0x");
    CatMsgx(sfmt);
    CatMsgs("\n--Destination Format = 0x");
    CatMsgx(dfmt);
    CatErr(S2DPXB | THISTUFAIL);
    return(S2DPXB | THISTUFAIL);
  }
  
  smht = Mapht(smap);
  dmht = Mapht(dmap);
  smwth= Mapwth(smap);
  dmwth= Mapwth(dmap);
  if ( (smht < syp+oht) || (smwth < sxp+owth) ||
       (dmht < dyp+oht) || (dmwth < dxp+owth)  )
  { sprintf(tinfo.info.msg_buff,"Operation Dimension is Greater that Map Dimension");
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
    CatErr(S2DPXB | THISTUFAIL);
    return(S2DPXB | THISTUFAIL);
  }
  
  /* All parameters appear to be valid, do pxblt */  
  *PIX_MAP_INDEX=smap;
  *DESTINATION_X_PTR=dxp;
  *DESTINATION_Y_PTR=dyp;
  *SOURCE_X_PTR=sxp;
  *SOURCE_Y_PTR=syp;
  *OP_DIM1=owth;
  *OP_DIM2=oht;
  *FGD_MIX=SRC;
  *COLOR_CMP_COND=COL_CMP_OFF;
  *PIX_OP_REG = (0x100*dmap) + (0x1000*smap) + (0x8000A8);
  rc=copdone();

  if (rc > ERR_LEVEL)
  { CatErr(S2DPXB | SUBTU_FAIL);
    return(S2DPXB | SUBTU_FAIL);
  }
  else
  { CatErr(S2DPXB | GOOD_OP);
    return(S2DPXB | GOOD_OP);
  }
} 
