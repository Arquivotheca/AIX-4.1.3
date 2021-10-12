static char sccsid[] = "@(#)11	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcbbpen.c, libKJI, bos411, 9428A410j 6/4/91 10:10:40";
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
 * MODULE NAME:       _Kcbbpen
 *
 * DESCRIPTIVE NAME:
 *
 * FUNCTION:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       Penalty
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------*  
 *      INCLUDE FILES                                                     
 *----------------------------------------------------------------------*/ 
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"                   /* Define Constant file         */
#include   "_Kcbpn.t"                   /* Penalty Between BTEs         */

/************************************************************************
 *      START OF FUNCTION                                                 
 ************************************************************************/ 
short _Kcbbpen(z_kcbptr,z_bteptr1,z_bteptr2)

struct KCB *z_kcbptr;                   /* hinr of pre bunsetu          */
struct BTE *z_bteptr1;                  /* hinr of pre bunsetu          */
struct BTE *z_bteptr2;                  /* hinl of next bunsetu         */
{
/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcbte.h"   /* Bunsetsu Table Entry (BTE)                   */
#include   "_Kcjtx.h"   /* Jiritsugo Tag eXchange table (JTX)           */
#include   "_Kcgpw.h"   /* General Purpose Workarea (GPW)               */
#include   "_Kcjkj.h"   /* Jiritsugo KanJi hyoki table entry (JKJ)      */
#include   "_Kcjte.h"   /* Jiritsugo Table Entry (JTE)                  */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/ 
#define   Z_SAHEN    0x08
#define   Z_PRO_MSK  (PRONOUN_SEI|PRONOUN_MEI|PRO_CHIMEI|PRO_CHIMEIXX)
                                        /* pronoun mask (0x78)          */
#define   Z_ELS_MSK  0x7F

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   short   z_pen;                       /* penalty save area            */
   short   z_pentmp;                    /* penalty tempolary area       */
   uschar  z_pdflg0;                    /* previous dflag[0]            */
   uschar  z_pdflg1;                    /* previous dflag[1]            */
   uschar  z_psahen;                    /* previous sahen flag          */
   uschar  z_ndflg0;                    /* next     dflag[0]            */
   uschar  z_ndflg1;                    /* next     dflag[1]            */
   uschar  z_nsahen;                    /* next     sahen flag          */
   uschar  z_phin;                      /* previous hinshi              */
   uschar  z_nhin;                      /* next     hinshi              */

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;
   gpwptr1 = kcb.gpwgpe;
   bteptr1 = z_bteptr1;
   bteptr2 = z_bteptr2;
   if(bteptr1 != NULL)                  /* pre. BTE is exist.           */
   {
      jteptr1 = bte.jteaddr;            /* set pre. JTE address         */
      z_phin  = bte.hinr;
      if(jteptr1 != NULL)               /* if FUZOKUGO bunsetsu         */
      {
         z_pdflg0 = jte.dflag[0];       /* pre. dflag                   */
         z_pdflg1 = jte.dflag[1];       /* pre. dflag                   */
         jtxptr1 = kcb.jtxjtx + jte.hinpos; /* get sahen flag           */
         z_psahen = (jtx.pos[1] & Z_SAHEN) >> 3;
      }
      else
      {
         z_pdflg0 = z_pdflg1 = z_psahen = NULL;
      }
   }
   else                                 /* if pre. BTE does not exist.  */
   {
         jteptr1 = NULL;                /* set pre. JTE address         */
         z_phin = gpw.pthpphin;
         z_pdflg0 = gpw.ppdflag[0];
         z_pdflg1 = gpw.ppdflag[1];
         z_psahen = gpw.ppsahen;
   }

   jteptr2 = bte2.jteaddr;              /* set back JTE address         */
   z_nhin  = bte2.hinl;
   if(jteptr2 != NULL)                  /* if back JTE does not exist   */
   {
      z_ndflg0 = jte2.dflag[0];         /* back dflag                   */
      z_ndflg1 = jte2.dflag[1];         /* back dflag                   */
      jtxptr1 = kcb.jtxjtx + jte2.hinpos;
      z_nsahen = (jtx.pos[1] & Z_SAHEN) >> 3;/* get sahen flag          */
   }
   else
   {
      z_ndflg0 = z_ndflg1 = z_nsahen = NULL;
   }

   if (z_phin == NULL)
      return( NULL );

   z_pentmp = bb_pen[ z_phin ][ z_nhin ];
   
   z_pen = 0;
   switch ( z_pentmp )
   {
      case CW:                    /* goki-goki                    */
         if ( (z_pdflg1 & COMPOUND_NOT) != NULL )
            z_pen +=  5;
         if ( (z_pdflg1 & SUC_GOKI_REQ) != NULL )
            z_pen -=  1;
         if ( (z_ndflg1 & COMPOUND_NOT) != NULL )
            z_pen +=  5;
         if ( (z_ndflg1 & PRE_GOKI_REQ) != NULL )
            z_pen -= 10;
         break;

      case PG:                    /* setto-goki                   */
         if (((z_pdflg1 & SAHEN_REQ   ) != NULL )
           && (z_nsahen == 1) )
            z_pen -=  3;

         if (((z_pdflg1 & MEISHI_REQ  ) != NULL )
           && (z_nsahen == NULL) )
            z_pen -=  3;

         if ( ((z_ndflg0 & SETTO_OK    ) == NULL )
           && ((z_pdflg1 & ALMIGHTY    ) == NULL ))
         {
            z_pen += 10;
         }

         if ( (z_ndflg1 & PRE_GOKI_REQ) != NULL )
            z_pen -= 10;
         break;

      case GS:                    /* goki-setsubi                 */
         z_pen = -10;
         if (((z_ndflg1 & SAHEN_REQ   ) != NULL )
           && (z_psahen == 1) )
            z_pen -=  3;

         if (((z_ndflg1 & MEISHI_REQ  ) != NULL )
           && (z_psahen == NULL) )
            z_pen -=  3;

         if (((z_pdflg0 & SETSUBI_OK  ) == NULL )
           && ((z_ndflg1 & ALMIGHTY    ) == NULL ))
         {
            z_pen += 10;
         }

         break;

      case OY:                    /* o-goki, o-yougen             */
         z_pen = 10;
         if ( (z_ndflg0 & O_OK        ) != NULL )
            z_pen -= 10;
         break;

      case GY:                    /* go-goki, go-yougen           */
         z_pen = 10;
         if ( (z_ndflg0 & GO_OK       ) != NULL )
            z_pen -= 10;
         break;

      case GP:                    /* goki-setto                   */
         if ( (z_pdflg1 & COMPOUND_NOT) != NULL )
            z_pen +=  5;
         if ( (z_ndflg1 & INTERNAL_NOT) != NULL )
            z_pen +=  1;
         break;

      case GN:                    /* noun-nsetsubi                */
         if ( ( z_pdflg1 & NUMBER ) != NULL )
            z_pen = -13;
         else
            z_pen = 0;
         break;

      case SG:                    /* setsubi-goki                 */
         if ( (z_pdflg1 & INTERNAL_NOT) != NULL )
            z_pen +=  1;
         if ( (z_ndflg1 & COMPOUND_NOT) != NULL )
            z_pen +=  5;
         break;

      case PP:                    /* setto-setto                  */
         z_pen = 10;
         if ( (z_ndflg1 & DUP_OK      ) != NULL )
            z_pen -=  5;
         break;

      case SS:                    /* setsubi-setsubi              */
         if ( (z_pdflg1 & DUP_OK      ) != NULL )
            z_pen -=  5;
         break;

      case SP:                    /* setsubi-setto                */
         if ( ( (z_pdflg1 | z_ndflg1) & INTERNAL_NOT) != NULL )
            z_pen +=  1;
         break;

      case NPN:                   /* suushisetto-noun             */
         if ( ( z_ndflg1 & NUMBER ) != NULL )
            z_pen = -5;
         else
            z_pen = 10;
         break;

      case NPG:                   /* suushisetto-suuchi           */
         break;

      case NGS:                   /* suuchi-josushi               */
         z_pen = -12;
         break;

      case NPS:                   /* suushisetto-josushi          */
         z_pen = 10;
         if ( (z_pdflg0 & KURAIDORI   ) != NULL )
            z_pen -= 20;
         break;

      case NSS:                   /* josushi-josushi              */
         if ((jte.dtype == TP_JOSUSHI)&&
             ((z_pdflg0 & KURAIDORI   ) != NULL ))
            z_pen -= 10;
         break;

      case KPG:                   /* pro_setto-pronoun            */
         if (((z_pdflg0 & z_ndflg0 & Z_PRO_MSK) == NULL)
           && ((z_pdflg1 & z_ndflg1 & Z_ELS_MSK) == NULL))
            z_pen += 10;
         break;

      case KGS:                   /* pronoun-pro_setsubi          */
         z_pen = -10;
         if (((z_pdflg0 & z_ndflg0 & Z_PRO_MSK) == NULL)
           && ((z_pdflg1 & z_ndflg1 & Z_ELS_MSK) == NULL))
            z_pen += 10;
         break;

      case KCW:                   /* pronoun-pronoun              */
         if ( ( ( z_pdflg0 & PRONOUN_SEI ) != NULL )
           && ( ( z_ndflg0 & PRONOUN_MEI ) != NULL ) )
            z_pen -= 7;

         /* IN  following cases add bounus                        */
         /*  CHIMEI   CHIMEI                                      */
         /*  CHIMEI-X CHIMEI-X                                    */
         /*  CHIMEI-X CHIMEI                                      */
         if(((z_pdflg0 & PRO_CHIMEIXX) &&
             (z_ndflg0 & ( PRO_CHIMEI | PRO_CHIMEIXX )))||
            ((z_pdflg0 & PRO_CHIMEI)&&(z_ndflg0 & PRO_CHIMEI)))
            z_pen -= 4;
         break;

      default:
         z_pen = z_pentmp;
         break;
   }

/*----------------------------------------------------------------------*
 *      ADD PENALTY BETWEEN 1 KANJI AND 1 KANJI
 *----------------------------------------------------------------------*/
   if((jteptr1 != NULL)&&(jteptr2 != NULL ))
   {
      jkjptr1 = jte.jkjaddr;
      jkjptr2 = jte2.jkjaddr;
                                        /* if Kanji exists              */
      if((jkjptr1 != NULL)&&(jkjptr2 != NULL ))
      {                                 /* The Not Kana Hyoki           */
         if((jkj.kj[0] != 0x7F)&&(jkj2.kj[0] != 0x7F))
         {                              /* Kanji is alone               */
           if((((jte.dflag[0]  & FREQ) == 0x6)&&
               ((jkj.kj[0] &  0x80)==NULL))||
              (((jte2.dflag[0] & FREQ) == 0x6)&&
               ((jkj2.kj[0] & 0x80)==NULL)))
           {
              z_pen += 2;
           }                             /* Bunsetsu consist only kanji  */
            if(((jkj.kj[0] & 0x80)&(jkj2.kj[0] & 0x80))==NULL)
            {                           /* MRU word                     */
               if((((bte.endp -bte.stap )+1)==jte.len)&&
                  (((bte2.endp-bte2.stap)+1)==jte2.len))
               {                        /* MRU word                     */
                  if(((jte.dflag[0]  & FREQ) == 0x6)||
                     ((jte2.dflag[0] & FREQ) == 0x6))
                     z_pen += 1;
               }
            }
         }
      }
   }
   return( z_pen );

}
