static char sccsid[] = "@(#)94	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mdaha.c, libKJI, bos411, 9428A410j 7/23/92 03:21:27";
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
 * MODULE NAME:         _Mdaha
 *
 * DESCRIPTIVE NAME:    Dakuten handakuten letter processing.
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
 * FUNCTION:            Try to convert input string with dakuten/handakuten
 *                                      code to a dakuten/handakuten letter.
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1188 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mdaha
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mdaha( kanain, kanaout )
 *
 *  INPUT:              kanain  :Input string.(Two PC Code or PC Code )
 *
 *
 *  OUTPUT:             kanaout :Output string.(PC Kanji Code or NULL
 *                                                                  String)
 * EXIT-NORMAL:         IMSUCC :Success of Execution.
 *
 * EXIT-ERROR:          IMFAIL :Nothing in conversion table.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              NA.
 *                      Standard Library.
 *                              bsearch  Binary search function.
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
 *                              _Mdaha.t   Dakuten/handakuten conversion
 *                                                                 table.
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
#include <memory.h>     /* Set memory & copy memory                     */
/*
 *      include Kanji Project.
 */
#include "kj.h"         /* Kanji Project Define File.                   */
#include "kcb.h"        /* Kanji Control Block Structure.               */
#include "_Mdaha.t"     /* Dakuten/handakuten conversion table          */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1988           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

static  int     rcompare();

/*
 *      Please Descripte This Module.
 */
_Mdaha( kanain,kanaout )

uchar   *kanain;    /* Input string (Two PC Code or PC Code and PC code)*/
uchar   *kanaout;   /* Output string (PC Kanji Code or NULL String)     */
{
        int     rcompare();     /* Dakuten table convert function.      */
        char    *memset();      /* The character memory set.            */
        char    *memcpy();      /* The strig copy from memory to memory.*/
        struct  table   *tp,t;  /* Dakuten conversion table pointer
                                   and input data area.                 */

/************************************************************************
 *      debug process routine.                                          */
CPRINT(START KMDAHA);

        /*  1.1
         *      Check Conversion mode.
         */
        /* Bsearch source data area NULL clear.                         */
        memset(t.t1,NULL,4);

        /* Source data( PC code string) input.                          */
        sprintf(t.t1,"%.4s",kanain);

        /* Conversion table search, get DBCS code pointer on table.     */
        tp = (struct table *)bsearch( (char *)(&t),(char *)td,
                                       TABSIZE,sizeof(t),rcompare );

        /* 1.1.a The conversion successfully finished.                  */
        if (tp != NULL)
            {
                /* DBCS code output.                                    */
                sprintf( kanaout,"%.4s",tp->t2 );
/************************************************************************
 *      debug process routine.                                          */
CPRINT(END KMDAHA);

                /* Conversion finished completely.                      */
                return ( IMSUCC );
            }

        /* 1.1.b The input string is nothing on table.                  */
/************************************************************************
 *      debug process routine.                                          */
CPRINT(ERROR KMDAHA);

        /* Convert incomplete.                                          */
        return ( IMFAIL );
}

        /*
         *      Compare two data strings.(for bsearch function.)
         */

static  int     rcompare ( tbdt1,tbdt2 )
char   *tbdt1;   /* compare data(1) address.                            */
char   *tbdt2;   /* compare data(2) address.                            */
{
        return ( strcmp (
                        ((struct table *)tbdt1)->t1,
                        ((struct table *)tbdt2)->t1  ));
}
