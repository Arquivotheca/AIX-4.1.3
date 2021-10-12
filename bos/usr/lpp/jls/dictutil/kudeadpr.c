static char sccsid[] = "@(#)16	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kudeadpr.c, cmdKJI, bos411, 9428A410j 7/23/92 01:00:32";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kudeadpr
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

/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         kudeadpr
 *
 * DESCRIPTIVE NAME:    User dictionary Data Add
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
 * FUNCTION:            User Dictionary Data Add
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1004 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kudeadpr
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kudeadpr ( mode, rbpt, rbpt1, length )
 *
 *  INPUT:              mode    : Mode to Data Add
 *                      rbpt    : Pointer to RRN (from data)
 *                      rbpt1   : Pointer to RRN (to   data)
 *                      length  : Data Add Length
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
 *  ROUTINES:           Internal Subroutines.
 *                              NA.
 *                      Kanji Project Subroutines.
 *                              kumvch
 *                      Standard Library.
 *                              memcpy
 *                      Advanced Display Graphics Support Library(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      NA.
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              IDENTIFY:Module Identify Create.
 *                              CHPTTOSH:char to short int transfer
 *                              SHTOCHPT;short int to char transfer
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     NA.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */

/*
 *      include Standard.
 */
#include <stdio.h>                 /* Standard I/O Package.                   */
/*#include <memory.h>*/

/*
 *      include Kanji Project.
 */
#include "kut.h"                   /* Kanji Utility include File              */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-125 COPYRIGHT IBM CORP 1989           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Please Descripte This Module.
 */
void  kudeadpr ( mode, rbpt, rbpt1, length )

short    mode;                     /* Mode to Add type                       */
uchar   *rbpt;                     /* Pointer to Data (from Data) for RRN    */
uchar   *rbpt1;                    /* Pointer to Data (to   Data) for RRN    */
short    length;                   /* Move to Data Length                    */

{

   int           kumvch();         /* Move Character function                */
   ushort        rl,               /* From Data Active Data's Size Save Area */
                 rl1;              /* To   Data Active Data's Size Save Area */
   int           rc,               /* Return Code from kumvch                */
                 pos,              /* Start Position(top) Work               */
                 len,              /* Move Length(byte)   Work               */
                 i;                /* Counter Work                           */
   uchar        *st1,              /* Pointer to String 1                    */
                *st2,              /* Pointer to String 2                    */
                 clrstr[U_REC_L];  /* Clear Data String                      */


     /*  1.1
      *      Initialize
      */
     rl  = CHPTTOSH(rbpt);         /* GET from RRN Active Data's Size        */
     rl1 = CHPTTOSH(rbpt1);        /* GET to   RRN Active Data's Size        */
                                   /* To RRN Active Data's Size is Zero      */
     if ( rl1 <= 0 )  rl1 = U_RLLEN;
                                   /* Set to NULL Clear Data Area            */
     for ( i = 0; i < U_REC_L; i++ )  clrstr[i] = NULL;

     /*  2.1
      *      Data Add Process (mode==1) Data Add to Left
      */
     if ( mode == U_DADDLF )
     {
        st1 = rbpt1 + rl1;             /* Copy To Address Set                */
        st2 = rbpt  + U_RLLEN;         /* Copy From Address Set              */
        memcpy ( st1, st2, length );   /* Entry Copy From RRN -> To RRN      */

        pos = U_RLLEN + length;        /* Move Character Position Set        */
        len = rl - pos;                /* Move Character Length Set          */
                                       /* Character Move after Null padding  */
        rc  = kumvch ( rbpt, pos, len, U_FORWD, length,
                             TRUE, clrstr, clrstr[0], len );

        rl1 += length;                 /* Update RRN Active Data's Area Size */
        rl  -= length;                 /* Update RRN Active Data's Area Size */
     }

     /*  2.2
      *      Data Add Process ( mode==2 ) Data Add to Right
      */
     if ( mode == U_DADDRT )
     {
        pos = U_RLLEN;                 /* Move Character Position Set        */
        len = rl1;                     /* Move Character Length Set          */
                                       /* Character Move after Null padding  */
        rc  = kumvch ( rbpt1, pos, len, U_BACKWD, length,
                              TRUE, clrstr, clrstr[0], length );

        st1 = rbpt1 + U_RLLEN;         /* Copy To Data Address Set           */
        st2 = rbpt  + rl - length;     /* Copy From Data Address Set         */
        memcpy ( st1, st2, length );   /* Entry Copy From RRN -> To RRN      */

        rl1 += length;                 /* Update RRN Active Data's Area Size */
        rl  -= length;                 /* Update RRN Active Data's Area Size */
                                       /* Character Move after Null padding  */
        rc  = kumvch ( rbpt, rl, length, U_BACKWD, length,
                             TRUE, clrstr, clrstr[0], length );
     }

     /*  2.3
      *      Data Add Process ( mode==3 ) Data Add to New
      */
     if ( mode == U_DADDNW )
     {
        st1 = rbpt1 + U_RLLEN;         /* Copy To Data Address Set           */
        st2 = rbpt  + U_RLLEN;         /* Copy From Data Address Set         */
        memcpy ( st1, st2, length );   /* Entry Copy From RRN -> To RRN      */

        pos = U_RLLEN + length;        /* Move Character Position Set        */
        len = rl - pos;                /* Move Character Length Set          */
                                       /* Character Move after Null padding  */
        rc  = kumvch ( rbpt, pos, len, U_FORWD, length,
                              TRUE, clrstr, clrstr[0], len );

        rl1  = length + U_RLLEN;       /* Update RRN Active Data's Area Size */
        rl  -= length;                 /* Update RRN Active Data's Area Size */
     }

     /*
      *      Return Code.
      */
     SHTOCHPT ( rbpt,  rl );           /* SET From RRN Active Data's Size    */
     SHTOCHPT ( rbpt1, rl1 );          /* SET To   RRN Active Data's Size    */
     return;
}
