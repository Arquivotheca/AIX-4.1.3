/* @(#)73	1.1  src/bos/diag/tu/gga/ggamisc.h, tu_gla, bos41J, 9515A_all 4/6/95 09:27:02 */
/*
*
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
*
*/

#define PCI_ID        0x80800852L  /* Machine ID register     */
#define PCISLOT1      0x80802000L  /* Sandalfoot PCI slot #1  */
#define PCISLOT2      0x80804000L  /* Sandalfoot PCI slot #2  */
#define PCISLOT       0x80807000L  /* Polo PCI slot           */
#define PCIINDEX      0x80000cf8L  /* Polo PCI index register */
#define PCIDATA       0x80000cfcL  /* Polo PCI data register  */

#define ULONG  unsigned long
#define VULONG volatile unsigned long
#define UINT   unsigned int
#define USHORT unsigned short
#define UCHAR  unsigned char

#define PATTERN_ENABLE_FIELD 14
#define DRAW_MODE_FIELD 15
#define MINTERM_FIELD 16
#define WIN_X_FIELD 3
#define WIN_Y_FIELD 19

#define NOCOLOR  BLACK

/* PCI configuration register offsets for 9100 */
/* Note: All data read/written from/to pci device is byte reversed at bridge */
#define PCI_WORD_0         0x00000000
#define   VEND_ID_LOW           0  
#define   VEND_ID_HIGH          1  
#define   DEVICE_ID_LOW         2  
#define   DEVICE_ID_HIGH        3  
#define PCI_WORD_1         0x00000004
#define   DAC_MEM_IO            4
#define   STATUS                7
#define PCI_WORD_2         0x00000008
#define   REVISION_ID           8
#define   VGA_PRESENT          10
#define   CLASS_CODE           11
#define PCI_WORD_4         0x00000010
#define   WBASE                19
#define PCI_WORD_12        0x00000030
#define   ROM_ENABLE           48
#define   ROM_BASE_0           49
#define   ROM_BASE_8_1         50
#define   ROM_BASE_16_9        51
#define PCI_WORD_16        0x00000040
#define   BUS_CFGBA_EEDAIN     64
#define   MODE_NATIVE_bBH      65
#define   CKSEL_VCEN           66

/***** 9100/9130 definitions *****/

/* Globals */
UCHAR  wbase;
USHORT monitor_type, machine_type;
UINT   display_mode;
ULONG  GGA_pcispace;
ULONG  LE_prefix, BE_prefix, prefix;

/* Base addresses (byte addressing) */
#define W9100_BASE_ADDR           (prefix)
#define SYS_CNTRL_REG_ADDR        (prefix | 0x00)
#define VIDEO_CNTRL_REG_ADDR      (prefix | 0x100)
#define VRAM_CNTRL_REG_ADDR       (prefix | 0x180)
#define RAMDAC_BASE_ADDR          (BE_prefix | 0x200)
#define W9130_BASE_ADDR           (prefix | 0x400)
#define PARAM_ENG_CNTRL_REG_ADDR  (prefix | 0x2180)
#define DRAW_ENG_CNTRL_REG_ADDR   (prefix | 0x2200)
#define PARAM_ENG_COORD_REG_ADDR  (prefix | 0x3000)
#define COORD_PSEUDO_REG_ADDR     (prefix | 0x3200)
#define FRAME_BUFFER              (BE_prefix | 0x800000)

#define SYS_HW_BUG                0x0000
#define VIDEO_HW_BUG              0x0100
#define RAMDAC_HW_BUG             0x0200
#define W9130_HW_BUG              0x0400

/* Registers */

#define W9100_PIXEL8_CMD          (prefix | 0x200c)
#define W9100_NEXT_PIXELS_CMD     (prefix | 0x2014)
#define W9100_PIXEL1_CMD          (prefix | 0x2080)
#define W9100_STATUS              (prefix | 0x2000)
#define W9100_BLIT_CMD            (prefix | 0x2004)
#define W9100_QUAD_CMD            (prefix | 0x2008)

#define W9100_OOR                 (PARAM_ENG_CNTRL_REG_ADDR | 0x0004)
#define W9100_CINDEX              (PARAM_ENG_CNTRL_REG_ADDR | 0x000C)
#define W9100_W_OFF_XY            (PARAM_ENG_CNTRL_REG_ADDR | 0x0010)
#define W9100_PE_W_WIN            (PARAM_ENG_CNTRL_REG_ADDR | 0x0014)
#define W9100_PE_W_MAX            (PARAM_ENG_CNTRL_REG_ADDR | 0x0018)
#define W9100_XCLIP               (PARAM_ENG_CNTRL_REG_ADDR | 0x0024)
#define W9100_YCLIP               (PARAM_ENG_CNTRL_REG_ADDR | 0x0020)

#define W9100_FOREGND             (DRAW_ENG_CNTRL_REG_ADDR | 0x0000)
#define W9100_BACKGND             (DRAW_ENG_CNTRL_REG_ADDR | 0x0004)
#define W9100_COLOR_0             (DRAW_ENG_CNTRL_REG_ADDR | 0x0000)
#define W9100_COLOR_1             (DRAW_ENG_CNTRL_REG_ADDR | 0x0004)
#define W9100_PLANE_MASK          (DRAW_ENG_CNTRL_REG_ADDR | 0x0008)
#define W9100_DRAW_MODE           (DRAW_ENG_CNTRL_REG_ADDR | 0x000c)
#define W9100_PAT_ORIGINX         (DRAW_ENG_CNTRL_REG_ADDR | 0x0010)
#define W9100_PAT_ORIGINY         (DRAW_ENG_CNTRL_REG_ADDR | 0x0014)
#define W9100_RASTER              (DRAW_ENG_CNTRL_REG_ADDR | 0x0018)
#define W9100_P_WINMIN            (DRAW_ENG_CNTRL_REG_ADDR | 0x0020)
#define W9100_P_WINMAX            (DRAW_ENG_CNTRL_REG_ADDR | 0x0024)
#define W9100_COLOR_2             (DRAW_ENG_CNTRL_REG_ADDR | 0x0038)
#define W9100_COLOR_3             (DRAW_ENG_CNTRL_REG_ADDR | 0x003c)
#define W9100_B_WINMIN            (DRAW_ENG_CNTRL_REG_ADDR | 0x00A0)
#define W9100_B_WINMAX            (DRAW_ENG_CNTRL_REG_ADDR | 0x00A4)

#define W9100_META_POINT_XY       (COORD_PSEUDO_REG_ADDR + 0x0018)
#define W9100_META_LINE_XY        (COORD_PSEUDO_REG_ADDR + 0x0058)
#define W9100_META_TRIANGLE_XY    (COORD_PSEUDO_REG_ADDR + 0x0098)
#define W9100_META_QUAD_XY        (COORD_PSEUDO_REG_ADDR + 0x00d8)
#define W9100_META_RECT_XY        (COORD_PSEUDO_REG_ADDR + 0x0118)

#define W9100_META_POINT_X        (COORD_PSEUDO_REG_ADDR + 0x0008)
#define W9100_META_LINE_X         (COORD_PSEUDO_REG_ADDR + 0x0048)
#define W9100_META_TRIANGLE_X     (COORD_PSEUDO_REG_ADDR + 0x0088)
#define W9100_META_QUAD_X         (COORD_PSEUDO_REG_ADDR + 0x00c8)
#define W9100_META_RECT_X         (COORD_PSEUDO_REG_ADDR + 0x0108)

#define W9100_META_POINT_Y        (COORD_PSEUDO_REG_ADDR + 0x0010)
#define W9100_META_LINE_Y         (COORD_PSEUDO_REG_ADDR + 0x0050)
#define W9100_META_TRIANGLE_Y     (COORD_PSEUDO_REG_ADDR + 0x0090)
#define W9100_META_QUAD_Y         (COORD_PSEUDO_REG_ADDR + 0x00d0)
#define W9100_META_RECT_Y         (COORD_PSEUDO_REG_ADDR + 0x0110)

#define W9100_COORD_XY0           (PARAM_ENG_COORD_REG_ADDR + 0x0038)
#define W9100_COORD_XY1           (PARAM_ENG_COORD_REG_ADDR + 0x0078)
#define W9100_COORD_XY2           (PARAM_ENG_COORD_REG_ADDR + 0x00B8)
#define W9100_COORD_XY3           (PARAM_ENG_COORD_REG_ADDR + 0x00F8)
#define W9100_COORD_X0            (PARAM_ENG_COORD_REG_ADDR + 0x0028)
#define W9100_COORD_X1            (PARAM_ENG_COORD_REG_ADDR + 0x0068)
#define W9100_COORD_X2            (PARAM_ENG_COORD_REG_ADDR + 0x00A8)
#define W9100_COORD_X3            (PARAM_ENG_COORD_REG_ADDR + 0x00E8)
#define W9100_COORD_Y0            (PARAM_ENG_COORD_REG_ADDR + 0x0030)
#define W9100_COORD_Y1            (PARAM_ENG_COORD_REG_ADDR + 0x0070)
#define W9100_COORD_Y2            (PARAM_ENG_COORD_REG_ADDR + 0x00B0)
#define W9100_COORD_Y3            (PARAM_ENG_COORD_REG_ADDR + 0x00F0)

#define W9100_HRZC                (VIDEO_CNTRL_REG_ADDR + 0x04)
#define W9100_HRZT                (VIDEO_CNTRL_REG_ADDR + 0x08)
#define W9100_HRZSR               (VIDEO_CNTRL_REG_ADDR + 0x0C)
#define W9100_HRZBR               (VIDEO_CNTRL_REG_ADDR + 0x10)
#define W9100_HRZBF               (VIDEO_CNTRL_REG_ADDR + 0x14)
#define W9100_PREHRZC             (VIDEO_CNTRL_REG_ADDR + 0x18)
#define W9100_VRTC                (VIDEO_CNTRL_REG_ADDR + 0x1C)
#define W9100_VRTT                (VIDEO_CNTRL_REG_ADDR + 0x20)
#define W9100_VRTSR               (VIDEO_CNTRL_REG_ADDR + 0x24)
#define W9100_VRTBR               (VIDEO_CNTRL_REG_ADDR + 0x28)
#define W9100_VRTBF               (VIDEO_CNTRL_REG_ADDR + 0x2c)
#define W9100_PREVRTC             (VIDEO_CNTRL_REG_ADDR + 0x30)
#define W9100_SRADDR              (VIDEO_CNTRL_REG_ADDR + 0x34)
#define W9100_SRTCTL              (VIDEO_CNTRL_REG_ADDR + 0x38)
#define W9100_SRADDR_INC          (VIDEO_CNTRL_REG_ADDR + 0x3C)
#define W9100_SRTCTL2             (VIDEO_CNTRL_REG_ADDR + 0x40)

#define W9100_PALCURRAMW          (RAMDAC_BASE_ADDR + 0x00)
#define W9100_PALDATA             (RAMDAC_BASE_ADDR + 0x04)
#define W9100_PIXELMASK           (RAMDAC_BASE_ADDR + 0x08)
#define W9100_PALCURRAMR          (RAMDAC_BASE_ADDR + 0x0c)
#define W9100_INDEXLOW            (RAMDAC_BASE_ADDR + 0x10)
#define W9100_INDEXHIGH           (RAMDAC_BASE_ADDR + 0x14)
#define W9100_INDEXDATA           (RAMDAC_BASE_ADDR + 0x18)
#define W9100_INDEXCONTR          (RAMDAC_BASE_ADDR + 0x1c)
#define   INDEX_CONTROL_MASK        (0x01)
#define     INDEX_CONTROL_INC_OFF     (0x00)
#define     INDEX_CONTROL_INC_ON      (0x01)

#define W9100_SYSCNFG             (SYS_CNTRL_REG_ADDR + 0x0004)
#define W9100_INTERRUPT           (SYS_CNTRL_REG_ADDR + 0x0008)
#define W9100_INT_EN              (SYS_CNTRL_REG_ADDR + 0x000c)

#define W9100_MEMCNFG             (VRAM_CNTRL_REG_ADDR + 0x04)
#define   MEMCNFG_MASK                  0x00000007
#define   VRAM_2MB                      0x00000006
#define   VRAM_4MB                      0x00000007
#define W9100_RFPER               (VRAM_CNTRL_REG_ADDR + 0x08)
#define W9100_RLMAX               (VRAM_CNTRL_REG_ADDR + 0x10)
#define W9100_PU_CONFIG           (VRAM_CNTRL_REG_ADDR + 0x18)

#define W9130_ID                  (W9130_BASE_ADDR + 0x00)
#define W9130_OUTPUT_MODE         (W9130_BASE_ADDR + 0x04)
#define   VERT_SYNC_POLARITY_0          0x00000000
#define   VERT_SYNC_POLARITY_1          0x20000000
#define   HORIZ_SYNC_POLARITY_0         0x00000000
#define   HORIZ_SYNC_POLARITY_1         0x10000000
#define   DEST_MODE_32BIT_PACKED        0x00000000
#define   DEST_MODE_24BIT_PACKED        0x01000000
#define   DEST_MODE_16BIT_PACKED_565    0x02000000
#define   DEST_MODE_16BIT_PACKED_555    0x03000000
#define   DEST_MODE_SINGLE_COMPONENT    0x04000000
#define W9130_MEM_CONFIG          (W9130_BASE_ADDR + 0x08)
#define   NOT_WAITING                   0x00000000
#define   WAITING                       0x00004000
#define   INT_MODE_ISA                  0x00000000
#define   INT_MODE_PCI                  0x00001000
#define   CONTINUE_EXECUTION            0x00000000
#define   PAUSE_EXECUTION               0x00000800
#define   LITTLE_ENDIAN                 0x00000000
#define   BIG_ENDIAN                    0x00000400
#define   DISABLED_SCALAR               0x00000000
#define   NORMAL_SCALAR                 0x00000100
#define   NORMAL_HOLD                   0x00000000
#define   EXTRA_HOLD                    0x00000080
#define   NORMAL_READS                  0x00000000
#define   SLOW_READS                    0x00000040
#define   NORMAL_WRITES                 0x00000000
#define   SLOW_WRITES                   0x00000020
#define   PREEMPT_MODE_0                0x00000000
#define   PREEMPT_MODE_1                0x00000010
#define   NORMAL_VRAM_SAMPLE            0x00000000
#define   LATE_VRAM_SAMPLE              0x00000008
#define W9130_ARBITRATION         (W9130_BASE_ADDR + 0x0C)
#define W9130_QUEUE_ADDRESS       (W9130_BASE_ADDR + 0x10)
#define W9130_QUEUE_POINTER       (W9130_BASE_ADDR + 0x14)
#define W9130_INTERRUPT_CONTROL   (W9130_BASE_ADDR + 0x18)
#define   ATTENTION_INT_STATUS_MASK     0x80000000
#define   SOURCE_VERT_SYNC_STATUS_MASK  0x00200000
#define   VP_PAUSED_STATUS_MASK         0x00080000
#define   QUEUE_EMPTY_STATUS_MASK       0x00040000
#define   VP_NOT_BUSY_STATUS_MASK       0x00020000
#define   P9100_VERT_SYNC_STATUS_MASK   0x00010000
#define     NOT_ACTIVE                  0x00
#define   ENABLE_SOURCE_VERT_SYNC_INT   0x00002000
#define   DISABLE_SOURCE_VERT_SYNC_INT  0x00000000
#define   ENABLE_FRAME_CAPTURE_INT      0x00001000
#define   DISABLE_FRAME_CAPTURE_INT     0x00000000
#define   ENABLE_VP_PAUSED_INT          0x00000800
#define   DISABLE_VP_PAUSED_INT         0x00000000
#define   ENABLE_QUEUE_EMPTY_INT        0x00000400
#define   DISABLE_QUEUE_EMPTY_INT       0x00000000
#define   ENABLE_VP_NOT_BUSY_INT        0x00000200
#define   DISABLE_VP_NOT_BUSY_INT       0x00000000
#define   ENABLE_P9100_VERT_SYNC_INT    0x00000100
#define   DISABLE_P9100_VERT_SYNC_INT   0x00000000
#define   SOURCE_VERT_SYNC_INT          0x00000020
#define   FRAME_CAPTURE_INT             0x00000010
#define   VP_PAUSED_INT                 0x00000008
#define   QUEUE_EMPTY_INT               0x00000004
#define   VP_NOT_BUSY_INT               0x00000002
#define   P9100_VERT_SYNC_INT           0x00000001
#define W9130_OUTPUT_LINE_COUNT   (W9130_BASE_ADDR + 0x1C)
#define W9130_I2C_INTERFACE       (W9130_BASE_ADDR + 0x20)
#define W9130_CAPTURE_CONTROL     (W9130_BASE_ADDR + 0x24)
#define   FRAME_RATE_CAPTURE_30FPS      0x00000000
#define   FRAME_RATE_CAPTURE_15FPS      0x00010000
#define   FRAME_RATE_CAPTURE_10FPS      0x00020000
#define   FRAME_RATE_CAPTURE_7FPS       0x00030000
#define   FIELD_CAPTURE_STOP            0x00000000
#define   FIELD_CAPTURE_EVEN            0x00001000
#define   FIELD_CAPTURE_ODD             0x00002000
#define   FIELD_CAPTURE_BOTH            0x00003000
#define   CAPTURE_SINGLE_BUFFER         0x00000000
#define   CAPTURE_DOUBLE_BUFFER         0x00000400
#define   CAPTURE_DISABLED              0x00000000
#define   CAPTURE_PHILIPS_ENABLED       0x00000100
#define   ACTIVE_BUFFER_STATUS_MASK     0x00000010
#define     BUFFER_0                    0x00
#define     BUFFER_1                    0x10
#define   SOURCE_FIELD_STATUS_MASK      0x00000008
#define     SOURCE_FIELD_EVEN           0x00
#define     SOURCE_FIELD_ODD            0x08
#define   CAPTURE_BUFFER_OVERRUN_MASK   0x00000004
#define     NO_OVERRUN                  0x00
#define     OVERRUN                     0x04
#define   BUFFER_1_FULL_MASK            0x00000002
#define     BUFFER_1_NOT_FULL           0x00
#define     BUFFER_1_FULL               0x02
#define   BUFFER_0_FULL_MASK            0x00000001
#define     BUFFER_0_NOT_FULL           0x00
#define     BUFFER_0_FULL               0x01
#define W9130_BUFF_0_ADDRESS      (W9130_BASE_ADDR + 0x28)
#define W9130_BUFF_1_ADDRESS      (W9130_BASE_ADDR + 0x2C)

#define RGB525_MISC_CLK     0x0002

/*****************************************************************************/
/********************         I/O  return codes         **********************/
/*****************************************************************************/

#define IO_ERROR                (-1)             /* returned by ioctl()      */


#define MAX_RAND           ((double) 32767)      /* max returned by rand()   */
#define GET_RAND_ULONG     (((ULONG) rand() << 16) | ((ULONG) rand()))
#define GET_RAND_UCHAR     ((uchar_t) ((rand() / MAX_RAND) * (double) 255))

#define BITS_IN_WORD       32
#define BITS_IN_BYTE       8
#define BYTES_IN_WORD      4

#define WTKN_YSHIFT        0x800


/*---------------------------- RGB525 stuff ---------------------------------*/

/* RAMDAC identifier constants */
#define BT485_c                         (0x01)
#define BT489_c                         (0x02)
#define RGB525_c                        (0x03)

/* Indexed registers */
#define RGB525_REVISION_LEVEL           (0x00)
#define RGB525_ID                       (0x01)
#define RGB525_MISC_CLOCK_CTL           (0x02)
#define RGB525_SYNC_CTL                 (0x03)
#define RGB525_HSYNC_POS                (0x04)
#define RGB525_POWER_MGNT               (0x05)
#define RGB525_DAC_OPER                 (0x06)
#define RGB525_PAL_CTRL                 (0x07)
#define RGB525_PIXEL_FORMAT             (0x0A)
#define RGB525_8BPP_CTL                 (0x0B)
#define RGB525_16BPP_CTL                (0x0C)
#define RGB525_24BPP_CTL                (0x0D)
#define RGB525_32BPP_CTL                (0x0E)
#define RGB525_PLL_CTL1                 (0x10)
#define RGB525_PLL_CTL2                 (0x11)
#define RGB525_FIXED_PLL_REF_DIV        (0x14)
#define RGB525_F0                       (0x20)
#define RGB525_F1                       (0x21)
#define RGB525_F2                       (0x22)
#define RGB525_F3                       (0x23)
#define RGB525_F4                       (0x24)
#define RGB525_F5                       (0x25)
#define RGB525_F6                       (0x26)
#define RGB525_F7                       (0x27)
#define RGB525_F8                       (0x28)
#define RGB525_F9                       (0x29)
#define RGB525_F10                      (0x2A)
#define RGB525_F11                      (0x2B)
#define RGB525_F12                      (0x2C)
#define RGB525_F13                      (0x2D)
#define RGB525_F14                      (0x2E)
#define RGB525_F15                      (0x2F)
#define RGB525_CURSOR_CTL               (0x30)
#define   RGB525_PIX_ORDR_MASK            (0x20)
#define     RGB525_PIX_ORDR_R_TO_L          (0x00)
#define     RGB525_PIX_ORDR_L_TO_R          (0x20)
#define   RGB525_LOC_READ_MASK            (0x10)
#define     RGB525_LOC_READ_WRITTEN         (0x00)
#define     RGB525_LOC_READ_ACTUAL          (0x10)
#define   RGB525_UPDT_CNTL_MASK           (0x08)
#define     RGB525_UPDT_CNTL_DELAYED        (0x00)
#define     RGB525_UPDT_CNTL_IMMEDIATE      (0x08)
#define   RGB525_CURSOR_SIZE_MASK         (0x04)
#define     RGB525_CURSOR_SIZE_32x32        (0x00)
#define     RGB525_CURSOR_SIZE_64x64        (0x04)
#define   RGB525_CURSOR_MODE_MASK         (0x03)
#define     RGB525_CURSOR_MODE_OFF          (0x00)
#define     RGB525_CURSOR_MODE_0            (0x01)
#define     RGB525_CURSOR_MODE_1            (0x02)
#define     RGB525_CURSOR_MODE_2            (0x03)
#define RGB525_CURSOR_X_LOW             (0x31)
#define RGB525_CURSOR_X_HIGH            (0x32)
#define RGB525_CURSOR_Y_LOW             (0x33)
#define RGB525_CURSOR_Y_HIGH            (0x34)
#define RGB525_CURSOR_HOT_X             (0x35)
#define RGB525_CURSOR_HOT_Y             (0x36)
#define RGB525_CURSOR_1_RED             (0x40)
#define RGB525_CURSOR_1_GREEN           (0x41)
#define RGB525_CURSOR_1_BLUE            (0x42)
#define RGB525_CURSOR_2_RED             (0x43)
#define RGB525_CURSOR_2_GREEN           (0x44)
#define RGB525_CURSOR_2_BLUE            (0x45)
#define RGB525_CURSOR_3_RED             (0x46)
#define RGB525_CURSOR_3_GREEN           (0x47)
#define RGB525_CURSOR_3_BLUE            (0x48)
#define RGB525_BORDER_RED               (0x60)
#define RGB525_BORDER_GREEN             (0x61)
#define RGB525_BORDER_BLUE              (0x62)
#define RGB525_MISC_CTL1                (0x70)
#define RGB525_MISC_CTL2                (0x71)
#define RGB525_MISC_CTL3                (0x72)
#define RGB525_DAC_SENSE                (0x82)
#define RGB525_MISR_RED                 (0x84)
#define RGB525_MISR_GREEN               (0x86)
#define RGB525_MISR_BLUE                (0x88)
#define RGB525_PLL_VCO_DIV              (0x8E)
#define RGB525_PLL_REF_DIV_IN           (0x8F)
#define RGB525_VRAM_MASK_LOW            (0x90)
#define RGB525_VRAM_MASK_HIGH           (0x91)
#define RGB525_CURSOR_ARRAY             (0x100)
#define ON                              (0xff)
#define OFF                             (0x0)

/*------------------- INTEGRATED GRAPHICS CONTROLLER MACROS -----------------*/

#define W9100ADDR(x,y)        ((ULONG *) (FRAME_BUFFER + (y)*WTKN_YSHIFT + (x)))
#define W9100_ADDR(addr)      ((ULONG *) (FRAME_BUFFER | (addr)))
#define W9100_NATIVE_WRITE(x, y, data)   (*(W9100ADDR(x,y)) = data)
#define W9100_NATIVE_READ(x, y)          ((ULONG) *(W9100ADDR(x,y)))
#define W9100_READ(addr)                 ((ULONG) *(W9100_ADDR(addr)))
#define IGC_REG_WRITE(addr, data)        (*((ULONG *) addr) = data)
#define IGC_REG_READ(addr)               (*((ULONG *) addr))

#define IGM_PACK(x, y)              ((x) << 16 & 0x1fff0000) | (y & 0x00001fff)

/*------------------- ERROR EXCEPTIONS --------------------------------------*/

#define QUAD_EXCEPTIONS                  0x18    /* Quad Exception & Concave */
#define BLIT_EXCEPTIONS                  0x20    /* Blit Exception           */

/*---------------------------------------------------------------------------*/
/* GLOBAL CONSTANTS:                                                         */
/*---------------------------------------------------------------------------*/

#define  IGC_F_MASK                       0xf00000f0
#define  IGC_B_MASK                       0xf00000f0 /* ??? */
#define  IGC_S_MASK                       0xf00000cc
#define  IGC_D_MASK                       0xf00000aa
#define  OVER_SIZED                       0xf0010000

#define ISSUEQBNMASK            (0x80000000)
#define BUSYMASK                (0x40000000)
#define OVERSIZE                (0xf0010000)
#define FMASKOVER               (0xf00100f0)
#define FMASK                   (0xf00000f0)
#define SMASKOVER               (0xf00100cc)
#define SMASK                   (0xf00000cc)
#define NSMASK                  (0xf0000033)
#define NDMASK                  (0xf0000055)

#define  VBLANKED_INT                     0x30
#define  PICKED_INT                       0x0C
#define  DE_IDLE_INT                      0x03
#define  MASTER_ENABLE_INT                0xC0

/* 9130 definitions */
#define LOW_PRIORITY     0     /* VideoPower has lower priority than   */
                               /* drawing engine and host              */
#define HIGH_PRIORITY    1     /* VideoPower has higher priority than  */
                               /* drawing engine and host              */
#define TIME_EXPIRED     0
#define TIME_LEFT        1
#define BAPIRUNTEST     -1
                         
#define VALID            0
#define INVALID         -1
                         
#define TOP              0
#define LEFT             1
#define RIGHT            2
#define BOTTOM           3
                         
#define LOW              0
#define HIGH             1
                         
#define OK               1
#define BAD              0
                         
#define YES              1
#define NO               0
                         
#define SUCCESS          0
#define FAIL             1
                         
#define TFT_MONITOR      6
#define NON_TFT_MONITOR  0

#ifdef TRUE
#undef TRUE
#define TRUE    ((BOOL) 1)
#endif

#ifdef FALSE
#undef FALSE
#define FALSE   ((BOOL) 0)
#endif

#define max(a, b)  ((a > b) ? a : b)
#define min(a, b)  ((a < b) ? a : b)

#define PACKYUV ((Input->SrcMode == 2) || (Input->SrcMode == 3))
#define YUVSRC ((Input->SrcMode > 7) || PACKYUV)

#define MODE640X480X8       0x201
#define MODE640X480X8_60HZ  0x202  /* 60Hz mode 201 - call it 202 */
#define MODE1024X768X8      0x205
#define MODE1280X1024X8     0x107
#define MODE1600X1280X8     0x274

enum {LOW_RES = 0, MEDIUM_RES, HIGH_RES, HIGHEST_RES};

enum {S_VIDEO = 0, COMPOSITE, CAMERA};

enum {INPUT_RES_80X60 = 0, INPUT_RES_160X120,
      INPUT_RES_240X180,   INPUT_RES_320X240};

enum {OUTPUT_RES_160X120 = 0, OUTPUT_RES_320X240, OUTPUT_RES_480X360,
      OUTPUT_RES_640X480,     OUTPUT_RES_800X600, OUTPUT_RES_960X720,
      OUTPUT_RES_1024X768,    OUTPUT_RES_1280X1024};

enum {BRIGHTNESS_LOW = 0,  BRIGHTNESS_NOMINAL,  BRIGHTNESS_HIGH,  BRIGHTNESS_MAX};

enum {CONTRAST_LOW = 0,    CONTRAST_NOMINAL,    CONTRAST_HIGH,    CONTRAST_MAX};

enum {HUE_LOW = 0,         HUE_NOMINAL,         HUE_HIGH,         HUE_MAX};

enum {SATURATION_LOW = 0,  SATURATION_NOMINAL,  SATURATION_HIGH,  SATURATION_MAX};

enum {CHROMA_GAIN_LOW = 0, CHROMA_GAIN_NOMINAL, CHROMA_GAIN_HIGH, CHROMA_GAIN_MAX};


/* The SrcSpec structure contains information about a frame of a video */
/* sequence, and where it is placed in offscreen memory */
typedef struct
                {
                int SrcWidth;      /* The width of the frame in pixels. For       */
                                   /* subsampled YUV images, the width refers     */
                                   /* to the Y component. For YUV 4:2:2 either    */
                                   /* packed or planar, The width must be a       */
                                   /* multiple of 2. For YUV 4:2:0 planar the     */
                                   /* width must be a multiple of 2 and for       */
                                   /* YUV 4:1:0 (Intel YUV9), the width must      */
                                   /* be a multiple of 4.                         */
                int SrcHeight;     /* The height of the frame in lines. For       */
                                   /* subsampled YUV images, the height refers    */
                                   /* to the Y component. For YUV 4:2:0 planar    */
                                   /* the height must be a multiple of 2 and      */
                                   /* for YUV 4:1:0 (Intel YUV9), the height      */
                                   /* must be a multiple of 4.                    */
                int SrcPitch;      /* The width of the frame in bytes. For YUV    */
                                   /* planar formats, this is the width of the    */
                                   /* Y component in bytes (same as width in      */
                                   /* pixels).                                    */
                int SrcStride;     /* The width of the frame in "DWORDS"          */
                                   /* (32-bit words) rounded up, after it has     */
                                   /* been transferred to offscreen memory.       */
                                   /* For planar YUV formats, this refers to      */
                                   /* the Y frame. For subsampled Planar formats  */
                                   /* there are restrictions on SrcStride. For    */
                                   /* Planar 4:2:2 and 4:2:0 formats, SrcStride   */
                                   /* must be even since VideoPower derives the   */
                                   /* stride of the Chrominance components by     */
                                   /* shifting SrcStride right by one position    */
                                   /* For Planar 4:1:0 (YUV9) format, SrcStride   */
                                   /* must be a multiple of 4 since VideoPower    */
                                   /* derives the stride of the chrominance by    */
                                   /* shifting right SrcStride by 2 poasitions    */
                int SrcChrPitch;   /* Width of the chrominance components in      */
                                   /* bytes. Only used with Planar frames.        */
                int SrcChrStride;  /* Width of the Chrominance components in      */
                                   /* DWORDS, rounded up, after the frame has     */
                                   /* transferred to offscreen memory. Only       */
                                   /* used with planar frames.                    */
                int SrcChrHeight;  /* Height of the Chrominance components in     */
                                   /* lines. Only used with planar formats.       */
                int SrcBytes;      /* Number of bytes per pixel. Set to 1 for     */
                                   /* planar formats.                             */
                int SrcBits;       /* Number of bits per pixel. Only used in      */
                                   /* Packed RGB formats.                         */
                int image_type;    /* Image format as described in the image      */
                                   /* header field. Values are as follows:-       */
                                   /* 2 - Packed RGB, number of bits defined      */
                                   /*       by SrcBits.                           */
                                   /* 3 - Packed 8 bit grey scale.                */
                                   /* 128 - Planar YUV 4:4:4                      */
                                   /* 129 - Planar YUV 4:2:2                      */
                                   /* 130 - Planar YUV 4:1:1                      */
                                   /* 131 - Planar YUV 4:2:0 (MPEG)               */
                                   /* 132 - Planar YUV 4:1:0 (YUV9)               */
                                   /* 133 - Packed YUV 4:2:2 (YYUV)               */
                                   /* 134 - Packed YUV 4:2:2 (YUYV)               */
                int SrcMode;       /* Image format as written to VideoPower       */
                                   /* Values are as follows:-                     */
                                   /* 0 - 32-bit RGB Packed.                      */
                                   /* 1 - 24-bit RGB Packed.                      */
                                   /* 2 - 16-bit YUV Packed (YYUV)                */
                                   /* 3 - 16-bit YUV Packed (YUYV)                */
                                   /* 6 - 16-bit RGB packed (5:5:5)               */
                                   /* 7 - 16-bit RGB packed (5:6:5)               */
                                   /* 8 - Planar YUV 4:4:4                        */
                                   /* 9 - Planar YUV 4:2:2                        */
                                   /* 10 - Planar YUV 4:1:1                       */
                                   /* 13 - Planar YUV 4:2:0 (MPEG)                */
                                   /* 14 - Planar YUV 4:1:0 (YUV9)                */
                                   /* 15 - 8-bit greyscale packed                 */
                char imag_des;     /* defines the origin of the frame. Used       */
                                   /* for orientation during scaling:-            */
                                   /* 0(hex) - Origin is at bottom left           */
                                   /* 20(hex) - Origin is at top left             */
                int SrcOrigin;     /* defines the origin of the frame. Used       */
                                   /* for orientation during scaling:-            */
                                   /* 0(hex) - Origin is at bottom left           */
                                   /* 1(hex) - Origin is at top left              */
                int SrcYSize;      /* The offscreen memory occupied by the        */
                                   /* image. For planar formats, the memory       */
                                   /* occupied by the Y component. Unit is DWORDS */
                int SrcChrSize;    /* For planar formats, the memory occupied     */
                                   /* by the each chrominance component.          */
                                   /* Unit is DWORDS                              */
                int SrcSize;       /* Total offscreen memory occupied by the      */
                                   /* frame. Unit is DWORDS.                      */
                int SrcAddr0;      /* Start address of the frame in offscreen     */
                                   /* memory (for planar formats, start address   */
                                   /* of the Y component. Unit is DWORDS.         */
                int SrcAddr1;      /* Start address of the U component. Used      */
                                   /* only in planar YUV formats. Unit is DWORDS  */
                int SrcAddr2;      /* Start address of the V component. Used      */
                                   /* only in planar YUV formats. Unit is DWORDS  */
                int SrcComps;      /* number of components in the frame. 3 for    */
                                   /* planar formats, 1 otherwise.                */
                } SrcSpec;

/* The DstSpec structure defines the size and position of an output window */
/* with respect to the top left of the screen */
typedef struct
                {
                long DstX;         /* X coordinate of left edge of window. -ve     */
                                   /* values indicate left window edge is off left */
                                   /* edge of screen                               */
                long DstY;         /* Y coordinate of top edge of window. -ve      */
                                   /* values indicate top window edge is off top   */
                                   /* edge of screen                               */
                long DstDx;        /* Width of window in pixels                    */
                long DstDy;        /* Height of window in lines                    */
                } DstSpec;

/* The ScrnSpec structure defines the current parameters of the screen  */
typedef struct
                {
                long ScrnMode;          /* Screen mode in a format as defined in */
                                        /* VideoPower registers:-                */
                                        /* 0 = 32-bits/pixel                     */
                                        /* 1 = 24-bits/pixel                     */
                                        /* 2 = 15-bits/pixel                     */
                                        /* 3 = 16-bits/pixel                     */
                                        /* 4 = 8-bits/pixel                      */
                long ScrnWidth;         /* Screen Width in pixels                */
                long ScrnHeight;        /* Screen Height in lines                */
                long ScrnPitch;         /* Screen Pitch in DWORDS                */
                long ScrnBitsPerPel;    /* Screen bits per pixel                 */
                long ScrnBytesPerPel;   /* Screen bytes per pixel                */
                } ScrnSpec;

/* The structure SegSpec defines a visible rectangle which is within    */
/* a window, after clipping to the screen and any overlaying windows    */
/* are taken into account.                                              */
typedef struct
                {
                long Status;            /* Defines whether the structure contains valid */
                                        /* data. 0 means the structure is valid, -1     */
                                        /* means the structure is invalid.              */
                long XOff;              /* The horizontal offset of the rectangle with  */
                                        /* reference to the left edge   of the original */
                                        /* window.                                      */
                long YOff;              /* The vertical offset of the rectangle with    */
                                        /* reference to the top edge    of the original */
                                        /* window.                                      */
                long X0;                /* The screen coordinate of the left edge of    */
                                        /* the rectangle.                               */
                long X1;                /* The screen coordinate of the right edge of   */
                                        /* the rectangle (inclusive).                   */
                long Y0;                /* The screen coordinate of the top edge of     */
                                        /* the rectangle.                               */
                long Y1;                /* The screen coordinate of the bottom edge of  */
                                        /* the rectangle (inclusive).                   */
                } SegSpec;


/* Monitor resolution video controller values */
#define  W1280x1024       0xff03ff04
#define  W1024x768        0xff02ff03
#define  W640x480         0xdf017f02

#define  NHres2048        0x00307000
#define  NHres1280        0x00006800
#define  NHres1024        0x00306000
#define  NHres640         0x00400700

#define  FontHeight640x480    14
#define  FontHeight1024x768   17
#define  FontHeight1280x1024  21

#define  FontWidth640x480     8
#define  FontWidth1024x768   10
#define  FontWidth1280x1024  12

/* ==================== sf_gga_init.c functions ============================= */
void setPixelClk(unsigned int ManchesterData);
void init_chips(void);
int init_crt(void);
void init_palette(void);
int get_monitor(void);
unsigned int gga_init(void);
unsigned int gga_detect(void);

/* ==================== sf_gga_util.c functions ============================= */
void Write_Reg_Bitfield(unsigned long address,
                        unsigned long value,
                        unsigned long field_position,
                        unsigned long field_length);
int Read_Reg_Bitfield(unsigned long address, unsigned long field,
                      unsigned long length);
void Setup_gga_String_Buffer(unsigned long *buffer_addr, unsigned long data);
void Print_Read_Buffer(unsigned long *buffer_addr, unsigned long Nwords);
int string_to_hex(char *string);
int string_to_dec(char * string);
void read_input(char *string);
int get_addr_data(int *hexaddr, int *hexdata);
int get_data(int *hexdata);
int get_addr(int *hexaddr);
int get_string_data(int *Nbytes, int *hexdata);
unsigned long gga_Get_Color(int color);
void Fill_VRAM_Native(int addr1, int addr2, int color);
void Clear_VRAM_Native(void);
void Draw_Shape(unsigned int type, unsigned int color);
void gga_dump_registers(void);
void vram_addr_native(void);
void gga_cls(int color);
char *gga_change_mode(void);
void WritePalette(int, int, unsigned char *);
void ReadPalette(int, int, unsigned char *);
void palettecheck(void);
void colorpalette(void);
void color216palette(void);
void dac_workaround(void);
void wait_for_wtkn_ready(void);
unsigned short get_monitor_type(void);
extern int get_slot(char *lname, unsigned int *, unsigned int *);
extern int wr_cfg_byte(int, unsigned char, unsigned int, unsigned int);
extern int rd_cfg_byte(int, unsigned char *, unsigned int, unsigned int);
extern int wr_cfg_word(int, unsigned long, unsigned int, unsigned int);
extern int rd_cfg_word(int, unsigned long *, unsigned int, unsigned int);

/* ==================== sf_gga_video.c functions =========================== */
unsigned int gga_video_power_test(void);
unsigned int gga_vp_nonint_auto_test(void);


