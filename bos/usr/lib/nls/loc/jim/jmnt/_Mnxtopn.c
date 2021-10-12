static char sccsid[] = "@(#)35	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mnxtopn.c, libKJI, bos411, 9428A410j 7/23/92 03:24:11";
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
 * MODULE NAME:         _Mnxtopn()
 *
 * DESCRIPTIVE NAME:    Next/Pre-Candidate Conversion Open Process
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
 * FUNCTION:            Set input parameter and call KKC-routine to open next/previous
 *                      conversion.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        796 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mnxtopn()
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mnxtopn (pt)
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
 *                              _Mkjgrst()      :Set input parameters of KKC-routine
 *                                               to KKCB from corresponded
 *                                               parameters in KMISA.
 *                              _Mkanagt()      :Get "kanadata" corresponds
 *                                               to the specified position
 *                                               of the string.
 *                              _Kcnxtop()      :KKC-routine, which open next/previous
 *                                               conversion.
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
 *                              NA.
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
 *                              NA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              NA.
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
#include <memory.h>     /* Memory operation package                     */
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
int _Mnxtopn (pt)

KCB     *pt;            /* Pointer to Kanji Control Block.              */
{
        register KMISA  *kjsvpt  ;      /* Pointer to Internal Save Area*/
        register KKCB   *kkcbsvpt;      /* Pointer to KKC Control Block */
                 short  knpos=0  ;      /* Position on kanamap in KMISA */
                 short  knlen=0  ;      /* Length of kana               */
                 short  jilen=0  ;      /* Length of kana for Jiritsugo */
                 short  phlen=0  ;      /* Length of phrase             */

CPRINT (--- _Mnxtopn ---) ;
snap3 (SNAP_KMISA|SNAP_KKCB,SNAP_Mnxtopn,"== Start _Mnxtopn ==") ;

        /** 0
         ** Set internal parameters
         **/
        kjsvpt = pt->kjsvpt ;
        kkcbsvpt = kjsvpt->kkcbsvpt ;

        /** 1
         ** Set input parameters to KKCB for KKC-routine
         **/
        /* Set input parameters except Kana to KKCB for KKC-routine */
        _Mkjgrst (pt,M_1SDATA) ;

        /* Get kana corresponds to the phrase */
        _Mkanagt (pt,kjsvpt->cconvpos - kjsvpt->convpos,&knpos,&knlen,&jilen,
                  &phlen) ;

        /* Set length of the Kana to KKCB */
        kkcbsvpt->kanalen2 = kkcbsvpt->kanalen1 = knlen ;

        /* Set the Kana string to KKCB */
        memcpy(kkcbsvpt->kanadata,(kjsvpt->kanadata + knpos),knlen) ;

        /** 2
         ** Call KKC-routine
         **/
        /* Call KKC NEXT-OPEN routine */
        kjsvpt->kkcrc = _Kcnxtop (kkcbsvpt) ;

        /* Check return code of KKC */
        if (kjsvpt->kkcrc >= K_KCNFCA)
        {
snap3 (SNAP_KMISA|SNAP_KKCB,SNAP_Mnxtopn,"== Return _Mnxtopn (KKC ERR) ==") ;
                return(IMFAIL) ;
        }

        /** 3
         ** Set KKC-NXT/PREV Open Flag
         **/
        kjsvpt->kkcflag = M_KKOPN ;

        /** E
         ** Return Code
         **/
snap3 (SNAP_KMISA|SNAP_KKCB,SNAP_Mnxtopn,"== Return _Mnxtopn (SUCC) ==") ;
        return(IMSUCC) ;
}
