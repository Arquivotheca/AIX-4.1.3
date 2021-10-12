static char sccsid[] = "@(#)83	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcicmpt.c, libKJI, bos411, 9428A410j 7/23/92 03:08:59";
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
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kcicmpt
 *
 * DESCRIPTIVE NAME:  CONVERSION TO MORA WITH DAKU-ON, HANDAKU-ON & YO-ON
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x00:   invalid
 *                    others: mora code
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
unsigned char  _Kcicmpt( z_incode, z_yomi )

unsigned char  z_incode;                /* set change mora code         */
unsigned char  z_yomi;                  /* set daku-ten or handaku-ten  */
{
/*----------------------------------------------------------------------* 
 *      DEFINE LOCAL CONSTANTS                                                
 *----------------------------------------------------------------------*/
#define   Z_INVALID   0x00

/*----------------------------------------------------------------------* 
 *      DEFINE LOCAL VARIABLES                                                
 *----------------------------------------------------------------------*/
   uschar   z_rtcode;

/*----------------------------------------------------------------------*
 *      MORA CODE 
 *----------------------------------------------------------------------*/
   switch( z_yomi )
   { 
      case Y_DKT: 
         switch( z_incode ) 
         {
            case M_KA :
            case M_KI :
            case M_KU :
            case M_KE :
            case M_KO :
            case M_SA :
            case M_SI :
            case M_SU :
            case M_SE :
            case M_SO :
            case M_TA :
            case M_TE : 
            case M_TO :
                        return( z_incode + 0x01 );
            case M_CHI : 
                        return( z_incode - 0x09 );
            case M_TSU :
                        return( z_incode - 0x0A );
            case M_KYA: 
            case M_KYU:
            case M_KYO: 
            case M_SYA:
            case M_SYU:
            case M_SYO:
                        return( z_incode + 0x03 );
            case M_HA :
            case M_HI :
            case M_FU :
            case M_HE :
            case M_HO :
                        return( z_incode + 0x01 );

            case M_HYA: 
            case M_HYU:
            case M_HYO:
                        return( z_incode + 0x03 );
            case M_U  :
                        return( M_VU );

            default   : return( Z_INVALID );
         }

      case Y_HDK:
         switch( z_incode )
         {
            case M_HA :
            case M_HI :
            case M_FU :
            case M_HE :
            case M_HO :
                        return( z_incode + 0x02 );

            case M_HYA: 
            case M_HYU: 
            case M_HYO:
                        return( z_incode + 0x06 );

            default   : return( Z_INVALID );
         }

      case Y_XYA :
      case Y_XYU :
      case Y_XYO :
         switch( z_incode )
         {
            case M_KI  : z_rtcode = M_KYA; break;
            case M_GI  : z_rtcode = M_GYA; break;
            case M_SI  : z_rtcode = M_SYA; break;
            case M_JI  : z_rtcode = M_JA;  break;
            case M_CHI : z_rtcode = M_CYA; break;
            case M_NI  : z_rtcode = M_NYA; break;
            case M_HI  : z_rtcode = M_HYA; break;
            case M_PI  : z_rtcode = M_PYA; break;
            case M_BI  : z_rtcode = M_BYA; break;
            case M_MI  : z_rtcode = M_MYA; break;
            case M_RI  : z_rtcode = M_RYA; break;
            default    : return( Z_INVALID );
         }

         switch( z_yomi )               /* check the kind of you-on     */
         {
            case Y_XYA : return( z_rtcode );
            case Y_XYU : return( z_rtcode + 0x01 );
            case Y_XYO : return( z_rtcode + 0x02 );
            default    : return( Z_INVALID );
         }

      default   : return( Z_INVALID );
   }
}
