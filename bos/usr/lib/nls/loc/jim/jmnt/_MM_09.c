static char sccsid[] = "@(#)72	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_MM_09.c, libKJI, bos411, 9428A410j 7/23/92 03:19:54";
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
 * MODULE NAME:         _MM_09
 *
 * DESCRIPTIVE NAME:    Mode switching ( Dictionary registration )
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
 * FUNCTION:            Check the environment and start dictionary
 *                      registration if permitted.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        820 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _MM_09
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _MM_09(pt)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC :Success of Execution.
 *
 * EXIT-ERROR:          IMNOTIFE : Insufficient input field length.
 *                      IMNOTRGE : Profile parameter is not customized for
 *                                 Registration of dictionary.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _Mifmst() : Input area message set.
 *                              _MK_rtn() : Kana Kanji conversion interface
 *                                          routine.
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
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              regist  : Dictionary registration.
 *                              realcol : max number of available column.
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
 *                              nextact  : next function.
 *                              kkcrmode : dictionary registration mode.
 *                              kkmode1  : current conversion mode.
 *                              kkmode2  : previous conversion mode.
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
 *      1.Check PROFILE.
 *      2.Check input field length.
 *      3.Display prompt for yomi registration.
 *      4.Set dictionary registration mode flag in KMISA.
 */

int _MM_09(pt)
KCB     *pt ;         /*  pointer to Kanji Control Block        */
{
        KMISA   *kjsvpt;         /*   pointer to kanji moniter ISA      */
        int     _Mifmst();       /* input area message set            */
        int     rc    ;          /*  return code                        */
        int     maxlen;          /*  max character length               */
        int     msglen;          /*  message character length           */


/* ### */
        CPRINT(======== start _MM_09 ===========);
snap3( SNAP_KCB | SNAP_KMISA , SNAP_MM_09 , "start _MM_09" );

         kjsvpt = pt->kjsvpt;  /*  set pointer to kanji moniter ISA  */
         rc = IMSUCC;          /*  set return value                  */

        /* 1.
         *   check  !  Dictionary registration permission from PROFILE
         */
         if((kjsvpt->kmpf[0].regist) == K_DICNG)  {

            rc = IMNOTRGE;  /*  Set error return code  */

                            /*  Check beep flag  */
                        if ( kjsvpt->kmpf[0].beep == K_BEEPON )
                                pt->beep = K_BEEPON;

/* ### */
snap3( SNAP_KCB | SNAP_KMISA , SNAP_MM_09 , "end ---No.1 ---- _MM_09 " );

            return(rc);
         };

        _MK_rtn( pt , A_CNVDEC );  /* call _MK_rtn for fixing conversion */

        /* 2.
         *   check  !  input field length.
         */
              /*  Maximum length of "YOMI>" or lenght of "KANJI>"  */
         maxlen = MAX( (sizeof(M_RGYMSG) - 1),(sizeof(M_RGKMSG) - 1) );

             /* Check length of input field  */
         if((M_RGIFL + maxlen) > (kjsvpt->realcol - pt->indlen)) {

                                   /* Display error message   */
            _Mifmst(pt, K_MSGDIC, C_FAUL, C_FAUL, C_COL, C_DBCS
                        ,sizeof(M_RGERMG) - 1, M_RGERMG);

                         /*  Set next function   */
            (kjsvpt->nextact)  = ((kjsvpt->nextact) | M_RGCON);

                         /*  Set dictionary registration mode  */
            (kjsvpt->kkcrmode) = K_MEDIC ;

                         /*  Set current conversion mode  */
            (kjsvpt->kkmode1)  = A_DCREGA;

                         /*  Set previous conversion mode */
            (kjsvpt->kkmode2)  = A_1STINP;

            rc = IMNOTIFE;    /*  Set error return code  */

/* ### */
snap3( SNAP_KCB | SNAP_KMISA , SNAP_MM_09 , "end ---No.2 ---- _MM_09 " );

            return(rc);
         };

        /* 3.
         *   Display prompt for yomi registration.
         */
               /*  Set length of message to be display  */
         msglen = sizeof(M_RGYMSG) - 1;

                                   /* Display error message   */
         _Mifmst(pt, K_MSGDIC, msglen, C_ROW, msglen,msglen+M_RGYLEN
                ,msglen, M_RGYMSG);

        /* 4.
         *   set  !  dictionary registration mode flag of KMISA.
         */

                         /*  Set next function   */
         (kjsvpt->nextact)  = ((kjsvpt->nextact) | M_RGAON);

                         /*  Set dictionary registration mode  */
         (kjsvpt->kkcrmode) = K_REDIC ;

                         /*  Set current conversion mode  */
         (kjsvpt->kkmode1)  = A_DCREGA;

                         /*  Set previous conversion mode */
         (kjsvpt->kkmode2)  = A_1STINP;

        /* 5.
         *   Return Value.
         */

/* ### */
snap3( SNAP_KCB | SNAP_KMISA , SNAP_MM_09 , "end ---No.3 ---- _MM_09 " );

         return(rc);
}
