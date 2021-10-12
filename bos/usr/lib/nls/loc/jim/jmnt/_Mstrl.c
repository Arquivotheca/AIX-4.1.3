static char sccsid[] = "@(#)49	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mstrl.c, libKJI, bos411, 9428A410j 7/23/92 03:25:07";
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

/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         _Mstrl
 *
 * DESCRIPTIVE NAME:    Get string last position
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
 * FUNCTION:            Get string last position
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        564 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mstrl
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mstrl( pt )
 *
 *  INPUT:              pt      :Pointer to Kanji Controle Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         Return Value : String Last Position in Input Field
 *
 * EXIT-ERROR:          NA.
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
 *                              kjsvpt  string  flatsd  indlen
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              realcol
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
 *                              NA.
 *
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


/*
 *      Copyright Identify.
 */
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1988           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      This module gets string last position in input field
 *      and returns it as return value.
 */
int  _Mstrl( pt )

KCB     *pt;            /* POinter to Kanji Control Block               */
{
        register        KMISA   *kjsvpt;        /* Pointer to KMISA     */

        uchar   *string;        /* Work String Pointer                  */
        int     str_len;        /* String Length                        */

        char    sw;             /* Lower Byte Flag for "SPACE"          */
        int     i;              /* Loop Counter                         */

        int     last_ch;        /* Last Character Position              */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_Mstrl,"start _Mstrl");

        /*
         *      Pointer to Kanji Monitor Internal Sace Area
         */

        kjsvpt = pt->kjsvpt;

        /*
         *      Initialaize Last Character Position
         */

        last_ch = 0;

        /* 1. 1
         *      Set Work Pointer to The String
         */

        string  = pt->string;

        /*
         *      Get Last Position
         *              (Double Bytes or Mixed for Byte)
         */

        switch ( pt->flatsd )
               {
                /* 1. 1. a
                 *      Input Field is Double Bytes Character
                 */
                case K_ODUBYT:
                        /* Set Search Length */
                        str_len = kjsvpt->realcol - pt->indlen - C_DBCS;

                        /* Move Work String Pointer */
                        string += str_len;

                        /*
                         *      Last Position Search Process
                         */

                        for ( i = str_len; i >= 0;
                              i -= C_DBCS , string -= C_DBCS )
                            {
                                /*
                                 *      Check The Character "SPACE"
                                 */
                                if ( *string         == C_SPACEH &&
                                     *( string + 1 ) == C_SPACEL )
                                        continue;

                                /*
                                 *      Check The Character "NULL"
                                 */
                                if ( *string         == 0x00 &&
                                     *( string + 1 ) == 0x00 )
                                        continue;

                                /* Set Last Position */
                                last_ch = i + C_DBCS;
                                break;
                            };
                        break;

                /* 1. 1. b
                 *      Input Field is for Mixed Byte Character
                 */
                case K_MIXMOD:
                        sw = C_SWOFF;   /* Reset Low Byte Flag for "SPACE" */

                        /* Set Search Length */
                        str_len = kjsvpt->realcol - pt->indlen - C_ANK;

                        /* Move Work String Pointer */
                        string += str_len;

                        /*
                         *      Last Position Search Process
                         */

                        for ( i = str_len; i  >= 0; i-- , string-- )
                            {
                                /*
                                 *      Check Lower Byte Flag for "SPACE"
                                 */
                                if ( sw == C_SWOFF )
                                   {
                                        /*
                                         *      Check Blank or NULL
                                         */
                                        if ( *string == ' ' ||
                                             *string == 0x00 )
                                                continue;

                                        /*
                                         *      Check Lower Byte Code
                                         *      of The "SPACE" in DBCS
                                         */
                                        if ( *string == C_SPACEL )
                                           {
                                                /*
                                                 *      Set Lower Byte Flag
                                                 *            for "SPACE"
                                                 */
                                                sw = C_SWON;
                                                continue;
                                           }
                                        else
                                           {
                                                /* Set Last Position */
                                                last_ch = i + C_ANK;
                                                break;
                                           };
                                   }
                                else
                                   {
                                        /*
                                         *      Check Higher Byte Code
                                         *      of The "SPACE" in DBCS
                                         */
                                        if ( *string == C_SPACEH )
                                           {
                                                /*
                                                 *      Reset Lower Byte Flag
                                                 */
                                                sw = C_SWOFF;
                                                continue;
                                           }
                                        else
                                           {
                                                i++;

                                                /* Set Last Position */
                                                last_ch = i + C_ANK;
                                                break;
                                           };
                                   };
                            };
                        break;
               };

        /* 1. 2
         *      Return Last Position
         */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_Mstrl,"return _Mstrl");

        return( last_ch );
}
