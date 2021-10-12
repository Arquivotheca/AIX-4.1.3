static char sccsid[] = "@(#)90	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mcrmv.c, libKJI, bos411, 9428A410j 7/23/92 03:21:06";
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

 /********************* START OF MODULE SPECIFICATIONS **********************  *
 * MODULE NAME:         _Mcrmv
 *
 * DESCRIPTIVE NAME:    move cursor .
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
 * FUNCTION:            Set highlight attribute and other parameters in
 *                      control block in accordance with inputted next
 *                      cursor position.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        3484 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mcrmv
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mcrmv(pt,nxtcol)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *                      nxtcol  :next cursor column position
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC :Success of Execution.
 *
 * EXIT-ERROR:          KMCROTE : Invalid cursor position.
 *                      KMCROTW : Invalid cursor position.(Warning)
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _Msetch() :Set changed position and changed
 *                                         length for display.
 *                              _Mckbk()  :Check a phrase.
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
 *                              *kjsvpt  : pointer to KMISA
 *                              curcol   : input field cursor column
 *                              *hlatst  : pointer to highlight attribute
 *                                         area.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              kmpf[0]  : Kanji Moniter Profile save area
 *                              realcol  : max number of available column
 *                              curright : cursor move right limit
 *                              curleft  : cursor move left limit
 *                              cursout  : cursor move permission outside
 *                                         field
 *                              cconvpos : current conversion position
 *                              cconvlen : current conversion length
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Control Block(KCB).
 *                              chpos    : changed character position
 *                              chlen    : changed character length
 *                              curcol   : input field cursor column
 *                              *kjsvpt  : pointer to KMISA
 *                              *hlatst  : pointer to highlight attribute
 *                                         area
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              cconvpos : current conversion position
 *                              cconvlen : current conversion length
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
 * CHANGE ACTIVITY:     Tuesday Aug. 23 1988 Satoshi Higuchi
 *                      Changed some statements for debugging
 *                      See problem collection sheet P-1 and
 *                      Monitor Improvement Spec. 3.2.2.
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

static void _Maust();
static void _Maset();

/*
 *   This module dose,
 *      1.Check inputted next cursor position.
 *      2.Set highlight attribute , current conversion position and
 *        current conversion length according to current cursor position
 *        and next cursor position.
 */

int _Mcrmv(pt,nxtcol)

KCB     *pt ;         /*  pointer to Kanji Control Block        */
short   nxtcol;       /*  next cursor column position           */

{
        KMISA   *kjsvpt;   /*   pointer to kanji moniter ISA            */
        extern  int     _Msetch(); /* Set change position and length    */
                                   /* for display.                      */
        extern  int     _Mckbk();  /* Check a phrase.                   */
                void    _Maust();  /* Set K_HLAT2 Routine.              */
                void    _Maset();  /* Set K_HLAT3 Routine.              */
        int     rc;        /*  return code                              */
        uchar   ck_cur ;   /*  highlight attribute of current cursor    */
                           /*  position                                 */
        uchar   ck_nxt ;   /*  highlight attribute of next cursor       */
                           /*  position                                 */
        int     icase  ;   /*  highlight attribute case                 */
        uchar   ck_pre ;   /*  highlight attribute of previous cursor   */
                           /*  position                                 */
        int     h      ;   /*  loop counter                              */
        int     hh     ;   /*  loop counter                              */
        short   ipos   ;   /*  Starting position of changed character    */
        short   ist    ;   /*  phrase start position                     */
        short   iancf  ;   /*  attribute no change flag                  */
        short   ilen1  ;   /*  Length of changed character               */
        short   ilen2  ;   /*  Length of changed character               */
        short   klen   ;   /*  Length of changed character               */
        short   klen1  ;   /*  Length of changed character               */
        short   kst    ;   /*  phrase start position                     */
        short   ist1   ;   /*  phrase start position                     */
        short   ist2   ;   /*  phrase start position                     */
        short   ied    ;   /*  phrase end position                       */
        short   ked    ;   /*  phrase end position                       */
        short   ied1   ;   /*  phrase end position                       */
        short   ied2   ;   /*  phrase end position                       */
        short   prepos;    /*  next cursor position                      */
        short   idir;      /*  cursor position flag                      */
/*----------------------------------------------------------------------*/
/*      #(B) Changed by S,Higuchi on Aug. 23 1988                       */
/*      OLD     char *errid;                                            */
/*      NEW     #ifdef NDEBUGPRINT                                      */
/*                  char *errid;                                        */
/*              #endif                                                  */
/*----------------------------------------------------------------------*/
#ifdef NDEBUGPRINT
        char    *errid;    /*  pointer to error return code for snap     */
#endif
        uchar   extjmp;    /*  while loop switch flag                    */

        CPRINT( start _Mcrmv );

        snap3( SNAP_KCB | SNAP_KMISA , SNAP_Mcrmv , "start _Mcrmv" );

         kjsvpt = pt->kjsvpt;  /*  set pointer to kanji moniter ISA  */

        /* 1.
         *   check  !  Next cursor position is in the field.
         */

         rc = IMSUCC;          /*  set return value                  */
         extjmp = C_SWOFF;     /*  Initialize while loop switch      */
/*----------------------------------------------------------------------*/
/*      #(B) Changed by S,Higuchi on Aug. 23 1988                       */
/*      OLD     errid = "normal";                                       */
/*      NEW     #ifdef NDEBUGPRINT                                      */
/*                  errid = "normal";                                   */
/*              #endif                                                  */
/*----------------------------------------------------------------------*/
#ifdef NDEBUGPRINT
         errid = "normal";     /*  Initialize return code for snap   */
#endif

  while ( extjmp == C_SWOFF) {

                /*  next cursor position == current curcol position  */
         if(nxtcol == pt->curcol)  {
              rc = KMCROTE;          /*  Set return code  */
/*----------------------------------------------------------------------*/
/*      #(B) Changed by S,Higuchi on Aug. 23 1988                       */
/*      OLD     errid = "KMCROTE";                                      */
/*      NEW     #ifdef NDEBUGPRINT                                      */
/*                  errid = "KMCROTE";                                  */
/*              #endif                                                  */
/*----------------------------------------------------------------------*/
#ifdef NDEBUGPRINT
              errid = "KMCROTE";     /*  Set return code  */
#endif
              break;
         };
                       /*  Check cursor move permission outside the field */
         if(kjsvpt->kmpf[0].cursout == K_CURIN)  {
              if(nxtcol < 0 || nxtcol > kjsvpt->realcol)  {
                   rc = KMCROTW;       /*  Set return code  */
/*----------------------------------------------------------------------*/
/*      #(B) Changed by S,Higuchi on Aug. 23 1988                       */
/*      OLD     errid = "KMCROTW";                                      */
/*      NEW     #ifdef NDEBUGPRINT                                      */
/*                  errid = "KMCROTW";                                  */
/*              #endif                                                  */
/*----------------------------------------------------------------------*/
#ifdef NDEBUGPRINT
		   errid = "KMCROTW";  /*  Set return code  */
#endif
                   break;
              };
                       /*  Check cursor move right limit and left limit  */
              if(kjsvpt->curleft != C_FAUL
                && kjsvpt->curright != C_FAUL)  {

                       /*  Check next cursor position   */
                   if(nxtcol < kjsvpt->curleft
                     || nxtcol > kjsvpt->curright)  {
                        rc = KMCROTW;        /*  Set return code  */
/*----------------------------------------------------------------------*/
/*      #(B) Changed by S,Higuchi on Aug. 23 1988                       */
/*      OLD     errid = "KMCROTW";                                      */
/*      NEW     #ifdef NDEBUGPRINT                                      */
/*                  errid = "KMCROTW";                                  */
/*              #endif                                                  */
/*----------------------------------------------------------------------*/
#ifdef NDEBUGPRINT
                        errid = "KMCROTW";   /*  Set return code  */
#endif
                        break;
                   };
              };
         }
                       /*  Check cursout of PROFILE is out of field */
         else  {
              rc = KMCROTE;                  /*  Set return code  */
/*----------------------------------------------------------------------*/
/*      #(B) Changed by S,Higuchi on Aug. 23 1988                       */
/*      OLD     errid = "KMCROTE";                                      */
/*      NEW     #ifdef NDEBUGPRINT                                      */
/*                  errid = "KMCROTE";                                  */
/*              #endif                                                  */
/*----------------------------------------------------------------------*/
#ifdef NDEBUGPRINT
              errid = "KMCROTE";             /*  Set return code  */
#endif
              break;
         };

        /* 2.
         *   set  !  highlight attributes(current curcol position,
         *           next cursor position) to local variable.
         */

         ck_cur = pt->hlatst[pt->curcol]; /*  Set highlight attribute to
                                              local variable             */
         ck_nxt = pt->hlatst[nxtcol];     /*  Set highlight attribute to
                                              local variable             */
        /* 3.
         *   set  !  KMISA variable in accordance with highlight attributes
         *           on current or next cursor position.
         */

         if(ck_cur == K_HLAT0 && ck_nxt == K_HLAT0)
              icase = 1;
         if(ck_cur == K_HLAT2 && ck_nxt == K_HLAT0)
              icase = 2;
         if(ck_cur == K_HLAT0 && ck_nxt == K_HLAT3)
              icase = 3;
         if(ck_cur == K_HLAT2 && ck_nxt == K_HLAT3)
              icase = 4;
         if(ck_cur == K_HLAT3 && ck_nxt == K_HLAT0)
              icase = 5;
         if(ck_cur == K_HLAT0 && ck_nxt == K_HLAT2)
              icase = 6;
         if(ck_cur == K_HLAT2 && ck_nxt == K_HLAT2)
              icase = 7;
         if(ck_cur == K_HLAT3 && ck_nxt == K_HLAT2)
              icase = 8;
         if(ck_cur == K_HLAT3 && ck_nxt == K_HLAT3)
              icase = 9;

         switch(icase)  {
                case 1 :    /* ck_cur == K_HLAT0 && ck_nxt == K_HLAT0 */
                case 2 :    /* ck_cur == K_HLAT2 && ck_nxt == K_HLAT0 */

                               /*  current cursor position > 0  */
                        if ( pt->curcol > 0 ) {

                              /*
                               *  Get highlight attribute before current
                               *  cursor position
                               */
                         ck_pre = pt->hlatst[pt->curcol - 1];

                               /*  current cursor position <= 0  */
                        } else {

                              /*  Set normal highlight attribute
                                  to local variable          */
                         ck_pre = K_HLAT0;
                        };

                            /*  Set cursor position in KCB to next cursor
                                position                                  */
                        pt->curcol = nxtcol;

                            /*  Case highlight attribute before current
                                cursor position is reverse area and under
                                line                                      */
                          if(ck_pre == K_HLAT3)  {

                                /*  Call K_HLAT2 Routine. */
                          _Maust(pt);

                              /*   Set changed position and changed length
                                   for display                            */
                          rc = _Msetch(pt,kjsvpt->cconvpos,kjsvpt->cconvlen);

                                  /*  Check Invalid return code  */
                            if(rc != IMSUCC)  {

/*----------------------------------------------------------------------*/
/*      #(B) Changed by S,Higuchi on Aug. 23 1988                       */
/*      OLD     errid = "_Msetch(error)";                               */
/*      NEW     #ifdef NDEBUGPRINT                                      */
/*                  errid = "_Msetch(error)";                           */
/*              #endif                                                  */
/*----------------------------------------------------------------------*/
#ifdef NDEBUGPRINT
                                                /*  Set error return code */
                                  errid = "_Msetch(error)";
#endif
                                  break;

                            };

                              /*  Set current conversion length of KMISA  */
                          kjsvpt->cconvlen = 0;
                        };
                        break;

                case 3 :    /* ck_cur == K_HLAT0 && ck_nxt == K_HLAT3 */

                               /* Get current start position of reversed
                                  highlight attribute                     */
                        ist1  = kjsvpt->cconvpos;

                               /* Get current end position of reversed
                                  highlight attribute                     */
                        ied1  = kjsvpt->cconvpos + kjsvpt->cconvlen - 1 ;

                               /* Get current length of reversed highlight
                                  attribute                               */
                        ilen1 = kjsvpt->cconvlen;

                               /* Get start position and end position of
                                  the phrase on next cursor position      */
                        rc = _Mckbk(pt,nxtcol,&ist,&ied,&iancf);

                               /*  Check Invalid return code   */
                        if(rc != IMSUCC)  {
/*----------------------------------------------------------------------*/
/*      #(B) Changed by S,Higuchi on Aug. 23 1988                       */
/*      OLD     errid = "_Mckbk(error)";                                */
/*      NEW     #ifdef NDEBUGPRINT                                      */
/*                  errid = "_Mckbk(error)";                            */
/*              #endif                                                  */
/*----------------------------------------------------------------------*/
#ifdef NDEBUGPRINT
                              errid = "_Mckbk(error)"; /* Set return code */
#endif
                              break;
                        };

                                /*  Call K_HLAT2 Routine. */
                        _Maust(pt);

                          /*  Set local variable for start position of the
                              phrase                                      */
                        ist2  = ist;

                          /*  Set local variable for length of the phrase */
                        ilen2 = ied - ist + 1;

                          /*  Set local variable for end position of the
                              phrase                                      */
                        ied2 =  ied;

                           /*  Set cursor position in KCB to next cursor
                               position                                  */
                        pt->curcol = nxtcol;

                           /*  Set current conversion position */
                        kjsvpt->cconvpos = ist2;

                           /*  Set current conversion length */
                        kjsvpt->cconvlen = ilen2;

                         /*  Case current start position of reversed
                             highlight attribute != phrase start position */
                        if ( ist1 != ist2 ) {

                                /* Set highlight attribute changing start
                                   position                               */
                            kst = MIN ( ist1 , ist2 );

                                /* Set highlight attribute changing end
                                   position                               */
                            ked = MAX ( ied1 , ied2 );

                                /*  Call K_HLAT3 Routine. */
                           _Maset(pt);

                              /*  Case current start position of reversed
                                  highlight attribute == phrase start
                                  position                                */
                        } else {

                              /*  Case current start position of reversed
                                  highlight attribute > phrase start
                                  position                                */
                            if (ied1 > ied2){

                                /* Set highlight attribute changing start
                                   position                               */
                                   kst = ied2 + 1;

                                /* Set highlight attribute changing end
                                   position                               */
                                   ked = ied1;

                              /*  Case current start position of reversed
                                  highlight attribute <= phrase start
                                  position                                */
                             } else {

                                /* Set highlight attribute changing start
                                   position                               */
                                   kst = ied1 + 1;

                                /* Set highlight attribute changing end
                                   position                               */
                                   ked = ied2;

                             };

                                /*  Call K_HLAT3 Routine. */
                            _Maset(pt);
                        };

                            /* Set highlight attribute changing length  */
                        klen = ked - kst + 1;

                              /*   Set changed position and changed length
                                   for display                            */
                        rc = _Msetch(pt, kst, klen);

                            /*  Check invalid return code  */
                        if(rc != IMSUCC)  {
/*----------------------------------------------------------------------*/
/*      #(B) Changed by S,Higuchi on Aug. 23 1988                       */
/*      OLD     errid = "_Msetch(error)";                               */
/*      NEW     #ifdef NDEBUGPRINT                                      */
/*                  errid = "_Msetch(error)";                           */
/*              #endif                                                  */
/*----------------------------------------------------------------------*/
#ifdef NDEBUGPRINT
                                             /* Set return code */
                                  errid = "_Msetch(error)";
#endif
                                  break;
                        };

                        break;

                case 4 :    /* ck_cur == K_HLAT2 && ck_nxt == K_HLAT3 */

                              /*  Set cursor position in KCB to next cursor
                                  position                                 */
                        pt->curcol = nxtcol;

                        break;

                case 5 :    /* ck_cur == K_HLAT3 && ck_nxt == K_HLAT0 */

                              /*  Set cursor position in KCB to next cursor
                                  position                                 */
                        pt->curcol = nxtcol;

                                /*  Call K_HLAT2 Routine. */
                        _Maust(pt);

                              /*   Set changed position and changed length
                                   for display                            */
                         rc = _Msetch(pt,kjsvpt->cconvpos,kjsvpt->cconvlen);

                                  /*  Check Invalid return code  */
                           if(rc != IMSUCC)  {

/*----------------------------------------------------------------------*/
/*      #(B) Changed by S,Higuchi on Aug. 23 1988                       */
/*      OLD     errid = "_Msetch(error)";                               */
/*      NEW     #ifdef NDEBUGPRINT                                      */
/*                  errid = "_Msetch(error)";                           */
/*              #endif                                                  */
/*----------------------------------------------------------------------*/
#ifdef NDEBUGPRINT
                                                /*  Set error return code */
                              errid = "_Msetch(error)";
#endif

                           };

                               /*  Reset current conversion length  */
                         kjsvpt->cconvlen = 0;

                        break;

                case 6 :    /* ck_cur == K_HLAT0 && ck_nxt == K_HLAT2 */

                                /*  Call K_HLAT2 Routine. */
                           _Maust(pt);

                               /* Get start position and end position of
                                  the phrase on next cursor position      */
                           rc = _Mckbk(pt,nxtcol,&ist,&ied,&iancf);

                               /*  Check Invalid return code   */
                              if(rc != IMSUCC)  {

/*----------------------------------------------------------------------*/
/*      #(B) Changed by S,Higuchi on Aug. 23 1988                       */
/*      OLD     errid = "_Mckbk(error)";                                */
/*      NEW     #ifdef NDEBUGPRINT                                      */
/*                  errid = "_Mckbk(error)";                            */
/*              #endif                                                  */
/*----------------------------------------------------------------------*/
#ifdef NDEBUGPRINT
                                         /*  Set error return code   */
                                    errid = "_Mckbk(error)";
#endif
                                    break;

                              };
                        /* Check output parameter of _Mckbk_() */
                        /* Normal Attribute flag off           */
                   if( iancf == C_SWOFF)  {

                                /*  Set cursor position in KCB to next
                                    cursor position                     */
                            pt->curcol = nxtcol;

                                /*  Set current conversion position   */
                            kjsvpt->cconvpos = ist;

                                /*  Set current conversion length     */
                            kjsvpt->cconvlen = ied - ist + 1;

                                /*  Call K_HLAT3 Routine. */
                         _Maset(pt);

                              /*   Set changed position and changed length
                                   for display                            */
                         rc = _Msetch(pt,kjsvpt->cconvpos,kjsvpt->cconvlen);

                                  /*  Check Invalid return code  */
                             if(rc != IMSUCC)  {

/*----------------------------------------------------------------------*/
/*      #(B) Changed by S,Higuchi on Aug. 23 1988                       */
/*      OLD     errid = "_Mckbk(error)";                                */
/*      NEW     #ifdef NDEBUGPRINT                                      */
/*                  errid = "_Mckbk(error)";                            */
/*              #endif                                                  */
/*----------------------------------------------------------------------*/
#ifdef NDEBUGPRINT
                                                /*  Set error return code */
                                   errid = "_Mckbk(error)";
#endif
                                   break;

                             };

                        /* Check output parameter of _Mckbk_() */
                        /* Normal Attribute flag on            */
                        } else  {

                                /*  Set cursor position in KCB to next
                                    cursor position                     */
                            pt->curcol = nxtcol;

                                /*  Set current conversion position   */
                            kjsvpt->cconvpos = nxtcol;

                                /*  Reset current conversion length     */
                            kjsvpt->cconvlen = 0;

                        };
                        break;

               case 7 :     /* ck_cur == K_HLAT2 && ck_nxt == K_HLAT2 */

                                /*  Check current curcol position */
                        if ( pt->curcol > 1 ) {

                              /*  Get highlight attribute before current
                                  cursor position                        */
                         ck_pre = pt->hlatst[pt->curcol - 2];

                              /*  Set local variable for position before
                                  current cursor position                */
                         prepos = pt->curcol - C_DBCS;

                                /*  Check current cursor position  */
                        } else {

                              /*  Set local variable for highlight
                                  attribute before current cursor
                                  position                          */
                         ck_pre = K_HLAT0;

                              /*  Set local variable for position before
                                  current cursor position                */
                         prepos = 0;

                        };

                       /*  Check position before current cursor >=
                           conversion position                            */
                if ( prepos >= kjsvpt->convpos ) {

                               /* Get start position and end position of
                                  the phrase on next cursor position      */
                          rc = _Mckbk( pt , prepos , &ist , &ied , &iancf );

                               /*  Check Invalid return code   */
                          if(rc != IMSUCC)  {

/*----------------------------------------------------------------------*/
/*      #(B) Changed by S,Higuchi on Aug. 23 1988                       */
/*      OLD     errid = "_Mckbk(error)";                                */
/*      NEW     #ifdef NDEBUGPRINT                                      */
/*                  errid = "_Mckbk(error)";                            */
/*              #endif                                                  */
/*----------------------------------------------------------------------*/
#ifdef NDEBUGPRINT
                                                /*  Set error return code */
                            errid = "_Mckbk(error)";
#endif
                            break;

                          };
                };

                             /*  Check highlight attribute before
                                 current cursor                      */
                         if (ck_pre == K_HLAT3 ) {

                                 /*  Set highlight attribute of the
                                     phrase to "underline"           */
                             for ( h = ist ; h <= ied ; h++ ) {
                                   pt->hlatst[h] = K_HLAT2;
                             };

                              /* Set changed position and changed length
                                 for display                              */
                             rc = _Msetch(pt,ist,ied-ist+1);

                                  /*  Check Invalid return code  */
                             if ( rc != IMSUCC ) {

/*----------------------------------------------------------------------*/
/*      #(B) Changed by S,Higuchi on Aug. 23 1988                       */
/*      OLD     errid = "_Mckbk(error)";                                */
/*      NEW     #ifdef NDEBUGPRINT                                      */
/*                  errid = "_Mckbk(error)";                            */
/*              #endif                                                  */
/*----------------------------------------------------------------------*/
#ifdef NDEBUGPRINT
                                                /*  Set error return code */
                                errid = "_Mckbk(error)";
#endif
                                break;

                             };

                        };

                               /* Get start position and end position of
                                  the phrase on next cursor position      */
                        rc = _Mckbk(pt,nxtcol,&ist,&ied,&iancf);

                                  /*  Check Invalid return code  */
                        if(rc != IMSUCC)  {

/*----------------------------------------------------------------------*/
/*      #(B) Changed by S,Higuchi on Aug. 23 1988                       */
/*      OLD     errid = "_Mckbk(error)";                                */
/*      NEW     #ifdef NDEBUGPRINT                                      */
/*                  errid = "_Mckbk(error)";                            */
/*              #endif                                                  */
/*----------------------------------------------------------------------*/
#ifdef NDEBUGPRINT
                                                /*  Set error return code */
                              errid = "_Mckbk(error)";
#endif
                              break;

                        };

                              /* Check output parameter of _Mckbk_() */
                              /* Normal Attribute flag off           */
                        if(iancf == C_SWOFF)  {

                                /*  Set cursor position in KCB to next
                                    cursor position                     */
                            pt->curcol = nxtcol;

                                /*  Set current conversion position   */
                            kjsvpt->cconvpos = ist;

                                /*  Set current conversion length     */
                            kjsvpt->cconvlen = ied - ist + C_ANK;

                                /*  Call K_HLAT3 Routine. */
                            _Maset(pt);

                              /*   Set changed position and changed length
                                   for display                            */
                         rc = _Msetch(pt,kjsvpt->cconvpos,kjsvpt->cconvlen);

                                  /*  Check Invalid return code  */
                            if(rc != IMSUCC)  {

/*----------------------------------------------------------------------*/
/*      #(B) Changed by S,Higuchi on Aug. 23 1988                       */
/*      OLD     errid = "_Msetch(error)";                               */
/*      NEW     #ifdef NDEBUGPRINT                                      */
/*                  errid = "_Msetch(error)";                           */
/*              #endif                                                  */
/*----------------------------------------------------------------------*/
#ifdef NDEBUGPRINT
                                                /*  Set error return code */
                                  errid = "_Msetch(error)";
#endif
                                  break;

                            };

                              /* Check output parameter of _Mckbk_() */
                              /* Normal Attribute flag on            */
                        } else  {

                                /*  Set cursor position in KCB to next
                                    cursor position                     */
                            pt->curcol = nxtcol;

                                /*  Set current conversion position   */
                            kjsvpt->cconvpos = nxtcol;

                                /*  Set current conversion length     */
                            kjsvpt->cconvlen = 0;

                        };

                        break;

                case 8 :    /* ck_cur == K_HLAT3 && ck_nxt == K_HLAT2 */

                               /* Get current start position of reversed
                                  highlight attribute                     */
                        ist1  = kjsvpt->cconvpos;

                               /* Get current end position of reversed
                                  highlight attribute                     */
                        ied1  = kjsvpt->cconvpos + kjsvpt->cconvlen - 1 ;

                               /* Get current length of reversed highlight
                                  attribute                               */
                        ilen1 = kjsvpt->cconvlen;

                               /* Get start position and end position of
                                  the phrase on next cursor position      */
                        rc = _Mckbk(pt,nxtcol,&ist,&ied,&iancf);

                               /*  Check Invalid return code   */
                        if(rc != IMSUCC)  {

/*----------------------------------------------------------------------*/
/*      #(B) Changed by S,Higuchi on Aug. 23 1988                       */
/*      OLD     errid = "_Mckbk(error)";                                */
/*      NEW     #ifdef NDEBUGPRINT                                      */
/*                  errid = "_Mckbk(error)";                            */
/*              #endif                                                  */
/*----------------------------------------------------------------------*/
#ifdef NDEBUGPRINT
                                             /* Set return code */
                              errid = "_Mckbk(error)";
#endif
                              break;

                        };

                                /*  Call K_HLAT2 Routine. */
                        _Maust(pt);

                        /* Check output parameter of _Mckbk_() */
                        /* Normal Attribute flag off           */
                     if(iancf == C_SWOFF)  {

                          /*  Set local variable for start position of the
                              phrase                                      */
                        ist2  = ist;

                          /*  Set local variable for length of the phrase */
                        ilen2 = ied - ist + 1;

                          /*  Set local variable for end position of the
                              phrase                                      */
                        ied2 =  ied;

                           /*  Set cursor position in KCB to next cursor
                               position                                  */
                        pt->curcol = nxtcol;

                           /*  Set current conversion position */
                        kjsvpt->cconvpos = ist2;

                           /*  Set current conversion length */
                        kjsvpt->cconvlen = ilen2;

                         /*  Case current start position of reversed
                             highlight attribute != phrase start position */
                        if ( ist1 != ist2 ) {

                                /* Set highlight attribute changing start
                                   position                               */
                            kst = MIN ( ist1 , ist2 );

                                /* Set highlight attribute changing end
                                   position                               */
                            ked = MAX ( ied1 , ied2 );

                                /*  Call K_HLAT3 Routine. */
                            _Maset(pt);

                              /*  Case current start position of reversed
                                  highlight attribute == phrase start
                                  position                                */
                        } else {

                              /*  Case current start position of reversed
                                  highlight attribute > phrase start
                                  position                                */
                            if (ied1 > ied2){

                                /* Set highlight attribute changing start
                                   position                               */
                                   kst = ied2 + 1;

                                /* Set highlight attribute changing end
                                   position                               */
                                   ked = ied1;

                              /*  Case current start position of reversed
                                  highlight attribute <= phrase start
                                  position                                */
                             } else {

                                /* Set highlight attribute changing start
                                   position                               */
                                   kst = ied1 + 1;

                                /* Set highlight attribute changing end
                                   position                               */
                                   ked = ied2;

                             };

                                /*  Call K_HLAT3 Routine. */
                             _Maset(pt);

                        };

                            /* Set highlight attribute changing length  */
                        klen = ked - kst + 1;

                              /*   Set changed position and changed length
                                   for display                            */
                        rc = _Msetch(pt, kst, klen);

                            /*  Check invalid return code  */
                        if(rc != IMSUCC)  {

/*----------------------------------------------------------------------*/
/*      #(B) Changed by S,Higuchi on Aug. 23 1988                       */
/*      OLD     errid = "_Msetch(error)";                               */
/*      NEW     #ifdef NDEBUGPRINT                                      */
/*                  errid = "_Msetch(error)";                           */
/*              #endif                                                  */
/*----------------------------------------------------------------------*/
#ifdef NDEBUGPRINT
                                             /* Set return code */
                                  errid = "_Msetch(error)";
#endif
                                  break;

                        };

                        /* Check output parameter of _Mckbk_() */
                        /* Normal Attribute flag on            */
                     } else  {

                              /*   Set changed position and changed length
                                   for display                            */
                         rc = _Msetch(pt,kjsvpt->cconvpos,kjsvpt->cconvlen);

                              /*  Set cursor position in KCB to next cursor
                                  position                                 */
                            pt->curcol = nxtcol;

                           /*  Set current conversion position */
                            kjsvpt->cconvpos = nxtcol;

                           /*  Set current conversion length */
                            kjsvpt->cconvlen = 0;

                               /*  Check Invalid return code   */
                            if(rc != IMSUCC)  {

/*----------------------------------------------------------------------*/
/*      #(B) Changed by S,Higuchi on Aug. 23 1988                       */
/*      OLD     errid = "_Msetch(error)";                               */
/*      NEW     #ifdef NDEBUGPRINT                                      */
/*                  errid = "_Msetch(error)";                           */
/*              #endif                                                  */
/*----------------------------------------------------------------------*/
#ifdef NDEBUGPRINT
                                         /*  Set error return code   */
                                  errid = "_Msetch(error)";
#endif
                                  break;

                            };

                     };
                     break;

                case 9 :    /* ck_cur == K_HLAT3 && ck_nxt == K_HLAT3 */

                               /* Get current start position of reversed
                                  highlight attribute                     */
                        ist1  = kjsvpt->cconvpos;

                               /* Get current length of reversed highlight
                                  attribute                               */
                        ilen1 = kjsvpt->cconvlen;

                               /* Get current end position of reversed
                                  highlight attribute                     */
                        ied1  = ist1 + ilen1 - 1;

                                /*  next cursor position is after the
                                    current cursor position              */
                        if ( nxtcol - pt->curcol > 0 ) {

                            idir = 1;

                                /*  next cursor position is before the
                                    current cursor position              */
                        } else {

                            idir = 0;

                        };

                               /* Get start position and end position of
                                  the phrase on next cursor position      */
                        rc = _Mckbk(pt,nxtcol,&ist,&ied,&iancf);

                               /*  Check Invalid return code   */
                        if(rc != IMSUCC)  {

/*----------------------------------------------------------------------*/
/*      #(B) Changed by S,Higuchi on Aug. 23 1988                       */
/*      OLD     errid = "_Mckbk(error)";                                */
/*      NEW     #ifdef NDEBUGPRINT                                      */
/*                  errid = "_Mckbk(error)";                            */
/*              #endif                                                  */
/*----------------------------------------------------------------------*/
#ifdef NDEBUGPRINT
                                                /*  Set error return code */
                                  errid = "_Mckbk(error)";
#endif
                                  break;

                        };

                          /*  Set local variable for start position of the
                              phrase                                      */
                        ist2  = ist;

                          /*  Set local variable for length of the phrase */
                        ilen2 = ied - ist + 1;

                          /*  Set local variable for end position of the
                              phrase                                      */
                        ied2  = ied;
                /*
                 *  Case current conversion start position
                 *  != phrase start position
                 *  ||
                 *  Case current conversion lenght position
                 *  != phrase lenght position
                 */

         if(ist1 != ist2 || ilen1 != ilen2)  {

                        /* Check output parameter of _Mckbk_() */
                        /* Normal Attribute flag off           */
              if ( iancf == C_SWOFF ) {

                            /*  Set cursor position in KCB to next cursor
                                position                                  */
                         pt->curcol = nxtcol;

                         /*  Case current start position of reversed
                             highlight attribute != phrase start position */
                         if ( ist1 != ist2 ) {

                                /*  Call K_HLAT2 Routine. */
                                _Maust( pt );

                           /*  Set current conversion position */
                                kjsvpt->cconvpos = ist2;

                           /*  Set current conversion length */
                                kjsvpt->cconvlen = ilen2;

                                /*  Call K_HLAT3 Routine. */
                                _Maset ( pt );

                                /* Set highlight attribute changing start
                                   position                               */
                                kst = MIN( ist1 , ist2 );

                                /* Set highlight attribute changing end
                                   position                               */
                                ked = MAX( ied1 , ied2 );

                                /* Check output parameter of _Mckbk_() */
                                /* Normal Attribute flag on            */
                         } else {

                           /*  Set current conversion position */
                                kjsvpt->cconvpos = ist2;

                           /*  Set current conversion length */
                                kjsvpt->cconvlen = ilen2;

                              /*  Case current start position of reversed
                                  highlight attribute > phrase start
                                  position                                */
                            if (ied1 > ied2){

                                /* Set highlight attribute changing start
                                   position                               */
                                   kst = ied2 + 1;

                                /* Set highlight attribute changing end
                                   position                               */
                                   ked = ied1;

                              /*  Case current start position of reversed
                                  highlight attribute <= phrase start
                                  position                                */
                            } else {

                                /* Set highlight attribute changing start
                                   position                               */
                                   kst = ied1 + 1;

                                /* Set highlight attribute changing end
                                   position                               */
                                   ked = ied2;

                            };

                                /* Case next cursor position -
                                   current cursor position <= 0  */
                            if ( idir == 0 ) {

                                 /*  Set highlight attribute of the
                                     phrase to "underline"           */
                               for(hh = kst; hh <= ked; hh++) {
                                   pt->hlatst[hh] = K_HLAT2;
                               };

                                /* Case next cursor position -
                                   current cursor position > 0  */
                            } else {

                                 /*  Set highlight attribute of the
                                     phrase to "reverse & underline"   */
                               for(hh = kst; hh <= ked; hh++) {
                                   pt->hlatst[hh] = K_HLAT3;
                               };

                           };

                         };

                            /* Set highlight attribute changing length  */
                         klen = ked - kst + 1;

                              /*   Set changed position and changed length
                                   for display                            */
                         rc = _Msetch(pt, kst , klen);

                            /*  Check invalid return code  */
                         if(rc != IMSUCC)  {

                                  return(rc);

                         };

                        /* Check output parameter of _Mckbk_() */
                        /* Normal Attribute flag on            */
              } else {

                              /*  Set cursor position in KCB to next cursor
                                  position                                 */
                  pt->curcol = nxtcol;
                  break;

              };

        /*
         *  Case current conversion start position
         *  = phrase start position
         *  &&
         *  Case current conversion lenght position
         *  = phrase lenght position
         */

         } else {

                              /*  Set cursor position in KCB to next cursor
                                  position                                 */
                pt->curcol = nxtcol;

                              /*  Set while loop switch */
                extjmp = C_SWON;

         };

      };        /*  end switch(icase)  */

      extjmp = C_SWON;

   };           /*  while loop         */

        /* 4.
         *  Riturn Value
         */

/*----------------------------------------------------------------------*/
/*      #(B) Changed by S,Higuchi on Aug. 23 1988                       */
/*      OLD     snap3( SNAP_KCB | SNAP_KMISA , SNAP_Mcrmv , errid );    */
/*      NEW     #ifdef NDEBUGPRINT                                      */
/*                  snap3( SNAP_KCB | SNAP_KMISA , SNAP_Mcrmv , errid );*/
/*              #endif                                                  */
/*----------------------------------------------------------------------*/
#ifdef NDEBUGPRINT
       snap3( SNAP_KCB | SNAP_KMISA , SNAP_Mcrmv , errid ) ;
#endif
       return(rc);

}

  /*  Set K_HLAT2 Routine. */
static void _Maust(pt)
KCB *pt;
{
        KMISA *kjsvpt;     /*   pointer to kanji moniter ISA            */
        int   i;           /*   for loop counter                        */
        kjsvpt = pt->kjsvpt;   /*  set pointer to kanji moniter ISA  */

              /*  Set highlight attribute on current conversion length
                  to "underlined"                                       */
        for ( i= kjsvpt->cconvpos ;
              i < kjsvpt->cconvpos + kjsvpt->cconvlen ;
              i++) {
                 pt->hlatst[i] = K_HLAT2;
        };
}

  /*  Set K_HLAT3 Routine. */
static void _Maset(pt)
KCB *pt;
{
        KMISA *kjsvpt;  /*   pointer to kanji moniter ISA            */
        int   i;        /*   for loop counter                        */
        kjsvpt = pt->kjsvpt;     /*  set pointer to kanji moniter ISA  */

              /*  Set highlight attribute on current conversion length
                  to "reversed & underlined"                           */
        for ( i= kjsvpt->cconvpos ;
              i < kjsvpt->cconvpos + kjsvpt->cconvlen ;
              i++) {
                 pt->hlatst[i] = K_HLAT3;
        };
}

