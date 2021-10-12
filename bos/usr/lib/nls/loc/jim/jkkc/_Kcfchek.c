static char sccsid[] = "@(#)57	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcfchek.c, libKJI, bos411, 9428A410j 6/4/91 10:26:28";
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
 * MODULE NAME:       _Kcfchek
 *
 * DESCRIPTIVE NAME:  JUDGE AS OBJECT OF LOOKING UP A FUZOKU-GO DICT.
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       FUZOKUGO ANALISYS POSITION
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
short           _Kcfchek(z_kcbptr,z_dctctl,z_mode)
struct KCB      *z_kcbptr;              /* get address of KCB           */
short  z_dctctl;                        /* dict. looking up cntrl info. */
short  z_mode;                          /* dict. looking up mode        */
                                        /* NORMAL or ABSOLUTE           */
{ 
/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcgpw.h"   /* General Purpose Workarea (GPW)               */
 
/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
/*----------------------------------------------------------------------*  
 *        JUDGE AS OBJECT OF LOOKING UP A WORD IN DICTIONARY               
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;                  /* set base address of KCB      */ 
   gpwptr1 = kcb.gpwgpe;                /* set base address of GPW      */ 

   switch(z_mode)                       /* branch off by mode           */
   {
      case ABS:                         /* when initial conversion      */
         switch(gpw.tbflag)             /* branch off by tanbunsetsu flg*/
         {
            case ON:                    /* if tanbunsetsu mode          */
               if(z_dctctl == kcb.mchacmce -1)
                 return(kcb.mchacmce);  /* return with number of mora   */

               else
                  return(0);            /* return with fatal            */
            default:                    /* else                         */
               return(z_dctctl+1);      /* return with fatal            */

         }
      default:
         switch(z_dctctl)               /* branch off by dict. control  */
         {
            case 0:                     /* if dict. control = 0 &       */
            case 1:                     /* if dict. control = 1 &       */
            case 2:                     /* if dict. control = 2 &       */
            case 3:                     /* if dict. control = 3 &       */
            case 4:                     /* if dict. control = 4 &       */
               return(0);
            case 5:                     /* if dict. control = 5 &       */
            case 6:                     /* if dict. control = 6 &       */
               return(0);
            case 7:                     /* if dict. control = 7 &       */
            case 8:                     /* if dict. control = 8 &       */
               return(2);
            case 9:                     /* if dict. control = 9 &       */
               return(4);
            default:                    /* else                         */
               return(z_dctctl-4);      /* return with number of dict-3 */
         }
   }
}
