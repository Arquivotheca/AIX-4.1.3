static char sccsid[] = "@(#)24	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kudicdlb.c, cmdKJI, bos411, 9428A410j 7/23/92 01:09:03";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kudicdlb
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
 * MODULE NAME:         kudicdlb
 *
 * DESCRIPTIVE NAME:    User Dictionary Buffer Delete Status Code Set
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
 * FUNCTION:            User Ditionary Buffer Set to be Delete Status Code
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        792 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kudicdlb
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kudicdlb( dmappt, fldno )
 *
 *  INPUT:              dmappt   : Pointer to Display Map
 *                      fldno    : Field No to be Delete
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                              NA.
 *
 * EXIT-ERROR:          Return Codes Returned to Caller.
 *                              NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              NA.
 *                      Standard Liblary.
 *                              memcpy
 *                              memcmp
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
void   kudicdlb ( dmappt, fldno )

UDMS    *dmappt;        /* Pointer to Display Map                            */
short    fldno;         /* Field No to be Delete                             */

{
    UDCS    *pt;            /* Pointer to Current Dictonary Buffer           */
    UDCS    *w_pt;          /* Pointer to Work Dictonary Buffer              */
    short   stat;           /* Field Status                                  */
    uchar   ymdt[U_B_YOMI]; /* Yomi Data String Save Area                    */
    uchar   ymln;           /* Yomi Data String Length Save Area             */

    /*
     *   Data Get from Display Map
     */
    stat = dmappt->fld[fldno].fstat;    /* Get Field Status     */
    pt   = dmappt->fld[fldno].dbufpt;   /* Get Buffer Pointer   */
    /*
     *   Goku Field Data Process
     */
     if ( stat == U_GOKUF )
        {                                     /* Goku Field Process          */
        if ( pt->status == U_S_YOMA )         /* Status Code Set Goku Delete */
           pt->status = U_S_ADEL;
        else
           pt->status = U_S_KNJD;
        }
    /*
     *   Yomi Field Data Process
     */
     else                                     /* Yomi Field Process          */
        {                                     /* Input Yomi Field Data Save  */
        ymln = pt->yomilen;                   /* Yomi Length Save            */
        memcpy ( ymdt, pt->yomi, (int)ymln ); /* Yomi Data String Save       */
       /*
        *  Yomi Data Check
        */
        if ( pt->status == U_S_YOMA )         /* Input Yomi Status Set       */
           pt->status = U_S_ADEL;             /* Set Delete Status Code      */
        else
           pt->status = U_S_YOMD;             /* Set Delete Status Code      */
        if ( pt->nx_pos != NULL )
           for ( w_pt = pt->nx_pos; w_pt != NULL; w_pt = w_pt->nx_pos )
              {                       /* Search for Same Yomi for Next       */
              if ( ymln != w_pt->yomilen ||   /* Check Yomi Data             */
                   memcmp ( ymdt, w_pt->yomi, (int)ymln ) != NULL )
                 break;                       /* Defference Yomi Search End  */
              if ( w_pt->status != U_S_YOMD   /* Check Status Code           */
               &&  w_pt->status != U_S_KNJD  &&  w_pt->status != U_S_ADEL )
                 if ( w_pt->status == U_S_YOMA )
                    w_pt->status = U_S_ADEL;  /* Set Delete Status Code      */
                 else
                    w_pt->status = U_S_YOMD;  /* Set Delete Status Code      */
              }
        if ( pt->pr_pos != NULL )
           for ( w_pt = pt->pr_pos; w_pt != NULL; w_pt = w_pt->pr_pos )
              {                       /* Search for Same Yomi for Previous   */
              if ( ymln != w_pt->yomilen ||   /* Check Yomi Data             */
                   memcmp ( ymdt, w_pt->yomi, (int)ymln ) != NULL )
                 break;                       /* Defference Yomi Search End  */
              if ( w_pt->status != U_S_YOMD   /* Check Status Code           */
               &&  w_pt->status != U_S_KNJD  &&  w_pt->status != U_S_ADEL )
                 if ( w_pt->status == U_S_YOMA )
                    w_pt->status = U_S_ADEL;  /* Set Delete Status Code      */
                 else
                    w_pt->status = U_S_YOMD;  /* Set Delete Status Code      */
              }
        }
    return;
}
