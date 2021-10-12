static char sccsid[] = "@(#)22	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mkcnxpr.c, libKJI, bos411, 9428A410j 7/23/92 03:23:27";
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
 * MODULE NAME:         _Mkcnxpr()
 *
 * DESCRIPTIVE NAME:    Next/Pre-Candidate Conversion Main Process
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
 * FUNCTION:            Call Next/Pre-conversion subroutines correspond to
 *                      the specified phrase, in accordance with the content
 *                      of *kjcvmap in KMISA.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1056 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mkcnxpr()
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mkcnxpr (pt,md)
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
 *                              _Mnxtpre()      :Call KKC-routines to get
 *                                               next/previous candidate.
 *                                               Call Post-KKC subroutines.
 *                              _Mkcflcn()      :Call KKC-routines to get
 *                                               first/last candidate.
 *                                               Call Post-KKC subroutines.
 *                              _Mnxtopn()      :Call KKC-routines for
 *                                               nex/pre conversion open.
 *                              _Mrscvym()      :Get the "yomi" corresponds
 *                                               to specified phrase.
 *                              _Mrsstrh()      :Reset the string, highlight
 *                                               attribute, etc.
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
#include <memory.h>     /* Memory Operation Package.                    */

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

int _Mkcnxpr (pt,md)
KCB     *pt;            /* Pointer to Kanji Control Block.              */
char    md ;            /* Conversion mode    (Next or Previous)        */
{
        register KMISA  *kjsvpt  ;      /* Pointer to Internal Save Area*/
                 char   cvstatus ;      /* Conversion status            */

CPRINT (--- _Mkcnxpr --- ) ;
snap3 (SNAP_KMISA,SNAP_Mkcnxpr,"== Start _Mkcnxpr ==") ;

        /** 1
         ** Set internal parameters
         **/
        kjsvpt = pt->kjsvpt ;

        /** 2
         ** Select subroutine
         **/
        /* Get conversion status at curr. conversion position */
        cvstatus = *(kjsvpt->kjcvmap + kjsvpt->cconvpos - kjsvpt->convpos + 1) ;

        switch (cvstatus) /* Select subroutine according to the status */
        {
                /*
                 Converted Kanji
                 */
                case  M_KSCNVK : /* Call Next-open routine if necessary */
                                 if((kjsvpt->kkcflag==M_KKNOP) ||
                                    (kjsvpt->preccpos!=kjsvpt->cconvpos))
                                        _Mnxtopn(pt) ;

                                 if (KKCPHYER(kjsvpt->kkcrc))
                                        break;

                                 /* Call Next/Previous candidate routine */
                                 _Mnxtpre (pt,md) ;
                                 break;

                /*
                 Yomi,once converted
                 */
                case  M_KSCNVY : /* Call Next-open routine if necessary */
                                 if((kjsvpt->kkcflag==M_KKNOP) ||
                                    (kjsvpt->preccpos!=kjsvpt->cconvpos))
                                        _Mnxtopn(pt) ;

                                 if (KKCPHYER(kjsvpt->kkcrc))
                                        break;

                                 /* Call First/Last candidate routine */
                                 _Mkcflcn (pt,md) ;
                                 break ;

                /*
                 KANJI set by Single Kanji Conversion
                 */
                case  M_KSCNSK : /* Call Yomi-restoring routine */
                                 _Mrscvym (pt) ;
                                 break ;

                /*
                 Yomi reset from Single Kanji Conversion
                 */
                case  M_KSCNSY : /* Call Next-open routine */
                                 _Mnxtopn (pt) ;

                                 if (KKCPHYER(kjsvpt->kkcrc))
                                        break;

                                 /* Call First/Last candidate routine */
                                 _Mkcflcn (pt,md) ;
        }

        /** 3
         ** Check R.C. of KKC
         **/
        if (kjsvpt->kkcrc > K_KCSUCC)
        {
snap3 (SNAP_KMISA,SNAP_Mkcnxpr,"== Return _Mkcnxpr (KKC ERR) ==") ;
                return (IMFAIL) ;
        }

        /** 4
         ** Change String, highlight attribute, etc.
         **/
        _Mrsstrh (pt) ; /* Call string & highlight-attribute update routine */

        /** E
         ** Return successfully
         **/
snap3 (SNAP_KMISA,SNAP_Mkcnxpr,"== Return _Mkcnxpr (SUCC) ==") ;
        return(IMSUCC) ;
}
