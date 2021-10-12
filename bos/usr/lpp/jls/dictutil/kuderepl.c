static char sccsid[] = "@(#)19	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kuderepl.c, cmdKJI, bos411, 9428A410j 7/23/92 01:04:32";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kuderepl
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
 * MODULE NAME:         kuderepl
 *
 * DESCRIPTIVE NAME:    Index Area Data Replace
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
 * FUNCTION:            User Dictionary Index Area Data Replace
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        712 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kuderepl
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kuderepl ( rbpt, rbreppos, rbreplen, repdata, replen )
 *
 *  INPUT:              rbpt    :Pointer to Index Area
 *                      rbreppos:Replace Start relative byte
 *                      rbreplen:Replace Data length(for Index)
 *                      repdata :Pointer to Replace Data
 *                      replen  :Replace Data length(for replace Data)
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
 *                              memcmp
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
 *                              CHPTTOSH:char to short int data transfer
 *                              SHTOCHPT:short int to char data transfer
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

/*
 *      include Kanji Project.
 */
#include "kut.h"                         /* Kanji Utility Define File.  */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-125 COPYRIGHT IBM CORP 1989           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Please Descripte This Module.
 */
void  kuderepl ( rbpt, rbreppos, rbreplen, repdata, replen )

uchar  *rbpt;                       /* Pointer to INDEX Area     (to   Data) */
short   rbreppos;                   /* Replace Start Postion     (to   Data) */
short   rbreplen;                   /* Replace Length            (to   Data) */
uchar  *repdata;                    /* Pointer to Replace Data   (from Data) */
short   replen;                     /* Replace Data Length       (from Data) */

{

   ushort        il;                /* IL Length for Work                    */
   int           rc,                /* return code Work                      */
                 pos,               /* Start Position(top) Work              */
                 len,               /* Move Length(byte)   Work              */
                 dist,              /* Move Distance (byte)                  */
                 i;                 /* Counter Work                          */
   uchar        *st1,               /* Pointer to String 1                   */
                 clrstr[U_REC_L];   /* Clear Data String                     */


        /*  1.1
         *      initialize
         */
        il  = CHPTTOSH(rbpt);       /* Active INDEX Area's Size Get          */
        for ( i = 0; i < U_REC_L; i++ )  clrstr[i] = NULL;

        /*  2.1
         *      Move Data Index Area
         */
        dist = replen - rbreplen;        /* Calculation Move Distance        */
        pos  = rbreppos + rbreplen;      /* Calculation Move Position        */
        len  = il - pos;                 /* Calculation Move Length          */
        if ( dist > 0 )
                                         /* INDEX Area BACKWARD Move         */
          rc = kumvch ( rbpt, pos, len, U_BACKWD, dist,
                              TRUE, clrstr, clrstr[0], len );

        if ( dist < 0 )
                                         /* INDEX Area FORWARD Move          */
          rc = kumvch ( rbpt, pos, len, U_FORWD, -dist,
                              TRUE, clrstr, clrstr[0], len );

        /*  2.2
         *      Replace Data Index Area
         */
        st1  = rbpt + rbreppos;          /* To Data Address Set              */
                                         /* INDEX Area Data Replace          */
        memcpy ( st1, repdata, replen );

        /*
         *      Return & Value set
         */
        il += replen - rbreplen;         /* Active INDEX Area's Size Calc.   */
        SHTOCHPT(rbpt, il);              /* Set Active INDEX Area's Size     */
        return;
}
