/* @(#)86       1.1  src/bos/diag/tu/wga/wgareg.h, tu_wga, bos411, 9428A410j 2/9/93 09:57:10 */
/*
 *   COMPONENT_NAME: TU_WGA
 *
 *   FUNCTIONS: WGA Test Units header file
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
#define COMPOSITE          0x00000000      /* 0 = separate sync, 1 = comp.   */
/*  Bit  7     INTERNAL HORIZONTAL SYNCHRONIZATION --------------------------*/
#define INTERNAL_HSYNC     0x00000000      /* 0 = ext. HSYNC, 1 = int. HSYNC */
/*  Bit  8     INTERNAL VERTICAL SYNCHRONIZATION ----------------------------*/
#define INTERNAL_VSYNC     0x00000100      /* 0 = ext. VSYNC, 1 = int. VSYNC */



/*****************************************************************************/
/********************** Bt459 RAMDAC ADDRESS FORMAT **************************/
/*****************  (redefine from Device Drive 'reg.h')  ********************/
/*****************************************************************************/
#define COLOR_PALETTE_RAM_OFFSET      0x0000     /* Bt459 palette RAM offset */
#define CURSOR_COLOR_REGISTERS_OFFSET 0x0181     /* Bt459 cursor color regs  */
#define COMMAND_REGISTER_0_OFFSET     0x0201     /* Bt459 command register 0 */
#define COMMAND_REGISTER_1_OFFSET     0x0202     /* Bt459 command register 1 */
#define COMMAND_REGISTER_2_OFFSET     0x0203     /* Bt459 command register 2 */
#define CURSOR_CMD_REG_OFFSET         0x0300     /* Bt459 cursor command addr*/
#define CURSOR_X_LOW_REG_OFFSET       0x0301     /* Bt459 cursor x low reg.  */
#define CURSOR_X_HIGH_REG_OFFSET      0x0302     /* Bt459 cursor y high reg. */
#define CURSOR_Y_LOW_REG_OFFSET       0x0303     /* Bt459 cursor x low reg.  */
#define CURSOR_Y_HIGH_REG_OFFSET      0x0304     /* Bt459 cursor y high reg. */
#define CURSOR_RAM_OFFSET             0x0400     /* Bt459 cursor RAM offset  */

#define DAC_ADL_REG_ADDR              DAC_IND1
#define DAC_ADH_REG_ADDR              DAC_IND2
#define CURCMND_REG_ADDR              DAC_CFG

#define COLOR_PALETTE_RAM_REG_ADDR (volatile ulong_t *)(WTKN_SYS_BASE + 0x20C)




/******************************************************************************/
/***************** DISPLAY VIDEO CONTROL ADRESSES/OFFSETS *********************/
/*****************  (redefine from Device Drive 'reg.h')  *********************/
/******************************************************************************/

/*-----------------  Control/Status Register Offsets  ------------------------*/

#define HRZC                      (volatile ulong_t *)(WTKN_SYS_BASE + 0x104)
#define VRTC                      (volatile ulong_t *)(WTKN_SYS_BASE + 0x11C)
#define SRADDR                    (volatile ulong_t *)(WTKN_SYS_BASE + 0x134)

#define HRZC_REG_ADDR             HRZC
#define HRZT_REG_ADDR             HRZT
#define HRZSR_REG_ADDR            HRZSR
#define HRZBR_REG_ADDR            HRZBR
#define HRZBF_REG_ADDR            HRZBF
#define PREHRZC_REG_ADDR          PREHRZC
#define VRTC_REG_ADDR             VRTC
#define VRTT_REG_ADDR             VRTT
#define VRTSR_REG_ADDR            VRTSR
#define VRTBR_REG_ADDR            VRTBR
#define VRTBF_REG_ADDR            VRTBF
#define PREVRTC_REG_ADDR          PREVRTC
#define SRADDR_REG_ADDR           SRADDR
#define SRTCTL_REG_ADDR           SRTCTL



/******************************************************************************/
/*************** DRAWING ENGINE REGISTERS ADDRESSES/OFFSETS *******************/
/******************************************************************************/


/*-----------------  Weitek Control Register Offsets  -----------------------*/
#define USER_REG_ADDR        (volatile ulong_t *)(WTKN_SYS_BASE + 0x80000)
#define PSEUDO_REGISTERS     (volatile ulong_t *)(WTKN_SYS_BASE + 0x81200)
#define PARM_ENGINE_ADDR     (volatile ulong_t *)(WTKN_SYS_BASE + 0x81000)
#define PARM_ENGINE_CNTL_ADDR (volatile ulong_t *)(WTKN_SYS_BASE + 0x80180)
#define DRAW_ENGINE_ADDR     (volatile ulong_t *)(WTKN_SYS_BASE + 0x80200)
#define PIXEL1_REG_ADDR      (volatile ulong_t *)(WTKN_SYS_BASE + 0x80080)
#define PIXEL8_REG_ADDR      (volatile ulong_t *)(WTKN_SYS_BASE + 0x8000C)


#define QUAD_CMD             (volatile ulong_t *)(WTKN_SYS_BASE + 0x80008)
#define BLIT_CMD             (volatile ulong_t *)(WTKN_SYS_BASE + 0x80004)

/*---------------------- DRAW ENGINE REGISTERS-------------------------------*/
#define FGROUND_REG_VAL                 0x000
#define BGROUND_REG_VAL                 0x004
#define PLANE_MASK_REG_VAL              0x008
#define DRAW_MODE_REG_VAL               0x00C
#define PAT_ORIGIN_X_REG_VAL            0x010
#define PAT_ORIGIN_Y_REG_VAL            0x014
#define RASTER_REG_VAL                  0x018
#define PIXEL8_REG_VAL                  0x01C
#define W_MIN_REG_VAL                   0x020
#define W_MAX_REG_VAL                   0x024

/*---------------------- DEVICE COORDINATE REGISTERS ------------------------*/
#define P_COORD_X0_Y0_REG_VAL           0x00
#define P_COORD_X1_Y1_REG_VAL           0x40
#define P_COORD_X2_Y2_REG_VAL           0x80
#define P_COORD_X3_Y3_REG_VAL           0xC0
#define DEVICE_REL                      0x00
#define DEVICE_YX                       0x18

/*---------------------- PARAMETER ENGINE REGISTERS--------------------------*/
#define CINDEX_REG_VAL                  0x0c
#define W_OFFSET_REG_VAL                0x10
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

