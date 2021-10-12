static char sccsid[] = "@(#)31	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mnumgt.c, libKJI, bos411, 9428A410j 7/23/92 03:24:02";
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

/********************* START OF MODULE SPECIFICATIONS *******************
 *
 * MODULE NAME:         _Mnumgt
 *
 * DESCRIPTIVE NAME:    Get a number code from a kana code
 *                                        that it share the same code.
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
 * FUNCTION:            Convert the PC code
 *                           to the number letter of PC kanji code.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        676 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mnumgt
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mnumgt( inum,onum )
 *
 *  INPUT:              inum  : PC code.(Single byte code)
 *
 *  OUTPUT:             onum  : PC kanji code.(Double byte code)
 *
 * EXIT-NORMAL:         IMSUCC   : Success of Execution.
 *
 *
 * EXIT-ERROR:          IMNTNUME : Nocorrespondance code on table.
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
 *                              NA.
 *                      Trace Block(TRB).
 *                              NA.
 *
 * TABLES:              Table Defines.
 *                              kmnumget.t
 *                                : Conversion table input one byte code
 *                                       to the double bytes number code.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              IDENTIFY : Module Identify Create.
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
#include <stdio.h>     /* Standard I/O Package.                         */

/*
 *      include Kanji Project.
 */
#include "kj.h"         /* Kanji Project Define File.                   */
#include "kcb.h"        /* Kanji Control Block Structure.               */
#include "_Mnumgt.t"    /* DBCS number code conversion table.           */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1988           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Get a number code from a kana code
 *                        that it share the same code.
 */
int     _Mnumgt(inum,onum)

uchar    *inum;         /* One byte code. (string)                      */
uchar    *onum;         /* Double byte code. (number)                   */
{
        int     ret_code;       /* Return Code.                         */

        ushort  dbcs_ch;        /* Code buffer(input code integer)      */
        ushort  dbcs_tb;        /* Code buffer(table code integer)      */
        uchar   *tp;            /* Table pointer                        */

        tp      = C_INIT;       /* Initialized table pointer.           */

/************************************************************************
 *      debug process routine.                                          */
CPRINT(START KMMUMGT);

        /*  1.1
         *      The conversion process one bytes code
         *                          to double bytes number code.
         */
        /* Input one bytes code.                                        */
        dbcs_ch = inum[0];

        /* Input one byte code is number code.                          */
        if ( (dbcs_ch >= 0x30) && (dbcs_ch <= 0x39) )
            {
                /* Number code table position set.                      */
                dbcs_ch = (dbcs_ch - 0x30) * 2;

                /* Number code table 't0' pointer address set.          */
                tp      = (uchar *)t0;
            }

        /* Input one byte code is katakana code.                        */
        if ( (dbcs_ch >= 0xb0) && (dbcs_ch <= 0xdf) )
            {
                /* Katakana code table position set.                    */
                dbcs_ch = (dbcs_ch - 0xb0) * 2;

                /* Katakana code table 't1' pointer address set.        */
                tp      = (uchar *)t1;
            }

        /*  1.2
         *      Table search.
         */
        /* Check the input code is can not convert code.                */
        if ( tp == C_INIT )                   /* nothing table.         */
                {
/************************************************************************
 *      debug process routine.                                          */
CPRINT(END 1.2 KMNUMGT);

                /* Error return (nothing conversion code.)              */
                return(IMNTNUME);
                }

        /* Get the double bytes code from conversion table.             */
        dbcs_tb = (C_HIBYTE * tp[dbcs_ch]) + tp[dbcs_ch + 1];

        /* Check the input code is can not convert code.                */
        if ( dbcs_tb == (ushort)M_UNDEF )
                {
/*************************************************************************
 *      debug process routine.                                           */
CPRINT(END 1.2 KMNUMGT);

                /* Error return (nothing conversion code.)              */
                return(IMNTNUME);
                }

        /*  1.3
         *      Set the DBCS double bytes number code to distination.
         */
        onum[0] = dbcs_tb / C_HIBYTE; /* Set the high byte code to dist.*/
        onum[1] = dbcs_tb % C_HIBYTE; /* Set the low  byte code to dist.*/

/************************************************************************
 *      debug process routine.                                          */
CPRINT(END 1.3 KMNUMGT);

        /* Success of execution.                                        */
        return( IMSUCC );
}

