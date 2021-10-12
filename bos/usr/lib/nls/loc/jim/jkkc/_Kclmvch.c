static char sccsid[] = "@(#)42	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kclmvch.c, libKJI, bos411, 9428A410j 6/4/91 15:12:57";
/*
 * COMPONENT_NAME: (libKJI) Japanese Input Method (JIM)
 * 
 * FUNCTIONS: Kana-Kanji-Conversion (KKC) Library
 *
 * ORIGINS: 27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when 
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kclmvch
 *
 * DESCRIPTIVE NAME:  MoVe CHaracter in a dimemsion.
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       VOID
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------*  
 *      INCLUDE FILES                                                     
 *----------------------------------------------------------------------*/ 
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"                   /* Define Constant file         */

/************************************************************************
 *      START OF FUNCTION                                                 
 ************************************************************************/ 
short _Kclmvch(str, pos, len, dir, dist, clear, clrstr, clrpos, clrlen)
char *str;              /* Pointer to character dimension.              */
int   pos;              /* Start position.                              */
int   len;              /* Character length you want move.              */
int   dir;              /* Move Direction. M_FORWD or M_BACKWD          */
int   dist;             /* Move Distance.                               */
int   clear;            /* Crear flag (You want clear remaining data)   */
                        /* specify TRUE or FALSE.                       */
char *clrstr;           /* Pointer to string contain clear data.        */
int   clrpos;           /* Clear data start position in clrstr.         */
int   clrlen;           /* Clear data length in clrstr(not in str).     */
{
/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   register char   *org;                /* Oreginal character address.  */
   register char   *dest;               /* Destination address.         */
   char            *stop;               /* Stop address.                */
   char            *clrstart;           /* clear data start address.    */
   int             times;               /* Clear loop counter.          */
   int             rem;                 /* remainder of Clear loop.     */
   char            *memcpy();           /* memory copy function         */

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
/*----------------------------------------------------------------------*  
 *      2. Branch by parameter -dir-.
 *----------------------------------------------------------------------*/ 
   if (dir == U_FORWD)                  /* Move foreawd                 */
   {
      org  = str + pos;         /* Calculate sorce start address        */
      stop = org + len;         /* Calculate move stop address          */
      dest = org - dist;        /* Calculate destination start address  */
      while(org <= stop)
      {
         *dest++ = *org++;              /* Copying data                 */
      }
      if (clear)
      {
         times    = dist / clrlen;      /* memcpy() Execution times     */

         rem      = dist % clrlen;      /* remainder after memcpy loop  */

         dest     = str + pos + len - dist;
                                        /* destination address.         */
         clrstart = clrstr + clrpos;
                                        /* Calculate sorce start address*/
         while (times > 0) {
            memcpy(dest, clrstart, clrlen);/* copying data              */
            dest += clrlen;             /* New destination address      */
            times--;                    /* Loop counter decrement       */
         }
         memcpy(dest, clrstart, rem);   /* fill in remaining area       */
      }
   }
   else                                 /* Move backward                */
   {
      org  = str + pos + len - 1;       /* Calculate sorce start address*/

      stop = str + pos;                 /* Calculate move stop address  */

      dest = org + dist;          /* Calculate destination start address*/

      while(org >= stop)                /* copying data                 */
      {
         *dest-- = *org--;
      }
      if (clear)
      {
         times    = dist / clrlen;      /* memcpy() Execution times     */

         rem      = dist % clrlen;      /* remainder after memcpy loop  */

         dest     = str + pos;          /* destination address.         */

         clrstart = clrstr + clrpos;    /* calculate sorce start address*/

         while (times > 0)
         {
            memcpy(dest, clrstart, clrlen);/* fill in specified data   */

            dest += clrlen;             /* New destination address  */
            times--;
         }
         memcpy(dest, clrstart, rem);   /* fill remaining area */
      }
   }
   return;
}
