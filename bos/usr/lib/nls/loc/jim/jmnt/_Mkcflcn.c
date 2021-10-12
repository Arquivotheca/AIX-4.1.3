static char sccsid[] = "@(#)20	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mkcflcn.c, libKJI, bos411, 9428A410j 7/23/92 03:23:21";
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
 * MODULE NAME:         _Mkcflcn()
 *
 * DESCRIPTIVE NAME:    Return First/Last Candidate
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
 * FUNCTION:            Call KKC-routine to get first/last candidate.
 *                      Call Post-KKC subroutine to update parameters in KMISA
 *                      in accordance with the result of KKC-routine.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        692 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mkcflcn()
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mkcflcn (pt,md)
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
 *                                               to the specified position
 *                                               of the string.
 *                              _Kcrtfcn()      :KKC-routine, which get first
 *                                               priority candidate.
 *                              _Kcrtlcn()      :KKC-routine, which get last
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
 *                              NA.
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
 *                              KKCMAJOR:Return KKC Major Error Code.
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
int _Mkcflcn (pt,md)

KCB     *pt;            /* Pointer to Kanji Control Block.              */
char    md ;            /* Conversion mode    (Next or Previous)        */
{
        register KMISA  *kjsvpt  ;      /* Pointer to Internal Save Area*/

CPRINT (--- _Mkcflcn ---) ;
snap3 (SNAP_KMISA,SNAP_Mkcflcn,"== Start _Mkcflcn ==") ;

       /** 0
        ** Set internal parameters
        **/
        kjsvpt = pt->kjsvpt ;

       /** 1
        ** Call KKC-routine
        **/
        if (md == M_KKCNXT)  /* Call First candidate */
                kjsvpt->kkcrc = _Kcrtfcn (kjsvpt->kkcbsvpt) ;
        else                 /* Call Last candidate */
                kjsvpt->kkcrc = _Kcrtlcn (kjsvpt->kkcbsvpt) ;

        /* Set previous curr. conversion position for next _Mkcnxpr() calling */
        kjsvpt->preccpos = kjsvpt->cconvpos ;

        if (KKCMAJOR(kjsvpt->kkcrc)>=K_KCLOGE) /* Check KKC return code */
        {
snap3 (SNAP_KMISA,SNAP_Mkcflcn,"== Return _Mkcflcn (KKC ERR) ==") ;
                return(IMFAIL) ;
        }

        /** 2
         ** Call Post-KKC subroutine
         **/
         _Mnxprps(pt) ;

        /** E
         ** Return successfully
         **/
snap3 (SNAP_KMISA,SNAP_Mkcflcn,"== Return _Mkcflcn (SUCC) ==") ;
        return(IMSUCC) ;
}
