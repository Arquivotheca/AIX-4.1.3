static char sccsid[] = "@(#)18	1.3  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcjuunq.c, libKJI, bos411, 9428A410j 7/23/92 03:15:44";
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
 * MODULE NAME:       _Kcjuunq
 *
 * DESCRIPTIVE NAME:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): success
 *                    0x1104 (JTEOVER): JTE table overflow
 *                    0x1204 (JKJOVER): JKJ table overflow
 *                    0x1304 (JLEOVER): JLE table overflow
 *                    0x7fff (UERROR):  unpredictable error
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
short _Kcjuunq(z_kcbptr,z_strpos,z_length)

struct KCB    *z_kcbptr;                /* pointer of KCB               */
unsigned char z_strpos;                 /* offset of 1st MCE            */
unsigned char z_length;                 /* length of string(mora code)  */

{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern short             _Kcjuget();
   extern struct RETXCMPY   _Kcxcmpy();

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcmce.h"
#include   "_Kcyce.h"

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short             z_rc;              /* return code save area        */
   short             z_rl;              /* record length                */
   short             z_yl;              /* yomi length                  */
   short             z_dl;              /* data length                  */
   short             z_i;               /* counter                      */
   short             z_j;               /* counter                      */
   short             z_mcnt;
   short             z_flag;            /* flag                         */
   short             z_offset;          /* offset of start pos in buf   */
   uschar            *z_udptr;          /* pointer moving in buffer     */
   uschar            *z_stptr;          /* pointer save area            */
   struct RETXCMPY   z_ret;             /* result of comparison         */
   struct YCE        *z_ypos;           /* pointer of YCE               */

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base pointer of KCB      */

/*----------------------------------------------------------------------*
 *      INITIALIZE LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   z_udptr = kcb.udeude + 2;            /* set pointer moving in buf    */
                                        /* set record length            */
   z_rl = (short)GETSHORT(kcb.udeude);
   mceptr2 = kcb.mchmce + z_strpos;     /* set pointer of 1st mora code */

/*----------------------------------------------------------------------*
 *      LOOK UP WORDS
 *----------------------------------------------------------------------*/
   ext_usr_conv = OFF;
   for ( z_i = 0; z_i < (z_rl - 2); z_i++ )
   {
      z_stptr = z_udptr;                /* set start pointer of entry   */
      z_yl = (short)*z_udptr++;         /* set yomi length              */
      z_dl = (short)*(z_udptr+z_yl-1);  /* set data length              */

      if ( z_yl == 0 )
         break;

      z_ret = _Kcxcmpy(z_kcbptr,z_udptr,z_yl-(short)1,z_strpos,z_length);

      /*--------------   add entries to Jiritu-go Table   --------------*/
      if ( z_ret.res == EQ_SHORT )
      {
         z_offset = z_stptr - kcb.udeude;

         z_rc = _Kcjuget(z_kcbptr,(short)JTB,z_offset+(short)z_yl,
                        z_offset+(short)z_yl+(short)z_dl-(short)1,
                        z_strpos,z_ret.len,z_offset+(short)1,z_yl-(short)1);
         if  ( z_rc != SUCCESS)
         {
            return( z_rc );
         }
         ext_usr_conv = ON;
      }

      /*--------------   add entries to Long-Word Table   --------------*/
      else if ( z_ret.res == LONG )
      {
         if ( z_length >= 3 )
         {
            if ( ( kcb.cnvx == KAKNO ) && ( kcb.env != ENVNEXT ) )
            {
               z_offset = z_stptr - kcb.udeude;

               z_rc = _Kcjuget(z_kcbptr,(short)LTB,z_offset+(short)z_yl,
                        z_offset+(short)z_yl+(short)z_dl-(short)1,
                        z_strpos,z_ret.len,z_offset+(short)1,z_yl-(short)1);
               if( z_rc != SUCCESS )
                   return( z_rc );
              
               mce2.jdok = JD_LONG;
            }
         }
      }

      /*---------------   set pointer of next entries   ----------------*/
      z_udptr = z_stptr + z_yl + z_dl;
   }

   if ( mce2.jdok == JD_READY )         /* if no candidates is found    */
      mce2.jdok = JD_COMPLETE;

/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
   return( SUCCESS );
}
