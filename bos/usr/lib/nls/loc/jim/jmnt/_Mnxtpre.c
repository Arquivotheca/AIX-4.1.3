static char sccsid[] = "@(#)36	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mnxtpre.c, libKJI, bos411, 9428A410j 7/23/92 03:24:16";
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
 * MODULE NAME:         _Mnxtpre()
 *
 * DESCRIPTIVE NAME:    Next/Pre-Candidate Conversion KKC Interface Process
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
 * FUNCTION:            Set input parameter and call KKC-routine to get
 *                      next/previous candidate. If there is no more candidate,
 *                      get "Yomi" corresponds to the specified phrase.
 *                      Call Post-KKC subroutine to update parameters in KMISA
 *                      in accordance with the result of KKC-routine.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1080 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mnxtpre()
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mnxtpre (pt,md)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *                      md      :Conversion Mode
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
 *                              _Mnxprps()      :Post-KKC subroutine, which
 *                                               update KMISA in accordance
 *                                               with the output of KKC-routine.
 *                              _Mrscvym()      :Get the "yomi" corresponds
 *                                               to specified phrase.
 *                              _Mkjgrst()      :Set input parameters of KKC-routine
 *                                               to KKCB from corresponded
 *                                               parameters in KMISA.
 *                              _Mkanagt()      :Get "kanadata" corresponds
 *                                               to the specified position
 *                                               of the string.
 *                              _Kcnxtcv()      :KKC-routine, which get next
 *                                               priority candidate.
 *                              _Kcprvcv()      :KKC-routine, which get previous
 *                                               priority candidate.
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
 *                              Modification.
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

int _Mnxtpre (pt,md)

KCB     *pt;            /* Pointer to Kanji Control Block.              */
char    md ;            /* Conversion mode    (Next or Previous)        */
{
        register KMISA  *kjsvpt  ;      /* Pointer to Internal Save Area*/
        register KKCB   *kkcbsvpt;      /* Pointer to KKC Control Block */
                 short  knpos=0  ;      /* Position on kanamap in KMISA */
                 short  knlen=0  ;      /* Length of kana               */
                 short  jilen=0  ;      /* Length of kana for Jiritsugo */
                 short  phlen=0  ;      /* Length of phrase             */

CPRINT (--- _Mnxtpre ---) ;
snap3 (SNAP_KMISA|SNAP_KKCB,SNAP_Mnxtpre,"== Start _Mnxtpre ==") ;

        /** 0
         ** Set internal parameters
         **/
        kjsvpt = pt->kjsvpt ;
        kkcbsvpt = kjsvpt->kkcbsvpt ;

        /** 1
         ** Set input parameters of KKC-routine
         **/
        /* Set input parameters except Kana to KKCB for KKC-routine */
        _Mkjgrst (pt,M_CCDATA) ;

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
        if (md == M_KKCNXT) /* Call Next candidate */
                kjsvpt->kkcrc = _Kcnxtcv (kkcbsvpt) ;
        else                /* Call Previous candidate */
                kjsvpt->kkcrc = _Kcprvcv (kkcbsvpt) ;

        /* Set previous curr. conversion position for next _Mkcnxpr() calling */
        kjsvpt->preccpos = kjsvpt->cconvpos ;

        /* Check return code of KKC */
        if ((kjsvpt->kkcrc)==K_KCNMCA)  /* if No more candidate */
        {       _Mrscvym (pt)  ;        /* Restore Yomi of the Phrase */
snap3 (SNAP_KMISA|SNAP_KKCB,SNAP_Mnxtpre,"== Return _Mnxtpre (NO MORE C.) ==");
                return(IMSUCC) ;
        }
        if ((kjsvpt->kkcrc)>=K_KCNFCA)  /* if KKC error */
        {
snap3 (SNAP_KMISA|SNAP_KKCB,SNAP_Mnxtpre,"== Return _Mnxtpre (KKC ERR) ==") ;
                return(IMFAIL) ;
        }

        /** 3
         ** Call Post-KKC subroutine
         **/
        _Mnxprps(pt)  ;

        /** E
         ** Return Code
         **/
snap3 (SNAP_KMISA|SNAP_KKCB,SNAP_Mnxtpre,"== Return _Mnxtpre (SUCC) ==") ;
        return(IMSUCC) ;
}
