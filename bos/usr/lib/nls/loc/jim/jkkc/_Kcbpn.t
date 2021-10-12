/* @(#)17	1.2 6/4/91 10:13:43 */

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
 * MODULE NAME:       _Kcbpn.t
 *
 * DESCRIPTIVE NAME:
 *
 * FUNCTION:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:
 *
 ******************** END OF SPECIFICATIONS *****************************/
/************************************************************************/
/************************************************************************/
/*                                                                      */
/*      Penalty of relation between bunsetsus                           */
/*                                                                      */
/************************************************************************/
/************************************************************************/

/* endhin x strhin -> penalty conv, */

static short  bb_pen[21][15] = {
/*             U   N   NY  Y   F   O   ,   .   C   P   S   Sn  Sk  N   K  */
/* UNDEF   */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* MEISHI  */  0, CW, CW,  2,  0, 10,  3,  7,  0, GP, GS,  0, -5,  0,  0,
/* RENYOU  */  0,  0,  0,  0,  0, 10, -3, 15,  0,  0,  0,  0,  0,  0,  0,
/* MEIREN  */  0, CW,  0,  0,  0, 10, -3,  7,  0, GP, GS,  0, -5,  0,  0,
/* SHUUSHI */  0, 15, 15, 15, 15, 15,  5, -7,  0, 15,  0,  0,  0, 15, 15,
/* RENTAI  */  0,  0,  0,  0,  0, 10,  0,  7,  0, -2,  0,  0,  0,  0, -2,
/* SHUREN  */  0, -2,  0,  0,  0, 10,  0, -7,  0, -2,  0,  0,  0,  0, -2,
/* FYOU    */  0,  0,  0,  0,  0, 10,  0,  7,  0,  0,  0,  0,  0,  0,  0,
/* FTAI    */  0,  0,  0,  0,  0, 10,  0,  7,  0, -2,  0,  0,  0,  0, -2,
/* FNEU    */  0,  0,  0,  0,  0, 10,  0,  7,  0,  0,  0,  0,  0,  0,  0,
/* OTHER   */  0,  0,  0,  0,  0,  0, -3,  7,  0,  7,  0,  0,  0,  7,  7,
/* KUGIRI  */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 10, 10, 10,  0,  0,
/* CONVKEY */  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, -1, -1, -1,  0,  0,
/* SETTO   */  0, PG, PG, 10, 10, 10, 10, 10, 10, PP, 10, 10, 10, 10, 10,
/* NSETTO  */  0, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,NPS, 10,NPG, 10,
/* PSETTO  */  0, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,KPG,
/* OSETTO  */  0, OY, OY, OY, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
/* GSETTO  */  0, GY, GY, GY, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
/* SETSUBI */  0, SG, SG,  5, 10,  0,  3,  7,  0, SP, SS,NSS,  0,  0,  0,
/* SUUCHI  */  0,  0,  0,  5, 10, 10,  3,  7,  0,  0,  0,NGS,  0,  0,  0,
/* KOYUU   */  0,  0,  0,  5, 10, 10,  3,  7,  0,  0, -6, 10,KGS,  0,KCW,
};
