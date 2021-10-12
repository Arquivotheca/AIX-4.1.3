/* @(#)89	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcinf.h, libKJI, bos411, 9428A410j 7/23/92 03:10:19	*/

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
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kcinf.h
 *
 * DESCRIPTIVE NAME:  EXTENDED INFOMATION
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:
 *
 ******************** END OF SPECIFICATIONS *****************************/
#ifdef _AIX
#define SDICT_NUM	16	
#define SDICT_LEN      	80  
struct INF
{
   char   *sysnm[SDICT_NUM+1];          /* name of system dictioanry    */
   char   *usrnm;                       /* name of user dictionary      */
   char   *adjnm;                       /* name of fuzoku-go dictionary */
};
#else
#   if defined(_AGC) || defined(_GSW)
struct INF
{
   uschar   *sysptr;                      /* pointer of system dictioanry */
   uschar   *usrptr;                      /* pointer of user dictionary   */
   uschar   *adjptr;                      /* pointer of adjunct dictionary*/
};
#   endif
#endif

 struct INF *infptr1;                   /* primary   INF pointer        */

#define inf      (*infptr1)
