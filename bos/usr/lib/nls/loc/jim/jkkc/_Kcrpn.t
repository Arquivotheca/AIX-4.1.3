/* @(#)80	1.2 6/4/91 15:21:28 */

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
 * MODULE NAME:       _Kcrpn.t
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
/************************************************************************/
/* _Right character's _Pe_Nalty					*/
/*                                                                      */
/*    The _R_P_N is a table of penalty of right character.	*/
/************************************************************************/

/* HINR x rightchar -> penalty conv, */
static short rchr_pen[21][13] = {
/*            UD  AL  KT   .   ,  K1  K2  K3  K4   (   )   DL  NM */
/* UNDEF   */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* MEISHI  */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* RENYOU  */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* MEIREN  */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* SHUUSHI */  0,  5,  5, -5,  5,  5,  5,  5,  5,  5,  0,  0,  5,
/* RENTAI  */  0, -2, -2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* SHUREN  */  0, -2, -2, -5,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* FYOU    */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* FTAI    */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* FNEU    */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* OTHER   */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* KUGIRI  */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* CONVKEY */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* SETTO   */ 10,  0,  0, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
/* NSETTO  */ 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,  0,
/* PSETTO  */ 10,  0,  0, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
/* OSETTO  */ 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
/* GSETTO  */ 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
/* SETSUBI */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* SUUCHI  */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* KOYUU   */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
};

