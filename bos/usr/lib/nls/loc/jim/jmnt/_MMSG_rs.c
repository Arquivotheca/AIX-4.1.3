static char sccsid[] = "@(#)71	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_MMSG_rs.c, libKJI, bos411, 9428A410j 7/23/92 03:19:51";
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
 * MODULE NAME:         _MMSG_rs
 *
 * DESCRIPTIVE NAME:    Message reset.
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
 * FUNCTION:            1.Check the code of depressed key.
 *                      2.Call subroutine to reset message area and to
 *                        change mode, if the key is assigned to reset.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1028 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _MMSG_rs
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _MMSG_rs(pt,mode)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *                      mode    :Reset mode.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC :Success of Execution.
 *
 * EXIT-ERROR:          IMMSGRSE : Invalid pseudo code input.
 *                      IMMSGRSI : Information of message reset.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _MM_rtn() : Mode switching routine.
 *                              _Mreset() : Reset mode.
 *                              _Mregrs() : Reset dictionary registration.
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
 *                              *kjsvpt : pointer to KMISA.
 *                              type    : input code type.
 *                              code    : input code.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              kmpf[0] : Kanji Moniter Profile save area.
 *                              nextact : next function.
 *                              kjin    : Input key.
 *                              reset   : reset key.
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Control Block(KCB).
 *                              NA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              nextact : next function.
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
 *  This module does,
 *      1.CHeck type of input code.
 *      2.Check input code.
 *      3.Call subroutine to reset or to beep, in accordance with input
 *        code.
 */

int _MMSG_rs(pt,mode)
KCB     *pt ;           /*  pointer to Kanji Control Block        */
uchar   mode;           /*  Reset mode                            */
{
        KMISA   *kjsvpt;         /*   pointer to kanji moniter ISA      */
        extern  int     _MM_rtn(); /* Mode switching routine.           */
        extern  int     _Mreset(); /* Reset mode.                       */
        extern  int     _Mregrs(); /* Reset dictionary registration.    */
        int     rc ;             /*   return code                       */

        short   rstflag;         /*  Reset flag                         */
        short   beepflg;         /*  Beep flag                          */

        /* ### */
        CPRINT(_MMSG_rs);
        snap3( SNAP_KCB | SNAP_KMISA , SNAP_MMSG_rs , "start" );

         kjsvpt  = pt->kjsvpt; /*  Set pointer to kanji moniter ISA  */
         rc      = IMSUCC;     /*  Set return value                  */
         rstflag = C_SWOFF;    /*  Initialize reset flag             */
         beepflg = C_SWOFF;    /*  Initialize beep flag              */

        /* 1.
         *   check  !  type of input code (KCB).
         */
        if( pt->type != K_INESCF ) { /*  Not Pseudo code                */

            _MM_rtn(pt, A_BEEP);     /*  Mode switching Routine to beep */
            rc = IMMSGRSE;           /*  Set error return code          */

            /* ### */
            snap3( SNAP_KCB | SNAP_KMISA , SNAP_MMSG_rs , "error" );

            return(rc);
        };

        /* 2.
         *   check  !  Input code (KCB).
         */

        switch(pt->code)  {

            /* 2.1.
             *   case  !  Enter.
             */
            case P_ENTER :

                         /*  Check PROFILE customized to reset  */
                if (  ((kjsvpt->kmpf[0].kjin  & K_DAENT) == K_DAENT) ||
                      ((kjsvpt->kmpf[0].reset & K_REENT) == K_REENT)  ) {

                    rstflag = C_SWON;      /*  Set reset flag      */

                } else {

                    beepflg = C_SWON;      /*  Set beep flag       */
                };
                break;

            /* 2.2.
             *   case  !  Action.
             */
            case P_ACTION :

                         /*  Check PROFILE customized to reset  */
                if (  ((kjsvpt->kmpf[0].kjin  & K_DAACT) == K_DAACT) ||
                      ((kjsvpt->kmpf[0].reset & K_REACT) == K_REACT)  ) {

                    rstflag = C_SWON;      /*  Set reset flag      */

                } else {

                    beepflg = C_SWON;      /*  Set beep flag       */
                };
                break;

            /* 2.3.
             *   case  !  CR.
             */
            case P_CR :

                         /*  Check PROFILE customized to reset  */
                if (  ((kjsvpt->kmpf[0].kjin  & K_DACR) == K_DACR) ||
                      ((kjsvpt->kmpf[0].reset & K_RECR) == K_RECR)  ) {

                    rstflag = C_SWON;      /*  Set reset flag      */

                } else {

                    beepflg = C_SWON;      /*  Set beep flag       */
                };
                break;

            /* 2.4.
             *   case  !  Reset.
             */
            case P_RESET :

                rc = _Mreset(pt, M_ALLRST);  /*  Reset mode           */

                if ( rc == IMMSGRSI ) {      /*  Check return code    */

                              /*  Reset flag of next function         */
                    kjsvpt->nextact = kjsvpt->nextact & ~M_MGRSON;

                    rc = IMMSGRSI;           /*  Set return code      */

                                             /*  Check return code    */
                } else if ( rc == IMRGRSTI ) {

                    rc = IMMSGRSI;           /*  Set return code      */

                                             /*  Check return code    */
                } else if ( rc == IMSUCC ) {

                    beepflg = C_SWON;        /*  Set beep flag        */
                } else {

                    rc = IMMSGRSE;           /*  Set return code      */
                };

                break;

            /* 2.5.
             *   case  !  Tab.
             */
            case P_TAB :

                beepflg = C_SWON;            /*  Set beep flag   */

                break;

            /* 2.6.
             *   case  !  Back Tab.
             */
            case P_BTAB :

                beepflg = C_SWON;            /*  Set beep flag   */

                break;

            /* 2.7.
             *   case  !  default.
             */
            default :

                beepflg = C_SWON;            /*  Set beep flag   */

                break;
        };

            /* 3.
             *   check  !  flags and call subroutine.
             */
        if ( rstflag ) {       /*  Check reset flag   */

            switch( mode )  {  /*  Check current mode of input parameter */

                        /*
                         *  Case  :  All candidates,
                         *  or conversion mode switching mode.
                         */
                case K_MSGOTH :

                    _Mreset(pt, M_MSGRST);   /*  Reset message area   */

                              /*  Reset flag of next function         */
                    kjsvpt->nextact = kjsvpt->nextact & ~M_MGRSON;

                    break;

                        /*  Case  :  dictionary registration  */
                case K_MSGDIC :

                    _Mregrs(pt);    /*   Reset dictionary registration. */

                    break;
            };

            rc = IMMSGRSI;   /*  Set error return code   */
        };

        if ( beepflg ) {   /*  Check beep flag  */

            _MM_rtn( pt, A_BEEP );    /*  Mode switching routine to beep */

            rc = IMMSGRSE;   /*  Set error return code   */
        };

        /* ### */
        snap3( SNAP_KCB | SNAP_KMISA , SNAP_MMSG_rs , "end");

        return( rc );

}
