static char sccsid[] = "@(#)37	1.3  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcldcta.c, libKJI, bos411, 9428A410j 5/18/93 05:28:24";
/*
 * COMPONENT_NAME: (libKJI) Japanese Input Method (JIM)
 *
 * FUNCTIONS: Kana-Kanji-Conversion (KKC) Library
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kcldcta
 *
 * DESCRIPTIVE NAME:  INQUARE AND REGISTRATION FOR USER DICTIONARY
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS)     : success
 *                    0x0310(USRDCT_OVER) : no free area in user dict
 *                    0x0510(EQ_YOMI_GOKU): goku is equal to yomi
 *                    0x0a10(UPDATING)    : being updated
 *                    0x0b10(RQ_RECOVER)  : request recovery of user dict.
 *                    0x0582(USR_LSEEK)   : error of lseek()
 *                    0x0682(USR_READ)    : error of read()
 *                    0x0782(USR_WRITE)   : error of write()
 *                    0x0882(USR_LOCKF)   : error of lockf()
 *                    0x7fff(UERROR)      : unpredictable error
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------*  
 *      INCLUDE FILES                                                     
 *----------------------------------------------------------------------*/ 
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"                   /* Define Constant file         */
#include   "_Kcalp.t"                   /* Define Translating table     */

/************************************************************************
 *      START OF FUNCTION                                                 
 ************************************************************************/ 
short   _Kcldcta( z_kcbptr , z_mode )
struct  KCB     *z_kcbptr;              /* initialize of KCB            */
short            z_mode;                /* Inquire or Registration      */
{

/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
extern  short   _Kcliccs();             /* check the system dictionary  */
extern  short   _Kclcadp();             /* check & add user dictionary  */
extern  void    _Kclmrua();             /* add MRU area                 */

/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcymi.h"   /* YoMI buffer (YMI)                            */
#include   "_Kcsei.h"   /* SEIsho buffer entry (SEI)                    */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   short        z_rlcadp;               /* set return code             */
   short        z_knjlen;               /* set kanji length            */
   short        z_hklen;                /* set hira.katakana length    */
   uschar       z_hkdata[ 4 ];          /* set hira.katakana data      */
   long int     z_rc;                   /* set return code from _Kcliccs*/
   short        z_rlsdck;
   short        z_i;

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;                  /* initialize of kcbptr1        */
   ymiptr1 = kcb.ymiaddr;               /* initialize of ymiptr1        */
   seiptr1 = kcb.seiaddr;               /* initialize of seiptr1        */
   z_knjlen = ( kcb.seill -2 );         /* sub length field length 2.   */

   if((kcb.mode == MODALPHA)||          /* if Alphanumeric mode         */
      (kcb.mode == MODALKAN))           /* Alpha kansu                  */
   {
   /*-------------------------------------------------------------------*
    *      CHECK THE SEISHO & YOMI  AND SHIFT YOMI BACKWARD
    *-------------------------------------------------------------------*/
      for(z_i = 0; z_i < kcb.ymill1; z_i++)
      {
         if((ymi.yomi[z_i] >= 'A')&&(ymi.yomi[z_i] <= 'Z'))
         {
            if((dbalph[(ymi.yomi[z_i])] != CHPTTOSH(sei.kj[z_i]))&&
               (dbalph[(ymi.yomi[z_i] - 0x20)] != CHPTTOSH(sei.kj[z_i])))
               break;
         }
         else
         {
            if(dbalph[(ymi.yomi[z_i] - 0x20)] != CHPTTOSH(sei.kj[z_i]))
               break;
         }
      }
      if ((z_i == kcb.ymill1)&&(z_i == z_knjlen/2))
         return(EQ_YOMI_GOKU);

      for(z_i = kcb.ymill1; z_i > 0; z_i--)
      {
         ymi.yomi[z_i] = ymi.yomi[z_i-1];
      }
      ymi.yomi[0] = ESC_ALPHA;
      kcb.ymill1++;
   }
   else
   /*-------------------------------------------------------------------*
    *      CHECK THE SYSTEM DICTIONARY
    *-------------------------------------------------------------------*/
   {
      unsigned char tmp[256], save[256];
      int           tlen, slen;

      /* save original yomi */
      if(*(ymi.yomi) == ESC_ALPHA){
         memcpy(save, ymi.yomi, kcb.ymill1);
         slen = kcb.ymill1;
         (kcb.ymill1)--;
         AlnumToEMORA(&(ymi.yomi[1]), kcb.ymill1, tmp);
         memcpy(ymi.yomi, tmp, kcb.ymill1);
         ymi.yomi[kcb.ymill1] = '\0';
         kcb.ymill2 = kcb.ymill1;
      }else{
         *save ='\0';
      }

      z_rlsdck = _Kcliccs( kcbptr1 );

      /* restore original yomi */
      if(*save == ESC_ALPHA){
         kcb.ymill1 = kcb.ymill2 = slen;
         memcpy(ymi.yomi, save, kcb.ymill1);
      }

      if ( z_rlsdck != SUCCESS )        /* already exist in system dict*/
         return(z_rlsdck);
   }
/*---------------------------------------------------------------------*
 *      CHECK AND REGISTER THE USER   DICTDIONARY
 *---------------------------------------------------------------------*/
   z_rlcadp = _Kclcadp( kcbptr1,z_mode, ymi.yomi, kcb.ymill1,
                                   sei.kj  , z_knjlen );

   if ( z_rlcadp  != SUCCESS )
      return( z_rlcadp );

/*---------------------------------------------------------------------*
 *      WRITE IN MRU
 *---------------------------------------------------------------------*/
   if(z_mode != U_MODINQ )
   {
      _Kclmrua(kcb.mdemde,ymi.yomi,
               (uschar)kcb.ymill1,sei.kj,(uschar)z_knjlen );
   }
/*---------------------------------------------------------------------*
 *      RETURN
 *---------------------------------------------------------------------*/

   return( SUCCESS );

}

/*----------------------------------------------------------------------*
 *	convert 7 bit alpha-num yomi to mora (first shift must not be in)
 *----------------------------------------------------------------------*/
static	int	AlnumToEMORA
(
unsigned char	*in,		/* yomi string to be checked		*/
int		inlen,		/* yomi (in) string length		*/
unsigned char	*out		/* mora string for system dictionary	*/
)
{
	while(inlen-- > 0){
		*out++ = OLDtoEMORA[*in++];
	}
}
