static char sccsid[] = "@(#)33	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kudicupc.c, cmdKJI, bos411, 9428A410j 7/23/92 01:22:54";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kudicupc
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
 * MODULE NAME:         kudicupc
 *
 * DESCRIPTIVE NAME:    User Dictionary buffer update
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
 * FUNCTION:            User Dictionary buffer status set to be
 *                     update code & buffer additional
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1632 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kudicupc
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kudicupc ( dmappt, fldno, ipdata, iplen,
 *                                 topptr, lastptr, newptr )
 *
 *  INPUT:              dmappt   : Pointer to Display Map
 *                      fldno    : Field No to be Updated
 *                      ipdata   : Pointer to Yomi or Goku Data String
 *                      iplen    : Yomi or Goku Data String Length
 *                      topptr   : Pointer to Pointer to Top  Dictionary Buffer
 *                      lastptr  : Pointer to Pointer to Last Dictionary Buffer
 *                      newptr   : Pointer to Pointer to
 *                                  Additional New Dictionary Buffer
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      IUSUCC   : Process is Succesful
 *
 * EXIT-ERROR:          Return Codes Returned to Caller.
 *                      UDCALOCE : New Dictionary Buffer allocation error
 *                      UDOVFDLE : Entry Goku Field is Full
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              NA.
 *                      Standard Liblary.
 *                              memcpy
 *                              memcmp
 *                              malloc
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
int    kudicupc ( dmappt, fldno, ipdata, iplen, topptr, lastptr, newptr )

UDMS    *dmappt;        /* Pointer to Display Map                            */
short    fldno;         /* Field No to be Updated                            */
uchar   *ipdata;        /* Pointer to Input Yomi or Goku Data String         */
short    iplen;         /* Input Yomi or Goku Data String Length             */
UDCS   **topptr;        /* Pointer to Pointer to Top  Dictionary Buffer      */
UDCS   **lastptr;       /* Pointer to Pointer to Last Dictionary Buffer      */
UDCS   **newptr;        /* Pointer to Pointer to                             */
                        /*  Additional New Dictionary Buffer                 */
{
    UDCS    *pt;            /* Pointer to Dictionary Buffer for Work         */
    UDCS    *w_pt;          /* Pointer to Dictionary Buffer for Alloc        */
    UDCS    *bgnpos;        /* Pointer to Dictionary Buffer for First        */
    UDCS    *endpos;        /* Pointer to Dictionary Buffer for Last         */
    UDCS    *inspos;        /* Pointer to Dictionary Buffer for Insert       */
    short   stat;           /* Field Status                                  */
    uchar   ymdt[U_B_YOMI]; /* Transfer Before Yomi Data Save Area           */
    uchar   ymdt7b[U_B_YOMI]; /* Transfer Before 7bit Code Yomi Data         */
    uchar   ymwd7b[U_B_YOMI]; /* Transfer Before 7bit Code Yomi Data WK      */
    uchar   space[U_B_KAN]; /* Yomi & Goku Area Space Data Padding Data      */
    short   ymln;           /* Transfer Before Yomi Data length Save Area    */
    short   ymln7b;         /* Transfer Before 7bit Code Yomi Data Length    */
    short   ymwln7b;        /* Transfer Before 7bit Code Yomi Data Length WK */
    short   mflag;          /* 7bit Code Yomi Data Type Flag                 */
    int     cmplen;         /* Yomi Data Compare Length                      */
    int     gokuln;         /* Same Yomi All Goku Field Length               */
    int     rc;             /* Memory Compare Return Code                    */
    int     i;              /* Counter Work Area                             */

    /*
     *   Initialize
     */
     for ( i = 0; i < U_B_KAN; i += U_1KJ_L )    /* Space Clear Area Code Set*/
        {
        space[i]     = C_SPACEH;                 /* 4byte Space High Code    */
        space[i + 1] = C_SPACEL;                 /* 4byte Space Low  Code    */
        }
    /*
     *   Get Data from Display Map
     */
    stat = dmappt->fld[fldno].fstat;    /* Get input field status       */
    pt   = dmappt->fld[fldno].dbufpt;   /* Get input buffer pointer     */
                                       /* Input Dictionary Buffer Data Save  */
    ymln = pt->yomilen;                          /* Yomi Length Save         */
    memcpy ( ymdt, pt->yomi, (int)ymln );        /* Yomi String Save         */
    kudicymc ( ymdt, ymln, ymdt7b, &ymln7b, &mflag ); /* pc code=>7bit code  */
   /*
    *   Goku Field Data Process
    */
    if ( stat == U_GOKUF )
       {
                                       /* Check Same Yomi All Goku Length    */
       gokuln = U_DLLEN + iplen + U_RSVLEN;      /* Set Initial DL Length    */
       if ( pt->nx_pos != NULL )
          for ( w_pt = pt->nx_pos; w_pt != NULL; w_pt = w_pt->nx_pos )
             {                         /* Search for Same Yomi for Next      */
             /* Pc Code Convert to 7bit Code                                 */
             kudicymc ( w_pt->yomi, w_pt->yomilen, ymwd7b,&ymwln7b,&mflag );
             if ( ymwln7b != ymln7b  ||          /* Check Yomi Data          */
                  memcmp ( ymwd7b, ymdt7b, (int)ymln7b ) != NULL )
                break;                 /* Difference Yomi Search is End      */
             if ( w_pt->status != U_S_YOMD  &&   /* Check Status Code        */
                  w_pt->status != U_S_KNJD  &&  w_pt->status != U_S_ADEL )
                gokuln += w_pt->kanlen + U_RSVLEN;  /* Increment Goku Length */
             }
       if ( pt->pr_pos != NULL )
          for ( w_pt = pt->pr_pos; w_pt != NULL; w_pt = w_pt->pr_pos )
             {                         /* Search for Same Yomi for Previous  */
             /* Pc Code Convert to 7bit Code                                 */
             kudicymc ( w_pt->yomi, w_pt->yomilen, ymwd7b,&ymwln7b,&mflag );
             if ( ymwln7b != ymln7b  ||          /* Check Yomi Data          */
                  memcmp ( ymwd7b, ymdt7b, (int)ymln7b ) != NULL )
                break;                 /* Difference Yomi Search is End      */
             if ( w_pt->status != U_S_YOMD  &&   /* Check Status Code        */
                  w_pt->status != U_S_KNJD  &&  w_pt->status != U_S_ADEL )
                gokuln += w_pt->kanlen + U_RSVLEN;  /* Increment Goku Length */
             }
       if ( gokuln > U_KAN_MX )
              return ( UDOVFDLE );               /* Error Entry DL Size Over */

       memcpy ( pt->kan, space, U_B_KAN );       /* Goku Data Space Clear    */
       if ( pt->status != U_S_YOMA )
          pt->status = U_S_KNJU;                 /* Set Status Code          */
       pt->kanlen = iplen;                       /* Set Goku String Length   */
       memcpy ( pt->kan, ipdata, (int)iplen );   /* Set Goku String          */
       }
   /*
    *   Yomi Field Data Process
    */
    else
       {
       bgnpos = NULL;                            /* Set NULL First  Pointer  */
       endpos = NULL;                            /* Set NULL Last   Pointer  */
       inspos = NULL;                            /* Set NULL Insert Pointer  */
      /* Pc code string convert to 7bit Code string.                         */
       kudicymc( ipdata, iplen, ymdt7b, &ymln7b, &mflag );

      /*
       *  Same Yomi Search (Delete & Insert Position) All Buffer
       */
       for ( pt = *topptr; pt != NULL; pt = pt->nx_pos )
          {
          /* In the Buffer Pc Code string convert to 7bit Code               */
          kudicymc( pt->yomi, pt->yomilen, ymwd7b, &ymwln7b, &mflag );

         /*
          *  Search for Insert Buffer Pointer
          */
          if ( inspos == NULL )
             {                                /* Compare Length Set          */
             cmplen = ( ymln7b < ymwln7b ) ? ymln7b : ymwln7b ;
                                              /* Yomi Data Compare           */
             rc = memcmp ( ymdt7b, ymwd7b, cmplen );
             if ( rc < NULL  || ( rc == NULL && iplen < pt->yomilen ) )
                inspos = pt;                  /* Set Insert Buffer Pointer   */
             }
         /*
          *  Search for Same Yomi Buffer
          */
          if ( pt->yomilen == ymln               /* Check Yomi Data          */
            && memcmp ( pt->yomi, ymdt, (int)ymln ) == NULL )
             {
             if ( pt->status != U_S_YOMD  &&     /* Check Status Code        */
                  pt->status != U_S_KNJD  &&   pt->status != U_S_ADEL  )
                {                                /* New Buffer Allocation    */
                if ( pt->ycaddflg == C_SWON )
                   {
                     /* Set Old Status Yomi Delete.                          */
                     if ( pt->status == U_S_YOMA )
                         pt->status   = U_S_ADEL;
                     else
                         pt->status   = U_S_YOMD;
                   }
                else
                   {
                   w_pt = ( UDCS * ) malloc ( sizeof ( UDCS ) );
                   if ( w_pt == NULL )
                      return ( UDCALOCE );       /* Allocation Error End     */
                                                 /* Old Buffer Data All Copy */
                   memcpy ( w_pt, pt, sizeof ( UDCS ) );
                   w_pt->yomilen = iplen;        /* Set Input Yomi Length    */
                                                 /* Yomi Data Space Clear    */
                   memcpy ( w_pt->yomi, space, U_B_YOMI );
                                                 /* Set Input Yomi Data      */
                   memcpy ( w_pt->yomi, ipdata, (int)iplen );
                   if ( pt->status == U_S_YOMA ) /* Set Old Status Yomi Del  */
                      pt->status   = U_S_ADEL;
                   else
                      pt->status   = U_S_YOMD;

                   w_pt->status = U_S_YOMA;      /* Set New Status Yomi Add  */
                                                 /* Set Buffer Chain Pointer */
                   if ( bgnpos == NULL )
                      {
                      bgnpos = w_pt;             /* Save First Pointer       */
                      endpos = w_pt;             /* Save Last  Pointer       */
                      }
                   else
                      {
                      w_pt->pr_pos = endpos;     /* Set Buffer Chain Pointer */
                      endpos->nx_pos = w_pt;     /* Set Buffer Chain Pointer */
                      endpos = w_pt;             /* Save Last Pointer        */
                      }
                   }
                }
             }
          }
      /*
       *  New Buffer Pointer Chain Set
       */
       if ( bgnpos == NULL )
          {                                   /* Case add data nothing.   */
	  *newptr = dmappt->fld[fldno].dbufpt;
          }
       else
          {
          if ( inspos == NULL )
             {                                /* Insert Position is Last  */
             bgnpos->pr_pos = *lastptr;       /* Set Buffer Chain Pointer */
             pt = *lastptr;                   /* Set Last Pointer         */
             pt->nx_pos = bgnpos;             /* Set Buffer Chain Pointer */
             endpos->nx_pos = NULL;           /* Set Last Chain is NULL   */
             *lastptr = endpos;               /* Update to Last Pointer   */
             }
          else if ( inspos == *topptr )
             {                                /* Insert Position is Last  */
             endpos->nx_pos = *topptr;        /* Set Buffer Chain Pointer */
             pt = *topptr;                    /* Set Top Pointer          */
             pt->pr_pos = endpos;             /* Set Buffer Chain Pointer */
             bgnpos->pr_pos = NULL;           /* Set First Chain is NULL  */
             *topptr = bgnpos;                /* Update to Top Pointer    */
             }
          else
             {                                /* Insert Position Halfway  */
             pt = inspos->pr_pos;             /* Set New Previous pt.     */
             bgnpos->pr_pos = inspos->pr_pos; /* Set New Previous pt.     */
             inspos->pr_pos = endpos;         /* Set Insert Previous pt.  */
             endpos->nx_pos = inspos;         /* Set New Next pt.         */
             pt->nx_pos = bgnpos;             /* Set Insert Next pt.      */
             }
          *newptr = bgnpos;                   /* Set Add New Pointer      */
          }
       }
    return( IUSUCC );                         /* Return Process Success   */
}
