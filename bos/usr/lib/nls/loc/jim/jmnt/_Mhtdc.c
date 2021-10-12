static char sccsid[] = "@(#)08	1.2.1.2  src/bos/usr/lib/nls/loc/jim/jmnt/_Mhtdc.c, libKJI, bos411, 9428A410j 2/10/94 07:23:53";
/*
 * COMPONENT_NAME : (libKJI) Japanese Input Method - Kanji Monitor
 *
 * FUNCTIONS : _Mhtdc.c
 *
 * ORIGINS : 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS *********************
 *
 * MODULE NAME:         _Mhtdc
 *
 * DESCRIPTIVE NAME:    Hankaku pc code to a pc kanji code conversion prog.
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
 * ENTRY POINT:         _Mhtdc()
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mhtdc( cmode,hcode,dbcode )
 *
 *  INPUT:              cmode  : Conversion mode
 *                               ( EISU=1  KATAKANA=2  HIRAGANA=3 )
 *                      hcode  : Input code (PC Code)
 *
 *
 *  OUTPUT:             *dbcode : Output string (PC Kanji Code)
 *
 * EXIT-NORMAL:         IMHTDOK  : Success of Execution.
 *
 * EXIT-ERROR:          IMHTDERR : No conversion character in table.
 *                      IMHTDCAN : Conversion mode error.
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
 *                              _Mhtdc.t : conversion table (pc code to
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
#include "kj.h"         /* Kanji Project Define File.                   */
#include "kcb.h"        /* Kanji Control Block Structure.               */
#include "_Mhtdc.t"     /* Kanji code Conversion table.                 */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1988           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Check a conversion mode, and then convert a PC code character.
 */
_Mhtdc( cmode,hcode,dbcode )

int     cmode;  /* Conversion mode (EISU=1 KATAKANA=2 HIRAGANA=3)       */
uchar   hcode;  /* Input code (PC Code)                                 */
uchar   *dbcode;/* Output string (PC Kanji Code)                        */
{
        int     i;      /* table pointer.                               */

/*************************************************************************
 *      debug process routine.                                           */
CPRINT(START KMHTDC);

        /*  1.1
         *      Check conversion mode.
         */
        if ( (cmode != M_SFTANK) &&
             (cmode != M_SFTKAT) && (cmode != M_SFTHIR) )
             {
/************************************************************************
 *      debug process routine.                                          */
CPRINT(END 1.1 KMHTDC);

             /* Noting conversion process for the mode.                 */
             return ( IMHTDCAN );
             }
        /* 1.2
         *      Convert PC code to PC Kanji code
         */
        /* Get the conversion code pointer on table. ( 0 , 2 or 4 )     */
        i = (cmode - 1) * 2;

        /* 1.2.a  Put Kanji code.                                       */
        if (tdc_tbl[hcode][i] != 0x00)
            {
                /* Put Kanji code.                                      */
                dbcode[0] = tdc_tbl[hcode][i];   /* Put high byte.      */
                dbcode[1] = tdc_tbl[hcode][i+1]; /* Put low  byte.      */
                dbcode[2] = 0x00;                /* Put NULL code.      */
/************************************************************************
 *      debug process routine.                                          */
CPRINT(END 1.2.a KMHTDC);

                /* Success of execution.                                */
                return ( IMHTDOK );
            };

        /* 1.2.b  Nothing conversion code on table.                     */
/************************************************************************
 *      debug process routine.                                          */
CPRINT(ERROR 1.2.b KMHTDC);

        /* Error of execution.                                          */
        return ( IMHTDERR );
}

Set_tdc_tbl(				/* change tdc_tbl[]		*/
	unsigned int	hcode,		/* hcode to be changed		*/
	unsigned int	high,		/* new high byte		*/
	unsigned int	low)		/* new low byte			*/
{
	
	tdc_tbl[hcode][0] = high;
	tdc_tbl[hcode][1] = low;
	tdc_tbl[hcode][2] = high;
	tdc_tbl[hcode][3] = low;
	tdc_tbl[hcode][4] = high;
	tdc_tbl[hcode][5] = low;
}
