static char sccsid[] = "@(#)41	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mrsstrh.c, libKJI, bos411, 9428A410j 7/23/92 03:24:36";
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
 * MODULE NAME:         _Mrsstrh()
 *
 * DESCRIPTIVE NAME:    Next/Pre-Candidate Conversion Post Process
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
 * FUNCTION:            Reset string, highlight attribute, cursor position
 *                      and current conversion length in Next/Pre-conversion
 *                      process.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        816 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mrsstrh()
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mrsstrh (pt)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
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
 *                              _Mexchng()      :Exchange the string to
 *                                               reference string in
 *                                               specified length.
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
 ********************* END OF MODULE SPECIFICATIONS ***********************/

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
int _Mrsstrh (pt)

KCB     *pt;            /* Pointer to Kanji Control Block.              */
{
        register KMISA  *kjsvpt  ;      /* Pointer to Internal Save Area*/
                 KKCB   *kkcbsvpt;      /* Pointer to KKC Control Block */
        register uchar  *hltpt   ;      /* Pointer to highlight attribute
                                                   where to be changed  */
        register short  i        ;      /* Loop counter                 */
                 uchar  *str     ;      /* Pointer to converted strings */
                 short  strlen   ;      /* Length of converted strings  */
                 short  hltend   ;      /* Length of highlight attribute
                                                   change area          */
                 short  pos      ;      /* Reference string position    */
                 short  prcvlen  ;      /* Previous conversion length   */
CPRINT (--- _Mrsstrh ---);
snap3 (SNAP_KCB|SNAP_KMISA, SNAP_Mrsstrh,"== Start _Mrsstrh ==");

        /** 0
         ** Set internal parameters
         **/
        kjsvpt = pt->kjsvpt ;
        kkcbsvpt = kjsvpt->kkcbsvpt ;
        hltpt = pt->hlatst + kjsvpt->cconvpos ; /* Pointer to high-light att.*/
                                                /* area corresponds to curr. */
                                                /* conversion position       */
        prcvlen = kjsvpt->convlen ;             /* conversion length before  */
                                                /* string changed            */

        /** 1
         ** Change string
         **/
        /* Set internal variables */
        str = kkcbsvpt->kjdata ;       /* Pointer to reference string area   */
        pos = 2 ;                      /* Offset due to area for length of   */
                                       /* reference string                   */
        strlen = kkcbsvpt->kjlen - 2 ; /* Length of reference string         */

        /* Call string chage routine */
        _Mexchng(pt,str,pos,strlen,kjsvpt->cconvpos,kjsvpt->cconvlen);

        /** 2
         ** In case Overflow Fixing
         **/
        /* Conver. length to be set to 0 in _Mexchng() if the newly changed  */
        /* string overflows the input field.                                 */

        if( kjsvpt->convlen == 0 )
        {    pt->curcol  = kjsvpt->cconvpos;    /* Set new cursor position  */
             kjsvpt->cconvlen= 0;               /* Set new curr.conv.length */
             kjsvpt->cconvpos= kjsvpt->curleft; /*Set new curr.conv.position*/
             _MM_rtn (pt,A_1STINP);      /* Change mode to First input mode */
snap3 (SNAP_KCB|SNAP_KMISA, SNAP_Mrsstrh,"== Return _Mrsstrh by overflow ==");
             return(IMSUCC) ;
        }

        /** 3
         ** Change highlight attribute
         **/

        /* Set high-light attr. of newly changed area to "REVERSE"    */
        for (i=0;i<strlen;i++)
        {       
                *hltpt = K_HLAT3 ;
                hltpt++ ;
        }

        /* Get the distance between the end of newly changed area and the end */
        /* of new conversion length .                                         */
        hltend = kjsvpt->convpos + kjsvpt->convlen - kjsvpt->cconvpos -
                 strlen ;

        /* Set high-light attr. of above mentioned area to "UNDERLINE"  */
        for (i=0;i<hltend;i++)
        {       
                *hltpt = K_HLAT2 ;
                hltpt++ ;
        }

        /* Reset high-light attr. of unnecessary area to "NULL"        */
        for (i=0;i< prcvlen - kjsvpt->convlen;i++)
        {
                *hltpt = K_HLAT0 ;
                hltpt++ ;
        }

        /** 4
         ** Change cursor position
         **/
        if (pt->curcol == kjsvpt->cconvpos + kjsvpt->cconvlen)
                pt->curcol = kjsvpt->cconvpos + strlen ;
        else
                pt->curcol = kjsvpt->cconvpos + strlen - C_DBCS ;

        /** 5
         ** Change current conversion length
         **/
        kjsvpt->cconvlen = strlen ;

        /** E
         ** Return Code
         **/
snap3 (SNAP_KCB|SNAP_KMISA, SNAP_Mrsstrh,"== Return _Mrsstrh ==");
        return(IMSUCC) ;
}
