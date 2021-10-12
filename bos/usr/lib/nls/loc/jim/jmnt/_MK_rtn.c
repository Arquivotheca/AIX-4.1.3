static char sccsid[] = "@(#)70	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_MK_rtn.c, libKJI, bos411, 9428A410j 7/23/92 03:19:48";
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
 * MODULE NAME:         _MK_rtn()
 *
 * DESCRIPTIVE NAME:    KKC Interface Routine
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
 * FUNCTION:            Call Kanji Moniter subroutines or KKC routines in
 *                      accordance with the KMAT action code.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        2600 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _MK_rtn()
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _MK_rtn (pt,actc1)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *                      actc1   :KMAT action code (K-code).
 *
 *  OUTPUT:             NONE
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      IMSUCC :Success of Execution.
 *
 * EXIT-ERROR:          Return Codes Returned to Caller.
 *                      IMFAIL :Failure of Execution.(Invalid input data)
 *                      K_KCSYPE : Physical error on sysdict
 *                      K_KCUSPE : Physical error on usrdict
 *                      K_KCFZPE : Physical error on fzkdict
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _Mymstl()       :Reset highlight attribute,etc.
 *                                               as post fixing process.
 *                              _Mstlcvl()      :Reset parameters in KMISA
 *                                               related to KKC.
                                                 Call KKC-routine for learning
 *                                               if necessary.
 *                              _MMSG_rs()      :Return to previous condition
 *                                               of Kanji Monitor.
 *                              _MK_a2()        :Get Kanji corresponds to
 *                                               specified Kanji No.
 *                              _MK_b4a()       :Make Hira/Kata Map and convert
 *                                               Hiragana to Katakana or
 *                                               Katakana to Hiragana in the
                                                 string.
 *                              _MK_b4b()       :Convert Katakana to Hiragana
 *                                               or Hiragana to Katakana in the
                                                 string.
 *                              _MK_c1()        :Set input parameters and call
 *                                               KKC-routine to get initially
 *                                               converted Kanji corresponds to
                                                 specified Yomi.
 *                              _Mkcnxpr()      :Call Next/Pre-conversion sub-
 *                                               routines correspond to the
 *                                               specified phrase in accordance
 *                                               with its conversion status.
 *                              _MK_c3()        :Call KKC-routine and  get next
 *                                               candidates in all candidate
 *                                               display mode.
 *                              _MK_e3()        :Call KKC-routine and  get
 *                                               previous candidates in all
 *                                               candidate display mode.
 *                              _MK_e4()        :Call KKC-routine to get first
 *                                               group of candidates and display
 *                                               them.
 *                              _MK_c1f()       :First Flying Conversion.
 *                              _Mflyrst()      :Reset Flying Conversion Area.
 *
 *                      Standard Liblary.
 *                              NA.
 *
 *                      Advanced Display Graphics Support Liblary(GSL).
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
 *                              Reference
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              Reference
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Controle Block(KCB).
 *                              NA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              Modification
 *                      Trace Block(TRB).
 *                              NA.
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Liblary.
 *                              IDENTIFY:Module Identify Create.
 *                      Standard Macro Liblary.
 *                              NA.
 *
 * CHANGE ACTIVITY:     Tuesday Aug. 23 1988 Satoshi Higuchi
 *                      Added kjsvpt->fconvflg = C_SWON for stopping
 *                      duplication of string.
 *                      See problem collection sheet P-11.
 *
 *                      Sept. 20 1988 Satoshi Higuchi
 *                      Changed a few lines for overflow of the kjdataf
 *                      etc. support.
 *
 ********************* END OF MODULE SPECIFICATIONS ***********************/

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
 *      Please Descripte This Module.
 */

int _MK_rtn (pt,actc1)

KCB     *pt;            /* Pointer to Kanji Control Block.              */
char  actc1;            /* KMAT action code (K-code)                    */
{
        register KMISA  *kjsvpt  ;      /* Pointer to Internal Save Area*/
                 char   hhactc   ;      /* Higher half byte of actc1    */
                 short  ist      ;      /* Phrase start position        */
                 short  ied      ;      /* Phrase end   position        */
                 short  iancf    ;      /* Attribute no change flag     */
                 int    ret_code ;      /* Return code                  */

CPRINT (--- _MK_rtn ---) ;
snap3 (SNAP_KCB | SNAP_KMISA, SNAP_MK_rtn, "== Start _MK_rtn ==") ;
        /** 1
         ** Set internal parameters
         **/
        kjsvpt = pt->kjsvpt ;
        ret_code = IMSUCC ;

        /** 2
         ** Select subroutine
         **/

        /*
         Yomi fixing
         */
        if ((hhactc = (actc1 & C_HHBYTE)) == A_REDDEC)
        {   /* call yomi-fixing routine */
            ret_code = _Mymstl (pt,kjsvpt->convpos+kjsvpt->convlen) ;
            kjsvpt->cconvpos = 0 ;   /* reset current conversion position  */
            kjsvpt->cconvlen = 0 ;   /* and current conversion length      */


/* #(B) 1987.12.04. Flying Conversion Add */

            /*
             *      Flying Conversion Mode.
             */
            if ( kjsvpt->kmpf[0].convtype == K_CIKUJI ) {

                _Mflyrst( pt ); /* Reset Flying Conversion Area.            */

                kjsvpt->fconvflg = C_SWOFF;/* Reset Flaying Conversion Flag.*/
            };

/* #(E) 1987.12.04. Flying Conversion Add */


snap3 (SNAP_KCB | SNAP_KMISA, SNAP_MK_rtn, "== Return _MK_rtn ==") ;
            return(ret_code) ;
        }

        /*
         Conversion fixing
         */
        if (hhactc == A_CNVDEC)
        {   /* call conversion-fixing routine */
            ret_code = _Mstlcvl (pt,kjsvpt->convpos+kjsvpt->convlen) ;
            kjsvpt->cconvpos = 0 ;   /* reset current conversion position  */
            kjsvpt->cconvlen = 0 ;   /* and current conversion length      */


/* #(B) 1987.12.04. Flying Conversion Add */

            /*
             *      Flying Conversion Mode.
             */
            if ( kjsvpt->kmpf[0].convtype == K_CIKUJI ) {

                _Mflyrst( pt ); /* Reset Flying Conversion Area.            */

                kjsvpt->fconvflg = C_SWOFF;/* Reset Flaying Conversion Flag.*/
            };

/* #(E) 1987.12.04. Flying Conversion Add */


snap3 (SNAP_KCB | SNAP_KMISA, SNAP_MK_rtn, "== Return _MK_rtn ==") ;
            return(ret_code) ;
        }

        switch (actc1)
        {
                /*
                 No operation
                 */
                case  A_NOP    :
snap3 (SNAP_KCB | SNAP_KMISA, SNAP_MK_rtn, "== Return _MK_rtn (NOP) ==") ;
                                 return (ret_code) ;
                /*
                 Return to previous condition
                 */
                 case  A_ENTALC :/* Call message and mode reset routine */
                                 ret_code = _MMSG_rs (pt,K_MSGOTH) ;
                                 break ;

                /*
                 Input Kanji No.
                 */
                case  A_ENTKJN : /* Call Input-Kanji-No. routine */
                                 ret_code = _MK_a2 (pt);
                                 break ;

                /*
                 No-conversion process (mode 1)
                 */
                case  A_NOCNV1 : /* Restore initial Yomi for whole conversion
                                    length */
                                 ret_code = _Mrstym (pt,
                                            kjsvpt->convpos+kjsvpt->convlen);


/* #(B) 1987.12.17. Flying Conversion Add */

            /*
             *      Flying Conversion Mode.
             */
            if ( kjsvpt->kmpf[0].convtype == K_CIKUJI ) {

                /* Reset Flying Conversion Area.                        */
                _Mflyrst( pt );

                /* Set Flying Conversion Position.                      */
                kjsvpt->fconvpos = kjsvpt->cconvpos;

                /* Reset Flaying Conversion Flag.                       */

                kjsvpt->fconvflg = C_SWOFF;
            };

/* #(E) 1987.12.17. Flying Conversion Add */

                                 break ;

                /*
                 No-conversion process (mode 2)
                 */
                case  A_NOCNV2 : /* Fix the left of curr. conversion position */
                                 ret_code = _Mstlcvl (pt,kjsvpt->cconvpos) ;

                                 /* Restore initial Yomi for the right of new
                                    conversion position */
                                 ret_code = _Mrstym (pt,
                                            kjsvpt->convpos+kjsvpt->convlen);


/* #(B) 1987.12.17. Flying Conversion Add */

            /*
             *      Flying Conversion Mode.
             */
            if ( kjsvpt->kmpf[0].convtype == K_CIKUJI ) {

                /* Reset Flying Conversion Area.                        */
                _Mflyrst( pt );

                /* Set Flying Conversion Position.                      */
                kjsvpt->fconvpos = kjsvpt->cconvpos;

                /* Reset Flaying Conversion Flag.                       */

                kjsvpt->fconvflg = C_SWOFF;
            };

/* #(E) 1987.12.17. Flying Conversion Add */


                                 break ;

                /*
                 No-conversion process (mode 3)
                 */
                case  A_NOCNV3 : /* Fix the left of cursor position, including
                                    the character of cursor position          */

                                 ret_code = _Mstlcvl (pt,pt->curcol+C_DBCS) ;

                                 /* Reset curr. conversion position and length*/
                                 if (kjsvpt->convlen > 0)
                                 {    kjsvpt->cconvpos = pt->curcol+C_DBCS ;
                                      kjsvpt->cconvlen = C_DBCS ;
                                 }
                                 else   /* in case all strings are fixed   */
                                 {    kjsvpt->cconvpos = 0 ;
                                      kjsvpt->cconvlen = 0 ;
                                 }


/* #(B) 1987.12.17. Flying Conversion Add */

            /*
             *      Flying Conversion Mode.
             */
            if ( kjsvpt->kmpf[0].convtype == K_CIKUJI ) {

                /* Reset Flying Conversion Area.                        */
                _Mflyrst( pt );

                /* Set Flying Conversion Position.                      */
                kjsvpt->fconvpos = kjsvpt->cconvpos;
            };

/* #(E) 1987.12.17. Flying Conversion Add */

                                 break ;

                /*
                 Hiragana/Katakana conversion
                 */
                case  A_NOCNV4 : switch (kjsvpt->hkmode)
                                 {   /* Conversion from Yomi input mode */
                                     case K_HKRES    :ret_code =_MK_b4a (pt) ;
                                                      break ;

                                     /* Conversion from first Hira/Kata mode */
                                     case K_HKKAN    :ret_code =_MK_b4b (pt) ;
                                 }
                                 break ;

                /*
                 No-conversion process (mode 5)
                 */
                case  A_NOCNV5 : /* Fix the left of cursor position */
                                 ret_code =_Mstlcvl (pt,pt->curcol) ;

                                 /* Restore initial Yomi for the right of new
                                    conversion position */
                                 if (kjsvpt->convlen > 0 )
                                      ret_code =_Mrstym (pt,kjsvpt->convpos
                                                         + kjsvpt->convlen);


/* #(B) 1987.12.17. Flying Conversion Add */

            /*
             *      Flying Conversion Mode.
             */
            if ( kjsvpt->kmpf[0].convtype == K_CIKUJI ) {

                /* Reset Flying Conversion Area.                        */
                _Mflyrst( pt );

                /* Set Flying Conversion Position.                      */
                kjsvpt->fconvpos = kjsvpt->cconvpos;

                /* Reset Flaying Conversion Flag.                       */
                kjsvpt->fconvflg = C_SWOFF;
            };

/* #(E) 1987.12.17. Flying Conversion Add */


                                 break ;

                /*
                 Initial Kanji conversion
                 */
                case  A_CONV1  : /* Call initial conversion routine */


/* #(B) 1987.12.07. Flying Conversion Change */

                            /*
                             *      Flying Conversion Mode.
                             */
                            if ( kjsvpt->kmpf[0].convtype == K_CIKUJI ) {

                               if (  (kjsvpt->fconvpos <= pt->curcol) &&
                                     (kjsvpt->fconvflg == C_SWOFF   )  ) {

                                  switch ( kjsvpt->chmode ) {

                                  case K_CHHIRA:
                                  case K_CHKATA:
/*======================================================================*/
/* #(B) Sept. 20 1988 Added by Satoshi Higuchi                          */
/*      Changed source.                                                 */
/*      Old source.                                                     */
/*          ret_code = _MK_c1f( pt );                                   */
/*          break;                                                      */
/*                                                                      */
/*      New source.                                                     */
/*          if(kjsvpt->fcvovfg == C_SWOFF)                              */
/*              ret_code = _MK_c1f(pt);                                 */
/*          if((kjsvpt->fcvovfg == C_SWON) ||                           */
/*            (ret_code == IMFCVOVF))                                   */
/*              ret_code = _MK_c1(pt);                                  */
/*          break;                                                      */
/*======================================================================*/
				     if(kjsvpt->fcvovfg == C_SWOFF)
					 ret_code = _MK_c1f(pt);
				     if((kjsvpt->fcvovfg == C_SWON) ||
				       (ret_code == IMFCVOVF))
					 ret_code = _MK_c1(pt);
				     break;
                                  default:

                                     ret_code = _MK_c1( pt ) ;
                                     break;
                                  };
                               } else {

                                  ret_code = _MK_c1( pt ) ;
                               };

                               (void)_Mflyrst( pt );
                               kjsvpt->fconvpos = kjsvpt->cconvpos;

/*      #(B) Added by S,Higuchi on Aug. 23 1988                         */
			       kjsvpt->fconvflg = C_SWON;

                            } else {

                               ret_code = _MK_c1( pt ) ;
                            };

/* #(E) 1987.12.07. Flying Conversion Change */



                                 if (KKCPHYER(kjsvpt->kkcrc))
                                 {
snap3 (SNAP_KCB | SNAP_KMISA, SNAP_MK_rtn, "== Return _MK_rtn (KKC ERR)==") ;
                                         return (kjsvpt->kkcrc) ;
                                 }
                                 break ;

                /*
                 Next candidate
                 */
                case  A_NXTCNV : /* Call Next/Pre candidate routine */
                                 ret_code = _Mkcnxpr(pt,M_KKCNXT) ;
                                 if (KKCPHYER(kjsvpt->kkcrc))
                                 {
snap3 (SNAP_KCB | SNAP_KMISA, SNAP_MK_rtn, "== Return _MK_rtn (KKC ERR) ==") ;
                                        return (kjsvpt->kkcrc) ;
                                 }
                                 break ;

                /*
                 Next candidate in all candidate mode
                 */
                case  A_NCVACN : /* Call next all candidate routine */
                                 ret_code = _MK_c3 (pt) ;
                                 if (KKCPHYER(kjsvpt->kkcrc))
                                 {
snap3 (SNAP_KCB | SNAP_KMISA, SNAP_MK_rtn, "== Return _MK_rtn (KKC ERR) ==") ;
                                        return (kjsvpt->kkcrc) ;
                                 }
                                 break ;

                /*
                 Yomi of a phrase (1)
                 */
                case  A_YOMICV : /* Fix the converted string left of the
                                    curr. conversion position */
                                 ret_code = _Mstlcvl (pt,kjsvpt->cconvpos) ;

                                 /* Restore Yomi of the curr. conversion
                                    phrase */
                                 ret_code =  _Mrstym (pt,
                                             kjsvpt->convpos+kjsvpt->cconvlen);


/* #(B) 1988.01.11. Flying Conversion Add */

            /*
             *      Flying Conversion Mode.
             */
            if ( kjsvpt->kmpf[0].convtype == K_CIKUJI ) {

                /* Reset Flying Conversion Area.                        */
                _Mflyrst( pt );

                /* Set Flying Conversion Position.                      */
                kjsvpt->fconvpos = kjsvpt->cconvpos;
            };

/* #(E) 1987.12.17. Flying Conversion Add */

                                 break;

                /*
                 Yomi of a phrase (2)
                 */
                case  A_YOMCV2 : /* Check the length of phrase at cursor
                                    position */
                                 _Mckbk (pt,pt->curcol,&ist,&ied,&iancf);

                                 /* Fix the left of cursor position */
                                 ret_code = _Mstlcvl (pt,pt->curcol) ;

                                 /* Restore Yomi of the phrase at cursor
                                    position */
                                 if (iancf != C_SWON)
                                        ret_code = _Mrstym (pt,ied + C_ANK) ;


/* #(B) 1988.01.11. Flying Conversion Add */

            /*
             *      Flying Conversion Mode.
             */
            if ( kjsvpt->kmpf[0].convtype == K_CIKUJI ) {

                /* Reset Flying Conversion Area.                        */
                _Mflyrst( pt );

                /* Set Flying Conversion Position.                      */
                kjsvpt->fconvpos = kjsvpt->cconvpos;
            };

/* #(E) 1987.12.17. Flying Conversion Add */


                                 break ;

                /*
                 Previous candidate
                 */
                case  A_PRVCAN : /* Call Next/Pre candidate routine */
                                 ret_code = _Mkcnxpr (pt,M_KKCPRE) ;
                                 if (KKCPHYER(kjsvpt->kkcrc))
                                 {
snap3 (SNAP_KCB | SNAP_KMISA, SNAP_MK_rtn, "== Return _MK_rtn (KKC ERR) ==") ;
                                        return (kjsvpt->kkcrc) ;
                                 }
                                 break ;

                /*
                 Previous candidate in all candidate mode
                 */
                case  A_PRVALL : /* Call previous all candidate routine */
                                 ret_code = _MK_e3 (pt) ;
                                 if (KKCPHYER(kjsvpt->kkcrc))
                                 {
snap3 (SNAP_KCB | SNAP_KMISA, SNAP_MK_rtn, "== Return _MK_rtn (KKC ERR) ==") ;
                                        return (kjsvpt->kkcrc) ;
                                 }
                                 break ;

                /*
                 First group of all candidate
                 */
                case  A_ALLCAN : /* Call first goup of  all candidate routine */
                                 ret_code = _MK_e4 (pt) ;
                                 if (KKCPHYER(kjsvpt->kkcrc))
                                 {
snap3 (SNAP_KCB | SNAP_KMISA, SNAP_MK_rtn, "== Return _MK_rtn (KKC ERR) ==") ;
                                        return (kjsvpt->kkcrc) ;
                                 }
                                 break ;

                default        : /* Invalid actc1 Error  */
snap3 (SNAP_KCB | SNAP_KMISA, SNAP_MK_rtn, "*****(actc1 ERR)*****");
                                  ret_code = IMFAIL ;
        }

        /** E
         ** Return Code
         **/
snap3 (SNAP_KCB | SNAP_KMISA, SNAP_MK_rtn, "== Return _MK_rtn ==") ;
        return(ret_code) ;
}
