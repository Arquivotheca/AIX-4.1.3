/* @(#)44	1.2 6/4/91 15:13:21 */

/*    
 * COMPONENT_NAME: (libKJI) Japanese Input Method 
 *
 * FUNCTIONS: kana-Kanji-Conversion (KKC) Library
 * 
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when 
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kcmac.h
 *
 * DESCRIPTIVE NAME:  Defines  Macros.
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------*  
 *      MRU Length Operation Macro.
 *----------------------------------------------------------------------*/ 
#define GMRULEN(adr)     GETSHORT(adr)
#define SMRULEN(adr,dat) SETSHORT(adr,dat)

/*----------------------------------------------------------------------*  
 *      Dictionary Short Value Get.
 *----------------------------------------------------------------------*/ 
#define GETSHORT(adr) ( (((uschar *)(adr))[1]<<8) + ((uschar *)(adr))[0])
#define SETSHORT(adr,dat) \
(((uschar *)adr)[0]=LOW(dat),((uschar *)adr)[1]=HIGH(dat))

/*----------------------------------------------------------------------*  
 *      Get from LSB to direction MSB 8bit.
 *----------------------------------------------------------------------*/ 
#define LOW(x)     ((unsigned char)(((unsigned short)x) & 0xff))

/*----------------------------------------------------------------------*  
 *      Get from LSB 8bit Position to Direction MSB 8Bit.
 *----------------------------------------------------------------------*/ 
#define HIGH(x)    ((unsigned char)(((unsigned short)x) >> 8))

/*----------------------------------------------------------------------*  
 *      Get DBCS Char from Character Pointed Area.
 *----------------------------------------------------------------------*/ 
#define CHTODBCS(x)  CHPTTOSH(x)

/*----------------------------------------------------------------------*  
 *      Get Short From Character Pointed 2byte.
 *----------------------------------------------------------------------*/ 
#define CHPTTOSH(adr) \
( (((unsigned char *)(adr))[0]<<8) +((unsigned char *)(adr))[1])

/*----------------------------------------------------------------------*  
 *      Set DBCS Char to Character Pointed Area.
 *----------------------------------------------------------------------*/ 
#define DBCSTOCH(adr,dat) SHTOCHPT(adr,dat)

/*----------------------------------------------------------------------*  
 *      Set Short Data to Character Pointed Area.
 *----------------------------------------------------------------------*/ 
#define SHTOCHPT(adr,dat)\
(((unsigned char *)adr)[0]=HIGH(dat),((unsigned char *)adr)[1]=LOW(dat))

/*----------------------------------------------------------------------*  
 *      Which is Miminum Number Get.
 *----------------------------------------------------------------------*/
#define MIN( a,b )  (( (a)<=(b) ) ? a:b)

/*----------------------------------------------------------------------*  
 *      Which is Maximum Number Get.
 *----------------------------------------------------------------------*/
#define MAX( a,b )  (( (a)> (b) ) ? a:b)

/*----------------------------------------------------------------------*  
 *      Allways Fail Condition.
 *----------------------------------------------------------------------*/
#define NILCOND     ( 0 == 1 )

/*----------------------------------------------------------------------*  
 *      Alignment Specified Number(Aligment Round).
 *----------------------------------------------------------------------*/
#define ALIGN( x,siz ) ((x) + ( ((x) % siz ) ? siz - ((x) % siz) : 0 ))

/*----------------------------------------------------------------------*  
 *      Alginment Specified Number(Alignment Truncate).
 *----------------------------------------------------------------------*/
#define ALIGNT( x,siz) ((x) - ( ((x) % siz) ? (x) % siz:0 ))
#ifdef  lint
#define IDENTIFY(func)
#else
#define IDENTIFY(func) static char *mod_name="@(#)func"
#endif

/*----------------------------------------------------------------------*
 *      Absolute Function
 *----------------------------------------------------------------------*/
#define abs(x) ( ((x) <0) ? (-x):(x) )

/*----------------------------------------------------------------------*  
 *      Macro for Chain Type Table Management
 *----------------------------------------------------------------------*/
#define    NEGA   -1
#define    CMOVF(hh,he)  he = hh.chepool + (*he).chef
#define    CMOVB(hh,he)  he = hh.chepool + (*he).cheb
#define    CMOVT(hh,he)\
 he = (hh.chhchh.act==NEGA) ?NULL:(hh.chepool + hh.chhchh.act)
#define    CMOVL(hh,he)\
 he = (hh.chhchh.act==NEGA) ?NULL:(hh.chepool + hh.chepool[hh.chhchh.act].cheb)

#define    FOR_FWD(hh,he,flg) \
 for(CMOVT(hh,he),flg=OFF;(flg!=ON)&&(hh.chhchh.act!=NEGA);CMOVF(hh,he))

#define    FOR_BWD(hh,he,flg)\
 for(CMOVL(hh,he),flg=OFF;(flg!=ON)&&(hh.chhchh.act!=NEGA);CMOVB(hh,he))

#define    LST_FWD(he,flg) if((*he).last == ON) flg = ON
#define    LST_BWD(he,flg) if((*he).top == ON)  flg = ON

#define    FOR_FWD_MID(hh,he,flg) \
 for(flg = OFF;(flg != ON)&&(hh.chhchh.act!=NEGA);CMOVF(hh,he))

#define    FOR_BWD_MID(hh,he,flg)\
 for(flg = OFF;(flg != ON)&&(hh.chhchh.act!=NEGA);CMOVB(hh,he))

#define    CACTV(hh)  ( hh.chhchh.actv )
#define    CSTTS(he)  ( (he != NULL)?(((*he).actv == ON )? ON : OFF):OFF )

#define    CHEPTR(sub) (struct CHE *)(chh.chepool+(chh.size * sub))
#define    CHESUB(ptr) (((uschar *)ptr - chh.chepool)/chh.size )
