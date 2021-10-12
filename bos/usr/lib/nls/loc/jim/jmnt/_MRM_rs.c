static char sccsid[] = "@(#)76	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_MRM_rs.c, libKJI, bos411, 9428A410j 7/23/92 03:20:12";
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
 * MODULE NAME:         _MRM_rs
 *
 * DESCRIPTIVE NAME:    Dictionary Registration Message Reset
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
 * FUNCTION:            Reset dictionary registration message of Yomi
 *                      or Kanji registration.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        920 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _MRM_rs
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _MRM_rs( pt )
 *
 *  INPUT:              pt      :Pointer to Kanji Controle Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC  :Successful.
 *                      IMRMRSE :Invalid Key is Inputted as Registration
                                 Message reset.
 *                      IMRMRSTI:Dic. Reg. Message Reset Succ. Information
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _MM_rtn() : Mode Conversion.
 *                              _Mreset() : Mode Cancel Routine.
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
 *                              kjsvpt  code
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              kjin    reset   kkcrmode
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block (ECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Controle Block(KCB).
 *                              NA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              kkmode2 nextact
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
 *      This module reset dictionary registration message of Yomi
 *      or Kanji registration.
 */
int  _MRM_rs( pt )

KCB     *pt;            /* Pointer to Kanji Control Block               */
{
        register        KMISA   *kjsvpt;        /* Pointer to KMISA     */

        extern  int     _MM_rtn();      /* Mode Conversion              */
        extern  int     _Mreset();      /* Mode Cancel Routine          */

        short   rstflg1;        /* Error Return Flag 1                  */
        short   rstflg2;        /* Error Return Flag 2                  */
        short   beepflg;        /* Beep Flag                            */

        int     rc;             /* Return Code                          */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_MRM_rs,"start _MRM_rs");

        /*
         *      Pointer to Kanji Moniter Internal Save Area
         */

        kjsvpt = pt->kjsvpt;

        /*
         *      Set Successful Return Code
         */

        rc = IMSUCC;

        /*
         *      Initialize Flags
         */

        rstflg1 = C_SWOFF;              /* Error Return Flag 1          */
        rstflg2 = C_SWOFF;              /* Error Return Flag 2          */
        beepflg = C_SWOFF;              /* Beep Flag (OFF)              */

        /* 1. 1
         *      KCB Input Type Code Check
         */

        if ( pt->type != K_INESCF )
           {
                /*
                 *      Mode Switching (Beep)
                 */
                rc = _MM_rtn( pt, A_BEEP );

                rc = IMRMRSE;                   /* Set Error Code       */

           }
        else
           {

                /* 1. 2
                 *      KCB Input Code & KMISA Profile Check
                 */

                switch ( pt->code )
                       {
                        /*
                         *      Pseudo Code "Enter"
                         */
                        case P_ENTER:
                                /*
                                 *      Check Profile Whether The Key is
                                 *      Customized as Kanji No. Input
                                 *      or Dictionary Registration
                                 */
                                if ( ((kjsvpt->kmpf[0].kjin  & K_DAENT)
                                                            == K_REENT) ||
                                     ((kjsvpt->kmpf[0].reset & K_REENT)
                                                            == K_REENT) )
                                   {
                                        /* Set Message Flag (ON) */
                                        rstflg1 = C_SWON;

                                        /* Set Dict. Reg. Flag (ON) */
                                        rstflg2 = C_SWON;
                                   }
                                else
                                   {
                                        /* Set Beep Flag (ON) */
                                        beepflg = C_SWON;
                                   };
                                break;

                        /*
                         *      Pseudo Code "Action"
                         */
                        case P_ACTION:
                                /*
                                 *      Check Profile Whether The Key is
                                 *      Customized as Kanji No. Input
                                 *      or Dictionary Registration
                                 */
                                if ( ((kjsvpt->kmpf[0].kjin  & K_DAACT)
                                                            == K_DAACT) ||
                                     ((kjsvpt->kmpf[0].reset & K_REACT)
                                                            == K_REACT) )
                                   {
                                        /* Set Message Flag (ON) */
                                        rstflg1 = C_SWON;

                                        /* Set Dict. Reg. Flag (ON) */
                                        rstflg2 = C_SWON;
                                   }
                                else
                                   {
                                        /* Set Beep Flag (ON) */
                                        beepflg = C_SWON;
                                   };
                                break;

                        /*
                         *      Pseudo Code "Carriage Return"
                         */
                        case P_CR:
                                /*
                                 *      Check Profile Whether The Key is
                                 *      Customized as Kanji No. Input
                                 *      or Dictionary Registration
                                 */
                                if ( ((kjsvpt->kmpf[0].kjin  & K_DACR)
                                                            == K_DACR) ||
                                     ((kjsvpt->kmpf[0].reset & K_RECR)
                                                            == K_RECR) )
                                   {
                                        /* Set Message Flag (ON) */
                                        rstflg1 = C_SWON;

                                        /* Set Dict. Reg. Flag (ON) */
                                        rstflg2 = C_SWON;
                                   }
                                else
                                   {
                                        /* Set Beep Flag (ON) */
                                        beepflg = C_SWON;
                                   };
                                break;

                        /*
                         *      Pseudo Code "Reset"
                         */
                        case P_RESET:
                                /*
                                 *      Reset Appropriate Status
                                 */
                                rc = _Mreset( pt, M_ALLRST );

                                /*
                                 *      _Mreset Information Check
                                 */
                                if ( rc == IMMSGRSI )
                                   {
                                        /* Set First Kana Input Mode */
                                        kjsvpt->kkmode2 = A_1STINP;

                                        /* Set Dict. Reg. Flag (ON) */
                                        rstflg2 = C_SWON;
                                   }
                                else
                                   {
                                        if ( rc == IMSUCC )
                                           {
                                                /* Set Beep Flag (ON) */
                                                beepflg = C_SWON;
                                           };
                                   };
                                break;

                        default:
                                /* Set Beep Flag (ON) */
                                beepflg = C_SWON;

                                break;
                       };

                /* 1. 3
                 *      Kanji Moniter Internal Save Area kkcrmode Check
                 */

                /*
                 *      Message Flag Check
                 */
                if ( rstflg1 )
                   {
                        /* Reset Message */
                        _Mreset( pt, M_MSGRST );

                        /* Set First Kana Input Mode */
                        kjsvpt->kkmode2 = A_1STINP;
                   };

                /*
                 *      Beep Flag Check
                 */
                if ( beepflg )
                   {
                        /* Mode Switching for Beep */
                        rc = _MM_rtn( pt, A_BEEP );

                        /*
                         *      Registration Message Reset Error
                         */
                        rc = IMRMRSE;
                   };

                /*
                 *      Dictionary Registration Flag Check
                 */
                if ( rstflg2 )
                   {
                        /*
                         *      Dictionary Registration Mode
                         */
                        switch ( kjsvpt->kkcrmode )
                               {
                                /*
                                 *      "Yomi" Registration Mode
                                 */
                                case K_REDIC:

                                        /*
                                         * Set Next Action to Call _MRM_rs
                                         */
                                        kjsvpt->nextact = kjsvpt->nextact &
                                                                 ~M_RMRSON;

                                        /*
                                         * Set Next Action to Call _MRG_a
                                         */
                                        kjsvpt->nextact = kjsvpt->nextact |
                                                                   M_RGAON;

                                        break;

                                /*
                                 *      "Kanji" Registration Mode
                                 */
                                case K_KADIC:

                                        /*
                                         * Set Next Action to Call _MRM_rs
                                         */
                                        kjsvpt->nextact = kjsvpt->nextact &
                                                                 ~M_RMRSON;

                                        /*
                                         * Set Next Action to Call _MRG_b
                                         */
                                        kjsvpt->nextact = kjsvpt->nextact |
                                                                   M_RGBON;

                                        break;
                               };

                        /*
                         *      Dictionary Registration
                         *              Message Reset Successful Code
                         */
                        rc = IMRMRSTI;
                   };
           };

snap3(SNAP_KCB | SNAP_KMISA,SNAP_MRM_rs,"return6 _MRM_rs");

        return( rc );
}
