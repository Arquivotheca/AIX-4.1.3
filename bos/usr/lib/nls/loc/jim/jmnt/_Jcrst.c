static char sccsid[] = "@(#)33	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Jcrst.c, libKJI, bos411, 9428A410j 7/23/92 03:17:27";
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
 * MODULE NAME:         _Jcrst
 *
 * DESCRIPTIVE NAME:    Forced cursor set.
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
 * FUNCTION:            Change the cursor position and set echo information.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        524 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Jcrst
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Jcrst(pt)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC :Success of Execution.
 *
 * EXIT-ERROR:          KMIFNAE  : DBCS input field is not active.
 *                      KMIVCURE : Invalid cursor position.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _Mcrmv()  : cursor move.
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
 *                              setcsc  : input field cursor column
 *                                        (for editer).
 *                              indlen  : length of shift indicater.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              realcol : max number of available column.
 *                              kmact   : Active/Inactive of input field.
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
 *  This module does,
 *      1.Check input field is active.
 *      2.Check cursor position after moving is in input
 *        field.
 *      3.Call _Mcrmv.
 */

int _Jcrst(pt)
KCB     *pt ;         /*  pointer to Kanji Control Block        */
{
        KMISA   *kjsvpt;      /*   pointer to kanji moniter ISA      */
        extern  int     _Mcrmv(); /* cursor move                     */
        int     rc ;          /*   return code                       */
        short   nextpos;      /*   next cursor column position       */


/* ### */
        CPRINT(======== start _Jcrst ===========);
snap3( SNAP_KCB | SNAP_KMISA , SNAP_Jcrst , "start _Jcrst" );

         kjsvpt = pt->kjsvpt;  /*  set pointer to kanji moniter ISA  */
         rc = IMSUCC;          /*  set return value                  */

        /* 1.
         *   check  !  not active field.
         */
                            /* If Inactive  */
         if(kjsvpt->kmact == K_IFINAC)  {
                rc = KMIFNAE;         /*  Set return code   */

/* ### */
snap3( SNAP_KCB | SNAP_KMISA , SNAP_Jcrst , "end _Jcrst --- No.1 ---- " );

                return(rc);
         };

        /* 2.
         *   check  !  cursor position is inside input field.
         */
                          /*  Check cursor position  */
         if(pt->setcsc < 0)  {
                rc = KMIVCURE;    /*  Set return code  */

/* ### */
snap3( SNAP_KCB | SNAP_KMISA , SNAP_Jcrst , "end _Jcrst --- No.2 --- " );

                return(rc);
         };

                             /*  Check shift indicator length  */
         if(pt->indlen <= 0)  {
                             /*  In case of setcsc of KCB greater than
                                 realcol of KMISA.                     */
                if(pt->setcsc > kjsvpt->realcol)  {
                        rc = KMIVCURE;   /*  Set return code   */

/* ### */
snap3( SNAP_KCB | SNAP_KMISA , SNAP_Jcrst , "end _Jcrst --- No.3 --- " );

                        return(rc);
                };
         };

                             /*  Check shift indicator length  */
         if(pt->indlen > 0)  {
                             /*  In case of setcsc of KCB greater
                                 than realcol of KMISA
                                 - shift indicator length + 2.     */
                if(pt->setcsc > kjsvpt->realcol
                              - pt->indlen + C_DBCS) {
                        rc = KMIVCURE;     /*  Set return code   */

/* ### */
snap3( SNAP_KCB | SNAP_KMISA , SNAP_Jcrst , "end _Jcrst --- No.4 --- " );

                        return(rc);
                };
         };

        /* 3.
         *   call  !  _Mcrmv().
         */
         nextpos = pt->setcsc;       /*  Set next cursor column position */
         rc = _Mcrmv(pt, nextpos);   /*  cursor move.                    */

        /* 4.
         *   Return Value.
         */

/* ### */
snap3( SNAP_KCB | SNAP_KMISA , SNAP_Jcrst , "end _Jcrst --- No.5 --- " );

         return(rc);
}
