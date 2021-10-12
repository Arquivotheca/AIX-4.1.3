static char sccsid[] = "@(#)13	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kudcmrud.c, cmdKJI, bos411, 9428A410j 7/23/92 00:59:15";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kudcmrud
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
 * MODULE NAME:         kudcmrud
 *
 * DESCRIPTIVE NAME:    User Dictionary MRU area data delete process
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
 * FUNCTION:            User Dictionary MRU area data delete process
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1076 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kudcmrud
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kudcmrud( mode, kanadata, kanalen, 
 *                                      kjdata, kjlen, udcbptr )
 *
 *  INPUT:              mode     :  Delete mode
 *                      kanadata :  Pointer to Yomi data
 *                      kanalen  :  Yomi data length
 *                      kjdata   :  Pointer to Kanji data
 *                      kjlen    :  Kanji data length
 *                      udcbptr  :  Pointer to user dictionary cntrol block
 *
 *  OUTPUT:                      :
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
 *                              NA.
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
 *                              CHPTTOSH:char to short int transfer.
 *                              SHTOCHPT:short int to char transfer.
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
void  kudcmrud( mode, kanadata, kanalen, kjdata, kjlen, udcbptr )

short    mode;          /* Delete Mode ( Kanji Delete or Yomi Delete )       */
uchar   *kanadata;      /* Pointer to Yomi Data                              */
short    kanalen;       /* Yomi Data Length                                  */
uchar   *kjdata;        /* Pointer to Kanji Data                             */
short    kjlen;         /* Kanji Data Length                                 */
UDCB    *udcbptr;       /* Pointer to User Dictionary Cntrol Block           */

{
   uchar   mrudata[U_MRU_A],       /* MRU Data Read Area                     */
           clrstr[U_REC_L],        /* Work Delete Entry                      */
           buff;                   /* Work Buffer                            */
   short   mrulen,                 /* MRU Data Length                        */
           pos,                    /* MRU Entry Postion                      */
           knl,                    /* Length of Yomi                         */
           kjl,                    /* Length of Kanji                        */
           del_f;                  /* Delete Flag                            */
   int     i,                      /* Work Counter                           */
           dpos,                   /* Move Charcter Start Position           */
           dlen,                   /* Move Charcter Length                   */
           ddst;                   /* Move Charcter Distance                 */

   /*
    *      Initialize & Delete Flag Set
    */
   for ( i = 0; i < U_REC_L; i++ )  clrstr[i] = NULL;
   del_f   = FALSE;                          /* Reset Delete Flag            */

   /*
    *     MRU Data Read
    */
                                             /* Copy MRU Data 7 Kbyte Read   */
   memcpy ( mrudata, udcbptr->dcptr, U_MRU_A );
   buff = mrudata[0];                        /* First 2 Byte Data Chenge     */
   mrudata[0] = mrudata[1];
   mrudata[1] = buff;
   mrulen = CHPTTOSH(mrudata);               /* Get MRU Active Data Size     */

   /*
    *      MRU Data Search & Delete
    */
   for ( pos = U_MRU_P; pos < mrulen; pos += knl + kjl )
       {
       knl = mrudata[pos];                   /* Get Yomi Data Length         */
       kjl = mrudata[pos + knl];             /* Get Kanji Data Length        */
                                             /* Compare of Yomi Data         */
       if ( knl - U_KNLLEN == kanalen  &&
            memcmp( &mrudata[pos + U_KNLLEN], kanadata, kanalen ) == NULL )
                                             /* Compare of Kanji Data        */
          if ( mode == U_S_YOMD ||
             ( kjl - U_KJLLEN == kjlen  &&
               memcmp(&mrudata[pos + knl + U_KJLLEN], kjdata, kjlen) == NULL) )
             {
             del_f = TRUE;                   /* Delete Flag Set              */ 
             dpos  = pos + knl + kjl;        /* Move Start Position Set      */
             dlen  = mrulen - dpos;          /* Move Length Set              */
             ddst  = knl + kjl;              /* Move Distance Set            */
             if ( mrulen == dpos )           /* Data is Last NULL Padding    */
                for ( i = pos; i < mrulen; i++ )
                   mrudata[i] = NULL;
             else                            /* Move Character Function      */
                kumvch ( mrudata, dpos, dlen, U_FORWD, ddst,
                         TRUE, clrstr, clrstr[0], ddst );
             mrulen -= knl + kjl;            /* MRU Active Data Size Set     */
             pos -= knl + kjl;               /* Position Update              */
             }
       }
       /*
        *      MRU Data Write
        */
   if ( del_f == TRUE )
       {
       SHTOCHPT ( mrudata, mrulen );         /* Set MRU Active Data Size     */
       buff = mrudata[0];                    /* First 2byte Data Chenge      */
       mrudata[0] = mrudata[1];
       mrudata[1] = buff;
                                             /* Copy MRU Data 7 Kbyte Write  */
       memcpy ( udcbptr->dcptr, mrudata, U_MRU_A );
       }
   return;
}
