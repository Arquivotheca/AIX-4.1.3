static char sccsid[] = "@(#)34	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Jinit.c, libKJI, bos411, 9428A410j 7/23/92 03:17:31";
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
 * MODULE NAME:         _Jinit
 *
 * DESCRIPTIVE NAME:    Initialize Kanji Field
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
 * FUNCTION:            Initialize Kanji Control Block and
 *                      Kanji Monitor Internal Save Area.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        844 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Jinit
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Jinit( pt )
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         KMSUCC  :Successful.
 *                      KMINITE :Specified field is already active.
 *                      KMIFLENE:Invalid DBCS input field length.
 *                      KMIVRIE :Invalid Cursor Position.
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _Mstrl()   : Get String Last Position.
 *                              _Mindset() : Set Shift Indicator.
 *                      Standard Library.
 *                              memset
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
 *                              kjsvpt  flatsd  actcol  indlen
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              kmact
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Controle Block(KCB).
 *                              string  hlatst  lastch
 *                              axuse1  axuse2  axuse3  axuse4  chlen
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              realcol kmact   msetflg
 *                              auxflg1 auxflg2 auxflg3 auxflg4
 *                              curleft curright        kkmode1 kkmide2
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
 *      This module initializes Kanji Control Block and
 *      Kanji Monitor Internal Save Area.
 */
int  _Jinit( pt )

KCB     *pt;            /* Pointer to Kanji Control Block               */
{
        register        KMISA   *kjsvpt;        /* Pointer to KMISA     */

        extern  int     _Mstrl();       /* Get String Last Position.    */
        extern  int     _Mindset();     /* Set Shift Indicator.         */

        uchar   *wstr;          /* String Pointer                       */
        int     lastpos;        /* String Last Position                 */

        int     rc;             /* Return Code.                         */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_Jinit,"start _Jinit");

        /*
         *      Pointer to Kanji Monitor Internal Save Area
         */

        kjsvpt = pt->kjsvpt;

        /*
         *      Set Successful Return Code
         */

        rc = KMSUCC;

        /* 1. 1
         *      Check Active Field
         */

        if ( kjsvpt->kmact != K_IFINAC )
           {
                /*
                 *      Set Return Code
                 */
                rc = KMINITE;
           }
        else
           {
                /* 1. 2
                 *      Check Field Attribute and Set Field Length
                 */
                switch ( pt->flatsd )
                       {
                        /*
                         *      Double Bytes Field
                         */
                        case K_ODUBYT:
                                /*
                                 *      Check Input Field
                                 */
                                if ( pt->actcol >= pt->indlen + C_DBCS )
                                   {
                                        /*
                                         *      Check 2 Bytes Boundary
                                         */
                                        if ( pt->actcol % C_DBCS == 0 )
                                         {
                                                /*
                                                 * Set Field Length
                                                 */
                                                kjsvpt->realcol =
                                                        pt->actcol;
                                        }
                                        else
                                           {
                                                /*
                                                 * Set 2 Bytes Boundary
                                                 */
                                                kjsvpt->realcol = pt->actcol
                                                                         - 1;
                                                /*
                                                 * Move String Pointer
                                                 */
                                                wstr  = pt->string +
                                                        kjsvpt->realcol;

                                                /*
                                                 * Add NULL to String
                                                 */
                                                *wstr = 0;
                                           };
                                   }
                                else
                                   {
                                        /*
                                         *      Set Error Code of Invalid
                                         *      Field Length
                                         */
                                        rc = KMIFLENE;

snap3(SNAP_KCB | SNAP_KMISA,SNAP_Jinit,"return1 _Jinit");

                                        return( rc );
                                   };
                                break;

                        /*
                         *      Mixed Field
                         */
                        case K_MIXMOD:
                                /*
                                 *      Check Input Field
                                 */
                                if ( pt->actcol >= pt->indlen + 1 )
                                   {
                                        /*
                                         *      Set Field Length
                                         */
                                        kjsvpt->realcol = pt->actcol;
                                   }
                                else
                                   {
                                        /*
                                         *      Set Error Code of Invalid
                                         *      Field Length
                                         */
                                        rc = KMIFLENE;

snap3(SNAP_KCB | SNAP_KMISA,SNAP_Jinit,"return2 _Jinit");

                                        return( rc );
                                   };
                                break;
                       };

                /* 1. 3
                 *      Check Cursor Position
                 */

                if ( pt->curcol < C_COL ||
                     pt->curcol > kjsvpt->realcol - pt->indlen )
                   {
                        /*
                         *      Set Error Code of Invalid Cursor Position
                         */
                        rc = KMIVRIE;

snap3(SNAP_KCB | SNAP_KMISA,SNAP_Jinit,"return3 _Jinit");

                        return( rc );
                   };

                /* 1. 4
                 *      Initialize Kanji Control Block
                 */

                /*
                 *      Get String Last Position
                 */
                lastpos = _Mstrl( pt );

                pt->lastch = lastpos;           /* String Last Position */
                pt->cnvsts = K_CONVF;           /* Conv. Finished       */
                pt->axuse1 = K_ANOUSE;          /* Aux. No1 Use Flag    */
                pt->axuse2 = K_ANOUSE;          /* Aux. No2 Use Flag    */
                pt->axuse3 = K_ANOUSE;          /* Aux. No3 Use Flag    */
                pt->axuse4 = K_ANOUSE;          /* Aux. No4 Use Flag    */
                pt->chlen  = 0;                 /* Character Length     */

                /* 1. 5
                 *      Initialize Kanji Monitor Internal Save Area
                 */

                kjsvpt->kmact    = K_IFACT;  /* Input Field Active Flag */
                kjsvpt->msetflg  = J_NULL;   /* Message Set Flag        */
                kjsvpt->auxflg1  = J_NULL;   /* Aux. No1 Use Flag       */
                kjsvpt->auxflg2  = J_NULL;   /* Aux. No2 Use Flag       */
                kjsvpt->auxflg3  = J_NULL;   /* Aux. No3 Use Flag       */
                kjsvpt->auxflg4  = J_NULL;   /* Aux. No4 Use Flag       */
                kjsvpt->curleft  = C_COL;    /* Cursor Left Limit       */
                                             /* Cursor Right Limit      */
                kjsvpt->curright = kjsvpt->realcol - pt->indlen;
                kjsvpt->kkmode1  = A_1STINP; /* Current Conv. Mode      */
                kjsvpt->kkmode2  = A_1STINP; /* Previous Conv. Mode     */

                /* 1. 6
                 *      Initialize Highlight Attribute
                 */

                memset(pt->hlatst,K_HLAT0,kjsvpt->realcol);

                /* 1. 7
                 *      Set Shift Indicator Code (Left and Right)
                 */
                rc = _Mindset( pt, M_INDB );
           };

        /* 1. 8
         *      Return Code.
         */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_Jinit,"return5 _Jinit");

        return( rc );
}
