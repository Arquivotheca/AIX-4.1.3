static char sccsid[] = "@(#)84	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Maxcrmv.c, libKJI, bos411, 9428A410j 7/23/92 03:20:41";
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
 * MODULE NAME:         _Maxcrmv
 *
 * DESCRIPTIVE NAME:    Auxiliary area cursor move.
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
 * FUNCTION:            Move pseudo cursor in auxiliary area.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        512 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Maxcrmv
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Maxcrmv(pt,nxtcol,nxtrow)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *                      nxtcol  :Next cursor column position.
 *                      nxtrow  :Next cursor row position.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC :Success of Execution.
 *
 * EXIT-ERROR:          KMCROTW : Invalid cursor position.
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
 *                      Kanji Monitor Control Block(KCB).
 *                              ax1col   : Number of columns in auxiliary
 *                                         area No1.
 *                              ax1row   : number of row in auxiliary area
 *                                         No1.
 *                              *kjsvpt  : pointer to KMISA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              curleft  : cursor move left limit.
 *                              curright : cursor move right limit.
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Control Block(KCB).
 *                              cura1c : Cursor columns in auxiliary
 *                                       area No1.
 *                              cura1r : Cursor row in auxiliary area
 *                                       No1.
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


/*
 *      Copyright Identify.
 */
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1988           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *   This module dose,
 *       1.Check input parameter.
 *       2.Set cursor position in auxiliary area.
 */

int _Maxcrmv(pt,nxtcol,nxtrow)
KCB     *pt ;         /*  pointer to Kanji Control Block        */
short   nxtcol;       /*  next cursor column position           */
short   nxtrow;       /*  next cursor row    position           */
{
        KMISA   *kjsvpt;         /*   pointer to kanji moniter ISA      */
        int     rc;              /*   return code                       */


/* ### */
        CPRINT(======== start _Maxcrmv ===========);
snap3( SNAP_KCB | SNAP_KMISA , SNAP_Maxcrmv , "start _Maxcrmv" );

         kjsvpt = pt->kjsvpt;  /*  set pointer to kanji moniter ISA  */
         rc = IMSUCC;          /*  set return value                  */

        /* 1.
         *   check  !  input parameter of cursor position is
         *             inside auxiliary area.
         */
                      /*  Check next cursor column position  */
         if(nxtcol < 0 || nxtcol > pt->ax1col)   {

              rc = KMCROTW;    /*  Set return code  */

/* ### */
snap3( SNAP_KCB | SNAP_KMISA , SNAP_Maxcrmv , "end ---No.1 ---- _Maxcrmv " );

              return(rc);

         };

                      /*  Check next cursor row position  */
         if(nxtrow < 0 || nxtrow > pt->ax1row)   {

              rc = KMCROTW;    /*  Set return code  */

/* ### */
snap3( SNAP_KCB | SNAP_KMISA , SNAP_Maxcrmv , "end ---No.2 ---- _Maxcrmv " );

              return(rc);

         };
                      /*  Check cursor move left limit and right limit   */
         if(kjsvpt->curleft != C_FAUL
           && kjsvpt->curright != C_FAUL) {
                      /*  Check next cursor position   */
                if(nxtcol < kjsvpt->curleft
                  || nxtcol > kjsvpt->curright)   {

                        rc = KMCROTW;    /*  Set return code  */

/* ### */
snap3( SNAP_KCB | SNAP_KMISA , SNAP_Maxcrmv , "end ---No.3 ---- _Maxcrmv " );

                        return(rc);

                };

         };

        /* 2.
         *   set  !  next cursor position to KCB.
         */
         pt->cura1c = nxtcol;     /*  Set cura1c of KCB  */
         pt->cura1r = nxtrow;     /*  Set cura1c of KCB  */

        /* 3.
         *   Return Value.
         */


/* ### */
snap3( SNAP_KCB | SNAP_KMISA , SNAP_Maxcrmv , "end ---No.4 ---- _Maxcrmv " );

         return(rc);
}
