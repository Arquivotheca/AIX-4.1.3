static char sccsid[] = "@(#)14	1.2  src/bos/diag/tu/sky/p510.c, tu_sky, bos411, 9428A410j 10/29/93 13:40:55";
/*
 *   COMPONENT_NAME : (tu_sky) Color Graphics Display Adapter Test Units
 *
 *   FUNCTIONS: set_por
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
* Name      : set_por                                     *
* Function  : Load the Pixel Operation Register           *
* Called by : testxy                                      *
* Calls     : writeword, getbaseadd                       *
* PreReqs   : tinfo.setup.cardnum must be set             *
*                                                         *
**********************************************************/
set_por(mask,dm,oct,patt,source,dest,bs,fs,step) 
byte bs;    /* Background Source- set to BGC or SPM */
byte fs;    /* Foreground Source- set to FGC or SPM */
byte step;  /* Step Function    - set to DSR,LDR,DSW,LDW,PXBLT,IPXBLT,FPXBLT */
byte source;/* Source Pixel Map - set to MAPA,MAPB OR MAPC */
byte dest  ;/* Destination Pixel Map - set to MAPA,MAPB OR MAPC */
byte patt  ;/* Pattern Pixel Map - set to MAPA,MAPB,MAPC,FIXED_PATTERN,GFS_PATTERN */
byte mask;  /* Mask  Pixel Map - set to MMD,MMBE, or MME */
byte dm;    /* Drawing Mode    - set to DAP,DFPN,LPN,DAB */
byte oct;   /* Direction Octant */        
{
  lword por_bits; /* assembled bit fields */
  lword testval;
  word  source_sz,destin_sz;

  GETBASE
  /* make sure data is valid */
   if (bs != BGC && bs != SPM)
   { CatErr( SET_POR | CPF_INVDAT);
     sprintf(tinfo.info.msg_buff,"Illegal Background Source sent to set_por routine: 0x%X", bs);
     return( SET_POR | CPF_INVDAT);
   }
   if (fs != FGC && fs != SPM)
   { CatErr( SET_POR | CPF_INVDAT);
     sprintf(tinfo.info.msg_buff,"Illegal Foreground Source sent to set_por routine: 0x%X", fs);
     return( SET_POR | CPF_INVDAT);
   }
   if ((step > FPXBLT) || (step < DSR) || ((step < PXBLT) && (step > LDW)))
   { CatErr( SET_POR | CPF_INVDAT);
     sprintf(tinfo.info.msg_buff,"Illegal Step Function sent to set_por routine: 0x%X", step);
     return( SET_POR | CPF_INVDAT);
   }
   if (source  > MAPC)/* || source < MAPA)*/
   { CatErr( SET_POR | CPF_INVDAT);
     sprintf(tinfo.info.msg_buff,"Illegal Source Pixel Map sent to set_por routine: 0x%X", source);
     return( SET_POR | CPF_INVDAT);
   }
   if (dest  > MAPC ) /* || dest < MAPA) */
   { CatErr( SET_POR | CPF_INVDAT);
     sprintf(tinfo.info.msg_buff,"Illegal Destination Pixel Map sent to set_por routine: 0x%X",dest);
     return( SET_POR | CPF_INVDAT);
   }
   if  ((patt  > GFS_PATTERN) || (patt < /*MAPA*/MASK) || ((patt < FIXED_PATTERN) && (patt > MAPC)))
   { CatErr( SET_POR | CPF_INVDAT);
     sprintf(tinfo.info.msg_buff,"Illegal Foreground Source sent to set_por routine: 0x%X", fs);
     return( SET_POR | CPF_INVDAT);
   }
   if (mask  > MME)                         
   { CatErr( SET_POR | CPF_INVDAT);
     sprintf(tinfo.info.msg_buff,"Illegal Mask Siccoring Code sent to set_por routine: 0x%X", mask);
     return( SET_POR | CPF_INVDAT);
   }
   if (dm > DAB)
   { CatErr( SET_POR | CPF_INVDAT);
     sprintf(tinfo.info.msg_buff,"Illegal Drawing Mode sent to set_por routine: 0x%X",dm);
     return( SET_POR | CPF_INVDAT);
   }
   if  (oct > 7)
   { CatErr( SET_POR | CPF_INVDAT);
     sprintf(tinfo.info.msg_buff,"Illegal Octant sent to set_por routine: 0x%X", oct);
     return( SET_POR | CPF_INVDAT);
   }
  
   source_sz = Mapfmt(source); /* get pel size for this map */ 
   destin_sz = Mapfmt(dest); /* get pel size for this map */ 
   if ((source_sz != destin_sz) && (step != FPXBLT))
   { CatErr( SET_POR | CPF_INVDAT);
     sprintf(tinfo.info.msg_buff,"Source and Destination do not have the same Pixel size:");
     CatMsgs("\n--Source PEL Size Code = ");
     CatMsgx(source_sz);
     CatMsgs("\n--Destination PEL Size Code = ");
     CatMsgx(destin_sz);
     return( SET_POR | CPF_INVDAT);
   }
   
   if ((patt==MAPA) || (patt == MAPB) || (patt == MAPC) || (patt == MASK))
   { source_sz = Mapfmt(patt); /* get pel size for this map */ 
     if (source_sz != BPP1) 
     { CatErr( SET_POR | CPF_INVDAT);
       sprintf(tinfo.info.msg_buff,"Pattern Pixel Size Code (0x%X) should be 0x%X",source_sz,BPP1);
       return( SET_POR | CPF_INVDAT);
     }
   }

  /* all data appears to be valid - assemble bit fields in to a longword */
  por_bits = step + (0x10*fs) + (0x40*bs) + (0x100*dest) + (0x1000*source) + (0x100000*patt)
                 + (0x1000000*oct) + (0x10000000*dm) + (0x40000000*mask); 

  /* write to the pixel operations register */
  writelword(por_bits, PIX_OP_REG);
  return(GOOD_OP);
}
