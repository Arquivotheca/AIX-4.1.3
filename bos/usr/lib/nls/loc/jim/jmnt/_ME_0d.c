static char sccsid[] = "@(#)57	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_ME_0d.c, libKJI, bos411, 9428A410j 7/23/92 03:18:57";
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
 * MODULE NAME:         _ME_0d
 *
 * DESCRIPTIVE NAME:    Switch KKC conversion mode.
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
 * FUNCTION:            Switch KKC conversion mode.
 *                      (Word , Single , Phrse , Multi Phrase)
 *
 *
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        2300 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _ME_0d
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _ME_0d( pt )
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         INSUCC  :Successful.
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _Mnumgt() : Get a Number Code form Kana Code
 *                                          That Share The Same Key.
 *                              _MM_rtn() : Kanji Monitor Mode Switching
 *                                          Routine.
 *                              _Msetch() : Set Change Position and
 *                                          Change Length for Display.
 *                      Standard Library.
 *                              memset()  : Set Characters into Destination
 *                                          Memory Area.
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
 *                              kjsvpt  code
 *                              maxa1c  maxa1r  chpsa1  chlna1
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              cvmdsw  conversn
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Controle Block(KCB).
 *                              hlatst  hlata1
 *                              chpsa1  chlna1
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
 * CHANGE ACTIVITY:     Tuesday Aug. 23 1988 Satoshi Higuchi
 *                      Added to check return code from _Mnumgt()
 *                      See problem collection sheet P-3 and
 *                      Monitor Improvement Spec. 3.2.7.
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

static  int     _Mastch();      /* Aux. Area Set Change Position*/
                                /* and Change Length for Display*/

/*
 *      This module switching KKC conversion mode
 *      to the inputted code.
 */
int  _ME_0d( pt )

KCB     *pt;            /* Pointer to Kanji Control Block               */
{
        register        KMISA   *kjsvpt;        /* Pointer to KMISA     */

        extern  int     _Mnumgt();      /* Get a Number Code form Kana  */
                                        /* Code That Share The Same Key.*/
        extern  int     _MM_rtn();      /* Kanji Monitor Mode Switching */
                                        /* Routine.                     */
        extern  int     _Msetch();      /* Set Change Position and      */
                                        /* Change Length for Display.   */

        uchar   otchcode[2];    /* _Mnumgt Output Par. Char Code        */

        ushort  ch_md_sw;       /* Conversion Mode Switch               */
        uchar   char_sw;        /* Character for Conversion Mode        */
                                /* Switch (Next)                        */
        uchar   now_sw;         /* Character for Conversion Mode        */
                                /* Switch (Current)                     */
        uchar   *hlatst;        /* Pointer to Hilighting Attribute.(ON) */
        uchar   *hlatsd;        /* Pointer to Hilighting Attribute.(OFF)*/

        int     rc;             /* Return Code.                         */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_ME_0d,"start _ME_0d");

        /*
         *      Pointer to Kanji Monitor Internal Save Area
         */

        kjsvpt = pt->kjsvpt;

        /*
         *      Set Successful Return Code
         */

        rc = IMSUCC;

        /* 1. 1
         *      Convert The Code into Number
         */

        rc = _Mnumgt( &(pt->code) , otchcode );

/*      #(B) Added by S,Higuchi on Aug. 23 1988                         */
	if(rc != IMSUCC) {
	    rc = _MM_rtn(pt,A_BEEP);
	    return(rc);
	}
/*      end of added    */

        /*
         *      Get Inputted Conversion Mode Switching Number
         */

        ch_md_sw = CHPTTOSH( otchcode ) - 0x824f;

        /*
         *      Get Character for Conversion Mode Switching
         */
        char_sw = (uchar)ch_md_sw;

        /*
         *      Check Conversion Mode Switching Message
         *              is Display in Auxiliary Area
         */
        if ( kjsvpt->cvmdsw == C_SWON )
           {
                /*
                 *      Set Pointer to Auxiliary Area Hilighting Attribute
                 */
                hlatst = pt->hlata1;
                hlatsd = pt->hlata1;
           }
        else
           {
                /*
                 *      Set Pointer to Input Field Hilighting Attribute
                 */
                hlatst = pt->hlatst;
                hlatsd = pt->hlatst;
           };

        /*
         *      Get Current Conversion Mode
         */
        now_sw = kjsvpt->kmpf[0].conversn;

        /*
         *      Erase Current Highlight Attribute for
         *              Conversion Mode Switching
         */
        switch ( now_sw  )
               {
                /*
                 *     Word
                 */
                case K_FUKUGO:
                        /*
                         *      Move Hilighting Attribute Pointer
                         */
                        hlatsd += M_FUKPOS;

                        /*
                         *      Check Character for Mode Switching
                         */
/* #(B) 1988.01.12. Flying Conversion Change */
                        if ( ( char_sw != 4 ) &&
                             ( char_sw <= 4 ) &&
                             ( char_sw >  0 ) )
                           {
                                /*
                                 *      Set Hilighting Attribute to "Normal"
                                 */
/* #(B) 1988.01.12. Flying Conversion Change */
                                memset( hlatsd, K_HLAT0, M_MSWATR );

                                /*
                                 *      Check Conversion Mode Switching
                                 *      Message is Display in Auxiliary Area
                                 */
                                if ( kjsvpt->cvmdsw == C_SWON )
                                   {
                                        /*
                                         * Auxiliary Area
                                         *      Set Change Position and
                                         *      Change Length for Display
                                         */
/* #(B) 1988.01.12. Flying Conversion Change */
                                        _Mastch( pt, M_FUKPOS, M_MSWATR );
                                   }
                                 else
                                   {
                                        /*
                                         *      Set Change Position and
                                         *      Change Length for Display
                                         */
/* #(B) 1988.01.12. Flying Conversion Change */
                                        _Msetch( pt, M_FUKPOS, M_MSWATR );
                                    };
                           };
                        break;

                /*
                 *      Single Phrase
                 */
               case K_TANBUN:
                        /*
                         *      Move Hilighting Attribute Pointer
                         */
                        hlatsd += M_TANPOS;

                        /*
                         *      Check Character for Mode Switching
                         */
/* #(B) 1988.01.12. Flying Conversion Change */
                        if ( ( char_sw != 3 ) &&
                             ( char_sw <= 4 ) &&
                             ( char_sw >  0 ) )
                           {
                                /*
                                 *      Set Hilighting Attribute to
                                 *                      "Single Phrase"
                                 */
/* #(B) 1988.01.12. Flying Conversion Change */
                                memset( hlatsd, K_HLAT0, M_MSWATR );

                                /*
                                 *      Check Conversion Mode Switching
                                 *      Message is Display in Auxiliary Area
                                 */
                                if ( kjsvpt->cvmdsw == C_SWON )
                                   {
                                        /*
                                         * Auxiliary Area
                                         *      Set Change Position and
                                         *      Change Length for Display
                                         */
/* #(B) 1988.01.12. Flying Conversion Change */
                                        _Mastch( pt, M_TANPOS, M_MSWATR );
                                   }
                                else
                                   {
                                        /*
                                         *      Set Change Position and
                                         *      Change Length for Display
                                         */
/* #(B) 1988.01.12. Flying Conversion Change */
                                        _Msetch( pt, M_TANPOS, M_MSWATR );
                                   };
                           };
                        break;

                /*
                 *      Multi Phrase
                 */
                 case K_RENBUN:



/* #(B) 1988.01.12. Flying Conversion Change */

                     /*
                      *     Check Conversion Type.
                      */

                     /* Flying Conversion And Multi Phrase Conversion.  */
                     if ( kjsvpt->kmpf[0].convtype == K_CIKUJI) {

                        /*
                         *      Move Hilighting Attribute Pointer
                         */
                         hlatsd += M_FLYPOS;

                        /*
                         *      Check Character for Mode Switching
                         */
                        if ( ( char_sw != 1 ) &&
                             ( char_sw <= 4 ) &&
                             ( char_sw >  0 ) )
                           {
                                /*
                                 *      Set Hilighting Attribute to
                                 *                      "Multi Phrase"
                                 */
                                memset( hlatsd, K_HLAT0, M_MSWATR );

                                /*
                                 *      Check Conversion Mode Switching
                                 *      Message is Display in Auxiliary Area
                                 */
                                if ( kjsvpt->cvmdsw == C_SWON )
                                   {
                                        /*
                                         * Auxiliary Area
                                         *      Set Change Position and
                                         *      Change Length for Display
                                         */
                                        _Mastch( pt, M_FLYPOS, M_MSWATR );
                                   }
                                else
                                   {
                                        /*
                                         *      Set Change Position and
                                         *      Change Length for Display
                                         */
                                        _Msetch( pt, M_FLYPOS, M_MSWATR );
                                   };
                           };

                     } else {   /* Multi Phrase Conversion.             */

                        /*
                         *      Move Hilighting Attribute Pointer
                         */
                         hlatsd += M_RENPOS;

                        /*
                         *      Check Character for Mode Switching
                         */
                        if ( ( char_sw != 2 ) &&
                             ( char_sw <= 4 ) &&
                             ( char_sw >  0 ) )
                           {
                                /*
                                 *      Set Hilighting Attribute to
                                 *                      "Multi Phrase"
                                 */
                                memset( hlatsd, K_HLAT0, M_MSWATR );

                                /*
                                 *      Check Conversion Mode Switching
                                 *      Message is Display in Auxiliary Area
                                 */
                                if ( kjsvpt->cvmdsw == C_SWON )
                                   {
                                        /*
                                         * Auxiliary Area
                                         *      Set Change Position and
                                         *      Change Length for Display
                                         */
                                        _Mastch( pt, M_RENPOS, M_MSWATR );
                                   }
                                else
                                   {
                                        /*
                                         *      Set Change Position and
                                         *      Change Length for Display
                                         */
                                        _Msetch( pt, M_RENPOS, M_MSWATR );
                                   };
                           };

                     };

/* #(E) 1988.01.12. Flying Conversion Change */



                     break;
               };

        /*
         *      Set Next Conversion Mode and
         *      Display Highlight Attribute for Next Mode
         */

        switch ( char_sw )
               {
                /*
                 *      Word
                 */
/* #(B) 1988.01.12. Flying Conversion Change */
                case 4:
                        /*
                         *      Check Current Mode in Not "Word"
                         */
                        if ( kjsvpt->kmpf[0].conversn != K_FUKUGO )
                           {
                                /*
                                 *      Set Conversion Mode to "Word"
                                 */
                                kjsvpt->kmpf[0].conversn = K_FUKUGO;

/* #(B) 1988.01.12. Flying Conversion Add    */
                                /*
                                 *      Set Conversion Type to
                                 *              "Ikkatsu"
                                 */
                                kjsvpt->kmpf[0].convtype = K_IKKATU;


                                /*
                                 *      Move Hilighting Attribute Pointer
                                 */
                                hlatst += M_FUKPOS;

                                /*
                                 *      Set Hilighting Attribute to Reverse
                                 */
/* #(B) 1988.01.12. Flying Conversion Change */
                                memset( hlatst, K_HLAT1, M_MSWATR );

                                /*
                                 *      Check Conversion Mode Switching
                                 *      Message is Display in Auxiliary Area
                                 */
                                if ( kjsvpt->cvmdsw == C_SWON )
                                   {
                                        /*
                                         * Auxiliary Area
                                         *      Set Change Position and
                                         *      Change Length for Display
                                         */
/* #(B) 1988.01.12. Flying Conversion Change */
                                        _Mastch( pt, M_FUKPOS, M_MSWATR );
                                   }
                                else
                                   {
                                        /*
                                         *      Set Change Position and
                                         *      Change Length for Display
                                         */
/* #(B) 1988.01.12. Flying Conversion Change */
                                        _Msetch( pt, M_FUKPOS, M_MSWATR );
                                   };
                           };
                        break;

                /*
                 *      Single Phrase
                 */
/* #(B) 1988.01.12. Flying Conversion Change */
                case 3:
                        /*
                         *      Check Current is Not "Single Phrase"
                         */
                        if ( kjsvpt->kmpf[0].conversn != K_TANBUN )
                           {
                                /*
                                 *      Set Conversion Mode to
                                 *              "Single Phrase"
                                 */
                                kjsvpt->kmpf[0].conversn = K_TANBUN;

/* #(B) 1988.01.12. Flying Conversion Add    */
                                /*
                                 *      Set Conversion Type to
                                 *              "Ikkatsu"
                                 */
                                kjsvpt->kmpf[0].convtype = K_IKKATU;


                                /*
                                 *      Move Hilighting Attribute Pointer
                                 */
                                hlatst += M_TANPOS;

                                /*
                                 *      Set Hilighting Attribute to Reverse
                                 */
/* #(B) 1988.01.12. Flying Conversion Change */
                                memset( hlatst, K_HLAT1, M_MSWATR );

                                /*
                                 *      Check Conversion Mode Switching
                                 *      Message is Display in Auxiliary Area
                                 */
                                if ( kjsvpt->cvmdsw == C_SWON )
                                   {
                                        /*
                                         * Auxiliary Area
                                         *      Set Change Position and
                                         *      Change Length for Display
                                         */
/* #(B) 1988.01.12. Flying Conversion Change */
                                        _Mastch( pt, M_TANPOS, M_MSWATR );
                                   }
                                else
                                   {
                                        /*
                                         *      Set Change Position and
                                         *      Change Length for Display
                                         */
/* #(B) 1988.01.12. Flying Conversion Change */
                                        _Msetch( pt, M_TANPOS, M_MSWATR );
                                   };
                           };
                        break;

                /*
                 *      Multi Phrase
                 */
/* #(B) 1988.01.12. Flying Conversion Change */
                case 2:
                        /*
                         *      Check Current is Not "Multi Phrase"
                         */
/* #(B) 1988.01.12. Flying Conversion Change */
                        if (   (kjsvpt->kmpf[0].conversn != K_RENBUN) ||
                               (kjsvpt->kmpf[0].convtype == K_CIKUJI)  )
                           {
                                /*
                                 *      Set Conversion Mode to
                                 *              "Multi Phrase"
                                 */
                                kjsvpt->kmpf[0].conversn = K_RENBUN;

/* #(B) 1988.01.12. Flying Conversion Add    */
                                /*
                                 *      Set Conversion Type to
                                 *              "Ikkatsu"
                                 */
                                kjsvpt->kmpf[0].convtype = K_IKKATU;


                                /*
                                 *      Move Hilighting Attribute Pointer
                                 */
                                hlatst += M_RENPOS;

                                /*
                                 *      Set Hilighting Attribute to Reverse
                                 */
/* #(B) 1988.01.12. Flying Conversion Change */
                                memset( hlatst, K_HLAT1, M_MSWATR );

                                /*
                                 *      Check Conversion Mode Switching
                                 *      Message is Display in Auxiliary Area
                                 */
                                if ( kjsvpt->cvmdsw == C_SWON )
                                   {
                                        /*
                                         * Auxiliary Area
                                         *      Set Change Position and
                                         *      Change Length for Display
                                         */
/* #(B) 1988.01.12. Flying Conversion Change */
                                        _Mastch( pt, M_RENPOS, M_MSWATR );
                                   }
                                else
                                   {
                                        /*
                                         *      Set Change Position and
                                         *      Change Length for Display
                                         */
/* #(B) 1988.01.12. Flying Conversion Change */
                                        _Msetch( pt, M_RENPOS, M_MSWATR );
                                   };
                           };
                        break;



/* #(B) 1988.01.12. Flying Conversion Add */
                case 1:

                        /*
                         *      Check Current is Not "Multi Phrase"
                         */
                        if (   (kjsvpt->kmpf[0].conversn != K_RENBUN) ||
                               (kjsvpt->kmpf[0].convtype == K_IKKATU)  )
                           {
                                /*
                                 *      Set Conversion Mode to
                                 *              "Multi Phrase"
                                 */
                                kjsvpt->kmpf[0].conversn = K_RENBUN;

                                /*
                                 *      Set Conversion Type to
                                 *              "Ikkatsu"
                                 */
                                kjsvpt->kmpf[0].convtype = K_CIKUJI;

                                /*
                                 *      Move Hilighting Attribute Pointer
                                 */
                                hlatst += M_FLYPOS;

                                /*
                                 *      Set Hilighting Attribute to Reverse
                                 */
                                memset( hlatst, K_HLAT1, M_MSWATR );

                                /*
                                 *      Check Conversion Mode Switching
                                 *      Message is Display in Auxiliary Area
                                 */
                                if ( kjsvpt->cvmdsw == C_SWON )
                                   {
                                        /*
                                         * Auxiliary Area
                                         *      Set Change Position and
                                         *      Change Length for Display
                                         */
                                        _Mastch( pt, M_FLYPOS, M_MSWATR );
                                   }
                                else
                                   {
                                        /*
                                         *      Set Change Position and
                                         *      Change Length for Display
                                         */
                                        _Msetch( pt, M_FLYPOS, M_MSWATR );
                                   };
                           };
                        break;
/* #(E) 1988.01.12. Flying Conversion Add */



                default:
                        /*
                         *      Beep (Inadqate Inputting Code)
                         */
                        rc = _MM_rtn( pt, A_BEEP );
                        break;
               };

        /* 1. 3
         *      Return Code.
         */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_ME_0d,"return _ME_0d");

        return( rc );
}
/***********************************************************************
  Internal Routine for Auxiliary Area No.1 echo controling which is
      nearly equal to  _Msetch() besides echo length & echo position name.
 ***********************************************************************/

static  int  _Mastch(pt,position,length)

KCB     *pt;            /* pointer to KCB.                              */
short   position;       /* starting position of changed characters aux. */
short   length;         /* length of changed characters in aux. area    */

{
        KMISA   *kjsvpt;        /* pointer to KMISA.                    */
        int      ret_code;      /* return value.                        */
        short    lastpos1;      /* next end position of changed char.   */
        short    lastpos2;      /* current end position of changed char.*/
        short    maxaux1;       /* maximum length of auxiliary string.  */
        short    ws1;           /* work variable.                       */

        ret_code = IMSUCC;      /* set return code.                     */
        kjsvpt = pt->kjsvpt;    /* set work pointer.                    */
        maxaux1 = pt->maxa1c * pt->maxa1r;
        /* 0.
         *      process loop.
         */
        while( 1 )  {

            /* 1.
             *      check !  input parameter.
             */
            if(    ( position < 0 ) || ( maxaux1 < position )
                || ( length < 1 )
                || ( maxaux1 - position < length )     )
                {
                    /* 1. a
                     *      input parameter error.
                     */
                    ret_code = IMFAIL;
                    break;
                 }

            /* 2.
             *      check ! length of changed character already exist
             */
            if( pt->chlna1 > 0 )
                {
                    /* 1. b
                     *      Get next end position and
                     *          current end position of changed characters
                     */
                    lastpos1 = position + length;
                    lastpos2 = pt->chpsa1 + pt->chlna1;

                    /* 1. b2
                     *      set start position of changed characters.
                     */

                    pt->chpsa1 = (position <  pt->chpsa1) ?
                                                position : pt->chpsa1 ;

                    /* 1. b3
                     *      Get end position of changed characters.
                     */
                    ws1 = (lastpos1 > lastpos2) ? lastpos1 : lastpos2 ;

                    /* 1. b4
                     *      set length of changed characters.
                     */
                    pt->chlna1 = ws1 - pt->chpsa1;

                    break;
                }
            else
                {
                    /* 1. a
                     *      set changed position and length
                     *          from input parameters.
                     */
                    pt->chpsa1 = position;
                    pt->chlna1 = length;

                    break;
                }
        }

        /* 3.
         *      return.
         */

        return( ret_code );

}
