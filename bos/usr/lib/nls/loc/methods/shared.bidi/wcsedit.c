static char sccsid[] = "@(#)52	1.1  src/bos/usr/lib/nls/loc/methods/shared.bidi/wcsedit.c, cfgnls, bos411, 9428A410j 8/30/93 15:04:48";
/*
 *   COMPONENT_NAME: LIBMETH
 *
 *   FUNCTIONS: wcsBidiEditShape
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
#include <stdlib.h>
#include <sys/lc_layout.h>
int  wcsBidiEditShape
               (LayoutObject plh,
                BooleanValue EditType,
                size_t *index,
                const wchar_t *InpBuf,
                size_t *InpSize,
                void *OutBuf,
                size_t *OutSize)
               
{
   char *mbsin;
   char *mbsout;
   size_t  mlenin;
   size_t  mlenout;
   int RC;


   mbsin = malloc(*InpSize * MB_CUR_MAX);  /* enough memory */
   mlenin = *InpSize * MB_CUR_MAX;
   wcstombs(mbsin, InpBuf, mlenin);

   mbsout = malloc(*OutSize * MB_CUR_MAX); 
   mlenout = *OutSize * MB_CUR_MAX;

   /* send multi-byte string */
   RC = BidiEditShape(EditType,index,
                      mbsin,&mlenin,
                      mbsout,&mlenout,
                      plh);

   *OutSize=mbstowcs(OutBuf, mbsout, mlenout);
   return(RC);
}


