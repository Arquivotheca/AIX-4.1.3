static char sccsid[] = "@(#)81	1.2  src/bos/diag/tu/swmono/p6a0.c, tu_swmono, bos411, 9428A410j 10/29/93 14:20:40";
/*
 *   COMPONENT_NAME : (tu_swmono) Grayscale Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: bres
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
* Name       : bres                                       *
* Function   : Set up and execute a Bresenham Line Draw   *
*                                                         *
*                                                         *
*                                                         *
*                                                         *
**********************************************************/
bres(xo,yo,xf,yf,mask,dm,patt,source,dest,bs,fs,step)
lword xo,yo,xf,yf;  /* line will be drawn from Original X,Y position to Final X,Y position */
byte bs,fs,step,source,dest,patt,mask,dm;  /* POR setup parameters to be passed to set_por() */
{
  long dx,dy,mag_x,mag_y,major,minor;  /* make these variables signed */
  byte  octant;
  lword errterm;
  lword bresk1;
  lword bresk2;
  word  rc;

  GETBASE

  do
  {
    dx = xf - xo;
    dy = yf - yo;
    mag_x = ((dx > 0) ? dx : (0 - dx));
    mag_y = ((dy > 0) ? dy : (0 - dy));
    major = (mag_x >= mag_y ? mag_x : mag_y);
    minor = (mag_x <  mag_y ? mag_x : mag_y);

    writeword(major,OP_DIM1);/*put length of major axis in Dimension1 Reg */
    errterm = 2*minor - major;
    bresk1  = 2*minor;
    bresk2  = 2*(minor - major);

    /* set octant code */
    if (mag_x < mag_y)
    { octant = 0x01;
    }
    else
    { octant = 0x00;
    }
    if (dy < 0)
    { octant += 0x02;
    }
    if (dx < 0)
    { octant += 0x04;
    }

    writelword(errterm, BRES_ERR_TERM);
    writelword(bresk1,  BRES_K1);
    writelword(bresk2,  BRES_K2);
    writeword(xo, DESTINATION_X_PTR);
    writeword(yo, DESTINATION_Y_PTR);
    rc=set_por(mask,dm,octant,patt,source,dest,bs,fs,step); FUSE;
    rc=copdone(); FUSE;
  } while (FALSE); /* do once, breaking on any error */
  if (rc > ERR_LEVEL)
  { CatErr(BRES | SUBTU_FAIL);
    return(BRES | SUBTU_FAIL);
  }
  else
  { CatErr(BRES | GOOD_OP);
    return(BRES | GOOD_OP);
  }
}
