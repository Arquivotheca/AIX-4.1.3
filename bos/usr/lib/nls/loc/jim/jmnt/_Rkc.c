static char sccsid[] = "@(#)53	1.2.1.2  src/bos/usr/lib/nls/loc/jim/jmnt/_Rkc.c, libKJI, bos411, 9428A410j 6/10/94 14:52:02";
/*
 * COMPONENT_NAME :	Japanese Input Method - Kanji Monitor
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:         _Rkc
 *
 * DESCRIPTIVE NAME:    Romaji kana conversion.
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
 * FUNCTION:            Convert romaji string to kana string.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        14452 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Rkc
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Rkc( mode,romaji,rlen,kana,klen,rset,rslen )
 *
 *  INPUT:              mode    : romaji to kana conversion mode.(1,2 or 3)
 *                      romaji  : input string. (romaji)
 *                      rlen    : input string length. (byte)
 *
 *  OUTPUT:             kana    : return string.(Pc code or Pc kanji code)
 *                      klen    : return string length.(byte)
 *                      rest    : remaining string.
 *                      rslen   : remaining string length.(byte)
 *
 * EXIT-NORMAL:         IMSUCC  : Success of Execution.
 *
 *
 *
 * EXIT-ERROR:          IMFAIL  : Nothing input string on table.
 *
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              NA.
 *                      Standard Library.
 *                              memset() : Set the characters
 *                                             in specified memory area.
 *                              memcpy() : Memory copy function.
 *                              bsearch(): Binary search function.
 *                              islower(): Check a lower case letter.
 *
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

#define _ILS_MACROS

/*
 *      include Standard.
 */
#include <stdio.h>      /* Standard I/O Package.                        */
#include <ctype.h>      /* Check lower case.                            */
#include <memory.h>     /* Performs memory operations.                  */

/*
 *      include Kanji Project.
 */
#include "kj.h"         /* Kanji Project Define File.                   */
#include "kcb.h"        /* Kanji Control Block Structure.               */
#include "_Rkc.t"        /* Romaji To Kana Conversion Table Structure.   */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1988           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

static int  _Rkc1_5();
static int  _Rkc1_6();

/*
 *      Compare two data strings.(for bsearch function.)
 */
static  int     _Rkccomp( tbdt1,tbdt2 )
char   *tbdt1;   /* compare data(1) address.                            */
char   *tbdt2;   /* compare data(2) address.                            */
{
        return ( strcmp (
                        ((struct table *)tbdt1)->romaji,
                        ((struct table *)tbdt2)->romaji ));
}


/*
 *      Convert romaji string to kana string.
 */
_Rkc( mode,romaji,rlen,kana,klen,rest,rslen )

short   mode;           /* romaji to kana conversion mode(1,2 or 3)    */
uchar  *romaji;         /* input string (romagi ?)                     */
short   rlen;           /* input string length (byte)                  */
uchar  *kana;           /* return string (PC code or PC kanji code)    */
short  *klen;           /* return string length (byte)                 */
uchar  *rest;           /* remaining string                            */
short  *rslen;          /* remaining string length (byte)              */
{
        short   ret_code;       /* Return Code.                         */

        char   *memset();
        char   *memcpy();

        struct  table   *tp,t1; /* romaji to kana coversion table
                                   pointer and input romaji data area. */

        short   c;              /* Character.                           */
        short   i,n;            /* Loop Counter                         */
        short   p1;             /* Rkc table internal pointer.          */
        short   rsize;          /* Romaji length.                       */
        uchar   rsave;          /* Last character save area.            */
        uchar   xsave;          /* 'x' character save area.             */
        char    tvowel[5];      /* Vowel character table.               */

        tvowel[0] = 'a';        /* Initialize vowel character table.    */
        tvowel[1] = 'i';
        tvowel[2] = 'u';
        tvowel[3] = 'e';
        tvowel[4] = 'o';

        rsize     =  0;         /* Internal romaji string length.       */
        rsave     =  0x00;      /* A byte code save area.               */
        xsave     =  0x00;      /* 'x' code save area.                  */

/************************************************************************
 *      debug process routine.                                          */
CPRINT(START RKC);
        /* 1.1
         *      Make sure all input characters are lower case letter.
         */

        /* Romaji string area NULL clear.        */
        memset(t1.romaji,NULL,6);

        /* The maximum length is 6 bytes.        */
        if (rlen > 6)
                rlen = 6;

        /* Input the romaji string.              */
        memcpy(t1.romaji,romaji,rlen);

        /* Set default mode.                     */
        if ( (mode < 1) || (mode > 3) )
                mode = 3;  /*  the default mode is 3. (3:hirakana)      */

        /* Make sure all input characters are lower case letter.        */
        for (i=0;i < rlen;i++)
            {
                /* Get a check code in romaji string.                   */
                c  =  t1.romaji[i];

                /* 1.1.a
                 *         Not alphabet character.
                 */
                if (islower(c) == 0 )
                    {
                        /* not lower case code is rsave                 */
                        rsave           = t1.romaji[i];

                        /* clear the romaji string area.                */
                        memset(t1.romaji,NULL,6);

                        /* set a not lower case code.                   */
                        t1.romaji[0]    = rsave;
                        /* set input code length.                       */
                        rsize           = 1;
                        /* clear a code save area.                      */
                        rsave           = 0x00;

                        /* Get the conversion code from rkc table.      */
                        ret_code = _Rkc1_6(mode,t1,rsize,kana,klen,rest,
                                                        rslen,rsave,xsave);
/************************************************************************
 *      debug process routine.                                          */
CPRINT(END RKC1.1.a);
                        /* Complete the code conversion.                */
                        return( ret_code );
                    }
             }
        /* Internal romaji string size is same as input romaji length.  */
        rsize = rlen;

        /* 1.2
         *      Check a vowel is the romaji last character.
         */
        n  =  rsize - 1;
        /*  Compare the vowel table code to romaji last character code. */
        for (i=0;i < 5;i++)
            {
                /* 1.2.a
                 *         Last character is vowel.
                 */
                if (t1.romaji[n] == tvowel[i])
                    {
                        /* Get the conversion string from rkc table.    */
                        ret_code = _Rkc1_6(mode,t1,rsize,kana,klen,rest,
                                                        rslen,rsave,xsave);
/************************************************************************
 *      debug process routine.                                          */
CPRINT(END RKC 1.2.a);
                        /* Complete the code conversion.                */
                        return( ret_code );
                    }
            }

        /* 1.2.b   Next process.(Last character is a consonant.)        */

        /* 1.3
         *         Check the double consonants string.
         */
        if ( rsize == 2 )
            {
                /* 1.3.a  The double consonants process.                */
                if ( t1.romaji[0] == t1.romaji[1] )
                    {
                        /* Binary search the rkc table.                 */
                        tp = (struct table *)bsearch( (char *)(&t1),
                             (char *)tabledt,TABSIZE,sizeof(t1),_Rkccomp);
                        if ( tp == NULL )
                        /* Nothing double consonants string on table.   */
                            {
/*************************************************************************
 *      debug process routine.                                           */
CPRINT(ERROR RKC1.3.a);
                            /* Error complete the code conversion.       */
                            return ( IMFAIL );           /* error return */
                            }
                        /* The double consonants is on table.            */
                        /* The conversion mode 0,1 or 2 set.             */
                        p1     = mode -1;

                        /* Set the conversion string from rkc table
                                                           to output.   */
                        memcpy( kana,&(tp->kana[p1][0]),tp->klen[p1]);

                        /* Set the conversion string length.            */
                        *klen  = tp->klen[p1];

                        /* remaine is one of input romaji string.       */
                        *rest  = t1.romaji[0];
                        /* the remain length is one byte.               */
                        *rslen = 1;

                        /* if input string is "nn" then
                                           remain string is noting.     */
                        if ( t1.romaji[0] == 'n' )
                            {
                                *rest  = 0x00;
                                *rslen = 0;
                            }

/************************************************************************
 *      debug process routine.                                          */
CPRINT(END RKC1.3.a);
                    /* Complete the code conversion.(double consonants) */
                        return ( IMSUCC );             /* normal return */
                    }
            }

        /* 1.3.b  Next proccess.
         (the string is a consonant or the different consonants string.)*/

        /* 1.4
         *      Check the farst caracter.('x'?)
         */

        /* 1.4.a
         *        Farst character is not 'x'.
         */
        if ( t1.romaji[0] != 'x' )
            {
            /* the string is consonant string without 'x' character.    */

                /* Rkc table serch process.                             */
                ret_code = _Rkc1_5(mode,t1,rsize,kana,klen,rest,
                                                rslen,rsave,xsave);
/************************************************************************
 *      debug process routine.                                          */
CPRINT(END RKC1.4.a);
                /* Complete the code conversion.                        */
                return ( ret_code );
            }

        /* 1.4.b
         *          Farst character is 'x'
         */

        if ( rsize == 1 )

            /* The input string is 'x'.                                 */
            {
                *rest  ='x';
                *rslen = 1;
/************************************************************************
 *      debug process routine.                                          */
CPRINT(END RKC1.4.b);
                /* Complete the code conversion.                        */
                return ( IMSUCC );                     /* normal return */
            }

        /* Save the ferst 'x' character.                                */
        xsave = t1.romaji[0];

        /* Shorten the string a byte.                                   */
        memcpy( &(t1.romaji[0]),&(t1.romaji[1]),rsize );

        /* Decrement the string length.                                 */
        rsize = rsize - 1;

        /* 1.4.b.1  Check the double consonants.                        */
        if ( rsize == 2 )
            {
                if ( t1.romaji[0] == t1.romaji[1] )
                    /* 1.4.b.1.a  The Double Consonant Process.         */
                    {
                        /* Binary search the rkc table.                 */
                        tp = (struct table *)bsearch((char *)(&t1),
                             (char *)tabledt,TABSIZE,sizeof(t1),_Rkccomp);
                        if ( tp == NULL )
                        /* Nothing double consonants string on table.   */
                            {
/*************************************************************************
 *      debug process routine.                                           */
CPRINT(ERROR RKC1.4.b.1);
                            /* Error complete the code conversion.       */
                            return ( IMFAIL );           /* error return */
                            }
                        /* The double consonants is on table.            */

                        /* The conversion mode 0,1 or 2 set.             */
                        p1     = mode -1;

                        /* Set the conversion string from rkc table
                                                           to output.   */
                        memcpy( kana,&(tp->kana[p1][0]),tp->klen[p1] );

                        /* Set the conversion string length.            */
                        *klen  = tp->klen[p1];

                        /* remaine is one of input romaji string.       */
                        *rest  = t1.romaji[0];
                        /* the remain length is one byte.               */
                        *rslen = 1;

                        /* if input string is "nn" then
                                           remain string is noting.     */
                        if ( t1.romaji[0] == 'n' )
                            {
                                *rest  = 0x00;
                                *rslen = 0;
                            }
/*************************************************************************
 *      debug process routine.                                           */
CPRINT(END RKC1.4.b.1);
                    /* Complete the code conversion.(double consonants) */
                        return ( IMSUCC );             /* normal return */
                    }
            }

        /* 1.4.b.1.b  the string is a consonant
                                   or different consonants string.      */
        ret_code = _Rkc1_5(mode,t1,rsize,kana,klen,rest,
                                        rslen,rsave,xsave);

/************************************************************************
 *      debug process routine.                                          */
CPRINT(END RKC1.4.b.1.b);
        /* Complete the code conversion.                                */
        return ( ret_code );
}

        /* 1.5
         *      Table search process.(input string is lower case.)
         *      ( The string is not double consonants or
         *             the string last character is not vowel. )        */

static int  _Rkc1_5 ( mode,t1,rsize,kana,klen,rest,rslen,rsave,xsave )

struct  table   t1;     /* input romaji data area.                      */
short   mode;           /* romaji to kana conversion mode.(1,2 or 3)    */
short   rsize;          /* input string length.(byte)                   */
uchar  *kana;           /* return string.(PC code or PC kanji code)     */
short  *klen;           /* return string length.(byte)                  */
uchar  *rest;           /* remaining string.                            */
short  *rslen;          /* remaining string length.(byte)               */
uchar   rsave;          /* last character save area.                    */
uchar   xsave;          /* 'x' character save area.                     */
{
        short   ret_code;        /* Return Code.                        */

        struct  table   *tp; /* romaji to kana coversion table pointer  */

        /* 1.5    Binary search the rkc table.                          */
        tp = (struct table *)bsearch((char *)(&t1),
                     (char *)tabledt,TABSIZE,sizeof(t1),_Rkccomp);

        /* 1.5.a
         *         The romaji(consonant) string is on rkc table.
         */
        if ( tp != NULL )
            {
                /* Rkc table search is successful.                      */

                /* Output string length is 0 clear.                     */
                *rslen = 0;

                /* If 'x' character save area is full then output 'x'.  */
                if ( xsave == 'x' )
                    {
                        *rest   = 'x';  /* Output string is 'x'.        */
                        *rslen  = 1;    /* Output string length is 1.   */
                        rest++;         /* Inclement the output pointer */
                    }
                /* All input romaji string is output.                   */
                memcpy(rest,&(t1.romaji[0]),rsize);

                /* Add the output string length.                        */
                *rslen = *rslen + rsize;

                /* Complete the code conversion.                        */
                return ( IMSUCC );                    /* normal return  */
            }

        /* 1.5.b
         *        The romaji(consonant) is nothing on rkc table.
         */

        /* 1.5.b.1 Input romaji(consonant) string length is 1.          */
        if ( rsize == 1 )
                return ( IMFAIL );                    /* error return   */

        /* 1.5.b.2  The string last is a extra character.               */

        /* 'x' character save area is NULL clear.                       */
        xsave = 0x00;

        /* Save the string last character.                              */
        rsave = t1.romaji[rsize - 1];

        /* NULL set the string last.                                    */
        t1.romaji[rsize - 1] = 0x00;

        /* Decrement the string length.                                 */
        rsize = rsize - 1;

        /* Get the conversion string from rkc table.                    */
        ret_code = _Rkc1_6(mode,t1,rsize,kana,klen,rest,
                                        rslen,rsave,xsave);

        /* Complete the code conversion.                                */
        return ( ret_code );
}

        /* 1.6
         *      Table search process.
         */

static  int _Rkc1_6 ( mode,t1,rsize,kana,klen,rest,rslen,rsave,xsave )

struct  table   t1;     /* input romaji data area                       */
short   mode;           /* romaji to kana conversion mode(1,2 or 3)     */
short   rsize;          /* input string length (byte)                   */
uchar  *kana;           /* return string (PC code or PC kanji code)     */
short  *klen;           /* return string length (byte)                  */
uchar  *rest;           /* remaining string                             */
short  *rslen;          /* remaining string length (byte)               */
uchar   rsave;          /* last character save area.                    */
uchar   xsave;          /* 'x' character save area.                     */
{
        short   ret_code;    /* Return code.                            */

        struct  table   *tp; /* romaji to kana coversion table pointer  */

        short   p1;     /* rkc table internal pointer.                  */

        /* 1.6    Binary search the rkc table.                          */
        tp = (struct table *)bsearch((char *)(&t1),
                     (char *)tabledt,TABSIZE,sizeof(t1),_Rkccomp);
        if ( tp != NULL )
            {
                /* 1.6.a   The string is on table.                      */

                /* The conversion mode 0,1 or 2 set.                    */
                p1    = mode - 1;

                /* Set the conversion string from rkc table to output.  */
                memcpy( kana,&(tp->kana[p1][0]),tp->klen[p1]);

                /* Set the conversion string length.                    */
                *klen = tp->klen[p1];

                /* Set remain string.                                   */
                if ( rsave != 0x00 )
                    {
                        *rest  = rsave; /* Set remain string.(consonant)*/
                        *rslen = 1;     /* Set remain length.           */
                    }

                /* Complete the code conversion.                        */
                return ( IMSUCC );                     /* normal return */
            }

        /*  1.6.b
         *      The string is nothing on table.(account of character.)
         */

        /* 1.6.b.1  Noting character on conversion table.               */
        if ( (rsize == 1) || (t1.romaji[0] != 'x') )

            /* Error complete the code conversion.                      */
            return ( IMFAIL );                          /* error return */

        /* 1.6.b.2   Start character is 'x' character.                  */

        /* Throw away a start character.                                */
        memcpy( &(t1.romaji[0]),&(t1.romaji[1]),rsize );

        /* Decrement the strig length.                                  */
        rsize = rsize - 1;

        /* Binary search the rkc table.                                 */
        tp = (struct table *)bsearch((char *)(&t1),
                     (char *)tabledt,TABSIZE,sizeof(t1),_Rkccomp);
        if ( tp != NULL )
            {
                /* The strings is on table.                             */

                /* The conversion mode 0,1 or 2 set.                    */
                p1    = mode - 1;

                /* Set the conversion string from rkc table to output.  */
                memcpy( kana,&(tp->kana[p1][0]),tp->klen[p1]);

                /* Set the conversion string length.                    */
                *klen = tp->klen[p1];

                /* Complete the code conversion.                        */
                return ( IMSUCC );                     /* normal return */
            }

         /* Nothing conversion table.                                   */
         return ( IMFAIL );                             /* error return */
}
