/* @(#)91       1.1  src/bos/kernext/disp/wga/inc/wga_reg.h, wgadd, bos411, 9428A410j 10/28/93 18:28:47 */
/*
 * COMPONENT_NAME: (WGADD) Whiteoak Graphics Adapter Device Driver
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

#ifndef _WGA_REG
#define _WGA_REG 

/****START OF SPECIFICATIONS *******************************************
 *    IDENTIFICATION: DEF.H                                            *
 *    DESCRIPTIVE NAME:  White Oak Graphics Adaptor Register Offsets      *
 *                       from the base address and possible values     *
 *                                                                     *
 *    FUNCTION:  Define register offsets and values                    *
 *                                                                     *
 *    ATTRIBUTES:                                                      *
 *    REFERENCES:  White Oak Graphics Adaptor Specification               *
 *                   Ron Arroyo, 5/15/92                               *
 ***********************************************************************
 *                                                                     *
 ****END   OF SPECIFICATIONS ******************************************/




/******************************************************************************/
/******************** BUID 40 Register BASE ADDRESSES *************************/
/******************************************************************************/

/********Start of White Oak Addresses *****************************************/
#define WGA_ADDR_BASE    (0x00000000)               /*0x00000000*/
#define VPD_BASE         (0x00000000+bus_base_addr) /*0x00000000*/
#define CNTL_REG_BASE    (0x08000000+bus_base_addr) /*0x08000000*/
#define VRAM_LIN_BASE    (0x00400000+bus_base_addr) /*0x00400000*/
#define VRAM_XY_BASE     (0x04000000+bus_base_addr) /*0x04000000*/
#define WTKN_REG_BASE	 (0x09000000+bus_base_addr) /*0x09000000*/
#define WTKN_VRAM_BASE	 (0x09200000+bus_base_addr) /*0x09200000*/
#define WTKN_VRAM_END    (0x093fffff+bus_base_addr) /*0x093fffff*/
/**************************************************End of Salmon Addresses*/

#define XYADDR(x,y,mode) (ulong *)(mode | (y)*YSHIFT + (x)*XSHIFT+bus_base_addr)
#define LINADDR(x,y)     (ulong *)(0x400000 | (y)*0x800  + (x) + bus_base_addr)

unsigned long bus_base_addr;/* received from BUSMEM_ATT(BUS_ID, WGA_ADDR_BASE)*/

/******************************************************************************/
/******************** VPD ADDRESSES/OFFSETS ***********************************/
/******************************************************************************/

/*-----------------  Vital Product Data Register Offsets  --------------------*/
#define VPD0        (volatile unsigned long *)(VPD_BASE+0x00)
#define VPD1        (volatile unsigned long *)(VPD_BASE+0x04)
#define VPD_DATA    (volatile unsigned long *)(VPD_BASE+0x08)

/******************************************************************************/
/***************** DISPLAY CONTROLLER ADDRESSES/OFFSETS ***********************/
/******************************************************************************/

/*-----------------  Control/Status Register Offsets  ------------------------*/
#define ADCNTL   (volatile unsigned long *)(CNTL_REG_BASE + 0x00)
#define ADSTAT   (volatile unsigned long *)(CNTL_REG_BASE + 0x04)

#define WEITEK_BASE_ADDR   (WTKN_REG_BASE + 0x00100000) 

/*------------  Weitek 8720 Control Register Offsets  ----------------------*/

#define WTKN_SYS_CONFIG (volatile unsigned long *)(WEITEK_BASE_ADDR + 0x4)
#define WTKN_INTR_ENBL  (volatile unsigned long *)(WEITEK_BASE_ADDR + 0xc)

/*--------------  Horz/Vert Display Control Register Offsets  ----------------*/

#define HRZT      (volatile unsigned long *)(WEITEK_BASE_ADDR + 0x108)
#define HRZSR     (volatile unsigned long *)(WEITEK_BASE_ADDR + 0x10C)
#define HRZBR     (volatile unsigned long *)(WEITEK_BASE_ADDR + 0x110)
#define HRZBF     (volatile unsigned long *)(WEITEK_BASE_ADDR + 0x114)
#define PREHRZC   (volatile unsigned long *)(WEITEK_BASE_ADDR + 0x118)
#define VRTT      (volatile unsigned long *)(WEITEK_BASE_ADDR + 0x120)
#define VRTSR     (volatile unsigned long *)(WEITEK_BASE_ADDR + 0x124)
#define VRTBR     (volatile unsigned long *)(WEITEK_BASE_ADDR + 0x128)
#define VRTBF     (volatile unsigned long *)(WEITEK_BASE_ADDR + 0x12C)
#define PREVRTC   (volatile unsigned long *)(WEITEK_BASE_ADDR + 0x130)
#define SRTCTL    (volatile unsigned long *)(WEITEK_BASE_ADDR + 0x138)

/*------------------  VRAM Control Register Offsets  ------------------------*/
#define MEMCFG    (volatile unsigned long *)(WEITEK_BASE_ADDR + 0x184)
#define RFPERIOD  (volatile unsigned long *)(WEITEK_BASE_ADDR + 0x188)
#define RLMAX     (volatile unsigned long *)(WEITEK_BASE_ADDR + 0x190)

/*----------------  Brooktree Access Register Offsets  -----------------------*/
#define DAC_IND1     (volatile unsigned long *)(WEITEK_BASE_ADDR + 0x200)
#define DAC_IND2     (volatile unsigned long *)(WEITEK_BASE_ADDR + 0x204)
#define DAC_CFG      (volatile unsigned long *)(WEITEK_BASE_ADDR + 0x208)
#define DAC_PALETTE  (volatile unsigned long *)(WEITEK_BASE_ADDR + 0x20C)

/*---------------------  Load Length Register Offsets  -----------------------*/

/*---------------------  Windowing Register Offsets  -------------------------*/
#define WORIG    (volatile unsigned long *)(CNTL_REG_BASE + 0x100)

/*--------------------- Cursor Control Register Offsets  ---------------------*/

/*
 * The Cursor color registers require three writes to initialize.  (R, G, B)
 */
#define CUR_COLOR_REG1 (volatile unsigned long *) (WEITEK_BASE_ADDR + 0x0181)
#define CUR_COLOR_REG2 (volatile unsigned long *) (WEITEK_BASE_ADDR + 0x0182)
#define CUR_COLOR_REG3 (volatile unsigned long *) (WEITEK_BASE_ADDR + 0x0183)

#define CUR_COMMAND_REG (volatile unsigned long *) (WEITEK_BASE_ADDR + 0x0300)

#define CUR_X_LOW_REG (volatile unsigned long *) (WEITEK_BASE_ADDR + 0x0301)
#define CUR_X_HIGH_REG (volatile unsigned long *) (WEITEK_BASE_ADDR + 0x0302)
#define CUR_Y_LOW_REG (volatile unsigned long *) (WEITEK_BASE_ADDR + 0x0303)
#define CUR_Y_HIGH_REG (volatile unsigned long *) (WEITEK_BASE_ADDR + 0x0304)

#define CUR_RAM_ADDR (WEITEK_BASE_ADDR + 0x0400)
/*---------------- Serializer Palette DAC Register Offsets  ------------------*/


/*------------------- Error Condition Register Offsets  ----------------------*/
#define ERRADDR  (volatile unsigned long *)(CNTL_REG_BASE + 0x30000)

#endif /* _WGA_REG */
