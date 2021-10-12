/* @(#)18       1.3  src/bos/kernext/disp/sga/inc/sga_reg.h, sgadd, bos411, 9428A410j 10/29/93 10:45:23 */
/*
 * COMPONENT_NAME: (SGADD) Salmon Graphics Adapter Device Driver
 *
 *   FUNCTIONS: 
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

#ifndef _SGA_REGS
#define _SGA_REGS

/****START OF SPECIFICATIONS *******************************************
 *    IDENTIFICATION: DEF.H                                            *
 *    DESCRIPTIVE NAME:  Salmon Graphics Adaptor Register Offsets      *
 *                       from the base address and possible values     *
 *                                                                     *
 *    FUNCTION:  Define register offsets and values                    *
 *                                                                     *
 *    ATTRIBUTES:                                                      *
 *    REFERENCES:  Salmon Graphics Adaptor Specification               *
 *                   Ron Arroyo, 4/11/90                               *
 ***********************************************************************
 *                                                                     *
 ****END   OF SPECIFICATIONS ******************************************/




/******************************************************************************/
/******************** BUID 40 Register BASE ADDRESSES *************************/
/******************************************************************************/

/************************************************Start of Brush Creek Addresses
#define SGA_ADDR_BASE    (0x04000000)               /*0x04000000*
#define VPD_BASE         (0x08000000+bus_base_addr) /*0x0C000000*
#define CNTL_REG_BASE    (0x04000000+bus_base_addr) /*0x08000000*
#define VRAM_LIN_BASE    (0x08400000+bus_base_addr) /*0x0C400000*
#define VRAM_XY_BASE     (0x00000000+bus_base_addr) /*0x04000000*
#define CURSOR_RAM_BASE  (0x04001000+bus_base_addr) /*0x08001000*
/*************************************************End of Brush Creek Addresses*/
/************************************************Start of Salmon Addresses**/
#define SGA_ADDR_BASE    (0x00000000)               /*0x00000000*/
#define VPD_BASE         (0x00000000+bus_base_addr) /*0x00000000*/
#define CNTL_REG_BASE    (0x08000000+bus_base_addr) /*0x08000000*/
#define VRAM_LIN_BASE    (0x00400000+bus_base_addr) /*0x00400000*/
#define VRAM_XY_BASE     (0x04000000+bus_base_addr) /*0x04000000*/
#define CURSOR_RAM_BASE  (0x08001000+bus_base_addr) /*0x08001000*/
/**************************************************End of Salmon Addresses*/

#define XYADDR(x,y,mode) (ulong *)(mode | (y)*YSHIFT + (x)*XSHIFT+bus_base_addr)
#define LINADDR(x,y)     (ulong *)(0x400000 | (y)*0x800  + (x) + bus_base_addr)

unsigned long bus_base_addr;/* received from BUSMEM_ATT(BUS_ID, SGA_ADDR_BASE)*/

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


/*--------------  Horz/Vert Display Control Register Offsets  ----------------*/
#define HTOTAL   (volatile unsigned long *)(CNTL_REG_BASE + 0x10)
#define HDISPE   (volatile unsigned long *)(CNTL_REG_BASE + 0x14)
#define HSSYNC   (volatile unsigned long *)(CNTL_REG_BASE + 0x18)
#define HESYNC   (volatile unsigned long *)(CNTL_REG_BASE + 0x1C)
#define VTOTAL   (volatile unsigned long *)(CNTL_REG_BASE + 0x30)
#define VDISPE   (volatile unsigned long *)(CNTL_REG_BASE + 0x34)
#define VSSYNC   (volatile unsigned long *)(CNTL_REG_BASE + 0x38)
#define VESYNC   (volatile unsigned long *)(CNTL_REG_BASE + 0x3C)
#define VINTR    (volatile unsigned long *)(CNTL_REG_BASE + 0x40)


/*---------------------  Load Length Register Offsets  -----------------------*/
#define RDLEN    (volatile unsigned long *)(CNTL_REG_BASE + 0x000080)


/*---------------------  Windowing Register Offsets  -------------------------*/
#define WORIG    (volatile unsigned long *)(CNTL_REG_BASE + 0x100)
#define WSTART   (volatile unsigned long *)(CNTL_REG_BASE + 0x104)
#define WEND     (volatile unsigned long *)(CNTL_REG_BASE + 0x108)


/*--------------------- Cursor Control Register Offsets  ---------------------*/
#define CURPOS   (volatile unsigned long *)(CNTL_REG_BASE + 0x10000)
#define COFFST   (volatile unsigned long *)(CNTL_REG_BASE + 0x10004)
#define CURCOL1  (volatile unsigned long *)(CNTL_REG_BASE + 0x10200)
#define CURCOL2  (volatile unsigned long *)(CNTL_REG_BASE + 0x10204)
#define CURCOL3  (volatile unsigned long *)(CNTL_REG_BASE + 0x10208)


/*---------------- Serializer Palette DAC Register Offsets  ------------------*/
#define SPD2AD   (volatile unsigned char *)(CNTL_REG_BASE + 0x20000)
#define SPD2DT1  (volatile unsigned char *)(CNTL_REG_BASE + 0x20004)
#define SPD2DT3  (volatile unsigned long *)(CNTL_REG_BASE + 0x2000C)


/*------------------- Error Condition Register Offsets  ----------------------*/
#define ERRADDR  (volatile unsigned long *)(CNTL_REG_BASE + 0x30000)

#endif /* _SGA_REGS */
