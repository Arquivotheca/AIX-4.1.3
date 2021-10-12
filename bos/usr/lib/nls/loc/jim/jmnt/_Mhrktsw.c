static char sccsid[] = "@(#)07	1.2.1.2  src/bos/usr/lib/nls/loc/jim/jmnt/_Mhrktsw.c, libKJI, bos411, 9428A410j 5/23/93 20:50:59";
/*
 * COMPONENT_NAME : (libKJI) Japanese Input Method - Kanji Monitor
 *
 * FUNCTIONS : _Mhrktsw
 *
 * ORIGINS : 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         _Mhrktsw
 *
 * DESCRIPTIVE NAME:    Hiragana/Katakana Conversion Routine
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
 * FUNCTION:            Convert hiragana in the string into katakana
 *                              or
 *                      Convert katakana in the string into hiragaga,
 *                      according to the hiragana/katakana conversion map.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        864 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mhrktsw
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mhrktsw( pt )
 *
 *  INPUT:              pt      :Pointer to Kanji Controle Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         NA.
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _Mexchng() : replace the part of the string.
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
 *                              kjsvpt
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              convpos kanadata        kanalen
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Controle Block(KCB).
 *                              string
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              kanadata
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
 * CHANGE ACTIVITY:     Tuesday Aug. 23 1988 Satoshi Higuchi
 *                      Deleted fix buffer area and changed pointer
 *                      from str = buff to str = kjsvpt->iws1.
 *                      See problem collection sheet P-6 and
 *                      Monitor Improvement Spec. 3.2.1.
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
 *      This module dose,
 *              1. Convert hiragana in the string into katakana or
 *                 convert katakana in the string into hiragana,
 *                 according hiragana/katakana conversion map.
 *              2. Set the hiragana/katakana map for next conversion.
 *              3. Replace the string as converted.
 */
int  _Mhrktsw( pt )

KCB     *pt;            /* Pointer to Kanji Control Block               */
{
        register        KMISA   *kjsvpt;        /* Pointer to KMISA     */

        extern  int     _Mexchng();     /* Pointer to Part of The String*/

/*----------------------------------------------------------------------*/
/*      #(B) Deleted by S,Higuchi on Aug. 23 1988                       */
/*      uchar   buff[256];         Memory Buffer                        */
/*----------------------------------------------------------------------*/
        uchar   *str;           /* Pointer to Buffer                    */
        uchar   *refstr;        /* Pointer to Reference String          */
        uchar   map_data;       /* Conversion Mode                      */
        uchar   *mpt;           /* Pointer to Hiragana/Katakana Map     */
        short   offpos;         /* Offset Position (Start Pos.)         */
        short   strlen;         /* String Length (*str)                 */
        short   pos;            /* Replacement Position                 */
        int     unstrlen;       /* Unchanged String Length              */
        int     i;              /* Loop Counter                         */

        int     rc;             /* Return Code.                         */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_Mhrktsw,"start _Mhrktsw");

        /*
         *      Pointer to Kanji Monitor Internal Save Area
         */

        kjsvpt = pt->kjsvpt;

        /* 1. 1
         *      Initialize Parameters
         */

/*----------------------------------------------------------------------*/
/*      #(B) Changed by S,Higuchi on Aug. 23 1988                       */
/*      OLD     str = buff;                                             */
/*      NEW     str = kjsvpt->iws1;                                     */
/*----------------------------------------------------------------------*/
	str      = kjsvpt->iws1;        /* Poniter to Converted String  */
                                        /* Data Area                    */
                                        /* String Conversion Position   */

        refstr   = pt->string + kjsvpt->convpos;
        mpt      = kjsvpt->kanadata;    /* Conv. Mode                   */
                                        /* (Hiragana or Katakana)       */
        strlen   = 0;                   /* String Length Initialize     */
        unstrlen = 0;                   /* Unchanged String Length      */
        offpos   = 0;                   /* String Offset                */

        map_data = *mpt;                /* First Map Data               */

        /*
         *      Conversion  Hiragana into Katakana or
         *                  Katakana into Hiragana
         *                  According to Conversion Mode
         */
        switch ( map_data )
               {
                /*
                 *      No Conversion Mode
                 */
                case M_NOHRKT:
                     break;

                /*
                 *      Convert Hiragana into Katakana
                 */
                case M_HRKTMX:
                     /*
                      *         Change Map Mode for Next Conversion
                      *                 (Hiragana to Katakana)
                      */
                     *mpt++ = M_KTALL;

                     /*
                      *         Conversion String Set Process
                      */
                     for ( i = 0; i < kjsvpt->kanalen;
                                      i++, mpt++, refstr += C_DBCS )
                         {
                           switch ( *mpt )
                                  {
                                   case M_HKHIRA:
					/* Alpha-numeric */
					if ( *(refstr + 1) < 0x9f )
						goto Alpha;

                                        /* Set Katakana Code */
                                        *mpt = M_HKKATA;

                                        /*
                                         *      Set Converted String Data
                                         *         (Hiragana to Katakana)
                                         */

                                        /* High Byte Conversion */
                                        *str++ = *refstr + 0x01;

                                        /*
                                         *      Boundary Check
                                         */
                                        if ( *(refstr + 1) <= 0xdd )
                                             /* Low Byte Conversion */
                                             *str++ = *(refstr + 1) - 0x5f;
                                        else
                                             /* Low Byte Conversion */
                                             *str++ = *(refstr + 1) - 0x5e;

                                        /* Add String Data Length */
                                        strlen += unstrlen + C_DBCS;

                                        /* Reset Unchanged String Length */
                                        unstrlen = 0;
                                        break;

                                   default:
					Alpha:
                                        /*
                                         *      Set Unchanged Length String
                                         *              and
                                         *      Move String Pointer
                                         */
                                        *str++ = *refstr;       /* High */
                                        *str++ = *(refstr + 1); /* Low  */

                                        /*
                                         *      Check String Length
                                         */
                                        if ( strlen != 0 )
                                             /* Add Unchanged String Length*/
                                             unstrlen += C_DBCS;
                                        else
                                             /* Add Offset Length */
                                             offpos += C_DBCS;
                                        break;
                                  };
                         };
                     break;

                /*
                 *      Convert Katakana to Hiragana
                 */
                case M_KTALL:
                     /*
                      *         Change Map Mode for Next Conversion
                      *                 (Katakana to Hiragana)
                      */
                     *mpt++ = M_HRKTMX;

                     /*
                      *         Conversion String Set Process
                      */
                     for ( i = 0; i < kjsvpt->kanalen;
                                      i++, mpt++, refstr += C_DBCS )
                         {
                           switch ( *mpt )
                                  {
                                   case M_HKKATA:
                                        /* Set Hiragana Code */
                                        *mpt   = M_HKHIRA;

                                        /*
                                         *      Set Converted String Data
                                         *         (Katakana to Hiragana)
                                         */

                                        /* High Byte Conversion */
                                        *str++ = *refstr - 0x01;

                                        /*
                                         *      Boundary Check
                                         */
                                        if ( *(refstr + 1) <= 0x7e )
                                             /* Low Byte Conversion */
                                             *str++ = *(refstr + 1) + 0x5f;
                                        else
                                             /* Low Byte Conversion */
                                             *str++ = *(refstr + 1) + 0x5e;

                                        /* Add String Data Length */
                                        strlen += unstrlen + C_DBCS;

                                        /* Reset Unchanged String Length */
                                        unstrlen = 0;
                                        break;

                                   default:
                                        /*
                                         *      Set Unchanged String
                                         *              and
                                         *      Move String Pointer
                                         */
                                        *str++ = *refstr;       /* High */
                                        *str++ = *(refstr + 1); /* Low  */

                                        /*
                                         *      Check String Length
                                         */
                                        if ( strlen != 0 )
                                             /* Add Unchanged String Length*/
                                             unstrlen += C_DBCS;
                                        else
                                             /* Add Offset Length */
                                             offpos += C_DBCS;
                                        break;
                                  };
                         };
                     break;
               };

        /* 1. 2
         *      Change String Set Routine Call
         */

/*----------------------------------------------------------------------*/
/*      #(B) Changed by S,Higuchi on Aug. 23 1988                       */
/*      OLD     str = buff;                                             */
/*      NEW     str = kjsvpt->iws1;                                     */
/*----------------------------------------------------------------------*/
	str = kjsvpt->iws1;             /* Pointer to Reference String  */
        pos = kjsvpt->convpos + offpos; /* Replacement Position         */

        /*
         *      Call _Mexchng to Reprace The Part of The String
         */
        rc = _Mexchng( pt, str, offpos, strlen, pos, strlen );

        /* 1. 3
         *      Return Code.
         */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_Mhrktsw,"return _Mhrktsw");

        return( rc );
}
