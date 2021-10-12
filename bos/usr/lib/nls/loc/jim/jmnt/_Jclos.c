static char sccsid[] = "@(#)31	1.2.1.1 src/bos/usr/lib/nls/loc/jim/jmnt/_Jclos.c, libKJI, bos411, 9428A410j 7/23/92 03:17:21";
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
 * MODULE NAME:         _Jclos
 *
 * DESCRIPTIVE NAME:    Kanji Close.
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
 * FUNCTION:           1.Get the pointer of KKC and TRB by input parameter.
 *                     2.Check PROFILE(learning) and call dictionary
 *                       learning area writing routine if necessary.
 *                     3.Close KKC routines.
 *                     4.Free KCB.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1100 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Jclos
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Jclos(pt)
 *
 *  INPUT:              pt      :Pointer to kanji Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC :Success of Execution.
 *
 * EXIT-ERROR:          KMMDEALE : Memory deallocation error.
 *
 *
 * EXTERNAL REFERENCES: See Below
 *  ROUTINES:           Internal Subroutines.
 *                              _Jclos1()   :KKC Return Code Convert to Kanji
 *                                           Monitor Return Code.
 *                      Kanji Project Subroutines.
 *                              _Kcclose()  : KKC close.
 *                              _Kcdctlw()  : Dictionary leaning area write.
 *                      Standard Library.
 *                              free()      : Memory free.
 *                      Advanced Display Graphics Support Library(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      See Below.
 *
 *   INPUT:             Kanji Monitor Control Block(KCB).
 *                              length    : length of DECB.
 *                              *kjsvpt   : pointer to KMISA.
 *                              *_Tacep   : pointer to Trace area.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              kbdmode   : key board mode.
 *                              *kkcbsvpt : pointer to KKCB.
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            Kanji Monitor Control Block(KCB).
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
 *                              KKCPHYER:Check KKC Phigical Error.
 *                              KKCMAJOR:Get KKC Major Error Code.
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
#include <memory.h>     /* System memory operation utylity              */

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
static  int     _Jclos1();  /* KKC Return Code Convert to Kanji */
                            /* Monitor Return Code.             */

/*
 *  This module dose,
 *      1.Get the pointer of KKC and TRB by input parameter
 *      2.Check PROFILE(learning) and call dictionary
 *        learning area writing routine if necessary.
 *      3.Close KKC routines.
 *      4.Free KCB.
 */

int _Jclos(pt)

KCB     *pt ;         /*  pointer to Kanji Control Block        */
{
        extern  int     _Kcclose(); /* KKC close                        */
        extern  int     _Kcdctlw(); /* Dictionary learning area write   */
        KMISA   *kjsvpt;       /*   pointer to kanji moniter ISA        */
        KKCB    *kkcbsvpt;     /*   pointer to Kana Kanji control Block */
        int     kcbmsg;         /* KCB Header Message Size.             */
        int     rc ;           /*   return code                         */
        char    *freebase;      /* Free the Memory Block Address.       */
/* ### */
snap3( SNAP_KCB | SNAP_KMISA , SNAP_Jclos , "start _Jclos" );

         kjsvpt = pt->kjsvpt;  /*  set pointer to kanji moniter ISA  */
         rc = KMSUCC;          /*  set return value                  */

        /* 1.
         *   get  !  pointer to KKCB by input parameter(pt).
         */
        kkcbsvpt = pt->kjsvpt->kkcbsvpt;        /* Get pointer to KKCB  */

        /* 2.
         *   check  !  learning(PROFILE).
         */
        if(kjsvpt->kmpf[0].learning == K_LEAON) {

                /*
                 *  Write Dictionary learning to disk file.
                 */

                kjsvpt->kkcrc = _Kcdctlw(kkcbsvpt);

                /*
                 *      Get KKC Return Code.
                 */
                if ( kjsvpt->kkcrc != K_KCSUCC )
                        rc = _Jclos1( kjsvpt->kkcrc );
        };


        /* 3.
         *      Close Request to Kana/Kanji Conversion Routine.
         *      CALL KKC:Close Kana/Kanji Routine Interface.
         */
        if( rc != KMSUCC ) {
                (void)_Kcclose(kkcbsvpt);
        } else {
                /*
                 *      Call KKC Close.
                 */
                kjsvpt->kkcrc = _Kcclose(kkcbsvpt);
                /*
                 *      Get KKC Return Code.
                 */
                if( kjsvpt->kkcrc != K_KCSUCC )
                        rc = _Jclos1( kjsvpt->kkcrc );
        };

        /* 4.
         *   Deallocate Kanji Controle Block & Kanji Monitor Internal
         *   Save Area & Trace Control Block & Field Save Block
         *   and Internal Work Space.
         */
        /*  Check pointer to KCB  */
        if(pt != NULL) {
                kcbmsg = (sizeof(K_KCBMSG)/sizeof(long))*sizeof(long);
                freebase= (char *)pt;
                freebase -= kcbmsg;
                pt->id  = ~pt->id;              /* Delete KCB ID.       */
                (void)free( freebase );         /*  Free KCB            */
        } else {
                if( rc == KMSUCC )
                        rc = KMMDEALE;          /*   Set return code    */
        };

/* ### */
snap3( SNAP_KCB | SNAP_KMISA , SNAP_Jclos , "####### end _Jclos ########" );

         return(rc);
}

/*
 *      Kana/Kanji Conversion Routine Return Code Convert to
 *      Kanji Monitor Error Code.
 */
static int _Jclos1( kkcrc )
int     kkcrc;
{
        int     ret_cod;

        /*
         *      Return Code Initialize.
         */
        ret_cod = KKSUCC;

        if( KKCPHYER( kkcrc ) ) {
                /*
                 *      Phigical Error Occure.
                 */
                switch( KKCMAJOR( kkcrc ) ) {
                /*
                 * Physical error on System Dictionary.
                 */
                case K_KCSYPE :
                        ret_cod = KKSYDCOE;     /* Set return code.     */
                        break;
                /*
                 * Physical error on User   Dictionary.
                 */
                case K_KCUSPE :
                        ret_cod = KKUSDCOE;     /* Set return code.     */
                        break;
                /*
                 * Physical error on Adjunct Dictionary.
                 */
                case K_KCFZPE :
                        ret_cod = KKFZDCOE;     /* Set return code.     */
                        break;
                /*
                 * Memory allocation error.
                 */
                case K_KCMALE :
                        ret_cod = KKMALOCE;     /* Set return code.     */
                        break;
                /*
                 *      If Unknown Error Accept then 'KKC Return Code'
                 *      Return to Application.
                 */
                default:
                        ret_cod = KKFATALE;     /* Set return code.     */
                        break;
                };
        };

        /*
         *      Return to Caller.
         */
        return( ret_cod );
}
