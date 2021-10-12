static char sccsid[] = "@(#)08	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcjstag.c, libKJI, bos411, 9428A410j 7/23/92 03:15:33";
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
 * MODULE NAME:       _Kcjstag
 *
 * DESCRIPTIVE NAME:  PICKS UP TYPE HINPOS DFLAG FROM THE SPECIFIED OFFSET.
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS): success
 *                    0x7fff(UERROR):  unpredictable error
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
struct RETJSTAG _Kcjstag(z_kcbptr,z_offset,z_morpos,z_morlen,z_file)

struct KCB   *z_kcbptr;
short        z_offset;
uschar       z_morpos;                  /* refer JLE uschar strap       */
uschar       z_morlen;                  /*           uschar len         */
short	     z_file;			/* System File Number		*/
{
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcmce.h"
#include   "_Kcmcb.h"
#include   "_Kcjtx.h"

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   struct RETJSTAG   z_ret;             /* return code sane area        */
   uschar            z_tag;             /* tag save area                */
   uschar            z_dflag[DFLAG_MAX];/* dflag save area              */
   uschar            *z_dbuf;           /* work to point                */
   uschar            z_wk1;             /* 1st byte of kj-code save area*/
   uschar            z_wk2;             /* 2nd byte of kj-code save area*/
   uschar	     z_shoki;		/* Not Use at Shoki Henkan Flag */
   uschar	     z_index;		/* Dict information data	*/
   uschar	     z_dfbyte;		/* Dflag Length			*/

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* establish address'ty to kcb  */
   mceptr1 = kcb.mchmce;                /* establish address'ty to mce  */

/*----------------------------------------------------------------------*
 *      INITIALIZE
 *----------------------------------------------------------------------*/
   z_ret.rc = SUCCESS;                  /* initialize return code       */
   if( z_morlen == 1 ) {
       mcbptr1 = (struct MCB *)kcb.myarea;
       z_dbuf  = mcb.dsyseg[z_file] + mcb.mulsys[z_file].record_size + z_offset;
   }
   else
       z_dbuf  = kcb.sdesde + z_offset;

/*----------------------------------------------------------------------*
 *      SET SHOKI HENKAN DATA					
 *----------------------------------------------------------------------*/
   z_shoki = *z_dbuf;

/*----------------------------------------------------------------------*
 *      MOVE DICTIONARY DATA IN BUFFER INTO LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   /*-------------------------------------------------------------------*
    *   CHECK NO SHOKI HENKAN 
    *-------------------------------------------------------------------*/
   if(( z_shoki == 0x80 ) || ( z_shoki == 0xC0 ))
       *z_dbuf++;

   /*-------------------------------------------------------------------*
    *   SET KANJI DATA & SKIP KANJI DATA          
    *-------------------------------------------------------------------*/
   z_wk1 = *( z_dbuf + 0 );
   z_wk2 = *( z_dbuf + 1 );
   for( ; ( *z_dbuf & 0x80 ) == 0 ; z_dbuf += 2 ) ; 
   
   /*-------------------------------------------------------------------*
    *   SET ADDITIONAL INFORMATION INDEX
    *-------------------------------------------------------------------*/
   z_index = *( z_dbuf += 2 );

   /*-------------------------------------------------------------------*
    *   SET TAG                           
    *-------------------------------------------------------------------*/
   if(( z_index & 0xC0 ) == 0 )
       z_tag = 0x00;
   else
       z_tag = *( ++z_dbuf );
   *z_dbuf++;

   /*-------------------------------------------------------------------*
    *   SET DFLAG LENGTH                  
    *-------------------------------------------------------------------*/
   z_dfbyte = ( z_index >> 4 ) & 0x03;

   /*-------------------------------------------------------------------*
    *   SET DFLAG                  
    *-------------------------------------------------------------------*/
   z_dflag[0] = 0x00; z_dflag[1] = 0x00; z_dflag[2] = 0x00;

   switch( z_dfbyte ) {
       case 1:
		z_dflag[0] = *z_dbuf++;
		break;
       case 2:
		z_dflag[0] = ( *z_dbuf++ | 0x80 );	
		z_dflag[1] =   *z_dbuf++;
		break;
       case 3:
		z_dflag[0] = ( *z_dbuf++ | 0x80 );
		z_dflag[1] = ( *z_dbuf++ | 0x80 );
		z_dflag[2] =   *z_dbuf++;
		break;
   }
   if( z_morlen == 1 ) 
       z_ret.newoff = z_dbuf-(mcb.dsyseg[z_file]+mcb.mulsys[z_file].record_size);
   else
       z_ret.newoff = z_dbuf - kcb.sdesde;

   if( z_tag == 0x6a ) z_tag = 0x71;	/* prefix			*/
   if( z_tag == 0x6b ) z_tag = 0x72;	/* suffix			*/

#ifdef DBG_MSG
printf("tag=%2x(%3d) ", z_tag, z_tag );
#endif

   /*-------------------------------------------------------------------*
    *   GET TYPE & POSITION OF HINSHI  
    *-------------------------------------------------------------------*/
   /*----------------------     general words     ----------------------*/
   if (z_tag < 112)                     /* regular word    ( < 0x70 )   */
   {
      if (z_dflag[1] & NUMBER)	
         z_ret.type = TP_SUUCHI;        /* set jiritsu-go type 6        */
      else
         z_ret.type = TP_IPPAN;         /* set jiritsu-go type 0        */
      z_ret.hinpos = z_tag;
   }
   /*----------------------     spetial words     ----------------------*/
   else
   {
      switch(z_tag)
      {
      /*----------------------------------------------------------------*
       *        PREFIX
       *----------------------------------------------------------------*/
      case 113 :                        /* prefix                       */
         z_ret.type = TP_SETTO;
         z_ret.hinpos = 106;
                                        /* setto-go 'o'                 */
                                        /*           &                  */
                                        /*     length 1                 */

         mceptr2 = kcb.mchmce + z_morpos;
         z_wk1 &= 0xbf;

         if ( ( ( z_wk1 == 0xbf ) && ( z_wk2 == 0xfe ) )
                || ( ( z_wk1 == 0x8c ) && ( z_wk2 == 0xe4 ) ) )
         {
            if ( ( z_morlen == 1 ) && ( mce2.code == 0x05 ) )
            {
               z_dflag[0] &= 0xf8;
               z_dflag[0] |= 0x05;
               z_ret.type = TP_OSETTO;
            }
                                        /* setto-go 'go'                */
                                        /*           &                  */
                                        /*     length 1                 */
            else if ( ( z_morlen == 1 ) && ( mce2.code == 0x0f ) )
            {
               z_dflag[0] &= 0xf8;
               z_dflag[0] |= 0x05;
               z_ret.type = TP_GSETTO;
            }
         }
         break;

      /*----------------------------------------------------------------*
       *        SUFFIX
       *----------------------------------------------------------------*/
      case 114 :                        /* suffix                       */
         z_ret.type = TP_SETSUBI;
         z_ret.hinpos = 107;

         if ( ( z_dflag[0] & 0x20 ) != 0 )
            z_ret.hinpos += 1;          /* SA-hen                       */

         if ( ( z_dflag[0] & 0x10 ) != 0 )
            z_ret.hinpos += 2;          /* adjective                    */

         if ( ( z_dflag[0] & 0x08 ) != 0 )
            z_ret.hinpos += 4;          /* adverb                       */

         break;

      /*----------------------------------------------------------------*
       *     PREFIX FOR NUMERIC  
       *----------------------------------------------------------------*/
      case 117 :                        /* prefix for numeric           */
         z_ret.type = TP_NSETTO;
         z_ret.hinpos = 106;
         break;

      /*----------------------------------------------------------------*
       *     SUFFIX FOR NUMERIC     
       *----------------------------------------------------------------*/
      case 118 :                        /* suffix for numeric           */
         z_ret.type = TP_JOSUSHI;
         z_ret.hinpos = 116;
         break;

      /*----------------------------------------------------------------*
       *     PROPER NOUN   
       *----------------------------------------------------------------*/
      case 120 :                        /* proper noun                  */
         z_ret.type = TP_KOYUU;
         z_ret.hinpos = 0;
         break;

      /*----------------------------------------------------------------*
       *     PREFIX FOR PROPER NOUN     
       *----------------------------------------------------------------*/
      case 121 :                        /* prefix for proper noun       */
         z_ret.type = TP_PSETTO;
         z_ret.hinpos = 106;
         break;

      /*----------------------------------------------------------------*
       *     SUFFIX FOR PROPER NOUN 
       *----------------------------------------------------------------*/
      case 122 :                        /* suffix for proper noun       */
         z_ret.type = TP_PSETSUBI;
         z_ret.hinpos = 107;
         break;

      }
   }

   z_ret.flag[0] = z_dflag[0];          /* set flag information in r/c  */
   z_ret.flag[1] = z_dflag[1];          /* set flag information in r/c  */

/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
   return(z_ret);
}
