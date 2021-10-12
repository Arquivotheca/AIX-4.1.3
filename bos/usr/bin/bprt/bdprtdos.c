static char sccsid[] = "@(#)16	1.1  src/bos/usr/bin/bprt/bdprtdos.c, libbidi, bos411, 9428A410j 8/27/93 09:56:46";
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: BidiPrint
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
/*   SOURCE FILE NAME: BDPRTDOS.C <Bidirectional Printer Processing in DOS>   */
/*                                                                            */
/*   FUNCTION: This file contains the main module of Bidirectional printer    */
/*             stream processing.                                             */
/*                                                                            */
/*   ENTRY POINT:                                                             */
/*          BidiPrint                                                         */
/*                                                                            */
/******************************************************************************/

 #include <stdio.h>
 #include "bdprtdos.h"
 #include "prt-proc.h"
 #include "prt-cfg.h"
 #include "prt-c-em.h"
  
 #define FALSE  0
 #define TRUE   1

 PBDInterface BDI;

 unsigned BidiPrint (PBDInterface PBDI )  /* Bidi Interface Function */
 { unsigned rc = RC_OK ;
   char  i;


char *tmp;


    BDI = PBDI;

    switch (BDI->Mode) {
    case _OPEN     : rc=LoadCFG ( BDI->CFG  );
                     ProcessorInitialize();
                     break;

    case _PROCESS  :

                         Processor ();
                     break;

    default:
                rc = RC_UNKNOWN_COMMAND ;

    } /* endswitch */

    return rc;
 }

