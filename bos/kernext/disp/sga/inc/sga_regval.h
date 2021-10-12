/* @(#)23       1.3.2.3  src/bos/kernext/disp/sga/inc/sga_regval.h, sgadd, bos411, 9428A410j 2/18/94 14:54:12 */
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

#ifndef _SGA_REGVALS
#define _SGA_REGVALS

/****START OF SPECIFICATIONS *******************************************
 *    IDENTIFICATION: DEF2.H                                           *
 *    DESCRIPTIVE NAME:  Salmon Graphics Adaptor Register Values       *
 *                                                                     *
 *    FUNCTION:  Define register offsets and values                    *
 *                                                                     *
 *    ATTRIBUTES:                                                      *
 *    REFERENCES:  Salmon Graphics Adaptor Specification               *
 *                   Ron Arroyo, 4/11/90                               *
 ***********************************************************************
 *                                                                     *
 ****END   OF SPECIFICATIONS ******************************************/




/***************************************************************************/
/********************** BUID 40 Register VALUES  ***************************/
/***************************************************************************/






/***************************************************************************/
/******************* VPD Register Values ***********************************/
/***************************************************************************/

/*--------------  Vital Product Data Registers are  -----------------------*/
/*--------------------------- READ ONLY -----------------------------------*/


/***************************************************************************/
/******************* XYADDR Register Values ********************************/
/***************************************************************************/
#define YSHIFT          0x2000     /* shifts y value for XYADDR register*/
#define XSHIFT          0x4        /* shifts y value for XYADDR register*/
#define PART_W          0x04000000 /* write partial word mode in XYADDR reg*/
#define FULL_W          0x05000000 /* write fullword mode in XYADDR reg*/
#define PART_R          0x06000000 /* read partial in XYADDR register*/
#define FULL_R          0x05000000 /* read fullword mode in XYADDR register*/
#define PEL1_R          0x04000000 /* read 1 pel in XYADDR register*/
#define FULL            0x05000000 /* read/write fullword  in XYADDR reg*/
/* NOTE: for PART_R, length should be put in the RDLEN register */




/***************************************************************************/
/************** DISPLAY CONTROLLER REGISTER VALUES  ************************/
/***************************************************************************/

/*-------------------------------------------------------------------------*/
/*--------------  CRTC (CTR Controller Regs) (see define suffixes)---------*/
/*-------------------------------------------------------------------------*/
/*  8507, 8514, 8515 Monitor CRTC Values */
#define HTOTAL_MTYPE_1  0x13C
#define HDISPE_MTYPE_1  0x0FF
#define HSSYNC_MTYPE_1  0x10B
#define HESYNC_MTYPE_1  0x137
#define VTOTAL_MTYPE_1  0x198
#define VDISPE_MTYPE_1  0x17F
#define VSSYNC_MTYPE_1  0x17F
#define VESYNC_MTYPE_1  0x183
#define VINTR_MTYPE_1   0x000
#define PIXCLK_MTYPE_1  0x59

/*  8508 Monitor CRTC Values */
#define HTOTAL_MTYPE_2  0x1C4
#define HDISPE_MTYPE_2  0x13F
#define HSSYNC_MTYPE_2  0x160
#define HESYNC_MTYPE_2  0x1A0
#define VTOTAL_MTYPE_2  0x41F
#define VDISPE_MTYPE_2  0x3FF
#define VSSYNC_MTYPE_2  0x3FF
#define VESYNC_MTYPE_2  0x407
#define VINTR_MTYPE_2   0x000
#define PIXCLK_MTYPE_2  0xBF

/*  5081, 6091, OHAYO (111 mhz) Monitor CRTC Values */
#define HTOTAL_MTYPE_3  0x1B5
#define HDISPE_MTYPE_3  0x13F
#define HSSYNC_MTYPE_3  0x14B
#define HESYNC_MTYPE_3  0x17D
#define VTOTAL_MTYPE_3  0x41F
#define VDISPE_MTYPE_3  0x3FF
#define VSSYNC_MTYPE_3  0x402
#define VESYNC_MTYPE_3  0x405
#define VINTR_MTYPE_3   0x000
#define PIXCLK_MTYPE_3  0xAE

/*  6091 (67 Hz) Monitor CRTC Values */
#define HTOTAL_MTYPE_4  0x1A7
#define HDISPE_MTYPE_4  0x13F
#define HSSYNC_MTYPE_4  0x14D
#define HESYNC_MTYPE_4  0x175
#define VTOTAL_MTYPE_4  0x41F
#define VDISPE_MTYPE_4  0x3FF
#define VSSYNC_MTYPE_4  0x402
#define VESYNC_MTYPE_4  0x405
#define VINTR_MTYPE_4   0x000
#define PIXCLK_MTYPE_4  0xB7

/* OHAYO (76 Hz), WINDHAM Monitor CRTC Values */
#define HTOTAL_MTYPE_5  0x15F
#define HDISPE_MTYPE_5  0x0FF
#define HSSYNC_MTYPE_5  0x10B
#define HESYNC_MTYPE_5  0x15B
#define VTOTAL_MTYPE_5  0x325
#define VDISPE_MTYPE_5  0x2FF
#define VSSYNC_MTYPE_5  0x2FF
#define VESYNC_MTYPE_5  0x307
#define VINTR_MTYPE_5   0x000
#define PIXCLK_MTYPE_5  0x95

/* DAYTONA (1024 x 768) Monitor CRTC Values */
#define HTOTAL_MTYPE_6  0x156
#define HDISPE_MTYPE_6  0x0FF
#define HSSYNC_MTYPE_6  0x10B
#define HESYNC_MTYPE_6  0x151
#define VTOTAL_MTYPE_6  0x32E
#define VDISPE_MTYPE_6  0x2FF
#define VSSYNC_MTYPE_6  0x2FF
#define VESYNC_MTYPE_6  0x307
#define VINTR_MTYPE_6   0x000
#define PIXCLK_MTYPE_6  0x8D

/* DAYTONA (1280 x 1024) Monitor CRTC Values */
#define HTOTAL_MTYPE_7  0x1C7
#define HDISPE_MTYPE_7  0x13F
#define HSSYNC_MTYPE_7  0x153
#define HESYNC_MTYPE_7  0x1AF
#define VTOTAL_MTYPE_7  0x223
#define VDISPE_MTYPE_7  0x1FF
#define VSSYNC_MTYPE_7  0x200
#define VESYNC_MTYPE_7  0x204
#define VINTR_MTYPE_7   0x000
#define PIXCLK_MTYPE_7  0xA6

/* VENDOR (1280 x 1024, 108 Mhz) Monitor CRTC Values */
#define HTOTAL_MTYPE_8  0x1A9
#define HDISPE_MTYPE_8  0x13F
#define HSSYNC_MTYPE_8  0x153
#define HESYNC_MTYPE_8  0x181
#define VTOTAL_MTYPE_8  0x41F
#define VDISPE_MTYPE_8  0x3FF
#define VSSYNC_MTYPE_8  0x402
#define VESYNC_MTYPE_8  0x405
#define VINTR_MTYPE_8   0x000
#define PIXCLK_MTYPE_8  0xAB

/* VENDOR (1024 x 768, 60 Hz) Monitor CRTC Values */
#define HTOTAL_MTYPE_9  0x14A
#define HDISPE_MTYPE_9  0x0FF
#define HSSYNC_MTYPE_9  0x115
#define HESYNC_MTYPE_9  0x125
#define VTOTAL_MTYPE_9  0x31E
#define VDISPE_MTYPE_9  0x2FF
#define VSSYNC_MTYPE_9  0x300
#define VESYNC_MTYPE_9  0x304
#define VINTR_MTYPE_9   0x000
#define PIXCLK_MTYPE_9  0x7F

/* VENDOR (1024 x 768, 70 Hz) Monitor CRTC Values */
#define HTOTAL_MTYPE_10  0x14A
#define HDISPE_MTYPE_10  0x0FF
#define HSSYNC_MTYPE_10  0x11C
#define HESYNC_MTYPE_10  0x12F
#define VTOTAL_MTYPE_10  0x327
#define VDISPE_MTYPE_10  0x2FF
#define VSSYNC_MTYPE_10  0x2FF
#define VESYNC_MTYPE_10  0x301
#define VINTR_MTYPE_10   0x000
#define PIXCLK_MTYPE_10  0x8A

/* SUGARLOAF Monitor CRTC Values */
#define HTOTAL_MTYPE_11  0x1A5
#define HDISPE_MTYPE_11  0x13F
#define HSSYNC_MTYPE_11  0x14D
#define HESYNC_MTYPE_11  0x170
#define VTOTAL_MTYPE_11  0x423
#define VDISPE_MTYPE_11  0x3FF
#define VSSYNC_MTYPE_11  0x402
#define VESYNC_MTYPE_11  0x405
#define VINTR_MTYPE_11   0x000
#define PIXCLK_MTYPE_11  0xBF


/*-------------------------------------------------------------------------*/
/*--------------  Adapter Control Register (ADCNTL/adcntl)  ---------------*/
/*-------------------------------------------------------------------------*/
/*  Bits 0-1 --------------------------------------------------------------*/
#define BPP1        0x00000000      /* 1 Bit Per Pel                       */
#define BPP4        0x80000000      /* 4 Bit Per Pel                       */
#define BPP8        0xC0000000      /* 8 Bit Per Pel                       */
/*  Bits 2   -----------------------------------RESERVED-------------------*/
/*  Bits 3   --------------------------------------------------------------*/
#define RES_1024x768   0x00000000   /* 1024 x 768  Screen Resolution       */
#define RES_1280x1024  0x10000000   /* 1280 x 1024 Screen Resolution       */
/*  Bits 4-5 --------------------------------------------------------------*/
#define VRAM_8         0x00000000   /* 08 bit VRAM interface               */
#define VRAM_16        0x04000000   /* 16 bit VRAM interface               */
#define VRAM_32        0x08000000   /* 32 bit VRAM interface               */
/*  Bits 6   --------------------------------------------------------------*/
#define LACE_OFF       0x00000000   /* Turn interlace scan OFF             */
#define LACE_ON        0x02000000   /* Turn interlace scan ON              */
/*  Bits 7   --------------------------------------------------------------*/
#define VIDEO_OFF      0x00000000   /* Turn Video output OFF               */
#define VIDEO_ON       0x01000000   /* Turn Video output ON                */
/*  Bits 8   -----------------------------------------------------------*/
#define HSYNC_MINUS    0x00000000   /* Horizontal sync is MINUS active     */
#define HSYNC_PLUS     0x00800000   /* Horizontal sync is POSITIVE active  */
/*  Bits 9   --------------------------------------------------------------*/
#define VSYNC_MINUS    0x00000000   /* Vertical sync is MINUS active       */
#define VSYNC_PLUS     0x00400000   /* Vertical sync is POSITIVE active    */
/*  Bits 10  --------------------------------------------------------------*/
#define CURSOR_OFF     0x00000000   /* Disable Cursor Image                */
#define CURSOR_ON      0x00200000   /* Enable Cursor Image                 */
/*  Bits 11  --------------------------------------------------------------*/
#define HWCLIP_OFF     0x00000000   /* Disable Hardware Clipping           */
#define HWCLIP_ON      0x00100000   /* Enable Hardware Clipping            */
/*  Bits 12  --------------------------------------------------------------*/
#define V_INTR_OFF     0x00000000   /* Disable Vertical Interrupt          */
#define V_INTR_ON      0x00080000   /* Enable Vertical Interrupt           */
/*  Bits 13  --------------------------------------------------------------*/
#define E_INTR_OFF     0x00000000   /* Disable Error Interrupt             */
#define E_INTR_ON      0x00040000   /* Enable Error Interrupt              */
/*  Bits 14-15  -----------------------------------------------------------*/
#define VRAM_PG4       0x00000000   /* 4 cycles, CAS low for 2 cycles      */
#define VRAM_PG3       0x00020000   /* 3 cycles, CAS low for 2 cycles      */
#define VRAM_PG2       0x00010000   /* 2 cycles, CAS low for 1 cycles      */
/*  Bits 16  --------------------------------------------------------------*/
#define VRAM_RAS4      0x00000000   /* 4 cycles                            */
#define VRAM_RAS3      0x00004000   /* 3 cycles                            */
#define VRAM_RAS2      0x00008000   /* 2 cycles                            */
/*  Bits 17  --------------------------------------------------------------*/
#define VRAM_XTEND1    0x00000000   /* 1 cycle extention added to VRAM Load*/
#define VRAM_XTEND0    0x00004000   /* 0 cylce extention added to VRAM Load*/
/*  Bits 18  --------------------------------------------------------------*/
#define VRAM_LOAD1     0x00000000   /* 1 cycle added for load accesses     */
#define VRAM_LOAD0     0x00002000   /* no cycle added for load accesses    */
/*  Bits 19  --------------------------------------------------------------*/
#define VRAM_STORE1    0x00000000   /* 1 cycle added for store accesses    */
#define VRAM_STORE0    0x00001000   /* no cycle added for store accesses   */
/*  Bits 20-30----------------------------------RESERVED-------------------*/
/*  Bits 31  --------------------------------------------------------------*/
#define DIAG_SYS       0x00000000   /* System GA loads Control Reg         */
#define DIAG_CUR       0x00000001   /* Cursor GA loads Control Reg         */





/*-------------------------------------------------------------------------*/
/*---------  Adapter Status Register Values  (ADSTAT/adstat)  -------------*/
/*-------------------------------------------------------------------------*/
/*  Bits 0-3 --------------------------------------------------------------*/
#define STAT_ERR_MASK  0xF0000000 /*Use to Mask off non-err portion of Status */
#define NO_ERR         0x00000000 /* No Error Reported by Status Register    */
#define PARITY_ERR     0x10000000 /* Parity Error on RSC addr/data bus       */
#define INVAL_CMD      0x20000000 /* Invalid Command Error on RSC bus        */
#define RESV_ACCESS    0x40000000 /* Attempt to Access Reserved Register Error*/
#define BAD_STRING_OP  0x60000000 /* Illegal String Operation Error          */
#define INVAL_VALUE    0x70000000 /* Invalid Value put in Control Register   */
/*  Bits 4   --------------------------------------------------------------*/
#define V_INTR_SET     0x08000000 /* Vertical Interrupt was Reported         */
/*  Bits 5   --------------------------------------------------------------*/
#define V_BLANK_SET    0x04000000 /* Vertical Blanking is now in progress    */
/*  Bits 6-9   ---------------RESERVED-------------------------------------*/
/*  Bits 10-11   (ORed with the Cable ID (Sense Bits 7-4 or the SPD2)------*/
/*               NOTE: the lo nibble is the Cable ID or SPD2 Sense Bits    */
/*                     the hi nibble is the SGA Control Register ID Bits   */
#define MT_111         0x38      /* 5081,6091,OHAYO Monitors 111 Mhz.      */
#define MT_6091_67     0x30      /* IBM 6091 Monitor 67 Hz.                */
#define MT_8507        0x03      /* IBM 8507 Monitor                       */
#define MT_8508        0x0E      /* IBM 8508 Monitor                       */
#define MT_8514        0x05      /* IBM 8514 Monitor                       */
#define MT_8515        0x07      /* IBM 8515 Monitor                       */
#define MT_DT1280      0x14	 /* DAYTONA 1280x1024 Interlaced           */
#define MT_DT1024      0x34	 /* DAYTONA 1024x768 Non-interlaced        */
#define MT_OHY_76      0x18      /* OHAYO 76 Hz 1024x768                   */
#define MT_CABEZA      0x0D      /* CABEZA 76 Hz 1024x768                  */
#define MT_BALSAM      0x02      /* BALSAM 76 Hz 1024x768                  */
#define MT_VD_60       0x08      /* Vendor 60 Hz 1024x768                  */
#define MT_VD_108      0x28      /* Vendor 108 Mhz 1280x1024               */
#define MT_VD_70       0x00      /* Vendor 70 Hz 1024x768                  */
#define MT_SGR_LOAF    0x0F      /* Sugarloaf Monitor                      */
#define MT_WINDHAM     0x01      /* Winhham Monitor                        */
#define MT_NONE        0x3F      /* No Monitor Attached                    */
#define MT_UNKNOWN     0xFFF     /* Unrecognized Monitor                   */

#define ADSTAT_GRP0    0x0       /* need ADSTAT reg. for 6091 or VENDOR    */
#define ADSTAT_GRP1    0x4       /* need ADSTAT reg. for Daytona           */
#define ADSTAT_GRP2    0x8       /* need ADSTAT reg. for 6091,5081 or VENDOR */

/*  Bits 12-21-------------------RESERVED-------------------------------*/






/***************************************************************************/
/************** Serializer Palette DAC Register Values *********************/
/***************************************************************************/

/*-------------------------------------------------------------------------*/
/*------------------------  Indirect Addresses  ---------------------------*/
/*-------------------------------------------------------------------------*/
#define PALET_ADDR     0x00        /* Indirect Address to Palette Address */
#define PALET_DATA     0x01        /* Indirect Address to Palette Data    */
#define MASK_PLANE     0x02        /* Indirect Address to Mask Plane      */
#define MASK_PIXEL     0x03        /* Indirect Address to Mask Pixel      */
#define CNTL_REG1      0x04        /* Indirect Address to Control Reg 1   */
#define MISR_CNTL      0x05        /* Indirect Address to MISR Control Reg*/
#define DAC_GOOD       0x06        /* Indirect Address to DAC GOOD  status*/
#define PEL_CLOCK      0x07        /* Indirect Address to Pixel Clock Freq*/
#define PROC_CLOCK     0x08        /* Indirect Address to Processor Clock */
#define CNTL_REG2      0x09        /* Indirect Address to Control Reg 2   */

/*-------------------------------------------------------------------------*/
/*------------------------  Control Reg Values  ---------------------------*/
/*-------------------------------------------------------------------------*/
#define SPD_CNTL1      0x2B        /* Cntl Reg1:8 Bits Per Pel            */
                                   /*           Video Clock Divide by 4   */
                                   /*           Video Clock Divide by 4   */
                                   /*           Video Word Length = 32    */
                                   /*           Video On                  */
                                   /*           External Clock off;use4MHz*/

#define SPD_CNTL2      0x10        /* Cntl Reg2:                          */
                                   /*           Normal OP thru Palette    */
                                   /*           Use 5,6,5:red,green,blue  */
                                   /*           External Clock off;use4MHz*/
                                   /*           Function as SPD2          */
                                   /*           Load Serializer ShiftedBy2*/

/*-------------------------------------------------------------------------*/
/*------------------------  MISR    Reg Values  ---------------------------*/
/*-------------------------------------------------------------------------*/
#define MISR_OFF       0x00        /* MISR Reg: MISR in stop and hold mode*/
#define MISR_ON        0x01        /* MISR Reg: MISR in preset & run mode */

/*-------------------------------------------------------------------------*/
/*------------------------ X Server defines -------------------------------*/
/*-------------------------------------------------------------------------*/

#define BPP_CLEAR       0x3FFFFFFF   /* Value to clear bits 0-1             */
#define RES_CLEAR       0xEFFFFFFF   /* Value to clear bits 3               */
#define VRAM_CLEAR      0xF3FFFFFF   /* Value to clear bits 4-5             */
#define LACE_CLEAR      0xFDFFFFFF   /* Value to clear bits 6               */
#define VIDEO_CLEAR     0xFEFFFFFF   /* Value to clear bits 7               */
#define HSYNC_CLEAR     0xFF7FFFFF   /* Value to clear bits 8               */
#define VSYNC_CLEAR     0xFFBFFFFF   /* Value to clear bits 9               */
#define CURSOR_CLEAR    0xFFDFFFFF   /* Value to clear bits 10              */
#define HWCLIP_CLEAR    0xFFEFFFFF   /* Value to clear bits 11              */
#define V_INTR_CLEAR    0xFFF7FFFF   /* Value to clear bits 12              */
#define E_INTR_CLEAR    0xFFFBFFFF   /* Value to clear bits 13              */
#define VRAM_PC_CLEAR   0xFFFCFFFF   /* Value to clear bits 14-15           */
#define VRAM_RAS_CLEAR  0xFFFF3FFF   /* Value to clear bits 16-17           */
#define VRAM_RCL_CLEAR  0xFFFFDFFF   /* Value to clear bits 18              */
#define VRAM_RCS_CLEAR  0xFFFFEFFF   /* Value to clear bits 19              */
#define DIAG_CLEAR      0xFFFFFFFE   /* Value to clear bits 31              */

#define VRAM_PC44       0x00000000   /* 4 cycles read, 4 cycles write       */
#define VRAM_PC43       0x00010000   /* 4 cycles read, 3 cycles write       */
#define VRAM_PC33       0x00020000   /* 3 cycles read, 3 cycles write       */
#define VRAM_PC32       0x00030000   /* 3 cycles read, 2 cycles write       */

#define VRAM_RCL1       0x00000000   /* 1 cycle added for load accesses     */
#define VRAM_RCL0       0x00002000   /* No cycles added for load accesses   */
#define VRAM_RCS1       0x00000000   /* 1 cycle added for store accesses    */
#define VRAM_RCS0       0x00001000   /* No cycles added for store accesses  */

#define PIX_CLOCK_8507_14_15    0x59                           /*  53.0 MHz */
#define PIX_CLOCK_8508          0xBF                           /* 128.0 MHz */
#define PIX_CLOCK_5081          0xAF                           /*       MHz */
#define PIX_CLOCK_6091          0xB7                           /*       MHz */

#endif /* _SGA_REGVALS */

