static char sccsid[] = "@(#)29	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mlock.c, libKJI, bos411, 9428A410j 7/23/92 03:23:54";
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
 * MODULE NAME:         _Mlock
 *
 * DESCRIPTIVE NAME:    key board lock.
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
 * FUNCTION:            Set the keyboard locked.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        552 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mlock
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mlock(pt)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC :Success of Execution.
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _Mindset() :Set indicator.
 *                      Standard Library.
 *                              NA.
 *                      Advanced Display Graphics Support Library(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      See Below.
 *
 *   INPUT:             Kanji Monitor Control Block(KCB).
 *                              *kjsvpt  : pointer to KMISA
 *                              kbdlok   : keyboard lock flag
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              kblock   : keyboard lock
 *                              nextact  : next function
 *
 *   OUTPUT:            Kanji Monitor Control Block(KCB).
 *                              kbdlok   : keyboard lock flag
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              nextact  : next function
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
 *      This module sets the keyboard locked.
 */

int _Mlock(pt)
KCB     *pt ;         /*  pointer to Kanji Control Block        */
{
        KMISA   *kjsvpt;                /* pointer to kanji moniter ISA */
        extern  int     _Mindset();     /* Set indicator                */
        int     rc;                     /* return code                  */


snap3( SNAP_KCB | SNAP_KMISA , SNAP_Mlock , "start" );

         kjsvpt = pt->kjsvpt;  /* set pointer to kanji moniter ISA      */
         rc = IMSUCC;          /* set return value                      */



        /* 1.
         *      Set keyboard lock flag.
         */
        pt->kbdlok = K_KBLON;



        /* 2.
         *   Check keyboard lock permission in PROFILE.
         */

        if ( kjsvpt->kmpf[0].kblock == K_KBLON )  {


                /* 2.1.
                 *   Set next function.
                 */
                kjsvpt->nextact = kjsvpt->nextact | M_KLRSON ;


                /* 2.2.
                 *   Indicate locked condition.
                 */
                rc = _Mindset(pt, M_INDL);
        };



        /* 3.
         *   Beep.
         */
        rc = _MM_rtn(pt, A_BEEP);



        /* 4.
         *   Return Value.
         */

snap3( SNAP_KCB | SNAP_KMISA , SNAP_Mlock , "end" );

        return( IMSUCC );
}
