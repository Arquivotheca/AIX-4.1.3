static char sccsid[] = "@(#)06	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mgetchm.c, libKJI, bos411, 9428A410j 7/23/92 03:22:22";
/*
 * COMPONENT_NAME :	Japanese Input Method - Kanji Monitor
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         _Mgetchm
 *
 * DESCRIPTIVE NAME:    Get the character mode.
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
 * FUNCTION:            Get the character mode.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        736 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         Module Entry Point Name
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mgetchm( str,pos,kjsvpt )
 *
 *  INPUT:              str     : Pointer to string.
 *                      pos     : Character position in String.
 *                      kjsvpt  : Pointr to KMISA.
 *
 *  OUTPUT:             N.A.
 *
 * EXIT-NORMAL:         The mode of character : Input code is PC Kanji code.
 *                      K_CHBYTE.             : input code is PC code.
 *
 *
 * EXIT-ERROR:          N.A.
 *
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
 *                      Kanji Monitor Controle Block(KCB).
 *                              NA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              NA.
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
 *      Return the character mode.
 */
uchar _Mgetchm( str, pos, kjsvpt )

uchar    *str;       /* Pointer to input string.(I)                     */
ushort   pos;        /* Character position on input string.(I)          */
KMISA   *kjsvpt;     /* Pointer to KMISA                                */
{
        int     high;           /* Input character.                     */
        int     code;           /* PC Kanji code.                       */

/************************************************************************
 *      debug process routine.                                          */
CPRINT(START KMGETCHM);

        /* 1.1 Input string convert to integer code.                    */
        high = str[pos];

        /* 1.1.a Check the start byte code is PC code.                  */
        if ( !((high >= C_PKC1BL) && (high <= C_PKC1BH)) &&
             !((high >= C_PKC2BL) && (high <= C_PKC2BH)) )
                {
/************************************************************************
 *      debug process routine.                                          */
CPRINT(END 1.1.a KMGETCHM);

                /* Return code is PC byte code mode code.               */
                return( K_CHBYTE );
                }

        /* 1.3 Get PC kanji code from input string.                     */
        code = high * 256 +(int)str[++pos];

        /* 1.4 Get the kind of PC Kanji code.                           */

        /* 1.4.1 Check hiragana character.                              */
        if ( (code == 0x815b) || ((code >= 0x829f) && (code <= 0x82f1)) )
                {
/************************************************************************
 *      debug process routine.                                          */
CPRINT(END 1.4.1 KMGETCHM);

                /* Return code is hiragana mode code.                   */
                return(K_CHHIRA);
                }

        /* 1.4.2 Check katakana character.                              */
        if ( (code >= 0x8340) && (code <= 0x8396) )
                {
/************************************************************************
 *      debug process routine.                                          */
CPRINT(END 1.4.2 KMGETCHM);

                /* Return code is katakana mode code.                   */
                return(K_CHKATA);
                }

        /* 1.4.3 Check Alphabet character.                              */
/************************************************************************
 *      debug process routine.                                          */
CPRINT(END 1.4.3 KMGETCHM);

        if ( (code >= 0x8146) && (code <= 0x8149) )	/* : ; ? ! 	*/
                return(K_CHALPH);
        if ( (code == 0x815e) )				/* /   		*/
                return(K_CHALPH);
        if ( (code == 0x8169) || (code == 0x816a) )	/* ( ) 		*/
                return(K_CHALPH);
        if ( (code == 0x817b) || (code == 0x817c) )	/* + - 		*/
                return(K_CHALPH);
        if ( (code == 0x8181) )				/* =		*/
                return(K_CHALPH);
        if ( (code == 0x8183) || (code == 0x8184) )	/* < >		*/
                return(K_CHALPH);
        if ( (code >= 0x8193) && (code <= 0x8197) )	/* % # & * @	*/
                return(K_CHALPH);
        if ( (code == C_LIT ) || (code == C_QUOTO) )	/* ' "		*/
                return(K_CHALPH);
        if ( (code >= 0x8260) && (code <= 0x8279) )	/* A Z		*/
                if ( kjsvpt->kmpf[0].alphanum == K_ALPON ) {
                        return(K_CHHIRA);
                } else {
                        return(K_CHALPH);
                }
        if ( (code >= 0x8281) && (code <= 0x829a) )	/* a z		*/
                if ( kjsvpt->kmpf[0].alphanum == K_ALPON ) {
                        return(K_CHHIRA);
                } else {
                        return(K_CHALPH);
                }
        if ( (code >= 0x824f) && (code <= 0x8258) )	/* 0 9		*/
                if ( kjsvpt->kmpf[0].alphanum == K_ALPON ) {
                        return(K_CHHIRA);
                } else {
                        return(K_CHNUM);
                }

        if ( (code == 0x8143) || (code == 0x8144) )	/* , . 		*/
     		return(K_CHNUM);

        /* Check blank code character.                                  */
        if (code == 0x8140)
                {
                /* Return code is blank mode code.                      */
                return(K_CHBLNK);
                }
        /* Another one. return code is sign mode code.                  */
        return(K_CHKIGO);

}

