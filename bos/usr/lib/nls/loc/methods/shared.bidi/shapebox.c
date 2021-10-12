static char sccsid[] = "@(#)50	1.1  src/bos/usr/lib/nls/loc/methods/shared.bidi/shapebox.c, cfgnls, bos411, 9428A410j 8/30/93 15:04:05";
/*
 *   COMPONENT_NAME: LIBMETH
 *
 *   FUNCTIONS: BidiShapeBoxChars
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
#include <sys/lc_layout.h>
#include <sys/types.h>
#include "bdstruct.h"

#define SWAP_SPEC 	6

static char swap_spec[SWAP_SPEC][2] = {
                        {'l','k'},
                        {'m','j'},
                        {'t','u'},
                        {'ê','í'},
                        {'ì','ë'},
                        {'ô','õ'},
                        };



int BidiShapeBoxChars(BidiValuesRec Values,const char *InpBuf,size_t InpSize,
                      char *OutBuf) 
{
    int i,j;

    if ((Values->Swap.in != Values->Swap.out) && 
	 (Values->Orient.in == ORIENTATION_RTL))
    	 for (i=0;i<InpSize;i++)
             for (j=0; j< SWAP_SPEC; j++)
                  if (InpBuf[i] == swap_spec [j][0])
                      OutBuf[i] = swap_spec [j][1];
                  else if (InpBuf[i]== swap_spec [j][1])
                      OutBuf[i] = swap_spec [j][0];
    return(0);
}

