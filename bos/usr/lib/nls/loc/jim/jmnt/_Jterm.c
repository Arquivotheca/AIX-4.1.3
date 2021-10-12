static char sccsid[] = "@(#)39	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Jterm.c, libKJI, bos411, 9428A410j 7/23/92 03:17:56";
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
 * MODULE NAME:         _Jterm
 *
 * DESCRIPTIVE NAME:    Kanji Field Terminate
 *
 * COPYRIGHT:           5601-061 COPYRIGHT IBM CORP 1988
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              Kanji Monitor V1.0
 *
 * CLASSIFICATION:      OCO Source Material - IBM Confidential.
 *                      (IBM Confidential-Restricted when aggregated)
 *
 * FUNCTION:            Terminate Kanji field and clear KMISA related
 *                      to Kana Kanji Conversion.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1008 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Jterm
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Jterm( pt )
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC  :Successful.
 *
 * EXIT-ERROR:          KMTERME :Input Field is Already Inactive
 *
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _Mkkcclr() :Clear KKC Interface Area
 *                              _Msetch()  :Set Changed Position and
 *                                          Changed Length for Display
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
 *                      Kanji Monitor Control Block(KCB).
 *                              kjsvpt  curcol  indlen
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              kmact   convlen msetflg realcol
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Controle Block(KCB).
 *                              chlen   curcol  string  hlatst
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              kmact
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
 *                      Moved terminator of if statement (};).
 *                      See problem collection sheet P-7.
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

static  int     _Matr();

/*
 *      This module terminates Kanji field and clears KMISA related
 *      to Kana Kanji Conversion.
 */
int  _Jterm( pt )

KCB     *pt;            /* Pointer to Kanji Control Block               */
{
        register        KMISA   *kjsvpt;        /* Pointer to KMISA     */
        extern  int     _Msetch();      /* Clear KKC Interface Area     */
        extern  int     _Mkkcclr();     /* Set Changed Position and     */
                                        /* Changed Length for Display   */

        uchar   *string;        /* Work Pointer to String               */
        uchar   *hlatst;        /* Work Pointer to Highlight Attribute  */

        short   wkchpos;        /* Work Position                        */
        short   wkchlen;        /* Work Length                          */
        short   oldcol;         /* Saved Cursor Position                */

        int     i;              /* Loop Counter                         */

        int     rc;             /* Return Code.                         */


snap3(SNAP_KCB | SNAP_KMISA,SNAP_Jterm,"start _Jterm");

        /*
         *      Pointer to Kanji Monitor Intanal Save Area
         */

        kjsvpt = pt->kjsvpt;

        /*
         *      Set Successful Return Code
         */

        rc = KMSUCC;

        /* 1. 1
         *      Active Field Check
         */

        if ( kjsvpt->kmact != K_IFACT )
           {
                /*
                 *      Set Return Code for "Field is Already Inactive"
                 */
                rc = KMTERME;
           }
        else
           {

                /* 1. 2
                 *      Clear KKC Interface Area
                 */

                pt->chlen = 0;          /* Reset Changed Length         */

                /*
                 *   KKC Physical error recovery
                 */

                 if ( KKCPHYER( kjsvpt-> kkcrc ) ) {
                   rc = _Matr( pt );
                 };

                /*
                 *      Conversion Length Check
                 *              or
                 *      Message Set Flag Check
                 */

                 kjsvpt->kkcrc = K_KCSUCC;

                 if ( ( kjsvpt->convlen > 0 ) ||
                      ( kjsvpt->msetflg > 0 ) ||
                      ( pt->kbdlok == K_KBLON ) )
                   {
                        oldcol = pt->curcol;    /* Save Cursor Position */


                        /*
                         *      Clear KKC Interface Area
                         */
                        rc = _Mkkcclr( pt );

                        pt->curcol = oldcol;    /* Reset Cursor Position*/
                   };

                /* 1. 3
                 *      Set Internal Parameters to Erase Shift Indicator
                 */

                string = pt->string;    /* Pointer to String            */
                hlatst = pt->hlatst;    /* Pointer to Highlight Att.    */

                                        /* Input Field                  */
                wkchpos = kjsvpt->realcol - pt->indlen;
                wkchlen = pt->indlen;   /* Indicator Length             */

                string += wkchpos;      /* String Pointer Move          */
                hlatst += wkchpos;      /* Highlight Att. Pointer Mode  */

                /* 1. 4
                 *      Erase Shift Indicator
                 *              Indicator Length Check
                 */
                if ( pt->indlen > 0 )
                   {
                        /*
                         *      Set String to Space
                         *              and
                         *      Set Highlight Attribute to Normal
                         *              on Shift Indicator Position
                         */
                        for ( i = 0; i < wkchlen; i += C_DBCS )
                            {
                                *string++ = C_SPACE / C_HIBYTE; /* High */
                                *string++ = C_SPACE & C_BYTEM;  /* Low  */
                                *hlatst++ = K_HLAT0;            /* High */
                                *hlatst++ = K_HLAT0;            /* Low  */
                            };
/*      #(B) Deleted by S,Higuchi on Aug. 23 1988                       */
/*                 };   This }; was moved other place.                  */

		    /*
		     *      Reset Changed Position and Length for Display
		     */

		     rc = _Msetch( pt, wkchpos, wkchlen );
		}   /* #(B) Added by S,Higuchi on Aug. 23 1988          */

                /*
                 *      Reset Field Active Flag to Inactive
                 */

                kjsvpt->kmact = K_IFINAC;
           };


        if( KKCPHYER( kjsvpt->kkcrc ) ) {

            /*
             *      Phigical Error Occure.
             */
            switch( KKCMAJOR( kjsvpt->kkcrc ) ) {
            /*
             * Physical error on System Dictionary.
             */
            case K_KCSYPE :
                    rc = KKSYDCOE;    /* Set return code. */
                    break;
            /*
             * Physical error on User   Dictionary.
             */
            case K_KCUSPE :
                    rc = KKUSDCOE;    /* Set return code. */
                    break;
            /*
             * Physical error on Adjunct Dictionary.
             */
            case K_KCFZPE :
                    rc = KKFZDCOE;    /* Set return code. */
                    break;
            /*
             * Memory allocation error.
             */
            case K_KCMALE :
                    rc = KKMALOCE;    /* Set return code. */
                    break;
            /*
             *      If Unknown Error Accept then
             *      'KKC Return Code'
             *      Return to Application.
             */
            default:
                    rc = KKFATALE;    /* Set return code. */
                    break;
            };
        };


        /* 1. 5
         *      Return Code.
         */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_Jterm,"return _Jterm");

        return( rc );
}

/*
 *  forced clear routine for highlighting attribute (DBCS field).
 */
static int  _Matr( pt )
KCB  *pt;
{
        extern  int     _Msetch();      /* Set Changed Position and     */
                                        /* Changed Length for Display   */

        KMISA   *kjsvpt;                /* Pointer to KMISA             */

        int     i;                      /* Loop Counter                 */
        int     ret;                    /* Return Coed                  */

        /*
         *      Pointer to Kanji Monitor Internal Save Area
         */
        kjsvpt = pt->kjsvpt;

        /*
         *      Set Successful Return Code
         */
        ret = IMSUCC;

        /*
         *      Set Highlight Atrribute
         */
        for ( i = 0; i < kjsvpt->realcol ; i++ ) {
                pt->hlatst[i] = K_HLAT0;
        };

        /*
         *      Set Changed Position and Changed Length for Echo.
         */
        ret = _Msetch( pt , (short)0 , kjsvpt->realcol - pt->indlen );

        /*
         *      Return
         */
        return ( ret );
}
