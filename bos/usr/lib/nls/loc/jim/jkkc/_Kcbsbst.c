static char sccsid[] = "@(#)22	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcbsbst.c, libKJI, bos411, 9428A410j 6/4/91 10:15:08";
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
 * MODULE NAME:       _Kcbsbst
 *
 * DESCRIPTIVE NAME:  SETTING DATA ON BTE
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS)    : Success
 *                    0x1704(BTEOVER)    : BTE overflow
 *                    0x7FFF(UERROR )    : Fatal error
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------*  
 *      INCLUDE FILES                                                     
 *----------------------------------------------------------------------*/ 
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"                   /* Define Constant file         */
#include   "_Kcrcb.h"                   /* Define Return Code Structure */

/************************************************************************
 *      START OF FUNCTION                                                 
 ************************************************************************/ 
struct RETBSBST _Kcbsbst(z_kcbptr,z_hinl,z_hinr,z_morpos,z_morend,
                         z_jteptr,z_pen,z_fteptr)
struct  KCB *z_kcbptr;
uschar       z_hinl;
uschar       z_hinr;
short        z_morpos;
short        z_morend;
struct  JTE *z_jteptr;
uschar       z_pen;
struct  FTE *z_fteptr;
{
/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern struct RETCGET  _Kccget();    /* Get  Any Kinds of Table Entry*/
   extern short           _Kccswpb();   /* Deleta Disused BTE           */

/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcbte.h"   /* Bunsetsu Table Entry (BTE)                   */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   struct RETCGET  z_rcget ;    /* Define Area for Return of _Kccget    */
   short           z_rcswpb;    /* Define Area for Return of _Kccswpb   */

   struct RETBSBST z_rtcode;    /* Define Area for Return of own        */

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;                  /* establish address'ty to KCB  */

/*----------------------------------------------------------------------*  
 *      GET NEW BTE
 *----------------------------------------------------------------------*/ 
   z_rcget = _Kccget(&kcb.bthchh);      /* obtain a BTE entry           */
   if ( z_rcget.rc == GET_EMPTY )
   {
      z_rcswpb = _Kccswpb(kcbptr1,z_morend); /* delete unused BTE    */

      z_rcget = _Kccget(&kcb.bthchh); /* RETRY                       */
      if ( z_rcget.rc == GET_EMPTY )
      {
         z_rtcode.rc = BTEOVER;
         return(z_rtcode);
      }
      else if ( ( z_rcget.rc != GET_TOP_MID ) && ( z_rcget.rc != GET_LAST ) )
      {
         z_rtcode.rc = UERROR;
         return(z_rtcode);
      }
   }
   else if ( ( z_rcget.rc != GET_TOP_MID ) && ( z_rcget.rc != GET_LAST ) )
   {
      z_rtcode.rc = UERROR;
      return(z_rtcode);
   }

   bteptr1 =(struct BTE *)z_rcget.cheptr;/* establish addres'ty to BTE  */

   z_rtcode.bteptr =(struct BTE *)z_rcget.cheptr;
                                        /* save BTE ptr in return code  */

                                        /* call _Kcbhyki to move FKX inf*/
                                        /* to BTE                       */
   _Kcbhyki(z_kcbptr,bteptr1,z_fteptr); /* set fzkgo hiyoki             */

   bte.hinl    = z_hinl;                /* left hinshi                  */
   bte.hinr    = z_hinr;                /* right hinshi                 */
   bte.stap    = z_morpos;              /* mora position                */
   bte.endp    = z_morend;              /* mora end                     */
   bte.jteaddr = z_jteptr;              /* JTE ptr                      */
   bte.pen     = z_pen;                 /* penalty                      */
   bte.dmyflg  = OFF;                   /* set dummy flag OFF           */
   bte.usage   = 0;                     /* clear usage count            */
   z_rtcode.rc = SUCCESS;               /* return to caller             */

   return(z_rtcode);
}
