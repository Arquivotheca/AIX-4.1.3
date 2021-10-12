/* @(#)23	1.1  src/bos/usr/bin/bprt/prt-acc.h, libbidi, bos411, 9428A410j 8/27/93 09:57:03 */
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/***************************** HEADER FILE HEADER *****************************/
/*                                                                            */
/*   HEADER FILE NAME: Acc.H  <Accumulator Handler>                           */
/*                                                                            */
/*   FUNCTION: Provides declaration and function heading for proper           */
/*             linkage with PRT-ACC.C                                         */
/*                                                                            */
/*   ENTRY POINTS:                                                            */
/*               AccProcessChar                                               */
/*               AccProcessLine                                               */
/*                                                                            */
/******************************************************************************/

#define LONG  long
#define WCHAR LONG

extern void AccProcessLine (LONG    LineLength,
                            LONG    AccStep,
                            char   * Line);

extern void AccProcessChar (LONG    Char);

