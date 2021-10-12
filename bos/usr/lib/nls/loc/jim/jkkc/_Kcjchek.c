static char sccsid[] = "@(#)91	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcjchek.c, libKJI, bos411, 9428A410j 6/4/91 12:53:00";
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
 * MODULE NAME:       _Kcjchek
 *
 * DESCRIPTIVE NAME:  JUDGE AS OBJECT OF LOOKING UP A WORD IN DICTIONARY
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x00ff (Z_NOTLUP): not proceed to look up in dictionary
 *                    0x01ff (Z_LUP):    proceed to look up in dictionary
 *                    0x7fff(UERROR):    unpredictable error
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
struct RETJCHEK _Kcjchek(z_kcbptr,z_dctctl,z_mode)

struct KCB      *z_kcbptr;              /* get address of KCB           */
short  z_dctctl;                        /* dict. looking up cntrl info. */
short  z_mode;                          /* dict. looking up mode        */
{ 
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcmce.h"
#include   "_Kcyce.h"
#include   "_Kcgpw.h"
 
/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define Z_NOTLUP 0x00ff                 /* don't proceed to dict lookup */
#define Z_LUP    0x01ff                 /* do dictionary look up        */

#define Z_INIT   1                      /* initial conversion           */
#define Z_NEXT   3                      /* next conversion              */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short      z_i;                      /* define loop counter          */
   short      z_endp;                   /* yomi end position            */
   struct  RETJCHEK   z_rc;             /* define return code area      */

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base address of KCB      */ 
   gpwptr1 = kcb.gpwgpe;                /* set base address of GPW      */ 

/*----------------------------------------------------------------------*
 *        JUDGE AS OBJECT OF LOOKING UP A WORD IN DICTIONARY
 *----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*
 *        RESET TANBUNSETSU FLAG   
 *----------------------------------------------------------------------*/
   /*-------------------------------------------------------------------*
    *   When initial conversion is occurred, 
    *   tbflag(tanbunsetsu flag) is "OFF" 
    *        if the length of MCEs is longer than 6
    *   ( The initializing module (_Kcxinia) sets tbflag "ON" )
    *-------------------------------------------------------------------*/
   if ( ( kcb.mchacmce > 6 ) && ( z_mode == ORD ) )
      gpw.tbflag = OFF;

   /*-------------------------------------------------------------------*
    *   When initial conversion(recutting)
    *-------------------------------------------------------------------*/
   else if( ( kcb.ymill2 < kcb.ymill1 ) && ( kcb.ymill2 != 0 ) )
      gpw.tbflag = OFF;

/*----------------------------------------------------------------------*
 *      SET JDOK
 *----------------------------------------------------------------------*/
   switch( z_mode )                     /* branch off by mode           */
   {
      /*---------------   ORDINARY CONVERSION   ------------------------*/
      case ORD:                         /* when initial conversion      */
         if (kcb.cnvx == KAKNO)         /* set end of dict. looking up  */
            z_rc.endmora = kcb.mchacmce- 2; /* 07/21/87                 */

         else
            z_rc.endmora = kcb.mchacmce- 1;

         switch(z_dctctl)               /* branch off by dict. control  */
         {
            case 0:                     /* if dict. control = 0 &       */
            case 1:                     /* if dict. control = 1 &       */
            case 2:                     /* if dict. control = 2 &       */
            case 3:                     /* if dict. control = 3 &       */
            case 4:                     /* if dict. control = 4         */
               z_rc.rc = Z_NOTLUP;      /* set ret. code of not proceed */
               return(z_rc);            /* return to the caller         */

            case 5:                     /* if dict. control = 5 &       */
            case 6:                     /* if dict. control = 6         */
               mceptr1=kcb.mchmce;      /* set first mora code pointer  */
               if (mce.jdok == JD_YET)  /* if jdok is 0 then            */
                  mce.jdok = JD_READY;  /* set 1 to jdok                */
               z_rc.rc = Z_LUP;         /* set ret. code of proceed     */
               return(z_rc);            /* return to the caller         */

            case 7:                     /* if dict. control = 7 &       */
            case 8:                     /* if dict. control = 8         */
               mceptr1=kcb.mchmce;      /* set first mora code pointer  */
               for(z_i=0;z_i<3;z_i++,mceptr1++)
                                        /* loop to third mora           */
                  if (mce.jdok==JD_YET) /* if jdok is 0 then            */
                     mce.jdok = JD_READY;/* set 1 to jdok               */
               z_rc.rc = Z_LUP;         /* set ret. code of proceed     */
               return(z_rc);            /* return to the caller         */

            case 9:                     /* if dict. control = 9         */
               mceptr1=kcb.mchmce;      /* set first mora code pointer  */
               for(z_i=0;z_i<5;z_i++,mceptr1++)
                                        /* loop to 5th mora             */
                  if (mce.jdok==JD_YET) /* if jdok is 0 then            */
                     mce.jdok = JD_READY;/* set 1 to jdok               */
               z_rc.rc = Z_LUP;         /* set ret. code of proceed     */
               return(z_rc);            /* return to the caller         */

            default:                    /* if dict. control >=10        */
               mceptr1=kcb.mchmce;      /* set first mora code pointer  */
               for(z_i=0;z_i<z_dctctl-4;z_i++,mceptr1++)
                                        /* loop to dict.control - 5     */
                  if (mce.jdok==JD_YET) /* if jdok is 0 then            */
                     mce.jdok = JD_READY;/* set 1 to jdok               */
               z_rc.rc = Z_LUP;         /* set ret. code of proceed     */
               return(z_rc);            /* return to the caller         */
         }

      /*---------------   ABSOLUTE CONVERSION   ------------------------*/
      case ABS:                         /* if absolute conversion       */
         z_rc.endmora = kcb.mchacmce-1; /* set mora end                 */
         if (gpw.tbflag == ON)          /* if tanbunsetsu = ON          */
         {
            mceptr1=kcb.mchmce;         /* set first mora code pointer  */
            if (mce.jdok == JD_YET)     /* if jdok is 0 then            */
               mce.jdok = JD_READY;     /* set 1 to jdok                */

            z_rc.rc = Z_LUP;            /* set ret. code of proceed     */
            return(z_rc);               /* return to the caller         */
         }
         else
         {
            mceptr1=kcb.mchmce;         /* set first mora code pointer  */
            for(z_i=0;z_i<=z_dctctl;z_i++,mceptr1++)
                                        /* regard all mora as the target*/
               if (mce.jdok == JD_YET)  /* if jdok is 0 then            */
                  mce.jdok = JD_READY;  /* set 1 to jdok                */
            z_rc.rc = Z_LUP;            /* set ret. code of proceed     */
            return(z_rc);               /* return to the caller         */
         }

      /*-----------------   MODE IS INVALID    -------------------------*/
      default:                          /* if illigal mode              */
         z_rc.rc = UERROR;              /* set  UERROR return code      */
         return(z_rc);                  /* return to the caller         */
   }
}
