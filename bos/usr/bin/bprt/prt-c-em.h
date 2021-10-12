/* @(#)25	1.1  src/bos/usr/bin/bprt/prt-c-em.h, libbidi, bos411, 9428A410j 8/27/93 09:57:07 */
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
/*****************************HEADER FILE HEADER*******************************/
/*                                                                            */
/*   HEADER FILE NAME: PRT-C-EM.H <Emulator of some C functions>              */
/*                                                                            */
/*   FUNCTION: This file contains the implementation of some C functions.     */
/*             These functions are used by the Printer's APIs.                */
/*                                                                            */
/******************************************************************************/

void   *MemMove( void *dest, void *src, unsigned count );
/* removed far , before *MemMove, *dest, *src ,commented for AIX*/
