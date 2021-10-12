static char sccsid[] = "@(#)39	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mreset.c, libKJI, bos411, 9428A410j 7/23/92 03:24:28";
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
 * MODULE NAME:         _Mreset
 *
 * DESCRIPTIVE NAME:    Mode reset routine.
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
 * FUNCTION:            Reset current status or Kanji Monitot mode.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        872 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mreset
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mreset( pt, rstmode )
 *
 *  INPUT:              pt      :Pointer to Kanji Controle Block.
 *                      rstmode :Reset mode
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC  :Successful.
 *                      IMKLRSTI:Keybord Lock Reset Information.(Successful)
 *                      IMMSGRSI:Message Reset Information.(Successful)
 *                      IMINSI  :Insert/Reset Information.
 *                      IMRGRSTI:Dictionary Registration Reset Information.
 *
 * EXIT-ERROR:          Wait State Codes.
 *                      NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _Mindset() : Indicator Set Routine.
 *                              _MK_rtn()  : KKC Interface Routine.
 *                              _Mfmrst()  : Field Message Reset.
 *                              _Mregrs()  : Dictionary Registration Reset.
 *
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
 *                              kjsvpt  kbdlok  repins
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              msetflg kkmode2 insert  kkcrmode
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Controle Block(KCB).
 *                              replins
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              nextact
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
 *      This module resets one of the following status according
 *      to their priority.
 *      If the status to be reset is specified by input parameter,
 *      resets the status without considering its priority.
 *              1. Keybord lock
 *              2. All candidate, Kanji Number input, or
 *                 Conversion mode Switching mode
 *              3. Insert mode
 *              4. Dictionary registration
 */
int  _Mreset( pt, rstmode )

KCB     *pt;            /* Pointer to Kanji Control Block               */
short   rstmode;        /* Reset Mode                                   */
{
        register        KMISA   *kjsvpt;        /* Pointer to KMISA     */

        extern  int     _Mindset();     /* Indicator Set Routine        */
        extern  int     _MK_rtn();      /* KKC Interface Routine        */
        extern  int     _Mfmrst();      /* Restore Send Field           */
        extern  int     _Mregrs();      /* Reset Dict. Registration     */

        int     msg_rst;        /* Message reset flag.                  */

        int     rc;             /* Return Code.                         */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_Mreset,"start _Mreset");

        /*
         *      Pointer to Kanji Monitor Internal Save Area
         */
        kjsvpt = pt->kjsvpt;

        /*
         *      Set Successful Resturn Code
         */
        rc = IMSUCC;

        /* 1. 1
         *      Select Reset Process by Input Parameter
         */

        switch ( rstmode )
               {
                /*
                 * Priority 1. Keybord lock
                 *          2. All candidate, Kanji Number input, or
                 *             Conversion mode Switching mode
                 *          3. Insert mode
                 *          4. Dictionary registration
                 */
                case M_ALLRST:

                /*
                 *      Reset Keybord Lock
                 */
                case M_KLRST:
                     /*
                      * Check Keybord Lock Flag
                      */
                     if ( pt->kbdlok == K_KBLON )
                        {
                          /* Reset Keybord Lock Flag */
                          pt->kbdlok = K_KBLOFF;

                          /* Set Next Action Flag */
                          kjsvpt->nextact = kjsvpt->nextact & ~M_KLRSON;

                          /*
                           *    Set Left Indicator
                           */
                          rc = _Mindset( pt, M_INDL );

                          /* Keybord Lock Reset Information (Successful) */
                          rc = IMKLRSTI;
                          break;
                        };

                     /*
                      * If This Routine is Specified by Input Parameter
                      */
                     if ( rstmode == M_KLRST )
                                break;

                /*
                 *      Reset All candidate, Kanji Number input, or
                 *            Conversion mode Switching mode
                 */
                case M_MSGRST:
                    /*
                     * Check Message Flag
                     */
                    if ( (kjsvpt->msetflg & K_MSGOTH) == K_MSGOTH )
                       {
                                /*
                                 * Restor Send Field
                                 */
                                rc = _Mfmrst( pt, K_MSGOTH );

                                /* Return to Previous Mode */
                                rc = _MM_rtn( pt, kjsvpt->kkmode2 );

                                /* Message Reset Informatin (Successful) */
                                rc = IMMSGRSI;
                                break;
                       };

                    /*
                     * If This Routine is Specified by Input Parameter
                     */
                    if ( rstmode == M_MSGRST )
                            break;

                /*
                 *      Reset/Insert Mode
                 */
                case M_RIRST:
                     /*
                      * Check Replace/Insert Flag (KCB)
                      *         and
                      * Check The Key Customized to Reset The Insert Mode
                      */
                     if ( pt->repins == K_INS &&
                          kjsvpt->kmpf[0].insert == K_RESET )
                        {
                          /* Set Replace Mode (KCB) */
                          pt->repins = K_REP;

                          /*
                           * Set Left Indicator
                           */
                          rc = _Mindset( pt, M_INDL );

                          /* Insert/Reset Information */
                          rc = IMINSI;
                          break;
                        };

                     /*
                      * If This Routine is Specified by Input Parameter
                      */
                     if ( rstmode == M_RIRST )
                                break;

                /*
                 *      Reset Dictionary Registration
                 */
                case M_RGRST:
                    /*
                     * Check Registration Mode Flag
                     */
                    if ( kjsvpt->kkcrmode != K_NODIC )
                       {
                          /*
                           * Reset Dict. Registration
                           */
                          rc = _Mregrs( pt );

                          /* Dictionary Registration Reset Information */
                          rc = IMRGRSTI;
                          break;
                       };

                    /*
                     * If This Routine is Specified by Input Parameter
                     */
                    if ( rstmode == M_RGRST )
                                break;

                /*
                 *      Other Status is Specified by Input Parameter
                 */
                default:
                     break;
               };

        /* 1. 2
         *      Return Code.
         */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_Mreset,"return _Mreset");

        return( rc );
}
