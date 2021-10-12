static char sccsid[] = "@(#)32	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Jclr.c, libKJI, bos411, 9428A410j 7/23/92 03:17:24";
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
 * MODULE NAME:         _Jclr
 *
 * DESCRIPTIVE NAME:    Kanji input field clear.
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
 * FUNCTION:            Initialize input field and KKC interface
 *                      area in KMISA.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        644 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Jclr
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Jclr(pt)
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
 *                              _Mkkcclr()  : KKC Interface area close.
 *                      Standard Library.
 *                              memset()    : change charcter string.
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
 *                              *kjsvpt  : pointer to KMISA.
 *                              indlen   : length of shift indicater.
 *                              cnvsts   : conversion status.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              kmact    : Active/Inactive of input field.
 *                              realcol  : max number of available column.
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Control Block(KCB).
 *                              string   : pointer to KCB string area.
 *                              hlatst   : pointer to KCB highlight
 *                                         attribute area.
 *                              curcol   : input field cursor column.
 *                              currow   : input field cursor row.
 *                              chpos    : change character position.
 *                              chlen    : change character length.
 *                              lastch   : input field lastchar position.
 *                              axuse1   : used flag(No.1) of auxiliary area.
 *                              cnvsts   : conversion status.
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
 * CHANGE ACTIVITY:     Bug Fix.
 *                      11/09,1987 Invalid Redraw Range.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */

/*
 *      include Standard.
 */
#include <stdio.h>      /* Standard I/O Package.                        */
#include <memory.h>     /* System memory operation utility              */

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
 *      This module initializes input field and KKC
 *      interfase area in KMISA.
 */

int _Jclr(pt)
KCB     *pt ;         /*  pointer to Kanji Control Block        */
{
        char *memset();          /*   change charcter string            */
        extern  int     _Mkkcclr(); /* interface area clear             */
        KMISA   *kjsvpt;         /*   pointer to kanji moniter ISA      */
        int     rc ;             /*   return code                       */
        uchar    *str_adr;       /*   top address of charcters string   */
        int     counter ;        /*   Space character counter           */
        int     i       ;        /*   loop counter                      */


/* ### */
        CPRINT(======== start _Jclr ===========);
snap3( SNAP_KCB | SNAP_KMISA , SNAP_Jclr , "start _Jclr" );

         kjsvpt = pt->kjsvpt;  /*  set pointer to kanji moniter ISA  */
         rc = IMSUCC;          /*  set return value                  */

        /* 1.
         *   initialize  !  character of input area.
         *   initialize  !  display attributes.
         */

                            /*  If Active  */
         if(kjsvpt->kmact == K_IFACT)  {
                            /*  Set data string address  */

         if ( (kjsvpt->convlen > 0 ) ||
              (kjsvpt->msetflg > 0 ) ||
              ( pt->kbdlok == K_KBLON ))
         {
                rc = _Mkkcclr(pt);
                pt->cnvsts = K_CONVF;
         };

                str_adr = pt->string;
                            /*  Set counter      */
                counter = (kjsvpt->realcol - pt->indlen) / C_DBCS;
                for(i = 0; i < counter; i++)  {

                        *str_adr = C_SPACEH;    /* Space High Byte        */
                        str_adr  = str_adr + 1; /* Next position of pointer */
                        *str_adr = C_SPACEL;    /* Space Lower Byte       */
                        str_adr  = str_adr + 1; /* Next position of pointer */

                };
         };
              /*   change charcter string                     */
              /*   Initialize highlight attribute area in KCB */
         (void)memset(pt->hlatst,K_HLAT0,kjsvpt->realcol);

        /* 2.
         *   initialize  !  cursor position ,last character position,
         *                  change position and length.
         */
         pt->chpos  = C_COL;    /*  Initialize change position of charcter*/
         /*****************************************************************/
         /*#(B) Bug Fix.  Mon Nov 09,1987                                 */
         /*     Modify Reazon.                                            */
         /*             Kanji Control Block Last Character Counts Allways */
         /*             Clear Zero.                                       */
         /*     Change Source Code.                                       */
         /*             pt->chlen = 0;   ---> pt->chlen = pt->lastch;     */
         /*****************************************************************/
         pt->chlen  = pt->lastch;
                                /*  Initialize change length of charcter  */
         pt->curcol = C_COL;    /*  Initialize cursor position            */
         pt->currow = C_ROW;    /*  Initialize cursor position            */
         pt->lastch = C_COL;    /*  Initialize last position of character */

        /* 4.
         *   initialize  !  KKC interface area in KMISA.
         */
                                   /*  If Conversion has been finished  */

        /* 5.
         *  Return Value.
         */

/* ### */
snap3( SNAP_KCB | SNAP_KMISA , SNAP_Jclr , "######## end _Jclr ########" );

         return(rc);
}
