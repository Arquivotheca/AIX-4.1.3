static char sccsid[] = "@(#)14	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mindset.c, libKJI, bos411, 9428A410j 7/23/92 03:22:50";
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
 * MODULE NAME:         _Mindset
 *
 * DESCRIPTIVE NAME:    Set Shift Indicator Code
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
 * FUNCTION:            Display specified shift indicator
 *                      corresponds to shift status.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1016 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mindset
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mindset( pt, scpf )
 *
 *  INPUT:              pt      :Pointer to Kanji Controle Block.
 *                      scpf    :Shift Indicator Position Flag.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC  :Successful.
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
 *                              kjsvpt  indlen  string  chlen   chpos
 *                              kbdlok  repins  shift1  shift2
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              realcol convimp
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Controle Block(KCB).
 *                              string  chpos   chlen
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
#include "dbcs.h"       /* DBCS Special Character.                      */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1988           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Shift Indicator Right Position Table
 */
static uchar shift_R[4][2] =

        /*      High Byte Set      ,    Low Byte Set            */
            { { M_IBLNK / C_HIBYTE ,    M_IBLNK & C_BYTEM },
              { M_IAN   / C_HIBYTE ,    M_IAN   & C_BYTEM },
              { M_IKATA / C_HIBYTE ,    M_IKATA & C_BYTEM },
              { M_IHIRA / C_HIBYTE ,    M_IHIRA & C_BYTEM }
            };

/*
 *      Shift Indicator Left Position Table
 */
static uchar shift_L[5][2] =
        /*      High Byte Set      ,    Low Byte Set            */
            { { M_IKBLK / C_HIBYTE ,    M_IKBLK & C_BYTEM },
              { M_INCHG / C_HIBYTE ,    M_INCHG & C_BYTEM },
              { M_IINS  / C_HIBYTE ,    M_IINS  & C_BYTEM },
              { M_IROM  / C_HIBYTE ,    M_IROM  & C_BYTEM },
              { M_IBLNK / C_HIBYTE ,    M_IBLNK & C_BYTEM }
            };

/*
 *      This module display specified shift indicator
 *      corresponds to shift status.
 */
int  _Mindset( pt, scpf )

KCB     *pt;            /* Pointer to Kanji Control Block               */
short   scpf;           /* Shift Indicator Position Flag            */
{
        register        KMISA   *kjsvpt;        /* Pointer to KMISA     */

        uchar   *string;        /* Work Pointer to String               */
        int     indlft;         /* Position in Array of Symbol for      */
                                /* Left Indicator                       */

        int     rc;             /* Return Code.                         */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_Mindset,"start _Mindset");

        /*
         *      Pointer to Kanji Internal Save Area
         */

        kjsvpt = pt->kjsvpt;

        /*
         *      Set Successful Return Code
         */

        rc = IMSUCC;

        /* 1. 1
         *      Shift Indicator Length Check
         */

        if ( pt->indlen > M_NOINDL )
           {
            string  = pt->string;

        /* 1. 2
         *      Set Shift Indicator
         */

            switch ( scpf )
              {
                /*
                 *      Set Shift Indicators (Both)
                 */
                case M_INDB:
                  /*
                   *    Set Indicator Position
                   */
                  string += kjsvpt->realcol - pt->indlen;

                  /*
                   *    Echo Range Check
                   */
                  if ( pt->chlen >  0 &&
                       pt->chpos <= kjsvpt->realcol - pt->indlen )
                     {
                      /* Set Echo Length */
                      pt->chlen = kjsvpt->realcol - pt->chpos;
                     }
                  else
                     {
                      /* Set Echo Position */
                      pt->chpos = kjsvpt->realcol - pt->indlen;
                      /* Set Echo Length */
                      pt->chlen = pt->indlen;
                     };

                  /*
                   *    Decide Left Indicator Symbol
                   */
                  switch ( pt->kbdlok )
                    {
                      /*
                       * Keybord Lock
                       */
                      case K_KBLON:
                        /*
                         * Set Array Position (Keybord Lock)
                         */
                        indlft = 0;
                        break;

                      case K_KBLOFF:
                        switch ( kjsvpt->convimp )
                          {
                            /*
                             * Conversion Impossible
                             */
                            case K_CVIPON:
                              /*
                               * Set Array Position (Conv. Impossible)
                               */
                              indlft = 1;
                              break;

                            case K_CVIPOF:
                              switch ( pt->repins )
                                {
                                  /*
                                   * Insert Mode
                                   */
                                  case K_INS:
                                    /*
                                     * Set Array Position (Insert Mode)
                                     */
                                    indlft = 2;
                                    break;

                                  case K_REP:
                                    switch ( pt->shift2 )
                                      {
                                        /*
                                         * Romagi ON
                                         */
                                        case K_ST2RON:
                                          /*
                                           * Set Array Position (Romaji ON)
                                           */
                                          indlft = 3;

                                          /*
                                           * Check Shift1 Undefined
                                           *       or Alphar/Numeric
                                           */
                                          if ( pt->shift1 == K_ST1UDF ||
                                               pt->shift1 == K_ST1AN )
                                             {
                                              /*
                                               * Set Array Position (Blank)
                                               */
                                              indlft = 4;
                                             };
                                          break;

                                        /*
                                         * Romaji OFF
                                         */
                                        case K_ST2ROF:
                                          /*
                                           * Set Array Position (Blank)
                                           */
                                          indlft = 4;
                                          break;

                                        default:
                                          /*
                                           * Set Array Postion (Blank)
                                           */
                                          indlft = 4;
                                          break;
                                      };
                                    break;
                                };
                              break;
                          };
                        break;
                    };

                  /*
                   *    Set Left Shift Indicator
                   */
                  *string++ = shift_L[indlft][0];       /* Set High Byte*/
                  *string++ = shift_L[indlft][1];       /* Set Low Byte */

                  /*
                   *    Set Rigth Shift Indicator
                   */
                  *string++ = shift_R[pt->shift1][0];   /* Set High Byte*/
                  *string   = shift_R[pt->shift1][1];   /* Set Low Byte */

                  break;

                /*
                 *      Set Shift Indicator (Right Only)
                 */
                case M_INDR:
                  /*
                   * Set Indicator Position
                   */
                  string += kjsvpt->realcol - pt->indlen + C_DBCS;

                  /*
                   *    Echo Range Check
                   */
                  if ( pt->chlen >  0 &&
                       pt->chpos <= kjsvpt->realcol - pt->indlen + C_DBCS )
                     {
                      /* Set Echo Length */
                      pt->chlen = kjsvpt->realcol - pt->chpos;
                     }
                  else
                     {
                      /* Set Echo Position */
                      pt->chpos = kjsvpt->realcol - pt->indlen + C_DBCS;
                      /* Set Echo Length */
                      pt->chlen = C_DBCS;
                     };

                  /*
                   *    Set Right Shift Indicator
                   */
                  *string++ = shift_R[pt->shift1][0];   /* Set High Byte*/
                  *string   = shift_R[pt->shift1][1];   /* Set Low Byte */
                  break;

                /*
                 * Set Shift Indicator (Left Only)
                 */
                case M_INDL:
                  /*
                   *    Set Indicator Position
                   */
                  string += kjsvpt->realcol - pt->indlen;

                  /*
                   *    Echo Range Check
                   */
                  if ( pt->chlen > 0 &&
                       pt->chpos < kjsvpt->realcol - pt->indlen )
                     {
                      /* Set Echo Length */
                      pt->chlen = kjsvpt->realcol - pt->chpos;
                     }
                  else
                     {
                      /* Set Echo Position */
                      pt->chpos = kjsvpt->realcol - pt->indlen;
                      /* Set Echo Length */
                      pt->chlen = C_DBCS;
                     };

                  /*
                   *    Decide Left Indicator Symbol
                   */
                  switch ( pt->kbdlok )
                    {
                      /*
                       * Keybord Lock
                       */
                      case K_KBLON:
                        /*
                         * Set Array Position (Keybord Lock)
                         */
                        indlft = 0;
                        break;

                      case K_KBLOFF:
                        switch ( kjsvpt->convimp )
                          {
                            /*
                             * Conversion Impossible
                             */
                            case K_CVIPON:
                              /*
                               * Set Array Position (Conv. Impossible)
                               */
                              indlft = 1;
                              break;

                            case K_CVIPOF:
                              switch ( pt->repins )
                                {
                                  /*
                                   * Insert Mode
                                   */
                                  case K_INS:
                                    /*
                                     * Set Array Position (Insert Mode)
                                     */
                                    indlft = 2;
                                    break;

                                  case K_REP:
                                    switch ( pt->shift2 )
                                      {
                                        /*
                                         * Romagi ON
                                         */
                                        case K_ST2RON:
                                          /*
                                           * Set Array Position (Romaji ON)
                                           */
                                          indlft = 3;

                                          /*
                                           * Check Shift1 Undefined
                                           *     or Alphar/Numeric
                                           */
                                          if ( pt->shift1 == K_ST1UDF ||
                                               pt->shift1 == K_ST1AN )
                                             {
                                              /*
                                               * Set Array Position (Blank)
                                               */
                                              indlft = 4;
                                             };
                                          break;

                                        /*
                                         * Romaji OFF
                                         */
                                        case K_ST2ROF:
                                          /*
                                           * Set Aray Position (Blank)
                                           */
                                          indlft = 4;
                                          break;

                                        default:
                                          /*
                                           * Set Array Position (Blank)
                                           */
                                          indlft = 4;
                                          break;
                                      };
                                    break;
                                };
                              break;
                          };
                        break;
                    };

                  /*
                   * Set Left Shift Indicator
                   */
                  *string++ = shift_L[indlft][0];       /* Set High Byte*/
                  *string   = shift_L[indlft][1];       /* Set Low Byte */

                  break;
              };
           };
        /* 1. 3
         *      Return Code.
         */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_Mindset,"return _Mindset");

        return( rc );
}
