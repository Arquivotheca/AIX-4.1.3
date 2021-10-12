static char sccsid[] = "@(#)38	1.1  src/bos/usr/lib/nls/loc/methods/shared.bidi/blkapifn.c, cfgnls, bos411, 9428A410j 8/30/93 15:01:31";
/*
 *   COMPONENT_NAME: LIBMETH
 *
 *   FUNCTIONS: Handlenumbers
 *		Swapall
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include "ics_def.h"
#include "bdstruct.h"

void Handlenumbers ( char *Buffer,
                     int  Begin,
                     int  End,
                     int  Num_Flag)
{
  unsigned long i;                 /* loop counter */
  void extern to_hindi();
  void extern arab_num();
  int  extern getgroup_A1046();

  if (Num_Flag == NUMERALS_NATIONAL)              /* convert to Hindi */
   {
     for (i=Begin;i<=End;i++)
        if (getgroup_A1046(Buffer[i])==DIGIT)
                    to_hindi(&(Buffer[i]));
   }
  else                             /* convert to Arabic */
   {
     for (i=Begin;i<=End;i++)
        if (getgroup_A1046(Buffer[i])==DIGIT)
                    arab_num(&(Buffer[i]));
   }
}

int  (*Swaps) ();

void Swapall( char *Buffer,
              char *codepage,
              int  Begin,
              int  End)

{
  unsigned long i;                 /* loop counter */
  int  extern Swap1046();
  int  extern Swap856();
  int  extern Swap8859_8();

      if ((strcmp(codepage,"IBM-856")==0)
      ||  (strcmp(codepage,"IBM-862")==0))
         Swaps = Swap856;

      else if (strcmp(codepage,"ISO8859-8")==0)
         Swaps = Swap8859_8;

      else   /* default */ 
         Swaps = Swap1046;

     for (i=Begin;i<=End;i++)
          Buffer[i] = Swaps(Buffer[i]);
}
/***************************************************************************/
