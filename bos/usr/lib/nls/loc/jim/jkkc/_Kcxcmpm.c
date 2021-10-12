static char sccsid[] = "@(#)03	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcxcmpm.c, libKJI, bos411, 9428A410j 6/4/91 15:28:32";
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
 * MODULE NAME:       _Kcxcmpm
 *
 * DESCRIPTIVE NAME:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:        1 (EQ_SHORT): equal but string1 is equal or shorter
 *                     2 (LONG):     equal but string1 is longer
 *                    -1 (NOT_EQU):  not equal
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
short _Kcxcmpm(z_kcbptr,z_str1,z_len1,z_str2,z_len2)

struct KCB      *z_kcbptr;              /* pointer of KCB               */
unsigned char   *z_str1;                /* pointer of string1           */
short           z_len1;                 /* length of string1            */
short           z_str2;                 /* offset of MCE                */
unsigned char   z_len2;                 /* length of MCEs               */
{
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcmce.h"

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short        z_rc;                   /* rerative record number       */
   uschar       *z_ptr1;                /* pointer of string1           */
                                        /*   which moves in buffer      */
   struct MCE   *z_ptr2;                /* pointer of string2           */
                                        /*   which moves in buffer      */
/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base pointer of KCB      */

   z_ptr1 = z_str1;                     /* set pointer of string1       */
   z_ptr2 = kcb.mchmce + z_str2;        /* set pointer of string2       */
   
/*----------------------------------------------------------------------*
 *       COMPARE STRING1 WITH STRING2  
 *----------------------------------------------------------------------*/
   while ( 1 )
   {
      if ( z_ptr1 >= z_str1 + z_len1 )
                                        /* if end of strinf1            */
      {
         z_rc = EQ_SHORT;
         break;
      }
                                        /* if end of string2            */
      else if ( z_ptr2 >= kcb.mchmce + z_str2 + z_len2 )
      {
         z_rc = LONG;
         break;
      }
                                        
      else
      {
         mceptr1 = z_ptr2;
         if ( *z_ptr1 != mce.code )/* if str1 is not equal to str2      */
         {
            z_rc = NOT_EQU;
            break;
         }

         else                           /* if str1 is equal to str2     */
         {
            z_ptr1++;
            z_ptr2++;
         }
      }
   }

/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
   return(z_rc);
}
