static char sccsid[] = "@(#)25	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kudicdlc.c, cmdKJI, bos411, 9428A410j 7/23/92 01:09:29";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kudicdlc
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
 * MODULE NAME:         kudicdlc
 *
 * DESCRIPTIVE NAME:    Kanji User Dictionary Delete Control
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
 * FUNCTION:            Kanji User Dictionary Delete Control
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1056 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kudicdlc
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kudicdlc( lastpos,udcbptr )
 *
 *  INPUT:              lastpos  :  Pointer Last Dictionary Buffer
 *                      udcbptr  :  Pointer to User Dictionary Control Block
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      NA.
 *
 * EXIT-ERROR:          Return Codes Returned to Caller.
 *                      NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              kudicymc
 *                              kudicdlp
 *                              kudcmrud
 *
 *                      Standard Liblary.
 *                              memcmp
 *                              memcpy
 *
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
/*#include <memory.h>*/ /*                                              */

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
void   kudicdlc ( lastpos, udcbptr )

UDCS    *lastpos;       /* Pointer Last Dictionary Buffer                    */
UDCB    *udcbptr;       /* Pointer to User Dictionary Control Block          */

{
    void    kudicymc();          /* PC Code -> 7bit Code Transfer Function   */
    void    kudicdlp();          /* Dictionary Data Delete Function          */
    void    kudcmrud();          /* MRU Data Delete Function                 */
    int     i;                   /* Loop Counter.                            */
    UDCS    *pt;                 /* Buffer Struct Pointer                    */
    short   mode,                /* Buffer Status Mode                       */
            kanalen,             /* Yomi Length                              */
            kjlen;               /* Kanji Length                             */
    uchar   *kanadata,           /* Pointer to Yomi Data String              */
            pos;                 /* Dictionary Data Position                 */
    uchar   kncv[U_KN_L];        /* Yomi ( 7bits Code )                      */
    short   kncvlen;             /* Yomi Length ( 7bits Code )               */
    short   mflag;               /* Type Flag                                */
    uchar   kjcv[U_KJ_L];        /* Kanji ( 7bits code )                     */
    uchar   *kjdata;             /* Kanji                                    */
    uchar   yomi_d[U_KN_L];      /* Previous Yomi Data                       */
    short   yomi_l,              /* Previous Yomi Data Length                */
            yomi_f;              /* Yomi Delete Flag                         */

    /*
     *      User Dictionary Data Delete
     */
    for ( pt = lastpos ; pt != NULL ; pt = pt->pr_pos )
      if ( pt->status == U_S_YOMD || pt->status == U_S_KNJD
                                  || pt->status == U_S_KNJU )
        {
        /*   Buffer Data Set     */
                                     /* Set Delete Mode                      */
        mode = ( pt->status == U_S_YOMD ) ? U_S_YOMD : U_S_KNJD;
        kanadata = pt->yomi;         /* Set Yomi Data                        */
        kanalen  = pt->yomilen;      /* Set Yomi Length                      */
        kjdata   = pt->kan;          /* Set Kanji Data                       */
        kjlen    = pt->kanlen;       /* Set Kanji Length                     */
        pos      = pt->pos;          /* Set Kanji Posituon                   */

        yomi_f = TRUE;               /* Delete Flag Set                      */
        if ( mode == U_S_YOMD )      /* Data Delete Check                    */
          {                          /* Previous Yomi Data Compare           */
          if ( yomi_l == kanalen &&
               memcmp ( yomi_d, kanadata, kanalen ) == NULL )
             yomi_f = FALSE;         /* Delete Flag Reset                    */
                                     /* Yomi Data Save Area Clear            */
          for ( i = 0; i < U_KN_L; i++ )  yomi_d[i] = NULL;
                                     /* Current Yomi Delete Data String Save */
          memcpy ( yomi_d, kanadata, kanalen );
          yomi_l = kanalen;          /* Current Yomi Data String Length Save */
          }

        if ( yomi_f == TRUE )
          {                           /* Yomi Transfer (PC Code->7 bit Code) */
          kudicymc( kanadata, kanalen, kncv, &kncvlen, &mflag );
                                      /* Dictionary Data Delete              */
          kudicdlp( mode, kncv, kncvlen, pos, udcbptr );
          if ( mode == U_S_YOMD )     /* Data Delete Check                   */
            {                         /* Kanji Data Work Area Copy           */
            memcpy( kjcv, kjdata, kjlen );
                                      /* Kanji Data First bit Off            */
            for ( i = 0; i < kjlen - U_1KJ_L; i += U_1KJ_L )
               kjcv[i] &= U_MSB_O;
                                      /* MRU Data Delete                     */
            kudcmrud( mode, kncv, kncvlen, kjcv, kjlen, udcbptr );
            }
          }
       }
    return;
}
