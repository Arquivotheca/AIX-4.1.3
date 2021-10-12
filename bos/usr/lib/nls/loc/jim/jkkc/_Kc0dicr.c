static char sccsid[] = "@(#)90	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kc0dicr.c, libKJI, bos411, 9428A410j 6/4/91 10:03:53";
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
 * MODULE NAME:       _Kc0dicr
 *
 * DESCRIPTIVE NAME:  REGISTRATION OF WORD ENTRY FOR USER DICTIONARY
 *
 * FUNCTION:
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

/************************************************************************
 *      START OF FUNCTION                                                 
 ************************************************************************/ 
short   _Kc0dicr( z_kcbptr )
struct  KCB     *z_kcbptr;              /* initialize of KCB           */
{

/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
extern  short   _Kcldcta();             /* check & add dict. addition   */

/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   short           z_rldcta;    /* Define Area for Return of _Kcldcta   */

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;                  /* initialize of kcbptr1        */

   z_rldcta = _Kcldcta(kcbptr1, (short)U_MODREG);
                                        /* Call Dict add. as registor   */
   return( z_rldcta );
}
