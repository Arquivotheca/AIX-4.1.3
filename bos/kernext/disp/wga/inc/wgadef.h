/* @(#)95		1.1	src/bos/kernext/disp/wga/inc/wgadef.h, wgadd, bos411, 9428A410j 10/28/93 18:55:54 */
/*
 * COMPONENT_NAME: (WGADD) Whiteoak Graphics Adapter Device Driver
 *
 * FUNCTIONS:
 *
 * ORIGINS:	27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _WGA_DEF
#define _WGA_DEF
 
 
/*----------------------------------------------------------------------*/
/* Return codes                                                         */
/*----------------------------------------------------------------------*/
 
#define ERR_DEF_DEV     -1              /* define device error          */
#define SUCCESS_INIT    0               /* initialize device            */
#define ERR_INIT_DEV    -1              /* initialize device error      */
#define SUCCESS_INTR    0               /* process interrupt            */
#define ERR_INTR_NOT_US 2               /* interrupt not for this device*/
#define SUCCESS_TERM    0               /* terminate device             */
#define ERR_TERM_DEV    -1              /* terminate device error       */
#define ERROR_LOGGED    0               /* invalid SLIH operation reques*/
#define SUCCESS_IO      0               /* ioinitiate                   */
#define INTR_NOT_PROCESSED 2            /* no interrupt handler         */
#define VTT_BAD_VRM_RC  1               /* bad vrm ret code from sys cal*/
#define VTT_BAD_MODE    2               /* invalid mode parameter       */
#define INT_NOT_PROCESSED -1            /* interrupt not ours flag      */
#define MALLOC_PHYS_DISPLAY_FAILURE -1
#define INTERRUPT_INIT_FAILURE -1
 
 
/*----------------------------------------------------------------------*/
/* Others                                                               */
/*----------------------------------------------------------------------*/
 
 
#define BLANK_PS_CHAR   0x0020          /* a blank character            */
#define BLANK_PS_ATTR   0x0000          /* default attributes           */
 
 
#define  ACTIVE    1
#define  INACTIVE  0
#define  MONITOR   0
#define  CHARACTER 1
 
#define CUR_DBL_US              2       /* cursor double underscroe     */
                                        /* env cursor settings in 'defc'*/
 

#define COLORTAB        16              /* number of colors in color tab*/
#define VTT_ACTIVE      1               /* virtual terminal active      */
#define VTT_INACTIVE    0               /* virtual terminal inactive    */


#define ADAPT_MAX_FONTS    8     /* Max number of fonts for use  */


#define BUS_ID          0x840c0020


/*----------------------------------------------------------------------*/
/* Defines for p space initialization                                   */
/*----------------------------------------------------------------------*/

#define BLANK_CODE      0x00200000   /* Blank ascii code                 */
#define ATTRIB_DEFAULT  0x00002000   /* Set attribute to color 6 (cyan)  */


/* 
 * The Brooktree Color Palette has to be intialized and need the size of 
 * each color palette for Color Palette ram, Overlay color palette, and
 * Cursor color palette
 */

#define COLOR_RAM_SIZE			256*3
#define OVERLAY_PALETTE_SIZE		16*3
#define CURSOR_COLOR_PALETTE_SIZE	3*3

#endif /* _WGA_DEF */
