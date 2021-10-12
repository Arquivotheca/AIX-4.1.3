static char sccsid[] = "@(#)47	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kuhtdc.c, cmdKJI, bos411, 9428A410j 7/23/92 01:27:57";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kuhtdc
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

/********************* START OF MODULE SPECIFICATIONS *********************
 *
 * MODULE NAME:         kuhtdc
 *
 * DESCRIPTIVE NAME:    Hankaku pc code to a pc kanji code conversion prog.
 *
 * COPYRIGHT:           5756-030 COPYRIGHT IBM CORP 1991
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              User Dictionary Maintenance for AIX 3.2
 *
 * CLASSIFICATION:      OCO Source Material - IBM Confidential.
 *                      (IBM Confidential - Restricted when aggregated)
 *
 * FUNCTION:            The converter of pc code to pc kanji code.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1988 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kuhtdc()
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kuhtdc( cmode,hcode,dbcode )
 *
 *  INPUT:              cmode  : Conversion mode
 *                               ( EISU=1  KATAKANA=2  HIRAGANA=3 )
 *                      hcode  : Input code (PC Code)
 *
 *
 *  OUTPUT:             *dbcode : Output string (PC Kanji Code)
 *
 * EXIT-NORMAL:         IUSUCC  : Success of Execution.
 *
 * EXIT-ERROR:          IUHTDERR : No conversion character in table.
 *                      IUHTDCAN : Conversion mode error.
 *
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              NA.
 *                      Standard Library.
 *                              NA.
 *                      Advanced Display Graphics Support Library(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      NA.
 *
 * TABLES:              Table Defines.
 *                              kuhtdc.t : conversion table (pc code to
 *                                                           pc kanji code.)
 * MACROS:              Kanji Project Macro Library.
 *                              IDENTIFY:Module Identify Create.
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

/*
 *      include Kanji Project.
 */
#include "kut.h"        /* Kanji utility define file.                   */
#include "kuhtdc.t"     /* Kanji code Conversion table.                 */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-125 COPYRIGHT IBM CORP 1989           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Check a conversion mode, and then convert a PC code character.
 */
kuhtdc( cmode,hcode,dbcode )

int     cmode;  /* Conversion mode (EISU=1 KATAKANA=2 HIRAGANA=3)       */
uchar   hcode;  /* Input code (PC Code)                                 */
uchar   *dbcode;/* Output string (PC Kanji Code)                        */
{
        int     i;      /* table pointer.                               */

        /*  1.1
         *      Check conversion mode.
         */
        if ( (cmode != U_SFTANK) &&
             (cmode != U_SFTKAT) && (cmode != U_SFTHIR) )
             {

             /* Noting conversion process for the mode.                 */
             return ( IUHTDCAN );
             }
        /* 1.2
         *      Convert PC code to PC Kanji code
         */
        /* Get the conversion code pointer on table. ( 0 , 2 or 4 )     */
        i = (cmode - 1) * 2;

        /* 1.2.a  Put Kanji code.                                       */
        if (tdc_tbl[hcode][i] != NULL)
            {
                /* Put Kanji code.                                      */
                dbcode[0] = tdc_tbl[hcode][i];   /* Put high byte.      */
                dbcode[1] = tdc_tbl[hcode][i+1]; /* Put low  byte.      */
                dbcode[2] = NULL;                /* Put NULL code.      */

                /* Success of execution.                                */
                return ( IUSUCC );
            };

        /* 1.2.b  Nothing conversion code on table.                     */
/************************************************************************
 *      debug process routine.                                          */

        /* Error of execution.                                          */
        return ( IUHTDERR );
}

