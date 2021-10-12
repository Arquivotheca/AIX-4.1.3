/* @(#)85       1.4  src/bos/diag/tu/wga/wgamisc.h, tu_wga, bos411, 9428A410j 7/27/93 22:24:23 */
/*
 *   COMPONENT_NAME: TU_WGA
 *
 *   FUNCTIONS: IGC_REG_READ
 *              IGC_REG_WRITE
 *              IGM_PACK
 *              VIDEO_ROM_SCAN_READ
 *              VRAM_LIN_READ
 *              VRAM_LIN_WRITE
 *              VRAM_XY_READ
 *              VRAM_XY_WRITE
 *              VRSADDR
 *              W8720ADDR
 *              W8720_ADDR
 *              W8720_NATIVE_READ
 *              W8720_NATIVE_WRITE
 *              W8720_READ
 *              WGA_REG_READ
 *              WGA_REG_WRITE
 *
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



#define SUCCESS 0
#define FAIL    1

#ifdef TRUE
#undef TRUE
#define TRUE    ((BOOL) 1)
#endif
#ifdef FALSE
#undef FALSE
#define FALSE   ((BOOL) 0)
#endif

/*****************************************************************************/
/********************         I/O  return codes         **********************/
/*****************************************************************************/

#define IO_ERROR                (-1)             /* returned by ioctl()      */


#define MAX_RAND           ((double) 32767)      /* max returned by rand()   */
#define GET_RAND_ULONG     (((ulong_t) rand() << 16) | ((ulong_t) rand()))
#define GET_RAND_UCHAR     ((uchar_t) ((rand() / MAX_RAND) * (double) 255))

#define BITS_IN_WORD       32
#define BITS_IN_BYTE       8
#define BYTES_IN_WORD      4

#define WTKN_YSHIFT        0x800


/*****************************************************************************/
/**************** REDEFINE DEVICE DRIVER ADDRESSES (regval.h) ****************/
/*****************************************************************************/

#define WTKN_SYS_BASE           WEITEK_BASE_ADDR
#define WTKN_INTR_ENBL_ADDR     WTKN_INTR_ENBL
#define MASK_PLANE_ADDR         MASK_PLANE    /* Ind. @ to Mask Plane        */
#define MASK_PIXEL_ADDR         MASK_PIXEL    /* Ind. @ to Mask Pixel        */
#define CNTL_REG1_ADDR          CNTL_REG1     /* Ind. @ to Control Reg 1     */
#define MISR_CNTL_ADDR          MISR_CNTL     /* Ind. @ to MISR Control Reg  */
#define DAC_GOOD_ADDR           DAC_GOOD      /* Ind. @ to DAC GOOD  status  */
#define PEL_CLOCK_ADDR          PEL_CLOCK     /* Ind. @ to Pixel Clock Freq  */
#define PROC_CLOCK_ADDR         PROC_CLOCK    /* Ind. @ to Processor Clock   */
#define CNTL_REG2_ADDR          CNTL_REG2     /* Ind. @ to Control Reg 2     */
#define CNTL_REG_BASE_ADDR      CNTL_REG_BASE /* Crtl register base address  */

#define VIDEO_ROM_BASE_ADDR     (0x0F000000+bus_base_addr)     /* 0x0F000000 */
#define W8720_BASE_ADDR         (0x09200000+bus_base_addr)

/*****************************************************************************/
/******************** VPD ADDRESSES/OFFSETS **********************************/
/*****************************************************************************/

/*-----------------  Vital Product Data Register Offsets  -------------------*/
#define VPD0_ADDR        VPD0
#define VPD1_ADDR        VPD1
#define VPD_DATA_ADDR    VPD_DATA

/*------------  Weitek 8720 Control Register Offsets  ----------------------*/

#define WTKN_INTR_ADDR  (volatile unsigned long *)(WEITEK_BASE_ADDR + 0x8)

/*****************************************************************************/
/***************** DISPLAY CONTROLLER ADDRESSES/OFFSETS **********************/
/*****************************************************************************/

/*-----------------  Control/Status Register Offsets  ------------------------*/
#define ADCNTL_ADDR   ((volatile ulong_t *)(CNTL_REG_BASE_ADDR + 0x00))
#define ADSTAT_ADDR   ((volatile ulong_t *)(CNTL_REG_BASE_ADDR + 0x04))


/*--------------  Horz/Vert Display Control Register Offsets  ----------------*/
#define HTOTAL_ADDR   (volatile ulong_t *)(CNTL_REG_BASE_ADDR + 0x10)
#define HDISPE_ADDR   (volatile ulong_t *)(CNTL_REG_BASE_ADDR + 0x14)
#define HSSYNC_ADDR   (volatile ulong_t *)(CNTL_REG_BASE_ADDR + 0x18)
#define HESYNC_ADDR   (volatile ulong_t *)(CNTL_REG_BASE_ADDR + 0x1C)
#define VTOTAL_ADDR   (volatile ulong_t *)(CNTL_REG_BASE_ADDR + 0x30)
#define VDISPE_ADDR   (volatile ulong_t *)(CNTL_REG_BASE_ADDR + 0x34)
#define VSSYNC_ADDR   (volatile ulong_t *)(CNTL_REG_BASE_ADDR + 0x38)
#define VESYNC_ADDR   (volatile ulong_t *)(CNTL_REG_BASE_ADDR + 0x3C)
#define VINTR_ADDR    (volatile ulong_t *)(CNTL_REG_BASE_ADDR + 0x40)


/*---------------------  Load Length Register Offsets  ----------------------*/
/* #define RDLEN_ADDR    (volatile ulong_t *)(CNTL_REG_BASE_ADDR + 0x000080) */


/*---------------------  Windowing Register Offsets  ------------------------*/
#define WORIG_ADDR       (volatile ulong_t *)(CNTL_REG_BASE_ADDR + 0x100)


/*---------------------  Video Rom Register Offsets  ------------------------*/
#define VIDEO_ROM_ADDR   (volatile ulong_t *)(VIDEO_ROM_BASE_ADDR + 0x000)


/*------------------- Error Condition Register Offsets  ---------------------*/
#define ERRADDR_ADDR  (volatile ulong_t *)(CNTL_REG_BASE_ADDR + 0x30000)



/*--------------------- Cursor Control Register -----------------------------*/
#define CURSOR_OFF      0x0000003F       /* disable 64x64 cursor plane 0 & 1 */
#define CURSOR_ON       0x000000C0       /* enable 64x64 cursor plane 0 & 1  */


/*---------------------------------------------------------------------------*/
/*--------------  Adapter Control Register (ADCNTL/adcntl)  -----------------*/
/*---------------------------------------------------------------------------*/
/*  Bits 0-6 -----------------------------------RESERVED---------------------*/
/*  Bits 7   ENABLE SYNC SIGNALS---------------------------------------------*/
/*  Bits 8   HORIZONTAL SYNC PULSE POLARITY----------------------------------*/
#define HSYNC_MINUS    0x00000000   /* Horizontal sync is MINUS active       */
#define HSYNC_PLUS     0x00800000   /* Horizontal sync is POSITIVE active    */
/*  Bits 9   ----------------------------------------------------------------*/
#define VSYNC_MINUS    0x00000000   /* Vertical sync is MINUS active         */
#define VSYNC_PLUS     0x00400000   /* Vertical sync is POSITIVE active      */
/*  Bit 10   ----------------------------------------------------------------*/
/*  Bit 11   -----------------------------------RESERVED---------------------*/
/*  Bits 12  WTKN INTERRUPT--------------------------------------------------*/
#define WTKN_INTR_OFF  0x00000000   /* Disable Vertical Interrupt            */
#define WTKN_INTR_ON   0x00080000   /* Enable Vertical Interrupt             */
/*  Bits 13  ERROR INTERRUPT-------------------------------------------------*/
#define E_INTR_OFF     0x00000000   /* Disable Error Interrupt               */
#define E_INTR_ON      0x00040000   /* Enable Error Interrupt                */
/*  Bits 14-23 ---------------------------------RESERVED---------------------*/
/*  Bits 24-28 FREQUENCY SELECT----------------------------------------------*/
/*  Bit  29    DIVIDE BY 8 DOTCLK TO WTKN------------------------------------*/
#define DOTCLK_FREQ_MASK    0x00000004  /* for monitors greater than 133 MHZ */
/*  Bits 30-31 ---------------------------------RESERVED---------------------*/


/*------------------- INTEGRATED GRAPHICS CONTROLLER MACROS -----------------*/
#define W8720ADDR(x,y)        ((ulong_t *) (W8720_BASE_ADDR + (y)*WTKN_YSHIFT + (x)))
#define W8720_ADDR(addr)      ((ulong_t *) (W8720_BASE_ADDR | (addr)))
#define VRSADDR(offset)       ((ulong_t *) (VIDEO_ROM_BASE_ADDR | (offset & 0xFFFFFFFC)))
#define VRAM_XY_WRITE(x, y, mode, data)  (*((ulong_t *) XYADDR(x, y, mode)) = data)
#define VRAM_XY_READ(x, y, mode)         (*((ulong_t *) XYADDR(x, y, mode)))
#define VRAM_LIN_WRITE(x, y, data)       (*((ulong_t *) LINADDR(x, y)) = data)
#define VRAM_LIN_READ(x, y)              (*((ulong_t *) LINADDR(x, y)))
#define VIDEO_ROM_SCAN_READ(offset)      (*(VRSADDR(offset)))
#define W8720_NATIVE_WRITE(x, y, data)   (*(W8720ADDR(x,y)) = data)
#define W8720_NATIVE_READ(x, y)          ((ulong_t) *(W8720ADDR(x,y)))
#define W8720_READ(addr)                 ((ulong_t) *(W8720_ADDR(addr)))
#define WGA_REG_WRITE(addr, data)        (*((ulong_t *) addr) = data)
#define WGA_REG_READ(addr)               (*((ulong_t *) addr))
#define IGC_REG_WRITE(addr, data)        (*((ulong_t *) addr) = data)
#define IGC_REG_READ(addr)               (*((ulong_t *) addr))

#define IGM_PACK(x, y)              ((x) << 16 & 0x1fff0000) | (y & 0x00001fff)


/*------------------- ERROR EXCEPTIONS --------------------------------------*/
#define QUAD_EXCEPTIONS                  0x18    /* Quad Exception & Concave */
#define BLIT_EXCEPTIONS                  0x20    /* Blit Exception           */



/*---------------------------------------------------------------------------*/
/* GLOBAL CONSTANTS:                                                         */
/*---------------------------------------------------------------------------*/
#define  IGC_F_MASK                       0xff00
#define  IGC_B_MASK                       0xf0f0
#define  IGC_S_MASK                       0xcccc
#define  IGC_D_MASK                       0xaaaa
#define  OVER_SIZED                       0x10000

#define  VBLANKED_INT                     0x30
#define  PICKED_INT                       0x0C
#define  DE_IDLE_INT                      0x03
#define  MASTER_ENABLE_INT                0xC0


