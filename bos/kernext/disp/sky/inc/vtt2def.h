/* @(#)13	1.11  src/bos/kernext/disp/sky/inc/vtt2def.h, sysxdispsky, bos411, 9428A410j 4/18/94 14:13:36 */
/*
 *   COMPONENT_NAME: SYSXDISPSKY
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

/*-----------------------------------------------------------------------*
*                                                                        *
*   IDENTIFICATION  VTT2DEF.H  870226                                    *
*   DESCRIPTION     Literal definitions for Skyway dd                    *
*   FUNCTION        Define literal values for use by the various         *
*                   subroutines of the SKYWAY  dd.                       *
*                                                                        *
*   UPDATES                                                              *
*                                                                        *
*   OWNER           C6C                                                  *
*                                                                        *
*-----------------------------------------------------------------------*/
 
 
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
#define SKYWAY_UNIQUE  696969
 
 
/*----------------------------------------------------------------------*/
/* Others                                                               */
/*----------------------------------------------------------------------*/
 
 
 
#define SKY_MAX_FONTS   8               /* max num of font table entries*/
#define SKY_RFNT_CLASS  2               /* real font class              */
#define REAL_FONT_PLAIN 0               /* plain font                   */
#define REAL_FONT_BOLD  1               /* bold  font                   */
#define REAL_FONT_ITALIC 2              /* italic font                  */
 
#define M_S_CHAN_ID     8               /* channel id for VTMP(type 113)*/
#define M_S_HOOK_ID     250             /* hook id for trace 250        */
#define M_V_CHAN_ID     8               /* channel id for VTMP(type 113)*/
#define M_V_HOOK_ID     251             /* hook id for trace 251        */
 
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

#define FRAME_FLYBACK     1          /* Frame Flyback interrupt code */
#define COPROCES_ACCESS_REJECT  4    /* Co Processor busy interrupt  */
#define COPROCES_OP_COMPLETE  8      /* Co Processor operation compl */

#define X_RES            1280    /* Max number of pels on x axis */
#define Y_RES            1024    /* Max number of pels on y axis */

#define SKY_COLOR        0x21    /* Device Id for color */
#define SKY_MONO         0x22    /* Device id for mono  */
#define NEW_POLL         0x01    /* Card supports new polling method */
#define POLLVAL          0x31    /* Card supports new polling method */

#define IO_SPACE_BASE   0x2100   /* Base address of pc io space registers */
#define MEM_SPACE_BASE  0xC0000  /* Base of memory space areas            */
#define BUS_ID          0x820c0020
#define COLOR_FONT_OFFSET  0x1c0000
#define MONO_FONT_OFFSET  0xc0000


/*----------------------------------------------------------------------*/
/* Defines for p space initialization                                   */
/*----------------------------------------------------------------------*/

#define BLANK_CODE      0x00200000   /* Blank ascii code                 */
#define ATTRIB_DEFAULT  0x00000000   /* Set attribute to color 6 (cyan)  */



/******************************************************************/
/* General Operation Fields for I/O and Coprocessor register data */
/******************************************************************/


#define NATMOTOROLA              0x0C
#define MOTO_ORDER_8BPP          0x0B
#define MOTO_ORDER_4BPP          0x0A
#define MOTO_ORDER_1BPP          0x08
#define S64_M256_PW64            0x009E
#define MONO_CONFIG              0x0096
#define EIGHTBPP                 0x0003
#define FOURBPP                  0x0002
#define HORIZ_TOTAL_1760         0x00DB
#define HORIZ_TOTAL_1808         0x00E1
#define PEL1504                  0x001B
#define PEL1704                  0x00D4   /* Mono H_syncP_end   */
#define PEL1448                  0x00B4   /* Mono H_syncP_start */
#define PEL1304                  0x00AF
#define PEL1280                  0x009F
#define PEL1280LO                0x000004FF
#define PEL1104                  0x0096
#define PEL1024                  0x03FF
#define PEL1024HI                0x03FF0000
#define ONE_F                    0x001F
#define FF                       0x00FF
#define COLOR_ONE                0x0001
#define TWO                      0x0002
#define THREE                    0x0003
#define FOUR                     0x0004
#define FIVE                     0x0005
#define EIGHT                    0x0008   /* Mono Vert_sync_end */
#define FORTY                    0x0040
#define FIFTY                    0x0050
#define ONE_SIXTY                0x00A0
#define PIX_MAP_A                0x0001
#define PIX_MAP_B                0x0002
#define PIX_MAP_C                0x0003
#define PIX_FMAT_1BPP            0x08
#define PIX_FMAT_4BPP            0x0A
#define PIX_FMAT_8BPP            0x0B
#define SPRITE_ON                0x0001
#define SPRITE_OFF               0x0000
#define COLOR_COMPARE_OFF        0x0004
#define LOAD_COLOR_RBGX          0x0004
#define LOAD_COLOR_RGB           0x0000
#define MODE_1_DATA              0x0063
#define MODE_1_DATA_M            0x00C3
#define MODE_1_RESET             0x0060
#define MODE_1_PRERESET          0x0061
#define FOUR_BPP_PIX_LINE         80
#define EIGHT_BPP_PIX_LINE        160
#define BYTES_CURSOR_ROW          8
#define LAST_CURSOR_BYTE          512
#define INVIS_DATA               0x0000
#define ONE_CSR_PEL              0x0080
#define TWO_CSR_PEL              0x00C0
#define THREE_CSR_PEL            0x00E0
#define FOUR_CSR_PEL             0x00F0
#define FIVE_CSR_PEL             0x00F8
#define SIX_CSR_PEL              0x00FC
#define SEVEN_CSR_PEL            0x00FE
#define EIGHT_CSR_PEL            0x00FF
#define BYTES_PER_LINE_4BPP      640
#define BYTES_PER_LINE_8BPP      1280
#define CURSOR_OFF               0x0004
#define CURSOR_ON                0x0044


/***********************************************************************/
/* Indexed register values                                             */
/***********************************************************************/

#define MEM_CONFIG               0x0000
#define MEM_PARAMTER             0x0100
#define HORIZ_TOTAL_REG          0x1000
#define HORIZ_DISPLAY_END        0x1200
#define HORIZ_BLANKING_START     0x1400
#define HORIZ_BLANKING_END       0x1600
#define HORIZ_SYNC_PUL_START     0x1800
#define HORIZ_SYNC_PUL_END1      0x1A00
#define HORIZ_SYNC_PUL_END2      0x1C00
#define HORIZ_SYNC_POS_REG       0x1E00
#define VERT_TOTAL_LO            0x2000
#define VERT_TOTAL_HI            0x2100
#define VERT_DISPLAY_END_LO      0x2200
#define VERT_DISPLAY_END_HI      0x2300
#define VERT_BLANK_START_LO      0x2400
#define VERT_BLANK_START_HI      0x2500
#define VERT_BLANK_END_LO        0x2600
#define VERT_BLANK_END_HI        0x2700
#define VERT_SYNC_PUL_START_LO   0x2800
#define VERT_SYNC_PUL_START_HI   0x2900
#define VERT_SYNC_PUL_END        0x2A00
#define VERT_LINE_COMPARE_LO     0x2C00
#define VERT_LINE_COMPARE_HI     0x2D00
#define SPRITE_HORIZ_START_LO    0x3000
#define SPRITE_HORIZ_START_HI    0x3100
#define SPRITE_HORIZ_START_PRE   0x3200
#define SPRITE_VERT_START_LO     0x3300
#define SPRITE_VERT_START_HI     0x3400
#define SPRITE_VERT_START_PRE    0x3500
#define SPRITE_CONTROL           0x3600
#define SPRITE_PALETTE_DATA      0x3800
#define START_ADDRESS_LO         0x4000
#define START_ADDRESS_MI         0x4100
#define START_ADDRESS_HI         0x4200
#define BUFFER_PITCH_LO          0x4300
#define BUFFER_PITCH_HI          0x4400
#define DISPLAY_MODE_1           0x5000
#define DISPLAY_MODE_2           0x5100
#define CLOCK_FREQ_SELECT        0x5400
#define SPRITE_ADR_LO_P0         0x5600
#define SPRITE_ADR_LO_P1         0x5700
#define SPRITE_ADR_HI_P0         0x5800
#define SPRITE_ADR_HI_P1         0x5900
#define SPRITE_IMAGE_P0          0x5A00
#define SPRITE_IMAGE_P1          0x5B00
#define PALETTE_INDEX_LO         0x6000
#define PALETTE_INDEX_HI         0x6100
#define SPRITE_INDEX_LO          0x6000
#define SPRITE_INDEX_HI          0x6100
#define PALETTE_MASK             0x6400
#define PALETTE_DATA             0x6500
#define PALETTE_SEQUENCE         0x6600
#define SPRITE_DATA              0x6A00
#define SPRITE_CNTRL_P0          0x6C00
#define SPRITE_CNTRL_P1          0x6D00

/****************************/
/* Pixel Map Index Register */
/****************************/
#define PixMapA   1
#define PixMapB   2
#define PixMapC   3
#define PixMapD   0  /* Mask Map */

/**********************/
/* Pixel Map n Format */
/**********************/
/* M/I Format */
#define MI0  0
#define MI1  0x8
/* Pixel Size */
#define PixSize1  0
#define PixSize2  1
#define PixSize4  2
#define PixSize8  3
#define PixSize16 4

/**********************************************************************/
/*  Octant fields                                                     */
/**********************************************************************/
#define  DX              0x4        /* 3rd pos. from the right        */
#define  DY              0x2        /* 2nd pos. from the right        */
#define  DZ              0x1        /* 1st pos. from the right        */

/**********************************************************************/
/*  Logical Operations for both Foreground & Background Mix           */
/**********************************************************************/
#define  Mix_All_0       0x00       /* All 0's                        */
#define  Mix_SrcAndDst   0x01       /* Source And Destination         */
#define  Mix_SrcAndCDst  0x02       /* Source And ^Destination        */
#define  Mix_Src         0x03       /* Source                         */
#define  Mix_CSrcAndDst  0x04       /* ^Source And Destination        */
#define  Mix_Dst         0x05       /* Destination                    */
#define  Mix_SrcXorDst   0x06       /* Source XOR  Destination        */
#define  Mix_SrcOrDst    0x07       /* Source OR   Destination        */
#define  Mix_CSrcAndCDst 0x08       /* ^Source And ^Destination       */
#define  Mix_SrcXorCDst  0x09       /*  Source XOR ^Destination       */
#define  Mix_CDst        0x0A       /* ^Destination                   */
#define  Mix_SrcOrCDst   0x0B       /* Source  OR ^Destination        */
#define  Mix_CSrc        0x0C       /* ^Source                        */
#define  Mix_CSrcOrDst   0x0D       /* ^Source  OR  Destination       */
#define  Mix_CSrcORCDst  0x0E       /* ^Source  OR ^Destination       */
#define  Mix_All_1       0x0F       /* All 1's                        */

/**********************************************************************/
/*  Color Compare Condition                                           */
/**********************************************************************/
#define  Color_Cmp_True  0x0        /* Always True (disable updates)  */
#define  Color_Grt_Col   0x1        /* Dest > Col value               */
#define  Color_Equ_Col   0x2        /* Dest = Col value               */
#define  Color_Les_Col   0x3        /* Dest < Col value               */
#define  Color_Cmp_Fal   0x4        /* Always False (enable updates)  */
#define  Color_GtEq_Col  0x5        /* Dest >= Col value              */
#define  Color_NtEq_Col  0x6        /* Dest <> Col value              */
#define  Color_LsEq_Col  0x7        /* Dest <= Col value              */

/**********************************************************************/
/*  Plane Mask Values                                                 */
/**********************************************************************/
#define  Plane_Mask_All  0xFFFF     /* All updates                    */

/**********************************************************************/
/*  Carry Chain Mask                                                  */
/**********************************************************************/
#define  Carry_Mask      0x3FFF     /* no breaks                      */


#define POBackReg 0                /* Background color  (register)   */
#define POBackSrc 0x80             /* Source Pixel Map               */

/* Foreground Source */
#define POForeReg 0                /* Foreground  color (register)   */
#define POForeSrc 0x20             /* Source Pixel Map               */

/* Step */
#define POStepDSR 0x2              /* Draw & Step Read               */
#define POStepLDR 0x3              /* Line Draw   Read               */
#define POStepDSW 0x4              /* Draw & Step Write              */
#define POStepLDW 0x5              /* Line Draw   Write              */
#define POStepBlt 0x8              /* Pxblt                          */
#define POStepIBlt 0x9             /* Inverting Pxblt                */
#define POStepAFBlt 0xa            /* Area Fill Pxblt                */

/* Source */
#define POSrcA 0x1000              /* Pixel Map A                    */
#define POSrcB 0x2000              /* Pixel Map B                    */
#define POSrcC 0x3000              /* Pixel Map C                    */

/* Destination */
#define PODestA 0x100              /* Pixel Map A                    */
#define PODestB 0x200              /* Pixel Map B                    */
#define PODestC 0x300              /* Pixel Map C                    */

/* Pattern */
#define POPatA 0x100000            /* Pixel Map A                    */
#define POPatB 0x200000            /* Pixel Map B                    */
#define POPatC 0x300000            /* Pixel Map C                    */
#define POPatFore 0x800000         /* Foreground (Fixed)             */
#define POPatSrc 0x900000          /* Generated from Source          */

/* Mask */
#define POMaskDis 0                /* Mask Map Disabled             */
#define POMaskBEn 0x40000000       /* Mask Map Boundary Enabled     */
#define POMaskEn  0x80000000       /* Mask Map Enabled              */

/* Drawing Mode */
#define POModeAll 0                /* Draw All Pixels                */
#define POModeLast 0x10000000      /* Draw 1s Pixel Null             */
#define POModeFirst 0x20000000     /* Draw Last Pixel Null           */
#define POModeArea 0x30000000      /* Draw Area Boundary             */

/* Direction Octant */
#define POOct0 0
#define POOct1 0x1000000
#define POOct2 0x2000000
#define POOct3 0x3000000
#define POOct4 0x4000000
#define POOct5 0x5000000
#define POOct6 0x6000000
#define POOct7 0x7000000


/****************************/
/* Pos register definitions */
/****************************/

#define PosReg1 0x01         /* This enables all non pos addresses */
			     /* actual io and ros address will be  */
			     /* ORed in by the configurator        */

#define PosReg2 0x06         /* This sets Burst inhibit off, fairness  */
			     /* enabled, and Ros decoding to enabled   */
			     /* The bus arbitration level              */
			     /* will be set by the device configurator */

#define PosReg3 0x01         /* The Vram window will be set by the     */
			     /* device configurator. Window is on.     */

#define PosReg4 0xC0         /* This turns the etiquette rules off.    */
			     /* 1 MB window is not used. Nibble On.    */


/****************************/
/* Interrupt Mask values    */
/****************************/

#define FRAME_FLYBACK_MASK 0x01
#define VERT_BLANK_END_MASK 0x02
#define SPRITE_END_MASK 0x04
#define COP_ACCESS_REJECT_MASK 0x40
#define OP_COMPLETE_MASK 0x80
#define OP_COMPLETE_CLEAR 0x7f
