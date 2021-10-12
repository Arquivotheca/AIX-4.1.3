/* @(#)75	1.1  src/bos/diag/tu/gga/ggareg.h, tu_gla, bos41J, 9515A_all 4/6/95 09:27:05 */
/*
 *   COMPONENT_NAME: tu_gla
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*****************************************************************************/
/******************** Video Control Registers: Screen Repaint ****************/
/*****************************************************************************/
/*  Bits 0-2   QSF COUNTER POSITION -----------------------------------------*/
/*  Bit  3     BUFFER FOR DISPLAY -------------------------------------------*/
#define DISP_BUFFER        0x00000000      /* 0 = Buffer 0, 1 = Buffer 1     */
/*  Bit  4     SCREEN REPAINT MODE ------------------------------------------*/
#define HBLNK_RELOAD       0x00000000      /* Screen repaint mode            */
/*  Bit  5     ENABLE VIDEO -------------------------------------------------*/
#define ENABLE_VIDEO       0x00000020      /* 0 = disable, 1 = enable        */
/*  Bit  6     COMPOSITE SYNCHRONIZATION ------------------------------------*/
#define COMPOSITE_SYNC     0x00000000      /* 0 = separate sync, 1 = comp.   */
/*  Bit  7     INTERNAL HORIZONTAL SYNCHRONIZATION --------------------------*/
#define INTERNAL_HSYNC     0x00000000      /* 0 = ext. HSYNC, 1 = int. HSYNC */
/*  Bit  8     INTERNAL VERTICAL SYNCHRONIZATION ----------------------------*/
#define INTERNAL_VSYNC     0x00000100      /* 0 = ext. VSYNC, 1 = int. VSYNC */

/******************************************************************************/
/*************** DRAWING ENGINE REGISTERS OFFSETS *****************************/
/******************************************************************************/

/*---------------------- PSEUDO REGISTERS -----------------------------------*/
#define IGM_POINT                       0x000
#define IGM_LINE                        0x040
#define IGM_TRIANGLE                    0x080
#define IGM_QUAD                        0x0C0
#define IGM_RECT                        0x100

#define IGM_ABS                         0x00     /* relative to window offset*/
#define IGM_REL                         0x20     /* relative to prev. vertex */
#define IGM_X                           0x08     /* read/write 32 bit X value*/
#define IGM_Y                           0x10     /* read/write 32 bit Y value*/
#define IGM_XY                          0x18     /* read/write 16 bit X & Y  */

/*--------------------------- WTK REGISTERS MASK ----------------------------*/
#define IGM_STATUS_QB_BUSY_MASK         0x80000000
#define IGM_STATUS_BUSY_MASK            0x40000000

