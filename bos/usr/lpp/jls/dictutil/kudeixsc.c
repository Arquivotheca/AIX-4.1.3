static char sccsid[] = "@(#)18	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kudeixsc.c, cmdKJI, bos411, 9428A410j 7/23/92 01:04:08";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kudeixsc
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
 * MODULE NAME:         kudeixsc
 *
 * DESCRIPTIVE NAME:    User Dictionary Index Area Search
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
 * FUNCTION:            User Dictionary Index(Kana) Search Process
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        612 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kudedtsc
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kudeixsc ( dicindex, kana, kanalen,
 *                                  indxpos, indxpos1, lastfg )
 *
 *  INPUT:              dicindex :Pointer to User Dictionary Index
 *                      kana     :Search Yomi Data
 *                      kanalen  :Search Yomi Data Length
 *
 *  OUTPUT:             indxpos  :Search for Data Relative Byte -1
 *                      indxpos1 :Search for Data Relative Byte -2
 *                      lastfg   :Search Data is Last Flag
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
 *                              NA.
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
 *                              CHPTTOSH:char to short int Data transfer
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
#include <stdio.h>      /* Standard I/O Package.                        */
/*#include <memory.h>*/ /* System memory operation uty.                 */

/*
 *      include Kanji Project.
 */
#include "kut.h"                        /* Kanji Utility Define File.   */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-125 COPYRIGHT IBM CORP 1989           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Please Descripte This Module.
 */
void  kudeixsc ( dicindex, kana, kanalen, indxpos, indxpos1, lastfg )

uchar   *dicindex;      /* Pointer to Index Data Area                        */
uchar   *kana;          /* Pointer to Yomi Data                              */
short   kanalen;        /* Length of Yomi Data                               */
short   *indxpos;       /* Index Position.                                   */
short   *indxpos1;      /* Index Position 1.                                 */
char    *lastfg;        /* Last Flag.                                        */

{
        int     len;            /* Memory Compare Length                     */
        int     rc;             /* Memory Compare return Code                */
        ushort  knl;            /* Yomi Length Save Area                     */
        ushort  il;             /* Index Active Area Data's Size Save Area   */

        *lastfg  = U_FOF;           /* Reset Last Flag                       */
        il = CHPTTOSH(dicindex);    /* Get Active INDEX Area's Size          */
                                    /* Initial Position Set  ( First Entry ) */
        *indxpos1 = U_ILLEN + U_STSLEN + U_HARLEN + U_NARLEN;
        *indxpos  = *indxpos1;

        while ( 1 )
        {
            knl = *(dicindex + *indxpos);       /* Get Index Yomi Length     */
                                                /* Set Memory Compare Length */
            len = ( kanalen < knl ) ? kanalen : knl;
                                                /* Memory Compare            */
            rc = memcmp ( kana, (char *)(dicindex + *indxpos + 1), len );
                                           /* Check Compare return Code      */
            if ( ( rc == 0 ) && ( kanalen <= len ) )  break;
            if ( rc < 0 )  break;
                                           /* Active Index Area Length Check */
            if ( ( *indxpos + knl + U_RRNLEN ) >= il )
            {
                *lastfg = U_FON;           /* Set Last Flag                  */
                break;
            }
                                           /* Set Index Area Position        */
            *indxpos1  = *indxpos;         /* Previous Position Set          */
            *indxpos  += knl + U_RRNLEN;   /* 1 Entry Length Add             */
        }

        return;

}
