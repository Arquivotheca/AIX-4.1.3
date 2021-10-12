static char sccsid[] = "@(#)77	1.2  src/bos/diag/tu/swmono/p511.c, tu_swmono, bos411, 9428A410j 10/29/93 14:20:19";
/*
 *   COMPONENT_NAME : (tu_swmono) Grayscale Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: set_dsr
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
* Name      : set_dsr                                     *
* Function  : Load the Direction Step Register            *
*               enough times to move or draw the specified*
*               number of steps                           *
* Called by : testxy                                      *
* Calls     : writeword, getbaseadd, writelword           *
* PreReqs   : tinfo.setup.cardnum must be set             *
*             set_por() must have been run to set up the  *
*               pixel operation register for draw and step*
*               operations.                               *
**********************************************************/
set_dsr(dir, m_d, steps)
word steps;  /*number of steps to take */
byte   m_d;  /* move(0) or draw(1) */
byte   dir;  /* 0 for 0   degrees, 1 for 45,  2 for 90,  3 for 135 */ 
             /* 4 for 180 degrees, 5 for 225, 6 for 170, 7 for 315 */ 
             /* or....                                             */
             /* EASTS,NES,NORTHS,NWS,WESTS,SWS,SOUTHS,SES          */
{ 
  word x, full, rc;
  word partial;
  byte code[4];
  lword lcode;
  lword testval;

  GETBASE

  full    =  (steps/15);         /* 16 step segments (maximum step size) - full is int, not float */
  partial = ((steps/15.0) - full)*15.0; /* remainder of steps */          

  do 
  { for (x=0; x<=3; x++)  /* fill the lword with byte codes */
    { if (full)                        
      { code[x] = (0x0F) + (m_d * 16) + (dir * 32);
        full--;
      }
      else if (partial)
      { code[x] = (partial) + (m_d * 16) + (dir * 32);
        partial = 0;
      }
      else
      { code[x] = 0x00;
      }
    }

    /* write the lword containing the 4 codes to the Direction Step Register */
    /*          in motorola order                                            */
    lcode = code[0]*0x1000000 + code[1]*0x10000 + code[2]*0x100 + code[3];
    writelword(lcode, DIR_STEP_REG);
    rc=copdone(); FUSE;
  } while (full || partial);

  if (rc > ERR_LEVEL)
  { CatErr(SET_DSR | SUBTU_FAIL);
    return(SET_DSR | SUBTU_FAIL);
  } 
  else
  { CatErr(SET_DSR | GOOD_OP);
    return(SET_DSR | GOOD_OP);
  }
}
