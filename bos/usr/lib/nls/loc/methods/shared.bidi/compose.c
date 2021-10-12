static char sccsid[] = "@(#)01	1.1  src/bos/usr/lib/nls/loc/methods/shared.bidi/compose.c, libmeth, bos411, 9428A410j 9/10/93 11:19:14";
/*
 *   COMPONENT_NAME: LIBMETH
 *
 *   FUNCTIONS: BidiCompose
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
#include <sys/types.h>
#include <sys/lc_layout.h>
#include "compose.h"

int BidiCompose(int ShapingOrient,char *buffer,size_t length)
{
 int i;
 char temp;

    if (ShapingOrient==ORIENTATION_RTL)
    {
           /* the consonants and vowels are in correct order, 
              so just make sure all vowels are not connected */
       for (i=0;i<length;i++)
         if (buffer[i]>=128)   /* if not Latin char */
            if (Vowels[buffer[i]-128]) /* if it is a vowel */
              buffer[i]=Vowels[buffer[i]-128];  /* set it to isolated */ 
    }
    else  /* LTR --> the consonants and vowels are in reverse order */
    {
       for (i=length-1;i>=0;i--)
         if (buffer[i]>=128)        /* if not Latin char */
            if (Vowels[buffer[i]-128])  /* if it is a vowel */
              if (i<(length-1))     /* not first char */
              {
                 /* swap i and i+1 */
                 temp=Vowels[buffer[i]-128];   /* get isolated form of vowel */
                 buffer[i]=buffer[i+1];
                 buffer[i+1]=temp; 
              }         
     }
}
