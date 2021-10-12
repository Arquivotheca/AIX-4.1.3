static char sccsid[] = "@(#)26	1.1  src/bos/usr/bin/bprt/prt-cfg.c, libbidi, bos411, 9428A410j 8/27/93 09:57:10";
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: LoadCFG
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/***************************** SOURCE FILE HEADER *****************************/
/*                                                                            */
/*   SOURCE FILE NAME: PRT-CFG.C  <Configuration file loader>                 */
/*                                                                            */
/*   FUNCTION: This unit (collection of routines) has the function of loading */
/*             printer specific information from (*.CFG) files.        This   */
/*             information is later used in processing the SFN data stream.   */
/*                                                                            */
/*                                                                            */
/*   ENTRY POINTS:                                                            */
/*               LoadCFG                                                      */
/*                                                                            */
/******************************************************************************/

#include <stdio.h> 
#include "prt-cfg.h" 
#include "prt-c-em.h"
#include "bdprtdos.h"

_CFG ALL_CFGs [MaxCFG] = {
      #include "4019.h"     /* new CFG - to be adjusted */ 
             , 
      #include "4201.h"   
             ,    
      #include "4202.h" 
             ,             
      #include "4207.h" 
             , 
      #include "4208.h" 
             , 
      #include "4216.h"
             ,
      #include "4234.h"       /* new CFG - to be adjusted */
             , 
      #include "5201.h"
             , 
      #include "5202.h"
             , 
      #include "5204.h"
             , 
      #include "3812.h"
             , 
      #include "4224.h"
}
;  

_PCFG  CFG;

unsigned short *PsmTable;    /* to point to current psm table used, in case */
                             /* psm table of cfg is not currently active.   */


 unsigned int  LoadCFG   ( int CFGSpec )
 {
   if  (CFGSpec<=MaxCFG)  {
           CFG = &ALL_CFGs[CFGSpec];
           return RC_OK;
        }
   else {
           CFG = ALL_CFGs;
           return ( RC_CFG_NOT_FOUND );
        }
 }  
