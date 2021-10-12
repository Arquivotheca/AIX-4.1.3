/* @(#)49	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcdct.h, libKJI, bos411, 9428A410j 7/23/92 02:59:38	*/

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
 * MODULE NAME:       _Kcdct.h
 *
 * DESCRIPTIVE NAME:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:
 *
 ******************** END OF SPECIFICATIONS *****************************/

/**************************************************************************/
/* System dictionaly indeX Entry (SXE)                                    */
/**************************************************************************/
struct SXE
{
 uschar   mkey[0x0C];                   /* mora code in sys dct index     */
					/* Same MAX_MORA_INDEX in _Knum.h */
};

/**************************************************************************/
/* System dictionaly Data (SD1)                                           */
/**************************************************************************/
struct SD1
{
   uschar   code;                       /* mora code in sys dct data      */
   uschar   offset[2];                  /* offset to next mora code       */
   uschar   length[2];                  /* length of data (exclusive)     */
};

/**************************************************************************/
/* System dictionaly Data (SD2)                                           */
/**************************************************************************/
struct SD2
{
   uschar   length[2];                  /* length of data (exclusive)     */
};

/**************************************************************************/
/* System dictionaly Data (SDC)                                           */
/**************************************************************************/
struct SDC
{
   uschar   count[3];                   /* Count of E.Mora                */
};

/**************************************************************************/
/* System dictionaly E.Mora Code (SDM)                                    */
/**************************************************************************/
struct SDM
{
   uschar   code[2];                    /* System dictionaly E.Mora Code  */
};

/**************************************************************************/
/* System dictionaly NEXT ADDRESS (SDA)                                   */
/**************************************************************************/
struct SDA
{
   uschar   addr[2];                    /* System dictionaly E.Mora Code  */
};

/**************************************************************************/
/* System dictionaly Data area length					  */
/**************************************************************************/
struct SDL
{
   uschar   length[2];                  /* System dictionaly E.Mora Code  */
};

/**************************************************************************/
/* User dictionaly indeX Entry (UXE)                                    */
/**************************************************************************/
struct UXE
{
   uschar il[2];                        /* length of index                */
   uschar  sts;                         /* data set status                */
   uschar  har;                         /* highest allocated RRN          */
   uschar  nar;                         /* next available RRN             */
   uschar  dptr;                        /* pointer of 1st entry           */
};

/**************************************************************************/
/* User dictionaly Data Entry (UDE)                                    */
/**************************************************************************/
struct UDE
{
   short   rl;                          /* length of the RRN              */
   uschar  *dptr;                       /* pointer of 1st entry           */
};
