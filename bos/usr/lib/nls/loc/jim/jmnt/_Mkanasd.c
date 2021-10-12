static char sccsid[] = "@(#)18	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mkanasd.c, libKJI, bos411, 9428A410j 7/23/92 03:23:11";
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

/********************* START OF MODULE SPECIFICATIONS *********************
 *
 * MODULE NAME:         _Mkanasd
 *
 * DESCRIPTIVE NAME:    Converter a single byte kana string
 *                                          to double byte string.
 * COPYRIGHT:           5601-061 COPYRIGHT IBM CORP 1988
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              Kanji Monitor V1.0
 *
 * CLASSIFICATION:      OCO Source Material - IBM Confidential.
 *                      (IBM Confidential - Restricted when aggregated)
 *
 * FUNCTION:            Convert single byte strings to double bytes strings.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1936 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mkanasd
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kmkakasd( cnvmd,sbstr,len,dbstr )
 *
 *  INPUT:              cnvmd   : Conversion mode.
 *                      sbstr   : Single byte code string.
 *                      len     : Length of single byte code string.
 *
 *  OUTPUT:             dbstr   : Double byte code string.
 *
 * EXIT-NORMAL:         IMSUCC :Success of Execution.
 *
 *
 * EXIT-ERROR:          IMSDVNLE : input string length is zero.
 *                      IMSDCVME : input conversion mode error.
 *
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              NA.
 *                      Standard Library.
 *                              memcpy : memory copy function.
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
 *                              _Mkanasd.t : conversion table.
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
#include <memory.h>     /* Preforms memory operations.                  */

/*
 *      include Kanji Project.
 */
#include "kj.h"         /* Kanji Project Define File.                   */
#include "kcb.h"        /* Kanji Control Block Structure.               */
#include "_Mkanasd.t"   /* conversion table                             */
#include "_Myomic.t"    /* YOMI code  table                             */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1988           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Converter a single byte kana string
 *                          to double byte string.
 */
int     _Mkanasd(cnvmd,sbstr,len,dbstr)

char    cnvmd;          /* convert mode                                */
char   *sbstr;          /* single byte string                          */
short   len;            /* length of shingle byte string               */
uchar  *dbstr;          /* double byte string                          */
{
        char    *memcpy();      /* Function copes from memory to memory.*/

        int     i;              /* Loop Counter.                        */
        char    sbch;           /* source data buffer.                  */
        uchar   convch[2];      /* distination data buffer.             */
        int     dblen;          /* distination data pointer.            */

/*************************************************************************
 *      debug process routine.                                           */
CPRINT(START KMKANASD);

        /*  1.1
         *      Check input parameter.
         */
        /* Check input lengthof single byte code string.                */
        if (len == 0)
                {
/************************************************************************
 *      debug process routine.                                          */
CPRINT(END 1.1 KMKANASD);
                /* input string length is zero.                         */
                return ( IMSDVNLE );   /* Error of execution.           */
                }

        /* Check the Hiragana conversion mode.                          */
        if (cnvmd == M_HIRACV)
             {
             /*   1.2 ( Hiragana conversion mode )
              *      Convert single byte string to double bytes string.
              */
                  /* 1.2.0    output pointer initiarized                */
                  dblen = C_INIT;

                  /* Conversion process count is len - 1.               */
                  for (i=0;i<=len-1;i++)  /* Loop count parameter is i  */
                      {
                          /* 1.2.1  Get a single byte code from input str.*/
                          sbch  = *sbstr;

                          /* Single byte code string pointer increment. */
                          sbstr = sbstr + C_ANK;

			  if( sbch <= 0x7f ) {
                              /* 1.2.2 convert the hiragane string.         */
                              memcpy(convch,thira[sbch].dt,C_DBCS);
			  }
			  else {
                              /* Single byte code string pointer increment. */
			      if(( sbch >= EM_a ) && ( sbch <= EM_z )) 
                                  sbch -= M_DULOWER;
			      else if(( sbch >= EM_A ) && ( sbch <= EM_Z )) 
                                  sbch -= M_DUPPER;

                              /* 1.2.2 convert the Alpha string.            */
                              memcpy(convch,tdalph[sbch].dt,C_DBCS);
			  }

                          /* 1.2.3 put Hiragana character to distination*/
                          memcpy(&(dbstr[dblen]),convch,C_DBCS);

                          /* double byte code string pointer increment. */
                          dblen = dblen + C_DBCS;
                      }
/************************************************************************
 *      debug process routine.                                          */
CPRINT(END 1.2.3 KMKANASD);

                  /* Success of execution.                              */
                  return( IMSUCC );
             }

        /* Check the Katakana conversion mode.                          */
        if (cnvmd == M_KATACV)
             {
             /*   1.3 ( Katakana conversion mode )
              *      Convert single byte string to double bytes string.
              */
                  /* 1.3.0    output pointer initiarized                */
                  dblen = C_INIT;

                  /* Conversion process count is len - 1.               */
                  for (i=0;i<=len-1;i++)  /* Loop count parameter is i  */
                      {
                          /* 1.3.1  Get a single byte code from input str.*/
                          sbch  = *sbstr;

                          /* Single byte code string pointer increment. */
                          sbstr = sbstr + C_ANK;

                          /* 1.3.2 convert the katanana character.      */
                          memcpy(convch,tkana[sbch].dt,C_DBCS);

                          /* 1.3.3 put Katakana character to distination*/
                          memcpy(&(dbstr[dblen]),convch,C_DBCS);

                          /* double byte code string pointer increment. */
                          dblen = dblen + C_DBCS;
                      }
/************************************************************************
 *      debug process routine.                                          */
CPRINT(END 1.3.3 KMKANASD);

                  /* Success of execution.                              */
                  return( IMSUCC );
             }
        /* Check the Hiragana conversion mode.                          */
        if (cnvmd == M_NUMCV)
             {
             /*   1.4 ( Number character conversion mode )
              *      Convert single byte string to double bytes string.
              */
                  /* 1.4.0    output pointer initiarized                */
                  dblen = C_INIT;

                  /* Conversion process count is len - 1.               */
                  for (i=0;i<=len-1;i++)
                      {
                          /* 1.4.1  Get a single byte code from input str.*/
                          sbch  = *sbstr;

                          /* Single byte code string pointer increment. */
                          sbstr = sbstr + C_ANK;

                          /* 1.4.2 convert the number string.           */
                          memcpy(convch,tnumb[sbch].dt,C_DBCS);

                          /* 1.4.3 put Number character to distination. */
                          memcpy(&(dbstr[dblen]),convch,C_DBCS);

                          /* double byte code string pointer increment. */
                          dblen = dblen + C_DBCS;
                      }
/************************************************************************
 *      debug process routine.                                          */
CPRINT(END 1.4.3 KMKANASD);

                  /* Success of execution.                              */
                  return( IMSUCC );
             }

        /* Check the Alphabet conversion mode.                          */
        if (cnvmd == M_ALPHCV)
             {
             /*   1.5 ( Alphabet conversion mode )
              *      Convert single byte string to double bytes string.
              */
                  /* 1.5.0    output pointer initiarized                */
                  dblen = C_INIT;

                  /* Conversion process count is len - 1.               */
                  for (i=0;i<=len-1;i++)  /* Loop count parameter is i  */
                      {
                          /* 1.3.1  Get a single byte code from input str.*/
                          sbch  = *sbstr;

                          /* Single byte code string pointer increment. */
                          sbstr = sbstr + C_ANK;

                          /* 1.5.2 convert the alphabet character.      */
                          memcpy(convch,talph[sbch].dt,C_DBCS);

                          /* 1.5.3 put Alphabet character to distinasion*/
                          memcpy(&(dbstr[dblen]),convch,C_DBCS);

                          /* double byte code string pointer increment. */
                          dblen = dblen + C_DBCS;
                      }
/*************************************************************************
 *      debug process routine.                                           */
CPRINT(END 1.5.3 KMKANASD);

                  /* Success of execution.                              */
                  return( IMSUCC );
             }

        /* The mode process is nothing.                                 */
/************************************************************************
 *      debug process routine.                                          */
CPRINT(ERROR 1.5.3 KMKANASD);

        /* input conversion mode error.                                 */
        return ( IMSDCVME );
}
