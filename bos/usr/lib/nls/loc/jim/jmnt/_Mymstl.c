static char sccsid[] = "@(#)50	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mymstl.c, libKJI, bos411, 9428A410j 7/23/92 03:25:11";
/*
 * COMPONENT_NAME :	Japanese Input Method - Kanji Monitor
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         _Mymstl()
 *
 * DESCRIPTIVE NAME:    Fixing Post Process
 *
 * COPYRIGHT:           5601-061 COPYRIGHT IBM CORP 1988
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              Kanji Monitor V1.0
 *
 * CLASSIFICATION:      OCO Source Material - IBM Confidential.
 *                      (IBM Confidential - Restricted when aggregated)
 *
 * FUNCTION:            Reset highlight attribute, string save area and
 *                      conversion status for fixing process.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        660 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mymstl()
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mymstl ( pt, pos )
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *                      pos     :Fixing position
 *
 *  OUTPUT:             NONE
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      IMSUCC :Success of Execution.
 *
 * EXIT-ERROR:          Return Codes Returned to Caller.
 *                      IMFAIL :Failure of Execution.(Invalid input data)
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _Msetch()      :Set string change position
 *                                              and length
 *                              _Mmvch()       :Character movement in a
 *                                              dimemsion, then clear
 *                                              remaining area.
 *
 *                      Standard Library.
 *                              NA.
 *
 *                      Advanced Display Graphics Support Library(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      See Below.
 *
 *   INPUT:             DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Controle Block(KCB).
 *                              Reference
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              Reference
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Controle Block(KCB).
 *                              Modification
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              Modification
 *                      Trace Block(TRB).
 *                              NA.
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              IDENTIFY:Module Identify Create.
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     NA.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************/

/*
 *      include Standard.
 */
#include <stdio.h>      /* Standard I/O Package.                        */

/*
 *      include Kanji Project.
 */
#include "kj.h"         /* Kanji Project Define File.                   */
#include "kcb.h"        /* Kanji Control Block Structure.               */


/*
 *      Copyright Identify.
 */
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1988           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Please Descripte This Module.
 */
int _Mymstl (pt,pos)

KCB     *pt;            /* Pointer to Kanji Control Block.              */
short   pos;            /* Fixing Position                              */
{


        register KMISA  *kjsvpt  ;      /* Pointer to Internal Save Area*/
                 short  convpos  ;      /* Conversion position          */
        register uchar   *hltpt  ;      /* Pointer to highlight atribute
                                           to be changed                */
        register uchar   *savept ;      /* Pointer to string save area
                                           to be shifted                */
        register short  chnglen  ;      /* Length to be changed         */
        register short  i        ;      /* Loop counter                 */
        register short  lpend    ;      /* Loop end value               */
                 int    rc       ;      /* Return code                  */

CPRINT (--- _Mymstl ---) ;
snap3 (SNAP_KCB|SNAP_KMISA, SNAP_Mymstl, "== Start _Mymstl ==") ;

  /** 0
   ** Set internal parameters
   **/
        kjsvpt = pt->kjsvpt ;
        convpos = kjsvpt->convpos ;
        hltpt = pt->hlatst + convpos ; /* Pointer to high-light attr. area at */
                                       /* conversion position                 */
        chnglen = pos - convpos ;      /* Length of Fixing                    */
        rc = IMSUCC ;                  /* Return code                         */

  /** 1
   ** Change highlight attribute
   **/

        if (kjsvpt->kkmode1 != A_HIRKAT ) /* High-light atrr.not changed in   */
        {                                 /* Hira/Kata conv.mode by _Mymstl() */
             for (i=0;i<chnglen;i++)
             {       *hltpt = K_HLAT0 ;
                  hltpt++ ;
             };
  /** 2
   ** Set string changed position and length
   **/
         rc = _Msetch(pt,convpos,chnglen) ;
        };


  /** 3
   **Shift string save area and Reset string save length
   **/

        savept = kjsvpt->stringsv ; /* Pointer to string save area           */

        if ((lpend=(kjsvpt->savelen-=chnglen)) > 0)
                for (i=0;i<lpend;i++)
                {    *savept = *(savept + chnglen); /* Shift string save area */
                      savept ++ ;
                }
        else
                kjsvpt->savelen = 0 ; /* whole string save area unnecessary */

  /** 4
   ** Reset conversion position and conversion length
   **/

        if ((kjsvpt->convlen -= chnglen) <= 0) /* In case whole string      */
        {       pt->cnvsts = K_CONVF ;         /* are Fixed .               */
                kjsvpt->convlen = 0 ;
                kjsvpt->convpos = kjsvpt->curleft ;
        }

        else
                kjsvpt->convpos = pos ;

        kjsvpt->savepos = kjsvpt->convpos ;  /* Set new string save position */

  /** 5
   ** Reset length of KANA for HIRA/KATA conversion
   **/
        if (kjsvpt->kkmode1 == A_HIRKAT )
            kjsvpt->kanalen = 0 ;

  /** E
   ** Return Code
   **/
snap3 (SNAP_KCB|SNAP_KMISA, SNAP_Mymstl, "== Return _Mymstl ==") ;
        return(rc) ;
}
