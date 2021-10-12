static char sccsid[] = "@(#)53	1.2  src/bos/usr/lib/nls/loc/methods/shared.bidi/wcstransform.c, cfgnls, bos411, 9428A410j 9/10/93 11:11:01";
/*
 *   COMPONENT_NAME: LIBMETH
 *
 *   FUNCTIONS: wcsBidiTransform
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
/***************************************************************************/
int wcsBidiTransform (LayoutObject plh, const wchar_t *InpBuf,
                   size_t *InpSize,
                   void *OutBuf,
                   size_t *OutSize,
                   size_t *ToOutBuf,
                   size_t *ToInpBuf,
                   unsigned char *BidiLvl)
                   
{

   char *mbsin;
   char *mbsout;
   int  mlenin;
   int  mlenout;
   int RC;


   mbsin = malloc(*InpSize * MB_CUR_MAX);  /* enough memory */
   mlenin = *InpSize * MB_CUR_MAX;
   wcstombs(mbsin, InpBuf, mlenin);

   mbsout = malloc(*OutSize * MB_CUR_MAX); 
   mlenout = *OutSize * MB_CUR_MAX;

   /* send multi-byte string */
   RC = BidiTransform(mbsin,&mlenin,
                      mbsout,&mlenout,
                      ToOutBuf, ToInpBuf,
                      BidiLvl, plh);

   *OutSize=mbstowcs(OutBuf, mbsout, mlenout);
   return(RC);
}



