static char sccsid[] = "@(#)03	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mflypro.c, libKJI, bos411, 9428A410j 7/23/92 03:22:09";
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
 * MODULE NAME:         _Mflypro
 *
 * DESCRIPTIVE NAME:    Input DBCS string fly conversion.
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
 * FUNCTION:            Input DBCS string fly conversion.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1500 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mflypro
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mflypro( pt )
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC  :Success of Execution.
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              NA.
 *                      Standard Library.
 *                              memcpy  :Copy characters from memory area
 *                                       A to B.
 *                      Advanced Display Graphics Support Library(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      See Below.
 *
 *   INPUT:             Kanji Monitor Control Block(KCB).
 *                              kjsvpt
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              NA.
 *                      Kana Kanji Control Block(KKCB).
 *                              NA.
 *
 *   OUTPUT:            Kanji Monitor Control Block(KCB).
 *                              NA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              NA.
 *                      Kana Kanji Control Block(KKCB).
 *                              NA.
 *
 * TABLES:              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              IDENTIFY:Module Identify Create.
 *                              SHTOCHPT:Set short data to character
 *                                       pointerd area.
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     Wednesday Aug. 24 1988 Satoshi Higuchi
 *                      Added some local variables and changed programes
 *                      at case statement about input dakuten, handakuten
 *                      after Katakana during Look-ahead KKC.
 *                      See Monitor Improvement Spec. 3.1.1.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */

/*
 *      include Standard.
 */
#include <stdio.h>      /* Standard I/O Package.                        */
#include <memory.h>     /* Perform Memory Operations.                   */

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

int     _Mflypro( pt )

register KCB    *pt;            /* Pointer to Kanji Control Block.      */
{
        register KMISA  *kjsvpt;/* Pointer to Kanji Monitor
                                   Internal Save Area.                  */
        register KKCB   *kkcbsvpt;
                                /* Pointer to Kana Kanji Control Block. */

        char    *memcpy();      /* Copy characters from memory area
                                   A to B.                              */

        int     rc;             /* Return code.                         */

        uchar   cnvmodfg;       /* Conversion Mode Flag.                */
        uchar   modchgfg;       /* Character Mode Change Flag.          */
        uchar   nocnvflg;       /* No Conversion Flag.                  */
        uchar   cmode;          /* First Position Character Mode.       */
        uchar   fmode;          /* Current Character Mode.              */
        short   loop1;          /* Loop Couunter.                       */
        short   loop2;          /* Loop Counter.                        */
        short   loop1st;        /* Loop First Counter.                  */
        short   loopend;        /* Loop End Counter.                    */
        short   fpos;           /* Flying Conversion Position.          */
        short   flen;           /* Flying Conversion Length.            */
        short   oldcol;         /* Old Curcor Position.                 */
/*----------------------------------------------------------------------*/
/*      #(B) Added by S,Higuchi on Aug. 24 1988                         */
/*      short   strpos;                                                 */
/*      short   kjlen;                                                  */
/*      uchar   charmode;                                               */
/*----------------------------------------------------------------------*/
	short   strpos;         /* Changes character position.          */
	short   kjlen;          /* Length of Kjdata by work             */
	uchar   charmode;       /* Character mode before curcol         */

        kjsvpt   = pt->kjsvpt;
        kkcbsvpt = kjsvpt->kkcbsvpt;

        /*
         *      Initialize return code.
         */
        rc = IMSUCC;



        /*
         *      Check Conversion Type.
         */
        if (  (kjsvpt->kmpf[0].convtype != K_CIKUJI) ||
              (kjsvpt->fconvflg         ==   C_SWON)  ) {
                return( IMSUCC );
        };



        /*
         *      Check Input Code.
         */
        if ( pt->type == K_INASCI ) {   /* Ascii Character.             */

            /*
             *      Check Current Mode.
             */
            if ( kjsvpt->kkmode1 == A_CONINP ) {

                /*
                 *      Check First Character Mode.
                 */
                switch( kjsvpt->chmode ) {

                case K_CHHIRA:
                case K_CHKATA:
/*----------------------------------------------------------------------*/
/*      #(B) Changed by S,Higuchi on Aug. 24 1988                       */
/*      OLD     nocnvflg = C_SWOFF;                                     */
/*              break;                                                  */
/*      NEW     if(((pt->code == 0xde) || (pt->code == 0xdf)) &&        */
/*                (kjsvpt->kmpf[0].katakana == K_KANAOF) &&             */
/*                (_Mgetchm(pt->string,pt->curcol-C_DBCS) == K_CHKATA)){*/
/*                  strpos = pt->curcol - C_DBCS;                       */
/*                  kjlen = CHPTTOSH(kjsvpt->kjdataf) - C_DBCS;         */
/*                  kjsvpt->kjdataf[kjlen] = pt->string[strpos];        */
/*                  kjsvpt->kjdataf[kjlen+1] = pt->string[strpos+1];    */
/*                  return(IMSUCC);                                     */
/*              }                                                       */
/*              else                                                    */
/*                  nocnvflg = C_SWOFF;                                 */
/*              break;                                                  */
/*----------------------------------------------------------------------*/
		    if(((pt->code == 0xde) || (pt->code == 0xdf)) &&
		      (kjsvpt->kmpf[0].katakana == K_KANAOF) &&
		      (_Mgetchm(pt->string,pt->curcol-C_DBCS,kjsvpt) ==
							  K_CHKATA)) {
			strpos = pt->curcol - C_DBCS;
			kjlen = CHPTTOSH(kjsvpt->kjdataf) - C_DBCS;
			kjsvpt->kjdataf[kjlen] = pt->string[strpos];
			kjsvpt->kjdataf[kjlen+1] = pt->string[strpos+1];
			return(IMSUCC);
		    }
		    else
			nocnvflg = C_SWOFF;

		    break;
                default:

                    nocnvflg = C_SWON;

                    return( IMSUCC );
                };
            } else {

                nocnvflg = C_SWON;
            };

        } else {

            nocnvflg = C_SWON;
        };



        if ( nocnvflg ) {       /* Not Flying Conversion.               */

            if ( pt->type == K_INASCI ) {       /* Ascii Character Input.*/

                /*
                 *      Get Old Cursor Position.
                 */
                oldcol = pt->curcol - kjsvpt->chcodlen;

                if ( (kjsvpt->cconvpos != kjsvpt->fconvpos) &&
                     (oldcol            < kjsvpt->fconvpos)  ) {

                    /*
                     *      Reset Flying Conversion Area.
                     */
                    _Mflyrst( pt );

                    /*
                     *      Set Flying Conversion Position.
                     */
                    kjsvpt->fconvpos = kjsvpt->cconvpos;
                } else {

                    /*
                     *      Reset Flying Conversion Error Flag.
                     */
                    kjsvpt->fcnverfg = C_SWOFF;
                };
            } else if ( pt->type == K_INESCF ) {/* Pseudo Code.         */

                /*
                 *      Check Input Code.
                 */
                switch( pt->code ) {

                case P_DELETE:

                    oldcol = pt->curcol;/* Get Old Cursor Position.     */

                    /*
                     *      Check Old Cursor Position.
                     */
                    if ( (kjsvpt->cconvpos != kjsvpt->fconvpos) &&
                         (oldcol           <= kjsvpt->fconvpos)  ) {

                        /*
                         *      Reset Flying Conversion Area.
                         */
                        _Mflyrst( pt );

                        /*
                         *      Set Flying Conversion Position.
                         */
                        kjsvpt->fconvpos = kjsvpt->cconvpos;
                    } else {

                        /*
                         *      Reset Flying Conversion Error Flag.
                         */
                        kjsvpt->fcnverfg = C_SWOFF;
                    };

                    break;

                case P_BSPACE:

                    /*
                     *      Get Old Cursor Position.
                     */
                    oldcol = pt->curcol + C_DBCS;

                    /*
                     *      Check Old Cursor Position.
                     */
                    if ( (kjsvpt->cconvpos != kjsvpt->fconvpos) &&
                         (oldcol           <= kjsvpt->fconvpos)  ) {

                        /*
                         *      Reset Flying Conversion Area.
                         */
                        _Mflyrst( pt );

                        /*
                         *      Set Flying Conversion Position.
                         */
                        kjsvpt->fconvpos = kjsvpt->cconvpos;
                    } else {

                        /*
                         *      Reset Flying Conversion Error Flag.
                         */
                        kjsvpt->fcnverfg = C_SWOFF;
                    };
                    break;

                default:
                    break;
                };
            };

        } else {        /* Flying Conversion.                           */

            /*
             *      Loop Start and End Position.
             */
            loop1st = kjsvpt->fconvpos;
            loopend = kjsvpt->cconvpos + kjsvpt->cconvlen;

            /*
             *      Initialize Conversion Flag.
             */
            cnvmodfg = C_SWOFF;
            modchgfg = C_SWOFF;

            for ( loop1 = loop1st ; loop1 < loopend ; ) {

                /*
                 *      Reset Conversion Flag.
                 */
                cnvmodfg = C_SWOFF;
                modchgfg = C_SWOFF;

                fpos = loop1;   /* Set Conversion Position.             */

                flen = C_DBCS;  /* Set Conversion Length.               */

                /*
                 *      Get Character Mode of First Position.
                 */
                cmode = _Mgetchm( pt->string , fpos , kjsvpt );

                /*
                 *      Check Character Mode.
                 */
                if (   ( cmode == K_CHHIRA )  ||
                       ( (cmode == K_CHKATA) &&
                         (kjsvpt->kmpf[0].katakana == K_KANAON) )   ) {

                    cnvmodfg = C_SWOFF;
                } else {

                    cnvmodfg = C_SWON;
                };

                for ( loop2 = (fpos + C_DBCS) ;
                      loop2 < loopend ;
                      loop2 += C_DBCS ) {

                    /*
                     *      Get Character Mode of Current Position.
                     */
                    fmode = _Mgetchm( pt->string , loop2 , kjsvpt );

                    /*
                     *      Check Character Mode.
                     */
                    if (   ( cmode == K_CHHIRA )  ||
                           ( (cmode == K_CHKATA) &&
                             (kjsvpt->kmpf[0].katakana == K_KANAON) )   ) {

                        switch( fmode ) {

                        case K_CHHIRA:

                            flen += C_DBCS; /* Increase Convert Length. */
                            modchgfg = C_SWOFF;
                            break;

                        case K_CHKATA:

                            if ( kjsvpt->kmpf[0].katakana == K_KANAON ) {

                                modchgfg = C_SWOFF;
                            } else {

                                modchgfg = C_SWON;
                            };

                            flen += C_DBCS; /* Increase Convert Length. */
                            break;

                        default:

                            flen += C_DBCS; /* Increase Convert Length. */
                            modchgfg = C_SWON;
                            break;
                        };

                    } else {

                        switch( fmode ) {

                        case K_CHHIRA:

                            modchgfg = C_SWON;
                            break;

                        case K_CHKATA:

                            if ( kjsvpt->kmpf[0].katakana == K_KANAON ) {

                                modchgfg = C_SWON;
                            } else {

                                flen += C_DBCS;
                                modchgfg = C_SWOFF;
                            };
                            break;

                        default:

                            flen += C_DBCS; /* Increase Convert Length. */
                            modchgfg = C_SWOFF;
                            break;
                        };
                    };

                    if ( modchgfg ) {

                        if ( cnvmodfg ) {

                           _Mansave( pt, fpos, flen );
                        } else {

                           _Midecid( pt, fpos, flen, cmode );
                        };

                        break;
                    };
                };

                loop1 += flen;
            };

            if ( !modchgfg ) {

                if ( cnvmodfg ) {

                    _Mansave( pt, fpos, flen );
                } else {

                    _Mflycnv( pt, fpos, flen, cmode );
                };
            };
        };



        /* 3.
         *      Return.
         */

        return( IMSUCC );
}
