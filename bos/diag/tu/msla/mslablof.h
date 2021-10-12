/* @(#)20       1.5  src/bos/diag/tu/msla/mslablof.h, tu_msla, bos411, 9428A410j 7/30/91 15:19:29 */
/*
 * COMPONENT_NAME: (mslablof.h) header file for MSLA diagnostic
 *			application.
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Path for microcode files.
 */

 #define DiagUcodeDir	"/etc/microcode/"

/*
 *  THE FOLLOWING MICROCODES FILES HAVE 7 BYTE IDENTIFICATION
 *  HEADERS FOR LOADS WITH THE BASIC INTERPRETER.
 *  HENCE, FUNCTIONS FOR LOADING THE CODES IN 'C' SHOULD
 *  DISCARD THE FIRST 7 BYTES.
 */

/*************************************************
 *   msla19test.c  needs the following microcodes   
 *************************************************/

#define MODEMWRAP   "8787dmod.00.01"    /* source ftp14.asm */
#define SDLCWRAP    "8787dwrp.00.01"    /* source ftp17.asm */

/*************************************************
 *   msla20test.c   needs the following microcodes   
 *************************************************/

#define MSLA20_BLO1  "8787d50.00.01"    /* source ftp50.asm   */

/*************************************************
 *  mslapostest.c   needs the following microcodes   
 *************************************************/

#define MSLAPOS_BLO1 "8787dpl2.00.01"   /* source mtpparl2.asm  */
#define MSLAPOS_BLO2 "8787dpl3.00.01"   /* source mtpparl3.asm  */

/*************************************************
 *  msladma.c   needs the following microcodes   
 *************************************************/

#define MSLA_DMA2  "8787dd2.00.01"      /* source dma2.asm       */
#define MSLA_DMA1  "8787dd1.00.01"      /* source dma1.asm       */

/*************************************************
 *  mslaintrios.c   needs the following microcodes   
 *************************************************/

#define PARITY1_BLO "8787dp.00.01"      /* source parity1.asm    */

