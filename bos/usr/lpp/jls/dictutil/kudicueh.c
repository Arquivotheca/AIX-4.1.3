static char sccsid[] = "@(#)32	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kudicueh.c, cmdKJI, bos411, 9428A410j 7/23/92 01:22:21";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kudicueh
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:         kudicueh
 *
 * DESCRIPTIVE NAME:    Kanji User Dictionary End Update Handler
 *
 * COPYRIGHT:           5756-030 COPYRIGHT IBM CORP 1991
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              User Dictionary Maintenance for AIX 3.2
 *
 * CLASSIFICATION:      OCO Source Material - IBM Confidential.
 *                      (IBM Confidential-Restricted when aggregated)
 *
 * FUNCTION:            Kanji User Dictionary End Update(Delete & Add) Handler
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        868 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kudicueh
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kudicueh ( frstpos, lastpos, udcbptr, errpos )
 *
 *  INPUT:              frstpos  : Pointer to First Dictionary Buffer
 *                      lastpos  : Pointer to Last Dictionary Buffer
 *                      udcbptr  : Pointer to User Dictionary Control Block
 *
 *  OUTPUT:             errpos   : Pointer to Pointer to Error Dictionary Buffer
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                              IUSUCC   :  Process Successful
 *
 * EXIT-ERROR:          Wait State Codes.
 *                              IUFAIL   :  kudicadp Error
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              kudicdlc
 *                              kudicymc
 *                              kudicadp
 *
 *                      Standard Liblary.
 *                              NA.
 *                      Advanced Display Graphics Support Liblary(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      NA.
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Liblary.
 *                              IDENTIFY:Module Identify Create.
 *                      Standard Macro Liblary.
 *                              NA.
 *
 * CHANGE ACTIVITY:     NA.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */

/*
 *      include Standard.
 */
#include <stdio.h>      /* Standard I/O Package.                        */

/*
 *      include Kanji Project.
 */
#include "kut.h"        /* Kanji user dictionary utility                */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-125 COPYRIGHT IBM CORP 1989           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Please Descripte This Module.
 */
int     kudicueh ( frstpos, lastpos, udcbptr, errpos )

UDCS    *frstpos;       /* Pointer to First Dictionary Buffer                */
UDCS    *lastpos;       /* Pointer to Last Dictionary Buffer                 */
UDCB    *udcbptr;       /* Pointer to User Dictionary Control Block          */
UDCS   **errpos;        /* Pointer to Pointer to Error Dictionary Buffer     */

{
    void    kudicdlc();     /* User dictionary Data Delete Function          */
    void    kudicymc();     /* PC code -> 7bit Code Transfer Function        */
    int     kudicadp();     /* User Dictionary Data Regist Function          */
    void    kudcmrua();     /* User Dictionary MRU Area Regist Function      */
    int     rc;             /* Return Code.                                  */
    UDCS    *pt;            /* Pointer to Dictionary Buffer Work             */
    short   mode,           /* Delete Mode                                   */
            kanalen,        /* Yomi Data Length                              */
            kjlen,          /* Kanji Data Length                             */
            kncvlen,        /* Yomi Length ( 7Bits Code )                    */
            mflag;          /* kudicymc Return Flag                          */
    uchar   *kanadata,      /* Yomi Data                                     */
            *kjdata,        /* Kanji Data                                    */
            kncv[U_KN_L],   /* Yomi ( 7Bits Code )                           */
            l_knln,         /* MRU Delete Yomi Data Length Area              */
            l_kjln;         /* MRU Delete Kanji Data Length Area             */
    /*
     *      Initialize Return Code
     */
    rc  =  IUSUCC;

    /*
     *     User Dictionary Delete
     */
    kudicdlc( lastpos, udcbptr );

    /*
     *     User Dictionary Add
     */
    for ( pt = frstpos ; pt != NULL ; pt = pt->nx_pos )
        if ( pt->status == U_S_YOMA || pt->status == U_S_KNJU )
            {
            mode     = U_REGIST;        /* Set Registration Mode             */
            kanadata = pt->yomi;        /* Set Yomi Data String              */
            kanalen  = pt->yomilen;     /* Set Yomi Data String Length       */
            kjdata   = pt->kan;         /* Set Kanji Data String             */
            kjlen    = pt->kanlen;      /* Set Kanji Data String Length      */
                                        /* Kana PC Code -> 7 Bit Code        */
            kudicymc( kanadata, kanalen, kncv, &kncvlen, &mflag );
                                   /* Dictionary Data Add Function           */
            rc = kudicadp( mode, kncv, kncvlen, kjdata, kjlen, udcbptr );
                                   /* Dictionary Registration Error          */
            if ( rc != IUSUCC )
                *errpos = pt;      /* Set Pointer to Error Dictionary Buffer */
            if ( rc != IUSUCC  &&  rc != UDOVFDLE )
                break;             /* Break for Loop Process is Terminated   */
            if ( rc == IUSUCC )    /* User Dictionary MRU Data Add Function  */
               {
               l_knln = *((uchar *)(&kncvlen) + 1);  /* Set Yomi Length  */
               l_kjln = *((uchar *)(&kjlen) + 1);    /* Set Kanji Length */
               kudcmrua ( udcbptr->dcptr, kncv, l_knln, kjdata, l_kjln );
               }
            }

    return( rc );
}
